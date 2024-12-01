import os
import sys
import argparse
import logging
from typing import List
from image_optimizer.core.optimizer import ImageOptimizer
from image_optimizer.utils.file_handler import get_image_files, ensure_dir
from image_optimizer.cli.progress_bar import ProgressBar

def validate_quality(value: str) -> int:
    ivalue = int(value)
    if ivalue < 0 or ivalue > 100:
        raise argparse.ArgumentTypeError(f"Quality must be between 0 and 100, got {value}")
    return ivalue

def validate_max_size(value: str) -> int:
    ivalue = int(value)
    if ivalue < 1:
        raise argparse.ArgumentTypeError(f"Max size must be positive, got {value}")
    return ivalue

def setup_logging() -> logging.Logger:
    """Configure and return the logger instance."""
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        handlers=[
            logging.StreamHandler(sys.stdout),
            logging.FileHandler('image_optimizer.log')
        ]
    )
    return logging.getLogger(__name__)

def process_directory(input_dir: str, output_dir: str, optimizer: ImageOptimizer, 
                     output_format: str, logger: logging.Logger) -> None:
    """Process all images in the input directory."""
    image_files = get_image_files(input_dir)
    if not image_files:
        logger.warning(f"No image files found in {input_dir}")
        return

    progress_bar = ProgressBar(len(image_files), "Optimizing images")
    try:
        for input_path in image_files:
            try:
                relative_path = os.path.relpath(input_path, input_dir)
                output_path = os.path.join(output_dir, 
                    os.path.splitext(relative_path)[0] + '.' + output_format)
                ensure_dir(os.path.dirname(output_path))
                optimizer.optimize_image(input_path, output_path)
                progress_bar.update()
            except Exception as e:
                logger.error(f"Failed to optimize {input_path}: {str(e)}")
    finally:
        progress_bar.close()

def validate_threads(value: str) -> int:
    """Validate the number of threads."""
    ivalue = int(value)
    if ivalue < 1:
        raise argparse.ArgumentTypeError(f"Number of threads must be positive, got {value}")
    return ivalue

def main() -> None:
    parser = argparse.ArgumentParser(description="Advanced Image Optimizer for web use")
    parser.add_argument("input", help="Input image file or directory")
    parser.add_argument("output", help="Output image file or directory")
    parser.add_argument("--format", default="webp", choices=["webp", "jpeg", "png", "avif"], help="Output image format")
    parser.add_argument("--quality", type=validate_quality, default=85, help="Output image quality (0-100)")
    parser.add_argument("--max-size", type=validate_max_size, default=1920, help="Maximum image dimension")
    parser.add_argument("--compress", action="store_true", help="Apply additional compression")
    parser.add_argument("--enhance", action="store_true", help="Apply image enhancement")
    parser.add_argument("--watermark", help="Path to watermark image")
    parser.add_argument("--metadata", action="store_true", help="Preserve image metadata")
    parser.add_argument("--threads", type=validate_threads, default=1, help="Number of threads for parallel processing")

    args = parser.parse_args()

    logger = setup_logging()
    
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

    try:
        if not os.path.exists(args.input):
            raise FileNotFoundError(f"Input path does not exist: {args.input}")

        ensure_dir(args.output)

        if os.path.isdir(args.input):
            process_directory(args.input, args.output, optimizer, args.format, logger)
        else:
            optimizer.optimize_image(args.input, args.output)
            logger.info(f"Optimized: {args.input}")

    except Exception as e:
        logger.error(f"Error during optimization: {str(e)}")
        sys.exit(1)

if __name__ == "__main__":
    main()
