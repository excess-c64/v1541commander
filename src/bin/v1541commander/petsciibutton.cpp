#include "petsciibutton.h"
#include "v1541commander.h"

#include <QKeyEvent>

PetsciiButton::PetsciiButton(ushort petscii, QWidget *parent) :
    QLabel(parent)
{
    setFont(app.c64font());
    setText(QString(petscii));
    setStyleSheet("QLabel { padding: 4px 2px; background-color: white; }");
}

void PetsciiButton::enterEvent(QEvent *ev)
{
    (void) ev;
    setStyleSheet("QLabel { padding: 4px 2px; "
            "background-color : lightskyblue; color: blue; }");
}

void PetsciiButton::leaveEvent(QEvent *ev)
{
    (void) ev;
    setStyleSheet("QLabel { padding: 4px 2px; background-color: white; }");
}

void PetsciiButton::mousePressEvent(QEvent *ev)
{
    (void) ev;
    QCoreApplication::postEvent(QApplication::activeWindow(), new QKeyEvent(
                QEvent::KeyPress, 0, 0, 0, 0, 0, text()));
}
