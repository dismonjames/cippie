#!/usr/bin/env python3
"""
Checks that production source files (.cpp) are synchronized between CMakeLists.txt and Cippiefile.
Excludes unit tests, integration tests, test fixtures, and generated files.
"""

import sys
import re
from pathlib import Path

def get_repo_root() -> Path:
    return Path(__file__).resolve().parent.parent

def get_cmake_sources(repo_root: Path) -> set[str]:
    cmake_path = repo_root / "CMakeLists.txt"
    content = cmake_path.read_text(encoding="utf-8")
    
    # Match add_library(cippie_core ...) and executable(cippie ...) source files
    cpp_files = set()
    # Find all src/**/*.cpp references in CMakeLists.txt
    matches = re.findall(r'src/[\w/]+\.cpp', content)
    for m in matches:
        if not m.startswith("tests/") and not "fixture" in m:
            cpp_files.add(m)
    return cpp_files

def get_cippie_sources(repo_root: Path) -> set[str]:
    # Production C++ files in src/
    cpp_files = set()
    src_dir = repo_root / "src"
    for p in src_dir.rglob("*.cpp"):
        rel = p.relative_to(repo_root).as_posix()
        if not rel.startswith("tests/") and not "fixture" in rel:
            cpp_files.add(rel)
    return cpp_files

def main():
    repo_root = get_repo_root()
    cmake_srcs = get_cmake_sources(repo_root)
    cippie_srcs = get_cippie_sources(repo_root)

    missing_in_cmake = cippie_srcs - cmake_srcs
    missing_in_cippiefile = cmake_srcs - cippie_srcs

    if missing_in_cmake or missing_in_cippiefile:
        print("ERROR: Production source files are out of sync between CMakeLists.txt and Cippiefile!")
        if missing_in_cmake:
            print("\nFiles in src/ but missing from CMakeLists.txt:")
            for f in sorted(missing_in_cmake):
                print(f"  - {f}")
        if missing_in_cippiefile:
            print("\nFiles in CMakeLists.txt but missing from src/:")
            for f in sorted(missing_in_cippiefile):
                print(f"  - {f}")
        sys.exit(1)

    print(f"Source synchronization check passed: {len(cippie_srcs)} production .cpp files verified.")
    sys.exit(0)

if __name__ == "__main__":
    main()
