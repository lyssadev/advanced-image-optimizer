using System.Windows;
using WinOptimizerWPF.ViewModels;
using WinOptimizerWPF.Services;

namespace WinOptimizerWPF.Views
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();

            // Initialize services
            var optimizerService = new OptimizerService();
            var dialogService = new DialogService();
            var settingsService = new SettingsService();

            // Set up view model
            DataContext = new MainViewModel(optimizerService, dialogService, settingsService);
        }
    }
} 