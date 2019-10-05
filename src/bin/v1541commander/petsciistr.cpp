#include "petsciistr.h"

#include <QChar>

PetsciiStr::PetsciiStr(const char *raw, uint8_t len)
    : raw(raw), len(len)
{}

QString PetsciiStr::toString(bool lower, bool reverse)
{
    ushort offset = 0xe000;
    if (lower) offset |= 0x100;
    if (reverse) offset |= 0x200;
    ushort rvoff = offset ^ 0x200;

    QString result;
    for (int i = 0; i < len; ++i)
    {
	unsigned char rawc = (unsigned char)raw[i];
	if (rawc < 0x20 || (rawc >= 0x80 && rawc < 0xa0))
	{
	    result.append(QChar(rvoff | (rawc + 0x40)));
	}
	else
	{
	    result.append(QChar(offset | rawc));
	}
    }

    return result;
}
