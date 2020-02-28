#include "mainwindow.h"
#include "v1541commander.h"
#include "v1541imgwidget.h"

#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QStatusBar>

class MainWindow::priv
{
    public:
        priv();
        Content content;
	QString filename;
};

MainWindow::priv::priv() :
    content(None),
    filename()
{}

MainWindow::MainWindow()
{
    d = new priv();
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(&cmdr.newAction());
    fileMenu->addAction(&cmdr.openAction());
    fileMenu->addAction(&cmdr.saveAction());
    fileMenu->addAction(&cmdr.saveAsAction());
    QMenu *exportMenu = fileMenu->addMenu(tr("&Export"));
    exportMenu->addAction(&cmdr.exportZipcodeAction());
    exportMenu->addAction(&cmdr.exportZipcodeD64Action());
    exportMenu->addAction(&cmdr.exportLynxAction());
    fileMenu->addAction(&cmdr.closeAction());
    fileMenu->addSeparator();
    fileMenu->addAction(&cmdr.settingsAction());
    fileMenu->addAction(&cmdr.aboutAction());
    fileMenu->addAction(&cmdr.exitAction());
    QMenu *displayMenu = menuBar()->addMenu(tr("&View"));
    displayMenu->addAction(&cmdr.lowerCaseAction());
    QMenu *cbmdosMenu = menuBar()->addMenu(tr("&CBM DOS"));
    cbmdosMenu->addAction(&cmdr.fsOptionsAction());
    cbmdosMenu->addAction(&cmdr.rewriteImageAction());
    QMenu *petsciiMenu = cbmdosMenu->addMenu(tr("&Map UC Gfx to LC"));
    petsciiMenu->addAction(&cmdr.autoMapLcAction());
    petsciiMenu->addAction(&cmdr.mapLcAction());
    cbmdosMenu->addSeparator();
    cbmdosMenu->addAction(&cmdr.newFileAction());
    cbmdosMenu->addAction(&cmdr.deleteFileAction());
    cbmdosMenu->addAction(&cmdr.fileOverridesAction());
    QMenu *windowsMenu = menuBar()->addMenu(tr("&Windows"));
    windowsMenu->addAction(&cmdr.petsciiWindowAction());
    windowsMenu->addAction(&cmdr.logWindowAction());
    menuBar()->setFont(cmdr.menufont());
    statusBar()->setStyleSheet("QStatusBar::item {border: none;}");
    statusBar()->setFont(cmdr.statusfont());

    setAcceptDrops(true);
    setWindowTitle(tr("V1541Commander: virtual 1541 disk commander[*]"));
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::contentSelectionChanged()
{
    emit selectionChanged();
}

void MainWindow::contentModified()
{
    setWindowModified(true);
    emit modifiedChanged();
}

void MainWindow::contentSaved()
{
    setWindowModified(false);
    emit modifiedChanged();
}

MainWindow::Content MainWindow::content() const 
{
    return d->content;
}

const QString &MainWindow::filename() const
{
    return d->filename;
}

bool MainWindow::hasValidContent() const
{
    switch (d->content)
    {
	V1541ImgWidget *imgWidget;

	case Content::Image:
	    imgWidget = static_cast<V1541ImgWidget *>(centralWidget());
	    return imgWidget->hasValidImage();

	default:
	    return false;
    }
}

bool MainWindow::hasValidSelection() const
{
    switch (d->content)
    {
	V1541ImgWidget *imgWidget;

	case Content::Image:
	    imgWidget = static_cast<V1541ImgWidget *>(centralWidget());
	    return imgWidget->hasValidSelection();

	default:
	    return false;
    }
}

bool MainWindow::isReadOnly() const
{
    switch (d->content)
    {
	V1541ImgWidget *imgWidget;

	case Content::Image:
	    imgWidget = static_cast<V1541ImgWidget *>(centralWidget());
	    return imgWidget->isReadOnly();

	default:
	    return true;
    }
}

bool MainWindow::event(QEvent *e)
{
    if (e->type() == QEvent::WindowActivate)
    {
        emit activated();
    }
    return QMainWindow::event(e);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    closeDocument();
    if (isWindowModified()) e->ignore();
    else emit closed();
}

QSize MainWindow::sizeHint() const
{
    if (d->content == Content::None)
    {
        return QSize(360 * logicalDpiX() / 72, 150 * logicalDpiY() / 72);
    }
    else
    {
        return QWidget::sizeHint();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->source())
    {
        event->ignore();
    }
    else if (event->mimeData()->hasUrls())
    {
        bool ok = false;
        QList<QUrl> urls = event->mimeData()->urls();
        for (QList<QUrl>::const_iterator i = urls.cbegin();
                i != urls.cend(); ++i)
        {
            if (i->isLocalFile()) ok = true;
        }
        if (ok)
        {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->source())
    {
        event->ignore();
    }
    else if (event->mimeData()->hasUrls())
    {
        bool ok = false;
        QList<QUrl> urls = event->mimeData()->urls();
        for (QList<QUrl>::const_iterator i = urls.cbegin();
                i != urls.cend(); ++i)
        {
            if (i->isLocalFile()) ok = true;
        }
        if (ok)
        {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (event->source())
    {
        event->ignore();
    }
    else if (event->mimeData()->hasUrls())
    {
        bool ok = false;
        QList<QUrl> urls = event->mimeData()->urls();
        for (QList<QUrl>::const_iterator i = urls.cbegin();
                i != urls.cend(); ++i)
        {
            if (i->isLocalFile())
            {
                ok = true;
                cmdr.open(QFileInfo(i->toLocalFile()).canonicalFilePath());
            }
        }
        if (ok)
        {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::newImage()
{
    V1541ImgWidget *imgWidget = new V1541ImgWidget(this);
    imgWidget->newImage();
    if (imgWidget->hasValidImage())
    {
	QWidget *current = centralWidget();
        if (current) current->setParent(0);
	setCentralWidget(imgWidget);
	delete current;
	d->filename = QString();
	setWindowTitle(tr("<new disk image>[*]"));
	d->content = Content::Image;
	connect(imgWidget, &V1541ImgWidget::selectionChanged,
		this, &MainWindow::contentSelectionChanged);
	connect(imgWidget, &V1541ImgWidget::modified,
		this, &MainWindow::contentModified);
	connect(imgWidget, &V1541ImgWidget::saved,
		this, &MainWindow::contentSaved);
	emit contentChanged();
	setWindowModified(false);
	emit modifiedChanged();
        statusBar()->addPermanentWidget(imgWidget->statusWidget());
	adjustSize();
    }
    else
    {
	delete imgWidget;
    }
}

void MainWindow::openImage(const QString &imgFile)
{
    if (!imgFile.isEmpty())
    {
	bool modified = isWindowModified();
	setWindowModified(false);
	V1541ImgWidget *imgWidget = new V1541ImgWidget(this);
	connect(imgWidget, &V1541ImgWidget::modified,
		this, &MainWindow::contentModified);
	imgWidget->open(imgFile);
	if (imgWidget->hasValidImage())
	{
            QWidget *current = centralWidget();
            if (current) current->setParent(0);
            setCentralWidget(imgWidget);
            delete current;
            d->content = Content::Image;
	    if (isWindowModified())
	    {
		d->filename = QString();
		setWindowTitle(tr("<new disk image>[*]"));
	    }
	    else
	    {
		d->filename = imgFile;
		setWindowTitle(QString(imgFile).append("[*]"));
	    }
	    connect(imgWidget, &V1541ImgWidget::selectionChanged,
		    this, &MainWindow::contentSelectionChanged);
	    connect(imgWidget, &V1541ImgWidget::saved,
		    this, &MainWindow::contentSaved);
            emit contentChanged();
            statusBar()->addPermanentWidget(imgWidget->statusWidget());
            adjustSize();
	}
	else
	{
            delete imgWidget;
	    setWindowModified(modified);
	}
    }
}

void MainWindow::openVfs(CbmdosVfs *vfs)
{
    V1541ImgWidget *imgWidget = new V1541ImgWidget(this);
    imgWidget->openVfs(vfs);
    if (imgWidget->hasValidImage())
    {
	QWidget *current = centralWidget();
	if (current) current->setParent(0);
	setCentralWidget(imgWidget);
	delete current;
	d->content = Content::Image;
	d->filename = QString();
	setWindowTitle(tr("<new disk image>[*]"));
	connect(imgWidget, &V1541ImgWidget::selectionChanged,
		this, &MainWindow::contentSelectionChanged);
	connect(imgWidget, &V1541ImgWidget::modified,
		this, &MainWindow::contentModified);
	connect(imgWidget, &V1541ImgWidget::saved,
		this, &MainWindow::contentSaved);
	setWindowModified(true);
	emit contentChanged();
        statusBar()->addPermanentWidget(imgWidget->statusWidget());
	adjustSize();
    }
    else
    {
	delete imgWidget;
    }
}

void MainWindow::save(const QString &imgFile)
{
    switch (d->content)
    {
	V1541ImgWidget *imgWidget;

	case Content::Image:
	    imgWidget = static_cast<V1541ImgWidget *>(centralWidget());
	    imgWidget->save(imgFile.isEmpty() ? d->filename : imgFile);
	    if (!imgFile.isEmpty() && !isWindowModified())
	    {
		d->filename = imgFile;
		setWindowTitle(QString(imgFile).append("[*]"));
	    }
	    break;

	default:
	    break;
    }
}

void MainWindow::exportZipcode(const QString &zcFile)
{
    switch (d->content)
    {
	V1541ImgWidget *imgWidget;

	case Content::Image:
	    imgWidget = static_cast<V1541ImgWidget *>(centralWidget());
	    imgWidget->exportZipcode(zcFile);
	    break;

	default:
	    break;
    }
}

CbmdosVfs *MainWindow::exportZipcodeVfs()
{
    switch (d->content)
    {
	V1541ImgWidget *imgWidget;

	case Content::Image:
	    imgWidget = static_cast<V1541ImgWidget *>(centralWidget());
	    return imgWidget->exportZipcodeVfs();

	default:
	    return 0;
    }
}

void MainWindow::exportLynx(const QString &lynxFile)
{
    switch (d->content)
    {
	V1541ImgWidget *imgWidget;

	case Content::Image:
	    imgWidget = static_cast<V1541ImgWidget *>(centralWidget());
	    imgWidget->exportLynx(lynxFile);
	    break;

	default:
	    break;
    }
}

void MainWindow::closeDocument()
{
    if (isWindowModified())
    {
	QMessageBox::StandardButton btn = QMessageBox::question(this,
		tr("Discard unsaved changes?"), QString(tr("%1 has unsaved "
			"changes. \nDo you want to save now?"))
		.arg(d->filename.isEmpty() ?
                    tr("<new disk image>") : d->filename),
		QMessageBox::Save|QMessageBox::Cancel|QMessageBox::Discard,
		QMessageBox::Save);
	if (btn == QMessageBox::Cancel) return;
	if (btn == QMessageBox::Save)
	{
	    if (d->filename.isEmpty())
	    {
		emit activated();
		cmdr.saveAs();
	    }
	    else
	    {
		save(QString());
	    }
	    if (isWindowModified()) return;
	}
    }
    QWidget *current = centralWidget();
    if (current) current->setParent(0);
    setCentralWidget(0);
    delete current;
    d->content = Content::None;
    d->filename = QString();
    emit contentChanged();
    setWindowModified(false);
    emit modifiedChanged();
    adjustSize();
    setWindowTitle(tr("V1541Commander: virtual 1541 disk commander[*]"));
}

void MainWindow::fsOptions()
{
    if (d->content != Content::Image) return;
    static_cast<V1541ImgWidget *>(centralWidget())->fsOptions();
}

void MainWindow::rewriteImage()
{
    if (d->content != Content::Image) return;
    static_cast<V1541ImgWidget *>(centralWidget())->rewriteImage();
}

void MainWindow::mapToLc()
{
    if (d->content != Content::Image) return;
    static_cast<V1541ImgWidget *>(centralWidget())->mapToLc();
}

void MainWindow::newFile()
{
    if (d->content != Content::Image) return;
    static_cast<V1541ImgWidget *>(centralWidget())->newFile();
}

void MainWindow::deleteFile()
{
    if (d->content != Content::Image) return;
    static_cast<V1541ImgWidget *>(centralWidget())->deleteFile();
}

void MainWindow::fileOverrides()
{
    if (d->content != Content::Image) return;
    static_cast<V1541ImgWidget *>(centralWidget())->fileOverrides();
}

void MainWindow::showStatusLine(const QString &line)
{
    statusBar()->showMessage(line, 10000);
}
