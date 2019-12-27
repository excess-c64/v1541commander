#include "petsciiedit.h"
#include "petsciiconvert.h"
#include "petsciistr.h"
#include "v1541commander.h"

#include <QStyle>

PetsciiEdit::PetsciiEdit(QWidget *parent) :
    QLineEdit(QString(), parent)
{
    setFont(cmdr.c64font());
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
    PetsciiStr petscii(translated);
    emit petsciiEdited(petscii);
}

void PetsciiEdit::petsciiInput(ushort val)
{
    insert(QString(val));
}

void PetsciiEdit::setMaxLength(int length)
{
    QFontMetricsF fm(font());
    setMinimumWidth((length+1) * fm.averageCharWidth()
	    + 2 * cmdr.style()->pixelMetric(QStyle::PM_DefaultFrameWidth));
    QLineEdit::setMaxLength(length);
}
