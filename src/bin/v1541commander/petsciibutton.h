#ifndef V1541C_PETSCIIBUTTON_H
#define V1541C_PETSCIIBUTTON_H

#include <QLabel>

class QEvent;
class QMouseEvent;

class PetsciiButton : public QLabel
{
    Q_OBJECT

    public:
        PetsciiButton(ushort petscii, QWidget *parent = 0);

    signals:
	void clicked(ushort val);

    protected:
        void enterEvent(QEvent *ev);
        void leaveEvent(QEvent *ev);
        void mousePressEvent(QMouseEvent *ev);
};

#endif
