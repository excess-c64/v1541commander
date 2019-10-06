#ifndef V1541C_CBMDOSFSMODEL_H
#define V1541C_CBMDOSFSMODEL_H

#include <QAbstractListModel>
#include <QFont>

struct CbmdosFs;

class CbmdosFsModel: public QAbstractListModel
{
    Q_OBJECT

    private:
	CbmdosFs *fs;
	QFont c64font;

    public:
	CbmdosFsModel(CbmdosFs *fs, QObject *parent = nullptr);
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index,
		int role = Qt::DisplayRole) const;
};

#endif
