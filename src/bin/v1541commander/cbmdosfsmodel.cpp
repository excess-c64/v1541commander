#include "cbmdosfsmodel.h"
#include "v1541commander.h"
#include "petsciistr.h"

#include <QSize>
#include <QFontMetrics>

extern "C" {
#include <1541img/cbmdosfs.h>
#include <1541img/cbmdosvfs.h>
#include <1541img/cbmdosfile.h>
}

CbmdosFsModel::CbmdosFsModel(CbmdosFs *fs, QObject *parent)
    : QAbstractListModel(parent), fs(fs)
{}

int CbmdosFsModel::rowCount(const QModelIndex &parent) const
{
    (void) parent;
    const CbmdosVfs *vfs = CbmdosFs_rvfs(fs);
    return CbmdosVfs_fileCount(vfs) + 2;
}

QVariant CbmdosFsModel::data(const QModelIndex &index, int role) const
{
    uint8_t buffer[27];

    if (role == Qt::FontRole)
    {
	return app.c64font();
    }

    if (role == Qt::SizeHintRole)
    {
	QFontMetrics fm(app.c64font());
	return QSize(fm.averageCharWidth() * 22, fm.ascent() - 1);
    }

    if (role != Qt::DisplayRole)
    {
	return QVariant();
    }

    int row = index.row();
    int rowcount = rowCount();
    const CbmdosVfs *vfs = CbmdosFs_rvfs(fs);

    if (row > 0 && row < rowcount - 1)
    {
	const CbmdosFile *file = CbmdosVfs_rfile(vfs, row-1);
	CbmdosFile_getDirLine(file, buffer);
	PetsciiStr dirLine((char *)buffer, 27);
	return dirLine.toString();
    }

    if (row == 0)
    {
	QString heading("0 ");
	CbmdosVfs_getDirHeader(vfs, buffer);
	PetsciiStr dirHeader((char *)buffer, 24);
	heading.append(dirHeader.toString(0, 1));
	return heading;
    }

    CbmdosFs_getFreeBlocksLine(fs, buffer);
    PetsciiStr freeLine((char *)buffer, 16);
    return freeLine.toString();
}

