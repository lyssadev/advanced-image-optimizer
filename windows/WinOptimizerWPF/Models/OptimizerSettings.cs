namespace WinOptimizerWPF.Models
{
    public class OptimizerSettings
    {
        public string OutputFormat { get; set; } = "WebP";
        public int Quality { get; set; } = 85;
        public bool PreserveMetadata { get; set; } = true;
        public bool AutoEnhance { get; set; } = false;
        public bool SmartCompress { get; set; } = true;
        public int MaxWidth { get; set; } = 0;
        public int MaxHeight { get; set; } = 0;
        public bool FaceEnhance { get; set; } = false;
        public bool SuperResolution { get; set; } = false;
        public bool SmartCrop { get; set; } = false;
        public bool UseGpu { get; set; } = true;
        public int ThreadCount { get; set; } = 4;
    }
} 