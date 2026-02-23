# mdeditor

A Qt-based Markdown editor built with modern CMake and C++17.

## Features

- Modern CMake build system (3.21+)
- C++17 standard
- GoogleTest for unit testing
- Pluggable Markdown parser with fallback renderer
- Optional cmark integration for full CommonMark compliance
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
│   ├── markdown/             # Markdown parser library
│   │   ├── CMakeLists.txt
│   │   ├── IMarkdownParser.h
│   │   ├── fallback_renderer.h/cpp
│   │   └── cmark_adapter.h/cpp
│   ├── cli/                  # CLI tools
│   │   ├── CMakeLists.txt
│   │   ├── main.cpp          # mdcli - GapBuffer demo
│   │   └── preview.cpp       # mdpreview - HTML preview
│   └── mdapp/                # Qt6 QML Application
│       ├── CMakeLists.txt
│       ├── DocumentController.h/cpp
│       ├── main.cpp
│       └── qml/Main.qml
├── samples/                  # Sample files for demos
│   └── sample.md
├── tests/                    # Unit tests
│   ├── CMakeLists.txt
│   ├── test_stub.cpp
│   ├── gapbuffer_tests.cpp
│   ├── markdown_tests.cpp
│   └── documentcontroller_tests.cpp
├── tools/                    # Development tools
│   └── CMakeLists.txt
└── .github/workflows/        # CI configuration
    └── ci.yml
```

## Prerequisites

- CMake 3.21 or higher
- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- (Optional) Qt 6 for GUI application (mdapp)
- (Optional) Qt WebEngine for HTML preview rendering

### Qt6 Installation

The `mdapp` QML application requires Qt6. If Qt6 is not found, the application will be skipped.

**Windows (Qt Online Installer):**
1. Download Qt Online Installer from https://www.qt.io/download
2. Install Qt 6.x with components: Qt Quick, Qt QML, Qt Quick Controls
3. Set CMAKE_PREFIX_PATH when configuring:
   ```bash
   cmake -S . -B build -D CMAKE_PREFIX_PATH=C:/Qt/6.x.x/msvc2022_64
   ```

**Linux (Ubuntu/Debian):**
```bash
sudo apt install qt6-base-dev qt6-declarative-dev qt6-quickcontrols2-dev
```

**macOS (Homebrew):**
```bash
brew install qt@6
cmake -S . -B build -D CMAKE_PREFIX_PATH="$(brew --prefix qt@6)"
```

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
| `MD_USE_CMARK` | `OFF` | Use cmark library for full CommonMark compliance |
| `MD_USE_WEBENGINE` | `OFF` | Enable Qt WebEngine for preview (requires x64) |
| `CMAKE_PREFIX_PATH` | (auto) | Path to Qt6 installation |

### Example with options

```bash
cmake -S . -B build \
    -D CMAKE_BUILD_TYPE=Release \
    -D BUILD_TESTING=ON \
    -D MD_USE_CMARK=ON \
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

### Running the Markdown Preview Tool

The `mdpreview` tool renders markdown to HTML:

```bash
# Linux/macOS
./build/src/cli/mdpreview samples/sample.md out/preview.html

# Windows (run from project root)
.\build\src\cli\Debug\mdpreview.exe samples\sample.md out\preview.html
```

The tool will:
1. Load markdown via GapBuffer
2. Render to HTML using the markdown parser
3. Generate a complete HTML page with styling
4. Write to the output file

Open `out/preview.html` in a browser to view the result.

### Running the Qt6 QML Application (mdapp)

The `mdapp` application provides a graphical markdown editor with live preview.

**Note:** Qt6 must be installed and CMAKE_PREFIX_PATH must be set.

```bash
# Configure with Qt6
cmake -S . -B build -D CMAKE_PREFIX_PATH=C:/Qt/6.10.1/msvc2022_arm64
cmake --build build --parallel

# Run the application
# Windows
.\build\src\mdapp\Debug\mdapp.exe

# Or with a file
.\build\src\mdapp\Debug\mdapp.exe samples\sample.md
```

**Features:**
- Split view: Markdown editor (left) + HTML preview (right)
- Load/Save markdown files via toolbar or keyboard shortcuts (Ctrl+O/S)
- Render button to update preview (Ctrl+R)
- Unsaved changes warning
- Syntax highlighting in preview (via HTML rich text)

**Keyboard Shortcuts:**
| Shortcut | Action |
|----------|--------|
| Ctrl+N | New document |
| Ctrl+O | Open file |
| Ctrl+S | Save file |
| Ctrl+R | Render preview |

**WebEngine Support:**
For full HTML/CSS preview rendering, enable WebEngine (x64 only, not ARM64):
```bash
cmake -S . -B build -D MD_USE_WEBENGINE=ON -D CMAKE_PREFIX_PATH=C:/Qt/6.x.x/msvc2022_64
```

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

### Markdown Parser API

The `markdown` library provides pluggable markdown-to-HTML rendering:

```cpp
#include "IMarkdownParser.h"

// Create default parser (FallbackRenderer or CMarkAdapter)
auto parser = mdeditor::createDefaultParser();

// Render markdown to HTML
std::string html = parser->renderToHtml("# Hello\n\nWorld");

// Check parser info
std::cout << parser->parserName();      // "FallbackRenderer" or "CMarkAdapter"
std::cout << parser->isFullCommonMark(); // false or true
```

**Supported Elements (Fallback Renderer):**
- Headings (# through ######)
- Paragraphs
- Fenced code blocks (``` or ~~~)
- Unordered lists (-, *, +)
- Ordered lists (1., 2.)
- Blockquotes (>)
- Horizontal rules (---, ***, ___)
- Inline: **bold**, *italic*, `code`

For full CommonMark compliance, build with `-DMD_USE_CMARK=ON`.

### Adding new source files

1. Add source files to the appropriate subdirectory
2. Update the corresponding `CMakeLists.txt` using `target_sources()`

## License

[Add your license here]
