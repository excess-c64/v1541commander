#ifndef V1541C_ABOUTBOX_H
#define V1541C_ABOUTBOX_H

#include <QDialog>

class QFont;

class AboutBox: public QDialog
{
    Q_OBJECT

    private:
	class priv;
	priv *d;

    protected:
	virtual void showEvent(QShowEvent *event);

    public:
	AboutBox(const QFont &c64font);
	~AboutBox();
};

#endif
