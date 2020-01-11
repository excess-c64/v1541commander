#include "cbmdosfswidget.h"
#include "petsciistr.h"
#include "petsciiedit.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>
#include <QSpinBox>
#include <QVBoxLayout>

#include <1541img/cbmdosfs.h>
#include <1541img/cbmdosvfs.h>

class CbmdosFsWidget::priv
{
    public:
	priv();
	CbmdosFs *fs;
	QVBoxLayout layout;
	QHBoxLayout nameLayout;
	QLabel nameLabel;
	PetsciiEdit name;
	QHBoxLayout idLayout;
	QLabel idLabel;
	PetsciiEdit id;
	QLabel dosVerLabel;
	QSpinBox dosVer;
	QPushButton dosVerReset;
};

CbmdosFsWidget::priv::priv() :
    fs(0),
    layout(),
    nameLayout(),
    nameLabel(tr("Name: ")),
    name(),
    idLayout(),
    idLabel(tr("ID: ")),
    id(),
    dosVerLabel(tr("DOS Version:")),
    dosVer(),
    dosVerReset(tr("reset"))
{
}

CbmdosFsWidget::CbmdosFsWidget(QWidget *parent)
    : QGroupBox(tr("Disk properties"), parent)
{
    d = new priv();
    d->name.setMaxLength(16);
    d->nameLayout.addWidget(&d->nameLabel);
    d->nameLayout.addWidget(&d->name);
    d->id.setMaxLength(5);
    d->dosVer.setMinimum(0);
    d->dosVer.setMaximum(0xff);
    d->dosVer.setDisplayIntegerBase(16);
    d->idLayout.addWidget(&d->idLabel);
    d->idLayout.addWidget(&d->id);
    d->idLayout.addWidget(&d->dosVerLabel);
    d->idLayout.addWidget(&d->dosVer);
    d->idLayout.addWidget(&d->dosVerReset);
    d->layout.addLayout(&d->nameLayout);
    d->layout.addLayout(&d->idLayout);
    setLayout(&d->layout);
    setEnabled(false);
    connect(&d->name, &PetsciiEdit::petsciiEdited,
	    this, &CbmdosFsWidget::nameChanged);
    connect(&d->id, &PetsciiEdit::petsciiEdited,
	    this, &CbmdosFsWidget::idChanged);
    connect(&d->dosVer, SIGNAL(valueChanged(int)),
	    this, SLOT(dosVerChanged(int)));
    connect(&d->dosVerReset, &QPushButton::clicked,
	    this, &CbmdosFsWidget::dosVerReset);

    QShortcut *sf2 = new QShortcut(QKeySequence(Qt::SHIFT+Qt::Key_F2), this);
    connect(sf2, &QShortcut::activated, this, [this](){
	    d->name.selectAll();
	    d->name.setFocus();
	    });
    d->name.setToolTip(tr("The name of the disk (Shift+F2)"));
    QShortcut *sf3 = new QShortcut(QKeySequence(Qt::SHIFT+Qt::Key_F3), this);
    connect(sf3, &QShortcut::activated, this, [this](){
	    d->id.selectAll();
	    d->id.setFocus();
	    });
    d->id.setToolTip(tr("The ID of the disk (Shift+F3)"));
    QShortcut *f4 = new QShortcut(QKeySequence(Qt::Key_F4), this);
    connect(f4, &QShortcut::activated, this, [this](){
	    d->dosVer.selectAll();
	    d->dosVer.setFocus();
	    });
    d->dosVer.setToolTip(tr("DOS version, non-default value is \"soft write "
		"protection\" (F4)"));
    QShortcut *sf4 = new QShortcut(QKeySequence(Qt::SHIFT+Qt::Key_F4), this);
    connect(sf4, &QShortcut::activated, &d->dosVerReset, &QPushButton::click);
    d->dosVerReset.setToolTip(tr("Reset DOS version to default value "
		"(Shift+F4)"));
}

CbmdosFsWidget::~CbmdosFsWidget()
{
    delete d;
}

void CbmdosFsWidget::nameChanged(const PetsciiStr &name)
{
    if (d->fs)
    {
	CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
	CbmdosVfs_setName(vfs, name.petscii(), name.length());
    }
}

void CbmdosFsWidget::idChanged(const PetsciiStr &id)
{
    if (d->fs)
    {
	CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
	CbmdosVfs_setId(vfs, id.petscii(), id.length());
    }
}

static uint8_t defaultDosVer(const CbmdosFs *fs)
{
    if (!fs) return 0x41;
    if (CbmdosFs_options(fs).flags & CbmdosFsFlags::CFF_PROLOGICDOSBAM)
    {
	return 0x50;
    }
    else return 0x41;
}

void CbmdosFsWidget::dosVerChanged(int val)
{
    if (d->fs)
    {
	d->dosVerReset.setEnabled(val != defaultDosVer(d->fs));
	CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
	CbmdosVfs_setDosver(vfs, val);
    }
}

void CbmdosFsWidget::dosVerReset()
{
    d->dosVer.setValue(defaultDosVer(d->fs));
}

CbmdosFs *CbmdosFsWidget::fs() const
{
    return d->fs;
}

void CbmdosFsWidget::setFs(CbmdosFs *fs)
{
    d->fs = 0;
    if (fs)
    {
	setEnabled(true);
	const CbmdosVfs *vfs = CbmdosFs_rvfs(fs);
	uint8_t len;
	const char *str = CbmdosVfs_name(vfs, &len);
	PetsciiStr name(str, len);
	d->name.setText(name.toString());
	str = CbmdosVfs_id(vfs, &len);
	PetsciiStr id(str, len);
	d->id.setText(id.toString());
	uint8_t dosver = CbmdosVfs_dosver(vfs);
	d->dosVer.setValue(dosver);
	d->dosVerReset.setEnabled(dosver != defaultDosVer(fs));
    }
    else
    {
	setEnabled(false);
	d->name.setText(QString());
	d->id.setText(QString());
	d->dosVer.setValue(0);
    }
    d->fs = fs;
}

