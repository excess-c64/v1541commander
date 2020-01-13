#include "petsciistr.h"
#include "petsciiconvert.h"

#include <cstring>

#include <QChar>

PetsciiStr::PetsciiStr(const char *raw, uint8_t len)
{
    this->raw = new char[len];
    memcpy(this->raw, raw, len);
    this->len = len;
}

PetsciiStr::PetsciiStr(const QString &str, bool reverse)
{
    raw = new char[str.count()];
    int i = 0;
    for (QString::const_iterator j = str.begin(); j != str.end(); ++i, ++j)
    {
	raw[i] = PetsciiConvert::fontToPetscii(*j, reverse);
    }
    len = i;
}

PetsciiStr::~PetsciiStr()
{
    delete[] raw;
}

QString PetsciiStr::toString(bool lc, bool reverse)
{
    QString result;
    for (int i = 0; i < len; ++i)
    {
	result.append(PetsciiConvert::petsciiToFont(raw[i], lc, reverse));
    }

    return result;
}

const char *PetsciiStr::petscii() const
{
    return raw;
}

uint8_t PetsciiStr::length() const
{
    return len;
}

