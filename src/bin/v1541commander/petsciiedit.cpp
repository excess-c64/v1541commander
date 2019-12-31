#include "petsciiedit.h"
#include "petsciiconvert.h"
#include "petsciistr.h"
#include "v1541commander.h"

#include <QKeyEvent>
#include <QStyle>

static const ushort cbmLetterChars[] =
{
    0xe0b0,
    0xe0bf,
    0xe0bc,
    0xe0ac,
    0xe0b1,
    0xe0bb,
    0xe0a5,
    0xe0b4,
    0xe0a2,
    0xe0b5,
    0xe0a1,
    0xe0b6,
    0xe0a7,
    0xe0aa,
    0xe0b9,
    0xe0af,
    0xe0ab,
    0xe0b2,
    0xe0ae,
    0xe0a3,
    0xe0b8,
    0xe0be,
    0xe0b3,
    0xe0bd,
    0xe0b7,
    0xe0ad
};

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

void PetsciiEdit::keyPressEvent(QKeyEvent *event)
{
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    if (mods == Qt::AltModifier)
    {
	int key = event->key();
	if (key >= Qt::Key_A && key <= Qt::Key_Z)
	{
	    petsciiInput(cbmLetterChars[key - Qt::Key_A]);
	}
    }
    else QLineEdit::keyPressEvent(event);
}

bool PetsciiEdit::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
    {
	Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
	if (mods == Qt::AltModifier)
	{
	    int key = static_cast<QKeyEvent *>(event)->key();
	    if (key >= Qt::Key_A && key <= Qt::Key_Z)
	    {
		event->accept();
	    }
	}
    }
    return QLineEdit::event(event);
}
