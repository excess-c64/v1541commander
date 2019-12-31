#include "cbmdosfilewidget.h"
#include "petsciistr.h"
#include "petsciiedit.h"
#include "utils.h"

#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

#include <1541img/cbmdosfile.h>
#include <1541img/filedata.h>
#include <1541img/hostfilereader.h>
#include <1541img/hostfilewriter.h>

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
	QHBoxLayout dataLayout;
	QLabel dataLabel;
	QLabel dataSizeLabel;
	QPushButton importButton;
	QPushButton exportButton;
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
    forcedBlocks(),
    dataLayout(),
    dataLabel(tr("Content: ")),
    dataSizeLabel(tr("<none>")),
    importButton(tr("import")),
    exportButton(tr("export"))
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
    d->dataLayout.addWidget(&d->dataLabel);
    d->dataLayout.addWidget(&d->dataSizeLabel);
    d->dataLayout.addWidget(&d->importButton);
    d->dataLayout.addWidget(&d->exportButton);
    d->layout.addLayout(&d->nameLayout);
    d->layout.addLayout(&d->optionsLayout);
    d->layout.addLayout(&d->forcedBlocksLayout);
    d->layout.addLayout(&d->dataLayout);
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
    connect(&d->importButton, &QPushButton::clicked,
	    this, &CbmdosFileWidget::importFile);
    connect(&d->exportButton, &QPushButton::clicked,
	    this, &CbmdosFileWidget::exportFile);
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
	CbmdosFileType type = (CbmdosFileType) d->type.currentData().toInt();
	if (type == CbmdosFileType::CFT_DEL)
	{
	    const FileData *data = CbmdosFile_rdata(d->file);
	    size_t size = FileData_size(data);
	    if (size)
	    {
		QMessageBox::StandardButton reply = QMessageBox::question(
			this, tr("Erase file contents?"), tr("Changing the "
			"file type to DEL will erase all file contents. "
			"Proceed anyways?"),
			QMessageBox::Ok|QMessageBox::Cancel);
		if (reply != QMessageBox::Ok)
		{
		    d->type.setCurrentIndex(d->type.findData(
				CbmdosFile_type(d->file)));
		    return;
		}
	    }
	}
	CbmdosFile_setType(d->file, type);
	if (type == CbmdosFileType::CFT_DEL)
	{
	    d->dataSizeLabel.setText(tr("<none>"));
	    d->importButton.setEnabled(false);
	    d->exportButton.setEnabled(false);
	}
	else
	{
	    const FileData *data = CbmdosFile_rdata(d->file);
	    size_t size = FileData_size(data);
	    if (!size)
	    {
		d->dataSizeLabel.setText(tr("<none>"));
		d->importButton.setEnabled(true);
		d->exportButton.setEnabled(false);
	    }
	    else
	    {
		d->dataSizeLabel.setText(QString("%1 KiB")
			.arg(size/1024.0, 5, 'f', 3));
		d->importButton.setEnabled(true);
		d->exportButton.setEnabled(true);
	    }
	}
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

static QString getFilterForType(CbmdosFileType type)
{
    switch (type)
    {
	case CbmdosFileType::CFT_PRG:
	    return QString(QT_TR_NOOP("PRG files (*.prg);;all files (*)"));
	case CbmdosFileType::CFT_SEQ:
	    return QString(QT_TR_NOOP("SEQ files (*.seq);;all files (*)"));
	case CbmdosFileType::CFT_USR:
	    return QString(QT_TR_NOOP("USR files (*.usr);;all files (*)"));
	default:
	    return QString(QT_TR_NOOP("all files (*)"));
    }
}

void CbmdosFileWidget::importFile()
{
    if (CbmdosFile_type(d->file) == CbmdosFileType::CFT_DEL) return;
    QString hostFile = QFileDialog::getOpenFileName(this, tr("Import file"),
	    QString(), getFilterForType(CbmdosFile_type(d->file)));
    if (!hostFile.isEmpty())
    {
	FILE *f = qfopen(hostFile, "rb");
	if (f)
	{
	    FileData *data = readHostFile(f);
	    fclose(f);
	    if (data)
	    {
		CbmdosFile_setData(d->file, data);
		d->dataSizeLabel.setText(QString("%1 KiB")
			.arg(FileData_size(data)/1024.0, 5, 'f', 3));
		d->exportButton.setEnabled(true);
	    }
	    else
	    {
		QMessageBox::critical(this, tr("Error reading file"),
			tr("The selected file couldn't be read."));
	    }
	}
	else
	{
	    QMessageBox::critical(this, tr("Error opening file"),
		    tr("The selected file couldn't be opened for reading."));
	}
    }
}

void CbmdosFileWidget::exportFile()
{
    if (CbmdosFile_type(d->file) == CbmdosFileType::CFT_DEL) return;
    const FileData *data = CbmdosFile_rdata(d->file);
    if (!data || !FileData_size(data)) return;
    QString hostFile = QFileDialog::getSaveFileName(this, tr("Export file"),
	    QString(), getFilterForType(CbmdosFile_type(d->file)));
    if (!hostFile.isEmpty())
    {
	FILE *f = qfopen(hostFile, "wb");
	if (f)
	{
	    if (writeHostFile(data, f) < 0)
	    {
		QMessageBox::critical(this, tr("Error writing file"),
			tr("The selected file couldn't be written."));
	    }
	    fclose(f);
	}
	else
	{
	    QMessageBox::critical(this, tr("Error opening file"),
		    tr("The selected file couldn't be opened for writing."));
	}
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
	CbmdosFileType type = CbmdosFile_type(file);
	d->type.setCurrentIndex(d->type.findData(type));
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
	const FileData *fd = CbmdosFile_rdata(file);
	size_t size = FileData_size(fd);
	if (type == CbmdosFileType::CFT_DEL)
	{
	    d->dataSizeLabel.setText(tr("<none>"));
	    d->importButton.setEnabled(false);
	    d->exportButton.setEnabled(false);
	}
	else if (!size)
	{
	    d->dataSizeLabel.setText(tr("<none>"));
	    d->importButton.setEnabled(true);
	    d->exportButton.setEnabled(false);
	}
	else
	{
	    d->dataSizeLabel.setText(QString("%1 KiB")
		    .arg(size/1024.0, 5, 'f', 3));
	    d->importButton.setEnabled(true);
	    d->exportButton.setEnabled(true);
	}
    }
    else
    {
	setEnabled(false);
	d->name.setText(QString());
	d->dataSizeLabel.setText(tr("<none>"));
    }
    d->file = file;
}
