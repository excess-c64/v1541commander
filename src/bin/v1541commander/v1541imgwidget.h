#ifndef V1541C_V1541IMGWIDGET_H
#define V1541C_V1541IMGWIDGET_H

#include <QWidget>

class EditOperationCheck;
class QKeyEvent;
class QModelIndex;

struct CbmdosVfs;

class V1541ImgWidget: public QWidget
{
    Q_OBJECT

    private:
	class priv;
	priv *d;

    private slots:
	void selected(const QModelIndex &current, const QModelIndex &previous);
	void modelModified();
	void checkEditOperation(EditOperationCheck &check);

    protected:
	virtual void keyPressEvent(QKeyEvent *event);

    public slots:
	void newImage();
	void open(const QString& filename);
	void openVfs(CbmdosVfs *vfs);
	void save(const QString& filename);
	void exportZipcode(const QString &filename);
	void exportLynx(const QString &filename);
	void fsOptions();
	void rewriteImage();
	void mapToLc();
	void newFile();
	void deleteFile(bool skipConfirmation = false);
	void fileOverrides();

    public:
	V1541ImgWidget(QWidget *parent = nullptr);
	~V1541ImgWidget();
        QWidget *statusWidget() const;
	bool hasValidImage() const;
	bool hasValidSelection() const;
        bool isReadOnly() const;
	CbmdosVfs *exportZipcodeVfs();

    signals:
	void selectionChanged();
	void modified();
	void saved();
};

#endif
