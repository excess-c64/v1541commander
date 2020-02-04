#include "cbmdosfsmodel.h"
#include "cbmdosfilemimedata.h"
#include "editoperationcheck.h"
#include "v1541commander.h"
#include "petsciistr.h"
#include "settings.h"
#include "utils.h"

#include <QFont>
#include <QList>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QUrl>

#include <1541img/cbmdosfs.h>
#include <1541img/cbmdosvfs.h>
#include <1541img/cbmdosvfseventargs.h>
#include <1541img/cbmdosfile.h>
#include <1541img/event.h>
#include <1541img/petscii.h>

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
	~priv();
	QTemporaryDir *tmpDir;
	CbmdosFs *fs;
	QSizeF itemSize;
};

CbmdosFsModel::priv::priv() :
    tmpDir(0), fs(0), itemSize()
{}

CbmdosFsModel::priv::~priv()
{
    delete tmpDir;
}

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
    EditOperationCheck check(newFile);
    emit checkEditOperation(check);
    if (!check.allowed())
    {
	CbmdosFile_destroy(newFile);
	return;
    }
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

void CbmdosFsModel::setItemSize(QSizeF size)
{
    d->itemSize = size;
}

const QTemporaryDir *CbmdosFsModel::tmpDir() const
{
    if (!d->tmpDir) d->tmpDir = new QTemporaryDir();
    return d->tmpDir;
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
	return d->itemSize;
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
	return dirLine.toString(cmdr.settings().lowercase());
    }

    if (row == 0)
    {
	QString heading("0 ");
	CbmdosVfs_getDirHeader(vfs, buffer);
	PetsciiStr dirHeader((char *)buffer, 24);
	heading.append(dirHeader.toString(cmdr.settings().lowercase(), true));
	return heading;
    }

    CbmdosFs_getFreeBlocksLine(d->fs, buffer);
    PetsciiStr freeLine((char *)buffer, 16);
    return freeLine.toString(cmdr.settings().lowercase());
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
    return QStringList({
	    CbmdosFileMimeData::internalFormat(),
	    "text/uri-list"});
}

QMimeData *CbmdosFsModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.empty()) return 0;
    CbmdosFileMimeData *mimeData = new CbmdosFileMimeData(this);
    CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
    for (QModelIndexList::const_iterator i = indexes.cbegin();
	    i != indexes.cend(); ++i)
    {
	if (i->isValid())
	{
	    int row = i->row();
	    if (row > 0 && row < rowCount() - 1)
	    {
		mimeData->addFile(CbmdosVfs_rfile(vfs, row-1), row-1);
	    }
	}
    }
    if (mimeData->files().empty())
    {
	delete mimeData;
	return 0;
    }
    return mimeData;
}

Qt::DropActions CbmdosFsModel::supportedDropActions() const
{
    return Qt::MoveAction|Qt::CopyAction;
}

bool CbmdosFsModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
	int row, int column, const QModelIndex &parent)
{
    (void) parent;
    if (!d->fs) return false;
    if (CbmdosFs_status(d->fs) & CFS_BROKEN) return false;
    const CbmdosFileMimeData *fileData =
	qobject_cast<const CbmdosFileMimeData *>(data);
    if (fileData)
    {
	if (action == Qt::MoveAction)
	{
	    if (row < 0 || column < 0) return false;
	    if (row < 1) ++row;
	    int to = row - 1;
	    QList<int> fromRows;
	    for (int r = 0; r < rowCount() - 2; ++r) fromRows.append(r);
	    CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
	    for (int i = 0; i < fileData->filePositions().size(); ++i)
	    {
		int from = fromRows.at(fileData->filePositions().at(i));
		if (to > from) --to;
		CbmdosVfs_move(vfs, to, from);
		int oldfrom = fromRows.at(to);
		fromRows.removeAt(to);
		fromRows.insert(from, oldfrom);
		++to;
	    }
	    return true;
	}
	if (action == Qt::CopyAction)
	{
	    if (fileData->files().empty()) return false;
	    for (int i = 0; i < fileData->files().size(); ++i)
	    {
		CbmdosFile *copy = CbmdosFile_clone(fileData->files().at(i));
		addFile(createIndex(row+i, column), copy);
	    }
	    return true;
	}
    }
    else if (data->hasUrls())
    {
	QList<QUrl> urls = data->urls();
	int fileno = 0;
	for (QList<QUrl>::const_iterator i = urls.cbegin();
		i != urls.cend(); ++i)
	{
	    if (i->isLocalFile())
	    {
		QFileInfo file(i->toLocalFile());
		FILE *fp = qfopen(file.canonicalFilePath(), "rb");
		if (fp)
		{
		    CbmdosFile *newFile = CbmdosFile_create();
		    char name[17];
		    QByteArray basename = file.completeBaseName().toUtf8();
		    size_t nameLen = petscii_fromUtf8(name, 17,
			    basename.data(), basename.size(), PC_UPPER, 1, 0);
		    if(--nameLen > 16) nameLen = 16;
		    CbmdosFile_setName(newFile, name, nameLen);
		    QString ext = file.suffix().toLower();
		    if (ext == "seq" || QRegularExpression("^s\\d{2}$")
			    .match(ext).hasMatch())
		    {
			CbmdosFile_setType(newFile, CbmdosFileType::CFT_SEQ);
		    }
		    else if (ext == "usr" || QRegularExpression("^u\\d{2}$").
			    match(ext).hasMatch())
		    {
			CbmdosFile_setType(newFile, CbmdosFileType::CFT_USR);
		    }
		    else if (ext == "prg" || QRegularExpression("^p\\d{2}$").
			    match(ext).hasMatch())
		    {
			CbmdosFile_setType(newFile, CbmdosFileType::CFT_PRG);
		    }
		    else if (ext == "rel" || QRegularExpression("^r\\d{2}$").
			    match(ext).hasMatch())
		    {
			CbmdosFile_setType(newFile, CbmdosFileType::CFT_REL);
		    }
		    else
		    {
			CbmdosFile_setType(newFile,
				cmdr.settings().defaultImportType());
		    }
		    int rc = CbmdosFile_import(newFile, fp);
		    fclose(fp);
		    if (rc < 0)
		    {
			CbmdosFile_destroy(newFile);
			break;
		    }
		    addFile(createIndex(row + (fileno++), column), newFile);
		}
	    }
	}
	return fileno > 0;
    }
    return false;
}
