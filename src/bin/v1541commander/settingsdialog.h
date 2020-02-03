#ifndef V1541C_SETTINGSDIALOG_H
#define V1541C_SETTINGSDIALOG_H

#include <QDialog>

class QAbstractButton;
class QShowEvent;

class SettingsDialog: public QDialog
{
    Q_OBJECT

    private:
	class priv;
	priv *d;

    public:
	SettingsDialog();
	virtual ~SettingsDialog();

    protected:
	virtual void showEvent(QShowEvent *event);

    private slots:
	virtual void save();
	void buttonPressed(QAbstractButton *button);
};

#endif
