#ifndef V1541C_V1541IMGWIDGET_H
#define V1541C_V1541IMGWIDGET_H

#include <QWidget>

struct D64;
struct CbmdosFs;
class QLabel;

class V1541ImgWidget: public QWidget
{
    Q_OBJECT

    private:
	D64 *d64;
	CbmdosFs *fs;
	QLabel *label;

    public slots:
	void open(const QString& filename);

    public:
	V1541ImgWidget();
	bool hasValidImage() const;
};

#endif
