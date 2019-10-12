#include "logwindow.h"

#include <QFontDatabase>

#include <1541img/log.h>

static const QString levels[] = {
    QString("[FATAL] %1"),
    QString("[ERROR] %1"),
    QString("[WARN ] %1"),
    QString("[INFO ] %1"),
    QString("[DEBUG] %1")
};

static void appendlog(LogLevel level, const char *message, void *data)
{
    LogWindow *logWindow = static_cast<LogWindow *>(data);
    logWindow->appendPlainText(levels[level].arg(message));
}

LogWindow::LogWindow(QWidget *parent) :
    QPlainTextEdit(parent)
{
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    QFontMetricsF fm(font());
    setMinimumWidth(82 * fm.averageCharWidth());
    setReadOnly(true);
    setMaximumBlockCount(1024);
    setWindowTitle(tr("lib1541img log viewer"));
    setCustomLogger(appendlog, this);
}
