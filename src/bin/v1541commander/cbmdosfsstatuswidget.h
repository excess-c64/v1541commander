#ifndef V1541C_CBMDOSFSSTATUSWIDGET_H
#define V1541C_CBMDOSFSSTATUSWIDGET_H

#include <QLabel>

#include <1541img/cbmdosfs.h>

class CbmdosFsStatusWidget : public QLabel
{
    Q_OBJECT

    public:
        CbmdosFsStatusWidget(QWidget *parent = 0);
        void setStatus(CbmdosFsStatus status);
};

#endif
