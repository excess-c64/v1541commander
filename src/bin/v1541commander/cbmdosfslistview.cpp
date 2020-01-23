#include "cbmdosfslistview.h"
#include "v1541commander.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

CbmdosFsListView::CbmdosFsListView() : QListView()
{
    setMinimumHeight(200);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    addAction(&cmdr.newFileAction());
    addAction(&cmdr.deleteFileAction());
    setContextMenuPolicy(Qt::ActionsContextMenu);
}

void CbmdosFsListView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->source() != this)
    {
        event->setDropAction(Qt::CopyAction);
    }
    QListView::dragEnterEvent(event);
    if (event->isAccepted() && event->source() != this)
    {
        event->setDropAction(Qt::CopyAction);
    }
}

void CbmdosFsListView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->source() != this)
    {
        event->setDropAction(Qt::CopyAction);
    }
    QListView::dragMoveEvent(event);
    if (event->isAccepted() && event->source() != this)
    {
        event->setDropAction(Qt::CopyAction);
    }
}

void CbmdosFsListView::dropEvent(QDropEvent *event)
{
    if (event->source() != this)
    {
        event->setDropAction(Qt::CopyAction);
    }
    QListView::dropEvent(event);
}

