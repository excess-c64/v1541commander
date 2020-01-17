#include "cbmdosfsoptionsdialog.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWindow>

#include <1541img/cbmdosfsoptions.h>

class CbmdosFsOptionsDialog::priv
{
    public:
	priv(CbmdosFsOptions *options, bool canCancel);
	CbmdosFsOptions *options;
	CbmdosFsOptions opts;
	QVBoxLayout mainLayout;
	QGridLayout optionsLayout;
	QRadioButton tracks35Button;
	QRadioButton tracks40Button;
	QRadioButton tracks42Button;
	QCheckBox allowLongDirCheckBox;
	QCheckBox filesOnDirTrackCheckBox;
	QCheckBox dolphinDosBamCheckBox;
	QCheckBox speedDosBamCheckBox;
	QCheckBox prologicDosBamCheckBox;
	QCheckBox zeroFreeCheckBox;
	QLabel dirInterleaveLabel;
	QSpinBox dirInterleaveSpinBox;
	QLabel fileInterleaveLabel;
	QSpinBox fileInterleaveSpinBox;
	QLabel recoverWarningLabel;
	QDialogButtonBox buttons;

	void reset();
	void clicked(QObject *sender, bool checked);
	void changed(QObject *sender, int value);
};

CbmdosFsOptionsDialog::priv::priv(CbmdosFsOptions *options, bool canCancel) :
    options(options),
    opts(),
    mainLayout(),
    optionsLayout(),
    tracks35Button(tr("35 tracks")),
    tracks40Button(tr("40 tracks")),
    tracks42Button(tr("42 tracks")),
    allowLongDirCheckBox(tr("allow long directories")),
    filesOnDirTrackCheckBox(tr("place files on dir track")),
    dolphinDosBamCheckBox(tr("DolphinDOS BAM")),
    speedDosBamCheckBox(tr("SpeedDOS BAM")),
    prologicDosBamCheckBox(tr("PrologicDOS BAM")),
    zeroFreeCheckBox(tr("report 0 blocks free")),
    dirInterleaveLabel(tr("directory interleave: ")),
    dirInterleaveSpinBox(),
    fileInterleaveLabel(tr("default file interleave: ")),
    fileInterleaveSpinBox(),
    recoverWarningLabel(tr("WARNING: this disk image is broken, trying to "
		"recover data from it. It will be recreated after opening, "
		"but data could be lost. It is therefore treated like a new "
		"disk image.")),
    buttons(canCancel ?
	    QDialogButtonBox::Ok|QDialogButtonBox::Cancel
	    |QDialogButtonBox::Reset
	    : QDialogButtonBox::Ok|QDialogButtonBox::Reset)
{
    dirInterleaveSpinBox.setMinimum(1);
    dirInterleaveSpinBox.setMaximum(20);
    fileInterleaveSpinBox.setMinimum(1);
    fileInterleaveSpinBox.setMaximum(20);
    recoverWarningLabel.setWordWrap(true);
    reset();
}

void CbmdosFsOptionsDialog::priv::reset()
{
    memcpy(&opts, options, sizeof opts);
    if (opts.flags & CbmdosFsFlags::CFF_42TRACK)
    {
	tracks42Button.setChecked(true);
	dolphinDosBamCheckBox.setEnabled(true);
	speedDosBamCheckBox.setEnabled(true);
	prologicDosBamCheckBox.setEnabled(true);
    }
    else if (opts.flags & CbmdosFsFlags::CFF_40TRACK)
    {
	tracks40Button.setChecked(true);
	dolphinDosBamCheckBox.setEnabled(true);
	speedDosBamCheckBox.setEnabled(true);
	prologicDosBamCheckBox.setEnabled(true);
    }
    else
    {
	tracks35Button.setChecked(true);
	dolphinDosBamCheckBox.setEnabled(false);
	speedDosBamCheckBox.setEnabled(false);
	prologicDosBamCheckBox.setEnabled(false);
    }
    allowLongDirCheckBox.setChecked(
	    opts.flags & CbmdosFsFlags::CFF_ALLOWLONGDIR);
    filesOnDirTrackCheckBox.setChecked(
	    opts.flags & CbmdosFsFlags::CFF_FILESONDIRTRACK);
    dolphinDosBamCheckBox.setChecked(
	    opts.flags & CbmdosFsFlags::CFF_DOLPHINDOSBAM);
    speedDosBamCheckBox.setChecked(
	    opts.flags & CbmdosFsFlags::CFF_SPEEDDOSBAM);
    prologicDosBamCheckBox.setChecked(
	    opts.flags & CbmdosFsFlags::CFF_PROLOGICDOSBAM);
    zeroFreeCheckBox.setChecked(
	    opts.flags & CbmdosFsFlags::CFF_ZEROFREE);
    dirInterleaveSpinBox.setValue(opts.dirInterleave);
    fileInterleaveSpinBox.setValue(opts.fileInterleave);
}

void CbmdosFsOptionsDialog::priv::clicked(QObject *sender, bool checked)
{
    if (sender == &tracks35Button)
    {
	opts.flags = (CbmdosFsFlags) (opts.flags & ~(
		CbmdosFsFlags::CFF_40TRACK |
		CbmdosFsFlags::CFF_42TRACK |
		CbmdosFsFlags::CFF_DOLPHINDOSBAM |
		CbmdosFsFlags::CFF_SPEEDDOSBAM |
		CbmdosFsFlags::CFF_PROLOGICDOSBAM
		));
	dolphinDosBamCheckBox.setChecked(false);
	dolphinDosBamCheckBox.setEnabled(false);
	speedDosBamCheckBox.setChecked(false);
	speedDosBamCheckBox.setEnabled(false);
	prologicDosBamCheckBox.setChecked(false);
	prologicDosBamCheckBox.setEnabled(false);

    }
    else if (sender == &tracks40Button)
    {
	opts.flags = (CbmdosFsFlags)
	    (opts.flags & ~CbmdosFsFlags::CFF_42TRACK);
	opts.flags = (CbmdosFsFlags)
	    (opts.flags | CbmdosFsFlags::CFF_40TRACK);
	dolphinDosBamCheckBox.setEnabled(true);
	speedDosBamCheckBox.setEnabled(true);
	prologicDosBamCheckBox.setEnabled(true);
    }
    else if (sender == &tracks42Button)
    {
	opts.flags = (CbmdosFsFlags)
	    (opts.flags & ~CbmdosFsFlags::CFF_40TRACK);
	opts.flags = (CbmdosFsFlags)
	    (opts.flags | CbmdosFsFlags::CFF_42TRACK);
	dolphinDosBamCheckBox.setEnabled(true);
	speedDosBamCheckBox.setEnabled(true);
	prologicDosBamCheckBox.setEnabled(true);
    }
    else if (sender == &dolphinDosBamCheckBox)
    {
	if (checked)
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags & ~CbmdosFsFlags::CFF_PROLOGICDOSBAM);
	    opts.flags = (CbmdosFsFlags)
		(opts.flags | CbmdosFsFlags::CFF_DOLPHINDOSBAM);
	    prologicDosBamCheckBox.setChecked(false);
	}
	else
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags & ~CbmdosFsFlags::CFF_DOLPHINDOSBAM);
	}
    }
    else if (sender == &speedDosBamCheckBox)
    {
	if (checked)
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags & ~CbmdosFsFlags::CFF_PROLOGICDOSBAM);
	    opts.flags = (CbmdosFsFlags)
		(opts.flags | CbmdosFsFlags::CFF_SPEEDDOSBAM);
	    prologicDosBamCheckBox.setChecked(false);
	}
	else
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags & ~CbmdosFsFlags::CFF_SPEEDDOSBAM);
	}
    }
    else if (sender == &prologicDosBamCheckBox)
    {
	if (checked)
	{
	    opts.flags = (CbmdosFsFlags) (opts.flags & ~(
		    CbmdosFsFlags::CFF_SPEEDDOSBAM |
		    CbmdosFsFlags::CFF_DOLPHINDOSBAM
		    ));
	    opts.flags = (CbmdosFsFlags)
		(opts.flags | CbmdosFsFlags::CFF_PROLOGICDOSBAM);
	    speedDosBamCheckBox.setChecked(false);
	    dolphinDosBamCheckBox.setChecked(false);
	}
	else
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags & ~CbmdosFsFlags::CFF_PROLOGICDOSBAM);
	}
    }
    else if (sender == &allowLongDirCheckBox)
    {
	if (checked)
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags | CbmdosFsFlags::CFF_ALLOWLONGDIR);
	}
	else
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags & ~CbmdosFsFlags::CFF_ALLOWLONGDIR);
	}
    }
    else if (sender == &filesOnDirTrackCheckBox)
    {
	if (checked)
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags | CbmdosFsFlags::CFF_FILESONDIRTRACK);
	}
	else
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags & ~CbmdosFsFlags::CFF_FILESONDIRTRACK);
	}
    }
    else if (sender == &zeroFreeCheckBox)
    {
	if (checked)
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags | CbmdosFsFlags::CFF_ZEROFREE);
	}
	else
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags & ~CbmdosFsFlags::CFF_ZEROFREE);
	}
    }
}

void CbmdosFsOptionsDialog::priv::changed(QObject *sender, int value)
{
    if (sender == &dirInterleaveSpinBox)
    {
	opts.dirInterleave = value;
    }
    else if (sender == &fileInterleaveSpinBox)
    {
	opts.fileInterleave = value;
    }
}

CbmdosFsOptionsDialog::CbmdosFsOptionsDialog(CbmdosFsOptions *options,
	QWidget *parent, bool canCancel) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint |
	    Qt::CustomizeWindowHint)
{
    d = new priv(options, canCancel);
    d->optionsLayout.addWidget(&d->tracks35Button, 0, 0, 1, 2);
    d->optionsLayout.addWidget(&d->tracks40Button, 1, 0, 1, 2);
    d->optionsLayout.addWidget(&d->tracks42Button, 2, 0, 1, 2);
    d->optionsLayout.addWidget(&d->dirInterleaveLabel, 4, 0);
    d->optionsLayout.addWidget(&d->dirInterleaveSpinBox, 4, 1);
    d->optionsLayout.addWidget(&d->fileInterleaveLabel, 5, 0);
    d->optionsLayout.addWidget(&d->fileInterleaveSpinBox, 5, 1);
    d->optionsLayout.addWidget(&d->dolphinDosBamCheckBox, 0, 2);
    d->optionsLayout.addWidget(&d->speedDosBamCheckBox, 1, 2);
    d->optionsLayout.addWidget(&d->prologicDosBamCheckBox, 2, 2);
    d->optionsLayout.addWidget(&d->allowLongDirCheckBox, 3, 2);
    d->optionsLayout.addWidget(&d->filesOnDirTrackCheckBox, 4, 2);
    d->optionsLayout.addWidget(&d->zeroFreeCheckBox, 5, 2);
    if (options->flags & CbmdosFsFlags::CFF_RECOVER)
    {
	d->optionsLayout.addWidget(&d->recoverWarningLabel, 6, 0, 1, 3);
    }
    d->mainLayout.addLayout(&d->optionsLayout);
    d->mainLayout.addWidget(&d->buttons);
    setLayout(&d->mainLayout);

    connect(&d->buttons, &QDialogButtonBox::accepted,
	    this, &CbmdosFsOptionsDialog::accept);
    connect(&d->buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(&d->buttons, &QDialogButtonBox::clicked, this,
	    &CbmdosFsOptionsDialog::buttonPressed);

    connect(&d->tracks35Button, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->tracks40Button, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->tracks42Button, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->dolphinDosBamCheckBox, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->speedDosBamCheckBox, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->prologicDosBamCheckBox, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->allowLongDirCheckBox, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->filesOnDirTrackCheckBox, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->zeroFreeCheckBox, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);

    connect(&d->dirInterleaveSpinBox, SIGNAL(valueChanged(int)),
	    this, SLOT(valueChanged(int)));
    connect(&d->fileInterleaveSpinBox, SIGNAL(valueChanged(int)),
	    this, SLOT(valueChanged(int)));
}

CbmdosFsOptionsDialog::~CbmdosFsOptionsDialog()
{
    delete d;
}

void CbmdosFsOptionsDialog::accept()
{
    memcpy(d->options, &d->opts, sizeof d->opts);
    QDialog::accept();
}

void CbmdosFsOptionsDialog::buttonPressed(QAbstractButton *button)
{
    if (d->buttons.buttonRole(button) == QDialogButtonBox::ResetRole)
    {
	d->reset();
    }
}

void CbmdosFsOptionsDialog::optionClicked(bool checked)
{
    d->clicked(sender(), checked);
}

void CbmdosFsOptionsDialog::valueChanged(int i)
{
    d->changed(sender(), i);
}

void CbmdosFsOptionsDialog::disable35Tracks()
{
    d->tracks35Button.setEnabled(false);
}

void CbmdosFsOptionsDialog::disable40Tracks()
{
    d->tracks40Button.setEnabled(false);
}

void CbmdosFsOptionsDialog::disable42Tracks()
{
    d->tracks42Button.setEnabled(false);
}

void CbmdosFsOptionsDialog::disableZeroFree()
{
    d->zeroFreeCheckBox.setEnabled(false);
}

void CbmdosFsOptionsDialog::reset()
{
    d->reset();
}

void CbmdosFsOptionsDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    adjustSize();
    setFixedSize(size());
    QCoreApplication::processEvents();

    QRect dlgRect = frameGeometry();
    QRect mainWinRect = parentWidget()->window()->frameGeometry();
    dlgRect.moveCenter(mainWinRect.center());

    const QScreen *screen = 0;
    const QWindow *currentWin = parentWidget()->windowHandle();
    if (currentWin)
    {
	screen = currentWin->screen();
    }
    if (screen)
    {
	QRect screenRect = screen->availableGeometry();
	if (dlgRect.right() > screenRect.right())
	{
	    dlgRect.moveRight(screenRect.right());
	}
	if (dlgRect.bottom() > screenRect.bottom())
	{
	    dlgRect.moveBottom(screenRect.bottom());
	}
	if (dlgRect.top() < screenRect.top())
	{
	    dlgRect.moveTop(screenRect.top());
	}
	if (dlgRect.left() < screenRect.left())
	{
	    dlgRect.moveLeft(screenRect.left());
	}
    }
    move(dlgRect.topLeft());
}

