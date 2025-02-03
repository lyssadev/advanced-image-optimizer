using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;
using System.Windows.Input;
using Microsoft.Win32;
using WinOptimizerWPF.Models;
using WinOptimizerWPF.Commands;

namespace WinOptimizerWPF.ViewModels
{
    public class MainViewModel : INotifyPropertyChanged
    {
        private readonly IOptimizerService _optimizerService;
        private readonly IDialogService _dialogService;
        private readonly ISettingsService _settingsService;

        private string _statusText;
        private double _progress;
        private bool _isProcessing;
        private string _selectedFormat;
        private int _quality;
        private bool _preserveMetadata;
        private bool _autoEnhance;
        private bool _smartCompress;
        private int _maxWidth;
        private int _maxHeight;
        private bool _faceEnhance;
        private bool _superResolution;
        private bool _smartCrop;
        private bool _useGpu;
        private int _selectedThreadCount;

        public MainViewModel(
            IOptimizerService optimizerService,
            IDialogService dialogService,
            ISettingsService settingsService)
        {
            _optimizerService = optimizerService;
            _dialogService = dialogService;
            _settingsService = settingsService;

            // Initialize commands
            AddFilesCommand = new AsyncRelayCommand(AddFilesAsync);
            AddFolderCommand = new AsyncRelayCommand(AddFolderAsync);
            ClearAllCommand = new RelayCommand(ClearAll);
            StartOptimizationCommand = new AsyncRelayCommand(StartOptimizationAsync, CanStartOptimization);
            RemoveFileCommand = new RelayCommand<FileItem>(RemoveFile);

            // Initialize collections
            Files = new ObservableCollection<FileItem>();
            OutputFormats = new ObservableCollection<string> { "WebP", "JPEG", "PNG", "AVIF" };
            ThreadCounts = new ObservableCollection<int> { 1, 2, 4, 8, 16 };

            // Load settings
            LoadSettings();
        }

        public ObservableCollection<FileItem> Files { get; }
        public ObservableCollection<string> OutputFormats { get; }
        public ObservableCollection<int> ThreadCounts { get; }

        public string StatusText
        {
            get => _statusText;
            set => SetProperty(ref _statusText, value);
        }

        public double Progress
        {
            get => _progress;
            set => SetProperty(ref _progress, value);
        }

        public bool IsProcessing
        {
            get => _isProcessing;
            set
            {
                if (SetProperty(ref _isProcessing, value))
                {
                    (StartOptimizationCommand as AsyncRelayCommand)?.RaiseCanExecuteChanged();
                }
            }
        }

        public string SelectedFormat
        {
            get => _selectedFormat;
            set
            {
                if (SetProperty(ref _selectedFormat, value))
                {
                    SaveSettings();
                }
            }
        }

        public int Quality
        {
            get => _quality;
            set
            {
                if (SetProperty(ref _quality, value))
                {
                    SaveSettings();
                }
            }
        }

        public bool PreserveMetadata
        {
            get => _preserveMetadata;
            set
            {
                if (SetProperty(ref _preserveMetadata, value))
                {
                    SaveSettings();
                }
            }
        }

        public bool AutoEnhance
        {
            get => _autoEnhance;
            set
            {
                if (SetProperty(ref _autoEnhance, value))
                {
                    SaveSettings();
                }
            }
        }

        public bool SmartCompress
        {
            get => _smartCompress;
            set
            {
                if (SetProperty(ref _smartCompress, value))
                {
                    SaveSettings();
                }
            }
        }

        public int MaxWidth
        {
            get => _maxWidth;
            set
            {
                if (SetProperty(ref _maxWidth, value))
                {
                    SaveSettings();
                }
            }
        }

        public int MaxHeight
        {
            get => _maxHeight;
            set
            {
                if (SetProperty(ref _maxHeight, value))
                {
                    SaveSettings();
                }
            }
        }

        public bool FaceEnhance
        {
            get => _faceEnhance;
            set
            {
                if (SetProperty(ref _faceEnhance, value))
                {
                    SaveSettings();
                }
            }
        }

        public bool SuperResolution
        {
            get => _superResolution;
            set
            {
                if (SetProperty(ref _superResolution, value))
                {
                    SaveSettings();
                }
            }
        }

        public bool SmartCrop
        {
            get => _smartCrop;
            set
            {
                if (SetProperty(ref _smartCrop, value))
                {
                    SaveSettings();
                }
            }
        }

        public bool UseGpu
        {
            get => _useGpu;
            set
            {
                if (SetProperty(ref _useGpu, value))
                {
                    SaveSettings();
                }
            }
        }

        public int SelectedThreadCount
        {
            get => _selectedThreadCount;
            set
            {
                if (SetProperty(ref _selectedThreadCount, value))
                {
                    SaveSettings();
                }
            }
        }

        public ICommand AddFilesCommand { get; }
        public ICommand AddFolderCommand { get; }
        public ICommand ClearAllCommand { get; }
        public ICommand StartOptimizationCommand { get; }
        public ICommand RemoveFileCommand { get; }

        private async Task AddFilesAsync()
        {
            var files = await _dialogService.ShowOpenFileDialogAsync(
                "Image Files|*.jpg;*.jpeg;*.png;*.webp;*.heic;*.heif|All Files|*.*",
                true);

            if (files != null)
            {
                foreach (var file in files)
                {
                    Files.Add(new FileItem(file));
                }
                UpdateStatus();
            }
        }

        private async Task AddFolderAsync()
        {
            var folder = await _dialogService.ShowFolderBrowserDialogAsync();
            if (folder != null)
            {
                var files = System.IO.Directory.GetFiles(
                    folder,
                    "*.*",
                    System.IO.SearchOption.AllDirectories);

                foreach (var file in files)
                {
                    if (IsImageFile(file))
                    {
                        Files.Add(new FileItem(file));
                    }
                }
                UpdateStatus();
            }
        }

        private void ClearAll()
        {
            Files.Clear();
            UpdateStatus();
        }

        private void RemoveFile(FileItem file)
        {
            if (file != null)
            {
                Files.Remove(file);
                UpdateStatus();
            }
        }

        private bool CanStartOptimization()
        {
            return Files.Count > 0 && !IsProcessing;
        }

        private async Task StartOptimizationAsync()
        {
            try
            {
                IsProcessing = true;
                Progress = 0;
                StatusText = "Optimizing images...";

                var options = new OptimizerOptions
                {
                    OutputFormat = SelectedFormat,
                    Quality = Quality,
                    PreserveMetadata = PreserveMetadata,
                    AutoEnhance = AutoEnhance,
                    SmartCompress = SmartCompress,
                    MaxWidth = MaxWidth,
                    MaxHeight = MaxHeight,
                    FaceEnhance = FaceEnhance,
                    SuperResolution = SuperResolution,
                    SmartCrop = SmartCrop,
                    UseGpu = UseGpu,
                    ThreadCount = SelectedThreadCount
                };

                var progress = new Progress<(int fileIndex, double fileProgress)>(OnProgress);
                await _optimizerService.OptimizeAsync(Files, options, progress);

                StatusText = "Optimization completed successfully!";
            }
            catch (Exception ex)
            {
                await _dialogService.ShowErrorAsync("Optimization Error", ex.Message);
                StatusText = "Optimization failed.";
            }
            finally
            {
                IsProcessing = false;
                Progress = 0;
            }
        }

        private void OnProgress((int fileIndex, double fileProgress) progress)
        {
            var totalProgress = (progress.fileIndex + progress.fileProgress) / Files.Count * 100;
            Progress = totalProgress;
            StatusText = $"Processing file {progress.fileIndex + 1} of {Files.Count}...";
        }

        private void UpdateStatus()
        {
            StatusText = Files.Count == 0
                ? "Add files to begin optimization"
                : $"Ready to optimize {Files.Count} files";
        }

        private bool IsImageFile(string path)
        {
            var extension = System.IO.Path.GetExtension(path).ToLower();
            return extension is ".jpg" or ".jpeg" or ".png" or ".webp" or ".heic" or ".heif";
        }

        private void LoadSettings()
        {
            var settings = _settingsService.LoadSettings();
            SelectedFormat = settings.OutputFormat;
            Quality = settings.Quality;
            PreserveMetadata = settings.PreserveMetadata;
            AutoEnhance = settings.AutoEnhance;
            SmartCompress = settings.SmartCompress;
            MaxWidth = settings.MaxWidth;
            MaxHeight = settings.MaxHeight;
            FaceEnhance = settings.FaceEnhance;
            SuperResolution = settings.SuperResolution;
            SmartCrop = settings.SmartCrop;
            UseGpu = settings.UseGpu;
            SelectedThreadCount = settings.ThreadCount;
        }

        private void SaveSettings()
        {
            var settings = new OptimizerSettings
            {
                OutputFormat = SelectedFormat,
                Quality = Quality,
                PreserveMetadata = PreserveMetadata,
                AutoEnhance = AutoEnhance,
                SmartCompress = SmartCompress,
                MaxWidth = MaxWidth,
                MaxHeight = MaxHeight,
                FaceEnhance = FaceEnhance,
                SuperResolution = SuperResolution,
                SmartCrop = SmartCrop,
                UseGpu = UseGpu,
                ThreadCount = SelectedThreadCount
            };
            _settingsService.SaveSettings(settings);
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