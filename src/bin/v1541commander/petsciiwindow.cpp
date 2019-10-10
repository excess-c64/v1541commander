#include "petsciiwindow.h"
#include "petsciibutton.h"

#include <QGridLayout>

#ifdef _WIN32
#include <windows.h>
#endif

PetsciiWindow::PetsciiWindow(QWidget *parent) :
    QWidget(parent)
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(1);
    for (ushort row = 0; row < 0x10; ++row)
    {
        for (ushort col = 0; col < 0x10; ++col)
        {
            ushort petscii = (row << 4) | col;
            if (petscii < 0x20 || (petscii  >= 0x80 && petscii < 0xa0))
            {
                petscii += 0xe240;
            }
            else
            {
                petscii += 0xe000;
            }
            PetsciiButton *button = new PetsciiButton(petscii, this);
	    connect(button, &PetsciiButton::clicked,
		    this, &PetsciiWindow::buttonClicked);
            layout->addWidget(button, row, col);
        }
    }
    setLayout(layout);
    setWindowTitle(tr("PETSCII Input"));
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::WindowDoesNotAcceptFocus);
#ifdef _WIN32
    SetWindowLongPtr(HWND(winId()), GWL_EXSTYLE, WS_EX_NOACTIVATE);
#endif
}

void PetsciiWindow::buttonClicked(ushort val)
{
    emit petsciiInput(val);
}
