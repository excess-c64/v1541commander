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

AboutBox::priv::priv() :
    mainLayout(),
    contentLayout(),
    textLayout(),
    buttonsLayout(),
    appLogo(),
    excessLogo("\n"
	    "\n"
	    "\n"
	    "\n"
	    ""),
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
    appLogo.setPixmap(QPixmap(":/gfx/icon_256.png"));
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

