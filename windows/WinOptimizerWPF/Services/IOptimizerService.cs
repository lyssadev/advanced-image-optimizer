using System;
using System.Collections.ObjectModel;
using System.Threading.Tasks;
using WinOptimizerWPF.Models;

namespace WinOptimizerWPF.Services
{
    public interface IOptimizerService
    {
        Task OptimizeAsync(
            ObservableCollection<FileItem> files,
            OptimizerOptions options,
            IProgress<(int fileIndex, double fileProgress)> progress = null);

        Task<bool> IsGpuAvailableAsync();
        Task<int> GetMaxThreadCountAsync();
        Task<string[]> GetSupportedFormatsAsync();
    }
} 