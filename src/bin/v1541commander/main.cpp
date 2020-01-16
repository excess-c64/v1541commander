#include "v1541commander.h"

#include <QCommandLineParser>
#include <QDataStream>
#include <QFileInfo>
#include <QLocalSocket>
#include <QTranslator>

#ifdef QT_STATICPLUGIN
#include <QtPlugin>
#endif

#ifdef _WIN32
#include <windows.h>
#include <QFont>
#include <QtGlobal>
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
    QCoreApplication::setApplicationVersion("1.0");

    QTranslator translator;
    QString qmsuffix = QLocale::system().name();
    if (!translator.load(":/qm/v1541commander-"+qmsuffix))
    {
        qmsuffix = QLocale::languageToString(QLocale::system().language());
        translator.load(":/qm/v1541commander-"+qmsuffix);
    }
    V1541Commander commander(argc, argv, &translator);

#ifdef _WIN32
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0) \
    || QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    // work around a QSpinBox layout bug in Qt 5.13:
    commander.setStyleSheet("QFoo{}");
#endif
    // correct the default font:
    NONCLIENTMETRICSW ncm;
    ncm.cbSize = sizeof ncm;
    if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof ncm, &ncm, 0))
    {
        if (ncm.lfMessageFont.lfHeight < 0) ncm.lfMessageFont.lfHeight *= -1;
        QFont sysfont(QString::fromWCharArray(ncm.lfMessageFont.lfFaceName));
        sysfont.setPixelSize(ncm.lfMessageFont.lfHeight);
        commander.setFont(sysfont);
    }
#endif

    QCommandLineParser parser;
    parser.setApplicationDescription("virtual 1541 disk image commander");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file",
	    QCoreApplication::translate("main", "file(s) to open (D64 disk "
		"images) or import (ZipCode, LyNX)."),
	    "[file ...]");
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
	return 0;
    }
}

