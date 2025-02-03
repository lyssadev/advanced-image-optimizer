from setuptools import setup, find_packages

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup(
    name="advanced-image-optimizer",
    version="3.0.0",
    author="Advanced Image Optimizer Team",
    author_email="contact@advanced-image-optimizer.com",
    description="Professional image optimization and enhancement library",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/lyssadev/advanced-image-optimizer",
    packages=find_packages(),
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Topic :: Multimedia :: Graphics",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Operating System :: OS Independent",
    ],
    python_requires=">=3.8",
    install_requires=[
        "numpy>=1.19.0",
        "pillow>=9.0.0",
    ],
    package_data={
        "optimizer": [
            "*.dll",
            "*.so",
            "*.dylib",
        ],
    },
    extras_require={
        "gpu": ["torch>=2.0.0"],
        "ai": ["onnxruntime-gpu>=1.15.0"],
        "all": [
            "torch>=2.0.0",
            "onnxruntime-gpu>=1.15.0",
        ],
    },
) 