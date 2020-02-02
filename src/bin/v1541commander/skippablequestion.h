#ifndef V1541C_SKIPPABLEQUESTION_H
#define V1541C_SKIPPABLEQUESTION_H

#include <QDialog>

class QShowEvent;

class SkippableQuestion: public QDialog
{
    Q_OBJECT

    private:
	class priv;
	priv *d;

    public:
	SkippableQuestion(const QString &title, const QString &caption,
		QWidget *parent = nullptr);
	~SkippableQuestion();
	bool skip() const;

    protected:
	virtual void showEvent(QShowEvent *event);
};

#endif
