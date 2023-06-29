#include "v1541commander.h"

#include <QCommandLineParser>
#include <QDataStream>
#include <QFileInfo>
#include <QLocalSocket>
#include <QTranslator>

#ifdef QT_STATICPLUGIN
#include <QtPlugin>
#else
#include <QLibraryInfo>
#endif

#ifdef _WIN32
#include <windows.h>
#include <QtGlobal>
#else
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifndef _WIN32
void handlesig(int sig)
{
    (void) sig;
    _exit(0);
}
#endif

int main(int argc, char **argv)
{
#ifndef _WIN32
#ifndef __APPLE__
#ifndef DEBUG
    pid_t pid = fork();
    if (pid < 0) return 1;
    if (pid)
    {
	signal(SIGHUP, handlesig);
	for (;;)
	{
	    int crc;
	    if (wait(&crc) > 0)
	    {
		if (WIFEXITED(crc)) return 0;
		else return WEXITSTATUS(crc);
	    }
	}
    }
    setsid();
#endif
#endif
#endif

#ifdef QT_STATICPLUGIN
#ifdef _WIN32
    Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin);
#endif
#else
    Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#endif
#endif

    QCoreApplication::setOrganizationName("Excess");
    QCoreApplication::setApplicationName("V1541Commander");
    QCoreApplication::setApplicationVersion("1.1");

    QTranslator translator;
    QString qmsuffix = QLocale::system().name();
    if (!translator.load(":/qm/v1541commander-"+qmsuffix))
    {
        qmsuffix = QLocale::languageToString(QLocale::system().language());
        translator.load(":/qm/v1541commander-"+qmsuffix);
    }
    V1541Commander commander(argc, argv, &translator);

#ifdef QT_STATICPLUGIN
    QString qttransloc(":/qm");
#else
    QString qttransloc = QLibraryInfo::location(
	    QLibraryInfo::TranslationsPath);
#endif
    QTranslator qttrans;
    qmsuffix = QLocale::system().name();
    if (!qttrans.load(qttransloc+"/qtbase_"+qmsuffix))
    {
        qmsuffix = QLocale::languageToString(QLocale::system().language());
        qttrans.load(qttransloc+":/qtbase_"+qmsuffix);
    }
    commander.installTranslator(&qttrans);

#ifdef _WIN32
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0) \
    && QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    // work around a QSpinBox layout bug in Qt 5.13:
    commander.setStyleSheet("QFoo{}");
#endif
#endif

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main",
		"virtual 1541 disk image commander"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QCoreApplication::translate("main", "file"),
	    QCoreApplication::translate("main", "file(s) to open (D64 disk "
		"images) or import (ZipCode, LyNX)."),
	    QCoreApplication::translate("main", "[file ...]"));
    parser.process(commander);

    const QStringList &positionalArgs = parser.positionalArguments();

    if (commander.isPrimaryInstance())
    {
#ifndef _WIN32
#ifndef __APPLE__
#ifndef DEBUG
        freopen("/dev/null", "r", stdin);
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);
	kill(getppid(), SIGHUP);
#endif
#endif
#endif

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
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
		stream.startTransaction();
#endif
		stream >> mainpid;
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
#ifdef _WIN32
		if (stream.commitTransaction())
		{
		    AllowSetForegroundWindow(DWORD(mainpid));
		}
#else
		stream.commitTransaction();
#endif
#else
#ifdef _WIN32
		AllowSetForegroundWindow(DWORD(mainpid));
#endif
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
            sock.state() == QLocalSocket::UnconnectedState ||
                sock.waitForDisconnected(1000);
	}
	return 0;
    }
}

