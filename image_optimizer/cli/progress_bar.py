import sys
from tqdm import tqdm

class ProgressBar:
    def __init__(self, total, description="Processing"):
        self.pbar = tqdm(total=total, desc=description, unit="image", file=sys.stdout)

    def update(self, n=1):
        self.pbar.update(n)

    def close(self):
        self.pbar.close()
