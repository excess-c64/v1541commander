#include "cbmdosfilemimedata.h"
#include "cbmdosfsmodel.h"
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
	priv(const CbmdosFile *file, int pos, const CbmdosFsModel *model);
	const CbmdosFile *file;
	int pos;
	const CbmdosFsModel *model;
	QString fileName;
	bool haveContent;
	bool tmpSaved;
};

CbmdosFileMimeData::priv::priv(const CbmdosFile *file, int pos,
	const CbmdosFsModel *model) :
    file(file), pos(pos), model(model)
{
    uint8_t namelen;
    const char *name = CbmdosFile_name(file, &namelen);
    char utf8name[65];
    petscii_toUtf8(utf8name, 65, name, namelen, cmdr.lowerCase(), 1, 0, 0);
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
    fileName = hostFileName;
    const FileData *data = CbmdosFile_rdata(file);
    haveContent = data && FileData_size(data);
    tmpSaved = false;
}

CbmdosFileMimeData::CbmdosFileMimeData(const CbmdosFile *file, int pos,
	const CbmdosFsModel *model)
{
    d = new priv(file, pos, model);
    setText(d->fileName);
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

const CbmdosFile *CbmdosFileMimeData::file() const
{
    return d->file;
}

int CbmdosFileMimeData::pos() const
{
    return d->pos;
}

const QString &CbmdosFileMimeData::internalFormat()
{
    return internalType;
}

QVariant CbmdosFileMimeData::retrieveData(
	const QString &mimeType, QVariant::Type type) const
{
    if (d->haveContent && mimeType == "text/uri-list" && !d->tmpSaved)
    {
	const QTemporaryDir *td = d->model->tmpDir();
	QString fullName = td->filePath(d->fileName);
	FILE *f = qfopen(fullName, "wb");
	if (f)
	{
	    int rc = CbmdosFile_exportRaw(d->file, f);
	    fclose(f);
	    if (rc >= 0)
	    {
		const_cast<CbmdosFileMimeData *>(this)->setUrls({
			QUrl::fromLocalFile(fullName)
			});
		d->tmpSaved = true;
	    }
	}
    }

    return QMimeData::retrieveData(mimeType, type);
}
