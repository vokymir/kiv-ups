import sys
import os

# add the prsi path to python path
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, current_dir)

from prsi.main import run_client

if __name__ == "__main__":
    print("Starting Prší Client")
    run_client()
