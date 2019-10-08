#ifndef V1541C_PETSCIIEDIT_H
#define V1541C_PETSCIIEDIT_H

#include <QLineEdit>

class PetsciiEdit : public QLineEdit
{
    Q_OBJECT

    private slots:
	void editText(const QString &text);

    public:
	PetsciiEdit(QWidget *parent = 0);
};

#endif
