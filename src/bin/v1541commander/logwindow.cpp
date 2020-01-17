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
    QString logmsg = levels[level].arg(message);
    logWindow->appendPlainText(logmsg);
    emit logWindow->logLineAppended(logmsg);
}

LogWindow::LogWindow(QWidget *parent) :
    QPlainTextEdit(parent)
{
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    setReadOnly(true);
    setMaximumBlockCount(1024);
    setWindowTitle(tr("lib1541img log viewer"));
    appendlog(L_INFO, "V1541Commander started, waiting for log messages from "
	    "lib1541img ...", this);
    setCustomLogger(appendlog, this);
    adjustSize();
}

QSize LogWindow::sizeHint() const
{
    QFontMetricsF fm(font());
    return QSize(82 * fm.averageCharWidth(), 16 * fm.height());
}

QSize LogWindow::minimumSizeHint() const
{
    return sizeHint();
}
