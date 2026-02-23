// =============================================================================
// html_utils.h - HTML Utility Functions
// =============================================================================
//
// Common utilities for HTML generation including escaping and formatting.
//
// =============================================================================

#ifndef MDEDITOR_HTML_UTILS_H
#define MDEDITOR_HTML_UTILS_H

#include <string>
#include <string_view>

namespace mdeditor {
namespace html {

/// Escapes HTML special characters to prevent XSS and rendering issues.
/// Converts: & < > " '
/// @param text The text to escape
/// @return HTML-escaped text
[[nodiscard]] inline std::string escape(std::string_view text) {
    std::string result;
    result.reserve(text.size() + text.size() / 8);  // Estimate extra space

    for (char c : text) {
        switch (c) {
            case '&':  result += "&amp;";  break;
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            case '"':  result += "&quot;"; break;
            case '\'': result += "&#39;";  break;
            default:   result += c;        break;
        }
    }

    return result;
}

/// Wraps content in an HTML tag.
/// @param tag The tag name (e.g., "p", "h1")
/// @param content The content to wrap (already escaped if needed)
/// @param className Optional CSS class
/// @return HTML element string
[[nodiscard]] inline std::string wrap(
    std::string_view tag,
    std::string_view content,
    std::string_view className = ""
) {
    std::string result;
    result.reserve(tag.size() * 2 + content.size() + className.size() + 10);
    
    result += '<';
    result += tag;
    if (!className.empty()) {
        result += " class=\"";
        result += className;
        result += '"';
    }
    result += '>';
    result += content;
    result += "</";
    result += tag;
    result += ">\n";
    
    return result;
}

} // namespace html
} // namespace mdeditor

#endif // MDEDITOR_HTML_UTILS_H
