#include "v1541commander.h"
#include "aboutbox.h"
#include "logwindow.h"
#include "mainwindow.h"
#include "petsciiwindow.h"
#include "petsciiedit.h"

#include <QAction>
#include <QFileDialog>
#include <QFont>
#include <QFontDatabase>
#ifndef _WIN32
#include <QIcon>
#endif
#include <QMessageBox>
#include <QSettings>

#include <1541img/log.h>

class V1541Commander::priv
{
    public:
        priv(V1541Commander *commander);
        V1541Commander *commander;
        QFont c64font;
#ifndef _WIN32
	QIcon appIcon;
#endif
        QAction newAction;
        QAction openAction;
	QAction saveAction;
	QAction saveAsAction;
        QAction closeAction;
	QAction aboutAction;
        QAction exitAction;
        QAction petsciiWindowAction;
	QAction logWindowAction;
	QAction fsOptionsAction;
	QAction rewriteImageAction;
	QAction newFileAction;
	QAction deleteFileAction;
        QVector<MainWindow *> allWindows;
        MainWindow *lastActiveWindow;
        PetsciiWindow *petsciiWindow;
	AboutBox aboutBox;
	LogWindow logWindow;
        
        MainWindow *addWindow(bool show = true);
        void removeWindow(MainWindow *w);
	void updateActions(MainWindow *w);
};

V1541Commander::priv::priv(V1541Commander *commander) :
    commander(commander),
    c64font("C64 Pro Mono"),
#ifndef _WIN32
    appIcon(),
#endif
    newAction(tr("&New")),
    openAction(tr("&Open")),
    saveAction(tr("&Save")),
    saveAsAction(tr("Save &As")),
    closeAction(tr("&Close")),
    aboutAction(tr("&About")),
    exitAction(tr("E&xit")),
    petsciiWindowAction(tr("&PETSCII Input")),
    logWindowAction(tr("lib1541img &log")),
    fsOptionsAction(tr("Filesystem &Options")),
    rewriteImageAction(tr("&Rewrite Image")),
    newFileAction(tr("&New File")),
    deleteFileAction(tr("&Delete File")),
    allWindows(),
    lastActiveWindow(0),
    petsciiWindow(0),
    aboutBox(c64font),
    logWindow()
{
    newAction.setShortcuts(QKeySequence::New);
    newAction.setStatusTip(tr("Create a new disk image"));
    openAction.setShortcuts(QKeySequence::Open);
    openAction.setStatusTip(tr("Open a disk image"));
    saveAction.setShortcuts(QKeySequence::Save);
    saveAction.setStatusTip(tr("Save disk image"));
#ifdef _WIN32
    saveAsAction.setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_S));
#else
    saveAsAction.setShortcuts(QKeySequence::SaveAs);
#endif
    saveAsAction.setStatusTip(tr("Save disk image as new file"));
    closeAction.setShortcuts(QKeySequence::Close);
    closeAction.setStatusTip(tr("Close current file"));
    exitAction.setShortcuts(QKeySequence::Quit);
    exitAction.setStatusTip(tr("Exit the application"));
    petsciiWindowAction.setShortcut(QKeySequence(Qt::CTRL+Qt::Key_P));
    petsciiWindowAction.setStatusTip(tr("Show PETSCII input window"));
    logWindowAction.setShortcut(QKeySequence(Qt::CTRL+Qt::Key_L));
    logWindowAction.setStatusTip(tr("Show lib1541img log messages"));
    fsOptionsAction.setShortcut(QKeySequence(Qt::CTRL+Qt::Key_F));
    fsOptionsAction.setStatusTip(tr("Change filesystem options"));
    rewriteImageAction.setShortcut(QKeySequence(Qt::CTRL+Qt::Key_R));
    rewriteImageAction.setStatusTip(tr("Rewrite disk image from scratch"));
    newFileAction.setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Period));
    newFileAction.setStatusTip(tr("Create new file at selection"));
    deleteFileAction.setShortcut(QKeySequence::Delete);
    deleteFileAction.setStatusTip(tr("Delete selected file"));
#ifndef _WIN32
    appIcon.addPixmap(QPixmap(":/gfx/icon_256.png"));
    appIcon.addPixmap(QPixmap(":/gfx/icon_48.png"));
    appIcon.addPixmap(QPixmap(":/gfx/icon_32.png"));
    appIcon.addPixmap(QPixmap(":/gfx/icon_16.png"));
    logWindow.setWindowIcon(appIcon);
    aboutBox.setWindowIcon(appIcon);
#endif
}

MainWindow *V1541Commander::priv::addWindow(bool show)
{
#ifdef _WIN32
    MainWindow *newWin = new MainWindow();
    MainWindow *lastWin = lastActiveWindow;
    if (!lastWin && allWindows.count() > 0) lastWin = allWindows.first();
    if (lastWin)
    {
        QPoint pos = lastWin->pos();
        pos.setX(pos.x() + 64);
        pos.setY(pos.y() + 32);
        bool posOk = false;
        for (int i = 0; i < 20; ++i)
        {
            posOk = true;
            for (QVector<MainWindow *>::const_iterator w = allWindows.cbegin();
                    w != allWindows.cend(); ++w)
            {
                if (pos == (*w)->pos())
                {
                    posOk = false;
                    break;
                }
            }
            if (posOk) break;
            pos.setX(pos.x() + 64);
            pos.setY(pos.y() + 32);
        }
        newWin->move(pos);
    }
    lastActiveWindow = newWin;
#else
    lastActiveWindow = new MainWindow();
    lastActiveWindow->setWindowIcon(appIcon);
#endif
    if (show) lastActiveWindow->show();
    allWindows.append(lastActiveWindow);
    connect(lastActiveWindow, &MainWindow::activated,
            commander, &V1541Commander::windowActivated);
    connect(lastActiveWindow, &MainWindow::closed,
            commander, &V1541Commander::windowClosed);
    connect(lastActiveWindow, &MainWindow::contentChanged,
            commander, &V1541Commander::windowContentChanged);
    connect(lastActiveWindow, &MainWindow::modifiedChanged,
            commander, &V1541Commander::windowContentChanged);
    connect(lastActiveWindow, &MainWindow::selectionChanged,
            commander, &V1541Commander::windowSelectionChanged);
    return lastActiveWindow;
}

void V1541Commander::priv::removeWindow(MainWindow *w)
{
    if (w == lastActiveWindow) lastActiveWindow = 0;
    allWindows.removeAll(w);
    w->close();
    if (allWindows.count() == 0)
    {
        QSettings settings(QCoreApplication::organizationName(),
                QCoreApplication::applicationName());
        settings.setValue("geometry", w->saveGeometry());
        closeAllWindows();
    }
    w->deleteLater();
}

void V1541Commander::priv::updateActions(MainWindow *w)
{
    closeAction.setEnabled(w->content() != MainWindow::Content::None);
    saveAction.setEnabled(w->hasValidContent() &&
	    (w->isWindowModified() || w->filename().isEmpty()));
    saveAsAction.setEnabled(w->hasValidContent());
    fsOptionsAction.setEnabled(w->content() == MainWindow::Content::Image
	    && w->hasValidContent());
    rewriteImageAction.setEnabled(w->content() == MainWindow::Content::Image
	    && w->hasValidContent());
    newFileAction.setEnabled(w->content() == MainWindow::Content::Image
	    && w->hasValidContent());
    deleteFileAction.setEnabled(w->content() == MainWindow::Content::Image
	    && w->hasValidContent() && w->hasValidSelection());
}

V1541Commander::V1541Commander(int &argc, char **argv)
    : QApplication(argc, argv)
{
#ifdef DEBUG
    setMaxLogLevel(L_DEBUG);
#endif
    QFontDatabase::addApplicationFont(":/C64_Pro_Mono-STYLE.ttf");
    d = new priv(this);
    d->addWindow(false);
    QSettings settings(QCoreApplication::organizationName(),
            QCoreApplication::applicationName());
    d->lastActiveWindow->restoreGeometry(
            settings.value("geometry").toByteArray());
    connect(&d->newAction, &QAction::triggered,
	    this, &V1541Commander::newImage);
    connect(&d->openAction, SIGNAL(triggered()),
	    this, SLOT(open()));
    connect(&d->saveAction, &QAction::triggered,
	    this, &V1541Commander::save);
    connect(&d->saveAsAction, &QAction::triggered,
	    this, &V1541Commander::saveAs);
    connect(&d->closeAction, &QAction::triggered,
	    this, &V1541Commander::close);
    connect(&d->aboutAction, &QAction::triggered,
	    this, &V1541Commander::about);
    connect(&d->exitAction, &QAction::triggered,
	    this, &V1541Commander::exit);
    connect(&d->petsciiWindowAction, &QAction::triggered,
            this, &V1541Commander::showPetsciiWindow);
    connect(&d->logWindowAction, &QAction::triggered,
            this, &V1541Commander::showLogWindow);
    connect(&d->fsOptionsAction, &QAction::triggered,
            this, &V1541Commander::fsOptions);
    connect(&d->rewriteImageAction, &QAction::triggered,
            this, &V1541Commander::rewriteImage);
    connect(&d->newFileAction, &QAction::triggered,
            this, &V1541Commander::newFile);
    connect(&d->deleteFileAction, &QAction::triggered,
            this, &V1541Commander::deleteFile);
    connect(&d->logWindow, &LogWindow::logLineAppended,
	    this, &V1541Commander::logLineAppended);
}

V1541Commander::~V1541Commander()
{
    delete d->petsciiWindow;
    delete d;
}

void V1541Commander::show()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    w->show();
}

void V1541Commander::newImage()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    if (w->content() != MainWindow::Content::None)
    {
	w = d->addWindow();
    }
    w->newImage();
    if (w->content() == MainWindow::Content::None)
    {
	if (d->allWindows.count() > 1)
	{
	    d->removeWindow(w);
	}
    }
}

void V1541Commander::open(const QString &filename)
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    if (w->content() != MainWindow::Content::None)
    {
	w = d->addWindow();
    }

    w->openImage(filename);
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

void V1541Commander::open()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    QString imgFile = QFileDialog::getOpenFileName(w, tr("Open disk image"),
	    QString(), tr("1541 disk images (*.d64);;all files (*)"));
    if (!imgFile.isEmpty())
    {
	open(imgFile);
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

    if (w->filename().isEmpty()) saveAs();
    else w->save();
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

void V1541Commander::about()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    d->aboutBox.show();
    d->aboutBox.activateWindow();
    d->aboutBox.raise();
    QRect aboutBoxRect = d->aboutBox.geometry();
    aboutBoxRect.moveCenter(w->frameGeometry().center());
    d->aboutBox.setGeometry(aboutBoxRect);
}

void V1541Commander::exit()
{
    closeAllWindows();
}

void V1541Commander::windowActivated()
{
    MainWindow *w = static_cast<MainWindow *>(sender());
    d->lastActiveWindow = w;
    d->updateActions(w);
}

void V1541Commander::windowClosed()
{
    d->removeWindow(static_cast<MainWindow*>(sender()));
}

void V1541Commander::windowContentChanged()
{
    MainWindow *w = static_cast<MainWindow*>(sender());
    if (w == d->lastActiveWindow) d->updateActions(w);
}

void V1541Commander::windowSelectionChanged()
{
    MainWindow *w = static_cast<MainWindow*>(sender());
    if (w == d->lastActiveWindow) d->updateActions(w);
}

void V1541Commander::showPetsciiWindow()
{
    if (!d->petsciiWindow)
    {
        d->petsciiWindow = new PetsciiWindow();
#ifndef _WIN32
	d->petsciiWindow->setWindowIcon(d->appIcon);
#endif
	connect(d->petsciiWindow, &PetsciiWindow::petsciiInput,
		this, &V1541Commander::petsciiInput);
    }
    d->petsciiWindow->show();
}

void V1541Commander::showLogWindow()
{
    d->logWindow.show();
}

void V1541Commander::fsOptions()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    w->fsOptions();
}

void V1541Commander::rewriteImage()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    w->rewriteImage();
}

void V1541Commander::newFile()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    w->newFile();
}

void V1541Commander::deleteFile()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    w->deleteFile();
}

void V1541Commander::logLineAppended(const QString &line)
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    w->showStatusLine(line);
}

void V1541Commander::petsciiInput(ushort val)
{
    PetsciiEdit *pe = dynamic_cast<PetsciiEdit *>(focusWidget());
    if (pe) pe->petsciiInput(val);
}

const QFont &V1541Commander::c64font() const
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

QAction &V1541Commander::aboutAction()
{
    return d->aboutAction;
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

QAction &V1541Commander::fsOptionsAction()
{
    return d->fsOptionsAction;
}

QAction &V1541Commander::rewriteImageAction()
{
    return d->rewriteImageAction;
}

QAction &V1541Commander::newFileAction()
{
    return d->newFileAction;
}

QAction &V1541Commander::deleteFileAction()
{
    return d->deleteFileAction;
}

V1541Commander &V1541Commander::instance()
{
    return *static_cast<V1541Commander *>(QCoreApplication::instance());
}
