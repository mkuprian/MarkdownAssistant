// =============================================================================
// gap_buffer.cpp - Gap Buffer Text Model Implementation
// =============================================================================

#include "gap_buffer.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>

namespace mdeditor {

// =============================================================================
// Construction / Destruction
// =============================================================================

GapBuffer::GapBuffer()
    : GapBuffer(kDefaultCapacity) {
}

GapBuffer::GapBuffer(size_t initialCapacity)
    : buffer_(initialCapacity)
    , gapStart_(0)
    , gapEnd_(initialCapacity) {
}

GapBuffer::GapBuffer(const GapBuffer& other)
    : buffer_(other.buffer_)
    , gapStart_(other.gapStart_)
    , gapEnd_(other.gapEnd_)
    , pendingPatches_(other.pendingPatches_) {
}

GapBuffer& GapBuffer::operator=(const GapBuffer& other) {
    if (this != &other) {
        buffer_ = other.buffer_;
        gapStart_ = other.gapStart_;
        gapEnd_ = other.gapEnd_;
        pendingPatches_ = other.pendingPatches_;
    }
    return *this;
}

GapBuffer::GapBuffer(GapBuffer&& other) noexcept
    : buffer_(std::move(other.buffer_))
    , gapStart_(other.gapStart_)
    , gapEnd_(other.gapEnd_)
    , pendingPatches_(std::move(other.pendingPatches_)) {
    other.gapStart_ = 0;
    other.gapEnd_ = 0;
}

GapBuffer& GapBuffer::operator=(GapBuffer&& other) noexcept {
    if (this != &other) {
        buffer_ = std::move(other.buffer_);
        gapStart_ = other.gapStart_;
        gapEnd_ = other.gapEnd_;
        pendingPatches_ = std::move(other.pendingPatches_);
        other.gapStart_ = 0;
        other.gapEnd_ = 0;
    }
    return *this;
}

// =============================================================================
// Loading Content
// =============================================================================

void GapBuffer::loadFromString(std::string_view text) {
    // Clear existing patches when loading new content
    pendingPatches_.clear();
    
    // Allocate buffer with room for gap
    const size_t newCapacity = std::max(text.size() + kMinGapSize, kDefaultCapacity);
    buffer_.resize(newCapacity);
    
    // Copy text to beginning of buffer
    if (!text.empty()) {
        std::memcpy(buffer_.data(), text.data(), text.size());
    }
    
    // Gap starts after the text
    gapStart_ = text.size();
    gapEnd_ = newCapacity;
}

void GapBuffer::clear() {
    gapStart_ = 0;
    gapEnd_ = buffer_.size();
    pendingPatches_.clear();
}

// =============================================================================
// Retrieving Content
// =============================================================================

std::string GapBuffer::getText() const {
    std::string result;
    result.reserve(length());
    
    // Append text before gap
    result.append(buffer_.data(), gapStart_);
    // Append text after gap
    result.append(buffer_.data() + gapEnd_, buffer_.size() - gapEnd_);
    
    return result;
}

std::string GapBuffer::getText(size_t start, size_t len) const {
    const size_t textLen = length();
    
    // Clamp to valid range
    if (start >= textLen) {
        return {};
    }
    len = std::min(len, textLen - start);
    
    if (len == 0) {
        return {};
    }
    
    std::string result;
    result.reserve(len);
    
    // Calculate how much is before and after the gap
    const size_t beforeGapLen = gapStart_;
    
    if (start < beforeGapLen) {
        // Some or all of the requested range is before the gap
        const size_t endBeforeGap = std::min(start + len, beforeGapLen);
        result.append(buffer_.data() + start, endBeforeGap - start);
        
        if (start + len > beforeGapLen) {
            // Range continues after the gap
            const size_t afterGapStart = gapEnd_;
            const size_t afterGapLen = (start + len) - beforeGapLen;
            result.append(buffer_.data() + afterGapStart, afterGapLen);
        }
    } else {
        // Entirely after the gap
        const size_t adjustedStart = gapEnd_ + (start - beforeGapLen);
        result.append(buffer_.data() + adjustedStart, len);
    }
    
    return result;
}

size_t GapBuffer::length() const noexcept {
    return buffer_.size() - (gapEnd_ - gapStart_);
}

bool GapBuffer::empty() const noexcept {
    return length() == 0;
}

// =============================================================================
// Editing Operations
// =============================================================================

void GapBuffer::insert(size_t offset, std::string_view text) {
    if (text.empty()) {
        return;
    }
    
    // Clamp offset to valid range
    offset = std::min(offset, length());
    
    // Ensure we have enough space
    ensureGapCapacity(text.size());
    
    // Move gap to insertion point
    moveGapTo(offset);
    
    // Insert text into gap
    std::memcpy(buffer_.data() + gapStart_, text.data(), text.size());
    gapStart_ += text.size();
    
    // Record the patch
    recordPatch(offset, 0, text);
}

void GapBuffer::erase(size_t offset, size_t len) {
    const size_t textLen = length();
    
    if (offset >= textLen || len == 0) {
        return;
    }
    
    // Clamp length to valid range
    len = std::min(len, textLen - offset);
    
    // Get the text being erased for the patch
    std::string erasedText = getText(offset, len);
    
    // Move gap to deletion point
    moveGapTo(offset);
    
    // Expand gap to cover deleted text
    gapEnd_ += len;
    
    // Record the patch (with empty inserted text)
    recordPatch(offset, len, "");
}

// =============================================================================
// Line/Offset Mapping
// =============================================================================

size_t GapBuffer::lineFromOffset(size_t offset) const {
    const size_t textLen = length();
    offset = std::min(offset, textLen);
    
    size_t line = 0;
    size_t pos = 0;
    
    // Count newlines in text before gap
    const size_t beforeGapLen = std::min(offset, gapStart_);
    for (size_t i = 0; i < beforeGapLen; ++i) {
        if (buffer_[i] == '\n') {
            ++line;
        }
    }
    pos = beforeGapLen;
    
    // If offset extends past the gap, count newlines after gap
    if (offset > gapStart_) {
        const size_t afterGapOffset = gapEnd_ + (offset - gapStart_);
        for (size_t i = gapEnd_; i < afterGapOffset; ++i) {
            if (buffer_[i] == '\n') {
                ++line;
            }
        }
    }
    
    return line;
}

size_t GapBuffer::offsetFromLine(size_t line, size_t column) const {
    if (line == 0 && column == 0) {
        return 0;
    }
    
    const size_t textLen = length();
    size_t currentLine = 0;
    size_t offset = 0;
    
    // Search before gap
    for (size_t i = 0; i < gapStart_ && currentLine < line; ++i) {
        if (buffer_[i] == '\n') {
            ++currentLine;
        }
        ++offset;
    }
    
    // If we haven't found the line yet, search after gap
    if (currentLine < line) {
        for (size_t i = gapEnd_; i < buffer_.size() && currentLine < line; ++i) {
            if (buffer_[i] == '\n') {
                ++currentLine;
            }
            ++offset;
        }
    }
    
    // Add column offset (clamped to text length)
    return std::min(offset + column, textLen);
}

size_t GapBuffer::lineCount() const {
    if (empty()) {
        return 0;
    }
    
    size_t lines = 1;  // At least one line if not empty
    
    // Count newlines before gap
    for (size_t i = 0; i < gapStart_; ++i) {
        if (buffer_[i] == '\n') {
            ++lines;
        }
    }
    
    // Count newlines after gap
    for (size_t i = gapEnd_; i < buffer_.size(); ++i) {
        if (buffer_[i] == '\n') {
            ++lines;
        }
    }
    
    return lines;
}

// =============================================================================
// Patch Management
// =============================================================================

std::vector<Patch> GapBuffer::flushPatches() {
    std::vector<Patch> result = std::move(pendingPatches_);
    pendingPatches_.clear();
    return result;
}

bool GapBuffer::hasPendingPatches() const noexcept {
    return !pendingPatches_.empty();
}

// =============================================================================
// Internal Implementation
// =============================================================================

void GapBuffer::moveGapTo(size_t position) {
    if (position == gapStart_) {
        return;  // Gap is already at the target position
    }
    
    const size_t gapSize = gapEnd_ - gapStart_;
    
    if (position < gapStart_) {
        // Move gap left: shift text right into the gap
        const size_t shiftSize = gapStart_ - position;
        std::memmove(
            buffer_.data() + gapEnd_ - shiftSize,
            buffer_.data() + position,
            shiftSize
        );
        gapStart_ = position;
        gapEnd_ = position + gapSize;
    } else {
        // Move gap right: shift text left into the gap
        const size_t shiftSize = position - gapStart_;
        std::memmove(
            buffer_.data() + gapStart_,
            buffer_.data() + gapEnd_,
            shiftSize
        );
        gapStart_ = position;
        gapEnd_ = position + gapSize;
    }
}

void GapBuffer::ensureGapCapacity(size_t requiredSize) {
    const size_t gapSize = gapEnd_ - gapStart_;
    if (gapSize >= requiredSize) {
        return;  // Already have enough space
    }
    
    // Calculate new capacity (at least double, or enough for required + min gap)
    const size_t currentCapacity = buffer_.size();
    const size_t needed = length() + requiredSize + kMinGapSize;
    const size_t newCapacity = std::max(currentCapacity * 2, needed);
    
    grow(newCapacity);
}

void GapBuffer::grow(size_t minCapacity) {
    const size_t oldCapacity = buffer_.size();
    const size_t textAfterGap = oldCapacity - gapEnd_;
    
    // Resize buffer
    buffer_.resize(minCapacity);
    
    // Move text after gap to end of new buffer
    if (textAfterGap > 0) {
        std::memmove(
            buffer_.data() + minCapacity - textAfterGap,
            buffer_.data() + gapEnd_,
            textAfterGap
        );
    }
    
    // Update gap end
    gapEnd_ = minCapacity - textAfterGap;
}

void GapBuffer::recordPatch(size_t start, size_t removedLength, std::string_view inserted) {
    // Simple coalescing: if the last patch is adjacent, try to merge
    if (!pendingPatches_.empty()) {
        Patch& last = pendingPatches_.back();
        
        // Check if this is a consecutive insert at the end of the last patch
        if (removedLength == 0 && last.removedLength == 0 &&
            start == last.start + last.insertedText.size()) {
            last.insertedText.append(inserted);
            last.timestamp = std::chrono::steady_clock::now();
            return;
        }
        
        // Check if this is a consecutive delete (backspace) before the last patch
        if (inserted.empty() && last.insertedText.empty() &&
            start + removedLength == last.start) {
            last.start = start;
            last.removedLength += removedLength;
            last.timestamp = std::chrono::steady_clock::now();
            return;
        }
    }
    
    // Cannot coalesce; create a new patch
    pendingPatches_.emplace_back(start, removedLength, inserted);
}

} // namespace mdeditor
