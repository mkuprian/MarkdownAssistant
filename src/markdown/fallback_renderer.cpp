// =============================================================================
// fallback_renderer.cpp - Fallback Markdown Renderer Implementation
// =============================================================================

#include "fallback_renderer.h"
#include "html_utils.h"

#include <algorithm>
#include <regex>
#include <sstream>

namespace mdeditor {

namespace {

/// Trims leading and trailing whitespace
std::string trim(const std::string& str) {
    const auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    const auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

/// Checks if a line is a horizontal rule
bool isHorizontalRule(const std::string& line) {
    std::string trimmed = trim(line);
    if (trimmed.length() < 3) return false;
    
    char ruleChar = trimmed[0];
    if (ruleChar != '-' && ruleChar != '*' && ruleChar != '_') return false;
    
    for (char c : trimmed) {
        if (c != ruleChar && c != ' ') return false;
    }
    
    size_t count = std::count(trimmed.begin(), trimmed.end(), ruleChar);
    return count >= 3;
}

/// Checks if a line starts a fenced code block and returns fence info
bool isFencedCodeStart(const std::string& line, std::string& language, char& fenceChar, size_t& fenceLen) {
    size_t i = 0;
    while (i < line.size() && line[i] == ' ' && i < 3) ++i;  // Up to 3 spaces
    
    if (i >= line.size()) return false;
    
    fenceChar = line[i];
    if (fenceChar != '`' && fenceChar != '~') return false;
    
    size_t fenceStart = i;
    while (i < line.size() && line[i] == fenceChar) ++i;
    fenceLen = i - fenceStart;
    
    if (fenceLen < 3) return false;
    
    // Extract language (if present, before any backticks)
    language = "";
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
    
    size_t langStart = i;
    while (i < line.size() && line[i] != ' ' && line[i] != '\t' && 
           line[i] != '`' && line[i] != '\n' && line[i] != '\r') ++i;
    
    if (i > langStart) {
        language = line.substr(langStart, i - langStart);
    }
    
    return true;
}

/// Checks if a line ends a fenced code block
bool isFencedCodeEnd(const std::string& line, char fenceChar, size_t minLen) {
    size_t i = 0;
    while (i < line.size() && line[i] == ' ' && i < 3) ++i;
    
    if (i >= line.size() || line[i] != fenceChar) return false;
    
    size_t count = 0;
    while (i < line.size() && line[i] == fenceChar) {
        ++count;
        ++i;
    }
    
    if (count < minLen) return false;
    
    // Rest should be whitespace
    while (i < line.size()) {
        if (line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n') {
            return false;
        }
        ++i;
    }
    
    return true;
}

/// Returns heading level (1-6) or 0 if not a heading
int getHeadingLevel(const std::string& line) {
    size_t i = 0;
    while (i < line.size() && line[i] == ' ' && i < 3) ++i;
    
    int level = 0;
    while (i < line.size() && line[i] == '#' && level < 6) {
        ++level;
        ++i;
    }
    
    // Must have space after # or be at end
    if (level > 0 && (i >= line.size() || line[i] == ' ' || line[i] == '\t')) {
        return level;
    }
    
    return 0;
}

/// Extracts heading content (without # prefix)
std::string getHeadingContent(const std::string& line) {
    size_t i = 0;
    while (i < line.size() && line[i] == ' ' && i < 3) ++i;
    while (i < line.size() && line[i] == '#') ++i;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
    
    std::string content = line.substr(i);
    
    // Remove trailing # and whitespace
    size_t end = content.size();
    while (end > 0 && (content[end-1] == ' ' || content[end-1] == '\t')) --end;
    while (end > 0 && content[end-1] == '#') --end;
    while (end > 0 && (content[end-1] == ' ' || content[end-1] == '\t')) --end;
    
    return content.substr(0, end);
}

/// Checks if line is an unordered list item
bool isUnorderedListItem(const std::string& line, std::string& content) {
    size_t i = 0;
    while (i < line.size() && line[i] == ' ' && i < 3) ++i;
    
    if (i >= line.size()) return false;
    
    char marker = line[i];
    if (marker != '-' && marker != '*' && marker != '+') return false;
    
    ++i;
    if (i >= line.size() || (line[i] != ' ' && line[i] != '\t')) return false;
    
    ++i;
    content = trim(line.substr(i));
    return true;
}

/// Checks if line is an ordered list item
bool isOrderedListItem(const std::string& line, std::string& content) {
    size_t i = 0;
    while (i < line.size() && line[i] == ' ' && i < 3) ++i;
    
    if (i >= line.size() || !std::isdigit(line[i])) return false;
    
    while (i < line.size() && std::isdigit(line[i])) ++i;
    
    if (i >= line.size() || (line[i] != '.' && line[i] != ')')) return false;
    ++i;
    
    if (i >= line.size() || (line[i] != ' ' && line[i] != '\t')) return false;
    ++i;
    
    content = trim(line.substr(i));
    return true;
}

/// Checks if line is a blockquote
bool isBlockquote(const std::string& line, std::string& content) {
    size_t i = 0;
    while (i < line.size() && line[i] == ' ' && i < 3) ++i;
    
    if (i >= line.size() || line[i] != '>') return false;
    ++i;
    
    // Optional space after >
    if (i < line.size() && line[i] == ' ') ++i;
    
    content = line.substr(i);
    return true;
}

} // anonymous namespace

// =============================================================================
// Public Methods
// =============================================================================

std::string FallbackRenderer::renderToHtml(const std::string& markdown) const {
    std::vector<Block> blocks = parseBlocks(markdown);
    
    std::string html;
    html.reserve(markdown.size() * 2);  // Estimate
    
    for (const Block& block : blocks) {
        html += renderBlock(block);
    }
    
    return html;
}

// =============================================================================
// Private Methods
// =============================================================================

std::vector<FallbackRenderer::Block> FallbackRenderer::parseBlocks(const std::string& markdown) const {
    std::vector<Block> blocks;
    std::istringstream stream(markdown);
    std::string line;
    
    std::string paragraphBuffer;
    bool inFencedCode = false;
    char fenceChar = '`';
    size_t fenceLen = 3;
    std::string codeLanguage;
    std::string codeBuffer;
    
    std::vector<std::string> listItems;
    bool inUnorderedList = false;
    bool inOrderedList = false;
    
    std::string blockquoteBuffer;
    bool inBlockquote = false;
    
    auto flushParagraph = [&]() {
        if (!paragraphBuffer.empty()) {
            Block block;
            block.type = Block::Type::Paragraph;
            block.content = trim(paragraphBuffer);
            if (!block.content.empty()) {
                blocks.push_back(std::move(block));
            }
            paragraphBuffer.clear();
        }
    };
    
    auto flushList = [&](bool ordered) {
        if (!listItems.empty()) {
            Block block;
            block.type = ordered ? Block::Type::OrderedList : Block::Type::UnorderedList;
            block.items = std::move(listItems);
            blocks.push_back(std::move(block));
            listItems.clear();
        }
    };
    
    auto flushBlockquote = [&]() {
        if (!blockquoteBuffer.empty()) {
            Block block;
            block.type = Block::Type::Blockquote;
            block.content = trim(blockquoteBuffer);
            blocks.push_back(std::move(block));
            blockquoteBuffer.clear();
        }
        inBlockquote = false;
    };
    
    while (std::getline(stream, line)) {
        // Handle fenced code blocks
        if (inFencedCode) {
            if (isFencedCodeEnd(line, fenceChar, fenceLen)) {
                Block block;
                block.type = Block::Type::FencedCode;
                block.language = codeLanguage;
                block.content = codeBuffer;
                blocks.push_back(std::move(block));
                codeBuffer.clear();
                inFencedCode = false;
            } else {
                if (!codeBuffer.empty()) codeBuffer += '\n';
                codeBuffer += line;
            }
            continue;
        }
        
        std::string tempLang;
        char tempFence;
        size_t tempLen;
        if (isFencedCodeStart(line, tempLang, tempFence, tempLen)) {
            flushParagraph();
            flushList(inOrderedList);
            inUnorderedList = inOrderedList = false;
            flushBlockquote();
            
            inFencedCode = true;
            fenceChar = tempFence;
            fenceLen = tempLen;
            codeLanguage = tempLang;
            codeBuffer.clear();
            continue;
        }
        
        // Horizontal rule
        if (isHorizontalRule(line)) {
            flushParagraph();
            flushList(inOrderedList);
            inUnorderedList = inOrderedList = false;
            flushBlockquote();
            
            Block block;
            block.type = Block::Type::HorizontalRule;
            blocks.push_back(std::move(block));
            continue;
        }
        
        // Heading
        int headingLevel = getHeadingLevel(line);
        if (headingLevel > 0) {
            flushParagraph();
            flushList(inOrderedList);
            inUnorderedList = inOrderedList = false;
            flushBlockquote();
            
            Block block;
            block.type = Block::Type::Heading;
            block.level = headingLevel;
            block.content = getHeadingContent(line);
            blocks.push_back(std::move(block));
            continue;
        }
        
        // Blockquote
        std::string quoteContent;
        if (isBlockquote(line, quoteContent)) {
            flushParagraph();
            flushList(inOrderedList);
            inUnorderedList = inOrderedList = false;
            
            if (!blockquoteBuffer.empty()) blockquoteBuffer += '\n';
            blockquoteBuffer += quoteContent;
            inBlockquote = true;
            continue;
        } else if (inBlockquote) {
            flushBlockquote();
        }
        
        // Unordered list
        std::string itemContent;
        if (isUnorderedListItem(line, itemContent)) {
            flushParagraph();
            if (inOrderedList) {
                flushList(true);
                inOrderedList = false;
            }
            inUnorderedList = true;
            listItems.push_back(itemContent);
            continue;
        }
        
        // Ordered list
        if (isOrderedListItem(line, itemContent)) {
            flushParagraph();
            if (inUnorderedList) {
                flushList(false);
                inUnorderedList = false;
            }
            inOrderedList = true;
            listItems.push_back(itemContent);
            continue;
        }
        
        // End of list on non-list line
        if (inUnorderedList || inOrderedList) {
            if (trim(line).empty()) {
                flushList(inOrderedList);
                inUnorderedList = inOrderedList = false;
                continue;
            }
        }
        
        // Blank line ends paragraph
        if (trim(line).empty()) {
            flushParagraph();
            flushList(inOrderedList);
            inUnorderedList = inOrderedList = false;
            continue;
        }
        
        // Regular paragraph text
        if (!paragraphBuffer.empty()) paragraphBuffer += '\n';
        paragraphBuffer += line;
    }
    
    // Flush remaining content
    if (inFencedCode) {
        // Unclosed code block - render what we have
        Block block;
        block.type = Block::Type::FencedCode;
        block.language = codeLanguage;
        block.content = codeBuffer;
        blocks.push_back(std::move(block));
    }
    
    flushParagraph();
    flushList(inOrderedList);
    flushBlockquote();
    
    return blocks;
}

std::string FallbackRenderer::renderBlock(const Block& block) const {
    switch (block.type) {
        case Block::Type::Heading: {
            std::string tag = "h" + std::to_string(block.level);
            return html::wrap(tag, processInline(block.content));
        }
        
        case Block::Type::Paragraph: {
            return html::wrap("p", processInline(block.content));
        }
        
        case Block::Type::FencedCode: {
            std::string result = "<pre><code";
            if (!block.language.empty()) {
                result += " class=\"language-";
                result += html::escape(block.language);
                result += '"';
            }
            result += '>';
            result += html::escape(block.content);
            result += "</code></pre>\n";
            return result;
        }
        
        case Block::Type::UnorderedList: {
            std::string result = "<ul>\n";
            for (const auto& item : block.items) {
                result += "  <li>" + processInline(item) + "</li>\n";
            }
            result += "</ul>\n";
            return result;
        }
        
        case Block::Type::OrderedList: {
            std::string result = "<ol>\n";
            for (const auto& item : block.items) {
                result += "  <li>" + processInline(item) + "</li>\n";
            }
            result += "</ol>\n";
            return result;
        }
        
        case Block::Type::Blockquote: {
            // Recursively parse blockquote content
            std::string innerHtml = renderToHtml(block.content);
            return "<blockquote>\n" + innerHtml + "</blockquote>\n";
        }
        
        case Block::Type::HorizontalRule: {
            return "<hr>\n";
        }
    }
    
    return "";
}

std::string FallbackRenderer::processInline(const std::string& text) const {
    std::string result;
    result.reserve(text.size() * 1.2);
    
    size_t i = 0;
    while (i < text.size()) {
        // Inline code (backticks)
        if (text[i] == '`') {
            size_t start = i + 1;
            size_t end = text.find('`', start);
            if (end != std::string::npos) {
                result += "<code>";
                result += html::escape(text.substr(start, end - start));
                result += "</code>";
                i = end + 1;
                continue;
            }
        }
        
        // Bold (**text**)
        if (i + 1 < text.size() && text[i] == '*' && text[i + 1] == '*') {
            size_t start = i + 2;
            size_t end = text.find("**", start);
            if (end != std::string::npos) {
                result += "<strong>";
                result += html::escape(text.substr(start, end - start));
                result += "</strong>";
                i = end + 2;
                continue;
            }
        }
        
        // Bold (__text__)
        if (i + 1 < text.size() && text[i] == '_' && text[i + 1] == '_') {
            size_t start = i + 2;
            size_t end = text.find("__", start);
            if (end != std::string::npos) {
                result += "<strong>";
                result += html::escape(text.substr(start, end - start));
                result += "</strong>";
                i = end + 2;
                continue;
            }
        }
        
        // Italic (*text*)
        if (text[i] == '*') {
            size_t start = i + 1;
            size_t end = text.find('*', start);
            if (end != std::string::npos && end > start) {
                result += "<em>";
                result += html::escape(text.substr(start, end - start));
                result += "</em>";
                i = end + 1;
                continue;
            }
        }
        
        // Italic (_text_)
        if (text[i] == '_') {
            size_t start = i + 1;
            size_t end = text.find('_', start);
            if (end != std::string::npos && end > start) {
                result += "<em>";
                result += html::escape(text.substr(start, end - start));
                result += "</em>";
                i = end + 1;
                continue;
            }
        }
        
        // Escape special HTML characters
        switch (text[i]) {
            case '&':  result += "&amp;";  break;
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            case '"':  result += "&quot;"; break;
            default:   result += text[i];  break;
        }
        ++i;
    }
    
    return result;
}

} // namespace mdeditor
