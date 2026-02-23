// =============================================================================
// markdown_tests.cpp - Unit Tests for Markdown Parser
// =============================================================================
//
// Tests for the markdown-to-HTML rendering functionality including:
// - Headings (h1-h6)
// - Paragraphs
// - Fenced code blocks
// - Unordered and ordered lists
// - Blockquotes
// - Horizontal rules
// - Inline formatting (bold, italic, code)
// - HTML escaping
//
// =============================================================================

#include <gtest/gtest.h>
#include "IMarkdownParser.h"
#include "fallback_renderer.h"
#include "html_utils.h"

using namespace mdeditor;

// =============================================================================
// Test Fixture
// =============================================================================

class MarkdownParserTest : public ::testing::Test {
protected:
    std::unique_ptr<IMarkdownParser> parser;

    void SetUp() override {
        // Use the fallback renderer for consistent testing
        parser = createFallbackRenderer();
    }
};

// =============================================================================
// Parser Creation Tests
// =============================================================================

TEST_F(MarkdownParserTest, CreateDefaultParser_ReturnsValidParser) {
    auto defaultParser = createDefaultParser();
    ASSERT_NE(defaultParser, nullptr);
    EXPECT_FALSE(defaultParser->parserName().empty());
}

TEST_F(MarkdownParserTest, CreateFallbackRenderer_ReturnsFallback) {
    auto fallback = createFallbackRenderer();
    ASSERT_NE(fallback, nullptr);
    EXPECT_EQ(fallback->parserName(), "FallbackRenderer");
    EXPECT_FALSE(fallback->isFullCommonMark());
}

// =============================================================================
// Heading Tests
// =============================================================================

TEST_F(MarkdownParserTest, Heading_H1) {
    std::string html = parser->renderToHtml("# Heading 1");
    EXPECT_NE(html.find("<h1>"), std::string::npos);
    EXPECT_NE(html.find("Heading 1"), std::string::npos);
    EXPECT_NE(html.find("</h1>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Heading_H2) {
    std::string html = parser->renderToHtml("## Heading 2");
    EXPECT_NE(html.find("<h2>"), std::string::npos);
    EXPECT_NE(html.find("Heading 2"), std::string::npos);
    EXPECT_NE(html.find("</h2>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Heading_H3) {
    std::string html = parser->renderToHtml("### Heading 3");
    EXPECT_NE(html.find("<h3>"), std::string::npos);
    EXPECT_NE(html.find("</h3>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Heading_H4) {
    std::string html = parser->renderToHtml("#### Heading 4");
    EXPECT_NE(html.find("<h4>"), std::string::npos);
    EXPECT_NE(html.find("</h4>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Heading_H5) {
    std::string html = parser->renderToHtml("##### Heading 5");
    EXPECT_NE(html.find("<h5>"), std::string::npos);
    EXPECT_NE(html.find("</h5>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Heading_H6) {
    std::string html = parser->renderToHtml("###### Heading 6");
    EXPECT_NE(html.find("<h6>"), std::string::npos);
    EXPECT_NE(html.find("</h6>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Heading_WithTrailingHashes) {
    std::string html = parser->renderToHtml("## Heading ##");
    EXPECT_NE(html.find("<h2>"), std::string::npos);
    EXPECT_NE(html.find("Heading"), std::string::npos);
    // Should not contain trailing hashes in rendered content
    EXPECT_EQ(html.find("##"), std::string::npos);
}

TEST_F(MarkdownParserTest, Heading_MultipleHeadings) {
    std::string markdown = "# First\n\n## Second\n\n### Third";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<h1>"), std::string::npos);
    EXPECT_NE(html.find("<h2>"), std::string::npos);
    EXPECT_NE(html.find("<h3>"), std::string::npos);
}

// =============================================================================
// Paragraph Tests
// =============================================================================

TEST_F(MarkdownParserTest, Paragraph_SingleLine) {
    std::string html = parser->renderToHtml("This is a paragraph.");
    EXPECT_NE(html.find("<p>"), std::string::npos);
    EXPECT_NE(html.find("This is a paragraph."), std::string::npos);
    EXPECT_NE(html.find("</p>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Paragraph_MultipleLines) {
    std::string markdown = "First paragraph.\n\nSecond paragraph.";
    std::string html = parser->renderToHtml(markdown);
    
    // Should have two paragraphs
    size_t firstP = html.find("<p>");
    ASSERT_NE(firstP, std::string::npos);
    size_t secondP = html.find("<p>", firstP + 1);
    EXPECT_NE(secondP, std::string::npos);
}

// =============================================================================
// Fenced Code Block Tests
// =============================================================================

TEST_F(MarkdownParserTest, FencedCode_BasicBackticks) {
    std::string markdown = "```\ncode here\n```";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<pre>"), std::string::npos);
    EXPECT_NE(html.find("<code>"), std::string::npos);
    EXPECT_NE(html.find("code here"), std::string::npos);
    EXPECT_NE(html.find("</code>"), std::string::npos);
    EXPECT_NE(html.find("</pre>"), std::string::npos);
}

TEST_F(MarkdownParserTest, FencedCode_WithLanguage) {
    std::string markdown = "```cpp\nint main() {}\n```";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<pre>"), std::string::npos);
    EXPECT_NE(html.find("<code"), std::string::npos);
    EXPECT_NE(html.find("language-cpp"), std::string::npos);
    EXPECT_NE(html.find("int main()"), std::string::npos);
}

TEST_F(MarkdownParserTest, FencedCode_Tildes) {
    std::string markdown = "~~~\ncode with tildes\n~~~";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<pre>"), std::string::npos);
    EXPECT_NE(html.find("<code>"), std::string::npos);
    EXPECT_NE(html.find("code with tildes"), std::string::npos);
}

TEST_F(MarkdownParserTest, FencedCode_HtmlEscaping) {
    std::string markdown = "```\n<script>alert('xss')</script>\n```";
    std::string html = parser->renderToHtml(markdown);
    
    // Should escape HTML characters
    EXPECT_NE(html.find("&lt;script&gt;"), std::string::npos);
    EXPECT_EQ(html.find("<script>"), std::string::npos);
}

TEST_F(MarkdownParserTest, FencedCode_PreservesIndentation) {
    std::string markdown = "```\n  indented\n    more\n```";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("  indented"), std::string::npos);
    EXPECT_NE(html.find("    more"), std::string::npos);
}

// =============================================================================
// Unordered List Tests
// =============================================================================

TEST_F(MarkdownParserTest, UnorderedList_DashMarker) {
    std::string markdown = "- Item 1\n- Item 2\n- Item 3";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<ul>"), std::string::npos);
    EXPECT_NE(html.find("<li>Item 1</li>"), std::string::npos);
    EXPECT_NE(html.find("<li>Item 2</li>"), std::string::npos);
    EXPECT_NE(html.find("<li>Item 3</li>"), std::string::npos);
    EXPECT_NE(html.find("</ul>"), std::string::npos);
}

TEST_F(MarkdownParserTest, UnorderedList_AsteriskMarker) {
    std::string markdown = "* First\n* Second";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<ul>"), std::string::npos);
    EXPECT_NE(html.find("<li>First</li>"), std::string::npos);
    EXPECT_NE(html.find("<li>Second</li>"), std::string::npos);
}

TEST_F(MarkdownParserTest, UnorderedList_PlusMarker) {
    std::string markdown = "+ Alpha\n+ Beta";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<ul>"), std::string::npos);
    EXPECT_NE(html.find("<li>Alpha</li>"), std::string::npos);
}

// =============================================================================
// Ordered List Tests
// =============================================================================

TEST_F(MarkdownParserTest, OrderedList_Basic) {
    std::string markdown = "1. First\n2. Second\n3. Third";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<ol>"), std::string::npos);
    EXPECT_NE(html.find("<li>First</li>"), std::string::npos);
    EXPECT_NE(html.find("<li>Second</li>"), std::string::npos);
    EXPECT_NE(html.find("<li>Third</li>"), std::string::npos);
    EXPECT_NE(html.find("</ol>"), std::string::npos);
}

TEST_F(MarkdownParserTest, OrderedList_WithParenthesis) {
    std::string markdown = "1) One\n2) Two";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<ol>"), std::string::npos);
    EXPECT_NE(html.find("<li>One</li>"), std::string::npos);
}

// =============================================================================
// Blockquote Tests
// =============================================================================

TEST_F(MarkdownParserTest, Blockquote_SingleLine) {
    std::string markdown = "> This is a quote.";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<blockquote>"), std::string::npos);
    EXPECT_NE(html.find("This is a quote."), std::string::npos);
    EXPECT_NE(html.find("</blockquote>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Blockquote_MultiLine) {
    std::string markdown = "> Line 1\n> Line 2";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<blockquote>"), std::string::npos);
    EXPECT_NE(html.find("Line 1"), std::string::npos);
    EXPECT_NE(html.find("Line 2"), std::string::npos);
}

// =============================================================================
// Horizontal Rule Tests
// =============================================================================

TEST_F(MarkdownParserTest, HorizontalRule_Dashes) {
    std::string markdown = "---";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<hr>"), std::string::npos);
}

TEST_F(MarkdownParserTest, HorizontalRule_Asterisks) {
    std::string markdown = "***";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<hr>"), std::string::npos);
}

TEST_F(MarkdownParserTest, HorizontalRule_Underscores) {
    std::string markdown = "___";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<hr>"), std::string::npos);
}

// =============================================================================
// Inline Formatting Tests
// =============================================================================

TEST_F(MarkdownParserTest, Inline_Bold_Asterisks) {
    std::string markdown = "This is **bold** text.";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<strong>bold</strong>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Inline_Bold_Underscores) {
    std::string markdown = "This is __bold__ text.";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<strong>bold</strong>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Inline_Italic_Asterisk) {
    std::string markdown = "This is *italic* text.";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<em>italic</em>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Inline_Italic_Underscore) {
    std::string markdown = "This is _italic_ text.";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<em>italic</em>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Inline_Code) {
    std::string markdown = "Use `printf()` function.";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<code>printf()</code>"), std::string::npos);
}

TEST_F(MarkdownParserTest, Inline_CodeEscaping) {
    std::string markdown = "Use `<tag>` in code.";
    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<code>&lt;tag&gt;</code>"), std::string::npos);
}

// =============================================================================
// HTML Utils Tests
// =============================================================================

TEST(HtmlUtilsTest, Escape_Ampersand) {
    EXPECT_EQ(html::escape("&"), "&amp;");
}

TEST(HtmlUtilsTest, Escape_LessThan) {
    EXPECT_EQ(html::escape("<"), "&lt;");
}

TEST(HtmlUtilsTest, Escape_GreaterThan) {
    EXPECT_EQ(html::escape(">"), "&gt;");
}

TEST(HtmlUtilsTest, Escape_Quote) {
    EXPECT_EQ(html::escape("\""), "&quot;");
}

TEST(HtmlUtilsTest, Escape_Apostrophe) {
    EXPECT_EQ(html::escape("'"), "&#39;");
}

TEST(HtmlUtilsTest, Escape_Mixed) {
    EXPECT_EQ(html::escape("<div class=\"test\">&nbsp;</div>"),
              "&lt;div class=&quot;test&quot;&gt;&amp;nbsp;&lt;/div&gt;");
}

TEST(HtmlUtilsTest, Escape_NoSpecialChars) {
    EXPECT_EQ(html::escape("Hello World"), "Hello World");
}

TEST(HtmlUtilsTest, Wrap_Basic) {
    EXPECT_EQ(html::wrap("p", "content"), "<p>content</p>\n");
}

TEST(HtmlUtilsTest, Wrap_WithClass) {
    std::string result = html::wrap("div", "content", "my-class");
    EXPECT_EQ(result, "<div class=\"my-class\">content</div>\n");
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_F(MarkdownParserTest, EmptyInput) {
    std::string html = parser->renderToHtml("");
    EXPECT_TRUE(html.empty() || html.find_first_not_of(" \n\t") == std::string::npos);
}

TEST_F(MarkdownParserTest, OnlyWhitespace) {
    std::string html = parser->renderToHtml("   \n\n   ");
    EXPECT_TRUE(html.empty() || html.find("<p>") == std::string::npos);
}

TEST_F(MarkdownParserTest, MixedContent) {
    std::string markdown = R"(# Title

This is a paragraph with **bold** and *italic*.

```cpp
int x = 42;
```

- List item 1
- List item 2

> A quote

---

End.)";

    std::string html = parser->renderToHtml(markdown);
    
    EXPECT_NE(html.find("<h1>"), std::string::npos);
    EXPECT_NE(html.find("<p>"), std::string::npos);
    EXPECT_NE(html.find("<strong>"), std::string::npos);
    EXPECT_NE(html.find("<em>"), std::string::npos);
    EXPECT_NE(html.find("<pre>"), std::string::npos);
    EXPECT_NE(html.find("<code"), std::string::npos);
    EXPECT_NE(html.find("<ul>"), std::string::npos);
    EXPECT_NE(html.find("<blockquote>"), std::string::npos);
    EXPECT_NE(html.find("<hr>"), std::string::npos);
}
