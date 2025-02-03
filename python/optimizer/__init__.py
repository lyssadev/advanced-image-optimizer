"""
Advanced Image Optimizer Pro - Python Bindings
===========================================

A high-performance image optimization and enhancement library with GPU acceleration
and AI-powered features.
"""

from .optimizer import (
    Optimizer,
    OptimizerError,
    OptimizerOptions,
    ImageInfo,
    VersionInfo,
    SystemInfo,
    SupportedFormat,
    ProcessingMode,
    CompressionLevel,
    AIModel,
)

__version__ = "3.0.0"
__author__ = "Advanced Image Optimizer Team"
__email__ = "contact@advanced-image-optimizer.com"
__description__ = "Professional image optimization and enhancement library"
__url__ = "https://github.com/lyssadev/advanced-image-optimizer"
__license__ = "MIT"

__all__ = [
    "Optimizer",
    "OptimizerError",
    "OptimizerOptions",
    "ImageInfo",
    "VersionInfo",
    "SystemInfo",
    "SupportedFormat",
    "ProcessingMode",
    "CompressionLevel",
    "AIModel",
] 