#include "petsciistr.h"

#include <QChar>

PetsciiStr::PetsciiStr(const char *raw, uint8_t len)
    : raw(raw), len(len)
{}

QString PetsciiStr::toString(bool lower, bool reverse)
{
    ushort offset = 0xe000;
    if (lower) offset += 0x100;
    if (reverse) offset += 0x200;

    QString result;
    for (int i = 0; i < len; ++i)
    {
	result.append(QChar(offset + (unsigned char)raw[i]));
    }

    return result;
}
