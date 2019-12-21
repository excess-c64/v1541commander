#include "v1541commander.h"
#include "logwindow.h"
#include "mainwindow.h"
#include "petsciiwindow.h"
#include "petsciiedit.h"

#include <QAction>
#include <QFileDialog>
#include <QFont>
#include <QFontDatabase>
#include <QMessageBox>

#include <1541img/log.h>

class V1541Commander::priv
{
    public:
        priv(V1541Commander *commander);
        V1541Commander *commander;
        QFont c64font;
        QAction newAction;
        QAction openAction;
	QAction saveAction;
	QAction saveAsAction;
        QAction closeAction;
        QAction exitAction;
        QAction petsciiWindowAction;
	QAction logWindowAction;
        QVector<MainWindow *> allWindows;
        MainWindow *lastActiveWindow;
        PetsciiWindow *petsciiWindow;
	LogWindow logWindow;
        
        MainWindow *addWindow();
        void removeWindow(MainWindow *w);
};

V1541Commander::priv::priv(V1541Commander *commander) :
    commander(commander),
    c64font("C64 Pro Mono"),
    newAction(tr("&New")),
    openAction(tr("&Open")),
    saveAction(tr("&Save")),
    saveAsAction(tr("Save &As")),
    closeAction(tr("&Close")),
    exitAction(tr("E&xit")),
    petsciiWindowAction(tr("&PETSCII Input")),
    logWindowAction(tr("lib1541img &log")),
    allWindows(),
    lastActiveWindow(0),
    petsciiWindow(0),
    logWindow()
{
    newAction.setShortcuts(QKeySequence::New);
    newAction.setStatusTip(tr("Create a new disk image"));
    openAction.setShortcuts(QKeySequence::Open);
    openAction.setStatusTip(tr("Open a disk image"));
    saveAction.setShortcuts(QKeySequence::Save);
    saveAction.setStatusTip(tr("Save disk image"));
    saveAsAction.setShortcuts(QKeySequence::SaveAs);
    saveAsAction.setStatusTip(tr("Save disk image as new file"));
    closeAction.setShortcuts(QKeySequence::Close);
    closeAction.setStatusTip(tr("Close current file"));
    exitAction.setShortcuts(QKeySequence::Quit);
    exitAction.setStatusTip(tr("Exit the application"));
    petsciiWindowAction.setShortcut(QKeySequence(Qt::CTRL+Qt::Key_P));
    petsciiWindowAction.setStatusTip(tr("Show PETSCII input window"));
    logWindowAction.setShortcut(QKeySequence(Qt::CTRL+Qt::Key_L));
    logWindowAction.setStatusTip(tr("Show lib1541img log messages"));
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
    if (allWindows.count() == 0) closeAllWindows();
}

V1541Commander::V1541Commander(int &argc, char **argv)
    : QApplication(argc, argv)
{
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
    connect(&d->saveAction, &QAction::triggered,
	    this, &V1541Commander::save);
    connect(&d->saveAsAction, &QAction::triggered,
	    this, &V1541Commander::saveAs);
    connect(&d->closeAction, &QAction::triggered,
	    this, &V1541Commander::close);
    connect(&d->exitAction, &QAction::triggered,
	    this, &V1541Commander::exit);
    connect(&d->petsciiWindowAction, &QAction::triggered,
            this, &V1541Commander::showPetsciiWindow);
    connect(&d->logWindowAction, &QAction::triggered,
            this, &V1541Commander::showLogWindow);
}

V1541Commander::~V1541Commander()
{
    delete d->petsciiWindow;
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

static QString getFilterForWindowContent(MainWindow::Content content)
{
    switch (content)
    {
	case MainWindow::Content::Image:
	    return QString(QT_TR_NOOP(
			"1541 disk images (*.d64);;all files (*)"));
	default:
	    return QString(QT_TR_NOOP("all files (*)"));
    }
}

void V1541Commander::save()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    w->save();
}

void V1541Commander::saveAs()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    QString imgFile = QFileDialog::getSaveFileName(w, tr("Save as ..."),
	    QString(), getFilterForWindowContent(w->content()));
    if (!imgFile.isEmpty())
    {
	w->save(imgFile);
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
    d->saveAction.setEnabled(!w->filename().isEmpty());
    d->saveAsAction.setEnabled(w->content() != MainWindow::Content::None);
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
	d->saveAction.setEnabled(!w->filename().isEmpty());
        d->saveAsAction.setEnabled(w->content() != MainWindow::Content::None);
    }
}

void V1541Commander::showPetsciiWindow()
{
    if (!d->petsciiWindow)
    {
        d->petsciiWindow = new PetsciiWindow();
	connect(d->petsciiWindow, &PetsciiWindow::petsciiInput,
		this, &V1541Commander::petsciiInput);
    }
    d->petsciiWindow->show();
}

void V1541Commander::showLogWindow()
{
    d->logWindow.show();
}

void V1541Commander::petsciiInput(ushort val)
{
    PetsciiEdit *pe = dynamic_cast<PetsciiEdit *>(focusWidget());
    if (pe) pe->petsciiInput(val);
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

QAction &V1541Commander::saveAction()
{
    return d->saveAction;
}

QAction &V1541Commander::saveAsAction()
{
    return d->saveAsAction;
}

QAction &V1541Commander::closeAction()
{
    return d->closeAction;
}

QAction &V1541Commander::exitAction()
{
    return d->exitAction;
}

QAction &V1541Commander::petsciiWindowAction()
{
    return d->petsciiWindowAction;
}

QAction &V1541Commander::logWindowAction()
{
    return d->logWindowAction;
}

V1541Commander &V1541Commander::instance()
{
    return *static_cast<V1541Commander *>(QCoreApplication::instance());
}
