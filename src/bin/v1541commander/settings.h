#ifndef V1541C_SETTINGS_H
#define V1541C_SETTINGS_H

class QByteArray;

#include <1541img/cbmdosfile.h>

class Settings
{
    private:
	class priv;
	priv *d;

    public:
	Settings();
	~Settings();

	bool rememberWindowPositions() const;
	void setRememberWindowPositions(bool remember);

	const QByteArray &mainGeometry() const;
	void setMainGeometry(const QByteArray &geometry);

	const QByteArray &petsciiGeometry() const;
	void setPetsciiGeometry(const QByteArray &geometry);

	const QByteArray &logGeometry() const;
	void setLogGeometry(const QByteArray &geometry);

	bool lowercase() const;
	void setLowercase(bool lowercase);

	bool automapPetsciiToLc() const;
	void setAutomapPetsciiToLc(bool automap);

	bool exportAsPc64() const;
	void setExportAsPc64(bool exportPc64);

	CbmdosFileType defaultImportType() const;
	void setDefaultImportType(CbmdosFileType type);

	bool warnDiskCapacity() const;
	void setWarnDiskCapacity(bool warn);

	bool warnDirCapacity() const;
	void setWarnDirCapacity(bool warn);
};

#endif
