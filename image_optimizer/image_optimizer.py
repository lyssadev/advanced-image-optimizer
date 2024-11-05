import os
import argparse
from image_optimizer.core.optimizer import ImageOptimizer
from image_optimizer.utils.file_handler import get_image_files
from image_optimizer.cli.progress_bar import ProgressBar

def main():
    parser = argparse.ArgumentParser(description="Advanced Image Optimizer for web use")
    parser.add_argument("input", help="Input image file or directory")
    parser.add_argument("output", help="Output image file or directory")
    parser.add_argument("--format", default="webp", choices=["webp", "jpeg", "png", "avif"], help="Output image format")
    parser.add_argument("--quality", type=int, default=85, help="Output image quality (0-100)")
    parser.add_argument("--max-size", type=int, default=1920, help="Maximum image dimension")
    parser.add_argument("--compress", action="store_true", help="Apply additional compression")
    parser.add_argument("--enhance", action="store_true", help="Apply image enhancement")
    parser.add_argument("--watermark", help="Path to watermark image")
    parser.add_argument("--metadata", action="store_true", help="Preserve image metadata")
    parser.add_argument("--threads", type=int, default=1, help="Number of threads for parallel processing")

    args = parser.parse_args()

    optimizer = ImageOptimizer(
        output_format=args.format,
        quality=args.quality,
        max_size=args.max_size,
        compress=args.compress,
        enhance=args.enhance,
        watermark_path=args.watermark,
        preserve_metadata=args.metadata,
        threads=args.threads
    )

    if os.path.isdir(args.input):
        image_files = get_image_files(args.input)
        progress_bar = ProgressBar(len(image_files), "Optimizing images")
        for input_path in image_files:
            relative_path = os.path.relpath(input_path, args.input)
            output_path = os.path.join(args.output, os.path.splitext(relative_path)[0] + '.' + args.format)
            os.makedirs(os.path.dirname(output_path), exist_ok=True)
            optimizer.optimize_image(input_path, output_path)
            progress_bar.update()
    else:
        optimizer.optimize_image(args.input, args.output)
        print(f"Optimized: {args.input}")

if __name__ == "__main__":
    main()
