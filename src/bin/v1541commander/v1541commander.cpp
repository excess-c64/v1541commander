#include "v1541commander.h"
#include "mainwindow.h"

#include <QAction>
#include <QFileDialog>
#include <QFont>
#include <QFontDatabase>
#include <QMessageBox>
#ifdef DEBUG
#include <QDebug>
#include <QFontMetricsF>
#include <QScreen>
#include <QStyle>
#include <QWindow>
#endif

#include <1541img/log.h>

class V1541Commander::priv
{
    public:
        priv(V1541Commander *commander);
        V1541Commander *commander;
        QFont c64font;
        QAction newAction;
        QAction openAction;
        QAction closeAction;
        QAction exitAction;
        QVector<MainWindow *> allWindows;
        MainWindow *lastActiveWindow;
        
        MainWindow *addWindow();
        void removeWindow(MainWindow *w);
};

V1541Commander::priv::priv(V1541Commander *commander) :
    commander(commander),
    c64font("C64 Pro Mono"),
    newAction(tr("&New")),
    openAction(tr("&Open")),
    closeAction(tr("&Close")),
    exitAction(tr("E&xit")),
    allWindows(),
    lastActiveWindow(0)
{
    newAction.setShortcuts(QKeySequence::New);
    newAction.setStatusTip(tr("Create a new disk image"));
    openAction.setShortcuts(QKeySequence::Open);
    openAction.setStatusTip(tr("Open a disk image"));
    closeAction.setShortcuts(QKeySequence::Close);
    closeAction.setStatusTip(tr("Close current file"));
    exitAction.setShortcuts(QKeySequence::Quit);
    exitAction.setStatusTip(tr("Exit the application"));
}

MainWindow *V1541Commander::priv::addWindow()
{
    lastActiveWindow = new MainWindow();
    lastActiveWindow->show();
    allWindows.append(lastActiveWindow);
    connect(lastActiveWindow, &MainWindow::activated,
            commander, &V1541Commander::windowActivated);
    connect(lastActiveWindow, &MainWindow::closed,
            commander, &V1541Commander::windowClosed);
    connect(lastActiveWindow, &MainWindow::contentChanged,
            commander, &V1541Commander::windowContentChanged);
    return lastActiveWindow;
}

void V1541Commander::priv::removeWindow(MainWindow *w)
{
    if (w == lastActiveWindow) lastActiveWindow = 0;
    allWindows.removeAll(w);
    w->close();
    w->deleteLater();
}

V1541Commander::V1541Commander(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setFileLogger(stderr);
#ifdef DEBUG
    setMaxLogLevel(L_DEBUG);
#endif
    QFontDatabase::addApplicationFont(":/C64_Pro_Mono-STYLE.ttf");
    d = new priv(this);
    d->addWindow();
    connect(&d->newAction, &QAction::triggered,
	    this, &V1541Commander::newImage);
    connect(&d->openAction, &QAction::triggered,
	    this, &V1541Commander::open);
    connect(&d->closeAction, &QAction::triggered,
	    this, &V1541Commander::close);
    connect(&d->exitAction, &QAction::triggered,
	    this, &V1541Commander::exit);
#ifdef DEBUG
    QScreen *screen = d->lastActiveWindow->window()->windowHandle()->screen();
    qDebug() << "DPI:         " << screen->logicalDotsPerInch();
    QFontMetricsF metrics(d->c64font);
    qDebug() << "ascent:      " << metrics.ascent();
    qDebug() << "avgWidth:    " << metrics.averageCharWidth();
    qDebug() << "scrBarWidth: "
	<< style()->pixelMetric(QStyle::PM_ScrollBarExtent);
#endif
}

V1541Commander::~V1541Commander()
{
    delete d;
}

void V1541Commander::newImage()
{
    if (!d->lastActiveWindow) return;

    QMessageBox::information(d->lastActiveWindow, "Not implemented",
	    "Function not yet implemented");
}

void V1541Commander::open()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    QString imgFile = QFileDialog::getOpenFileName(w, tr("Open disk image"),
	    QString(), tr("1541 disk images (*.d64);;all files (*)"));
    if (!imgFile.isEmpty())
    {
        if (w->content() != MainWindow::Content::None)
        {
            w = d->addWindow();
        }

        w->openImage(imgFile);
        if (w->content() == MainWindow::Content::None)
        {
	    QMessageBox::critical(w, tr("Error reading file"),
		    tr("<p>The file you selected couldn't be read.</p>"
			"<p>This means you either haven't permission to read "
			"it or it doesn't contain a valid 1541 disc "
			"image.</p>"));
            if (d->allWindows.count() > 1)
            {
                d->removeWindow(w);
            }
        }
    }
}

void V1541Commander::close()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    w->closeDocument();
    if (d->allWindows.count() > 1)
    {
        d->removeWindow(w);
    }
}

void V1541Commander::exit()
{
    closeAllWindows();
}

void V1541Commander::windowActivated()
{
    MainWindow *w = static_cast<MainWindow *>(sender());
    d->lastActiveWindow = w;
    d->closeAction.setEnabled(w->content() != MainWindow::Content::None);
}

void V1541Commander::windowClosed()
{
    d->removeWindow(static_cast<MainWindow*>(sender()));
}

void V1541Commander::windowContentChanged()
{
    MainWindow *w = static_cast<MainWindow*>(sender());
    if (w == d->lastActiveWindow)
    {
        d->closeAction.setEnabled(w->content() != MainWindow::Content::None);
    }
}

QFont &V1541Commander::c64font()
{
    return d->c64font;
}

QAction &V1541Commander::newAction()
{
    return d->newAction;
}

QAction &V1541Commander::openAction()
{
    return d->openAction;
}

QAction &V1541Commander::closeAction()
{
    return d->closeAction;
}

QAction &V1541Commander::exitAction()
{
    return d->exitAction;
}

V1541Commander &V1541Commander::instance()
{
    return *static_cast<V1541Commander *>(QCoreApplication::instance());
}
