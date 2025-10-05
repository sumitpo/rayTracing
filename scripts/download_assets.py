# tests/download_assets.py
import os
import urllib.request

ASSETS = {"reference.png": "https://example.com/reference.png"}  # replace with real URL


def main():
    os.makedirs("tests/assets", exist_ok=True)
    for name, url in ASSETS.items():
        path = os.path.join("tests/assets", name)
        if not os.path.exists(path):
            print(f"Downloading {name}...")
            urllib.request.urlretrieve(url, path)
        else:
            print(f"{name} already exists.")


if __name__ == "__main__":
    main()
