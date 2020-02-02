#ifndef V1541C_CBMDOSFILEWIDGET_H
#define V1541C_CBMDOSFILEWIDGET_H

#include <QGroupBox>

struct CbmdosFile;
struct CbmdosFileEventArgs;
class EditOperationCheck;
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
	void recordLengthChanged(int value);
	void forcedBlocksActiveChanged(int activeState);
	void forcedBlocksValueChanged(int value);
	void importFile();
	void exportFile();

    public:
	CbmdosFileWidget(QWidget *parent = 0);
	~CbmdosFileWidget();

	CbmdosFile *file() const;
	void setFile(CbmdosFile *file);
	void fileChanged(const CbmdosFileEventArgs *args);

    signals:
	void checkEditOperation(EditOperationCheck &check);
};

#endif
