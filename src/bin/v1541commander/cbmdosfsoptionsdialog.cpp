#include "cbmdosfsoptionsdialog.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
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
	QGroupBox allocationOptions;
	QGridLayout allocationLayout;
	QRadioButton allocOriginalButton;
	QRadioButton allocTrackloaderButton;
	QRadioButton allocSimpleButton;
	QCheckBox simpleInterleaveCheckBox;
	QCheckBox prefDirTrackCheckBox;
	QCheckBox chainInterlvCheckBox;
	QLabel recoverWarningLabel;
        QCheckBox rewriteAfterRecover;
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
    allocationOptions(tr("Track/block allocation options")),
    allocationLayout(),
    allocOriginalButton(tr("CBM DOS strategy")),
    allocTrackloaderButton(tr("Trackloader strategy")),
    allocSimpleButton(tr("Simple strategy")),
    simpleInterleaveCheckBox(tr("Use simple interleave")),
    prefDirTrackCheckBox(tr("Prefer dir track for files")),
    chainInterlvCheckBox(tr("Apply interleave on track change")),
    recoverWarningLabel(tr("WARNING: this disk image is broken, trying to "
		"recover data from it. It will be treated like a new disk "
                "image. It's recommended to rewrite the disk image after "
                "recovery, so you can edit it.")),
    rewriteAfterRecover(tr("Rewrite image after recovery")),
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
    tracks35Button.setToolTip(
	    tr("Filesystem uses 35 tracks on disk (default)."));
    tracks40Button.setToolTip(
	    tr("Filesystem uses 40 tracks on disk, like some speeder DOS "
		"versions do."));
    tracks42Button.setToolTip(
	    tr("Filesystem uses 42 tracks on disk.\nNOT RECOMMENDED, not all "
		"drives can physically access these tracks."));
    allowLongDirCheckBox.setToolTip(
	    tr("Allow directories longer than 144 files.\n"
		"Directory blocks will be allocated on tracks other than "
		"track 18.\n"
		"The original 1541 DOS can read this, but corrupts the "
		"directory on write."));
    filesOnDirTrackCheckBox.setToolTip(
	    tr("Allow to use track 18 also for files when all other tracks "
		"are full.\n"
		"The original 1541 DOS can still read such a filesystem."));
    dolphinDosBamCheckBox.setToolTip(
	    tr("Write BAM entries for tracks 36-40 in the format "
		"DolphinDOS uses."));
    speedDosBamCheckBox.setToolTip(
	    tr("Write BAM entries for tracks 36-40 in the format "
		"SpeedDOS uses."));
    prologicDosBamCheckBox.setToolTip(
	    tr("Write BAM entries for tracks 36-40 in the format "
		"Prologic DOS uses.\n"
		"This is NOT compatible with the other extended "
		"BAM formats."));
    zeroFreeCheckBox.setToolTip(
	    tr("Write 0 as the number of free blocks per track for every "
		"track in the BAM.\n"
		"As a result, the directory listing shows "
		"\"0 blocks free.\""));
    QString dirInterleaveToolTip(
	    tr("Interleave value to use for blocks of the directory "
		"(default: 3)"));
    QString fileInterleaveToolTip(
	    tr("Interleave value to use by default for blocks of files "
		"(default: 10)"));
    dirInterleaveLabel.setToolTip(dirInterleaveToolTip);
    dirInterleaveSpinBox.setToolTip(dirInterleaveToolTip);
    fileInterleaveLabel.setToolTip(fileInterleaveToolTip);
    fileInterleaveSpinBox.setToolTip(fileInterleaveToolTip);
    allocOriginalButton.setToolTip(
	    tr("Use the original CBM DOS strategy for allocating blocks.\n"
		"For starting a new file, a track as close as possible to "
		"track 18 is used.\n"
		"While writing a file, when the current track is full, the "
		"next track in the direction away from track 18 is used.\n"
		"Good for random access to different files, with the files "
		"at the top of the directory preferred."));
    allocTrackloaderButton.setToolTip(
	    tr("Use an allocation strategy often used with trackloaders.\n"
		"Tracks are used starting at track 1 and strictly "
		"incrementing.\n"
		"Good for a set of files that's always read in the same "
		"order."));
    allocSimpleButton.setToolTip(
	    tr("Use a simplified allocation strategy.\n"
		"Tracks are used starting at track 19 and strictly "
		"incrementing up to track 40, then wrapping around to "
		"track 1.\n"
		"On 42-track images, tracks 41 and 42 are still used last.\n"
		"This is a compromise between the other strategies, with "
		"the first files close to track 18, and the files "
		"\"following\" each other on disk."));
    simpleInterleaveCheckBox.setToolTip(
	    tr("When adding interleave, use a simple \"modulo\" to get a "
		"valid sector number on the current track.\n"
		"If this is not checked, interleave is applied like the "
		"original CBM DOS does it:\n"
		"When wrapping over sector 0, 1 is subtracted from "
		"the result."));
    prefDirTrackCheckBox.setToolTip(
	    tr("Put files on the directory track first.\n"
		"This has only an effect if files on the directory track "
		"are allowed.\n"
		"When this option is set, you should \"rewrite\" your image "
		"from time to time,\n"
		"to ensure exactly the sectors on track 18 not used by the "
		"directory are allocated."));
    chainInterlvCheckBox.setToolTip(
	    tr("When having to switch to another track, still apply "
		"interleave to the sector number.\n"
		"If this is not checked, looking for a free sector on a new "
		"track\n"
		"always starts at sector 0 (original CBM DOS behavior)."));
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
    if (opts.flags & CbmdosFsFlags::CFF_FILESONDIRTRACK)
    {
	filesOnDirTrackCheckBox.setChecked(true);
	prefDirTrackCheckBox.setChecked(
		opts.flags & CbmdosFsFlags::CFF_TALLOC_PREFDIRTRACK);
	prefDirTrackCheckBox.setEnabled(true);
    }
    else
    {
	filesOnDirTrackCheckBox.setChecked(false);
	prefDirTrackCheckBox.setChecked(false);
	prefDirTrackCheckBox.setEnabled(false);
    }
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
    if (opts.flags & CbmdosFsFlags::CFF_TALLOC_TRACKLOAD)
    {
	allocTrackloaderButton.setChecked(true);
    }
    else if (opts.flags & CbmdosFsFlags::CFF_TALLOC_SIMPLE)
    {
	allocSimpleButton.setChecked(true);
    }
    else
    {
	allocOriginalButton.setChecked(true);
    }
    simpleInterleaveCheckBox.setChecked(
	    opts.flags & CbmdosFsFlags::CFF_SIMPLEINTERLEAVE);
    chainInterlvCheckBox.setChecked(
	    opts.flags & CbmdosFsFlags::CFF_TALLOC_CHAININTERLV);
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
	    prefDirTrackCheckBox.setEnabled(true);
	}
	else
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags & ~CbmdosFsFlags::CFF_FILESONDIRTRACK);
	    prefDirTrackCheckBox.setEnabled(false);
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
    else if (sender == &allocOriginalButton)
    {
	opts.flags = (CbmdosFsFlags) (opts.flags & ~(
		CbmdosFsFlags::CFF_TALLOC_TRACKLOAD |
		CbmdosFsFlags::CFF_TALLOC_SIMPLE
		));
    }
    else if (sender == &allocTrackloaderButton)
    {
	opts.flags = (CbmdosFsFlags)
	    (opts.flags & ~CbmdosFsFlags::CFF_TALLOC_SIMPLE);
	opts.flags = (CbmdosFsFlags)
	    (opts.flags | CbmdosFsFlags::CFF_TALLOC_TRACKLOAD);
    }
    else if (sender == &allocSimpleButton)
    {
	opts.flags = (CbmdosFsFlags)
	    (opts.flags & ~CbmdosFsFlags::CFF_TALLOC_TRACKLOAD);
	opts.flags = (CbmdosFsFlags)
	    (opts.flags | CbmdosFsFlags::CFF_TALLOC_SIMPLE);
    }
    else if (sender == &simpleInterleaveCheckBox)
    {
	if (checked)
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags | CbmdosFsFlags::CFF_SIMPLEINTERLEAVE);
	}
	else
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags & ~CbmdosFsFlags::CFF_SIMPLEINTERLEAVE);
	}
    }
    else if (sender == &prefDirTrackCheckBox)
    {
	if (checked)
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags | CbmdosFsFlags::CFF_TALLOC_PREFDIRTRACK);
	}
	else
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags & ~CbmdosFsFlags::CFF_TALLOC_PREFDIRTRACK);
	}
    }
    else if (sender == &chainInterlvCheckBox)
    {
	if (checked)
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags | CbmdosFsFlags::CFF_TALLOC_CHAININTERLV);
	}
	else
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags & ~CbmdosFsFlags::CFF_TALLOC_CHAININTERLV);
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
	    Qt::CustomizeWindowHint | Qt::MSWindowsFixedSizeDialogHint)
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
    d->allocationLayout.addWidget(&d->allocOriginalButton, 0, 0);
    d->allocationLayout.addWidget(&d->allocTrackloaderButton, 1, 0);
    d->allocationLayout.addWidget(&d->allocSimpleButton, 2, 0);
    d->allocationLayout.addWidget(&d->simpleInterleaveCheckBox, 0, 1);
    d->allocationLayout.addWidget(&d->prefDirTrackCheckBox, 1, 1);
    d->allocationLayout.addWidget(&d->chainInterlvCheckBox, 2, 1);
    d->allocationOptions.setLayout(&d->allocationLayout);
    d->optionsLayout.addWidget(&d->allocationOptions, 6, 0, 1, 3);
    if (options->flags & CbmdosFsFlags::CFF_RECOVER)
    {
	d->optionsLayout.addWidget(&d->recoverWarningLabel, 7, 0, 1, 3);
        d->rewriteAfterRecover.setChecked(true);
        d->optionsLayout.addWidget(&d->rewriteAfterRecover, 8, 0, 1, 3);
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
    connect(&d->allocOriginalButton, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->allocTrackloaderButton, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->allocSimpleButton, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->simpleInterleaveCheckBox, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->prefDirTrackCheckBox, &QAbstractButton::clicked,
	    this, &CbmdosFsOptionsDialog::optionClicked);
    connect(&d->chainInterlvCheckBox, &QAbstractButton::clicked,
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

bool CbmdosFsOptionsDialog::wantRewrite()
{
    return d->rewriteAfterRecover.isChecked();
}

void CbmdosFsOptionsDialog::showEvent(QShowEvent *event)
{
    QCoreApplication::processEvents();
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

