#include "petsciiedit.h"
#include "petsciiconvert.h"

PetsciiEdit::PetsciiEdit(QWidget *parent) :
    QLineEdit(QString(), parent)
{
    connect(this, &QLineEdit::textEdited, this, &PetsciiEdit::editText);
}

void PetsciiEdit::editText(const QString &text)
{
    int pos = cursorPosition();
    QString translated;
    for (QString::const_iterator i = text.begin(); i != text.end(); ++i)
    {
	QChar tc = PetsciiConvert::unicodeToFont(*i);
	if (tc.isNull())
	{
	    if (pos > 0) --pos;
	}
	else
	{
	    translated.append(tc);
	}
    }
    setText(translated);
    setCursorPosition(pos);
}
