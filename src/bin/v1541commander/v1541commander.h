#ifndef V1541C_V1541COMMANDER_H
#define V1541C_V1541COMMANDER_H

#include <QApplication>

class QAction;

#define cmdr (V1541Commander::instance())

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
        void close();
	void about();
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
	QAction &aboutAction();
        QAction &exitAction();
        QAction &petsciiWindowAction();
	QAction &logWindowAction();
	QAction &newFileAction();
	QAction &deleteFileAction();

        static V1541Commander &instance();

    public slots:
	void open(const QString &filename);
	void saveAs();
};

#endif
