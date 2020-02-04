#ifndef V1541C_MAINWINDOW_H
#define V1541C_MAINWINDOW_H

#include <QMainWindow>

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QEvent;
class QWidget;

struct CbmdosVfs;

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
        virtual ~MainWindow();
        Content content() const;
	const QString &filename() const;
	bool hasValidContent() const;
	bool hasValidSelection() const;
        bool isReadOnly() const;
	CbmdosVfs *exportZipcodeVfs();
        virtual bool event(QEvent *e);
        virtual void closeEvent(QCloseEvent *e);
        virtual QSize sizeHint() const;

    protected:
        virtual void dragEnterEvent(QDragEnterEvent *event);
        virtual void dragMoveEvent(QDragMoveEvent *event);
        virtual void dropEvent(QDropEvent *event);

    signals:
        void activated();
        void closed();
        void contentChanged();
	void modifiedChanged();
	void selectionChanged();

    public slots:
	void newImage();
        void openImage(const QString &imgFile);
	void openVfs(CbmdosVfs *vfs);
	void save(const QString &imgFile = QString());
	void exportZipcode(const QString &zcFile);
	void exportLynx(const QString &lynxFile);
        void closeDocument();
	void fsOptions();
	void rewriteImage();
	void mapToLc();
	void newFile();
	void deleteFile();
	void showStatusLine(const QString &line);
};

#endif
