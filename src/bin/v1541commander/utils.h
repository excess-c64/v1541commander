#ifndef V1541C_UTILS_H
#define V1541C_UTILS_H

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
#define qfopen(n,m) _wfopen(reinterpret_cast<const wchar_t *>((n).utf16()), \
        reinterpret_cast<const wchar_t *>(QString((m)).utf16()))
#define qfname(n) ((n).toUtf8().data())
#define qfnsan(n) ((n).replace(QRegularExpression("[|<>/\\\":]"), "_"))
#else
#define qfopen(n,m) fopen((n).toLocal8Bit().data(), m)
#define qfname(n) ((n).toLocal8Bit().data())
#define qfnsan(n) ((n).replace('/', '_'))
#endif

#endif
