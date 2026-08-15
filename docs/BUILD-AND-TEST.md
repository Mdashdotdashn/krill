# Build And Test

## JavaScript

Install dependencies and run the full JS suite from the repository root:

```bash
npm install
node test.js
```

The package script is also available:

```bash
npm test
```

## C++ On Windows

The verified Git Bash workflow uses the Visual Studio-backed CMake generator:

```bash
mkdir -p core/cpp/build
cd core/cpp/build
cmake ..
cmake --build .
./tests/Debug/Tests.exe.exe
```

The multi-configuration generator places the test executable under `tests/Debug/`. A flat `./tests/Tests.exe.exe` path may not exist.

To rebuild from the repository root:

```bash
cmake --build core/cpp/build
./core/cpp/build/tests/Debug/Tests.exe.exe
```

## Parity

Run the cross-language validation workflow from the repository root:

```bash
npm run test-parity-contract-all
```

It runs the JS contract gates, builds the C++ tests, checks AST/parser parity, and runs the shared render cases.

The JS-only parity gate is:

```bash
npm run test-parity-contract-js
```

## AST Snapshots

When a parser change intentionally changes the normalized AST:

```bash
npm run update-ast-cases
```

Review the generated snapshot together with the parser and grammar changes.

## Fixture Ownership

- `core/test-cases.json` contains user-facing pattern behavior.
- `core/test-cases-runner.json` contains harness normalization and schema cases.
- `core/test-cases-ast.json` contains generated AST parity snapshots.

## Troubleshooting

If C++ tests cannot be found, check the multi-configuration path:

```bash
./core/cpp/build/tests/Debug/Tests.exe.exe
```

If the build directory does not exist, configure it first with `cmake ..` from `core/cpp/build`.
