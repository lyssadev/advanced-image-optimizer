import pytest
import asyncio
from pathlib import Path
import shutil
import tempfile
from PIL import Image
import numpy as np
from image_optimizer.config import OptimizerConfig
from image_optimizer.core.optimizer import ImageOptimizer

@pytest.fixture
def temp_dir():
    """Create a temporary directory for test files."""
    temp_dir = tempfile.mkdtemp()
    yield Path(temp_dir)
    shutil.rmtree(temp_dir)

@pytest.fixture
def test_image(temp_dir):
    """Create a test image for optimization tests."""
    image_path = temp_dir / "test.jpg"
    # Create a simple test image
    img = Image.fromarray(np.random.randint(0, 255, (100, 100, 3), dtype=np.uint8))
    img.save(image_path, "JPEG", quality=95)
    return image_path

@pytest.fixture
def test_config(temp_dir, test_image):
    """Create a test configuration."""
    return OptimizerConfig(
        input_path=test_image.parent,
        output_path=temp_dir / "output",
        output_format="webp",
        quality=85,
        max_size=1920,
        compress=True,
        enhance=True,
        watermark_path=None,
        preserve_metadata=False,
        threads=1,
        recursive=True,
        skip_existing=False,
        smart_compress=True,
        keep_originals=False
    )

@pytest.fixture
def optimizer(test_config):
    """Create an image optimizer instance."""
    return ImageOptimizer(test_config)

@pytest.mark.asyncio
async def test_optimize_single_image(optimizer, test_image):
    """Test optimizing a single image."""
    result = await optimizer.optimize_image(test_image)
    
    assert result["success"]
    assert result["input_path"] == test_image
    assert result["output_path"].exists()
    assert result["original_size"] > 0
    assert result["optimized_size"] > 0

def test_resize_image(optimizer):
    """Test image resizing functionality."""
    # Create a large test image
    img = Image.fromarray(np.random.randint(0, 255, (2000, 2000, 3), dtype=np.uint8))
    
    # Test resizing
    resized = optimizer._resize_image(img)
    assert max(resized.size) == optimizer.config.max_size
    
    # Check aspect ratio is maintained
    original_ratio = img.size[0] / img.size[1]
    resized_ratio = resized.size[0] / resized.size[1]
    assert abs(original_ratio - resized_ratio) < 0.01

def test_enhance_image(optimizer):
    """Test image enhancement functionality."""
    # Create a test image
    img = Image.fromarray(np.random.randint(0, 255, (100, 100, 3), dtype=np.uint8))
    
    # Test enhancement
    enhanced = optimizer._enhance_image(img)
    assert enhanced.size == img.size
    assert isinstance(enhanced, Image.Image)

@pytest.mark.asyncio
async def test_batch_optimization(optimizer, temp_dir):
    """Test batch optimization of multiple images."""
    # Create multiple test images
    image_files = []
    for i in range(3):
        img_path = temp_dir / f"test_{i}.jpg"
        img = Image.fromarray(np.random.randint(0, 255, (100, 100, 3), dtype=np.uint8))
        img.save(img_path, "JPEG", quality=95)
        image_files.append(img_path)
    
    # Run batch optimization
    results = await optimizer.optimize_images(image_files)
    
    assert results["total"] == len(image_files)
    assert results["successful"] == len(image_files)
    assert results["failed"] == 0
    assert results["total_size_before"] > 0
    assert results["total_size_after"] > 0

def test_smart_compress(optimizer):
    """Test smart compression functionality."""
    # Create test images with different complexities
    simple_img = Image.fromarray(np.ones((100, 100, 3), dtype=np.uint8) * 128)
    complex_img = Image.fromarray(np.random.randint(0, 255, (100, 100, 3), dtype=np.uint8))
    
    # Test compression on simple image
    original_quality = optimizer.config.quality
    optimizer._smart_compress(simple_img)
    assert optimizer.config.quality <= original_quality
    
    # Reset quality
    optimizer.config.quality = original_quality
    
    # Test compression on complex image
    optimizer._smart_compress(complex_img)
    assert optimizer.config.quality >= 85

def test_output_path_generation(optimizer, test_image):
    """Test output path generation."""
    output_path = optimizer._get_output_path(test_image)
    
    assert output_path.suffix == f".{optimizer.config.output_format}"
    assert output_path.parent == optimizer.config.output_path
    
@pytest.mark.asyncio
async def test_error_handling(optimizer, temp_dir):
    """Test error handling for invalid files."""
    # Create an invalid "image" file
    invalid_file = temp_dir / "invalid.jpg"
    with open(invalid_file, "w") as f:
        f.write("This is not an image")
    
    result = await optimizer.optimize_image(invalid_file)
    
    assert not result["success"]
    assert "error" in result
    assert str(invalid_file) in str(result["error"])

def test_supported_formats(optimizer):
    """Test supported format validation."""
    assert "webp" in optimizer.supported_formats
    assert "jpeg" in optimizer.supported_formats
    assert "png" in optimizer.supported_formats
    assert "avif" in optimizer.supported_formats

@pytest.mark.asyncio
async def test_skip_existing(optimizer, test_image):
    """Test skip existing functionality."""
    # First optimization
    result1 = await optimizer.optimize_image(test_image)
    assert result1["success"]
    
    # Enable skip_existing
    optimizer.config.skip_existing = True
    
    # Second optimization should be skipped
    result2 = await optimizer.optimize_image(test_image)
    assert not result2["success"]
    assert "already exists" in str(result2.get("error", "")) 