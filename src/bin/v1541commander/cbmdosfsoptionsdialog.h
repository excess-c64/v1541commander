#ifndef V1541C_CBMDOSFSOPTIONSDIALOG_H
#define V1541C_CBMDOSFSOPTIONSDIALOG_H

#include <QDialog>

struct CbmdosFsOptions;
class QAbstractButton;
class QShowEvent;

class CbmdosFsOptionsDialog: public QDialog
{
    Q_OBJECT

    private:
	class priv;
	priv *d;

    public:
	CbmdosFsOptionsDialog(CbmdosFsOptions *options,
		QWidget *parent = nullptr, bool canCancel = true);
	~CbmdosFsOptionsDialog();
	void disable35Tracks();
	void disable40Tracks();
	void disable42Tracks();
	void disableZeroFree();
	void reset();

    protected:
	virtual void showEvent(QShowEvent *event);

    public slots:
	virtual void accept();

    private slots:
	void buttonPressed(QAbstractButton *button);
	void optionClicked(bool checked);
	void valueChanged(int i);
};

#endif
