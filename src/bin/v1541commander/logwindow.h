#ifndef V1541C_LOGWINDOW_H
#define V1541C_LOGWINDOW_H

#include <QPlainTextEdit>

class LogWindow : public QPlainTextEdit
{
    Q_OBJECT

    public:
	LogWindow(QWidget *parent = 0);
};

#endif
