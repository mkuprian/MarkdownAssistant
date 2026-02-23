# Sample Markdown Document

This is a sample markdown file used to demonstrate the GapBuffer text model.

## Features

The GapBuffer provides efficient text editing operations:

- Fast insertions at any position
- Fast deletions of text ranges
- Line and offset mapping
- Patch tracking for undo/redo

## Code Example

```cpp
#include "gap_buffer.h"

int main() {
    mdeditor::GapBuffer buffer;
    buffer.loadFromString("Hello, World!");
    buffer.insert(7, "Beautiful ");
    // Result: "Hello, Beautiful World!"
    return 0;
}
```

## Notes

The GapBuffer uses **byte offsets** for all operations. This means:

1. Multi-byte UTF-8 characters span multiple offsets
2. Care must be taken not to split multi-byte sequences
3. Line counts are based on `\n` (LF) characters

---

*End of sample document*
