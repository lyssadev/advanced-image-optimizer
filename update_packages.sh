#!/bin/bash

# Check if virtual environment exists
if [ ! -d "venv" ]; then
    echo "Virtual environment not found. Creating one..."
    python3 -m venv venv
fi

# Activate virtual environment
source venv/bin/activate

# Upgrade pip
pip install --upgrade pip

# Update packages
pip list --outdated | cut -d ' ' -f1 | xargs -n1 pip install -U

# Update requirements.txt
pip freeze > requirements.txt

echo "Packages updated successfully!"
