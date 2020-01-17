#ifndef V1541C_LOGWINDOW_H
#define V1541C_LOGWINDOW_H

#include <QPlainTextEdit>

class LogWindow : public QPlainTextEdit
{
    Q_OBJECT

    public:
	LogWindow(QWidget *parent = 0);

    protected:
	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;

    signals:
	void logLineAppended(const QString &line);
};

#endif
