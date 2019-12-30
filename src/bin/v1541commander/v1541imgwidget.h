#ifndef V1541C_V1541IMGWIDGET_H
#define V1541C_V1541IMGWIDGET_H

#include <QWidget>

class QModelIndex;

class V1541ImgWidget: public QWidget
{
    Q_OBJECT

    private:
	class priv;
	priv *d;

    private slots:
	void selected(const QModelIndex &current, const QModelIndex &previous);
	void modelModified();

    public slots:
	void newImage();
	void open(const QString& filename);
	void save(const QString& filename);
	void newFile();
	void deleteFile();

    public:
	V1541ImgWidget();
	~V1541ImgWidget();
	bool hasValidImage() const;
	bool hasValidSelection() const;

    signals:
	void selectionChanged();
	void modified();
	void saved();
};

#endif
