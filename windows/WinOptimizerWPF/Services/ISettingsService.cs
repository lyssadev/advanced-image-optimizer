using WinOptimizerWPF.Models;

namespace WinOptimizerWPF.Services
{
    public interface ISettingsService
    {
        OptimizerSettings LoadSettings();
        void SaveSettings(OptimizerSettings settings);
    }
} 