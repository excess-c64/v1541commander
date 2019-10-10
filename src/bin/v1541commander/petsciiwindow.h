#ifndef V1541C_PETSCIIWINDOW_H
#define V1541C_PETSCIIWINDOW_H

#include <QWidget>

class PetsciiWindow : public QWidget
{
    Q_OBJECT

    signals:
	void petsciiInput(ushort val);

    private slots:
	void buttonClicked(ushort val);

    public:
        PetsciiWindow(QWidget *parent = 0);
};

#endif
