"""Delete all extracted and generated files, keeping data/ and .env."""
import argparse
import shutil

from zbtools import paths


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-n", "--dry-run", action="store_true",
                        help="list what would be deleted without deleting it")
    args = parser.parse_args()

    targets = [p for p in paths.GENERATED if p.exists()]
    targets += paths.REPO_ROOT.glob("src/**/__pycache__")
    for p in targets:
        print(f"{'would remove' if args.dry_run else 'removing'} {p.relative_to(paths.REPO_ROOT)}")
        if not args.dry_run:
            shutil.rmtree(p) if p.is_dir() else p.unlink()


if __name__ == "__main__":
    main()
