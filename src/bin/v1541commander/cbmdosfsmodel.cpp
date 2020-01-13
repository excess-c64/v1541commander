#include "cbmdosfsmodel.h"
#include "v1541commander.h"
#include "petsciistr.h"

#include <QFontMetricsF>
#include <QMimeData>
#include <QSizeF>
#include <QStyle>

#include <1541img/cbmdosfs.h>
#include <1541img/cbmdosvfs.h>
#include <1541img/cbmdosvfseventargs.h>
#include <1541img/cbmdosfile.h>
#include <1541img/event.h>

static const QString fileposMimeType("application/x-cbmdosfs-filepos");

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

    connect(&cmdr, &V1541Commander::lowerCaseChanged,
            this, [this](bool lowerCase){
                (void) lowerCase;
                emit dataChanged(createIndex(0, 0),
                        createIndex(rowCount()-1, 0),
                        QVector<int>(Qt::DisplayRole));
            });
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
    emit modified();
    switch (args->what)
    {
	case CbmdosVfsEventArgs::CVE_FILECHANGED:
	    {
		QModelIndex pos = createIndex(args->filepos + 1, 0);
		emit dataChanged(pos, pos, QVector<int>(Qt::DisplayRole));
	    }
	    break;

	case CbmdosVfsEventArgs::CVE_FILEMOVED:
	    if (args->targetpos > args->filepos)
	    {
		QModelIndex from = createIndex(args->filepos + 1, 0);
		QModelIndex to = createIndex(args->targetpos + 1, 0);
		emit dataChanged(from, to, QVector<int>(Qt::DisplayRole));
		emit selectedIndexChanged(to,
			QItemSelectionModel::ClearAndSelect);
	    }
	    else
	    {
		QModelIndex from = createIndex(args->targetpos + 1, 0);
		QModelIndex to = createIndex(args->filepos + 1, 0);
		emit dataChanged(from, to, QVector<int>(Qt::DisplayRole));
		emit selectedIndexChanged(from,
			QItemSelectionModel::ClearAndSelect);
	    }
	    break;

	case CbmdosVfsEventArgs::CVE_NAMECHANGED:
	case CbmdosVfsEventArgs::CVE_IDCHANGED:
	case CbmdosVfsEventArgs::CVE_DOSVERCHANGED:
	    {
		QModelIndex pos = createIndex(0, 0);
		emit dataChanged(pos, pos, QVector<int>(Qt::DisplayRole));
	    }
	    break;

	case CbmdosVfsEventArgs::CVE_FILEDELETED:
	    {
		endRemoveRows();
		QModelIndex last = createIndex(rowCount() - 1, 0);
		emit dataChanged(last, last, QVector<int>(Qt::DisplayRole));
	    }
	    break;

	case CbmdosVfsEventArgs::CVE_FILEADDED:
	    {
		endInsertRows();
		QModelIndex last = createIndex(rowCount() - 1, 0);
		emit dataChanged(last, last, QVector<int>(Qt::DisplayRole));
		QModelIndex at = createIndex(args->filepos + 1, 0);
		emit selectedIndexChanged(at,
			QItemSelectionModel::ClearAndSelect);
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

void CbmdosFsModel::deleteAt(const QModelIndex &at)
{
    if (at.row() > 0 && at.row() < rowCount() - 1)
    {
	beginRemoveRows(QModelIndex(), at.row(), at.row());
	CbmdosVfs_deleteAt(CbmdosFs_vfs(d->fs), at.row() - 1);
    }
}

void CbmdosFsModel::addFile(const QModelIndex &at, CbmdosFile *newFile)
{
    if (at.isValid() && at.row() > 0)
    {
	beginInsertRows(QModelIndex(), at.row(), at.row());
	CbmdosVfs_insert(CbmdosFs_vfs(d->fs), newFile, at.row() - 1);
    }
    else
    {
	beginInsertRows(QModelIndex(), rowCount() - 1, rowCount() - 1);
	CbmdosVfs_append(CbmdosFs_vfs(d->fs), newFile);
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
	return cmdr.c64font();
    }

    if (role == Qt::SizeHintRole)
    {
	QFontMetricsF fm(cmdr.c64font());
	return QSizeF(fm.ascent() * 29 * 13 / 14
		+ cmdr.style()->pixelMetric(QStyle::PM_ScrollBarExtent),
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
	return dirLine.toString(cmdr.lowerCase());
    }

    if (row == 0)
    {
	QString heading("0 ");
	CbmdosVfs_getDirHeader(vfs, buffer);
	PetsciiStr dirHeader((char *)buffer, 24);
	heading.append(dirHeader.toString(cmdr.lowerCase(), true));
	return heading;
    }

    CbmdosFs_getFreeBlocksLine(d->fs, buffer);
    PetsciiStr freeLine((char *)buffer, 16);
    return freeLine.toString(cmdr.lowerCase());
}

Qt::ItemFlags CbmdosFsModel::flags(const QModelIndex &index) const
{
    if (!d->fs) return Qt::ItemNeverHasChildren;
    if (!index.isValid()) return Qt::ItemIsDropEnabled;

    int row = index.row();
    int rowcount = rowCount();
    if (row > 0 && row < rowcount - 1)
    {
	return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled
	    | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    }

    return Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

QStringList CbmdosFsModel::mimeTypes() const
{
    return QStringList(fileposMimeType);
}

QMimeData *CbmdosFsModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.count() != 1) return 0;
    const QModelIndex &index = indexes.front();
    if (!index.isValid()) return 0;
    int row = index.row();
    if (row > 0 && row < rowCount() - 1)
    {
	--row;
	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);
	dataStream << row;
	QMimeData *mimeData = new QMimeData;
	mimeData->setData(fileposMimeType, itemData);
	return mimeData;
    }
    return 0;
}

Qt::DropActions CbmdosFsModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool CbmdosFsModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
	int row, int column, const QModelIndex &parent)
{
    (void) parent;
    if (!d->fs) return false;
    if (action != Qt::MoveAction) return false;
    if (row < 0 || column < 0) return false;
    if (row < 1) ++row;
    if (!data->hasFormat(fileposMimeType)) return false;
    int to = row - 1;
    const QByteArray &itemData = data->data(fileposMimeType);
    QDataStream dataStream(itemData);
    int from;
    dataStream >> from;
    CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
    if (to > from) --to;
    CbmdosVfs_move(vfs, to, from);
    return true;
}
