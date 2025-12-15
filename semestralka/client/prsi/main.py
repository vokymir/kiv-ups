
from prsi.client import Client


def run_client() -> None:
    client: Client = Client()
    client.run()

if __name__ == "__main__":
    run_client()
