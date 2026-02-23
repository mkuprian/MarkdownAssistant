// =============================================================================
// cmark_adapter.h - CommonMark Library Adapter
// =============================================================================
//
// This adapter wraps the cmark (CommonMark reference implementation) library
// to provide full CommonMark-compliant markdown rendering.
//
// AVAILABILITY:
// -------------
// This file is only compiled when MD_USE_CMARK is enabled via CMake:
//   cmake -S . -B build -D MD_USE_CMARK=ON
//
// FEATURES:
// ---------
// When using cmark, you get full CommonMark spec compliance including:
//   - All block and inline elements
//   - Proper nesting and edge cases
//   - Reference links and images
//   - Tables (with cmark-gfm)
//
// =============================================================================

#ifndef MDEDITOR_CMARK_ADAPTER_H
#define MDEDITOR_CMARK_ADAPTER_H

#include "IMarkdownParser.h"

#include <string>

namespace mdeditor {

/// CMarkAdapter wraps the cmark CommonMark library for full-spec rendering.
/// Only available when MD_USE_CMARK is enabled at build time.
class CMarkAdapter : public IMarkdownParser {
public:
    /// Constructor with optional rendering options.
    /// @param options cmark options (default: CMARK_OPT_DEFAULT)
    explicit CMarkAdapter(int options = 0);
    
    ~CMarkAdapter() override = default;

    /// Renders markdown to HTML using cmark.
    [[nodiscard]] std::string renderToHtml(const std::string& markdown) const override;

    /// Returns "CMarkAdapter".
    [[nodiscard]] std::string parserName() const override { return "CMarkAdapter"; }

    /// Returns true (full CommonMark compliance).
    [[nodiscard]] bool isFullCommonMark() const override { return true; }

private:
    int options_;
};

} // namespace mdeditor

#endif // MDEDITOR_CMARK_ADAPTER_H
