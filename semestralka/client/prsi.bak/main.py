import os
from . import config
from .client import GameClient

# Ensure assets directory exists for the image generator logic
if not os.path.exists(config.ASSETS_DIR):
    os.makedirs(config.ASSETS_DIR)

def run_client():
    client = GameClient()
    client.run()

if __name__ == "__main__":
    run_client()
