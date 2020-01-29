#ifndef V1541C_CBMDOSFILEMIMEDATA_H
#define V1541C_CBMDOSFILEMIMEDATA_H

#include <QMimeData>

struct CbmdosFile;
class CbmdosFsModel;

class CbmdosFileMimeData: public QMimeData
{
    Q_OBJECT

    private:
	class priv;
	priv *d;

    public:
	CbmdosFileMimeData(
		const CbmdosFile *file, int pos, const CbmdosFsModel *model);
	virtual ~CbmdosFileMimeData();
	virtual bool hasFormat(const QString &mimeType) const;
	virtual QStringList formats() const;
	const CbmdosFile *file() const;
	int pos() const;

	static const QString &internalFormat();

    protected:
	virtual QVariant retrieveData(
		const QString &mimeType, QVariant::Type type) const;
};

#endif
