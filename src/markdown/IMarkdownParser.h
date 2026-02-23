// =============================================================================
// IMarkdownParser.h - Pluggable Markdown Parser Interface
// =============================================================================
//
// This interface defines the contract for markdown parsing implementations.
// The default implementation uses a fallback renderer that handles common
// markdown elements. When MD_USE_CMARK is enabled, CMarkAdapter provides
// full CommonMark compliance via the cmark library.
//
// USAGE:
// ------
//   #include "IMarkdownParser.h"
//   #include "fallback_renderer.h"  // or "cmark_adapter.h"
//
//   auto parser = mdeditor::createDefaultParser();
//   std::string html = parser->renderToHtml(markdownText);
//
// SUPPORTED ELEMENTS (Fallback):
// ------------------------------
//   - Headings (# through ######)
//   - Paragraphs
//   - Fenced code blocks (``` or ~~~)
//   - Unordered lists (-, *, +)
//   - Ordered lists (1., 2., etc.)
//   - Blockquotes (>)
//   - Horizontal rules (---, ***, ___)
//   - Inline formatting: **bold**, *italic*, `code`
//
// =============================================================================

#ifndef MDEDITOR_IMARKDOWN_PARSER_H
#define MDEDITOR_IMARKDOWN_PARSER_H

#include <memory>
#include <string>

namespace mdeditor {

// =============================================================================
// IMarkdownParser - Abstract interface for markdown rendering
// =============================================================================
/// Interface for pluggable markdown parsers.
/// Implementations convert markdown text to HTML.
class IMarkdownParser {
public:
    virtual ~IMarkdownParser() = default;

    /// Renders markdown text to HTML.
    /// @param markdown The markdown source text
    /// @return HTML representation of the markdown
    [[nodiscard]] virtual std::string renderToHtml(const std::string& markdown) const = 0;

    /// Returns the name of this parser implementation.
    /// @return Parser implementation name (e.g., "FallbackRenderer", "CMarkAdapter")
    [[nodiscard]] virtual std::string parserName() const = 0;

    /// Returns true if this parser supports the full CommonMark specification.
    [[nodiscard]] virtual bool isFullCommonMark() const = 0;
};

/// Creates the default markdown parser.
/// Returns CMarkAdapter if MD_USE_CMARK was enabled at build time,
/// otherwise returns FallbackRenderer.
[[nodiscard]] std::unique_ptr<IMarkdownParser> createDefaultParser();

/// Creates the fallback renderer (always available).
[[nodiscard]] std::unique_ptr<IMarkdownParser> createFallbackRenderer();

#ifdef MD_USE_CMARK
/// Creates the CMarkAdapter (only available when MD_USE_CMARK is enabled).
[[nodiscard]] std::unique_ptr<IMarkdownParser> createCMarkAdapter();
#endif

} // namespace mdeditor

#endif // MDEDITOR_IMARKDOWN_PARSER_H
