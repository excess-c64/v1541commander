#include "cbmdosfilewidget.h"
#include "v1541commander.h"
#include "petsciistr.h"
#include "petsciiedit.h"

#include <QFontMetricsF>
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
    QFontMetricsF fm(app.c64font());
    d->name.setMinimumWidth(fm.ascent() * 18 * 13 / 14);
    d->name.setFont(app.c64font());
    d->name.setMaxLength(16);
    setLayout(&d->nameLayout);
    setEnabled(false);
}

CbmdosFileWidget::~CbmdosFileWidget()
{
    delete d;
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
