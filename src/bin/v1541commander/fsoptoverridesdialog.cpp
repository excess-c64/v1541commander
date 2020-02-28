#include "fsoptoverridesdialog.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QScreen>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWindow>

#include <1541img/cbmdosfsoptions.h>

class FsOptOverridesDialog::priv
{
    public:
	priv(CbmdosFsOptOverrides *overrides);
	CbmdosFsOptOverrides *overrides;
	CbmdosFsOptOverrides ovrd;
	QVBoxLayout mainLayout;
	QGridLayout ovrdLayout;
	QCheckBox ovrdInterleaveCheckBox;
	QLabel interleaveLabel;
	QSpinBox interleaveSpinBox;
	QCheckBox ovrdStrategyCheckBox;
	QRadioButton allocOriginalButton;
	QRadioButton allocTrackloaderButton;
	QRadioButton allocSimpleButton;
	QCheckBox simpleInterleaveCheckBox;
	QCheckBox prefDirTrackCheckBox;
	QCheckBox chainInterlvCheckBox;
	QDialogButtonBox buttons;

	void reset();
	void clicked(QObject *sender);
	void changed(int value);
};

FsOptOverridesDialog::priv::priv(CbmdosFsOptOverrides *overrides) :
    overrides(overrides),
    ovrd(),
    mainLayout(),
    ovrdLayout(),
    ovrdInterleaveCheckBox(tr("Override interleave")),
    interleaveLabel(tr("interleave: ")),
    interleaveSpinBox(),
    ovrdStrategyCheckBox(tr("Override strategy")),
    allocOriginalButton(tr("CBM DOS strategy")),
    allocTrackloaderButton(tr("Trackloader strategy")),
    allocSimpleButton(tr("Simple strategy")),
    simpleInterleaveCheckBox(tr("Use simple interleave")),
    prefDirTrackCheckBox(tr("Prefer dir track for files")),
    chainInterlvCheckBox(tr("Apply interleave on track change")),
    buttons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel
	    |QDialogButtonBox::Reset)
{
    interleaveLabel.setAlignment(Qt::AlignRight);
    interleaveSpinBox.setMinimum(1);
    interleaveSpinBox.setMaximum(20);
    simpleInterleaveCheckBox.setTristate(true);
    prefDirTrackCheckBox.setTristate(true);
    chainInterlvCheckBox.setTristate(true);
    ovrdInterleaveCheckBox.setToolTip(
	    tr("Override interleave when writing this file"));
    QString interleaveToolTip(
	    tr("Interleave value to use for blocks of this file "
		"(default: 10)"));
    interleaveLabel.setToolTip(interleaveToolTip);
    interleaveSpinBox.setToolTip(interleaveToolTip);
    ovrdStrategyCheckBox.setToolTip(
	    tr("Override the base track/block allocation strategy"));
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
		"the result.\n"
		"Partially checked means not to override this flag."));
    prefDirTrackCheckBox.setToolTip(
	    tr("Put files on the directory track first.\n"
		"This has only an effect if files on the directory track "
		"are allowed.\n"
		"Partially checked means not to override this flag."));
    chainInterlvCheckBox.setToolTip(
	    tr("When having to switch to another track, still apply "
		"interleave to the sector number.\n"
		"If this is not checked, looking for a free sector on a new "
		"track\n"
		"always starts at sector 0 (original CBM DOS behavior).\n"
		"Partially checked means not to override this flag."));
    reset();

}

void FsOptOverridesDialog::priv::reset()
{
    memcpy(&ovrd, overrides, sizeof ovrd);
    if (ovrd.mask & CbmdosFsFlags::CFF_OVERRIDE_INTERLEAVE)
    {
	ovrdInterleaveCheckBox.setChecked(true);
	interleaveSpinBox.setEnabled(true);
	int interlv = ovrd.flags & CbmdosFsFlags::CFF_OVERRIDE_INTERLEAVE;
	if (interlv < 1) interlv = 1;
	else if (interlv > 20) interlv = 20;
	interleaveSpinBox.setValue(interlv);
    }
    else
    {
	ovrdInterleaveCheckBox.setChecked(false);
	interleaveSpinBox.setEnabled(false);
	interleaveSpinBox.setValue(10);
    }

    if (ovrd.mask & (
		CbmdosFsFlags::CFF_TALLOC_TRACKLOAD |
		CbmdosFsFlags::CFF_TALLOC_SIMPLE ))
    {
	ovrdStrategyCheckBox.setChecked(true);
	allocOriginalButton.setEnabled(true);
	allocTrackloaderButton.setEnabled(true);
	allocSimpleButton.setEnabled(true);
	if (ovrd.flags & CbmdosFsFlags::CFF_TALLOC_TRACKLOAD)
	{
	    allocTrackloaderButton.setChecked(true);
	}
	else if (ovrd.flags & CbmdosFsFlags::CFF_TALLOC_SIMPLE)
	{
	    allocSimpleButton.setChecked(true);
	}
	else
	{
	    allocOriginalButton.setChecked(true);
	}
    }
    else
    {
	ovrdStrategyCheckBox.setChecked(false);
	allocOriginalButton.setChecked(true);
	allocOriginalButton.setEnabled(false);
	allocTrackloaderButton.setEnabled(false);
	allocSimpleButton.setEnabled(false);
    }

    if (ovrd.mask & CbmdosFsFlags::CFF_SIMPLEINTERLEAVE)
    {
	if (ovrd.flags & CbmdosFsFlags::CFF_SIMPLEINTERLEAVE)
	{
	    simpleInterleaveCheckBox.setCheckState(Qt::Checked);
	}
	else
	{
	    simpleInterleaveCheckBox.setCheckState(Qt::Unchecked);
	}
    }
    else
    {
	simpleInterleaveCheckBox.setCheckState(Qt::PartiallyChecked);
    }

    if (ovrd.mask & CbmdosFsFlags::CFF_TALLOC_PREFDIRTRACK)
    {
	if (ovrd.flags & CbmdosFsFlags::CFF_TALLOC_PREFDIRTRACK)
	{
	    prefDirTrackCheckBox.setCheckState(Qt::Checked);
	}
	else
	{
	    prefDirTrackCheckBox.setCheckState(Qt::Unchecked);
	}
    }
    else
    {
	prefDirTrackCheckBox.setCheckState(Qt::PartiallyChecked);
    }

    if (ovrd.mask & CbmdosFsFlags::CFF_TALLOC_CHAININTERLV)
    {
	if (ovrd.flags & CbmdosFsFlags::CFF_TALLOC_CHAININTERLV)
	{
	    chainInterlvCheckBox.setCheckState(Qt::Checked);
	}
	else
	{
	    chainInterlvCheckBox.setCheckState(Qt::Unchecked);
	}
    }
    else
    {
	chainInterlvCheckBox.setCheckState(Qt::PartiallyChecked);
    }
}

void FsOptOverridesDialog::priv::clicked(QObject *sender)
{
    if (sender == &ovrdInterleaveCheckBox)
    {
	interleaveSpinBox.setValue(10);
	ovrd.flags = (CbmdosFsFlags)
	    (ovrd.flags & ~CbmdosFsFlags::CFF_OVERRIDE_INTERLEAVE);
	ovrd.flags = (CbmdosFsFlags)
	    (ovrd.flags | 10);
	if (ovrdInterleaveCheckBox.isChecked())
	{
	    interleaveSpinBox.setEnabled(true);
	    ovrd.mask = (CbmdosFsFlags) (ovrd.mask
		    | CbmdosFsFlags::CFF_OVERRIDE_INTERLEAVE);
	}
	else
	{
	    interleaveSpinBox.setEnabled(false);
	    ovrd.mask = (CbmdosFsFlags) (ovrd.mask
		    & ~CbmdosFsFlags::CFF_OVERRIDE_INTERLEAVE);
	}
    }
    else if (sender == &ovrdStrategyCheckBox)
    {
	allocOriginalButton.setChecked(true);
	ovrd.flags = (CbmdosFsFlags) (ovrd.flags & ~(
		    CbmdosFsFlags::CFF_TALLOC_TRACKLOAD |
		    CbmdosFsFlags::CFF_TALLOC_SIMPLE
		    ));
	if (ovrdStrategyCheckBox.isChecked())
	{
	    allocOriginalButton.setEnabled(true);
	    allocTrackloaderButton.setEnabled(true);
	    allocSimpleButton.setEnabled(true);
	    ovrd.mask = (CbmdosFsFlags) (ovrd.mask
		    | CbmdosFsFlags::CFF_TALLOC_TRACKLOAD
		    | CbmdosFsFlags::CFF_TALLOC_SIMPLE);
	}
	else
	{
	    allocOriginalButton.setEnabled(false);
	    allocTrackloaderButton.setEnabled(false);
	    allocSimpleButton.setEnabled(false);
	    ovrd.mask = (CbmdosFsFlags) (ovrd.mask & ~(
			CbmdosFsFlags::CFF_TALLOC_TRACKLOAD |
			CbmdosFsFlags::CFF_TALLOC_SIMPLE
			));
	}
    }
    else if (sender == &allocOriginalButton)
    {
	ovrd.flags = (CbmdosFsFlags) (ovrd.flags & ~(
		    CbmdosFsFlags::CFF_TALLOC_TRACKLOAD |
		    CbmdosFsFlags::CFF_TALLOC_SIMPLE
		    ));
    }
    else if (sender == &allocTrackloaderButton)
    {
	ovrd.flags = (CbmdosFsFlags)
	    (ovrd.flags & ~CbmdosFsFlags::CFF_TALLOC_SIMPLE);
	ovrd.flags = (CbmdosFsFlags)
	    (ovrd.flags | CbmdosFsFlags::CFF_TALLOC_TRACKLOAD);
    }
    else if (sender == &allocSimpleButton)
    {
	ovrd.flags = (CbmdosFsFlags)
	    (ovrd.flags & ~CbmdosFsFlags::CFF_TALLOC_TRACKLOAD);
	ovrd.flags = (CbmdosFsFlags)
	    (ovrd.flags | CbmdosFsFlags::CFF_TALLOC_SIMPLE);
    }
    else if (sender == &simpleInterleaveCheckBox)
    {
	switch (simpleInterleaveCheckBox.checkState())
	{
	    case Qt::Unchecked:
		ovrd.mask = (CbmdosFsFlags)
		    (ovrd.mask | CbmdosFsFlags::CFF_SIMPLEINTERLEAVE);
		ovrd.flags = (CbmdosFsFlags)
		    (ovrd.flags & ~CbmdosFsFlags::CFF_SIMPLEINTERLEAVE);
		break;
	    case Qt::PartiallyChecked:
		ovrd.mask = (CbmdosFsFlags)
		    (ovrd.mask & ~CbmdosFsFlags::CFF_SIMPLEINTERLEAVE);
		ovrd.flags = (CbmdosFsFlags)
		    (ovrd.flags & ~CbmdosFsFlags::CFF_SIMPLEINTERLEAVE);
		break;
	    case Qt::Checked:
		ovrd.mask = (CbmdosFsFlags)
		    (ovrd.mask | CbmdosFsFlags::CFF_SIMPLEINTERLEAVE);
		ovrd.flags = (CbmdosFsFlags)
		    (ovrd.flags | CbmdosFsFlags::CFF_SIMPLEINTERLEAVE);
		break;
	}
    }
    else if (sender == &prefDirTrackCheckBox)
    {
	switch (prefDirTrackCheckBox.checkState())
	{
	    case Qt::Unchecked:
		ovrd.mask = (CbmdosFsFlags)
		    (ovrd.mask | CbmdosFsFlags::CFF_TALLOC_PREFDIRTRACK);
		ovrd.flags = (CbmdosFsFlags)
		    (ovrd.flags & ~CbmdosFsFlags::CFF_TALLOC_PREFDIRTRACK);
		break;
	    case Qt::PartiallyChecked:
		ovrd.mask = (CbmdosFsFlags)
		    (ovrd.mask & ~CbmdosFsFlags::CFF_TALLOC_PREFDIRTRACK);
		ovrd.flags = (CbmdosFsFlags)
		    (ovrd.flags & ~CbmdosFsFlags::CFF_TALLOC_PREFDIRTRACK);
		break;
	    case Qt::Checked:
		ovrd.mask = (CbmdosFsFlags)
		    (ovrd.mask | CbmdosFsFlags::CFF_TALLOC_PREFDIRTRACK);
		ovrd.flags = (CbmdosFsFlags)
		    (ovrd.flags | CbmdosFsFlags::CFF_TALLOC_PREFDIRTRACK);
		break;
	}
    }
    else if (sender == &chainInterlvCheckBox)
    {
	switch (chainInterlvCheckBox.checkState())
	{
	    case Qt::Unchecked:
		ovrd.mask = (CbmdosFsFlags)
		    (ovrd.mask | CbmdosFsFlags::CFF_TALLOC_CHAININTERLV);
		ovrd.flags = (CbmdosFsFlags)
		    (ovrd.flags & ~CbmdosFsFlags::CFF_TALLOC_CHAININTERLV);
		break;
	    case Qt::PartiallyChecked:
		ovrd.mask = (CbmdosFsFlags)
		    (ovrd.mask & ~CbmdosFsFlags::CFF_TALLOC_CHAININTERLV);
		ovrd.flags = (CbmdosFsFlags)
		    (ovrd.flags & ~CbmdosFsFlags::CFF_TALLOC_CHAININTERLV);
		break;
	    case Qt::Checked:
		ovrd.mask = (CbmdosFsFlags)
		    (ovrd.mask | CbmdosFsFlags::CFF_TALLOC_CHAININTERLV);
		ovrd.flags = (CbmdosFsFlags)
		    (ovrd.flags | CbmdosFsFlags::CFF_TALLOC_CHAININTERLV);
		break;
	}
    }
}

void FsOptOverridesDialog::priv::changed(int value)
{
    if (ovrd.mask & CFF_OVERRIDE_INTERLEAVE)
    {
	ovrd.flags = (CbmdosFsFlags)
	    (ovrd.flags & ~CbmdosFsFlags::CFF_OVERRIDE_INTERLEAVE);
	ovrd.flags = (CbmdosFsFlags)
	    (ovrd.flags | value);
    }
}

FsOptOverridesDialog::FsOptOverridesDialog(CbmdosFsOptOverrides *overrides,
	QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint |
	    Qt::CustomizeWindowHint | Qt::MSWindowsFixedSizeDialogHint)
{
    d = new priv(overrides);
    d->ovrdLayout.addWidget(&d->ovrdInterleaveCheckBox, 0, 0, 1, 2);
    d->ovrdLayout.addWidget(&d->interleaveLabel, 0, 2);
    d->ovrdLayout.addWidget(&d->interleaveSpinBox, 0, 3);
    d->ovrdLayout.addWidget(&d->ovrdStrategyCheckBox, 1, 0, 1, 4);
    d->ovrdLayout.addWidget(&d->allocOriginalButton, 2, 0, 1, 2);
    d->ovrdLayout.addWidget(&d->allocTrackloaderButton, 3, 0, 1, 2);
    d->ovrdLayout.addWidget(&d->allocSimpleButton, 4, 0, 1, 2);
    d->ovrdLayout.addWidget(&d->simpleInterleaveCheckBox, 2, 2, 1, 2);
    d->ovrdLayout.addWidget(&d->prefDirTrackCheckBox, 3, 2, 1, 2);
    d->ovrdLayout.addWidget(&d->chainInterlvCheckBox, 4, 2, 1, 2);
    d->mainLayout.addLayout(&d->ovrdLayout);
    d->mainLayout.addWidget(&d->buttons);
    setLayout(&d->mainLayout);

    connect(&d->buttons, &QDialogButtonBox::accepted,
	    this, &FsOptOverridesDialog::accept);
    connect(&d->buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(&d->buttons, &QDialogButtonBox::clicked, this,
	    [this](QAbstractButton *button)
	    {
		if (d->buttons.buttonRole(button)
			== QDialogButtonBox::ResetRole)
		{
		    d->reset();
		}
	    });

    auto handleChange = [this](int){ d->clicked(sender()); };
    connect(&d->ovrdInterleaveCheckBox, &QCheckBox::stateChanged,
	    this, handleChange);
    connect(&d->interleaveSpinBox,
	    static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
	    this, [this](int i){d->changed(i);});
    connect(&d->ovrdStrategyCheckBox, &QCheckBox::stateChanged,
	    this, handleChange);
    connect(&d->allocOriginalButton, &QAbstractButton::clicked,
	    this, handleChange);
    connect(&d->allocTrackloaderButton, &QAbstractButton::clicked,
	    this, handleChange);
    connect(&d->allocSimpleButton, &QAbstractButton::clicked,
	    this, handleChange);
    connect(&d->simpleInterleaveCheckBox, &QCheckBox::stateChanged,
	    this, handleChange);
    connect(&d->prefDirTrackCheckBox, &QCheckBox::stateChanged,
	    this, handleChange);
    connect(&d->chainInterlvCheckBox, &QCheckBox::stateChanged,
	    this, handleChange);
}

FsOptOverridesDialog::~FsOptOverridesDialog()
{
    delete d;
}

void FsOptOverridesDialog::accept()
{
    memcpy(d->overrides, &d->ovrd, sizeof d->ovrd);
    QDialog::accept();
}

void FsOptOverridesDialog::showEvent(QShowEvent *event)
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

