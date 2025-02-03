using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using WinOptimizerWPF.Models;

namespace WinOptimizerWPF.Services
{
    public class OptimizerService : IOptimizerService
    {
        private const string DllName = "optimizer.dll";

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern bool optimizer_init();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern void optimizer_cleanup();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern bool optimizer_process_file(
            [MarshalAs(UnmanagedType.LPStr)] string inputPath,
            [MarshalAs(UnmanagedType.LPStr)] string outputPath,
            IntPtr options,
            IntPtr progress_callback,
            IntPtr user_data);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern bool optimizer_is_gpu_available();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int optimizer_get_max_thread_count();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr optimizer_get_supported_formats(out int count);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern void optimizer_free_string_array(IntPtr array, int count);

        private bool _isInitialized;

        public OptimizerService()
        {
            Initialize();
        }

        ~OptimizerService()
        {
            if (_isInitialized)
            {
                optimizer_cleanup();
            }
        }

        private void Initialize()
        {
            if (!_isInitialized)
            {
                if (!optimizer_init())
                {
                    throw new InvalidOperationException("Failed to initialize the optimizer");
                }
                _isInitialized = true;
            }
        }

        public async Task OptimizeAsync(
            ObservableCollection<FileItem> files,
            OptimizerOptions options,
            IProgress<(int fileIndex, double fileProgress)> progress = null)
        {
            await Task.Run(() =>
            {
                for (int i = 0; i < files.Count; i++)
                {
                    var file = files[i];
                    try
                    {
                        file.IsProcessing = true;
                        file.Status = "Processing...";
                        file.Progress = 0;

                        var outputPath = GetOutputPath(file.FilePath, options.OutputFormat);
                        ProcessFile(file.FilePath, outputPath, options, progress, i);

                        file.OptimizedSize = new FileInfo(outputPath).Length;
                        file.Status = "Completed";
                        file.IsCompleted = true;
                        file.Progress = 100;
                    }
                    catch (Exception ex)
                    {
                        file.Status = "Error";
                        file.HasError = true;
                        file.ErrorMessage = ex.Message;
                    }
                    finally
                    {
                        file.IsProcessing = false;
                    }
                }
            });
        }

        private void ProcessFile(
            string inputPath,
            string outputPath,
            OptimizerOptions options,
            IProgress<(int fileIndex, double fileProgress)> progress,
            int fileIndex)
        {
            // TODO: Implement native options struct marshaling and progress callback
            if (!optimizer_process_file(inputPath, outputPath, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero))
            {
                throw new InvalidOperationException("Failed to process file");
            }
        }

        private string GetOutputPath(string inputPath, string format)
        {
            var directory = Path.GetDirectoryName(inputPath);
            var fileName = Path.GetFileNameWithoutExtension(inputPath);
            return Path.Combine(directory, $"{fileName}_optimized.{format.ToLower()}");
        }

        public async Task<bool> IsGpuAvailableAsync()
        {
            return await Task.Run(() => optimizer_is_gpu_available());
        }

        public async Task<int> GetMaxThreadCountAsync()
        {
            return await Task.Run(() => optimizer_get_max_thread_count());
        }

        public async Task<string[]> GetSupportedFormatsAsync()
        {
            return await Task.Run(() =>
            {
                var formatsPtr = optimizer_get_supported_formats(out int count);
                if (formatsPtr == IntPtr.Zero)
                {
                    return Array.Empty<string>();
                }

                try
                {
                    var formats = new string[count];
                    var size = Marshal.SizeOf<IntPtr>();
                    for (int i = 0; i < count; i++)
                    {
                        var strPtr = Marshal.ReadIntPtr(formatsPtr, i * size);
                        formats[i] = Marshal.PtrToStringAnsi(strPtr);
                    }
                    return formats;
                }
                finally
                {
                    optimizer_free_string_array(formatsPtr, count);
                }
            });
        }
    }
} 