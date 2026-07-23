#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EMBEDDED_BUILD_DIR="${ROOT_DIR}/embedded/build"

cd "${ROOT_DIR}"

echo "[parity] JS contract gates"
node tests/test-parity-contract.js

if [[ ! -d "${EMBEDDED_BUILD_DIR}" ]]; then
  echo "[parity] ERROR: embedded build directory not found at ${EMBEDDED_BUILD_DIR}" >&2
  echo "[parity] Run ./embedded/prepare_build.sh first." >&2
  exit 1
fi

echo "[parity] Embedded build"
cmake --build "${EMBEDDED_BUILD_DIR}"

pushd "${EMBEDDED_BUILD_DIR}" >/dev/null

echo "[parity] Embedded AST parity"
./tests/Tests.exe "*AST parity*"

echo "[parity] Embedded parser"
./tests/Tests.exe "*Parser*"

echo "[parity] Embedded run cases"
(cd tests && ./Tests.exe "*Rendertree*")

popd >/dev/null

echo "[parity] PASS"
