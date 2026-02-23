// =============================================================================
// cmark_adapter.cpp - CommonMark Library Adapter Implementation
// =============================================================================
//
// This file is only compiled when MD_USE_CMARK is enabled.
//
// =============================================================================

#include "cmark_adapter.h"

#include <cmark.h>
#include <cstdlib>
#include <stdexcept>

namespace mdeditor {

CMarkAdapter::CMarkAdapter(int options)
    : options_(options) {
    // Use safe defaults if no options specified
    if (options_ == 0) {
        options_ = CMARK_OPT_DEFAULT;
    }
}

std::string CMarkAdapter::renderToHtml(const std::string& markdown) const {
    // Parse markdown to AST
    cmark_node* document = cmark_parse_document(
        markdown.data(),
        markdown.size(),
        options_
    );
    
    if (!document) {
        throw std::runtime_error("cmark failed to parse markdown document");
    }
    
    // Render AST to HTML
    char* html = cmark_render_html(document, options_);
    
    // Free the AST
    cmark_node_free(document);
    
    if (!html) {
        throw std::runtime_error("cmark failed to render HTML");
    }
    
    // Copy to std::string and free cmark's allocation
    std::string result(html);
    std::free(html);
    
    return result;
}

} // namespace mdeditor
