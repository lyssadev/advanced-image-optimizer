import os
import shutil
import unittest
from PIL import Image
from image_optimizer.core.optimizer import ImageOptimizer
from image_optimizer.utils.file_handler import get_image_files, ensure_dir

class TestImageOptimizer(unittest.TestCase):
    def setUp(self):
        self.test_dir = 'test_images'
        self.output_dir = 'test_output'
        ensure_dir(self.test_dir)
        ensure_dir(self.output_dir)
        
        img = Image.new('RGB', (100, 100), color='red')
        img.save(os.path.join(self.test_dir, 'test_image.jpg'))

    def tearDown(self):
        shutil.rmtree(self.test_dir)
        shutil.rmtree(self.output_dir)

    def test_optimize_image(self):
        optimizer = ImageOptimizer(output_format='webp', quality=85, max_size=50)
        input_path = os.path.join(self.test_dir, 'test_image.jpg')
        output_path = os.path.join(self.output_dir, 'test_image.webp')
        
        optimizer.optimize_image(input_path, output_path)
        
        self.assertTrue(os.path.exists(output_path))
        with Image.open(output_path) as img:
            self.assertEqual(img.format, 'WEBP')
            self.assertLessEqual(max(img.size), 50)

    def test_batch_optimize(self):
        optimizer = ImageOptimizer(output_format='png', quality=85, max_size=50, threads=2)
        input_files = get_image_files(self.test_dir)
        
        optimizer.batch_optimize(input_files, self.output_dir)
        
        output_files = get_image_files(self.output_dir)
        self.assertEqual(len(input_files), len(output_files))
        for output_file in output_files:
            self.assertTrue(output_file.endswith('.png'))

if __name__ == '__main__':
    unittest.main()
