#ifndef V1541C_CBMDOSFILEWIDGET_H
#define V1541C_CBMDOSFILEWIDGET_H

#include <QGroupBox>

struct CbmdosFile;

class CbmdosFileWidget: public QGroupBox
{
    Q_OBJECT

    private:
	class priv;
	priv *d;

    public:
	CbmdosFileWidget(QWidget *parent = 0);
	~CbmdosFileWidget();

	CbmdosFile *file() const;
	void setFile(CbmdosFile *file);
};

#endif