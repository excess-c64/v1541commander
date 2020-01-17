#ifndef V1541C_PETSCIIWINDOW_H
#define V1541C_PETSCIIWINDOW_H

#include <QWidget>

class PetsciiButton;
class QShowEvent;

class PetsciiWindow : public QWidget
{
    Q_OBJECT

    private:
        PetsciiButton *buttons[0x100];

    signals:
	void petsciiInput(ushort val);

    private slots:
	void buttonClicked(ushort val);

    public:
        PetsciiWindow(const QFont &c64font, QWidget *parent = 0);
        void setLowercase(bool lowerCase);

    protected:
	virtual void showEvent(QShowEvent *event);
};

#endif
