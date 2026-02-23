### Project Manifest

#### Overview
A Qt 6 and modern C++ (C++17/20) **Markdown editor** with a QML front end that supports large-document performance, syntax highlighting, live cursor-synchronized preview, a live Table of Contents sidebar, and **real-time collaborative editing**. The core document model will use a multicursor gap buffer that will feed into a data layer that will intially be **piece tree (piece table / piece-tree)** data structure inspired by Visual Studio Code’s implementation to provide efficient edits, snapshots, and incremental rendering for very large files. That said the data layer should be made for ability to update and change the backend and needs a unified API to feed the gap buffers.

#### Scope
- **Core features**: load/save piece tree either in mmap or makrdown buffer, export Markdown, incremental parsing and rendering, syntax highlighting for fenced code blocks, TOC extraction, undo/redo, atomic autosave, search and find functionality, lazy loading.
- **UI**: QML-based split edit/preview mode, TOC sidebar, presence/cursor indicators, conflict resolution UI hooks. Stacked tiles for extensibility. Ability to partial load text buffer but seemlessly scrollable to the entire text document
- **Collaboration**: CRDT-based document synchronization with pluggable transports (WebSocket default), presence/cursor sync, local-first editing and offline merge.
- **Performance**: responsive editing for documents up to 1 GB; cursor-to-preview latency target **< 50ms** on a modern dev machine for typical edits.
- **Extensibility**: pluggable parser, highlighter, and sync transport, unified data layer allowing potential replacement of piece tree.
- **Quality**: unit tests, integration tests, benchmark harness, CI.

#### Key Components
- **mdcore**: Document model (piece tree), parser adapter, incremental renderer, TOC extractor, syntax-highlighting interface, search and find interface.
- **mdsync**: CRDT adapter for piece tree operations, transport abstraction, presence/cursor sync.
- **mdio**: File I/O, autosave, atomic writes, export.
- **mdapp**: QML frontend, C++ glue, QML-exposed models and types.
- **tools/server**: Reference collaboration server (WebSocket relay + persistence).
- **tests/benchmarks/docs**: Unit/integration tests, performance harness, documentation.


### Technical Design

#### Why a multicursor gap buffer with a Piece Tree
- **Rationale**: Piece tree (piece table / piece-tree) is optimized for text editors that perform many small edits on large documents while gap buffers are optimized for inserts at the cursor. By combining these two we can have optimized inserts for each user (who will have their own gap buffer but lazy loading of the piece tree) It stores the original file as a read-only buffer and appends all insertions to an add buffer. The document is represented as a sequence of *pieces* referencing ranges in either buffer. This yields:
  - **O(1)** append for insertions into add buffer.
  - **O(log n)** navigation and updates when backed by a balanced tree.
  - Efficient snapshots and minimal copying for large documents.
- **VS Code lineage**: VS Code’s piece tree design demonstrates real-world scalability for multi-megabyte files and is a proven pattern for editor responsiveness.

#### Piece Tree Data Structures and Algorithms
- **Core types**
  - **Buffer**: two buffers — `originalBuffer` (read-only) and `addBuffer` (append-only).
  - **Piece**: `{bufferId, start, length, lineBreakCount}`.
  - **PieceNode**: tree node containing pieces and augmented metadata.
  - **PieceTree**: balanced tree (B-tree or weight-balanced binary tree) where each node stores subtree length and line counts for fast offset/line queries.
- **Augmented metadata**
  - **byteLength** and **charLength** if supporting multi-byte UTF-8.
  - **lineCount** for each piece and subtree to support line-based operations and TOC mapping.
- **Operations**
  - **Insert**: append text to `addBuffer`, split piece at insertion offset, insert new piece referencing addBuffer, update subtree metadata.
  - **Delete**: split pieces to isolate deletion range, mark pieces as removed by replacing with tombstone pieces or by removing nodes and recording tombstones for CRDT compatibility.
  - **GetText(range)**: traverse tree to collect pieces covering range; avoid copying by streaming fragments.
  - **Offset ↔ Line mapping**: use subtree line counts to find line start quickly.
- **Balancing and rebalancing**
  - Implement node split/merge heuristics to keep tree height low.
  - Periodic compaction: coalesce adjacent pieces from same buffer to reduce fragmentation.
- **Memory and performance**
  - Use small piece granularity to avoid excessive fragmentation; tune piece size heuristics.
  - Use move semantics and string views to avoid copying large strings.
  - Provide a compaction API to rebuild a compact piece tree for long-lived documents.

#### Patch Model and Events
- **Patch structure**
  - `Patch { id, startOffset, removedLength, insertedText, authorId, timestamp, localSeq }`
- **Atomic application**
  - Apply patch by performing split operations and updating tree nodes inside a single transaction.
  - Emit `contentChanged(range)` and `patchApplied(patch)` signals after commit.
- **Coalescing**
  - Coalesce adjacent local edits into a single patch for efficiency and to reduce network chatter, with a short debounce window for user typing.

#### CRDT Integration with Piece Tree
- **Mapping edits to CRDT ops**
  - Represent insertions as CRDT insert operations that create uniquely identified pieces in the add buffer.
  - Represent deletions as tombstone operations that mark pieces as logically removed but keep them for causal history.
- **Identifiers**
  - Use globally unique identifiers for inserted pieces: `{clientId, localCounter}`.
  - Maintain causal metadata to support commutative merges.
- **Merge semantics**
  - On remote op arrival, translate CRDT ops into piece tree modifications: insert new piece nodes or mark pieces as tombstones.
  - Ensure idempotency and commutativity by ignoring duplicate ops and using tombstones for deletions.
- **Snapshots and persistence**
  - Persist CRDT state as a sequence of operations or as a compacted snapshot of the piece tree plus add buffer.
- **Cursor mapping**
  - Map CRDT positions to piece tree offsets by walking pieces and counting visible characters (skip tombstones).
  - Provide stable position identifiers for remote cursors that survive concurrent edits.

#### Incremental Parsing and Rendering Pipeline
- **Goals**
  - Avoid full-document re-parse/render on each keystroke.
  - Prioritize viewport and cursor vicinity rendering.
- **Pipeline stages**
  1. **Edit event**: piece tree emits `patchApplied`.
  2. **Tokenizer**: incremental tokenizer re-tokenizes only affected lines or blocks.
  3. **Parser**: incremental parser updates AST fragments for changed ranges.
  4. **Renderer**: produce HTML fragments for changed AST fragments.
  5. **Highlighter**: highlight code blocks during render or via separate tokenization pass.
  6. **UI update**: send minimal DOM/HTML patches to preview component.
- **Implementation details**
  - Use **range-based parsing**: parser accepts `startOffset` and `endOffset` and returns AST fragment plus new end offset if block boundaries extend.
  - Maintain **range-to-fragment mapping** to patch preview DOM.
  - For cursor responsiveness, schedule two rendering priorities:
    - **High priority**: render viewport and cursor vicinity synchronously or with minimal worker latency.
    - **Low priority**: background render of the rest of the document.
- **Worker threads**
  - Use `QThreadPool` and `QRunnable` for tokenizer/parser/renderer tasks.
  - Use `QMetaObject::invokeMethod` with `Qt::QueuedConnection` to post results to main thread.
  - Use lock-free or mutex-protected queues for task results.

#### Syntax Highlighting Strategy
- **Editor highlighting**
  - Lightweight incremental tokenizer that re-tokenizes only affected lines.
  - Provide token ranges and style classes to QML editor for inline styling.
- **Preview highlighting**
  - Integrate highlighter into render stage so code blocks are rendered as highlighted HTML fragments.
  - Options:
    - **Native C++ highlighter**: fast, deterministic, no WebEngine dependency.
    - **WebEngine + JS highlighter**: richer language support via highlight.js or Prism; sandboxed rendering and sanitized HTML required.
- **Pluggability**
  - `ISyntaxHighlighter` interface with implementations for native and WebEngine-based highlighters.

#### QML Integration Patterns
- **Expose minimal C++ surface**
  - `DocumentController` QObject exposing:
    - `load(path)`, `save(path)`, `applyPatch(patch)`, `cursorPosition`, `undo`, `redo`.
    - Signals: `tocChanged`, `patchApplied`, `previewFragmentsReady`.
  - `TocModel` as `QAbstractListModel` for headings with `line`, `level`, `anchor`.
- **Editor component**
  - Use `TextArea` or a custom `QQuickItem` for advanced features.
  - Emit edits as patches to `DocumentController` rather than letting QML own the full text.
- **Preview component**
  - Accept incremental HTML fragments and apply them to a `WebEngineView` or a native rich text renderer.
  - If using WebEngine, use a small JS bridge to apply patches to the DOM and to report link clicks and anchor navigation back to QML.
- **Threading and signals**
  - All QObjects live on the main thread; heavy work happens in worker threads and results are posted back via signals.

#### Autosave, File I/O, and Conflict Handling
- **Autosave**
  - Debounced autosave with atomic write: write to temp file then `rename`.
  - Configurable interval and size threshold.
- **Conflict detection**
  - Detect external file changes via file watchers and compare file mtime or checksum.
  - Present conflict resolution UI with options: keep local, accept external, merge (three-way merge using CRDT snapshot).
- **Export**
  - Raw Markdown export is trivial from piece tree.
  - HTML export must sanitize output; provide hooks for custom sanitizers.

#### Testing and Benchmarking
- **Unit tests**
  - Piece tree operations, line mapping, patch application, undo/redo.
  - Parser adapter correctness and TOC extraction.
  - CRDT merge and idempotency.
- **Integration tests**
  - Two-client convergence tests using the reference server.
  - Cursor mapping and presence tests.
- **Benchmarks**
  - Scripts to open 10MB/50MB files, perform edits at random positions and measure:
    - Time to apply patch in piece tree.
    - Time to produce preview fragments for cursor vicinity.
    - End-to-end cursor-to-preview latency.
  - Use high-resolution timers and produce CSV output for analysis.
- **Performance targets**
  - Cursor-to-preview latency < 50ms for typical edits on modern dev machine.
  - Patch application and local edit operations should be sub-millisecond for small edits.

#### Security, Internationalization, Accessibility
- **Security**
  - Sanitize HTML before rendering in preview.
  - Use TLS for collaboration transport in production.
  - Rate-limit and size-limit incoming messages.
  - Provide authentication hooks and token-based auth for server.
- **Internationalization**
  - Full UTF-8 support; ensure piece tree offsets are in code points or provide mapping for grapheme clusters.
  - Right-to-left support in QML UI and editor.
- **Accessibility**
  - Expose accessible names and roles in QML.
  - Keyboard navigation for TOC and editor.
  - High-contrast and font-size settings.

#### Extensibility and Best Practices
- **Pluggable components**: parser, highlighter, sync transport, sanitizer.
- **API stability**: keep public C++ API minimal and stable; prefer signals/slots over polling.
- **Memory management**: RAII, smart pointers, avoid unnecessary copies.
- **Profiling hooks**: add instrumentation points for timing parse/render phases.
- **Documentation**: document public APIs, message formats, and CRDT operation formats.

---

### Updated Manifest Changes Summary

- **Replace rope with piece tree** in `mdcore` as the canonical document model.
- **Add piece tree-specific tasks**: design, balancing, line index augmentation, compaction, serialization.
- **CRDT mapping** updated to operate on piece tree primitives: unique piece identifiers and tombstones.
- **Benchmarks** updated to measure piece tree operations and compaction behavior.
- **Docs** updated to include `docs/PieceTree.md` describing data structures, algorithms, and tuning knobs.

---

If you want, I can now:
- Produce a **detailed API specification** (class and method signatures in Markdown) for the piece tree and its adapters to QML and CRDT layers.
- Generate a **file-by-file scaffold** (CMake + headers + stubs) that implements the manifest and task list so a developer can begin implementing the piece tree and the rest of the system.
