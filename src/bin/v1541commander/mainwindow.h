#ifndef V1541C_MAINWINDOW_H
#define V1541C_MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QMenu;

class MainWindow: public QMainWindow
{
    Q_OBJECT

    private:
	void createActions();
	void createMenus();

	QMenu *fileMenu;
	QAction *newAct;
	QAction *openAct;
	QAction *exitAct;

    private slots:
	void newImage();
	void open();
	void exit();

    public:
	MainWindow();

};

#endif
