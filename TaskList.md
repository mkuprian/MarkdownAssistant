### Iterative Task Roadmap with Copilot Prompts  
**Goal:** Deliver a sequence of development milestones from a minimal prototype to a production‑grade Qt 6 + C++ Markdown editor. Each step is ordered from simplest to most complex and ends with CRDT integration. For every step you’ll find: **objective**, **deliverables**, **files/CMake targets**, **tests & acceptance criteria**, **performance/security notes**, and a ready‑to‑paste **Copilot/Claude prompt** that instructs an agent to implement the step in a production‑ready, testable way.

> **How to use:** Work one step at a time. Run tests and benchmarks at each milestone. Keep commits small and documented. Each Copilot prompt assumes the repository scaffold exists (Iteration 0) and that CMake is used.

---

## Iteration 0 — Repository scaffold and CI baseline (foundation)

**Objective**  
Create a robust repository scaffold with modern CMake, CI, and test harness so subsequent steps can be implemented and validated.

**Deliverables**
- Root `CMakeLists.txt`, `cmake/` helpers.
- Module directories: `src/`, `tests/`, `tools/`, `docs/`, `benchmarks/`.
- Minimal targets: `mdcore_stub`, `mdapp_stub`, `mdtests`.
- `.github/workflows/ci.yml` skeleton that builds and runs tests on Linux/macOS/Windows.
- README with build/test instructions.

**Files / CMake targets**
- `CMakeLists.txt` (root)
- `cmake/FindThirdParty.cmake`
- `src/CMakeLists.txt` (adds `mdcore_stub`)
- `src/mdcore_stub/mdcore_stub.cpp`
- `src/mdapp_stub/CMakeLists.txt`, `src/mdapp_stub/main.cpp` (console stub)
- `tests/CMakeLists.txt`, `tests/test_stub.cpp`
- `.gitignore`, `README.md`, `docs/CONTRIBUTING.md`
- `.github/workflows/ci.yml`

**Tests & Acceptance**
- `cmake -S . -B build && cmake --build build` succeeds.
- `ctest --test-dir build` runs and passes the trivial test.
- CI workflow triggers on push and PR and runs build + tests.

**Copilot prompt (Iteration 0)**

```
Create a C++ project scaffold using CMake for a Qt-based Markdown editor. Target C++17. Produce the following files and content:

- Root CMakeLists.txt: set project name "mdeditor", C++ standard 17, options BUILD_TESTING=ON, MD_USE_WEBENGINE=OFF by default. Add subdirectories `src/`, `tests/`, `tools/`.
- cmake/FindThirdParty.cmake: placeholder module with comments on how to add Find modules.
- src/CMakeLists.txt: add a static library target `mdcore_stub` with one source `mdcore_stub.cpp`.
- src/mdcore_stub/mdcore_stub.cpp: minimal implementation with a function `std::string mdcore_version()` returning "0.0.0-stub".
- src/mdapp_stub/CMakeLists.txt and src/mdapp_stub/main.cpp: console app `mdapp_stub` that prints "mdapp stub".
- tests/CMakeLists.txt: use FetchContent to pull GoogleTest (or Catch2) and add `mdtests` target linking to `mdcore_stub`.
- tests/test_stub.cpp: one trivial test that asserts true.
- .gitignore: ignore build/, .vscode/, *.user, *.swp.
- README.md: build and test instructions:
  - `cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug`
  - `cmake --build build --parallel`
  - `ctest --test-dir build`
- .github/workflows/ci.yml: workflow that checks out code, sets up CMake, builds, and runs tests on ubuntu-latest (macOS/Windows optional).

Make sure all CMake targets use modern CMake idioms (`target_sources`, `target_link_libraries`, `target_compile_features`). Keep code minimal and well-commented. Provide exact file paths and contents.
```

---

## Iteration 1 — Gap buffer editor model (single‑threaded CLI)

**Objective**  
Implement a simple, correct **gap buffer** text model with unit tests and a CLI demo. This provides a fast, local editing primitive and a patch API for later integration.

**Deliverables**
- `IGapBuffer` interface and `GapBuffer` implementation.
- CLI demo that loads a file, performs edits, prints results.
- Unit tests covering insert, delete, getText, line/offset mapping, and patch flush.

**Files / CMake targets**
- `src/gapbuffer/CMakeLists.txt` → `gapbuffer` static lib
- `src/gapbuffer/gap_buffer.h`, `gap_buffer.cpp`
- `src/cli/CMakeLists.txt`, `src/cli/main.cpp` (tool `mdcli`)
- `tests/gapbuffer_tests.cpp` (GoogleTest)
- Update root CMake to include new targets.

**API (public)**
- `GapBuffer()`
- `void loadFromString(const std::string&)`
- `std::string getText() const`
- `std::string getText(size_t start, size_t len) const`
- `void insert(size_t offset, const std::string &text)`
- `void erase(size_t offset, size_t length)`
- `size_t lineFromOffset(size_t offset) const`
- `size_t offsetFromLine(size_t line, size_t column) const`
- `Patch flushPatches()` — coalesced patch for commit.

**Tests & Acceptance**
- Unit tests pass.
- CLI `mdcli samples/sample.md` prints before/after edits.
- Document limitations: byte offsets (UTF‑8 caveat).

**Performance/security notes**
- Single-threaded; optimized for correctness. No grapheme handling yet.

**Copilot prompt (Iteration 1)**

```
Implement a GapBuffer text model in C++ with unit tests and a CLI demo.

Requirements:
- Create a static library target `gapbuffer`.
- Implement `gap_buffer.h` and `gap_buffer.cpp` with the public API:
  - constructor, loadFromString, getText, getText(start,len), insert(offset,text), erase(offset,len), lineFromOffset, offsetFromLine, flushPatches.
- Implement a simple Patch struct: {start, removedLength, insertedText, timestamp}.
- Implement `src/cli/main.cpp` that:
  - loads `samples/sample.md` into GapBuffer,
  - performs a sequence of inserts/deletes,
  - prints original and modified text and line counts.
- Add unit tests `tests/gapbuffer_tests.cpp` using GoogleTest (via FetchContent) covering:
  - load, insert at beginning/middle/end, delete ranges, getText, line/offset mapping, flushPatches.
- Update CMake files to add targets and link tests.

Quality:
- Use RAII, `std::string_view` where appropriate, `const` correctness.
- Document UTF-8 byte-offset semantics in header comments.
- Provide build and test instructions in README.

Acceptance:
- `cmake` + `cmake --build` builds targets.
- `ctest` passes all gapbuffer tests.
- CLI demo runs and prints expected output.
```

---

## Iteration 2 — Full-document Markdown render (CommonMark adapter) + preview CLI

**Objective**  
Add a pluggable `IMarkdownParser` and a renderer that converts the entire document to HTML. Use `cmark` via `FetchContent` if available, otherwise provide a minimal fallback.

**Deliverables**
- `IMarkdownParser` interface and `CMarkAdapter`.
- `mdrender` library that depends on `gapbuffer`.
- CLI `mdpreview` that writes `out/preview.html`.
- Unit tests for parser correctness (headings, code fences, lists).

**Files / CMake targets**
- `src/markdown/CMakeLists.txt` → `markdown` lib
- `src/markdown/IMarkdownParser.h`, `cmark_adapter.cpp/h`, `fallback_renderer.cpp/h`
- `src/cli/preview.cpp` (tool `mdpreview`)
- `tests/markdown_tests.cpp`

**Tests & Acceptance**
- `mdpreview samples/sample.md` produces valid HTML.
- Parser tests pass.
- CMake option `-DMD_USE_CMARK=ON` toggles use of `cmark`.

**Performance/security notes**
- Full re-render for now; acceptable at this stage. Escape code blocks.

**Copilot prompt (Iteration 2)**

```
Add a pluggable markdown parser and full-document renderer.

Requirements:
- Define `IMarkdownParser` interface with `std::string renderToHtml(const std::string &markdown)`.
- Implement `CMarkAdapter` that uses `cmark` (CommonMark C) via FetchContent when `-DMD_USE_CMARK=ON`. Provide a fallback `FallbackRenderer` that supports headings, paragraphs, fenced code blocks, and lists.
- Create `markdown` static library exposing the parser interface.
- Implement CLI `mdpreview` that:
  - loads text from GapBuffer,
  - calls the parser to render HTML,
  - writes `out/preview.html`.
- Add unit tests verifying:
  - headings render to `<h1..h6>`,
  - fenced code blocks are wrapped in `<pre><code>`,
  - lists render to `<ul>/<ol>`.
- Update CMake to add `-DMD_USE_CMARK` option and FetchContent for `cmark` with fallback.

Quality:
- Sanitize code block content by HTML-escaping.
- Keep parser pluggable and documented.

Acceptance:
- `mdpreview` produces HTML for sample markdown.
- Parser tests pass.
```

---

## Iteration 3 — Minimal QML UI (TextArea + full re-render preview)

**Objective**  
Create a minimal Qt QML application that binds to a `DocumentController` and supports load/save and full-document preview. This validates QML integration.

**Deliverables**
- `DocumentController` QObject exposing `text` property and `render()` method.
- QML UI: split view with `TextArea` and `WebEngineView` (or `Text` fallback).
- Build target `mdapp` (Qt 6).

**Files / CMake targets**
- `src/mdapp/CMakeLists.txt` → `mdapp` executable
- `src/mdapp/documentcontroller.h/cpp`
- `src/mdapp/qml/Main.qml`
- `src/mdapp/main.cpp` (registers types)

**Tests & Acceptance**
- `mdapp` runs, loads `samples/sample.md`, editing and clicking Render updates preview.
- DocumentController unit tests for load/save and render invocation.

**Performance/security notes**
- Full re-render on demand; not per-keystroke. WebEngine optional via `-DMD_USE_WEBENGINE`.

**Copilot prompt (Iteration 3)**

```
Create a minimal Qt 6 QML application that integrates the GapBuffer and markdown renderer.

Requirements:
- Add `mdapp` executable target that links Qt6::Core, Qt6::Qml, Qt6::Quick, and optionally Qt6::WebEngine (controlled by -DMD_USE_WEBENGINE).
- Implement `DocumentController` (QObject) with:
  - `Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)`
  - `Q_INVOKABLE void loadFile(const QString &path)`
  - `Q_INVOKABLE void saveFile(const QString &path)`
  - `Q_INVOKABLE void renderToHtml()` which calls the markdown parser and emits `previewReady(QString html)`.
- QML `Main.qml`:
  - Left `TextArea` bound to `DocumentController.text`.
  - Right `WebEngineView` or `Text` that displays `previewReady` HTML.
  - Toolbar with Load/Save/Render buttons.
- main.cpp: register `DocumentController` to QML and load `Main.qml`.
- Add unit tests for `DocumentController` load/save/render behavior.

Quality:
- Keep heavy logic in C++.
- Document how to enable WebEngine via CMake option.

Acceptance:
- `mdapp` builds and runs.
- Loading a sample file and clicking Render updates preview.
```

---

## Iteration 4 — Patch API between QML and model + debounce

**Objective**  
Replace coarse `setText` updates with a patch API. Implement a short debounce and ensure cursor position is sent to prioritize preview rendering.

**Deliverables**
- `applyPatch(start, removedLength, insertedText)` on `DocumentController`.
- QML editor computes minimal diffs and calls `applyPatch`.
- Debounce coalescing in `DocumentController` (10–30 ms).
- `renderAroundCursor(offset, radius)` API and preview fragment signal.

**Files / CMake targets**
- Update `src/mdapp/documentcontroller.*`
- QML changes to compute diffs and call `applyPatch`.
- Tests for patch application and debounce behavior.

**Tests & Acceptance**
- Unit tests for patch application on GapBuffer.
- Manual test: typing in QML triggers `applyPatch` and preview updates cursor vicinity.

**Performance/security notes**
- Keep debounce short to balance responsiveness and batching.

**Copilot prompt (Iteration 4)**

```
Implement a patch-based edit API and debounce in the QML bridge.

Requirements:
- Extend `DocumentController` with:
  - `Q_INVOKABLE void applyPatch(int start, int removedLength, const QString &insertedText, const QString &authorId = QString())`
  - `Q_INVOKABLE void setCursorPosition(int offset, const QString &cursorId = QString())`
  - Signal `previewFragmentReady(QString htmlFragment, int start, int length)`
- Implement a short debounce (10–30 ms) that coalesces rapid local patches into a single commit to the backend model.
- Implement `renderAroundCursor(int offset, int radius)` in the renderer to produce HTML fragments for the cursor vicinity and emit `previewFragmentReady`.
- Modify QML `TextArea` to compute a minimal diff between previous and current text (first/last differing index heuristic) and call `applyPatch` with the computed patch and `setCursorPosition` on cursor moves.
- Add unit tests:
  - patch application correctness,
  - debounce coalescing behavior (simulate rapid patches and assert single commit),
  - `renderAroundCursor` returns expected fragment for a known document.

Quality:
- Ensure thread-safety: background rendering tasks post results to UI thread via signals.
- Document the patch format and debounce rationale.

Acceptance:
- Typing in QML triggers `applyPatch` calls and preview updates for cursor vicinity without full re-render on each keystroke.
```

---

## Iteration 5 — Native syntax highlighter and preview highlighting

**Objective**  
Add `ISyntaxHighlighter` and a native C++ highlighter for common languages. Integrate highlighting into preview rendering.

**Deliverables**
- `ISyntaxHighlighter` interface.
- `NativeHighlighter` implementation (C++, token-based) for C/C++, Python, JS.
- Renderer calls highlighter for fenced code blocks and embeds highlighted HTML.
- Unit tests for highlighter tokenization and HTML output.

**Files / CMake targets**
- `src/highlight/` library target `highlighter`.
- Update `markdown` renderer to call highlighter.

**Tests & Acceptance**
- Code fences in sample markdown are highlighted in preview.
- Native highlighter tests pass.

**Performance/security notes**
- Native highlighter runs in worker threads; avoid blocking UI.

**Copilot prompt (Iteration 5)**

```
Add a pluggable syntax highlighting interface and a native highlighter.

Requirements:
- Define `ISyntaxHighlighter` with `struct Token { size_t start; size_t length; std::string className; };` and `std::vector<Token> highlight(const std::string &code, const std::string &language)`.
- Implement `NativeHighlighter` with simple tokenizers for C/C++, Python, and JavaScript that produce token classes for keywords, strings, comments, numbers.
- Integrate highlighter into the markdown renderer: when rendering fenced code blocks, call `highlight` and wrap tokens with `<span class="...">`.
- Add unit tests verifying tokenization and HTML wrapping for sample code blocks.
- Ensure highlighter runs in worker threads and returns token lists to renderer.

Quality:
- Keep highlighter pluggable; renderer depends only on `ISyntaxHighlighter`.
- Provide CSS classes for highlighted tokens in preview.

Acceptance:
- Preview shows highlighted code blocks for sample documents.
- Highlighter unit tests pass.
```

---

## Iteration 6 — Replace gap buffer with piece table backend (piece table prototype)

**Objective**  
Swap the single-editor gap buffer for a shared **piece table** backend that implements `IDocumentModel`. Keep the multicursor gap buffers as front-end editors that flush patches to the piece table.

**Deliverables**
- `PieceTable` implementation (original + add buffers, piece list).
- `IDocumentModel` interface and adapter that wraps `PieceTable`.
- Keep `IGapBuffer` front-end; `DocumentController` commits flushed patches to `IDocumentModel`.
- Unit tests for piece table correctness and round-trip with gap buffer patches.

**Files / CMake targets**
- `src/piecetable/CMakeLists.txt` → `piecetable` lib
- `src/piecetable/piece_table.h/cpp`
- Update `mdcore` to expose `IDocumentModel` and use `PieceTable` as default backend.

**Tests & Acceptance**
- All previous tests pass after swapping backend.
- Piece table unit tests for insert/delete, line mapping, getText.

**Performance/security notes**
- Piece list implemented as linked list or vector of pieces; O(n) traversal acceptable for prototype.

**Copilot prompt (Iteration 6)**

```
Implement a Piece Table backend and unify it behind IDocumentModel.

Requirements:
- Define `IDocumentModel` interface with methods: snapshot, applyPatch, getText, lineFromOffset, offsetFromLine, registerChangeListener, serialize/deserialize.
- Implement `PieceTable`:
  - Two buffers: `originalBuffer` and `addBuffer`.
  - Piece list (doubly-linked list) where each piece references bufferId, start, length, and lineCount.
  - Implement insert (append to addBuffer and insert piece), delete (split pieces and remove or mark tombstones), getText, and line/offset mapping.
- Provide adapter `PieceTableDocument` implementing `IDocumentModel`.
- Keep `IGapBuffer` as editor front-end; `DocumentController` should call `applyPatch` on `IDocumentModel` when gap buffer flushes.
- Add unit tests for piece table operations and integration tests verifying gap buffer → piece table patch application.

Quality:
- Document serialization format for piece table (compact binary snapshot).
- Provide `compact()` method to coalesce adjacent pieces.

Acceptance:
- Tests pass.
- CLI and QML app work unchanged after swapping to piece table backend.
```

---

## Iteration 7 — Piece tree (balanced tree with line index)

**Objective**  
Upgrade the piece table to a **piece tree** (balanced tree) with augmented metadata for O(log n) navigation and efficient large-document performance.

**Deliverables**
- `PieceTreeDocument` implementing `IDocumentModel`.
- Balanced tree nodes storing subtree `charCount` and `lineCount`.
- Split/insert/delete with rebalancing (AVL or weight-balanced).
- Compaction and snapshot/serialize/deserialize.
- Microbenchmarks comparing piece list vs piece tree for large synthetic documents.

**Files / CMake targets**
- `src/piecetree/CMakeLists.txt` → `piecetree` lib
- `src/piecetree/piece_tree.h/cpp`
- `benchmarks/piecetree_bench.cpp`

**Tests & Acceptance**
- Unit tests for correctness and line/offset mapping on large documents (e.g., 1M lines).
- Benchmarks show O(log n) behavior for random inserts/deletes.

**Performance/security notes**
- Use `std::unique_ptr` for nodes, minimize allocations, and provide compaction background task.

**Copilot prompt (Iteration 7)**

```
Implement a Piece Tree (balanced tree) backend for IDocumentModel.

Requirements:
- Implement `PieceTreeDocument` with:
  - Balanced binary tree nodes (AVL or weight-balanced).
  - Each node stores subtree `charCount` and `lineCount` for fast offset/line mapping.
  - Pieces reference `originalBuffer` or `addBuffer`.
  - Implement insert (append to addBuffer, split node, insert piece), delete (split and mark/remove), getText, lineFromOffset, offsetFromLine.
  - Implement rebalancing (rotations) and node metadata updates.
  - Provide `snapshot()` and `serializeTo(stream)` / `deserializeFrom(stream)`.
  - Provide `compact()` to coalesce adjacent pieces.
- Add unit tests for:
  - Randomized insert/delete sequences and invariants,
  - Line/offset mapping correctness for large synthetic documents (1M lines).
- Add a benchmark `benchmarks/piecetree_bench.cpp` that measures insert/delete latency and compares to piece list implementation.

Quality:
- Use move semantics, avoid unnecessary copies, and instrument timing hooks.
- Document UTF-8 byte offset semantics and optional grapheme mapping.

Acceptance:
- Unit tests pass.
- Benchmarks demonstrate improved scaling vs piece list for large documents.
```

---

## Iteration 8 — Incremental parser & prioritized renderer (worker threads)

**Objective**  
Implement an incremental tokenizer/parser and prioritized render pipeline that runs in worker threads and patches the preview incrementally.

**Deliverables**
- `IncrementalTokenizer`, `IncrementalParser`, `IncrementalRenderer`.
- Priority task scheduler using `QThreadPool` and a priority queue wrapper.
- Mapping `documentRange -> ASTFragment -> HTMLFragment`.
- Preview patching mechanism (DOM patch via WebEngine or native fragment replacement).
- Benchmarks for cursor-to-preview latency.

**Files / CMake targets**
- `src/markdown/incremental_*` libs
- `benchmarks/render_latency.cpp`

**Tests & Acceptance**
- Unit tests for incremental parser correctness.
- Benchmarks: median cursor-to-preview latency < 50ms on a modern dev machine for typical edits (document hardware used).
- No UI thread blocking during parse/render.

**Performance/security notes**
- Use copy-on-write AST fragments, `std::string_view`, and short debounce (10–20 ms) for batching.

**Copilot prompt (Iteration 8)**

```
Implement an incremental parsing and prioritized rendering pipeline.

Requirements:
- Create `IncrementalTokenizer` that re-tokenizes only affected lines and returns token ranges.
- Create `IncrementalParser` that accepts patch ranges and updates AST fragments; it must handle block constructs crossing boundaries.
- Create `IncrementalRenderer` that renders AST fragments to HTML fragments and calls `ISyntaxHighlighter` for code blocks.
- Implement a priority task scheduler:
  - Use `QThreadPool` and `QRunnable`.
  - Implement a priority queue wrapper so tasks for cursor vicinity are executed before background tasks.
- Maintain mapping `documentRange -> ASTFragment -> HTMLFragment` and compute minimal set of fragments to re-render on patch.
- Provide API `renderAroundCursor(offset, radius, priority)` and `renderRange(start,end)`.
- Integrate with `DocumentController` so patch events schedule parse/render tasks and results are posted to UI thread via signals.
- Add benchmarks to measure:
  - patch application time,
  - tokenizer/parser/render times,
  - end-to-end cursor-to-preview latency.

Quality:
- Use `std::string_view` and avoid copying large buffers.
- Ensure thread-safety: AST fragments are immutable once produced.

Acceptance:
- Incremental rendering reduces full re-renders.
- Benchmarks show cursor-to-preview latency < 50ms for typical edits on a modern dev machine (document hardware).
```

---

## Iteration 9 — Search, find, and TOC indexing (scalable)

**Objective**  
Add scalable search/find and a live TOC extractor that supports rapid navigation and lazy indexing for large documents.

**Deliverables**
- `SearchIndex` that supports incremental indexing and fast substring/regex search.
- `TocExtractor` that updates on patch events and exposes `TocModel` (`QAbstractListModel`) to QML.
- UI: TOC sidebar with click-to-navigate.

**Files / CMake targets**
- `src/search/` and `src/toc/` libraries
- QML updates for TOC sidebar

**Tests & Acceptance**
- Unit tests for search correctness and performance on large documents.
- TOC updates live on edits and clicking an entry scrolls editor to offset.

**Performance/security notes**
- Use streaming index and avoid full re-index on each patch. Use suffix arrays or rolling hash for incremental search.

**Copilot prompt (Iteration 9)**

```
Implement scalable search/find and a live TOC extractor.

Requirements:
- Implement `SearchIndex`:
  - Incremental indexing that updates only affected ranges on patch events.
  - Support substring search and regex search.
  - Provide `search(query, startOffset, maxResults)` returning offsets and context snippets.
- Implement `TocExtractor`:
  - Extract headings during parse and maintain hierarchical model with title, level, anchor, offset.
  - Expose `TocModel` as `QAbstractListModel` with roles: title, level, anchor, offset.
- Integrate with `DocumentController`:
  - Emit `tocChanged()` on updates.
  - Provide `jumpToOffset(offset)` for QML to call.
- Add unit tests for search correctness and TOC extraction.
- Update QML UI to show TOC sidebar and support click-to-navigate.

Quality:
- Use efficient incremental algorithms (rolling hash or streaming tokenizer).
- Avoid full re-index for large documents.

Acceptance:
- TOC updates live as document changes.
- Search returns correct results and performs acceptably on large documents.
```

---

## Iteration 10 — Autosave, atomic writes, conflict detection & UI hooks

**Objective**  
Implement robust file I/O: atomic saves, autosave manager, file watchers for external changes, and conflict resolution UI hooks.

**Deliverables**
- `AutosaveManager` with debounce and atomic write (temp + rename).
- File watcher integration using `QFileSystemWatcher`.
- Conflict detection and `conflictDetected` signal with UI dialog skeleton.
- Export: raw Markdown and sanitized HTML.

**Files / CMake targets**
- `src/mdio/` library
- QML dialog components for conflict resolution

**Tests & Acceptance**
- Unit tests for atomic write semantics and conflict detection.
- Manual test: external modification triggers conflict UI.

**Performance/security notes**
- Use checksum or mtime to detect external changes; prefer checksum for reliability.

**Copilot prompt (Iteration 10)**

```
Implement autosave, atomic file writes, and conflict detection.

Requirements:
- Implement `AutosaveManager`:
  - Configurable debounce interval and autosave path.
  - Atomic write: write to temp file then `std::filesystem::rename` to target.
  - Optionally persist versioned snapshots.
- Integrate `QFileSystemWatcher` to detect external file changes.
- On external change, compute diff/checksum and emit `conflictDetected(localSnapshot, externalSnapshot)` signal.
- Provide QML conflict resolution dialog skeleton with options: Keep Local, Accept External, Merge (CRDT-based hook).
- Implement export functions: `exportMarkdown(path)` and `exportSanitizedHtml(path)` (use sanitizer hook).
- Add unit tests for atomic write and conflict detection.

Quality:
- Ensure autosave does not block UI; run file writes in background thread.
- Document how to configure autosave and snapshot retention.

Acceptance:
- Autosave writes atomically.
- External file change triggers `conflictDetected` and UI dialog appears.
```

---

## Iteration 11 — CRDT integration (final, collaborative layer)

**Objective**  
Implement a production‑grade CRDT layer mapped to piece tree primitives, a pluggable transport (WebSocket), presence/cursor sync, and a reference server. This is the final integration enabling real‑time collaboration.

**Deliverables**
- `DocumentCRDT` implementing sequence CRDT semantics (insert ops with unique IDs, tombstones for deletes).
- `INetworkTransport` with Qt WebSockets client implementation.
- Reference server `tools/server/relay_server.js` (Node.js) that relays ops, persists op log, and serves snapshots.
- Presence and cursor ephemeral channels.
- Integration tests: two clients editing concurrently converge.
- Security: token-based auth hooks and TLS configuration notes.

**Files / CMake targets**
- `src/mdsync/` library
- `tools/server/relay_server.js`
- Integration tests under `tests/integration/`

**Tests & Acceptance**
- Unit tests for CRDT merge and idempotency.
- Integration test: start server, run two `mdapp` instances (or CLI clients), perform concurrent edits, assert convergence.
- Presence/cursor updates visible in UI.

**Performance/security notes**
- Persist op log and periodic snapshots for fast join.
- Rate-limit and size-limit messages; require auth token for production.

**Copilot prompt (Iteration 11)**

```
Implement CRDT-based collaboration mapped to the piece tree and a WebSocket transport.

Requirements:
- Implement `DocumentCRDT`:
  - Sequence CRDT (RGA-like) where inserts create unique piece identifiers `{clientId, seq}`.
  - Deletes are tombstones referencing piece ids.
  - Methods: `applyLocalChange(patch) -> op`, `applyRemoteOp(op)`, `generateSnapshot()`, `mergeSnapshot(snapshot)`.
  - Maintain applied opId set for idempotency.
- Implement `INetworkTransport` interface and a Qt WebSockets client transport:
  - `connect(uri, token)`, `send(json)`, `onMessage(callback)`.
- Implement presence/cursor messages on a separate ephemeral channel.
- Provide a reference server `tools/server/relay_server.js` (Node.js + ws) that:
  - Accepts WebSocket connections,
  - Authenticates via token header (simple stub),
  - Relays ops to other clients,
  - Persists op log to disk and serves snapshots on request.
- Integrate CRDT with `PieceTreeDocument`:
  - Local patches produce ops via `applyLocalChange` and are sent over transport.
  - Remote ops are applied via `applyRemoteOp` and update the piece tree.
- Add integration tests:
  - Start server, start two clients, perform concurrent inserts/deletes, assert final document equality.
  - Cursor/presence messages are received and displayed.
- Document op JSON schema and server run instructions.

Quality:
- Ensure idempotency and commutativity of ops.
- Provide hooks for TLS and token-based auth for production.

Acceptance:
- Two clients editing concurrently converge to the same document.
- Presence and remote cursors are visible in UI.
- Integration tests pass.
```

---

## Final notes on process, branching, and QA

- **Branching:** one feature branch per iteration (e.g., `feature/gapbuffer`, `feature/piecetable`, `feature/piecetree`, `feature/incremental-render`, `feature/crdt`). Merge to `dev` after tests pass.
- **Commits:** small, testable commits with clear messages and unit tests.
- **Instrumentation:** add timing logs for parse/render/patch operations at each iteration.
- **Documentation:** update `docs/` at each milestone: API.md, Architecture.md, Performance.md, Security.md.
- **CI:** ensure each iteration adds tests to CI and that CI runs `ctest`.
- **Extensibility:** keep interfaces (`IDocumentModel`, `IMarkdownParser`, `ISyntaxHighlighter`, `INetworkTransport`) stable and minimal.

---

If you want, I can now:
- Generate the **first Copilot-ready patch** (Iteration 0 files) as concrete file contents, or
- Produce the **detailed API specification** (class signatures and header comments) for `IDocumentModel`, `IGapBuffer`, `PieceTable`, `PieceTree`, `DocumentCRDT`, and `DocumentController` to use in the next coding step. Which would you like next?
