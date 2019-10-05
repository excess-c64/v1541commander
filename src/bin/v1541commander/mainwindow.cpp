#include "mainwindow.h"
#include "v1541imgwidget.h"

#include <QMdiArea>
#include <QMdiSubWindow>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QMessageBox>
#include <QFileDialog>
#include <QFontDatabase>

extern "C" {
#include <1541img/log.h>
}

void MainWindow::createActions()
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new disk image"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newImage);

    openAct = new QAction(tr("&Open"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open a disk image"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &MainWindow::exit);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
}

void MainWindow::newImage()
{
    QMessageBox::information(this, "Not implemented",
	    "Function not yet implemented");
}

void MainWindow::open()
{
    QString imgFile = QFileDialog::getOpenFileName(this, tr("Open disk image"),
	    QString(), tr("1541 disk images (*.d64)"));
    if (!imgFile.isEmpty())
    {
	V1541ImgWidget *imgWidget = new V1541ImgWidget();
	imgWidget->open(imgFile);
	if (imgWidget->hasValidImage())
	{
	    QMdiArea *mdiArea = static_cast<QMdiArea *>(centralWidget());
	    mdiArea->addSubWindow(imgWidget)->show();
	}
	else
	{
	    QMessageBox::critical(this, tr("Error reading file"),
		    tr("<p>The file you selected couldn't be read.</p>"
			"<p>This means you either haven't permission to read "
			"it or it doesn't contain a valid 1541 disc "
			"image.</p>"));
	}
    }
}

void MainWindow::exit()
{
    close();
}

MainWindow::MainWindow()
{
    setFileLogger(stderr);
#ifdef DEBUG
    setMaxLogLevel(L_DEBUG);
#endif
    QFontDatabase::addApplicationFont(":/C64_Pro_Mono-STYLE.ttf");
    createActions();
    createMenus();
    setCentralWidget(new QMdiArea);
    setWindowTitle(tr("V1541Commander: virtual 1541 disk commander"));
}

