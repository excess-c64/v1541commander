#include "mainwindow.h"
#include "v1541commander.h"
#include "v1541imgwidget.h"

#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
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
    fileMenu->addAction(&cmdr.aboutAction());
    fileMenu->addAction(&cmdr.exitAction());
    QMenu *cbmdosMenu = menuBar()->addMenu(tr("CBM &DOS"));
    cbmdosMenu->addAction(&cmdr.fsOptionsAction());
    cbmdosMenu->addAction(&cmdr.rewriteImageAction());
    cbmdosMenu->addSeparator();
    cbmdosMenu->addAction(&cmdr.newFileAction());
    cbmdosMenu->addAction(&cmdr.deleteFileAction());
    QMenu *windowsMenu = menuBar()->addMenu(tr("&Windows"));
    windowsMenu->addAction(&cmdr.petsciiWindowAction());
    windowsMenu->addAction(&cmdr.logWindowAction());
    (void) statusBar();

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
        return QSize(480,200);
    }
    else
    {
        return QWidget::sizeHint();
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
	    if (!imgFile.isEmpty())
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
		.arg(d->filename.isEmpty() ? "<new disk image>" : d->filename),
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

void MainWindow::showStatusLine(const QString &line)
{
    statusBar()->showMessage(line);
}
