#ifndef V1541C_MAINWINDOW_H
#define V1541C_MAINWINDOW_H

#include <QMainWindow>

class QWidget;
class QEvent;

class MainWindow: public QMainWindow
{
    Q_OBJECT

    private:
        class priv;
        priv *d;

    public:
        enum Content
        {
            None,
            Image
        };

	MainWindow();
        ~MainWindow();
        Content content();
        virtual bool event(QEvent *e);
        virtual void closeEvent(QCloseEvent *e);
        virtual QSize sizeHint() const;

    signals:
        void activated();
        void closed();
        void contentChanged();

    public slots:
        void openImage(QString &imgFile);
        void closeDocument();
};

#endif
