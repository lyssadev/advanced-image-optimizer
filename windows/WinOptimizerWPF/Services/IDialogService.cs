using System.Threading.Tasks;

namespace WinOptimizerWPF.Services
{
    public interface IDialogService
    {
        Task<string[]> ShowOpenFileDialogAsync(string filter, bool multiSelect);
        Task<string> ShowFolderBrowserDialogAsync();
        Task ShowErrorAsync(string title, string message);
        Task<bool> ShowConfirmationAsync(string title, string message);
        Task ShowInfoAsync(string title, string message);
    }
} 