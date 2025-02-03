using System;
using System.IO;
using System.Text.Json;
using WinOptimizerWPF.Models;

namespace WinOptimizerWPF.Services
{
    public class SettingsService : ISettingsService
    {
        private const string SettingsFileName = "optimizer_settings.json";
        private readonly string _settingsFilePath;

        public SettingsService()
        {
            var appDataPath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
            var appFolder = Path.Combine(appDataPath, "ImageOptimizer");
            Directory.CreateDirectory(appFolder);
            _settingsFilePath = Path.Combine(appFolder, SettingsFileName);
        }

        public OptimizerSettings LoadSettings()
        {
            try
            {
                if (File.Exists(_settingsFilePath))
                {
                    var json = File.ReadAllText(_settingsFilePath);
                    return JsonSerializer.Deserialize<OptimizerSettings>(json) ?? CreateDefaultSettings();
                }
            }
            catch (Exception)
            {
                // Ignore errors and return default settings
            }

            return CreateDefaultSettings();
        }

        public void SaveSettings(OptimizerSettings settings)
        {
            try
            {
                var json = JsonSerializer.Serialize(settings, new JsonSerializerOptions
                {
                    WriteIndented = true
                });
                File.WriteAllText(_settingsFilePath, json);
            }
            catch (Exception)
            {
                // Ignore errors
            }
        }

        private static OptimizerSettings CreateDefaultSettings()
        {
            return new OptimizerSettings
            {
                OutputFormat = "WebP",
                Quality = 85,
                PreserveMetadata = true,
                AutoEnhance = false,
                SmartCompress = true,
                MaxWidth = 0,
                MaxHeight = 0,
                FaceEnhance = false,
                SuperResolution = false,
                SmartCrop = false,
                UseGpu = true,
                ThreadCount = 4
            };
        }
    }
} 