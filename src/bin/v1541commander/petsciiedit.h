#ifndef V1541C_PETSCIIEDIT_H
#define V1541C_PETSCIIEDIT_H

#include <QLineEdit>

class PetsciiStr;
class QKeyEvent;

class PetsciiEdit : public QLineEdit
{
    Q_OBJECT

    public slots:
	void petsciiInput(ushort val);
        void updateCase(bool lowerCase);

    private slots:
	void editText(const QString &text);

    signals:
	void petsciiEdited(const PetsciiStr &petscii);

    public:
	PetsciiEdit(QWidget *parent = 0);
	virtual void setMaxLength(int length);

    protected:
	virtual void keyPressEvent(QKeyEvent *event);
	virtual bool event(QEvent *event);
};

#endif
