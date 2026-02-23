// =============================================================================
// gapbuffer_tests.cpp - Unit Tests for GapBuffer
// =============================================================================
//
// Comprehensive tests for the GapBuffer text model covering:
// - Loading content
// - Insert operations (beginning, middle, end)
// - Delete operations
// - getText functionality
// - Line/offset mapping
// - Patch tracking and flushing
//
// =============================================================================

#include <gtest/gtest.h>
#include "gap_buffer.h"

using namespace mdeditor;

// =============================================================================
// Test Fixture
// =============================================================================

class GapBufferTest : public ::testing::Test {
protected:
    GapBuffer buffer;
    
    void SetUp() override {
        // Fresh buffer for each test
    }
};

// =============================================================================
// Construction Tests
// =============================================================================

TEST_F(GapBufferTest, DefaultConstructor_CreatesEmptyBuffer) {
    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.length(), 0);
    EXPECT_EQ(buffer.getText(), "");
}

TEST_F(GapBufferTest, CapacityConstructor_CreatesEmptyBuffer) {
    GapBuffer largeBuffer(10000);
    EXPECT_TRUE(largeBuffer.empty());
    EXPECT_EQ(largeBuffer.length(), 0);
}

// =============================================================================
// Load Tests
// =============================================================================

TEST_F(GapBufferTest, LoadFromString_SimplString) {
    buffer.loadFromString("Hello, World!");
    
    EXPECT_FALSE(buffer.empty());
    EXPECT_EQ(buffer.length(), 13);
    EXPECT_EQ(buffer.getText(), "Hello, World!");
}

TEST_F(GapBufferTest, LoadFromString_EmptyString) {
    buffer.loadFromString("");
    
    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.length(), 0);
}

TEST_F(GapBufferTest, LoadFromString_MultiLineString) {
    buffer.loadFromString("Line 1\nLine 2\nLine 3");
    
    EXPECT_EQ(buffer.length(), 20);
    EXPECT_EQ(buffer.lineCount(), 3);
}

TEST_F(GapBufferTest, LoadFromString_ReplacesExistingContent) {
    buffer.loadFromString("First content");
    buffer.loadFromString("Second content");
    
    EXPECT_EQ(buffer.getText(), "Second content");
}

TEST_F(GapBufferTest, Clear_RemovesAllContent) {
    buffer.loadFromString("Some content");
    buffer.clear();
    
    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.getText(), "");
}

// =============================================================================
// Insert Tests
// =============================================================================

TEST_F(GapBufferTest, Insert_AtBeginning) {
    buffer.loadFromString("World!");
    buffer.insert(0, "Hello, ");
    
    EXPECT_EQ(buffer.getText(), "Hello, World!");
}

TEST_F(GapBufferTest, Insert_AtEnd) {
    buffer.loadFromString("Hello");
    buffer.insert(buffer.length(), ", World!");
    
    EXPECT_EQ(buffer.getText(), "Hello, World!");
}

TEST_F(GapBufferTest, Insert_InMiddle) {
    buffer.loadFromString("Hello World!");
    buffer.insert(5, ",");
    
    EXPECT_EQ(buffer.getText(), "Hello, World!");
}

TEST_F(GapBufferTest, Insert_EmptyString) {
    buffer.loadFromString("Hello");
    buffer.insert(2, "");
    
    EXPECT_EQ(buffer.getText(), "Hello");
}

TEST_F(GapBufferTest, Insert_IntoEmptyBuffer) {
    buffer.insert(0, "Hello");
    
    EXPECT_EQ(buffer.getText(), "Hello");
}

TEST_F(GapBufferTest, Insert_OffsetBeyondLength_ClampsToEnd) {
    buffer.loadFromString("Hello");
    buffer.insert(100, " World");
    
    EXPECT_EQ(buffer.getText(), "Hello World");
}

TEST_F(GapBufferTest, Insert_MultipleConsecutive) {
    buffer.loadFromString("AC");
    buffer.insert(1, "B");
    
    EXPECT_EQ(buffer.getText(), "ABC");
    
    buffer.insert(3, "D");
    EXPECT_EQ(buffer.getText(), "ABCD");
}

TEST_F(GapBufferTest, Insert_LargeText) {
    std::string largeText(10000, 'x');
    buffer.insert(0, largeText);
    
    EXPECT_EQ(buffer.length(), 10000);
    EXPECT_EQ(buffer.getText(), largeText);
}

// =============================================================================
// Erase Tests
// =============================================================================

TEST_F(GapBufferTest, Erase_FromBeginning) {
    buffer.loadFromString("Hello, World!");
    buffer.erase(0, 7);
    
    EXPECT_EQ(buffer.getText(), "World!");
}

TEST_F(GapBufferTest, Erase_FromEnd) {
    buffer.loadFromString("Hello, World!");
    buffer.erase(7, 6);
    
    EXPECT_EQ(buffer.getText(), "Hello, ");
}

TEST_F(GapBufferTest, Erase_FromMiddle) {
    buffer.loadFromString("Hello, World!");
    buffer.erase(5, 2);
    
    EXPECT_EQ(buffer.getText(), "HelloWorld!");
}

TEST_F(GapBufferTest, Erase_EntireContent) {
    buffer.loadFromString("Hello");
    buffer.erase(0, 5);
    
    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.getText(), "");
}

TEST_F(GapBufferTest, Erase_ZeroLength) {
    buffer.loadFromString("Hello");
    buffer.erase(2, 0);
    
    EXPECT_EQ(buffer.getText(), "Hello");
}

TEST_F(GapBufferTest, Erase_OffsetBeyondLength) {
    buffer.loadFromString("Hello");
    buffer.erase(100, 5);
    
    EXPECT_EQ(buffer.getText(), "Hello");
}

TEST_F(GapBufferTest, Erase_LengthClamped) {
    buffer.loadFromString("Hello");
    buffer.erase(2, 100);  // Should only erase "llo"
    
    EXPECT_EQ(buffer.getText(), "He");
}

// =============================================================================
// getText Tests
// =============================================================================

TEST_F(GapBufferTest, GetText_EntireBuffer) {
    buffer.loadFromString("Hello, World!");
    
    EXPECT_EQ(buffer.getText(), "Hello, World!");
}

TEST_F(GapBufferTest, GetText_Substring_FromBeginning) {
    buffer.loadFromString("Hello, World!");
    
    EXPECT_EQ(buffer.getText(0, 5), "Hello");
}

TEST_F(GapBufferTest, GetText_Substring_FromMiddle) {
    buffer.loadFromString("Hello, World!");
    
    EXPECT_EQ(buffer.getText(7, 5), "World");
}

TEST_F(GapBufferTest, GetText_Substring_ToEnd) {
    buffer.loadFromString("Hello, World!");
    
    EXPECT_EQ(buffer.getText(7, 100), "World!");  // Clamped
}

TEST_F(GapBufferTest, GetText_Substring_StartBeyondLength) {
    buffer.loadFromString("Hello");
    
    EXPECT_EQ(buffer.getText(100, 5), "");
}

TEST_F(GapBufferTest, GetText_Substring_ZeroLength) {
    buffer.loadFromString("Hello");
    
    EXPECT_EQ(buffer.getText(2, 0), "");
}

TEST_F(GapBufferTest, GetText_AfterGapMove) {
    buffer.loadFromString("ABCDEFGHIJ");
    buffer.insert(5, "X");  // Gap moves to position 5
    
    EXPECT_EQ(buffer.getText(0, 5), "ABCDE");
    EXPECT_EQ(buffer.getText(5, 1), "X");
    EXPECT_EQ(buffer.getText(6, 5), "FGHIJ");
    EXPECT_EQ(buffer.getText(), "ABCDEXFGHIJ");
}

TEST_F(GapBufferTest, GetText_SpansGap) {
    buffer.loadFromString("ABCDEFGHIJ");
    buffer.insert(5, "XYZ");  // Gap at position 5
    
    // Substring that spans the gap position
    EXPECT_EQ(buffer.getText(3, 6), "DEXYZF");
}

// =============================================================================
// Line/Offset Mapping Tests
// =============================================================================

TEST_F(GapBufferTest, LineCount_EmptyBuffer) {
    EXPECT_EQ(buffer.lineCount(), 0);
}

TEST_F(GapBufferTest, LineCount_SingleLine) {
    buffer.loadFromString("Hello");
    
    EXPECT_EQ(buffer.lineCount(), 1);
}

TEST_F(GapBufferTest, LineCount_MultipleLines) {
    buffer.loadFromString("Line 1\nLine 2\nLine 3\n");
    
    EXPECT_EQ(buffer.lineCount(), 4);  // 3 newlines = 4 lines
}

TEST_F(GapBufferTest, LineFromOffset_FirstLine) {
    buffer.loadFromString("Line 1\nLine 2\nLine 3");
    
    EXPECT_EQ(buffer.lineFromOffset(0), 0);
    EXPECT_EQ(buffer.lineFromOffset(3), 0);
    EXPECT_EQ(buffer.lineFromOffset(6), 0);  // At newline
}

TEST_F(GapBufferTest, LineFromOffset_SecondLine) {
    buffer.loadFromString("Line 1\nLine 2\nLine 3");
    
    EXPECT_EQ(buffer.lineFromOffset(7), 1);   // Start of line 2
    EXPECT_EQ(buffer.lineFromOffset(10), 1);
}

TEST_F(GapBufferTest, LineFromOffset_ThirdLine) {
    buffer.loadFromString("Line 1\nLine 2\nLine 3");
    
    EXPECT_EQ(buffer.lineFromOffset(14), 2);  // Start of line 3
    EXPECT_EQ(buffer.lineFromOffset(19), 2);  // End of buffer
}

TEST_F(GapBufferTest, LineFromOffset_BeyondEnd) {
    buffer.loadFromString("Line 1\nLine 2");
    
    // Offset beyond end should clamp
    EXPECT_EQ(buffer.lineFromOffset(100), 1);
}

TEST_F(GapBufferTest, OffsetFromLine_FirstLine) {
    buffer.loadFromString("Line 1\nLine 2\nLine 3");
    
    EXPECT_EQ(buffer.offsetFromLine(0, 0), 0);
    EXPECT_EQ(buffer.offsetFromLine(0, 3), 3);
}

TEST_F(GapBufferTest, OffsetFromLine_SecondLine) {
    buffer.loadFromString("Line 1\nLine 2\nLine 3");
    
    EXPECT_EQ(buffer.offsetFromLine(1, 0), 7);
    EXPECT_EQ(buffer.offsetFromLine(1, 4), 11);
}

TEST_F(GapBufferTest, OffsetFromLine_ThirdLine) {
    buffer.loadFromString("Line 1\nLine 2\nLine 3");
    
    EXPECT_EQ(buffer.offsetFromLine(2, 0), 14);
}

TEST_F(GapBufferTest, OffsetFromLine_ColumnClamped) {
    buffer.loadFromString("Short\nLine");
    
    // Column beyond line length should clamp to buffer length
    size_t offset = buffer.offsetFromLine(0, 100);
    EXPECT_LE(offset, buffer.length());
}

TEST_F(GapBufferTest, LineMapping_RoundTrip) {
    buffer.loadFromString("Line 1\nLine 2\nLine 3\nLine 4\nLine 5");
    
    for (size_t line = 0; line < buffer.lineCount(); ++line) {
        size_t offset = buffer.offsetFromLine(line, 0);
        size_t foundLine = buffer.lineFromOffset(offset);
        EXPECT_EQ(foundLine, line) << "Line " << line << " maps to offset " 
                                   << offset << " which maps back to line " << foundLine;
    }
}

// =============================================================================
// Patch Tests
// =============================================================================

TEST_F(GapBufferTest, FlushPatches_EmptyAfterLoad) {
    buffer.loadFromString("Hello");
    
    auto patches = buffer.flushPatches();
    EXPECT_TRUE(patches.empty());
}

TEST_F(GapBufferTest, HasPendingPatches_AfterInsert) {
    buffer.loadFromString("Hello");
    buffer.insert(0, "X");
    
    EXPECT_TRUE(buffer.hasPendingPatches());
}

TEST_F(GapBufferTest, FlushPatches_ClearsPatches) {
    buffer.loadFromString("Hello");
    buffer.insert(0, "X");
    
    auto patches1 = buffer.flushPatches();
    EXPECT_FALSE(patches1.empty());
    
    auto patches2 = buffer.flushPatches();
    EXPECT_TRUE(patches2.empty());
    EXPECT_FALSE(buffer.hasPendingPatches());
}

TEST_F(GapBufferTest, FlushPatches_SingleInsert) {
    buffer.loadFromString("Hello");
    buffer.insert(0, "XYZ");
    
    auto patches = buffer.flushPatches();
    ASSERT_EQ(patches.size(), 1);
    
    const auto& p = patches[0];
    EXPECT_EQ(p.start, 0);
    EXPECT_EQ(p.removedLength, 0);
    EXPECT_EQ(p.insertedText, "XYZ");
}

TEST_F(GapBufferTest, FlushPatches_SingleDelete) {
    buffer.loadFromString("Hello");
    buffer.erase(2, 2);  // Erase "ll"
    
    auto patches = buffer.flushPatches();
    ASSERT_EQ(patches.size(), 1);
    
    const auto& p = patches[0];
    EXPECT_EQ(p.start, 2);
    EXPECT_EQ(p.removedLength, 2);
    EXPECT_EQ(p.insertedText, "");
}

TEST_F(GapBufferTest, FlushPatches_ConsecutiveInserts_Coalesced) {
    buffer.loadFromString("");
    buffer.insert(0, "A");
    buffer.insert(1, "B");
    buffer.insert(2, "C");
    
    auto patches = buffer.flushPatches();
    // Consecutive forward inserts should be coalesced
    ASSERT_EQ(patches.size(), 1);
    EXPECT_EQ(patches[0].insertedText, "ABC");
}

TEST_F(GapBufferTest, FlushPatches_NonConsecutive_NotCoalesced) {
    buffer.loadFromString("Hello");
    buffer.insert(0, "A");    // Insert at beginning
    buffer.insert(10, "B");   // Insert at end (non-adjacent)
    
    auto patches = buffer.flushPatches();
    EXPECT_GT(patches.size(), 1);  // Should have multiple patches
}

TEST_F(GapBufferTest, FlushPatches_HasTimestamp) {
    buffer.loadFromString("");
    buffer.insert(0, "Test");
    
    auto patches = buffer.flushPatches();
    ASSERT_EQ(patches.size(), 1);
    
    // Timestamp should be set (non-zero duration since epoch)
    auto elapsed = patches[0].timestamp.time_since_epoch();
    EXPECT_GT(elapsed.count(), 0);
}

// =============================================================================
// Copy/Move Semantics Tests
// =============================================================================

TEST_F(GapBufferTest, CopyConstructor) {
    buffer.loadFromString("Hello");
    buffer.insert(5, " World");
    
    GapBuffer copy(buffer);
    
    EXPECT_EQ(copy.getText(), "Hello World");
    EXPECT_EQ(copy.length(), buffer.length());
}

TEST_F(GapBufferTest, CopyAssignment) {
    buffer.loadFromString("Hello");
    
    GapBuffer other;
    other.loadFromString("Different");
    
    other = buffer;
    
    EXPECT_EQ(other.getText(), "Hello");
}

TEST_F(GapBufferTest, MoveConstructor) {
    buffer.loadFromString("Hello World");
    
    GapBuffer moved(std::move(buffer));
    
    EXPECT_EQ(moved.getText(), "Hello World");
}

TEST_F(GapBufferTest, MoveAssignment) {
    buffer.loadFromString("Hello");
    
    GapBuffer other;
    other = std::move(buffer);
    
    EXPECT_EQ(other.getText(), "Hello");
}

// =============================================================================
// Edge Cases and Stress Tests
// =============================================================================

TEST_F(GapBufferTest, ManyRandomOperations) {
    // Perform many operations to test gap movement and resizing
    buffer.loadFromString("Initial content here");
    
    for (int i = 0; i < 100; ++i) {
        size_t pos = i % buffer.length();
        buffer.insert(pos, "X");
    }
    
    EXPECT_EQ(buffer.length(), 20 + 100);  // Original + 100 'X's
}

TEST_F(GapBufferTest, AlternatingInsertDelete) {
    buffer.loadFromString("ABCDE");
    
    buffer.insert(2, "X");   // ABXCDE
    buffer.erase(4, 1);      // ABXCE
    buffer.insert(0, "Y");   // YABXCE
    buffer.erase(3, 2);      // YABCE (wait, let me trace this)
    
    // Let's verify step by step:
    buffer.clear();
    buffer.loadFromString("ABCDE");
    EXPECT_EQ(buffer.getText(), "ABCDE");
    
    buffer.insert(2, "X");
    EXPECT_EQ(buffer.getText(), "ABXCDE");
    
    buffer.erase(4, 1);
    EXPECT_EQ(buffer.getText(), "ABXCE");
    
    buffer.insert(0, "Y");
    EXPECT_EQ(buffer.getText(), "YABXCE");
}

TEST_F(GapBufferTest, EmptyOperations) {
    // Operations on empty buffer shouldn't crash
    EXPECT_EQ(buffer.getText(), "");
    EXPECT_EQ(buffer.getText(0, 10), "");
    EXPECT_EQ(buffer.lineCount(), 0);
    EXPECT_EQ(buffer.lineFromOffset(0), 0);
    EXPECT_EQ(buffer.offsetFromLine(0, 0), 0);
    
    buffer.erase(0, 10);  // Should be no-op
    EXPECT_TRUE(buffer.empty());
}
