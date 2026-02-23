// =============================================================================
// DocumentController.cpp - QML Document Bridge Implementation
// =============================================================================

#include "DocumentController.h"
#include "../gapbuffer/gap_buffer.h"
#include "../markdown/IMarkdownParser.h"

#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QFileInfo>

namespace mdeditor {

// =============================================================================
// Construction / Destruction
// =============================================================================

DocumentController::DocumentController(QObject* parent)
    : QObject(parent)
    , m_buffer(std::make_unique<GapBuffer>())
    , m_parser(createDefaultParser())
{
}

DocumentController::~DocumentController() = default;

// =============================================================================
// Property Accessors
// =============================================================================

QString DocumentController::text() const
{
    return QString::fromStdString(m_buffer->getText());
}

void DocumentController::setText(const QString& newText)
{
    const std::string newTextStd = newText.toStdString();
    if (m_buffer->getText() == newTextStd) {
        return;
    }

    m_buffer->loadFromString(newTextStd);
    emit textChanged();

    // Check if different from last saved
    setModified(newText != m_lastSavedText);
}

QString DocumentController::filePath() const
{
    return m_filePath;
}

bool DocumentController::isModified() const
{
    return m_modified;
}

QString DocumentController::parserName() const
{
    return QString::fromStdString(m_parser->parserName());
}

// =============================================================================
// File Operations
// =============================================================================

bool DocumentController::loadFile(const QString& path)
{
    QString localPath = path;

    // Handle URL format
    if (path.startsWith("file:///")) {
        QUrl url(path);
        localPath = url.toLocalFile();
    }

    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred(tr("Cannot open file: %1").arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString content = in.readAll();
    file.close();

    m_buffer->loadFromString(content.toStdString());
    m_lastSavedText = content;
    setFilePath(localPath);
    setModified(false);
    emit textChanged();

    return true;
}

bool DocumentController::loadFileUrl(const QUrl& url)
{
    return loadFile(url.toLocalFile());
}

bool DocumentController::saveFile(const QString& path)
{
    QString localPath = path;

    // Handle URL format
    if (path.startsWith("file:///")) {
        QUrl url(path);
        localPath = url.toLocalFile();
    }

    QFile file(localPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred(tr("Cannot save file: %1").arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    const QString content = QString::fromStdString(m_buffer->getText());
    out << content;
    file.close();

    m_lastSavedText = content;
    setFilePath(localPath);
    setModified(false);

    return true;
}

bool DocumentController::saveFileUrl(const QUrl& url)
{
    return saveFile(url.toLocalFile());
}

void DocumentController::renderToHtml()
{
    const std::string markdown = m_buffer->getText();
    const std::string html = m_parser->renderToHtml(markdown);
    emit previewReady(QString::fromStdString(html));
}

void DocumentController::newDocument()
{
    m_buffer->clear();
    m_lastSavedText.clear();
    setFilePath(QString());
    setModified(false);
    emit textChanged();
}

// =============================================================================
// Private Helpers
// =============================================================================

void DocumentController::setModified(bool modified)
{
    if (m_modified != modified) {
        m_modified = modified;
        emit modifiedChanged();
    }
}

void DocumentController::setFilePath(const QString& path)
{
    if (m_filePath != path) {
        m_filePath = path;
        emit filePathChanged();
    }
}

} // namespace mdeditor
