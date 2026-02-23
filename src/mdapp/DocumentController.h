// =============================================================================
// DocumentController.h - QML Document Bridge
// =============================================================================
//
// This QObject provides the bridge between QML and the C++ document model.
// It wraps a GapBuffer for text storage and uses the markdown parser for
// rendering previews.
//
// USAGE (from QML):
// -----------------
//   DocumentController {
//       id: doc
//       onPreviewReady: (html) => previewText.text = html
//   }
//   TextArea { text: doc.text; onTextChanged: doc.text = text }
//   Button { onClicked: doc.loadFile("path/to/file.md") }
//   Button { onClicked: doc.saveFile("path/to/file.md") }
//   Button { onClicked: doc.renderToHtml() }
//
// =============================================================================

#ifndef MDEDITOR_DOCUMENT_CONTROLLER_H
#define MDEDITOR_DOCUMENT_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QUrl>

#include <memory>

namespace mdeditor {

class GapBuffer;
class IMarkdownParser;

/// DocumentController - QML bridge for document editing and markdown preview.
///
/// This controller wraps the GapBuffer text model and markdown parser,
/// exposing them to QML through properties and invokable methods.
class DocumentController : public QObject {
    Q_OBJECT

    /// The document text content (read/write from QML).
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

    /// The file path of the currently loaded document.
    Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)

    /// True if the document has unsaved changes.
    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)

    /// The name of the parser being used.
    Q_PROPERTY(QString parserName READ parserName CONSTANT)

public:
    /// Constructs a DocumentController with default parser.
    explicit DocumentController(QObject* parent = nullptr);

    /// Destructor.
    ~DocumentController() override;

    // -------------------------------------------------------------------------
    // Property Accessors
    // -------------------------------------------------------------------------

    /// Returns the current document text.
    [[nodiscard]] QString text() const;

    /// Sets the document text.
    void setText(const QString& newText);

    /// Returns the current file path, or empty string if no file is loaded.
    [[nodiscard]] QString filePath() const;

    /// Returns true if the document has unsaved modifications.
    [[nodiscard]] bool isModified() const;

    /// Returns the name of the markdown parser implementation.
    [[nodiscard]] QString parserName() const;

    // -------------------------------------------------------------------------
    // Invokable Methods (callable from QML)
    // -------------------------------------------------------------------------

    /// Loads a file from the given path.
    /// @param path File path (can be URL or local path)
    /// @return true if load succeeded
    Q_INVOKABLE bool loadFile(const QString& path);

    /// Loads a file from a URL (for file dialogs).
    /// @param url File URL
    /// @return true if load succeeded
    Q_INVOKABLE bool loadFileUrl(const QUrl& url);

    /// Saves the document to the given path.
    /// @param path File path (can be URL or local path)
    /// @return true if save succeeded
    Q_INVOKABLE bool saveFile(const QString& path);

    /// Saves the document to a URL (for file dialogs).
    /// @param url File URL
    /// @return true if save succeeded
    Q_INVOKABLE bool saveFileUrl(const QUrl& url);

    /// Renders the current document to HTML and emits previewReady.
    Q_INVOKABLE void renderToHtml();

    /// Creates a new empty document.
    Q_INVOKABLE void newDocument();

signals:
    /// Emitted when the text content changes.
    void textChanged();

    /// Emitted when the file path changes.
    void filePathChanged();

    /// Emitted when the modified state changes.
    void modifiedChanged();

    /// Emitted when HTML preview is ready.
    /// @param html The rendered HTML content
    void previewReady(const QString& html);

    /// Emitted when an error occurs.
    /// @param message Error description
    void errorOccurred(const QString& message);

private:
    std::unique_ptr<GapBuffer> m_buffer;
    std::unique_ptr<IMarkdownParser> m_parser;
    QString m_filePath;
    bool m_modified = false;
    QString m_lastSavedText;

    void setModified(bool modified);
    void setFilePath(const QString& path);
};

} // namespace mdeditor

#endif // MDEDITOR_DOCUMENT_CONTROLLER_H
