from __future__ import annotations

from pathlib import Path
import shutil
import sys
import tempfile
import zipfile


ROOT = Path(__file__).resolve().parents[1]
OUTPUT_DIR = ROOT / "out" / "RbxBuild"
LIBRARIES = {
    "Debug": ROOT / "out" / "build" / "windows-x64-debug" / "Debug" / "RbxLib.lib",
    "Release": ROOT / "out" / "build" / "windows-x64-release" / "Release" / "RbxLib.lib",
}


def main() -> int:
    required_paths = [*LIBRARIES.values(), ROOT / "LICENSE", ROOT / "include"]
    missing_paths = [path for path in required_paths if not path.exists()]
    if missing_paths:
        for path in missing_paths:
            print(f"Package.py: required path not found: {path}", file=sys.stderr)
        return 1

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    archive_path = OUTPUT_DIR / "RbxLib.zip"

    with tempfile.TemporaryDirectory(dir=OUTPUT_DIR) as temporary_dir:
        package_dir = Path(temporary_dir)
        for configuration, library in LIBRARIES.items():
            configuration_dir = package_dir / configuration
            configuration_dir.mkdir()
            shutil.copy2(library, configuration_dir / library.name)

        shutil.copy2(ROOT / "LICENSE", package_dir / "LICENSE")
        shutil.copytree(ROOT / "include", package_dir / "include")

        temporary_archive = package_dir / "RbxLib.zip"
        with zipfile.ZipFile(
            temporary_archive, "w", compression=zipfile.ZIP_DEFLATED
        ) as archive:
            for path in sorted(package_dir.rglob("*")):
                if path.is_file() and path != temporary_archive:
                    archive.write(path, path.relative_to(package_dir).as_posix())

        temporary_archive.replace(archive_path)

    print(f"Created {archive_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())