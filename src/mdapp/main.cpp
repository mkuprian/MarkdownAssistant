// =============================================================================
// main.cpp - Markdown Editor Application Entry Point
// =============================================================================
//
// This file initializes the Qt application, registers custom QML types,
// and loads the main QML interface.
//
// USAGE:
// ------
//   mdapp                     # Start with empty document
//   mdapp path/to/file.md     # Open a specific file
//
// =============================================================================

#include "DocumentController.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDir>

#ifdef MD_USE_WEBENGINE
#include <QtWebEngineQuick/QtWebEngineQuick>
#endif

int main(int argc, char* argv[])
{
    // -------------------------------------------------------------------------
    // Initialize WebEngine if available (must be done BEFORE QGuiApplication)
    // -------------------------------------------------------------------------
#ifdef MD_USE_WEBENGINE
    QtWebEngineQuick::initialize();
#endif

    // -------------------------------------------------------------------------
    // Create Application
    // -------------------------------------------------------------------------
    QGuiApplication app(argc, argv);

    // Set application metadata
    app.setOrganizationName("mdeditor");
    app.setOrganizationDomain("mdeditor.local");
    app.setApplicationName("Markdown Editor");
    app.setApplicationVersion("0.1.0");

    // -------------------------------------------------------------------------
    // Register Custom QML Types
    // -------------------------------------------------------------------------
    // Register DocumentController to be instantiable from QML
    qmlRegisterType<mdeditor::DocumentController>("MdEditor", 1, 0, "DocumentController");

    // -------------------------------------------------------------------------
    // Create QML Engine and Load Main Window
    // -------------------------------------------------------------------------
    QQmlApplicationEngine engine;

    // Add import path for local QML modules
    engine.addImportPath(QDir::currentPath() + "/qml");

    // Load main QML file
    const QUrl mainUrl(QStringLiteral("qrc:/qml/Main.qml"));

    // Handle QML loading errors
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
    );

    engine.load(mainUrl);

    // -------------------------------------------------------------------------
    // Run Event Loop
    // -------------------------------------------------------------------------
    return app.exec();
}
