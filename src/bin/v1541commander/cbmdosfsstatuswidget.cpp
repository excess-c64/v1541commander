#include "cbmdosfsstatuswidget.h"
#include "v1541commander.h"

CbmdosFsStatusWidget::CbmdosFsStatusWidget(QWidget *parent) :
    QLabel(parent)
{
    setPixmap(cmdr.statusLedGreen());
    setToolTip(tr("Filesystem OK"));
}

void CbmdosFsStatusWidget::setStatus(CbmdosFsStatus status)
{
    if (status & (
                CbmdosFsStatus::CFS_DISKFULL |
                CbmdosFsStatus::CFS_DIRFULL |
                CbmdosFsStatus::CFS_BROKEN ))
    {
        setPixmap(cmdr.statusLedRed());
    }
    else if (status & CbmdosFsStatus::CFS_INVALIDBAM)
    {
        setPixmap(cmdr.statusLedYellow());
    }
    else
    {
        setPixmap(cmdr.statusLedGreen());
    }

    if (status == CbmdosFsStatus::CFS_OK)
    {
        setToolTip(tr("Filesystem OK"));
    }
    else
    {
        QStringList stmsgs;
        if (status & CbmdosFsStatus::CFS_BROKEN)
        {
            stmsgs << tr("Broken Filesystem");
        }
        if (status & CbmdosFsStatus::CFS_DIRFULL)
        {
            stmsgs << tr("Directory full");
        }
        if (status & CbmdosFsStatus::CFS_DISKFULL)
        {
            stmsgs << tr("Disk full");
        }
        if (status & CbmdosFsStatus::CFS_INVALIDBAM)
        {
            stmsgs << tr("BAM is invalid");
        }
        setToolTip(stmsgs.join('\n'));
    }
}
