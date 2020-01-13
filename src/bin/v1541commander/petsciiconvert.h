#ifndef V1541C_PETSCIICONVERT_H
#define V1541C_PETSCIICONVERT_H

#include <QChar>

namespace PetsciiConvert
{
    QChar unicodeToFont(const QChar &uc, bool lc, bool reverse = false);
    QChar petsciiToFont(unsigned char petscii, bool lc, bool reverse = false);
    unsigned char fontToPetscii(const QChar &fc, bool reverse = false);
}

#endif
