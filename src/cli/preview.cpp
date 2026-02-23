// =============================================================================
// preview.cpp - Markdown Preview CLI Tool
// =============================================================================
//
// This tool loads a markdown file, renders it to HTML, and writes the output
// to a preview file. It demonstrates the markdown parser integration.
//
// Usage: mdpreview [input.md] [output.html]
//        Default input:  samples/sample.md
//        Default output: out/preview.html
//
// =============================================================================

#include "gap_buffer.h"
#include "IMarkdownParser.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

/// HTML template for the preview page
constexpr const char* HTML_TEMPLATE = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Markdown Preview</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 
                         Oxygen, Ubuntu, Cantarell, sans-serif;
            line-height: 1.6;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            color: #333;
            background-color: #fff;
        }
        h1, h2, h3, h4, h5, h6 {
            margin-top: 1.5em;
            margin-bottom: 0.5em;
            color: #222;
        }
        h1 { border-bottom: 2px solid #eee; padding-bottom: 0.3em; }
        h2 { border-bottom: 1px solid #eee; padding-bottom: 0.3em; }
        code {
            background-color: #f4f4f4;
            padding: 0.2em 0.4em;
            border-radius: 3px;
            font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace;
            font-size: 0.9em;
        }
        pre {
            background-color: #f6f8fa;
            padding: 16px;
            border-radius: 6px;
            overflow-x: auto;
        }
        pre code {
            background-color: transparent;
            padding: 0;
            font-size: 0.85em;
            line-height: 1.45;
        }
        blockquote {
            border-left: 4px solid #dfe2e5;
            margin: 0;
            padding-left: 16px;
            color: #6a737d;
        }
        ul, ol {
            padding-left: 2em;
        }
        li {
            margin: 0.25em 0;
        }
        hr {
            border: none;
            border-top: 1px solid #eee;
            margin: 2em 0;
        }
        a {
            color: #0366d6;
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }
        em {
            font-style: italic;
        }
        strong {
            font-weight: 600;
        }
        /* Syntax highlighting classes */
        .language-cpp, .language-c, .language-python, .language-js,
        .language-javascript, .language-rust, .language-go {
            color: #24292e;
        }
    </style>
</head>
<body>
<!-- CONTENT_PLACEHOLDER -->
</body>
</html>
)";

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

/// Writes content to a file
void writeFile(const fs::path& path, const std::string& content) {
    // Create parent directories if needed
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }
    
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot write file: " + path.string());
    }
    file << content;
}

/// Generates a complete HTML page with the rendered content
std::string generateHtmlPage(const std::string& renderedContent) {
    std::string html = HTML_TEMPLATE;
    const std::string placeholder = "<!-- CONTENT_PLACEHOLDER -->";
    
    size_t pos = html.find(placeholder);
    if (pos != std::string::npos) {
        html.replace(pos, placeholder.length(), renderedContent);
    }
    
    return html;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [input.md] [output.html]\n";
    std::cout << "\n";
    std::cout << "Arguments:\n";
    std::cout << "  input.md     Markdown file to render (default: samples/sample.md)\n";
    std::cout << "  output.html  Output HTML file (default: out/preview.html)\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << "\n";
    std::cout << "  " << programName << " README.md\n";
    std::cout << "  " << programName << " doc.md doc.html\n";
}

int main(int argc, char* argv[]) {
    try {
        // Parse arguments
        fs::path inputPath = "samples/sample.md";
        fs::path outputPath = "out/preview.html";
        
        if (argc > 1) {
            std::string arg1 = argv[1];
            if (arg1 == "-h" || arg1 == "--help") {
                printUsage(argv[0]);
                return 0;
            }
            inputPath = arg1;
        }
        if (argc > 2) {
            outputPath = argv[2];
        }
        
        std::cout << "Markdown Preview Generator\n";
        std::cout << "==========================\n\n";
        
        // Create parser
        auto parser = mdeditor::createDefaultParser();
        std::cout << "Parser: " << parser->parserName();
        if (parser->isFullCommonMark()) {
            std::cout << " (Full CommonMark)";
        }
        std::cout << "\n\n";
        
        // Check input file
        if (!fs::exists(inputPath)) {
            std::cerr << "Error: Input file not found: " << inputPath << "\n";
            return 1;
        }
        
        std::cout << "Input:  " << inputPath << "\n";
        std::cout << "Output: " << outputPath << "\n\n";
        
        // Load file into GapBuffer
        std::cout << "Loading markdown file...\n";
        std::string content = readFile(inputPath);
        
        mdeditor::GapBuffer buffer;
        buffer.loadFromString(content);
        
        std::cout << "  Size: " << buffer.length() << " bytes\n";
        std::cout << "  Lines: " << buffer.lineCount() << "\n\n";
        
        // Render to HTML
        std::cout << "Rendering to HTML...\n";
        std::string markdown = buffer.getText();
        std::string renderedHtml = parser->renderToHtml(markdown);
        
        std::cout << "  Rendered HTML: " << renderedHtml.size() << " bytes\n\n";
        
        // Generate complete HTML page
        std::string fullPage = generateHtmlPage(renderedHtml);
        
        // Write output
        std::cout << "Writing output file...\n";
        writeFile(outputPath, fullPage);
        
        std::cout << "  Written: " << fullPage.size() << " bytes\n\n";
        
        std::cout << "Done! Open " << outputPath << " in a browser to view.\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
