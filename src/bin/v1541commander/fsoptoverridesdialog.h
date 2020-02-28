#ifndef V1541C_FSOPTOVERRIDESDIALOG_H
#define V1541C_FSOPTOVERRIDESDIALOG_H

#include <QDialog>

struct CbmdosFsOptOverrides;
class QAbstractButton;
class QShowEvent;

class FsOptOverridesDialog: public QDialog
{
    Q_OBJECT

    private:
	class priv;
	priv *d;

    public:
	FsOptOverridesDialog(CbmdosFsOptOverrides *overrides,
		QWidget *parent = nullptr);
	~FsOptOverridesDialog();

    protected:
	virtual void showEvent(QShowEvent *event);

    public slots:
	virtual void accept();
};

#endif
