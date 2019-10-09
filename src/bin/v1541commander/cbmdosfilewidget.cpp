#include "cbmdosfilewidget.h"
#include "petsciistr.h"
#include "petsciiedit.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

#include <1541img/cbmdosfile.h>

class CbmdosFileWidget::priv
{
    public:
	priv();
	CbmdosFile *file;
	QVBoxLayout layout;
	QHBoxLayout nameLayout;
	QLabel nameLabel;
	PetsciiEdit name;
	QHBoxLayout optionsLayout;
	QLabel typeLabel;
	QComboBox type;
	QCheckBox locked;
	QCheckBox closed;
	QHBoxLayout forcedBlocksLayout;
	QCheckBox forcedBlocksActive;
	QSpinBox forcedBlocks;
};

CbmdosFileWidget::priv::priv() :
    file(0),
    layout(),
    nameLayout(),
    nameLabel(tr("Name: ")),
    name(),
    optionsLayout(),
    typeLabel(tr("Type: ")),
    type(),
    locked(tr("locked")),
    closed(tr("closed")),
    forcedBlocksLayout(),
    forcedBlocksActive(tr("forced block size: ")),
    forcedBlocks()
{
}

CbmdosFileWidget::CbmdosFileWidget(QWidget *parent)
    : QGroupBox(tr("File properties"), parent)
{
    d = new priv();
    d->nameLayout.addWidget(&d->nameLabel);
    d->nameLayout.addWidget(&d->name);
    d->name.setMaxLength(16);
    d->type.addItem("DEL", CbmdosFileType::CFT_DEL);
    d->type.addItem("SEQ", CbmdosFileType::CFT_SEQ);
    d->type.addItem("PRG", CbmdosFileType::CFT_PRG);
    d->type.addItem("USR", CbmdosFileType::CFT_USR);
    d->optionsLayout.addWidget(&d->typeLabel);
    d->optionsLayout.addWidget(&d->type);
    d->optionsLayout.addWidget(&d->locked);
    d->optionsLayout.addWidget(&d->closed);
    d->forcedBlocks.setMinimum(0);
    d->forcedBlocks.setMaximum(0xfffe);
    d->forcedBlocks.setValue(0);
    d->forcedBlocks.setEnabled(false);
    d->forcedBlocksLayout.addWidget(&d->forcedBlocksActive);
    d->forcedBlocksLayout.addWidget(&d->forcedBlocks);
    d->layout.addLayout(&d->nameLayout);
    d->layout.addLayout(&d->optionsLayout);
    d->layout.addLayout(&d->forcedBlocksLayout);
    setLayout(&d->layout);
    setEnabled(false);
    connect(&d->name, &PetsciiEdit::petsciiEdited,
	    this, &CbmdosFileWidget::nameChanged);
    connect(&d->type, SIGNAL(currentIndexChanged(int)),
	    this, SLOT(typeChanged(int)));
    connect(&d->locked, &QCheckBox::stateChanged,
	    this, &CbmdosFileWidget::lockedChanged);
    connect(&d->closed, &QCheckBox::stateChanged,
	    this, &CbmdosFileWidget::closedChanged);
    connect(&d->forcedBlocksActive, &QCheckBox::stateChanged,
	    this, &CbmdosFileWidget::forcedBlocksActiveChanged);
    connect(&d->forcedBlocks, SIGNAL(valueChanged(int)),
	    this, SLOT(forcedBlocksValueChanged(int)));
}

CbmdosFileWidget::~CbmdosFileWidget()
{
    delete d;
}

void CbmdosFileWidget::nameChanged(const PetsciiStr &name)
{
    if (d->file)
    {
	CbmdosFile_setName(d->file, name.petscii(), name.length());
    }
}

void CbmdosFileWidget::typeChanged(int typeIndex)
{
    if (typeIndex >= 0 && d->file)
    {
	CbmdosFile_setType(d->file,
		(CbmdosFileType) d->type.currentData().toInt());
    }
}

void CbmdosFileWidget::lockedChanged(int lockedState)
{
    if (d->file)
    {
	CbmdosFile_setLocked(d->file, !!lockedState);
    }
}

void CbmdosFileWidget::closedChanged(int closedState)
{
    if (d->file)
    {
	CbmdosFile_setClosed(d->file, !!closedState);
    }
}

void CbmdosFileWidget::forcedBlocksActiveChanged(int activeState)
{
    if (d->file)
    {
	if (activeState)
	{
	    d->forcedBlocks.setEnabled(true);
	    d->forcedBlocks.setValue(0);
	    CbmdosFile_setForcedBlocks(d->file, 0);
	}
	else
	{
	    d->forcedBlocks.setEnabled(false);
	    d->forcedBlocks.setValue(0);
	    CbmdosFile_setForcedBlocks(d->file, 0xffff);
	}
    }
}

void CbmdosFileWidget::forcedBlocksValueChanged(int value)
{
    if (d->file && d->forcedBlocks.isEnabled())
    {
	CbmdosFile_setForcedBlocks(d->file, value);
    }
}

CbmdosFile *CbmdosFileWidget::file() const
{
    return d->file;
}

void CbmdosFileWidget::setFile(CbmdosFile *file)
{
    d->file = 0;
    if (file)
    {
	setEnabled(true);
	uint8_t nameLength;
	const char *name = CbmdosFile_name(file, &nameLength);
	PetsciiStr str(name, nameLength);
	d->name.setText(str.toString());
	d->type.setCurrentIndex(d->type.findData(CbmdosFile_type(file)));
	d->locked.setChecked(CbmdosFile_locked(file));
	d->closed.setChecked(CbmdosFile_closed(file));
	uint16_t forcedBlocks = CbmdosFile_forcedBlocks(file);
	if (forcedBlocks == 0xffff)
	{
	    d->forcedBlocksActive.setChecked(false);
	    d->forcedBlocks.setEnabled(false);
	    d->forcedBlocks.setValue(0);
	}
	else
	{
	    d->forcedBlocksActive.setChecked(true);
	    d->forcedBlocks.setEnabled(true);
	    d->forcedBlocks.setValue(forcedBlocks);
	}
    }
    else
    {
	setEnabled(false);
	d->name.setText(QString());
    }
    d->file = file;
}
