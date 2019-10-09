#ifndef V1541C_CBMDOSFILEWIDGET_H
#define V1541C_CBMDOSFILEWIDGET_H

#include <QGroupBox>

struct CbmdosFile;
class PetsciiStr;

class CbmdosFileWidget: public QGroupBox
{
    Q_OBJECT

    private:
	class priv;
	priv *d;

    private slots:
	void nameChanged(const PetsciiStr &name);
	void typeChanged(int typeIndex);
	void lockedChanged(int lockedState);
	void closedChanged(int closedState);
	void forcedBlocksActiveChanged(int activeState);
	void forcedBlocksValueChanged(int value);

    public:
	CbmdosFileWidget(QWidget *parent = 0);
	~CbmdosFileWidget();

	CbmdosFile *file() const;
	void setFile(CbmdosFile *file);
};

#endif
