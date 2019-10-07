#ifndef V1541C_V1541COMMANDER_H
#define V1541C_V1541COMMANDER_H

#include <QApplication>

class QAction;

#define app (V1541Commander::instance())

class V1541Commander: public QApplication
{
    Q_OBJECT

    private:
        class priv;
        priv *d;

    private slots:
        void newImage();
        void open();
        void close();
        void exit();
        void windowActivated();
        void windowClosed();
        void windowContentChanged();
        
    public:
        V1541Commander(int &argc, char **argv);
        ~V1541Commander();
        QFont &c64font();
        QAction &newAction();
        QAction &openAction();
        QAction &closeAction();
        QAction &exitAction();

        static V1541Commander &instance();
};

#endif
