#include "settingsdialog.h"
#include "settings.h"
#include "v1541commander.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

class SettingsDialog::priv
{
    public:
	priv();
	void load();
	void save();
	QVBoxLayout mainLayout;
	QCheckBox rememberWindowPositions;
	QCheckBox exportAsPc64;
	QHBoxLayout defaultImportTypeLayout;
	QLabel defaultImportTypeLabel;
	QComboBox defaultImportType;
	QCheckBox warnDiskCapacity;
	QCheckBox warnDirCapacity;
	QDialogButtonBox buttons;
};

SettingsDialog::priv::priv() :
    mainLayout(),
    rememberWindowPositions(tr("remember window positions")),
    exportAsPc64(tr("drag && drop exports as PC64 file")),
    defaultImportTypeLayout(),
    defaultImportTypeLabel(tr("import unknown dropped files as: ")),
    defaultImportType(),
    warnDiskCapacity(tr("warn when disk would overflow")),
    warnDirCapacity(tr("warn when directory would overflow")),
    buttons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel
	    |QDialogButtonBox::Apply)
{}

void SettingsDialog::priv::load()
{
    rememberWindowPositions.setChecked(
	    cmdr.settings().rememberWindowPositions());
    exportAsPc64.setChecked(cmdr.settings().exportAsPc64());
    defaultImportType.setCurrentIndex(defaultImportType.findData(
		cmdr.settings().defaultImportType()));
    warnDiskCapacity.setChecked(cmdr.settings().warnDiskCapacity());
    warnDirCapacity.setChecked(cmdr.settings().warnDirCapacity());
}

void SettingsDialog::priv::save()
{
    cmdr.settings().setRememberWindowPositions(
	    rememberWindowPositions.isChecked());
    cmdr.settings().setExportAsPc64(exportAsPc64.isChecked());
    cmdr.settings().setDefaultImportType((CbmdosFileType)
	    defaultImportType.currentData().toInt());
    cmdr.settings().setWarnDiskCapacity(warnDiskCapacity.isChecked());
    cmdr.settings().setWarnDirCapacity(warnDirCapacity.isChecked());
}

SettingsDialog::SettingsDialog() :
    QDialog(0, Qt::WindowSystemMenuHint
	    | Qt::WindowTitleHint | Qt::WindowCloseButtonHint
	    | Qt::CustomizeWindowHint)
{
    d = new priv();
    d->mainLayout.addWidget(&d->rememberWindowPositions);
    d->mainLayout.addWidget(&d->exportAsPc64);
    d->defaultImportType.addItem("SEQ", CFT_SEQ);
    d->defaultImportType.addItem("PRG", CFT_PRG);
    d->defaultImportType.addItem("USR", CFT_USR);
    d->defaultImportType.addItem("REL", CFT_REL);
    d->defaultImportTypeLayout.addWidget(&d->defaultImportTypeLabel);
    d->defaultImportTypeLayout.addWidget(&d->defaultImportType);
    d->mainLayout.addLayout(&d->defaultImportTypeLayout);
    d->mainLayout.addWidget(&d->warnDiskCapacity);
    d->mainLayout.addWidget(&d->warnDirCapacity);
    d->mainLayout.addWidget(&d->buttons);
    setLayout(&d->mainLayout);
    setWindowTitle(tr("V1541Commander settings"));

    connect(&d->buttons, &QDialogButtonBox::accepted,
	    this, &SettingsDialog::save);
    connect(&d->buttons, &QDialogButtonBox::rejected,
	    this, &QDialog::reject);
    connect(&d->buttons, &QDialogButtonBox::clicked,
	    this, &SettingsDialog::buttonPressed);
}

SettingsDialog::~SettingsDialog()
{
    delete d;
}

void SettingsDialog::showEvent(QShowEvent *event)
{
    d->load();
    QDialog::showEvent(event);
    setFixedSize(size());
}

void SettingsDialog::save()
{
    d->save();
    hide();
}

void SettingsDialog::buttonPressed(QAbstractButton *button)
{
    if (d->buttons.buttonRole(button) == QDialogButtonBox::ApplyRole)
    {
	d->save();
    }
}

