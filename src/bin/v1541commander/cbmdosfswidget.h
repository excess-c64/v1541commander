#ifndef V1541C_CBMDOSFSWIDGET_H
#define V1541C_CBMDOSFSWIDGET_H

#include <QGroupBox>

struct CbmdosFs;
struct CbmdosVfsEventArgs;
class PetsciiStr;

class CbmdosFsWidget : public QGroupBox
{
    Q_OBJECT

    private:
	class priv;
	priv *d;

    private slots:
	void nameChanged(const PetsciiStr &name);
	void idChanged(const PetsciiStr &id);
	void dosVerChanged(int val);
	void dosVerReset();

    public:
	CbmdosFsWidget(QWidget *parent = 0);
	~CbmdosFsWidget();

	CbmdosFs *fs() const;
	void setFs(CbmdosFs *fs);
	void fsChanged(const CbmdosVfsEventArgs *args);
};

#endif
