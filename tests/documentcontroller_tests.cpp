// =============================================================================
// documentcontroller_tests.cpp - Unit Tests for DocumentController
// =============================================================================
//
// Tests for the DocumentController QObject that bridges QML and C++.
// Uses GoogleTest with Qt integration using QCoreApplication.
//
// =============================================================================

#include <gtest/gtest.h>

#include "../src/mdapp/DocumentController.h"

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QDir>
#include <QFile>
#include <QTextStream>

// =============================================================================
// Test Fixture
// =============================================================================

class DocumentControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        controller = std::make_unique<mdeditor::DocumentController>();
    }

    void TearDown() override {
        controller.reset();
    }

    // Helper to create a temporary file with content
    QString createTempFile(const QString& content) {
        QTemporaryFile* file = new QTemporaryFile(this->tempDir.path() + "/test_XXXXXX.md");
        if (file->open()) {
            QTextStream out(file);
            out << content;
            file->close();
            tempFiles.append(file);
            return file->fileName();
        }
        return QString();
    }

    // Helper to read file content
    QString readFile(const QString& path) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            return in.readAll();
        }
        return QString();
    }

    std::unique_ptr<mdeditor::DocumentController> controller;
    QTemporaryDir tempDir;
    QList<QTemporaryFile*> tempFiles;
};

// =============================================================================
// Construction Tests
// =============================================================================

TEST_F(DocumentControllerTest, DefaultConstructor_CreatesEmptyDocument) {
    EXPECT_TRUE(controller->text().isEmpty());
    EXPECT_TRUE(controller->filePath().isEmpty());
    EXPECT_FALSE(controller->isModified());
}

TEST_F(DocumentControllerTest, DefaultConstructor_HasParser) {
    EXPECT_FALSE(controller->parserName().isEmpty());
}

// =============================================================================
// Text Property Tests
// =============================================================================

TEST_F(DocumentControllerTest, SetText_UpdatesText) {
    controller->setText("Hello, World!");
    EXPECT_EQ(controller->text(), "Hello, World!");
}

TEST_F(DocumentControllerTest, SetText_EmitsTextChanged) {
    QSignalSpy spy(controller.get(), &mdeditor::DocumentController::textChanged);
    controller->setText("New content");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(DocumentControllerTest, SetText_SameText_NoSignal) {
    controller->setText("Content");
    QSignalSpy spy(controller.get(), &mdeditor::DocumentController::textChanged);
    controller->setText("Content");
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(DocumentControllerTest, SetText_SetsModified) {
    // Initially not modified
    EXPECT_FALSE(controller->isModified());
    
    // After setting new text, should be modified
    controller->setText("New content");
    EXPECT_TRUE(controller->isModified());
}

TEST_F(DocumentControllerTest, SetText_Multiline) {
    QString multiline = "Line 1\nLine 2\nLine 3";
    controller->setText(multiline);
    EXPECT_EQ(controller->text(), multiline);
}

TEST_F(DocumentControllerTest, SetText_Unicode) {
    QString unicode = "Hello 世界 🌍 مرحبا";
    controller->setText(unicode);
    EXPECT_EQ(controller->text(), unicode);
}

// =============================================================================
// Load File Tests
// =============================================================================

TEST_F(DocumentControllerTest, LoadFile_LoadsContent) {
    QString path = createTempFile("# Test Markdown\n\nHello world!");
    ASSERT_FALSE(path.isEmpty());

    bool result = controller->loadFile(path);
    EXPECT_TRUE(result);
    EXPECT_EQ(controller->text(), "# Test Markdown\n\nHello world!");
}

TEST_F(DocumentControllerTest, LoadFile_SetsFilePath) {
    QString path = createTempFile("Content");
    ASSERT_FALSE(path.isEmpty());

    controller->loadFile(path);
    EXPECT_EQ(controller->filePath(), path);
}

TEST_F(DocumentControllerTest, LoadFile_ClearsModified) {
    controller->setText("Unsaved changes");
    EXPECT_TRUE(controller->isModified());

    QString path = createTempFile("Saved content");
    controller->loadFile(path);
    EXPECT_FALSE(controller->isModified());
}

TEST_F(DocumentControllerTest, LoadFile_EmitsSignals) {
    QString path = createTempFile("Content");
    ASSERT_FALSE(path.isEmpty());

    QSignalSpy textSpy(controller.get(), &mdeditor::DocumentController::textChanged);
    QSignalSpy pathSpy(controller.get(), &mdeditor::DocumentController::filePathChanged);

    controller->loadFile(path);

    EXPECT_GE(textSpy.count(), 1);
    EXPECT_GE(pathSpy.count(), 1);
}

TEST_F(DocumentControllerTest, LoadFile_NonExistent_ReturnsFalse) {
    bool result = controller->loadFile("/nonexistent/path/file.md");
    EXPECT_FALSE(result);
}

TEST_F(DocumentControllerTest, LoadFile_NonExistent_EmitsError) {
    QSignalSpy spy(controller.get(), &mdeditor::DocumentController::errorOccurred);
    controller->loadFile("/nonexistent/path/file.md");
    EXPECT_GE(spy.count(), 1);
}

// =============================================================================
// Save File Tests
// =============================================================================

TEST_F(DocumentControllerTest, SaveFile_SavesContent) {
    controller->setText("# Saved Content\n\nThis should be saved.");

    QString path = tempDir.path() + "/saved_file.md";
    bool result = controller->saveFile(path);

    EXPECT_TRUE(result);
    EXPECT_EQ(readFile(path), "# Saved Content\n\nThis should be saved.");
}

TEST_F(DocumentControllerTest, SaveFile_SetsFilePath) {
    controller->setText("Content");
    QString path = tempDir.path() + "/new_file.md";

    controller->saveFile(path);
    EXPECT_EQ(controller->filePath(), path);
}

TEST_F(DocumentControllerTest, SaveFile_ClearsModified) {
    controller->setText("Content");
    EXPECT_TRUE(controller->isModified());

    QString path = tempDir.path() + "/file.md";
    controller->saveFile(path);
    EXPECT_FALSE(controller->isModified());
}

TEST_F(DocumentControllerTest, SaveFile_InvalidPath_ReturnsFalse) {
    controller->setText("Content");
    
    // Try to save to a directory that doesn't exist
    bool result = controller->saveFile("/nonexistent/directory/file.md");
    EXPECT_FALSE(result);
}

TEST_F(DocumentControllerTest, SaveFile_InvalidPath_EmitsError) {
    controller->setText("Content");
    QSignalSpy spy(controller.get(), &mdeditor::DocumentController::errorOccurred);
    
    controller->saveFile("/nonexistent/directory/file.md");
    EXPECT_GE(spy.count(), 1);
}

// =============================================================================
// Render Tests
// =============================================================================

TEST_F(DocumentControllerTest, RenderToHtml_EmitsPreviewReady) {
    controller->setText("# Hello\n\nWorld");

    QSignalSpy spy(controller.get(), &mdeditor::DocumentController::previewReady);
    controller->renderToHtml();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(DocumentControllerTest, RenderToHtml_ProducesHtml) {
    controller->setText("# Heading\n\nParagraph text.");

    QSignalSpy spy(controller.get(), &mdeditor::DocumentController::previewReady);
    controller->renderToHtml();

    ASSERT_EQ(spy.count(), 1);
    QString html = spy.at(0).at(0).toString();
    
    // Check that HTML contains expected elements
    EXPECT_TRUE(html.contains("<h1>") || html.contains("<h1 ")) 
        << "HTML should contain h1 tag: " << html.toStdString();
    EXPECT_TRUE(html.contains("<p>") || html.contains("<p "))
        << "HTML should contain p tag: " << html.toStdString();
}

TEST_F(DocumentControllerTest, RenderToHtml_CodeBlock) {
    controller->setText("```cpp\nint main() { return 0; }\n```");

    QSignalSpy spy(controller.get(), &mdeditor::DocumentController::previewReady);
    controller->renderToHtml();

    ASSERT_EQ(spy.count(), 1);
    QString html = spy.at(0).at(0).toString();
    
    EXPECT_TRUE(html.contains("<pre>") || html.contains("<code>"))
        << "HTML should contain code block: " << html.toStdString();
}

TEST_F(DocumentControllerTest, RenderToHtml_List) {
    controller->setText("- Item 1\n- Item 2\n- Item 3");

    QSignalSpy spy(controller.get(), &mdeditor::DocumentController::previewReady);
    controller->renderToHtml();

    ASSERT_EQ(spy.count(), 1);
    QString html = spy.at(0).at(0).toString();
    
    EXPECT_TRUE(html.contains("<ul>") || html.contains("<li>"))
        << "HTML should contain list elements: " << html.toStdString();
}

// =============================================================================
// New Document Tests
// =============================================================================

TEST_F(DocumentControllerTest, NewDocument_ClearsText) {
    controller->setText("Existing content");
    controller->newDocument();
    EXPECT_TRUE(controller->text().isEmpty());
}

TEST_F(DocumentControllerTest, NewDocument_ClearsFilePath) {
    QString path = createTempFile("Content");
    controller->loadFile(path);
    EXPECT_FALSE(controller->filePath().isEmpty());

    controller->newDocument();
    EXPECT_TRUE(controller->filePath().isEmpty());
}

TEST_F(DocumentControllerTest, NewDocument_ClearsModified) {
    controller->setText("Modified content");
    EXPECT_TRUE(controller->isModified());

    controller->newDocument();
    EXPECT_FALSE(controller->isModified());
}

TEST_F(DocumentControllerTest, NewDocument_EmitsTextChanged) {
    controller->setText("Content");

    QSignalSpy spy(controller.get(), &mdeditor::DocumentController::textChanged);
    controller->newDocument();

    EXPECT_GE(spy.count(), 1);
}

// =============================================================================
// Round-Trip Tests
// =============================================================================

TEST_F(DocumentControllerTest, RoundTrip_SaveAndLoad) {
    QString original = "# Round Trip Test\n\n## Section 1\n\nParagraph content.\n\n## Section 2\n\n- List item\n";
    
    controller->setText(original);
    
    QString path = tempDir.path() + "/roundtrip.md";
    EXPECT_TRUE(controller->saveFile(path));
    
    // Create new controller and load the file
    auto controller2 = std::make_unique<mdeditor::DocumentController>();
    EXPECT_TRUE(controller2->loadFile(path));
    
    EXPECT_EQ(controller2->text(), original);
}

TEST_F(DocumentControllerTest, RoundTrip_LoadEditSave) {
    QString path = createTempFile("Original content");
    
    controller->loadFile(path);
    controller->setText("Modified content");
    controller->saveFile(path);
    
    // Verify file was updated
    EXPECT_EQ(readFile(path), "Modified content");
}

// =============================================================================
// Main (for Qt integration)
// =============================================================================

int main(int argc, char** argv) {
    // Qt requires QCoreApplication for signal/slot and file operations
    QCoreApplication app(argc, argv);
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
