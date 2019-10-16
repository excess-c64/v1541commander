#include "v1541commander.h"

#ifdef QT_STATICPLUGIN
#include <QtPlugin>
#endif

int main(int argc, char **argv)
{
#ifdef QT_STATICPLUGIN
#ifdef _WIN32
    Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
    Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin);
#else
    Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#endif
#endif
    V1541Commander commander(argc, argv);
    return commander.exec();
}

