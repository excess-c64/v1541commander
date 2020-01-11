#include "v1541commander.h"
#include "aboutbox.h"
#include "logwindow.h"
#include "mainwindow.h"
#include "petsciiwindow.h"
#include "petsciiedit.h"

#include <QAction>
#include <QCryptographicHash>
#include <QFileDialog>
#include <QFont>
#include <QFontDatabase>
#ifndef _WIN32
#include <QIcon>
#endif
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QSet>
#include <QSettings>
#ifdef _WIN32
#include <windows.h>
#include <lmcons.h>
#else
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#endif

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
	QAction exportZipcodeAction;
	QAction exportZipcodeD64Action;
	QAction exportLynxAction;
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
	QString instanceServerName;
	QLocalServer instanceServer;
	bool isPrimaryInstance;
	QSet<QLocalSocket *> activeClients;
        
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
    exportZipcodeAction(tr("&Zipcode")),
    exportZipcodeD64Action(tr("Zipcode (&D64)")),
    exportLynxAction(tr("&LyNX")),
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
    logWindow(),
    instanceServerName(),
    instanceServer(),
    isPrimaryInstance(false),
    activeClients()
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
    exportZipcodeAction.setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Z));
    exportZipcodeAction.setStatusTip(tr("Export as a set of Zipcode files"));
    exportZipcodeD64Action.setShortcut(
	    QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_Z));
    exportZipcodeD64Action.setStatusTip(
	    tr("Export as Zipcode files on a new D64 image"));
    exportLynxAction.setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Y));
    exportLynxAction.setStatusTip(tr("Export as a LyNX file"));
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
    QCryptographicHash appData(QCryptographicHash::Sha256);
    appData.addData(QCoreApplication::organizationName().toUtf8());
    appData.addData(QCoreApplication::applicationName().toUtf8());
#ifdef _WIN32
    wchar_t username[UNLEN + 1];
    DWORD usernameLen = UNLEN + 1;
    if (GetUserNameW(username, &usernameLen))
    {
	appData.addData(QString::fromWCharArray(username).toUtf8());
    }
    else
    {
	appData.addData(qgetenv("USERNAME"));
    }
#else
    QByteArray username;
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw)
    {
	username = pw->pw_name;
    }
    if (username.isEmpty())
    {
	username = qgetenv("USER");
    }
    appData.addData(username);
#endif
    instanceServerName = appData.result().toBase64().replace("/","_");
    bool listening = instanceServer.listen(instanceServerName);
#ifdef _WIN32
    if (listening)
    {
	CreateMutexW(0, true,
		reinterpret_cast<LPCWSTR>(instanceServerName.utf16()));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
	    instanceServer.close();
	    listening = false;
	}
    }
#else
    if (!listening)
    {
	QLocalSocket sock;
	sock.connectToServer(instanceServerName, QIODevice::WriteOnly);
	if (sock.state() != QLocalSocket::ConnectedState &&
		!sock.waitForConnected(500))
	{
	    QLocalServer::removeServer(instanceServerName);
	    listening = instanceServer.listen(instanceServerName);
	}
	else
	{
	    sock.disconnectFromServer();
	}
    }
#endif
    isPrimaryInstance = listening;
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
    exportZipcodeAction.setEnabled(w->hasValidContent());
    exportZipcodeD64Action.setEnabled(w->hasValidContent());
    exportLynxAction.setEnabled(w->hasValidContent());
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
    connect(&d->exportZipcodeAction, &QAction::triggered,
	    this, &V1541Commander::exportZipcode);
    connect(&d->exportZipcodeD64Action, &QAction::triggered,
	    this, &V1541Commander::exportZipcodeD64);
    connect(&d->exportLynxAction, &QAction::triggered,
	    this, &V1541Commander::exportLynx);
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
    if (d->isPrimaryInstance)
    {
	connect(&d->instanceServer, &QLocalServer::newConnection,
		this, &V1541Commander::newConnection);
    }
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
    if (!w || w->content() != MainWindow::Content::None)
    {
	w = d->addWindow();
    }
    w->show();
    w->raise();
    w->activateWindow();

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
	    QString(), tr("1541 disk images (*.d64);;"
		"Zipcode files (*!*.prg);;LyNX files (*.lnx);;all files (*)"));
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

void V1541Commander::exportZipcode()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    QString zcFile = QFileDialog::getSaveFileName(w, tr("Export as ..."),
	    QString(), QString(QT_TR_NOOP(
		    "Zipcode files (*.prg);;all files (*)")));

    if (!zcFile.isEmpty())
    {
	w->exportZipcode(zcFile);
    }
}

void V1541Commander::exportZipcodeD64()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;
    CbmdosVfs *vfs = w->exportZipcodeVfs();
    if (vfs)
    {
	w = d->addWindow();
	w->openVfs(vfs);
    }
}

void V1541Commander::exportLynx()
{
    MainWindow *w = d->lastActiveWindow;
    if (!w) return;

    QString lynxFile = QFileDialog::getSaveFileName(w, tr("Export as ..."),
	    QString(), QString(QT_TR_NOOP(
		    "LyNX files (*.lnx);;all files (*)")));

    if (!lynxFile.isEmpty())
    {
	w->exportLynx(lynxFile);
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

void V1541Commander::newConnection()
{
    QLocalSocket *conn = d->instanceServer.nextPendingConnection();
    connect(conn, &QIODevice::readyRead, this, &V1541Commander::readyRead);
    connect(conn, &QLocalSocket::disconnected,
	    this, &V1541Commander::disconnected);
    QDataStream sendStream(conn);
    sendStream << applicationPid();
    conn->flush();
}

void V1541Commander::disconnected()
{
    QLocalSocket *sock = dynamic_cast<QLocalSocket *>(sender());
    if (sock)
    {
	if (!d->activeClients.contains(sock))
	{
	    MainWindow *w = d->lastActiveWindow;
	    if (!w) w = d->allWindows.first();
	    if (w)
	    {
		w->show();
		w->raise();
		w->activateWindow();
	    }
	    sock->deleteLater();
	}
    }
}

void V1541Commander::readyRead()
{
    QLocalSocket *sock = dynamic_cast<QLocalSocket *>(sender());
    if (sock)
    {
	d->activeClients.insert(sock);
	QDataStream recvStream(sock);
	QString filename;
	for (;;)
	{
	    recvStream.startTransaction();
	    recvStream >> filename;
	    if (!recvStream.commitTransaction()) break;
	    open(filename);
	}
	d->activeClients.remove(sock);
	if (sock->state() != QLocalSocket::ConnectedState)
	{
	    sock->deleteLater();
	}
    }
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

QAction &V1541Commander::exportZipcodeAction()
{
    return d->exportZipcodeAction;
}

QAction &V1541Commander::exportZipcodeD64Action()
{
    return d->exportZipcodeD64Action;
}

QAction &V1541Commander::exportLynxAction()
{
    return d->exportLynxAction;
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

const QString &V1541Commander::instanceServerName() const
{
    return d->instanceServerName;
}

bool V1541Commander::isPrimaryInstance() const
{
    return d->isPrimaryInstance;
}

V1541Commander &V1541Commander::instance()
{
    return *static_cast<V1541Commander *>(QCoreApplication::instance());
}
