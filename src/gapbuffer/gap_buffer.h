// =============================================================================
// gap_buffer.h - Gap Buffer Text Model
// =============================================================================
//
// A gap buffer is a dynamic array with a "gap" (unused space) that can be
// efficiently moved to any position, enabling fast local insertions and
// deletions. This implementation is optimized for text editing use cases.
//
// UTF-8 BYTE OFFSET SEMANTICS:
// ----------------------------
// All offsets and lengths in this API refer to BYTE positions, not character
// or grapheme positions. When working with UTF-8 text:
//   - Multi-byte characters (e.g., emoji, non-ASCII) occupy multiple bytes
//   - Inserting at an arbitrary byte offset may split a multi-byte character
//   - Callers are responsible for ensuring offsets align to character boundaries
//   - Use a UTF-8 library for character-aware operations
//
// LINE MAPPING:
// -------------
// Lines are 0-indexed. Line boundaries are detected by '\n' (LF).
// lineFromOffset returns the line number containing a given byte offset.
// offsetFromLine returns the byte offset of the start of a given line.
//
// =============================================================================

#ifndef MDEDITOR_GAP_BUFFER_H
#define MDEDITOR_GAP_BUFFER_H

#include <chrono>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace mdeditor {

// =============================================================================
// Patch - Represents a single edit operation
// =============================================================================
/// A Patch captures an edit operation for undo/redo, synchronization, or CRDT.
/// It records what was removed (by length) and what was inserted (by content).
struct Patch {
    size_t start;                      ///< Byte offset where the edit occurred
    size_t removedLength;              ///< Number of bytes removed (0 for pure insert)
    std::string insertedText;          ///< Text that was inserted (empty for pure delete)
    std::chrono::steady_clock::time_point timestamp;  ///< When the patch was created

    Patch() : start(0), removedLength(0), timestamp(std::chrono::steady_clock::now()) {}
    
    Patch(size_t start_, size_t removedLen, std::string_view inserted)
        : start(start_)
        , removedLength(removedLen)
        , insertedText(inserted)
        , timestamp(std::chrono::steady_clock::now()) {}
};

// =============================================================================
// GapBuffer - Efficient text buffer for editing
// =============================================================================
/// GapBuffer implements a gap buffer data structure for efficient text editing.
/// The buffer maintains a "gap" that can be moved to any position, allowing
/// O(1) insertions and deletions at the gap position.
class GapBuffer {
public:
    // -------------------------------------------------------------------------
    // Construction / Destruction
    // -------------------------------------------------------------------------
    
    /// Constructs an empty GapBuffer with default initial capacity.
    GapBuffer();
    
    /// Constructs a GapBuffer with specified initial capacity.
    /// @param initialCapacity Initial buffer size in bytes
    explicit GapBuffer(size_t initialCapacity);
    
    /// Destructor - releases buffer memory
    ~GapBuffer() = default;
    
    // Rule of five - support move semantics
    GapBuffer(const GapBuffer& other);
    GapBuffer& operator=(const GapBuffer& other);
    GapBuffer(GapBuffer&& other) noexcept;
    GapBuffer& operator=(GapBuffer&& other) noexcept;

    // -------------------------------------------------------------------------
    // Loading Content
    // -------------------------------------------------------------------------
    
    /// Loads content from a string, replacing any existing content.
    /// @param text The text to load into the buffer
    void loadFromString(std::string_view text);
    
    /// Clears all content from the buffer.
    void clear();

    // -------------------------------------------------------------------------
    // Retrieving Content
    // -------------------------------------------------------------------------
    
    /// Returns the entire text content as a string.
    /// @return The complete text in the buffer
    [[nodiscard]] std::string getText() const;
    
    /// Returns a substring of the text content.
    /// @param start Byte offset to start from
    /// @param len Number of bytes to retrieve
    /// @return The requested substring (clamped to valid range)
    [[nodiscard]] std::string getText(size_t start, size_t len) const;
    
    /// Returns the total length of the text in bytes (excluding gap).
    /// @return Text length in bytes
    [[nodiscard]] size_t length() const noexcept;
    
    /// Returns true if the buffer contains no text.
    [[nodiscard]] bool empty() const noexcept;

    // -------------------------------------------------------------------------
    // Editing Operations
    // -------------------------------------------------------------------------
    
    /// Inserts text at the specified byte offset.
    /// @param offset Byte position to insert at (0 = beginning)
    /// @param text The text to insert
    /// @note Offset is clamped to [0, length()]
    void insert(size_t offset, std::string_view text);
    
    /// Erases a range of bytes from the buffer.
    /// @param offset Byte position to start erasing
    /// @param len Number of bytes to erase
    /// @note Range is clamped to valid buffer bounds
    void erase(size_t offset, size_t len);

    // -------------------------------------------------------------------------
    // Line/Offset Mapping
    // -------------------------------------------------------------------------
    
    /// Returns the 0-indexed line number containing the given byte offset.
    /// @param offset Byte offset to query
    /// @return Line number (0-indexed)
    [[nodiscard]] size_t lineFromOffset(size_t offset) const;
    
    /// Returns the byte offset of the start of a line.
    /// @param line 0-indexed line number
    /// @param column Column offset within the line (in bytes)
    /// @return Byte offset of position (line, column)
    /// @note Returns end of buffer if line is past last line
    [[nodiscard]] size_t offsetFromLine(size_t line, size_t column = 0) const;
    
    /// Returns the total number of lines (at least 1 for non-empty buffer).
    /// @return Number of lines
    [[nodiscard]] size_t lineCount() const;

    // -------------------------------------------------------------------------
    // Patch Management
    // -------------------------------------------------------------------------
    
    /// Returns and clears the accumulated patches since last flush.
    /// Patches are coalesced where possible (adjacent inserts/deletes).
    /// @return Vector of patches representing edits since last flush
    [[nodiscard]] std::vector<Patch> flushPatches();
    
    /// Returns true if there are unflushed patches.
    [[nodiscard]] bool hasPendingPatches() const noexcept;

private:
    // -------------------------------------------------------------------------
    // Internal Implementation
    // -------------------------------------------------------------------------
    
    /// Moves the gap to the specified position
    void moveGapTo(size_t position);
    
    /// Ensures there is enough room in the gap for an insertion
    void ensureGapCapacity(size_t requiredSize);
    
    /// Grows the buffer capacity
    void grow(size_t minCapacity);
    
    /// Records a patch for the edit history
    void recordPatch(size_t start, size_t removedLength, std::string_view inserted);

    // Buffer layout: [text before gap][...gap...][text after gap]
    std::vector<char> buffer_;    ///< The underlying buffer
    size_t gapStart_;             ///< Start index of the gap
    size_t gapEnd_;               ///< End index of the gap (one past last gap byte)
    
    std::vector<Patch> pendingPatches_;  ///< Unflushed edit patches
    
    static constexpr size_t kDefaultCapacity = 4096;
    static constexpr size_t kMinGapSize = 256;
};

} // namespace mdeditor

#endif // MDEDITOR_GAP_BUFFER_H
