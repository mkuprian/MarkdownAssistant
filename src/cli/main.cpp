// =============================================================================
// main.cpp - GapBuffer CLI Demo
// =============================================================================
//
// Demonstrates the GapBuffer text model by:
// 1. Loading a sample markdown file
// 2. Performing a sequence of insert/delete operations  
// 3. Displaying original and modified text with statistics
//
// Usage: mdcli [path/to/file.md]
//        Default: samples/sample.md
//
// =============================================================================

#include "gap_buffer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

/// Reads entire file content into a string
std::string readFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + path.string());
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

/// Prints a separator line
void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

/// Prints buffer statistics
void printStats(const mdeditor::GapBuffer& buffer, const std::string& label) {
    std::cout << "[" << label << "] "
              << "Length: " << buffer.length() << " bytes, "
              << "Lines: " << buffer.lineCount() << "\n";
}

int main(int argc, char* argv[]) {
    try {
        // Determine file path
        fs::path filePath = "samples/sample.md";
        if (argc > 1) {
            filePath = argv[1];
        }
        
        std::cout << "GapBuffer CLI Demo\n";
        std::cout << "==================\n\n";
        std::cout << "Loading file: " << filePath << "\n";
        
        // Check if file exists
        if (!fs::exists(filePath)) {
            std::cerr << "Error: File not found: " << filePath << "\n";
            std::cerr << "Please run from the project root directory.\n";
            return 1;
        }
        
        // Read file content
        std::string content = readFile(filePath);
        
        // Create GapBuffer and load content
        mdeditor::GapBuffer buffer;
        buffer.loadFromString(content);
        
        printSeparator("ORIGINAL CONTENT");
        std::cout << buffer.getText() << "\n";
        printStats(buffer, "Original");
        
        // --- Perform a sequence of edits ---
        
        printSeparator("PERFORMING EDITS");
        
        // 1. Insert at beginning
        std::cout << "\n1. Inserting header comment at beginning...\n";
        buffer.insert(0, "<!-- Edited by GapBuffer CLI Demo -->\n\n");
        
        // 2. Insert in middle (after first line)
        std::cout << "2. Inserting text after line 3...\n";
        size_t line3Start = buffer.offsetFromLine(3, 0);
        buffer.insert(line3Start, "> **Note:** This line was inserted by the demo.\n\n");
        
        // 3. Insert at end
        std::cout << "3. Appending footer at end...\n";
        buffer.insert(buffer.length(), "\n---\n*Modified by mdcli*\n");
        
        // 4. Delete a range (delete 10 bytes starting at offset 50, if buffer is large enough)
        if (buffer.length() > 60) {
            std::cout << "4. Deleting 10 bytes at offset 50...\n";
            std::cout << "   Deleted text: \"" << buffer.getText(50, 10) << "\"\n";
            buffer.erase(50, 10);
        }
        
        printSeparator("MODIFIED CONTENT");
        std::cout << buffer.getText() << "\n";
        printStats(buffer, "Modified");
        
        // --- Demonstrate line/offset mapping ---
        
        printSeparator("LINE/OFFSET MAPPING DEMO");
        
        // Show first 5 lines with their offsets
        const size_t maxLines = std::min(buffer.lineCount(), size_t(5));
        std::cout << "First " << maxLines << " lines:\n\n";
        
        for (size_t line = 0; line < maxLines; ++line) {
            size_t offset = buffer.offsetFromLine(line, 0);
            size_t nextOffset = (line + 1 < buffer.lineCount()) 
                ? buffer.offsetFromLine(line + 1, 0)
                : buffer.length();
            size_t lineLen = nextOffset - offset;
            
            // Get line content (without newline)
            std::string lineContent = buffer.getText(offset, lineLen);
            if (!lineContent.empty() && lineContent.back() == '\n') {
                lineContent.pop_back();
            }
            // Truncate long lines for display
            if (lineContent.length() > 50) {
                lineContent = lineContent.substr(0, 47) + "...";
            }
            
            std::cout << "  Line " << line << " (offset " << offset << "): \"" 
                      << lineContent << "\"\n";
        }
        
        // --- Demonstrate patch flushing ---
        
        printSeparator("PATCH HISTORY");
        
        auto patches = buffer.flushPatches();
        std::cout << "Total patches: " << patches.size() << "\n\n";
        
        for (size_t i = 0; i < patches.size(); ++i) {
            const auto& p = patches[i];
            std::cout << "Patch " << i << ":\n";
            std::cout << "  Start: " << p.start << "\n";
            std::cout << "  Removed: " << p.removedLength << " bytes\n";
            std::cout << "  Inserted: " << p.insertedText.length() << " bytes";
            if (p.insertedText.length() > 0 && p.insertedText.length() <= 30) {
                std::cout << " (\"" << p.insertedText << "\")";
            }
            std::cout << "\n\n";
        }
        
        std::cout << "\nDemo completed successfully!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
