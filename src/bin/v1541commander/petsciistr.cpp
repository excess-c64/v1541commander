#include "petsciistr.h"
#include "petsciiconvert.h"

#include <QChar>

PetsciiStr::PetsciiStr(const char *raw, uint8_t len)
    : raw(raw), len(len)
{}

QString PetsciiStr::toString(bool reverse)
{
    QString result;
    for (int i = 0; i < len; ++i)
    {
	result.append(PetsciiConvert::petsciiToFont(raw[i], reverse));
    }

    return result;
}
