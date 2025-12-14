import sys
import os

# 1. Add the path containing the 'prsi' package to the Python path
# This step is often necessary when importing a package that is not installed globally
# and is located relative to your runner script.
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, current_dir)

# 2. Now you can import the client module and run the application
from prsi.main import run_client

if __name__ == "__main__":
    print("Starting Prší Client from external runner...")
    run_client()
