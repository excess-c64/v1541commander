#include "cbmdosfsmodel.h"
#include "v1541commander.h"
#include "petsciistr.h"

#include <QFontMetricsF>
#include <QSizeF>
#include <QStyle>

#include <1541img/cbmdosfs.h>
#include <1541img/cbmdosvfs.h>
#include <1541img/cbmdosvfseventargs.h>
#include <1541img/cbmdosfile.h>
#include <1541img/event.h>

static void evhdl(void *receiver, int id, const void *sender, const void *args)
{
    (void) id;
    (void) sender;

    CbmdosFsModel *model = (CbmdosFsModel *)receiver;
    const CbmdosVfsEventArgs *eventArgs = (const CbmdosVfsEventArgs *)args;
    model->fsChanged(eventArgs);
}

class CbmdosFsModel::priv
{
    public:
	priv();
	CbmdosFs *fs;
};

CbmdosFsModel::priv::priv() :
    fs(0)
{}

CbmdosFsModel::CbmdosFsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    d = new priv();
}

CbmdosFsModel::~CbmdosFsModel()
{
    if (d->fs)
    {
	CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
	Event_unregister(CbmdosVfs_changedEvent(vfs), this, evhdl);
    }
    delete d;
}

void CbmdosFsModel::fsChanged(const CbmdosVfsEventArgs *args)
{
    switch (args->what)
    {
	case CbmdosVfsEventArgs::CVE_FILECHANGED:
	    {
		QModelIndex pos = createIndex(args->filepos + 1, 0);
		emit dataChanged(pos, pos, QVector<int>(Qt::DisplayRole));
	    }
	    break;

	case CbmdosVfsEventArgs::CVE_NAMECHANGED:
	case CbmdosVfsEventArgs::CVE_IDCHANGED:
	    {
		QModelIndex pos = createIndex(0, 0);
		emit dataChanged(pos, pos, QVector<int>(Qt::DisplayRole));
	    }
	    break;

	default:
	    break;
    }
}

CbmdosFs *CbmdosFsModel::fs() const
{
    return d->fs;
}

void CbmdosFsModel::setFs(CbmdosFs *fs)
{
    if (d->fs)
    {
	CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
	Event_unregister(CbmdosVfs_changedEvent(vfs), this, evhdl);
	beginRemoveRows(QModelIndex(), 0, CbmdosVfs_fileCount(vfs)+1);
	d->fs = 0;
	endRemoveRows();
    }
    if (fs)
    {
	CbmdosVfs *vfs = CbmdosFs_vfs(fs);
	beginInsertRows(QModelIndex(), 0, CbmdosVfs_fileCount(vfs)+1);
	d->fs = fs;
	Event_register(CbmdosVfs_changedEvent(vfs), this, evhdl);
	endInsertRows();
    }
}

int CbmdosFsModel::rowCount(const QModelIndex &parent) const
{
    (void) parent;
    if (!d->fs) return 0;
    const CbmdosVfs *vfs = CbmdosFs_rvfs(d->fs);
    return CbmdosVfs_fileCount(vfs) + 2;
}

QVariant CbmdosFsModel::data(const QModelIndex &index, int role) const
{
    uint8_t buffer[28];

    if (role == Qt::FontRole)
    {
	return app.c64font();
    }

    if (role == Qt::SizeHintRole)
    {
	QFontMetricsF fm(app.c64font());
	return QSizeF(fm.ascent() * 29 * 13 / 14
		+ app.style()->pixelMetric(QStyle::PM_ScrollBarExtent),
		fm.ascent() * 13 / 14);
    }

    if (!d->fs) return QVariant();

    if (role != Qt::DisplayRole)
    {
	return QVariant();
    }

    int row = index.row();
    int rowcount = rowCount();
    const CbmdosVfs *vfs = CbmdosFs_rvfs(d->fs);

    if (row > 0 && row < rowcount - 1)
    {
	const CbmdosFile *file = CbmdosVfs_rfile(vfs, row-1);
	CbmdosFile_getDirLine(file, buffer);
	PetsciiStr dirLine((char *)buffer, 28);
	return dirLine.toString();
    }

    if (row == 0)
    {
	QString heading("0 ");
	CbmdosVfs_getDirHeader(vfs, buffer);
	PetsciiStr dirHeader((char *)buffer, 24);
	heading.append(dirHeader.toString(true));
	return heading;
    }

    CbmdosFs_getFreeBlocksLine(d->fs, buffer);
    PetsciiStr freeLine((char *)buffer, 16);
    return freeLine.toString();
}

