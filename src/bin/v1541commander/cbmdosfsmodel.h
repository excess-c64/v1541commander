#ifndef V1541C_CBMDOSFSMODEL_H
#define V1541C_CBMDOSFSMODEL_H

#include <QAbstractListModel>
#include <QItemSelectionModel>
#include <QSizeF>

struct CbmdosFile;
struct CbmdosFs;
struct CbmdosVfsEventArgs;
class QTemporaryDir;

class CbmdosFsModel: public QAbstractListModel
{
    Q_OBJECT

    private:
	class priv;
	priv *d;

    public:
	CbmdosFsModel(QObject *parent = nullptr);
	~CbmdosFsModel();
	CbmdosFs *fs() const;
	void setFs(CbmdosFs *fs);
	void fsChanged(const CbmdosVfsEventArgs *args);
	void deleteAt(const QModelIndex &at);
	void addFile(const QModelIndex &at, CbmdosFile *newFile);
	void setItemSize(QSizeF size);
	const QTemporaryDir *tmpDir() const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index,
		int role = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual QStringList mimeTypes() const;
	virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
	virtual Qt::DropActions supportedDropActions() const;
	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
		int row, int column, const QModelIndex &parent);

    signals:
	void selectedIndexChanged(const QModelIndex &to,
		QItemSelectionModel::SelectionFlags command);
	void modified();
};

#endif
