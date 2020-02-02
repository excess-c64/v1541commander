#include "cbmdosfilewidget.h"
#include "petsciistr.h"
#include "petsciiedit.h"
#include "settings.h"
#include "utils.h"
#include "v1541commander.h"

#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#ifdef _WIN32
#include <QFileInfo>
#endif
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QRegExp>
#include <QShortcut>
#include <QSpinBox>
#include <QVBoxLayout>

#include <1541img/cbmdosfile.h>
#include <1541img/cbmdosfileeventargs.h>
#include <1541img/event.h>
#include <1541img/filedata.h>
#include <1541img/petscii.h>

static void evhdl(void *receiver, int id, const void *sender, const void *args)
{
    (void) id;
    (void) sender;

    CbmdosFileWidget *widget = (CbmdosFileWidget *)receiver;
    const CbmdosFileEventArgs *eventArgs = (const CbmdosFileEventArgs *)args;
    widget->fileChanged(eventArgs);
}

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
	QHBoxLayout recordLengthLayout;
	QLabel recordLengthLabel;
	QSpinBox recordLength;
	QHBoxLayout forcedBlocksLayout;
	QCheckBox forcedBlocksActive;
	QSpinBox forcedBlocks;
	QHBoxLayout dataLayout;
	QLabel dataLabel;
	QLabel dataSizeLabel;
	QPushButton importButton;
	QPushButton exportButton;
	int lastNameCursorPos;
	bool ignoreNameCursorPos;

        void setDataSize(size_t size);
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
    recordLengthLayout(),
    recordLengthLabel(tr("record length (for REL files): ")),
    recordLength(),
    forcedBlocksLayout(),
    forcedBlocksActive(tr("forced block size: ")),
    forcedBlocks(),
    dataLayout(),
    dataLabel(tr("Content: ")),
    dataSizeLabel(tr("<none>")),
    importButton(tr("import")),
    exportButton(tr("export")),
    lastNameCursorPos(0),
    ignoreNameCursorPos(false)
{
}

void CbmdosFileWidget::priv::setDataSize(size_t size)
{
    dataSizeLabel.setText(QString("%1 KiB")
            .arg(QLocale::system().toString(size/1024.0, 'f', 3), -5, '0'));
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
    d->type.addItem("REL", CbmdosFileType::CFT_REL);
    d->optionsLayout.addWidget(&d->typeLabel);
    d->optionsLayout.addWidget(&d->type);
    d->optionsLayout.addWidget(&d->locked);
    d->optionsLayout.addWidget(&d->closed);
    d->recordLength.setMinimum(1);
    d->recordLength.setMaximum(254);
    d->recordLength.setValue(254);
    d->recordLengthLayout.addWidget(&d->recordLengthLabel);
    d->recordLengthLayout.addWidget(&d->recordLength);
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
    d->layout.addLayout(&d->recordLengthLayout);
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
    connect(&d->recordLength, SIGNAL(valueChanged(int)),
	    this, SLOT(recordLengthChanged(int)));
    connect(&d->forcedBlocksActive, &QCheckBox::stateChanged,
	    this, &CbmdosFileWidget::forcedBlocksActiveChanged);
    connect(&d->forcedBlocks, SIGNAL(valueChanged(int)),
	    this, SLOT(forcedBlocksValueChanged(int)));
    connect(&d->importButton, &QPushButton::clicked,
	    this, &CbmdosFileWidget::importFile);
    connect(&d->exportButton, &QPushButton::clicked,
	    this, &CbmdosFileWidget::exportFile);

    QShortcut *f2 = new QShortcut(QKeySequence(Qt::Key_F2), this);
    connect(f2, &QShortcut::activated, this, [this](){
		d->name.selectAll();
		d->name.setFocus();
	    });
    d->name.setToolTip(tr("The name of the file (F2)"));
    d->nameLabel.setToolTip(tr("The name of the file (F2)"));
    QShortcut *f8 = new QShortcut(QKeySequence(Qt::Key_F8), this);
    connect(f8, &QShortcut::activated, this, [this](){
	    d->type.setCurrentIndex(0);});
    QShortcut *f9 = new QShortcut(QKeySequence(Qt::Key_F9), this);
    connect(f9, &QShortcut::activated, this, [this](){
	    d->type.setCurrentIndex(1);});
    QShortcut *f10 = new QShortcut(QKeySequence(Qt::Key_F10), this);
    connect(f10, &QShortcut::activated, this, [this](){
	    d->type.setCurrentIndex(2);});
    QShortcut *f11 = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(f11, &QShortcut::activated, this, [this](){
	    d->type.setCurrentIndex(3);});
    QShortcut *f12 = new QShortcut(QKeySequence(Qt::Key_F12), this);
    connect(f12, &QShortcut::activated, this, [this](){
	    d->type.setCurrentIndex(4);});
    d->type.setToolTip(tr("The type of the file (F8-F12)"));
    d->typeLabel.setToolTip(tr("The type of the file (F8-F12)"));
    QShortcut *csl = new QShortcut(
	    QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_L), this);
    connect(csl, &QShortcut::activated, &d->locked, &QCheckBox::click);
    d->locked.setToolTip(tr("File locked flag (Ctrl+Shift+L)"));
    QShortcut *csc = new QShortcut(
	    QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_C), this);
    connect(csc, &QShortcut::activated, &d->closed, &QCheckBox::click);
    d->closed.setToolTip(tr("File closed flag (Ctrl+Shift+C)"));
    QShortcut *f7 = new QShortcut(QKeySequence(Qt::Key_F7), this);
    connect(f7, &QShortcut::activated, this, [this](){
		d->recordLength.selectAll();
		d->recordLength.setFocus();
	    });
    d->recordLength.setToolTip(
	    tr("The length of a record in a REL file (F7)"));
    d->recordLengthLabel.setToolTip(
	    tr("The length of a record in a REL file (F7)"));
    QShortcut *sf6 = new QShortcut(QKeySequence(Qt::SHIFT+Qt::Key_F6), this);
    connect(sf6, &QShortcut::activated,
	    &d->forcedBlocksActive, &QCheckBox::click);
    d->forcedBlocksActive.setToolTip(
	    tr("Enable fake display of block size (Shift+F6)"));
    QShortcut *f6 = new QShortcut(QKeySequence(Qt::Key_F6), this);
    connect(f6, &QShortcut::activated, this, [this](){
		if (!d->forcedBlocksActive.isChecked())
		{
		    d->forcedBlocksActive.click();
		}
		d->forcedBlocks.selectAll();
		d->forcedBlocks.setFocus();
	    });
    d->forcedBlocks.setToolTip(tr("Fake block size to display (F6)"));
    QShortcut *csi = new QShortcut(
	    QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_I), this);
    connect(csi, &QShortcut::activated, &d->importButton, &QPushButton::click);
    d->importButton.setToolTip(tr("Import file contents (Ctrl+Shift+I)"));
    QShortcut *cse = new QShortcut(
	    QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_E), this);
    connect(cse, &QShortcut::activated, &d->exportButton, &QPushButton::click);
    d->exportButton.setToolTip(tr("Export file contents (Ctrl+Shift+E)"));

    connect(&d->name, &QLineEdit::cursorPositionChanged,
	    this, [this](int oldPos, int newPos){
		if (!d->ignoreNameCursorPos)
		{
		    (void)oldPos;
		    d->lastNameCursorPos = newPos;
		}
	    });

    connect(&cmdr, &V1541Commander::lowerCaseChanged,
            this, [this](bool lowerCase){
                d->ignoreNameCursorPos = true;
                d->name.updateCase(lowerCase);
                d->ignoreNameCursorPos = false;
            });
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
		d->setDataSize(size);
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

void CbmdosFileWidget::recordLengthChanged(int value)
{
    if (d->file)
    {
	CbmdosFile_setRecordLength(d->file, value);
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
#ifdef _WIN32
	    return QCoreApplication::translate("CbmdosFileWidget",
                    "PRG files (*.prg);;P00 files (*.p00);;all files (*)");
#else
	    return QCoreApplication::translate("CbmdosFileWidget",
                    "PRG files (*.prg);;"
                    "P00 files (*.p[0-9][0-9]);;all files (*)");
#endif
	case CbmdosFileType::CFT_SEQ:
#ifdef _WIN32
	    return QCoreApplication::translate("CbmdosFileWidget",
                    "SEQ files (*.seq);;S00 files (*.s00);;all files (*)");
#else
	    return QCoreApplication::translate("CbmdosFileWidget",
                    "SEQ files (*.seq);;"
                    "S00 files (*.s[0-9][0-9]);;all files (*)");
#endif
	case CbmdosFileType::CFT_USR:
#ifdef _WIN32
	    return QCoreApplication::translate("CbmdosFileWidget",
                    "USR files (*.usr);;U00 files (*.u00);;all files (*)");
#else
	    return QCoreApplication::translate("CbmdosFileWidget",
                    "USR files (*.usr);;"
                    "U00 files (*.u[0-9][0-9]);;all files (*)");
#endif
	case CbmdosFileType::CFT_REL:
#ifdef _WIN32
	    return QCoreApplication::translate("CbmdosFileWidget",
                    "R00 files (*.r00);;all files (*)");
#else
	    return QCoreApplication::translate("CbmdosFileWidget", 
                        "R00 files (*.r[0-9][0-9]);;all files (*)");
#endif
	default:
	    return QCoreApplication::translate("CbmdosFileWidget",
                    "all files (*)");
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
	    if (CbmdosFile_import(d->file, f) >= 0)
	    {
		d->setDataSize(FileData_size(CbmdosFile_rdata(d->file)));
		d->exportButton.setEnabled(true);
		uint8_t nameLength;
		const char *name = CbmdosFile_name(d->file, &nameLength);
		d->name.setPetscii(PetsciiStr(name, nameLength));
		d->recordLength.setValue(CbmdosFile_recordLength(d->file));
	    }
	    else
	    {
		QMessageBox::critical(this, tr("Error reading file"),
			tr("The selected file couldn't be read."));
	    }
	    fclose(f);
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
    uint8_t namelen;
    const char *name = CbmdosFile_name(d->file, &namelen);
    char utf8name[65];
    petscii_toUtf8(utf8name, 65, name, namelen,
	    cmdr.settings().lowercase(), 1, 0, 0);
    QString hostFile = QFileDialog::getSaveFileName(this, tr("Export file"),
	    qfnsan(QString::fromUtf8(utf8name)),
            getFilterForType(CbmdosFile_type(d->file)));
    if (!hostFile.isEmpty())
    {
#ifdef _WIN32
	QFileInfo fileInfo(hostFile);
	if (fileInfo.completeSuffix() != fileInfo.suffix())
	{
	    QFileInfo corrected(fileInfo.dir(), fileInfo.completeBaseName());
	    hostFile = corrected.filePath();
	}
#endif
	QRegExp pc64pat("\\.[PpSsUuRr]\\d{2}");
	bool ispc64 = pc64pat.indexIn(hostFile, -4) > 0;
	if (!ispc64 && CbmdosFile_type(d->file) == CbmdosFileType::CFT_REL
		&& QMessageBox::question(this,
		    tr("Export REL file as raw data?"),
		    tr("You are about to export a REL file as raw data. To "
			"export it as PC64 R00 file instead, specify a "
			"matching file extension (like .R00, .R01, .R02, "
			"etc.)\n"
			"Exporting as raw data will lose the record length. "
			"Do you still want to export it as raw data?"),
		    QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
	{
	    return;
	}
	FILE *f = qfopen(hostFile, "wb");
	if (f)
	{
	    int rc = 0;
	    if (ispc64) rc = CbmdosFile_exportPC64(d->file, f);
	    else rc = CbmdosFile_exportRaw(d->file, f);
	    fclose(f);
	    if (rc < 0)
	    {
		QMessageBox::critical(this, tr("Error writing file"),
			tr("The selected file couldn't be written."));
	    }
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
    if (d->file)
    {
	Event_unregister(CbmdosFile_changedEvent(d->file), this, evhdl);
    }
    d->file = 0;
    bool hadSelectedText = d->name.hasSelectedText();
    d->ignoreNameCursorPos = true;
    if (file)
    {
	setEnabled(true);
	uint8_t nameLength;
	const char *name = CbmdosFile_name(file, &nameLength);
	d->name.setPetscii(PetsciiStr(name, nameLength));
	CbmdosFileType type = CbmdosFile_type(file);
	d->type.setCurrentIndex(d->type.findData(type));
	d->locked.setChecked(CbmdosFile_locked(file));
	d->closed.setChecked(CbmdosFile_closed(file));
	d->recordLength.setValue(CbmdosFile_recordLength(file));
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
	    d->setDataSize(size);
	    d->importButton.setEnabled(true);
	    d->exportButton.setEnabled(true);
	}
	Event_register(CbmdosFile_changedEvent(file), this, evhdl);
    }
    else
    {
	setEnabled(false);
	d->name.setText(QString());
	d->dataSizeLabel.setText(tr("<none>"));
    }
    d->file = file;
    if (hadSelectedText)
    {
	d->name.selectAll();
    }
    else
    {
	d->name.setCursorPosition(d->lastNameCursorPos);
    }
    d->ignoreNameCursorPos = false;
}

void CbmdosFileWidget::fileChanged(const CbmdosFileEventArgs *args)
{
    if (args->what == CbmdosFileEventArgs::CFE_NAMECHANGED)
    {
	d->ignoreNameCursorPos = true;
	uint8_t nameLength;
	const char *name = CbmdosFile_name(d->file, &nameLength);
	d->name.setPetscii(PetsciiStr(name, nameLength), true);
	d->ignoreNameCursorPos = false;
    }
}
