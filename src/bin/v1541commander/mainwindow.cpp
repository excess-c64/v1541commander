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
};

MainWindow::priv::priv() :
    content(None)
{}

MainWindow::MainWindow()
{
    d = new priv();
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(&app.newAction());
    fileMenu->addAction(&app.openAction());
    fileMenu->addAction(&app.saveAsAction());
    fileMenu->addAction(&app.closeAction());
    fileMenu->addSeparator();
    fileMenu->addAction(&app.exitAction());
    QMenu *windowsMenu = menuBar()->addMenu(tr("&Windows"));
    windowsMenu->addAction(&app.petsciiWindowAction());
    windowsMenu->addAction(&app.logWindowAction());

    setWindowTitle(tr("V1541Commander: virtual 1541 disk commander"));
}

MainWindow::~MainWindow()
{
    delete d;
}

MainWindow::Content MainWindow::content()
{
    return d->content;
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
        return QSize(320,200);
    }
    else
    {
        return QWidget::sizeHint();
    }
}

void MainWindow::openImage(QString &imgFile)
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
            setWindowTitle(imgWidget->windowTitle());
            d->content = Content::Image;
            emit contentChanged();
            adjustSize();
	}
	else
	{
            delete imgWidget;
	}
    }
}

void MainWindow::save(QString &imgFile)
{
    switch (d->content)
    {
	V1541ImgWidget *imgWidget;

	case Content::Image:
	    imgWidget = static_cast<V1541ImgWidget *>(centralWidget());
	    imgWidget->save(imgFile);
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
}

