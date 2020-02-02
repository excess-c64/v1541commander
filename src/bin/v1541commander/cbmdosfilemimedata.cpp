#include "cbmdosfilemimedata.h"
#include "cbmdosfsmodel.h"
#include "settings.h"
#include "utils.h"
#include "v1541commander.h"

#include <QTemporaryDir>
#include <QUrl>

#include <1541img/cbmdosfile.h>
#include <1541img/filedata.h>
#include <1541img/petscii.h>

static const QString internalType = "application/x.v1541c.cbmdosfile";

class CbmdosFileMimeData::priv
{
    public:
	priv(const CbmdosFsModel *model);
	void addFile(const CbmdosFile *file, int pos);
	const CbmdosFsModel *model;
	QList<const CbmdosFile *> files;
	QList<int> positions;
	QStringList fileNames;
	bool haveContent;
	bool tmpSaved;
};

CbmdosFileMimeData::priv::priv(const CbmdosFsModel *model) :
    model(model), files(), positions(), fileNames(),
    haveContent(false), tmpSaved(false)
{}

void CbmdosFileMimeData::priv::addFile(const CbmdosFile *file, int pos)
{
    files.append(file);
    positions.append(pos);
    uint8_t namelen;
    const char *name = CbmdosFile_name(file, &namelen);
    char utf8name[65];
    petscii_toUtf8(utf8name, 65, name, namelen,
	    cmdr.settings().lowercase(), 1, 0, 0);
    QString hostFileName = qfnsan(QString::fromUtf8(utf8name));
    switch (CbmdosFile_type(file))
    {
	case CbmdosFileType::CFT_DEL:
	    hostFileName.append(".del");
	    break;
	case CbmdosFileType::CFT_SEQ:
	    hostFileName.append(".seq");
	    break;
	case CbmdosFileType::CFT_USR:
	    hostFileName.append(".usr");
	    break;
	case CbmdosFileType::CFT_PRG:
	    hostFileName.append(".prg");
	    break;
	case CbmdosFileType::CFT_REL:
	    hostFileName.append(".rel");
	    break;
    }
    fileNames.append(hostFileName);
    if (!haveContent)
    {
	const FileData *data = CbmdosFile_rdata(file);
	haveContent = data && FileData_size(data);
    }
}

CbmdosFileMimeData::CbmdosFileMimeData(const CbmdosFsModel *model)
{
    d = new priv(model);
}

CbmdosFileMimeData::~CbmdosFileMimeData()
{
    delete d;
}

bool CbmdosFileMimeData::hasFormat(const QString &mimeType) const
{
    if (mimeType == internalType) return true;
    if (mimeType == "text/uri-list") return d->haveContent;
    return QMimeData::hasFormat(mimeType);
}

QStringList CbmdosFileMimeData::formats() const
{
    QStringList formats = QMimeData::formats();
    if (d->haveContent && !d->tmpSaved)
    {
	formats.prepend("text/uri-list");
    }
    return formats;
}

void CbmdosFileMimeData::addFile(const CbmdosFile *file, int pos)
{
    d->addFile(file, pos);
    setText(d->fileNames.join('\n'));
}

const QList<const CbmdosFile *> &CbmdosFileMimeData::files() const
{
    return d->files;
}

const QList<int> &CbmdosFileMimeData::filePositions() const
{
    return d->positions;
}

/* static */
const QString &CbmdosFileMimeData::internalFormat()
{
    return internalType;
}

QVariant CbmdosFileMimeData::retrieveData(
	const QString &mimeType, QVariant::Type type) const
{
    if (d->haveContent && mimeType == "text/uri-list" && !d->tmpSaved)
    {
	QList<QUrl> uris;
	const QTemporaryDir *td = d->model->tmpDir();
	for (int i = 0; i < d->files.size(); ++i)
	{
	    QString fullName = td->filePath(d->fileNames.at(i));
	    FILE *f = qfopen(fullName, "wb");
	    if (f)
	    {
		int rc = CbmdosFile_exportRaw(d->files.at(i), f);
		fclose(f);
		if (rc >= 0)
		{
		    uris.append(QUrl::fromLocalFile(fullName));
		}
	    }
	}
	if (!uris.empty())
	{
	    const_cast<CbmdosFileMimeData *>(this)->setUrls(uris);
	    d->tmpSaved = true;
	}
    }

    return QMimeData::retrieveData(mimeType, type);
}
