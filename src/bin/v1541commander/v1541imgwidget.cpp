#include "v1541imgwidget.h"
#include "v1541commander.h"
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
#include <1541img/cbmdosfile.h>
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
    d->dirList.setDragEnabled(true);
    d->dirList.setAcceptDrops(true);
    d->dirList.setDragDropMode(QAbstractItemView::InternalMove);
    d->dirList.addAction(&cmdr.newFileAction());
    d->dirList.addAction(&cmdr.deleteFileAction());
    d->dirList.setContextMenuPolicy(Qt::ActionsContextMenu);
    layout->addWidget(&d->dirList);
    propLayout->addWidget(&d->fsprop);
    propLayout->addWidget(&d->file);
    propLayout->addStretch();
    layout->addLayout(propLayout);
    connect(d->dirList.selectionModel(), &QItemSelectionModel::currentChanged,
	    this, &V1541ImgWidget::selected);
    connect(&d->model,
	    SIGNAL(selectedIndexChanged(const QModelIndex &,
		    QItemSelectionModel::SelectionFlags)),
	    d->dirList.selectionModel(),
	    SLOT(setCurrentIndex(const QModelIndex &,
		    QItemSelectionModel::SelectionFlags)));
    connect(&d->model, &CbmdosFsModel::modified,
	    this, &V1541ImgWidget::modelModified);
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
    emit selectionChanged();
}

void V1541ImgWidget::modelModified()
{
    emit modified();
}

void V1541ImgWidget::newImage()
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
    d->fs = CbmdosFs_create(CFO_DEFAULT);
    d->model.setFs(d->fs);
    d->fsprop.setFs(d->fs);
    d->dirList.setMinimumWidth(
	    d->dirList.sizeHintForColumn(0)
	    + 2 * d->dirList.frameWidth());
    d->dirList.setMinimumHeight(
	    d->dirList.sizeHintForRow(0) * 10
	    + 2 * d->dirList.frameWidth());
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
	}
    }
}

void V1541ImgWidget::save(const QString& filename)
{
    const D64 *d64 = d->d64;
    if (!d64 && d->fs) d64 = CbmdosFs_image(d->fs);
    if (d64)
    {
	FILE *d64file = qfopen(filename, "wb");
	if (d64file)
	{
	    if (writeD64(d64file, d64) < 0)
	    {
		QMessageBox::critical(this, tr("Error writing file"),
			tr("The selected file couldn't be written."));
	    }
	    else
	    {
		emit saved();
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

void V1541ImgWidget::newFile()
{
    if (!hasValidImage()) return;
    CbmdosFile *newFile = CbmdosFile_create();
    const QModelIndex &index = d->dirList.selectionModel()->currentIndex();
    d->model.addFile(index, newFile);
}

void V1541ImgWidget::deleteFile()
{
    if (!hasValidImage() || !hasValidSelection()) return;
    QMessageBox::StandardButton reply = QMessageBox::question(this,
	    tr("Delete this file?"), tr("A deleted file cannot be restored. "
	    "Are you sure you want to delete this file now?"),
	    QMessageBox::Ok|QMessageBox::Cancel);
    if (reply == QMessageBox::Ok)
    {
	const QModelIndex &index = d->dirList.selectionModel()->currentIndex();
	d->model.deleteAt(index);
    }
}

bool V1541ImgWidget::hasValidImage() const
{
    return d->fs || d->d64;
}

bool V1541ImgWidget::hasValidSelection() const
{
    const QModelIndex &index = d->dirList.selectionModel()->currentIndex();
    if (!index.isValid()) return false;
    return (index.row() > 0 && index.row() < d->model.rowCount() - 1);
}
