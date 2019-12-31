#include "v1541commander.h"

#include <QCommandLineParser>
#include <QFileInfo>

#ifdef QT_STATICPLUGIN
#include <QtPlugin>
#endif

#ifdef _WIN32
#ifdef DEBUG
#include <windows.h>
#endif
#endif

int main(int argc, char **argv)
{
#ifdef QT_STATICPLUGIN
#ifdef _WIN32
    Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
    Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin);
#ifdef DEBUG
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif
#else
    Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#endif
#endif
    QCoreApplication::setApplicationVersion("0.1");

    V1541Commander commander(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("virtual 1541 disk image commander");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("d64file",
	    QCoreApplication::translate("main", "D64 disk image(s) to open."),
	    "[d64file ...]");
    parser.process(commander);

    commander.show();

    const QStringList &positionalArgs = parser.positionalArguments();
    for (QStringList::const_iterator i = positionalArgs.constBegin();
	    i != positionalArgs.constEnd(); ++i)
    {
	const QString &path = QFileInfo(*i).canonicalFilePath();
	if (!path.isEmpty()) commander.open(path);
    }

    return commander.exec();
}

