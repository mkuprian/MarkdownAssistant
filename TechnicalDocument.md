### Overview

A production‑grade **Qt 6 + modern C++ (C++17/20)** Markdown editor with a QML front end. The application is designed for **extreme scale** (documents up to 1 GB), **low latency** (cursor‑to‑preview < 50 ms typical), and **real‑time collaboration**. The data layer uses a **multicursor gap buffer** fronting a pluggable backend that initially implements a **piece table / piece tree** (VS Code inspired). The architecture is modular so the backend can be swapped without changing UI or collaboration layers.

This document specifies the architecture, interfaces, packages, threading and synchronization strategies, rendering pipeline, collaboration model, persistence, testing, and extension points required to implement the manifest.

---

### High Level Architecture

**Components and responsibilities**

- **mdcore**
  - **Document Layer**: multicursor gap buffers + unified data layer API; initial backend: piece table / piece tree.
  - **Parser Adapter**: pluggable `IMarkdownParser` (CommonMark compatible).
  - **Incremental Renderer**: incremental parse → AST fragments → HTML fragments.
  - **TOC Extractor**: hierarchical heading model.
  - **Syntax Highlighting Interface**: `ISyntaxHighlighter` for editor and preview.
  - **Search and Find**: indexed, incremental search over piece tree.
- **mdsync**
  - **CRDT Engine**: sequence CRDT mapped to piece primitives (inserts, tombstones).
  - **Transport Abstraction**: `INetworkTransport` with WebSocket default; pluggable WebRTC.
  - **Presence and Cursor Sync**: ephemeral channels for cursors and selections.
- **mdio**
  - **File I/O**: load/save, mmap support, atomic writes, autosave manager.
  - **Export**: raw Markdown, sanitized HTML.
  - **Lazy Loading**: partial load and on‑demand materialization for very large files.
- **mdapp**
  - **QML Front End**: editor, preview, TOC sidebar, presence UI, stacked tiles.
  - **Glue**: `DocumentController` and QML‑exposed models.
- **tools/server**
  - **Reference Server**: WebSocket relay with persistence and snapshot endpoints.
- **tests / benchmarks / docs**
  - Unit tests, integration tests, performance harness, CI.

**Module boundaries**
- Each module is a CMake target: `mdcore`, `mdsync`, `mdio`, `mdapp`, `mdtests`.
- Public APIs are minimal and stable; internal implementations are replaceable via interfaces.

---

### Data Layer Design and APIs

**Design goals**
- Provide **low-latency local editing** with multicursor support.
- Support **efficient large-document operations** (inserts/deletes, line mapping).
- Expose a **unified API** so the UI and CRDT layers can operate without knowing the concrete backend.
- Allow **backend replacement** (piece table → piece tree → other) without UI changes.

**Two-tier model**
1. **Editor Buffers (front-end)**
   - **Multicursor Gap Buffers**: per-editor local buffers optimized for immediate keystrokes and local undo/redo. Each local editor instance maintains a gap buffer that records local edits and produces *patches* (range, removedLength, insertedText, metadata).
   - **Responsibilities**: immediate responsiveness, local undo/redo, cursor handling, composition support (IME).
2. **Unified Data Layer (back-end)**
   - **Piece Table / Piece Tree**: authoritative document state. Accepts patches from gap buffers and applies them atomically.
   - **Responsibilities**: global document state, persistence, CRDT mapping, long-term performance.

**Core interfaces (C++ style)**

- **IDocumentModel** (unified API)
  - `virtual DocumentSnapshot snapshot() const = 0;`
  - `virtual Patch applyPatch(const Patch &p) = 0;` // returns canonicalized patch
  - `virtual std::string getText(size_t start, size_t length) const = 0;`
  - `virtual size_t lineFromOffset(size_t offset) const = 0;`
  - `virtual size_t offsetFromLine(size_t line, size_t column) const = 0;`
  - `virtual void registerChangeListener(ChangeListener*) = 0;`
  - `virtual void serializeTo(Stream&) const = 0;`
  - `virtual void deserializeFrom(Stream&) = 0;`

- **IGapBuffer**
  - `void insert(size_t offset, std::string_view text);`
  - `void erase(size_t offset, size_t length);`
  - `Patch flushPatches();` // coalesced patches for commit to IDocumentModel
  - `void setCursor(size_t offset, CursorId id);`
  - `CursorInfo getCursor(CursorId id) const;`

- **IPieceBackend** (piece table / piece tree)
  - `applyPatch`, `getText`, `snapshot`, `compact`, `mmapLoad`, `persistAddBuffer`.

**Patch model**
- `Patch { id, authorId, startOffset, removedLength, insertedText, timestamp, localSeq }`
- Patches are **atomic** and **coalescable**. Local typing is coalesced with a short debounce (10–30 ms) to reduce network chatter while preserving cursor responsiveness.

**Piece Tree internals**
- **Buffers**: `originalBuffer` (read-only, mmap optional) and `addBuffer` (append-only).
- **Piece**: `{bufferId, start, length, lineCount}`.
- **Tree**: balanced binary tree (weight-balanced or B-tree) where each node stores subtree `charCount` and `lineCount` for O(log n) navigation.
- **Operations**:
  - **Insert**: append to `addBuffer`, split piece at offset, insert new piece node, update subtree metadata.
  - **Delete**: split pieces to isolate deletion range, mark removed pieces as tombstones (for CRDT) or remove nodes (for local-only).
  - **Compaction**: coalesce adjacent pieces referencing same buffer; optional background compaction.
- **Line index**: maintain `lineCount` per piece and subtree for fast line/offset mapping.

**UTF-8 and grapheme handling**
- Internally use **byte offsets** for storage and navigation for performance.
- Provide optional grapheme cluster mapping using **ICU** or `libunibreak` for accurate cursor movement and selection in UI.
- Document API semantics: offsets are byte offsets unless `useGrapheme=true` is configured.

**Memory mapping and lazy loading**
- For very large files, support `mmap` for `originalBuffer` to avoid copying.
- Provide **partial materialization**: load only the ranges needed for viewport rendering; background fetch for adjacent ranges.

---

### Collaboration and CRDT Design

**Goals**
- Real‑time multi‑user editing with **local‑first** semantics and offline merge.
- Convergence guarantees and minimal operational complexity.
- Pluggable transport and server implementations.

**CRDT choice and mapping**
- Use a **sequence CRDT** (RGA or Logoot/WOOT family) adapted to piece primitives:
  - **Insert ops** create uniquely identified pieces: `{opId, clientId, localCounter, textRef}`.
  - **Delete ops** mark pieces as tombstones (logical deletion) to preserve causal history.
- **Why tombstones**: simplifies commutativity and idempotency; allows cursor mapping to remain stable across concurrent edits.
- **Op representation**
  - JSON schema example:
    ```json
    {
      "type": "insert",
      "opId": "clientA:123",
      "parentId": "clientA:122",
      "position": 12345,
      "text": "hello",
      "author": "clientA",
      "timestamp": 1670000000
    }
    ```
- **Merge semantics**
  - On remote op arrival, translate op to piece tree modifications: insert piece nodes or mark tombstones.
  - Ensure idempotency by tracking applied `opId`s.
  - Provide `generateSnapshot()` and `mergeSnapshot()` for fast join.

**Transport abstraction**
- **INetworkTransport**
  - `connect(uri, authToken)`
  - `send(message)`
  - `onMessage(callback)`
  - Implementations:
    - **WebSocket** (Qt WebSockets or `uWebSockets` for server).
    - **WebRTC** (optional, for P2P).
    - **Local relay** for CLI tests (TCP/JSON).
- **Server**
  - Reference server in Node.js (ws) or C++ (Boost.Beast/uWebSockets) that:
    - Relays ops to connected clients.
    - Persists op log to disk.
    - Exposes snapshot endpoint and optional TLS.
  - Server must be pluggable; production deployments should add auth and TLS.

**Presence and cursor sync**
- Separate ephemeral channel for presence:
  - `presence { clientId, displayName, color, lastSeen }`
  - `cursor { clientId, anchorOffset, headOffset }`
- Map CRDT positions to current offsets by walking visible pieces (skip tombstones).

**Security**
- Transport supports TLS and token-based auth.
- Rate-limit ops and size-limit messages.
- Sanitize any user-supplied HTML before rendering.

---

### Incremental Parsing and Rendering Pipeline

**Goals**
- Avoid full-document re-render on each keystroke.
- Prioritize viewport and cursor vicinity rendering.
- Use worker threads for heavy work.

**Pipeline stages**
1. **Patch ingestion**
   - Gap buffer flushes coalesced patch to `IDocumentModel`.
   - `IDocumentModel` emits `patchApplied` event with affected range.
2. **Tokenizer**
   - Incremental tokenizer re-tokenizes only affected lines/blocks.
   - Tokenization runs in worker threads; returns token ranges and types.
3. **Parser**
   - Incremental parser updates AST fragments for changed ranges.
   - Parser must be able to extend parsing boundaries if block constructs cross patch boundaries (e.g., fenced code blocks).
4. **Renderer**
   - Render AST fragments to HTML fragments.
   - For code blocks, call `ISyntaxHighlighter` to produce highlighted HTML.
5. **Preview patching**
   - Send minimal HTML fragment patches to preview component.
   - Preview applies DOM patches (WebEngine) or updates native rich text fragments.

**Prioritization and scheduling**
- **Priority queue** for parse/render tasks:
  - **High priority**: viewport and cursor vicinity (synchronous-ish).
  - **Medium**: user-visible viewport outside cursor.
  - **Low**: background rendering of rest of document.
- Use `QThreadPool` + `QRunnable` for tasks; use a priority scheduler wrapper.
- Use `QMetaObject::invokeMethod` with `Qt::QueuedConnection` to post results to UI thread.

**Incremental AST and fragment mapping**
- Maintain mapping: `documentRange -> ASTFragment -> HTMLFragment`.
- On patch, compute minimal set of fragments to reparse and re-render.
- Use copy-on-write AST fragments to avoid locking.

**Tokenization and highlighting**
- **Editor highlighting**: incremental line tokenizer that returns token ranges for `TextArea` styling.
- **Preview highlighting**: highlighter invoked during render; two implementations:
  - **NativeHighlighter**: C++ tokenizers for common languages (fast, deterministic).
  - **WebHighlighter**: use highlight.js inside WebEngine for broader language support.
- Highlighter interface:
  - `TokenList highlightCode(std::string_view code, std::string_view language);`

**Performance techniques**
- **Zero-copy** where possible: use `std::string_view` and buffer references.
- **Batching**: coalesce frequent edits into single parse/render tasks with short debounce (10–30 ms).
- **Viewport virtualization**: only materialize and render visible lines in UI.
- **Lazy loading**: for huge files, load and parse on demand.

---

### QML Integration and UI Patterns

**QML surface design**
- **DocumentController** (QObject)
  - Exposes:
    - `Q_PROPERTY(QString filePath READ filePath NOTIFY fileChanged)`
    - `Q_INVOKABLE void load(const QString &path)`
    - `Q_INVOKABLE void save(const QString &path)`
    - `Q_INVOKABLE void applyPatch(int start, int removedLength, const QString &insertedText, QString authorId)`
    - `Q_INVOKABLE void setCursorPosition(int offset, QString cursorId)`
    - Signals: `previewFragmentsReady(QString html, int start, int length)`, `tocChanged()`, `presenceUpdated()`, `conflictDetected()`
- **TocModel** (`QAbstractListModel`)
  - Roles: `title`, `level`, `anchor`, `offset`
- **PresenceModel** (`QAbstractListModel`)
  - Roles: `clientId`, `displayName`, `color`, `cursorOffset`

**Editor component**
- Use `TextArea` or a custom `QQuickItem` for advanced features.
- Editor emits minimal patches to `DocumentController` rather than full text replacement.
- For multicursor support, expose cursor APIs and render remote cursors as overlays.

**Preview component**
- Two options:
  - **Native renderer**: render sanitized HTML fragments into a `QQuickPaintedItem` or `Text` with rich text support.
  - **WebEngine**: use `QWebEngineView` for full HTML/CSS support and highlight.js integration. Use a JS bridge to apply DOM patches and to report link clicks back to QML.
- **Security**: always sanitize HTML before sending to WebEngine; set strict CSP.

**UI features**
- **Split view**: toggle between edit, preview, and split modes.
- **TOC sidebar**: live updates, click to navigate (scroll to offset).
- **Stacked tiles**: allow multiple documents or views in a tiled layout.
- **Presence indicators**: colored cursors, selection overlays, presence list.
- **Conflict resolution UI**: modal or inline merge suggestions.

**Accessibility and i18n**
- Expose accessible names and roles in QML.
- Support RTL layouts and font scaling.
- Provide localization hooks for UI strings.

---

### File I/O, Persistence, and Autosave

**Loading**
- Support:
  - **Full load**: read file into `originalBuffer` (mmap if large).
  - **Partial load**: load only ranges required for initial viewport; background load for rest.
- Use streaming reads for very large files.

**Saving**
- **Atomic write**: write to temp file then `rename` to target.
- **Autosave**:
  - Debounced autosave manager with configurable interval and size threshold.
  - Save to `.md~` or versioned snapshots.
- **Conflict detection**
  - Use file watchers (Qt `QFileSystemWatcher`) and compare mtimes/checksums.
  - On external change, surface conflict UI with options: keep local, accept external, merge (CRDT merge).

**Persistence formats**
- **Raw Markdown**: default export.
- **Piece tree snapshot**: compact binary snapshot containing piece list and add buffer for fast reload.
- **CRDT op log**: append-only JSON or binary op log for server persistence and replay.

**Backup and versioning**
- Optional periodic snapshots and undo history persisted to disk for crash recovery.

---

### Testing, Benchmarks, and CI

**Testing strategy**
- **Unit tests**:
  - Document model operations (gap buffer, piece table, piece tree).
  - Parser adapter correctness.
  - Syntax highlighter tokenization.
  - CRDT merge and idempotency.
- **Integration tests**:
  - Two-client convergence tests against reference server.
  - UI smoke tests (QML) using Qt Test or Squish (optional).
- **Fuzz tests**:
  - Random insert/delete sequences to validate piece tree invariants and CRDT convergence.

**Benchmark harness**
- Scripts to measure:
  - Patch application latency.
  - Tokenization and parse time for changed ranges.
  - Render time for cursor vicinity.
  - End‑to‑end cursor‑to‑preview latency.
- Target scenarios:
  - 10 MB, 50 MB, 200 MB, 1 GB synthetic documents.
  - Random edits near start, middle, end.
- Output CSV for analysis.

**CI**
- `.github/workflows/ci.yml`:
  - Build on Ubuntu, macOS, Windows.
  - Run unit tests and integration tests.
  - Optional: run benchmarks on self-hosted runners.
- Use `ctest` for test orchestration.

---

### Packages, Dependencies, and Build

**Core dependencies**
- **Qt 6**: Core, Gui, Qml, Quick, WebEngine (optional), WebSockets.
- **C++ standard**: C++17 or C++20 (choose one; document in CMake).
- **CMake**: >= 3.16 (prefer >= 3.20).

**Recommended third‑party libraries**
- **CommonMark parser**: `cmark` (CommonMark C) via `FetchContent` or `libcmark-gfm`.
- **Syntax highlighting**:
  - Native: small tokenizer libraries or custom implementation.
  - Web: `highlight.js` (used inside WebEngine).
- **CRDT libraries**: none required; implement prototype CRDT in `mdsync` and allow pluggable replacement (e.g., Automerge C++ or Yrs via bindings).
- **JSON**: `nlohmann/json` for op messages and config.
- **Logging**: `spdlog` or `plog`.
- **Testing**: GoogleTest or Catch2 via `FetchContent`.
- **Unicode**: ICU or `libunibreak` for grapheme cluster support (optional but recommended).
- **Networking**: Qt WebSockets for client; server may use Node.js `ws` or C++ `uWebSockets`/Boost.Beast.
- **Optional**: `fmt` for formatting, `benchmark` for microbenchmarks.

**CMake patterns**
- Use modern CMake: `target_sources`, `target_include_directories`, `target_compile_features`, `target_link_libraries` with `PRIVATE/PUBLIC/INTERFACE`.
- Provide options:
  - `-DMD_USE_WEBENGINE=ON/OFF`
  - `-DMD_USE_CMARK=ON/OFF`
  - `-DBUILD_TESTING=ON/OFF`
  - `-DBUILD_BENCHMARKS=ON/OFF`
- Use `FetchContent` for optional libs and provide clear fallback behavior.

---

### Security, Privacy, and Operational Considerations

**Security**
- Sanitize HTML before rendering in WebEngine; use a robust sanitizer (e.g., `libxml2` + whitelist) or server-side sanitization.
- Use TLS for transport in production; provide configuration for certs.
- Authenticate clients (token-based) and authorize document access.
- Rate-limit and size-limit incoming messages to prevent DoS.

**Privacy**
- Document what is persisted locally and what is sent to server.
- Provide opt-in telemetry only.

**Operational**
- Server persistence: store CRDT op log and periodic snapshots.
- Provide admin endpoints for snapshot restore and log compaction.
- Provide monitoring hooks and metrics for parse/render latencies.

---

### Extensibility, Plugin Points, and Roadmap

**Pluggable interfaces**
- `IMarkdownParser` — swap parser implementations.
- `ISyntaxHighlighter` — native or WebEngine highlighters.
- `INetworkTransport` — WebSocket, WebRTC, or custom transports.
- `IDocumentModel` — allow replacing piece tree with other backends.
- `ICodeBlockRenderer` — custom renderers for code blocks (e.g., run code, embed diagrams).

**Plugin model**
- Define a plugin API for:
  - Custom markdown extensions.
  - Exporters (PDF, HTML templates).
  - Language servers for code blocks (future).

**Roadmap (incremental)**
1. **Prototype**: gap buffer + full re-render CLI + minimal QML UI.
2. **Patch API**: multicursor gap buffer → unified data layer (piece table).
3. **Piece tree**: balanced tree with line index and mmap support.
4. **CRDT prototype**: op log + WebSocket relay server.
5. **Incremental parser/renderer**: prioritized worker threads and preview patching.
6. **Polish**: autosave, conflict UI, search, benchmarks, CI.
7. **Production hardening**: TLS, auth, sanitizer, performance tuning for 1 GB.

---

### Appendix Summary of Key Interfaces and Message Formats

**Key C++ interfaces**
- `IDocumentModel`, `IGapBuffer`, `IPieceBackend`, `IMarkdownParser`, `ISyntaxHighlighter`, `DocumentCRDT`, `INetworkTransport`, `DocumentController` (QML bridge).

**CRDT op JSON schema (example)**
- `insert`:
  - `{ "type":"insert", "opId":"client:seq", "pos":1234, "text":"...", "author":"client" }`
- `delete`:
  - `{ "type":"delete", "opId":"client:seq", "pos":1234, "length":5, "author":"client" }`
- `presence`:
  - `{ "type":"presence", "client":"id", "name":"Alice", "color":"#ff0000" }`
- `cursor`:
  - `{ "type":"cursor", "client":"id", "anchor":100, "head":105 }`

---

This technical document is intended as a **developer blueprint**. It defines stable public APIs, internal data structures, threading and scheduling strategies, and the integration points required to implement a scalable, extensible, and collaborative Markdown editor in Qt and modern C++. If you want, I can now generate:

- **A detailed API specification** with class and method signatures for each interface above, or
- **A file-by-file CMake scaffold** for the initial prototype (Iteration 0 → gap buffer), including tests and a minimal QML app.

Which deliverable should I produce next?
