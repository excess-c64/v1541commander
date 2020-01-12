#include "aboutbox.h"

#include <QCoreApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class AboutBox::priv
{
    public:
	priv();
	QVBoxLayout mainLayout;
	QHBoxLayout contentLayout;
	QVBoxLayout textLayout;
	QHBoxLayout buttonsLayout;
	QLabel appLogo;
	QLabel excessLogo;
	QLabel aboutText;
	QPushButton okButton;
};

static const QChar logo[] = {
    0xe0b0, 0xe0d5, 0xe0c0, 0xe0b2, 0xe0b2, 0xe0b2, 0xe0c0, 0xe0b2,
    0xe0c0, 0xe0b2, 0xe0c0, 0xe0b2, 0xe0c0, 0xe0b2, 0xe0c9, 0xe0ae, 0xa,
    0xe0b0, 0xe0b3, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd,
    0xe0dd, 0xe0dd, 0xe0ad, 0xe0b3, 0xe0ad, 0xe0b3, 0xe0ab, 0xe0ae, 0xa,
    0xe0dd, 0xe0dd, 0xe0b0, 0xe0db, 0xe020, 0xe0b3, 0xe0ab, 0xe0b3,
    0xe0b0, 0xe0db, 0xe0ae, 0xe0ab, 0xe0ae, 0xe0dd, 0xe0dd, 0xe0dd, 0xa,
    0xe0ad, 0xe0b3, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd,
    0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0dd, 0xe0ab, 0xe0db, 0xe0bd, 0xa,
    0xe0ad, 0xe0ca, 0xe0c0, 0xe0b1, 0xe0b1, 0xe0b1, 0xe0c0, 0xe0b1,
    0xe0c0, 0xe0b1, 0xe0c0, 0xe0b1, 0xe0c0, 0xe0b1, 0xe0cb, 0xe0bd
};

AboutBox::priv::priv() :
    mainLayout(),
    contentLayout(),
    textLayout(),
    buttonsLayout(),
    appLogo(),
    excessLogo(QString::fromRawData(logo, sizeof logo / sizeof *logo)),
    aboutText(QString("<p>%1 v%2<br />"
	    "virtual 1541 disk commander<br />"
	    "by Zirias/Excess</p>"
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
	    "different library versions.</p>")
	    .arg(QCoreApplication::applicationName())
	    .arg(QCoreApplication::applicationVersion())),
    okButton(tr("Ok"))
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
    d->textLayout.addWidget(&d->excessLogo);
    d->textLayout.addWidget(&d->aboutText);
    d->contentLayout.addWidget(&d->appLogo);
    d->contentLayout.addLayout(&d->textLayout);
    d->mainLayout.addLayout(&d->contentLayout);
    d->buttonsLayout.addStretch();
    d->buttonsLayout.addWidget(&d->okButton);
    d->mainLayout.addLayout(&d->buttonsLayout);
    d->mainLayout.setSizeConstraint(QLayout::SetFixedSize);
    setLayout(&d->mainLayout);
    connect(&d->okButton, &QPushButton::clicked, this, &AboutBox::hide);
}

AboutBox::~AboutBox()
{
    delete d;
}

void AboutBox::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    resize(0, 0);
}

