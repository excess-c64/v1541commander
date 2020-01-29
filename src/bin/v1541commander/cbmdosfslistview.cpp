#include "cbmdosfslistview.h"
#include "cbmdosfsmodel.h"
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
    setDefaultDropAction(Qt::CopyAction);
    addAction(&cmdr.newFileAction());
    addAction(&cmdr.deleteFileAction());
    setContextMenuPolicy(Qt::ActionsContextMenu);
}

void CbmdosFsListView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->source() == this)
    {
        event->setDropAction(Qt::MoveAction);
    }
    QListView::dragEnterEvent(event);
    if (event->source() == this)
    {
        event->setDropAction(Qt::MoveAction);
    }
}

void CbmdosFsListView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->source() == this)
    {
        event->setDropAction(Qt::MoveAction);
    }
    QListView::dragMoveEvent(event);
    if (event->source() == this)
    {
        event->setDropAction(Qt::MoveAction);
    }
}

void CbmdosFsListView::dropEvent(QDropEvent *event)
{
    if (event->source() == this)
    {
        event->setDropAction(Qt::MoveAction);
    }
    QListView::dropEvent(event);
}

