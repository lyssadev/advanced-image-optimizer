
import os
from pathlib import Path

def get_image_files(directory):
    image_extensions = ('.png', '.jpg', '.jpeg', '.gif', '.webp', '.tiff', '.bmp', '.heic')
    return [
        os.path.join(root, file)
        for root, _, files in os.walk(directory)
        for file in files
        if file.lower().endswith(image_extensions)
    ]

def ensure_dir(directory):
    Path(directory).mkdir(parents=True, exist_ok=True)

def get_output_path(input_path, output_dir, output_format):
    relative_path = os.path.relpath(input_path, start=os.path.dirname(input_path))
    output_path = os.path.join(output_dir, os.path.splitext(relative_path)[0] + f'.{output_format}')
    ensure_dir(os.path.dirname(output_path))
    return output_path
