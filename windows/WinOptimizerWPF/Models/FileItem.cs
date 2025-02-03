using System;
using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;

namespace WinOptimizerWPF.Models
{
    public class FileItem : INotifyPropertyChanged
    {
        private string _filePath;
        private long _originalSize;
        private long _optimizedSize;
        private double _progress;
        private string _status;
        private bool _isProcessing;
        private bool _isCompleted;
        private bool _hasError;
        private string _errorMessage;

        public FileItem(string filePath)
        {
            FilePath = filePath;
            OriginalSize = new FileInfo(filePath).Length;
            Status = "Ready";
        }

        public string FilePath
        {
            get => _filePath;
            set => SetProperty(ref _filePath, value);
        }

        public string FileName => Path.GetFileName(FilePath);

        public long OriginalSize
        {
            get => _originalSize;
            set => SetProperty(ref _originalSize, value);
        }

        public string OriginalSizeFormatted => FormatFileSize(OriginalSize);

        public long OptimizedSize
        {
            get => _optimizedSize;
            set
            {
                if (SetProperty(ref _optimizedSize, value))
                {
                    OnPropertyChanged(nameof(OptimizedSizeFormatted));
                    OnPropertyChanged(nameof(CompressionRatio));
                    OnPropertyChanged(nameof(SpaceSaved));
                    OnPropertyChanged(nameof(SpaceSavedFormatted));
                }
            }
        }

        public string OptimizedSizeFormatted => FormatFileSize(OptimizedSize);

        public double CompressionRatio => OriginalSize > 0 ? 100.0 - ((double)OptimizedSize / OriginalSize * 100.0) : 0;

        public long SpaceSaved => OriginalSize - OptimizedSize;

        public string SpaceSavedFormatted => FormatFileSize(SpaceSaved);

        public double Progress
        {
            get => _progress;
            set => SetProperty(ref _progress, value);
        }

        public string Status
        {
            get => _status;
            set => SetProperty(ref _status, value);
        }

        public bool IsProcessing
        {
            get => _isProcessing;
            set => SetProperty(ref _isProcessing, value);
        }

        public bool IsCompleted
        {
            get => _isCompleted;
            set => SetProperty(ref _isCompleted, value);
        }

        public bool HasError
        {
            get => _hasError;
            set => SetProperty(ref _hasError, value);
        }

        public string ErrorMessage
        {
            get => _errorMessage;
            set => SetProperty(ref _errorMessage, value);
        }

        private static string FormatFileSize(long bytes)
        {
            string[] sizes = { "B", "KB", "MB", "GB", "TB" };
            int order = 0;
            double size = bytes;

            while (size >= 1024 && order < sizes.Length - 1)
            {
                order++;
                size /= 1024;
            }

            return $"{size:0.##} {sizes[order]}";
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        protected bool SetProperty<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (Equals(field, value)) return false;
            field = value;
            OnPropertyChanged(propertyName);
            return true;
        }
    }
} 