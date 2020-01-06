#ifndef V1541C_MAINWINDOW_H
#define V1541C_MAINWINDOW_H

#include <QMainWindow>

class QWidget;
class QEvent;

class MainWindow: public QMainWindow
{
    Q_OBJECT

    private:
        class priv;
        priv *d;

    private slots:
	void contentSelectionChanged();
	void contentModified();
	void contentSaved();

    public:
        enum Content
        {
            None,
            Image
        };

	MainWindow();
        ~MainWindow();
        Content content() const;
	const QString &filename() const;
	bool hasValidContent() const;
	bool hasValidSelection() const;
        virtual bool event(QEvent *e);
        virtual void closeEvent(QCloseEvent *e);
        virtual QSize sizeHint() const;

    signals:
        void activated();
        void closed();
        void contentChanged();
	void selectionChanged();

    public slots:
	void newImage();
        void openImage(const QString &imgFile);
	void save(const QString &imgFile = QString());
        void closeDocument();
	void fsOptions();
	void rewriteImage();
	void newFile();
	void deleteFile();
	void showStatusLine(const QString &line);
};

#endif
