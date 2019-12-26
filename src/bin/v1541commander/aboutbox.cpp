#include "aboutbox.h"

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
    aboutText("v1541commander v0.1\n"
	    "virtual 1541 disk commander\n\n"
	    "by Zirias/Excess"),
    okButton(tr("Ok"))
{
    appLogo.setPixmap(QPixmap(":/gfx/icon_256.png"));
}

AboutBox::AboutBox(const QFont &c64font) :
    QDialog()
{
    d = new priv();
    d->excessLogo.setFont(c64font);
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

