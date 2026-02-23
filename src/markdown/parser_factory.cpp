// =============================================================================
// parser_factory.cpp - Parser Factory Implementation
// =============================================================================
//
// Provides factory functions for creating markdown parser instances.
//
// =============================================================================

#include "IMarkdownParser.h"
#include "fallback_renderer.h"

#ifdef MD_USE_CMARK
#include "cmark_adapter.h"
#endif

namespace mdeditor {

std::unique_ptr<IMarkdownParser> createFallbackRenderer() {
    return std::make_unique<FallbackRenderer>();
}

#ifdef MD_USE_CMARK
std::unique_ptr<IMarkdownParser> createCMarkAdapter() {
    return std::make_unique<CMarkAdapter>();
}
#endif

std::unique_ptr<IMarkdownParser> createDefaultParser() {
#ifdef MD_USE_CMARK
    return createCMarkAdapter();
#else
    return createFallbackRenderer();
#endif
}

} // namespace mdeditor
