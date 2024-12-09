# Advanced Image Optimizer

This is an advanced image optimization tool that can process single images or batches of images, with various optimization options.

## Features

- Resize images while maintaining aspect ratio
- Convert between different image formats (WebP, JPEG, PNG, AVIF)
- Apply additional compression
- Enhance image quality
- Add watermarks
- Preserve image metadata
- Remove duplicate images
- Convert HEIC images to JPG
- Multi-threaded batch processing

## Installation

1. Clone this repository:
   ```
   git clone https://github.com/lyssadev/advanced-image-optimizer.git
   cd advanced-image-optimizer
   ```

2. Create a virtual environment and activate it:
   ```
   python -m venv venv
   source venv/bin/activate  # On Windows, use `venv\Scripts\activate`
   ```

3. Install the required packages:
   ```
   pip install -r requirements.txt
   ```

## Usage

To optimize a single image:

```
python -m image_optimizer.image_optimizer input_image.jpg output_image.webp --format webp --quality 85 --max-size 1920
```

To optimize a directory of images:

```
python -m image_optimizer.image_optimizer input_directory/ output_directory/ --format webp --quality 85 --max-size 1920 --compress --enhance --threads 4
```

### Options

- `--format`: Output image format (webp, jpeg, png, avif)
- `--quality`: Output image quality (0-100)
- `--max-size`: Maximum image dimension
- `--compress`: Apply additional compression
- `--enhance`: Apply image enhancement
- `--watermark`: Path to watermark image
- `--metadata`: Preserve image metadata
- `--threads`: Number of threads for parallel processing

## Updating Packages

To update the project's packages to their latest versions, run the following command:

```
bash update_packages.sh
```

This script will create a virtual environment if it doesn't exist, activate it, upgrade pip, update all packages to their latest versions, and update the requirements.txt file.

## Running Tests

To run the test suite, execute the following command:

```
python -m unittest tests.py
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
