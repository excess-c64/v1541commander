#ifndef V1541C_PETSCIIBUTTON_H
#define V1541C_PETSCIIBUTTON_H

#include <QLabel>

class QEvent;

class PetsciiButton : public QLabel
{
    Q_OBJECT

    public:
        PetsciiButton(ushort petscii, QWidget *parent = 0);

    protected:
        void enterEvent(QEvent *ev);
        void leaveEvent(QEvent *ev);
        void mousePressEvent(QEvent *ev);
};

#endif
