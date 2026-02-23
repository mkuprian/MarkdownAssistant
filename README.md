# mdeditor

A Qt-based Markdown editor built with modern CMake and C++17.

## Features

- Modern CMake build system (3.21+)
- C++17 standard
- GoogleTest for unit testing
- Optional Qt WebEngine support for preview rendering
- CI/CD with GitHub Actions

## Project Structure

```
mdeditor/
├── CMakeLists.txt           # Root CMake configuration
├── cmake/                    # Custom CMake modules
│   └── FindThirdParty.cmake
├── src/
│   ├── CMakeLists.txt
│   ├── mdcore_stub/          # Core Markdown library (stub)
│   │   ├── CMakeLists.txt
│   │   └── mdcore_stub.cpp
│   ├── mdapp_stub/           # Console application (stub)
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   ├── gapbuffer/            # Gap Buffer text model
│   │   ├── CMakeLists.txt
│   │   ├── gap_buffer.h
│   │   └── gap_buffer.cpp
│   └── cli/                  # CLI demo for GapBuffer
│       ├── CMakeLists.txt
│       └── main.cpp
├── samples/                  # Sample files for demos
│   └── sample.md
├── tests/                    # Unit tests
│   ├── CMakeLists.txt
│   ├── test_stub.cpp
│   └── gapbuffer_tests.cpp
├── tools/                    # Development tools
│   └── CMakeLists.txt
└── .github/workflows/        # CI configuration
    └── ci.yml
```

## Prerequisites

- CMake 3.21 or higher
- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- (Optional) Qt 6 for GUI components
- (Optional) Qt WebEngine for preview rendering

## Build Instructions

### Configure

```bash
cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug
```

### Build

```bash
cmake --build build --parallel
```

### Run Tests

```bash
ctest --test-dir build
```

Or with verbose output:

```bash
ctest --test-dir build --output-on-failure
```

## Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTING` | `ON` | Build unit tests |
| `MD_USE_WEBENGINE` | `OFF` | Enable Qt WebEngine for preview |

### Example with options

```bash
cmake -S . -B build \
    -D CMAKE_BUILD_TYPE=Release \
    -D BUILD_TESTING=ON \
    -D MD_USE_WEBENGINE=OFF
```

## Development

### Running the GapBuffer CLI Demo

The `mdcli` tool demonstrates the GapBuffer text model:

```bash
# Linux/macOS
./build/src/cli/mdcli samples/sample.md

# Windows (run from project root)
.\build\src\cli\Debug\mdcli.exe samples\sample.md
```

The demo will:
1. Load the sample markdown file
2. Perform insert/delete operations
3. Display original and modified text
4. Show line/offset mapping
5. Print patch history

### Running the stub application

After building:

```bash
# Linux/macOS
./build/src/mdapp_stub/mdapp_stub

# Windows
.\build\src\mdapp_stub\Debug\mdapp_stub.exe
```

### GapBuffer API

The `gapbuffer` library provides an efficient text editing model:

```cpp
#include "gap_buffer.h"

mdeditor::GapBuffer buffer;
buffer.loadFromString("Hello, World!");
buffer.insert(7, "Beautiful ");   // "Hello, Beautiful World!"
buffer.erase(5, 1);               // Remove comma

std::string text = buffer.getText();
size_t line = buffer.lineFromOffset(10);
size_t offset = buffer.offsetFromLine(0, 5);

auto patches = buffer.flushPatches();  // Get edit history
```

**Note:** All offsets are in UTF-8 bytes, not characters. See `gap_buffer.h` for details.

### Adding new source files

1. Add source files to the appropriate subdirectory
2. Update the corresponding `CMakeLists.txt` using `target_sources()`

## License

[Add your license here]
