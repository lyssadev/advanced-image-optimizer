import os
from PIL import Image, ImageEnhance, ImageFilter
from pillow_heif import register_heif_opener
from concurrent.futures import ThreadPoolExecutor
import piexif
import imagehash
from wand.image import Image as WandImage
import cv2
import numpy as np

register_heif_opener()

class ImageOptimizer:
    def __init__(self, output_format='webp', quality=85, max_size=1920, compress=False, enhance=False, watermark_path=None, preserve_metadata=False, threads=1):
        self.output_format = output_format
        self.quality = quality
        self.max_size = max_size
        self.compress = compress
        self.enhance = enhance
        self.watermark_path = watermark_path
        self.preserve_metadata = preserve_metadata
        self.threads = threads

    def optimize_image(self, input_path, output_path):
        with Image.open(input_path) as img:
            # Resize image if it's larger than max_size
            if max(img.size) > self.max_size:
                img.thumbnail((self.max_size, self.max_size), Image.LANCZOS)

            if self.enhance:
                img = self._enhance_image(img)

            if self.watermark_path:
                img = self._add_watermark(img)

            if self.compress:
                img = self._compress_image(img)

            # Preserve metadata if requested
            exif_bytes = piexif.dump(piexif.load(input_path)) if self.preserve_metadata else None

            # Save the optimized image
            save_params = {
                'format': self.output_format,
                'quality': self.quality,
                'optimize': True
            }
            if exif_bytes:
                save_params['exif'] = exif_bytes
            
            img.save(output_path, **save_params)

    def _enhance_image(self, img):
        enhancer = ImageEnhance.Contrast(img)
        img = enhancer.enhance(1.2)
        enhancer = ImageEnhance.Sharpness(img)
        img = enhancer.enhance(1.1)
        return img

    def _add_watermark(self, img):
        watermark = Image.open(self.watermark_path).convert("RGBA")
        watermark.thumbnail((img.width // 4, img.height // 4), Image.LANCZOS)
        position = (img.width - watermark.width, img.height - watermark.height)
        img = img.convert("RGBA")
        img.paste(watermark, position, watermark)
        return img.convert("RGB")

    def _compress_image(self, img):
        img_array = np.array(img)
        encoded = cv2.imencode(f'.{self.output_format}', img_array, [int(cv2.IMWRITE_JPEG_QUALITY), self.quality])[1]
        decoded = cv2.imdecode(encoded, cv2.IMREAD_UNCHANGED)
        return Image.fromarray(decoded)

    def batch_optimize(self, input_files, output_dir):
        with ThreadPoolExecutor(max_workers=self.threads) as executor:
            futures = []
            for input_path in input_files:
                output_path = os.path.join(output_dir, os.path.splitext(os.path.basename(input_path))[0] + f'.{self.output_format}')
                futures.append(executor.submit(self.optimize_image, input_path, output_path))
            for future in futures:
                future.result()

    def remove_duplicates(self, input_dir):
        hash_dict = {}
        for filename in os.listdir(input_dir):
            if filename.lower().endswith(('.png', '.jpg', '.jpeg', '.gif', '.webp')):
                file_path = os.path.join(input_dir, filename)
                with Image.open(file_path) as img:
                    img_hash = str(imagehash.average_hash(img))
                    if img_hash in hash_dict:
                        os.remove(file_path)
                        print(f"Removed duplicate: {filename}")
                    else:
                        hash_dict[img_hash] = file_path

    def convert_heic_to_jpg(self, input_path, output_path):
        with WandImage(filename=input_path) as img:
            img.format = 'jpg'
            img.save(filename=output_path)
