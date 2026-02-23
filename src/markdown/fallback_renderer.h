// =============================================================================
// fallback_renderer.h - Fallback Markdown Renderer
// =============================================================================
//
// A simple markdown renderer that handles common elements without external
// dependencies. This is always available regardless of build configuration.
//
// SUPPORTED ELEMENTS:
// -------------------
//   - ATX Headings: # H1 through ###### H6
//   - Paragraphs: Blank-line separated text blocks
//   - Fenced code blocks: ``` or ~~~ with optional language
//   - Unordered lists: -, *, + prefixed items
//   - Ordered lists: 1., 2., etc. prefixed items
//   - Blockquotes: > prefixed lines
//   - Horizontal rules: ---, ***, ___
//   - Inline: **bold**, *italic*, `code`
//
// LIMITATIONS:
// ------------
//   - No nested lists (single level only)
//   - No reference-style links
//   - No tables
//   - Simplified inline parsing
//
// NEXT STEPS:
// ------------
// - add support for nested lists and blockquotes
// - add table support
//
// =============================================================================

#ifndef MDEDITOR_FALLBACK_RENDERER_H
#define MDEDITOR_FALLBACK_RENDERER_H

#include "IMarkdownParser.h"

#include <string>
#include <vector>

namespace mdeditor {

/// FallbackRenderer provides basic markdown-to-HTML conversion.
/// Use this when cmark is not available or for lightweight rendering.
class FallbackRenderer : public IMarkdownParser {
public:
    FallbackRenderer() = default;
    ~FallbackRenderer() override = default;

    /// Renders markdown to HTML.
    [[nodiscard]] std::string renderToHtml(const std::string& markdown) const override;

    /// Returns "FallbackRenderer".
    [[nodiscard]] std::string parserName() const override { return "FallbackRenderer"; }

    /// Returns false (not full CommonMark).
    [[nodiscard]] bool isFullCommonMark() const override { return false; }

private:
    /// Represents a parsed block element
    struct Block {
        enum class Type {
            Paragraph,
            Heading,
            FencedCode,
            UnorderedList,
            OrderedList,
            Blockquote,
            HorizontalRule
        };

        Type type;
        int level;              // For headings (1-6) or list nesting
        std::string content;    // Raw content
        std::string language;   // For code blocks
        std::vector<std::string> items;  // For lists
    };

    /// Parses markdown into blocks
    [[nodiscard]] std::vector<Block> parseBlocks(const std::string& markdown) const;

    /// Renders a single block to HTML
    [[nodiscard]] std::string renderBlock(const Block& block) const;

    /// Processes inline formatting (bold, italic, code)
    [[nodiscard]] std::string processInline(const std::string& text) const;
};

} // namespace mdeditor

#endif // MDEDITOR_FALLBACK_RENDERER_H
