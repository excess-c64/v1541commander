#ifndef V1541C_PETSCIISTR_H
#define V1541C_PETSCIISTR_H

#include <cstdint>
#include <QString>

class PetsciiStr
{
    private:
	char *raw;
	uint8_t len;

    public:
	PetsciiStr(const char *raw, uint8_t len);
	PetsciiStr(const QString &str, bool reverse = false);
	~PetsciiStr();
	QString toString(bool lc, bool reverse = false);
	const char *petscii() const;
	uint8_t length() const;
};

#endif
