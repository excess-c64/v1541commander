#include "mainwindow.h"
#include "v1541commander.h"
#include "v1541imgwidget.h"

#include <QEvent>
#include <QMenu>
#include <QMenuBar>

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
    fileMenu->addAction(&app.newAction());
    fileMenu->addAction(&app.openAction());
    fileMenu->addAction(&app.saveAction());
    fileMenu->addAction(&app.saveAsAction());
    fileMenu->addAction(&app.closeAction());
    fileMenu->addSeparator();
    fileMenu->addAction(&app.exitAction());
    QMenu *cbmdosMenu = menuBar()->addMenu(tr("CBM &DOS"));
    cbmdosMenu->addAction(&app.newFileAction());
    cbmdosMenu->addAction(&app.deleteFileAction());
    QMenu *windowsMenu = menuBar()->addMenu(tr("&Windows"));
    windowsMenu->addAction(&app.petsciiWindowAction());
    windowsMenu->addAction(&app.logWindowAction());
    (void) statusBar();

    setWindowTitle(tr("V1541Commander: virtual 1541 disk commander"));
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::contentSelectionChanged()
{
    emit selectionChanged();
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
    (void) e;
    emit closed();
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
    V1541ImgWidget *imgWidget = new V1541ImgWidget();
    imgWidget->newImage();
    QWidget *current = centralWidget();
    setCentralWidget(imgWidget);
    delete current;
    imgWidget->setParent(this);
    setWindowTitle(tr("<new disk image>"));
    d->content = Content::Image;
    connect(imgWidget, &V1541ImgWidget::selectionChanged,
	    this, &MainWindow::contentSelectionChanged);
    emit contentChanged();
    adjustSize();
}

void MainWindow::openImage(const QString &imgFile)
{
    if (!imgFile.isEmpty())
    {
	V1541ImgWidget *imgWidget = new V1541ImgWidget();
	imgWidget->open(imgFile);
	if (imgWidget->hasValidImage())
	{
            QWidget *current = centralWidget();
            setCentralWidget(imgWidget);
            delete current;
            imgWidget->setParent(this);
            d->content = Content::Image;
	    d->filename = imgFile;
            setWindowTitle(d->filename);
	    connect(imgWidget, &V1541ImgWidget::selectionChanged,
		    this, &MainWindow::contentSelectionChanged);
            emit contentChanged();
            adjustSize();
	}
	else
	{
            delete imgWidget;
	}
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
	    if (d->filename.isEmpty())
	    {
		d->filename = imgFile;
		setWindowTitle(d->filename);
	    }
	    break;

	default:
	    break;
    }
}

void MainWindow::closeDocument()
{
    QWidget *current = centralWidget();
    setCentralWidget(0);
    delete current;
    d->content = Content::None;
    emit contentChanged();
    adjustSize();
    setWindowTitle(tr("V1541Commander: virtual 1541 disk commander"));
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

