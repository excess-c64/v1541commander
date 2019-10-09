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

    public slots:
	void open(const QString& filename);
	void save(const QString& filename);

    public:
	V1541ImgWidget();
	~V1541ImgWidget();
	bool hasValidImage() const;
};

#endif
