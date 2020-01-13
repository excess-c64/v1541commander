#include "petsciiconvert.h"

QChar PetsciiConvert::unicodeToFont(const QChar &uc, bool lc, bool reverse)
{
    if (uc.isNull()) return QChar();
    ushort val = uc.unicode();
    if (val < 0x20) return QChar();
    if (val > 0xff && val < 0xe000) return QChar();
    if (val > 0xe3ff) return QChar();
    val &= 0xfeff;
    if (reverse)
    {
	if (!(val & 0x200))
	{
	    if (val < 0xe040
		    || (val >= 0xe060 && val < 0xe0c0)
		    || val >= 0xe0e0)
		val |= 0x200;
	}
    }
    else if (val & 0x200)
    {
	if (val < 0xe240
		|| (val >= 0xe260 && val < 0xe2c0)
		|| val >= 0xe2e0)
	    val &= ~0x200;
    }
    if (val < 0x100)
    {
	if (val > 0x40 && val < 0x5b) val += 0x20;
	else if (val > 0x60 && val < 0x7b) val -= 0x20;
	val |= 0xe000;
	if (reverse) val |= 0x200;
    }
    if (lc) val |= 0x100;
    return QChar(val);
}

QChar PetsciiConvert::petsciiToFont(unsigned char petscii,
        bool lc, bool reverse)
{
    ushort val;
    if (petscii < 0x20 || (petscii >= 0x80 && petscii < 0xa0))
	val = 0xe240 + petscii;
    else
	val = 0xe000 + petscii;

    if (reverse) val ^= 0x200;
    if (lc) val |= 0x100;
    return QChar(val);
}

unsigned char PetsciiConvert::fontToPetscii(const QChar &fc, bool reverse)
{
    ushort val = fc.unicode();
    if (reverse) val ^= 0x200;
    val &= 0xfeff;
    if (val < 0xe000 || val > 0xe2ff) return '?';
    if (val > 0xe200)
    {
	val -= 0xe200;
	if ((val >= 0x40 && val < 0x60) || (val >= 0xc0 && val < 0xe0))
	    val += (0xe000 - 0x40);
	else return '?';
    }
    return val & 0xff;
}
