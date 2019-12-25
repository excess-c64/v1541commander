#ifndef V1541C_V1541COMMANDER_H
#define V1541C_V1541COMMANDER_H

#include <QApplication>

class QAction;

#define app (V1541Commander::instance())

class V1541Commander: public QApplication
{
    Q_OBJECT

    private:
        class priv;
        priv *d;

    private slots:
        void newImage();
        void open();
	void save();
	void saveAs();
        void close();
        void exit();
        void windowActivated();
        void windowClosed();
        void windowContentChanged();
	void windowSelectionChanged();
        void showPetsciiWindow();
	void showLogWindow();
	void petsciiInput(ushort val);
	void newFile();
	void deleteFile();
	void logLineAppended(const QString &line);
        
    public:
        V1541Commander(int &argc, char **argv);
        ~V1541Commander();
        QFont &c64font();
        QAction &newAction();
        QAction &openAction();
	QAction &saveAction();
	QAction &saveAsAction();
        QAction &closeAction();
        QAction &exitAction();
        QAction &petsciiWindowAction();
	QAction &logWindowAction();
	QAction &newFileAction();
	QAction &deleteFileAction();

        static V1541Commander &instance();
};

#endif
