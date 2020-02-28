#ifndef V1541C_V1541COMMANDER_H
#define V1541C_V1541COMMANDER_H

#include <QApplication>

class QAction;
class QPixmap;
class QTranslator;
class Settings;

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
	void exportZipcode();
	void exportZipcodeD64();
	void exportLynx();
        void close();
	void showSettings();
	void about();
        void exit();
        void windowActivated();
        void windowClosed();
        void windowContentChanged();
	void windowSelectionChanged();
        void showPetsciiWindow();
	void showLogWindow();
	void petsciiInput(ushort val);
	void fsOptions();
	void rewriteImage();
	void mapToLc();
	void newFile();
	void deleteFile();
	void fileOverrides();
	void logLineAppended(const QString &line);
	void newConnection();
	void disconnected();
	void readyRead();
        
    public:
        V1541Commander(int &argc, char **argv, QTranslator *translator);
        ~V1541Commander();
        const QFont &c64font() const;
        const QFont &menufont() const;
        const QFont &statusfont() const;
        const QPixmap &statusLedRed() const;
        const QPixmap &statusLedYellow() const;
        const QPixmap &statusLedGreen() const;
        QAction &newAction();
        QAction &openAction();
	QAction &saveAction();
	QAction &saveAsAction();
	QAction &exportZipcodeAction();
	QAction &exportZipcodeD64Action();
	QAction &exportLynxAction();
        QAction &closeAction();
	QAction &settingsAction();
	QAction &aboutAction();
        QAction &exitAction();
        QAction &petsciiWindowAction();
	QAction &logWindowAction();
	QAction &fsOptionsAction();
	QAction &rewriteImageAction();
	QAction &autoMapLcAction();
	QAction &mapLcAction();
	QAction &newFileAction();
	QAction &deleteFileAction();
	QAction &fileOverridesAction();
        QAction &lowerCaseAction();
	const QString &instanceServerName() const;
	bool isPrimaryInstance() const;
	Settings &settings();

        static V1541Commander &instance();

    public slots:
	void open(const QString &filename);
	void saveAs();
	void show();

    signals:
        void lowerCaseChanged(bool lowerCase);
	void autoMapToLcChanged(bool autoMap);
};

#endif
