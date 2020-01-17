#include "aboutbox.h"

#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

class AboutBox::priv
{
    public:
	priv();
	QVBoxLayout mainLayout;
	QHBoxLayout contentLayout;
	QVBoxLayout textLayout;
        QHBoxLayout titleLayout;
	QLabel appLogo;
	QLabel excessLogo;
	QLabel aboutText;
        QLabel titleText;
	QDialogButtonBox buttons;
};

static const QChar logo[] = {0xa,
    0xe0b0, 0xe0d5, 0xe0c0, 0xe0b2, 0xe0b2, 0xe0b2, 0xe0c0, 0xe0b2,
    0xe0c0, 0xe0b2, 0xe0c0, 0xe0b2, 0xe0c0, 0xe0b2, 0xe0c9, 0xe0ae, 0xa,
    0xe0b0, 0xe0b3, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd,
    0xe0dd, 0xe0dd, 0xe0ad, 0xe0b3, 0xe0ad, 0xe0b3, 0xe0ab, 0xe0ae, 0xa,
    0xe0dd, 0xe0dd, 0xe0b0, 0xe0db, 0xe020, 0xe0b3, 0xe0ab, 0xe0b3,
    0xe0b0, 0xe0db, 0xe0ae, 0xe0ab, 0xe0ae, 0xe0dd, 0xe0dd, 0xe0dd, 0xa,
    0xe0ad, 0xe0b3, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd,
    0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0ab, 0xe0db, 0xe0bd, 0xa,
    0xe0ad, 0xe0ca, 0xe0c0, 0xe0b1, 0xe0b1, 0xe0b1, 0xe0c0, 0xe0b1,
    0xe0c0, 0xe0b1, 0xe0c0, 0xe0b1, 0xe0c0, 0xe0b1, 0xe0cb, 0xe0bd, 0xa
};

AboutBox::priv::priv() :
    mainLayout(),
    contentLayout(),
    textLayout(),
    titleLayout(),
    appLogo(),
    excessLogo(QString::fromRawData(logo, sizeof logo / sizeof *logo)),
    aboutText(tr(
	    "<p>This is free software provided under BSD 2clause license, "
	    "with no warranties whatsoever, see LICENSE.txt included with the "
	    "package.</p>"
	    "<p>About the C64 font used, see LICENSE-font.txt, also "
	    "included with the package.</p>"
	    "<p>The UI is driven by Qt, using LGPL licensing.</p>"
	    "<p>If you received a statically linked package, see "
	    "<a href=\"https://github.com/excess-c64/v1541commander\">"
	    "https://github.com/excess-c64/v1541commander</a> for "
	    "instructions to build your own executable, possibly linking "
	    "different library versions.</p>")),
    titleText(QString(tr("<p><font size=\"+1\"><b>%1 v%2</b></font></p>"
            "<p>virtual 1541 disk commander<br />"
            "by Zirias/Excess</p>"))
	    .arg(QCoreApplication::applicationName())
	    .arg(QCoreApplication::applicationVersion())),
    buttons(QDialogButtonBox::Ok)
{
    appLogo.setPixmap(QPixmap(":/icon_256.png"));
}

AboutBox::AboutBox(const QFont &c64font) :
    QDialog(0, Qt::WindowSystemMenuHint
            | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    d = new priv();
    d->excessLogo.setFont(c64font);
    d->aboutText.setWordWrap(true);
    d->titleLayout.addWidget(&d->titleText);
    d->titleLayout.addWidget(&d->excessLogo);
    d->textLayout.addLayout(&d->titleLayout);
    d->textLayout.addWidget(&d->aboutText);
    d->contentLayout.addWidget(&d->appLogo);
    d->contentLayout.addLayout(&d->textLayout);
    d->mainLayout.addLayout(&d->contentLayout);
    d->mainLayout.addWidget(&d->buttons);
    d->mainLayout.setSizeConstraint(QLayout::SetFixedSize);
    setLayout(&d->mainLayout);
    connect(&d->buttons, &QDialogButtonBox::accepted, this, &AboutBox::hide);
}

AboutBox::~AboutBox()
{
    delete d;
}

void AboutBox::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    setFixedSize(minimumSize());
}

