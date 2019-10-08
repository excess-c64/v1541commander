#include "cbmdosfilewidget.h"
#include "petsciistr.h"
#include "petsciiedit.h"

#include <QHBoxLayout>
#include <QLabel>

#include <1541img/cbmdosfile.h>

class CbmdosFileWidget::priv
{
    public:
	priv();
	CbmdosFile *file;
	QHBoxLayout nameLayout;
	QLabel nameLabel;
	PetsciiEdit name;
};

CbmdosFileWidget::priv::priv() :
    file(0),
    nameLayout(),
    nameLabel(tr("Name: ")),
    name()
{
}

CbmdosFileWidget::CbmdosFileWidget(QWidget *parent)
    : QGroupBox(tr("File properties"), parent)
{
    d = new priv();
    d->nameLayout.addWidget(&d->nameLabel);
    d->nameLayout.addWidget(&d->name);
    d->name.setMaxLength(16);
    setLayout(&d->nameLayout);
    setEnabled(false);
    connect(&d->name, &PetsciiEdit::petsciiEdited,
	    this, &CbmdosFileWidget::nameChanged);
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

CbmdosFile *CbmdosFileWidget::file() const
{
    return d->file;
}

void CbmdosFileWidget::setFile(CbmdosFile *file)
{
    d->file = file;
    if (file)
    {
	setEnabled(true);
	uint8_t nameLength;
	const char *name = CbmdosFile_name(file, &nameLength);
	PetsciiStr str(name, nameLength);
	d->name.setText(str.toString());
    }
    else
    {
	setEnabled(false);
	d->name.setText(QString());
    }
}
