#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

BUILD_DIR="$HOME/build/hd-linux-release"
EXE="$BUILD_DIR/HausdorffDistanceOptimized/HausdorffDistanceOptimized"

cmake --build "$BUILD_DIR"

"$EXE" \
  --folder "$PROJECT_ROOT/../TestImages/WalkingSilhouette" \
  --folder "$PROJECT_ROOT/../TestImages/snowflake" \
  --batches 5 \
  --algorithms contour,shuffle,bruteforce