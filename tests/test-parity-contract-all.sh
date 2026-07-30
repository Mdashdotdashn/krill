#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EMBEDDED_BUILD_DIR="${ROOT_DIR}/core/cpp/build"

cd "${ROOT_DIR}"

echo "[parity] JS contract gates"
node tests/test-parity-contract.js

if [[ ! -d "${EMBEDDED_BUILD_DIR}" ]]; then
  echo "[parity] ERROR: embedded build directory not found at ${EMBEDDED_BUILD_DIR}" >&2
  echo "[parity] Run ./core/cpp/prepare_build.sh first." >&2
  exit 1
fi

echo "[parity] Embedded build"
cmake --build "${EMBEDDED_BUILD_DIR}"

pushd "${EMBEDDED_BUILD_DIR}" >/dev/null

TESTS_EXE="./tests/Tests.exe.exe"

echo "[parity] Embedded AST parity"
"${TESTS_EXE}" "*AST parity*"

echo "[parity] Embedded parser"
"${TESTS_EXE}" "*Parser*"

echo "[parity] Embedded run cases"
"${TESTS_EXE}" "*Rendertree*"

popd >/dev/null

echo "[parity] PASS"
