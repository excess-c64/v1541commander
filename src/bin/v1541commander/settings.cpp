#include "settings.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QSettings>

class Settings::priv
{
    public:
	priv();
	~priv();
	QSettings qsettings;

	bool rememberWindowPositions;
	QByteArray mainGeometry;
	QByteArray petsciiGeometry;
	QByteArray logGeometry;
	bool lowercase;
	bool automapPetsciiToLc;
	bool exportAsPc64;
	CbmdosFileType defaultImportType;
	bool warnDiskCapacity;
	bool warnDirCapacity;
};

Settings::priv::priv() :
    qsettings(QCoreApplication::organizationName(),
	    QCoreApplication::applicationName())
{
    rememberWindowPositions = qsettings.value("rememberPos", true).toBool();
    mainGeometry = qsettings.value("geometry").toByteArray();
    petsciiGeometry = qsettings.value("petsciiGeometry").toByteArray();
    logGeometry = qsettings.value("logGeometry").toByteArray();
    lowercase = qsettings.value("lowercase", false).toBool();
    automapPetsciiToLc = qsettings.value("automap", false).toBool();
    exportAsPc64 = qsettings.value("exportPc64", false).toBool();
    defaultImportType = (CbmdosFileType)
	qsettings.value("importType", CFT_USR).toInt();
    if (defaultImportType < CFT_DEL || defaultImportType > CFT_REL)
	defaultImportType = CFT_USR;
    warnDiskCapacity = qsettings.value("warnDiskCapacity", true).toBool();
    warnDirCapacity = qsettings.value("warnDirCapacity", true).toBool();
}

Settings::priv::~priv()
{
    qsettings.setValue("rememberPos", rememberWindowPositions);
    qsettings.setValue("geometry", mainGeometry);
    qsettings.setValue("petsciiGeometry", petsciiGeometry);
    qsettings.setValue("logGeometry", logGeometry);
    qsettings.setValue("lowercase", lowercase);
    qsettings.setValue("automap", automapPetsciiToLc);
    qsettings.setValue("exportPc64", exportAsPc64);
    qsettings.setValue("importType", (int)defaultImportType);
    qsettings.setValue("warnDiskCapacity", warnDiskCapacity);
    qsettings.setValue("warnDirCapacity", warnDirCapacity);
}

Settings::Settings()
{
    d = new priv();
}

Settings::~Settings()
{
    delete d;
}

bool Settings::rememberWindowPositions() const
{
    return d->rememberWindowPositions;
}

void Settings::setRememberWindowPositions(bool remember)
{
    d->rememberWindowPositions = remember;
}

const QByteArray &Settings::mainGeometry() const
{
    return d->mainGeometry;
}

void Settings::setMainGeometry(const QByteArray &geometry)
{
    d->mainGeometry = geometry;
}

const QByteArray &Settings::petsciiGeometry() const
{
    return d->petsciiGeometry;
}

void Settings::setPetsciiGeometry(const QByteArray &geometry)
{
    d->petsciiGeometry = geometry;
}

const QByteArray &Settings::logGeometry() const
{
    return d->logGeometry;
}

void Settings::setLogGeometry(const QByteArray &geometry)
{
    d->logGeometry = geometry;
}

bool Settings::lowercase() const
{
    return d->lowercase;
}

void Settings::setLowercase(bool lowercase)
{
    d->lowercase = lowercase;
}

bool Settings::automapPetsciiToLc() const
{
    return d->automapPetsciiToLc;
}

void Settings::setAutomapPetsciiToLc(bool automap)
{
    d->automapPetsciiToLc = automap;
}

bool Settings::exportAsPc64() const
{
    return d->exportAsPc64;
}

void Settings::setExportAsPc64(bool exportPc64)
{
    d->exportAsPc64 = exportPc64;
}

CbmdosFileType Settings::defaultImportType() const
{
    return d->defaultImportType;
}

void Settings::setDefaultImportType(CbmdosFileType type)
{
    d->defaultImportType = type;
}

bool Settings::warnDiskCapacity() const
{
    return d->warnDiskCapacity;
}

void Settings::setWarnDiskCapacity(bool warn)
{
    d->warnDiskCapacity = warn;
}

bool Settings::warnDirCapacity() const
{
    return d->warnDirCapacity;
}

void Settings::setWarnDirCapacity(bool warn)
{
    d->warnDirCapacity = warn;
}

