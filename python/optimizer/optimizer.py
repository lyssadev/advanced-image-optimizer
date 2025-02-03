"""Core implementation of the Python bindings for the Advanced Image Optimizer."""

import os
import sys
import enum
import ctypes
import pathlib
from typing import List, Optional, Tuple, Union, Callable
from dataclasses import dataclass
from contextlib import contextmanager

# Load the optimizer library
if sys.platform == "win32":
    _lib_name = "optimizer.dll"
elif sys.platform == "darwin":
    _lib_name = "liboptimizer.dylib"
else:
    _lib_name = "liboptimizer.so"

_lib_path = os.path.join(os.path.dirname(__file__), _lib_name)
_lib = ctypes.CDLL(_lib_path)

# Define enums
class ProcessingMode(enum.Enum):
    """Image processing modes."""
    BALANCED = 0
    QUALITY = 1
    SPEED = 2
    EXTREME = 3

class CompressionLevel(enum.Enum):
    """Compression levels."""
    LOSSLESS = 0
    MINIMAL = 1
    BALANCED = 2
    AGGRESSIVE = 3
    MAXIMUM = 4

class AIModel(enum.Enum):
    """AI model types."""
    NONE = 0
    FAST = 1
    BALANCED = 2
    QUALITY = 3

class SupportedFormat(enum.Enum):
    """Supported image formats."""
    JPEG = "jpeg"
    PNG = "png"
    WEBP = "webp"
    AVIF = "avif"
    HEIC = "heic"

# Define data classes
@dataclass
class VersionInfo:
    """Version information."""
    major: int
    minor: int
    patch: int
    build_date: str
    build_hash: str
    platform: str
    has_gpu_support: bool
    has_ai_support: bool

@dataclass
class SystemInfo:
    """System information."""
    cpu_cores: int
    total_memory: int
    available_memory: int
    os_name: str
    os_version: str
    cpu_name: str
    has_avx: bool
    has_avx2: bool
    has_avx512: bool
    has_cuda: bool

@dataclass
class ImageInfo:
    """Image information."""
    width: int
    height: int
    channels: int
    bit_depth: int
    format: SupportedFormat
    has_alpha: bool
    has_metadata: bool
    file_size: int

@dataclass
class OptimizerOptions:
    """Optimizer configuration options."""
    output_format: SupportedFormat = SupportedFormat.WEBP
    quality: int = 85
    processing_mode: ProcessingMode = ProcessingMode.BALANCED
    compression_level: CompressionLevel = CompressionLevel.BALANCED
    max_width: int = 0
    max_height: int = 0
    preserve_metadata: bool = True
    strip_color_profile: bool = False
    auto_enhance: bool = False
    smart_compress: bool = True
    face_enhance: bool = False
    super_resolution: bool = False
    smart_crop: bool = False
    ai_model: AIModel = AIModel.NONE
    use_gpu: bool = True
    thread_count: int = 0

class OptimizerError(Exception):
    """Exception raised for optimizer errors."""
    pass

# Define callback types
ProgressCallback = ctypes.CFUNCTYPE(None, ctypes.c_float, ctypes.c_void_p)
LogCallback = ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_void_p)
ErrorCallback = ctypes.CFUNCTYPE(None, ctypes.c_int, ctypes.c_char_p, ctypes.c_void_p)

class Optimizer:
    """Main optimizer class for image processing."""

    def __init__(self, enable_gpu: bool = True, enable_ai: bool = True, thread_count: int = 0):
        """Initialize the optimizer.
        
        Args:
            enable_gpu: Enable GPU acceleration if available
            enable_ai: Enable AI features if available
            thread_count: Number of threads to use (0 for auto)
        """
        self._handle = None
        self._progress_cb = None
        self._log_cb = None
        self._error_cb = None
        
        # Initialize the optimizer
        init_options = {
            "enable_gpu": enable_gpu,
            "enable_ai": enable_ai,
            "thread_count": thread_count,
        }
        
        if not _lib.optimizer_init(ctypes.byref(ctypes.c_void_p(init_options))):
            raise OptimizerError("Failed to initialize optimizer")
        
        # Register cleanup
        self._initialized = True

    def __del__(self):
        """Clean up resources."""
        if hasattr(self, '_initialized') and self._initialized:
            _lib.optimizer_cleanup()

    @staticmethod
    def version_info() -> VersionInfo:
        """Get version information."""
        info = ctypes.c_void_p()
        if not _lib.optimizer_get_version_info(ctypes.byref(info)):
            raise OptimizerError("Failed to get version info")
        
        # Convert C struct to Python dataclass
        return VersionInfo(
            major=info.major,
            minor=info.minor,
            patch=info.patch,
            build_date=info.build_date.decode(),
            build_hash=info.build_hash.decode(),
            platform=info.platform.decode(),
            has_gpu_support=info.has_gpu_support,
            has_ai_support=info.has_ai_support
        )

    @staticmethod
    def system_info() -> SystemInfo:
        """Get system information."""
        info = ctypes.c_void_p()
        if not _lib.optimizer_get_system_info(ctypes.byref(info)):
            raise OptimizerError("Failed to get system info")
        
        return SystemInfo(
            cpu_cores=info.cpu_cores,
            total_memory=info.total_memory,
            available_memory=info.available_memory,
            os_name=info.os_name.decode(),
            os_version=info.os_version.decode(),
            cpu_name=info.cpu_name.decode(),
            has_avx=info.has_avx,
            has_avx2=info.has_avx2,
            has_avx512=info.has_avx512,
            has_cuda=info.has_cuda
        )

    def get_image_info(self, path: Union[str, pathlib.Path]) -> ImageInfo:
        """Get information about an image file.
        
        Args:
            path: Path to the image file
            
        Returns:
            ImageInfo object containing image details
        """
        info = ctypes.c_void_p()
        path_str = str(path).encode()
        if not _lib.optimizer_get_image_info(path_str, ctypes.byref(info)):
            raise OptimizerError("Failed to get image info")
        
        return ImageInfo(
            width=info.width,
            height=info.height,
            channels=info.channels,
            bit_depth=info.bit_depth,
            format=SupportedFormat(info.format.decode()),
            has_alpha=info.has_alpha,
            has_metadata=info.has_metadata,
            file_size=info.file_size
        )

    def process_file(
        self,
        input_path: Union[str, pathlib.Path],
        output_path: Union[str, pathlib.Path],
        options: Optional[OptimizerOptions] = None,
        progress_callback: Optional[Callable[[float], None]] = None
    ) -> None:
        """Process an image file.
        
        Args:
            input_path: Path to input image
            output_path: Path to save optimized image
            options: Processing options
            progress_callback: Optional callback for progress updates
        """
        if options is None:
            options = OptimizerOptions()

        # Convert paths to strings
        input_str = str(input_path).encode()
        output_str = str(output_path).encode()

        # Set up progress callback if provided
        if progress_callback:
            def callback_wrapper(progress: float, _: ctypes.c_void_p) -> None:
                progress_callback(progress)
            
            self._progress_cb = ProgressCallback(callback_wrapper)
        else:
            self._progress_cb = None

        # Process the file
        if not _lib.optimizer_process_file(
            input_str,
            output_str,
            ctypes.byref(ctypes.c_void_p(options)),
            self._progress_cb,
            None
        ):
            raise OptimizerError("Failed to process file")

    def process_buffer(
        self,
        input_buffer: bytes,
        options: Optional[OptimizerOptions] = None,
        progress_callback: Optional[Callable[[float], None]] = None
    ) -> bytes:
        """Process an image in memory.
        
        Args:
            input_buffer: Input image data
            options: Processing options
            progress_callback: Optional callback for progress updates
            
        Returns:
            Optimized image data as bytes
        """
        if options is None:
            options = OptimizerOptions()

        # Set up progress callback if provided
        if progress_callback:
            def callback_wrapper(progress: float, _: ctypes.c_void_p) -> None:
                progress_callback(progress)
            
            self._progress_cb = ProgressCallback(callback_wrapper)
        else:
            self._progress_cb = None

        # Create input buffer
        input_size = len(input_buffer)
        input_data = (ctypes.c_char * input_size)(*input_buffer)

        # Create output buffer
        output_size = ctypes.c_size_t()
        output_data = ctypes.POINTER(ctypes.c_char)()

        # Process the buffer
        if not _lib.optimizer_process_buffer(
            input_data,
            input_size,
            ctypes.byref(output_data),
            ctypes.byref(output_size),
            ctypes.byref(ctypes.c_void_p(options)),
            self._progress_cb,
            None
        ):
            raise OptimizerError("Failed to process buffer")

        try:
            # Convert output to bytes
            result = bytes(output_data[:output_size.value])
            return result
        finally:
            # Free the output buffer
            _lib.optimizer_free_buffer(output_data)

    @contextmanager
    def batch_process(self) -> None:
        """Context manager for batch processing multiple images.
        
        This allows reusing resources across multiple operations.
        """
        try:
            if not _lib.optimizer_batch_begin():
                raise OptimizerError("Failed to begin batch processing")
            yield
        finally:
            _lib.optimizer_batch_end()

    @property
    def supported_formats(self) -> List[SupportedFormat]:
        """Get list of supported image formats."""
        count = ctypes.c_int()
        formats_ptr = _lib.optimizer_get_supported_formats(ctypes.byref(count))
        
        try:
            formats = []
            for i in range(count.value):
                format_str = ctypes.string_at(formats_ptr[i]).decode()
                formats.append(SupportedFormat(format_str))
            return formats
        finally:
            _lib.optimizer_free_string_array(formats_ptr, count)

    @property
    def has_gpu_support(self) -> bool:
        """Check if GPU acceleration is available."""
        return bool(_lib.optimizer_is_gpu_available())

    @property
    def max_thread_count(self) -> int:
        """Get maximum number of threads supported."""
        return _lib.optimizer_get_max_thread_count() 