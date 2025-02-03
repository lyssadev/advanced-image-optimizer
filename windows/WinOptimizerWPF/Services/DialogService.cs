using System.Threading.Tasks;
using System.Windows;
using Microsoft.Win32;
using System.Windows.Forms;
using MessageBox = System.Windows.MessageBox;

namespace WinOptimizerWPF.Services
{
    public class DialogService : IDialogService
    {
        public Task<string[]> ShowOpenFileDialogAsync(string filter, bool multiSelect)
        {
            var dialog = new OpenFileDialog
            {
                Filter = filter,
                Multiselect = multiSelect,
                Title = "Select Image Files"
            };

            return Task.FromResult(dialog.ShowDialog() == true
                ? dialog.FileNames
                : null);
        }

        public Task<string> ShowFolderBrowserDialogAsync()
        {
            using var dialog = new FolderBrowserDialog
            {
                Description = "Select Folder with Images",
                UseDescriptionForTitle = true,
                ShowNewFolderButton = true
            };

            return Task.FromResult(dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK
                ? dialog.SelectedPath
                : null);
        }

        public Task ShowErrorAsync(string title, string message)
        {
            MessageBox.Show(
                message,
                title,
                MessageBoxButton.OK,
                MessageBoxImage.Error);

            return Task.CompletedTask;
        }

        public Task<bool> ShowConfirmationAsync(string title, string message)
        {
            var result = MessageBox.Show(
                message,
                title,
                MessageBoxButton.YesNo,
                MessageBoxImage.Question);

            return Task.FromResult(result == MessageBoxResult.Yes);
        }

        public Task ShowInfoAsync(string title, string message)
        {
            MessageBox.Show(
                message,
                title,
                MessageBoxButton.OK,
                MessageBoxImage.Information);

            return Task.CompletedTask;
        }
    }
} 