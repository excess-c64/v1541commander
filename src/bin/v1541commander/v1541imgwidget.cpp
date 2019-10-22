#include "v1541imgwidget.h"
#include "cbmdosfsmodel.h"
#include "cbmdosfswidget.h"
#include "cbmdosfilewidget.h"
#include "utils.h"

#include <QHBoxLayout>
#include <QListView>
#include <QMessageBox>
#include <QVBoxLayout>

#include <1541img/d64.h>
#include <1541img/d64reader.h>
#include <1541img/d64writer.h>
#include <1541img/cbmdosfs.h>
#include <1541img/cbmdosvfs.h>

class V1541ImgWidget::priv
{
    public:
	priv();
	D64 *d64;
	CbmdosFs *fs;
	CbmdosFsModel model;
	QListView dirList;
	CbmdosFsWidget fsprop;
	CbmdosFileWidget file;
};

V1541ImgWidget::priv::priv() :
    d64(0),
    fs(0),
    model(),
    dirList(),
    fsprop(),
    file()
{}

V1541ImgWidget::V1541ImgWidget() : QWidget()
{
    d = new priv();
    QHBoxLayout *layout = new QHBoxLayout(this);
    QVBoxLayout *propLayout = new QVBoxLayout();
    d->dirList.setMinimumHeight(200);
    d->dirList.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->dirList.setModel(&d->model);
    layout->addWidget(&d->dirList);
    propLayout->addWidget(&d->fsprop);
    propLayout->addWidget(&d->file);
    propLayout->addStretch();
    layout->addLayout(propLayout);
    connect(d->dirList.selectionModel(), &QItemSelectionModel::currentChanged,
	    this, &V1541ImgWidget::selected);
}

V1541ImgWidget::~V1541ImgWidget()
{
    CbmdosFs *fs = d->fs;
    D64 *d64 = d->d64;
    delete d;

    if (fs)
    {
	CbmdosFs_destroy(fs);
    }
    else if (d64)
    {
	D64_destroy(d64);
    }
}

void V1541ImgWidget::selected(const QModelIndex &current,
	const QModelIndex &previous)
{
    (void) previous;

    CbmdosFile *file = 0;
    if (current.row() > 0 && d->fs)
    {
	CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
	if (current.row() <= (int)CbmdosVfs_fileCount(vfs))
	{
	    file = CbmdosVfs_file(vfs, current.row()-1);
	}
    }
    d->file.setFile(file);
}

void V1541ImgWidget::open(const QString& filename)
{
    if (d->fs)
    {
	CbmdosFs_destroy(d->fs);
	d->fs = 0;
	d->d64 = 0;
    }
    if (d->d64)
    {
	D64_destroy(d->d64);
	d->d64 = 0;
    }
    FILE *d64file = qfopen(filename, "rb");
    if (d64file)
    {
	d->d64 = readD64(d64file);
	fclose(d64file);

	if (d->d64)
	{
	    CbmdosFsOptions opts = CFO_DEFAULT;
	    if (D64_type(d->d64) == D64_40TRACK) opts.flags = CFF_40TRACK;
	    d->fs = CbmdosFs_fromImage(d->d64, opts);
	    if (d->fs)
	    {
		d->model.setFs(d->fs);
		d->fsprop.setFs(d->fs);
                d->dirList.setMinimumWidth(
                        d->dirList.sizeHintForColumn(0)
                        + 2 * d->dirList.frameWidth());
                int minItems = d->model.rowCount();
                if (minItems < 10) minItems = 10;
                if (minItems > 40) minItems = 40;
                d->dirList.setMinimumHeight(
                        d->dirList.sizeHintForRow(0) * minItems
                        + 2 * d->dirList.frameWidth());
	    }
	    setWindowTitle(filename);
	}
    }
}

void V1541ImgWidget::save(const QString& filename)
{
    if (d->d64)
    {
	FILE *d64file = qfopen(filename, "wb");
	if (d64file)
	{
	    if (writeD64(d64file, d->d64) < 0)
	    {
		QMessageBox::critical(this, tr("Error writing file"),
			tr("The selected file couldn't be written."));
	    }
	    fclose(d64file);
	}
	else
	{
	    QMessageBox::critical(this, tr("Error opening file"),
		    tr("The selected file couldn't be opened for writing."));
	}
    }
}

bool V1541ImgWidget::hasValidImage() const
{
    return d->fs || d->d64;
}

