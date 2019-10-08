#ifndef V1541C_PETSCIIEDIT_H
#define V1541C_PETSCIIEDIT_H

#include <QLineEdit>

class PetsciiStr;

class PetsciiEdit : public QLineEdit
{
    Q_OBJECT

    private slots:
	void editText(const QString &text);

    signals:
	void petsciiEdited(const PetsciiStr &petscii);

    public:
	PetsciiEdit(QWidget *parent = 0);
	virtual void setMaxLength(int length);
};

#endif
