#include <QListView>

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;

class CbmdosFsListView : public QListView
{
    Q_OBJECT

    public:
        CbmdosFsListView();

    protected:
        virtual void dragEnterEvent(QDragEnterEvent *event);
        virtual void dragMoveEvent(QDragMoveEvent *event);
        virtual void dropEvent(QDropEvent *event);
};

