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
        void dragEnterEvent(QDragEnterEvent *event);
        void dragMoveEvent(QDragMoveEvent *event);
        void dropEvent(QDropEvent *event);
};

