#ifndef V1541C_PETSCIISTR_H
#define V1541C_PETSCIISTR_H

#include <cstdint>
#include <QString>

class PetsciiStr
{
    private:
	const char *raw;
	uint8_t len;

    public:
	PetsciiStr(const char *raw, uint8_t len);
	QString toString(bool lower = 0, bool reverse = 0);
};

#endif
