#include "v1541commander.h"

#include <QCommandLineParser>
#include <QDataStream>
#include <QFileInfo>
#include <QLocalSocket>

#ifdef QT_STATICPLUGIN
#include <QtPlugin>
#endif

#ifdef _WIN32
#include <windows.h>
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
    QCoreApplication::setOrganizationName("Excess");
    QCoreApplication::setApplicationName("V1541Commander");
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

    const QStringList &positionalArgs = parser.positionalArguments();

    if (commander.isPrimaryInstance())
    {
	commander.show();

	for (QStringList::const_iterator i = positionalArgs.constBegin();
		i != positionalArgs.constEnd(); ++i)
	{
	    const QString &path = QFileInfo(*i).canonicalFilePath();
	    if (!path.isEmpty()) commander.open(path);
	}

	return commander.exec();
    }
    else
    {
	QLocalSocket sock;
	sock.connectToServer(commander.instanceServerName());
	if (sock.state() == QLocalSocket::ConnectedState
		|| sock.waitForConnected(5000))
	{
	    QDataStream stream(&sock);
	    if (sock.waitForReadyRead(5000))
	    {
		qint64 mainpid;
		stream.startTransaction();
		stream >> mainpid;
#ifdef _WIN32
		if (stream.commitTransaction())
		{
		    AllowSetForegroundWindow(DWORD(mainpid));
		}
#else
		stream.commitTransaction();
#endif
	    }
	    for (QStringList::const_iterator i = positionalArgs.constBegin();
		    i != positionalArgs.constEnd(); ++i)
	    {
		const QString &path = QFileInfo(*i).canonicalFilePath();
		if (!path.isEmpty()) stream << path;
	    }
	    sock.flush();
	    sock.disconnectFromServer();
	}
    }
}

