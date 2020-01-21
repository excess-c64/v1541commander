#include <wchar.h>

#define OEMRESOURCE
#include <windows.h>
#include <accctrl.h>
#include <aclapi.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shlobj.h>

#define WC_mainWindow L"V1541CommanderSetup"
#define CID_register 0x101
#define CID_cancel 0x102
#define CID_d64 0x103
#define CID_lynx 0x104
#define CID_zipcode 0x105

static HINSTANCE instance;
static HWND mainWindow;
static NONCLIENTMETRICSW ncm;
static HFONT messageFont;
static TEXTMETRICW messageFontMetrics;
static int buttonWidth;
static int buttonHeight;

static int ftD64 = 1;
static int ftLynx = 1;
static int ftZipcode = 0;

static WCHAR commanderPath[MAX_PATH];
static WCHAR commanderQuotedPath[MAX_PATH+2];
static WCHAR commanderRegCommand[MAX_PATH+7];
static DWORD commanderRegCommandSize;
static WCHAR regValTmp[MAX_PATH+16];

#define FEXIDX (sizeof "SOFTWARE\\Microsoft\\Windows\\" \
    "CurrentVersion\\Explorer\\FileExts\\" - 1)
#define UCNLEN ((FEXIDX) + sizeof ".xxxxxxxx\\UserChoice")

static WCHAR userChoiceName[UCNLEN] = L"SOFTWARE\\Microsoft\\"
    "Windows\\CurrentVersion\\Explorer\\FileExts\\";

static PSID sid = 0;

static const WCHAR *locales[] = {
    L"C",
    L"de",
};

enum textid {
    TID_title,
    TID_notfound_title,
    TID_notfound_message,
    TID_regsuccess_title,
    TID_regsuccess_message,
    TID_regfailure_title,
    TID_regfailure_message,
    TID_select_types,
    TID_prg_warn,
    TID_register,
    TID_cancel,
    TID_d64,
    TID_lynx,
    TID_zipcode,

    TID_N_texts
};

static const WCHAR *locale_texts[][TID_N_texts] = {
    {
	L"V1541Commander Setup",
	L"v1541commander.exe not found",
	L"Please run this from the same directory\n"
	    L"where v1541commander.exe is located.",
	L"File types registered",
	L"The selected file types were associated\n"
	    L"to V1541Commander successfully.",
	L"Registration failed",
	L"There was an unexpected error registering\n"
	    L"the selected file types to V1541Commander.",
	L"This tool registers V1541Commander with windows\n"
            L"and associates it with all file types that can be\n"
            L"opened or imported.\n"
            L"\n"
            L"Select here for which file types V1541Commander\n"
            L"should be set as the default application:",
        L"It's not recommended to set V1541Commander as the\n"
            L"default application for .prg files, as most .prg files\n"
            L"aren't Zipcode files.",
	L"Register",
        L"Cancel",
	L"1541 disk images (.d64)",
	L"LyNX archives (.lnx)",
	L"Zipcode files (.prg)",
    },
    {
	L"V1541Commander Einrichtung",
	L"v1541commander.exe nicht gefunden",
	L"Bitte starten Sie dieses Programm aus dem\n"
	    L"gleichen Verzeichnis, in dem sich auch\n"
	    L"v1541commander.exe befindet.",
	L"Dateitypen registriert",
	L"Die gewählten Dateitypen wurden erfolgreich\n"
	    L"V1541Commander zugeordnet.",
	L"Registrierung fehlgeschlagen",
	L"Bei der Registrierung der gewählten Dateitypen\n"
	    L"für V1541Commander ist ein unerwarteter Fehler\n"
	    L"aufgetreten.",
	L"Dieses Programm registriert V1541Commander in Windows\n"
            L"und verknüpft es mit allen Dateitypen, die geöffnet oder\n"
            L"importiert werden können.\n"
            L"\n"
            L"Wählen Sie hier die Dateitypen, für die V1541Commander\n"
            L"als Standardanwendung gesetzt werden soll:",
        L"Es wird nicht empfohlen, V1541Commander als Standard-\n"
            L"anwendung für .prg-Dateien zu setzen, da die meisten .prg-\n"
            L"Dateien keine Zipcode Dateien sind.",
	L"Registrieren",
        L"Abbrechen",
	L"1541 Diskettenabbilddateien (.d64)",
	L"LyNX Archive (.lnx)",
	L"Zipcode Dateien (.prg)",
    },
};

static const WCHAR **texts = locale_texts[0];

static void init(void)
{
    INITCOMMONCONTROLSEX icx;
    icx.dwSize = sizeof icx;
    icx.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icx);
    ncm.cbSize = sizeof ncm;
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
    messageFont = CreateFontIndirectW(&ncm.lfMessageFont);
    HDC dc = GetDC(0);
    SelectObject(dc, (HGDIOBJ) messageFont);
    GetTextMetricsW(dc, &messageFontMetrics);
    SIZE sampleSize;
    GetTextExtentExPointW(dc,
            L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
            52, 0, 0, 0, &sampleSize);
    ReleaseDC(0, dc);
    buttonWidth = MulDiv(sampleSize.cx, 50, 4 * 52);
    buttonHeight = MulDiv(messageFontMetrics.tmHeight, 14, 8);
    instance = GetModuleHandleW(0);

    WCHAR lang[10];
    if (GetLocaleInfoW(LOCALE_USER_DEFAULT,
		LOCALE_SISO639LANGNAME, lang, 10) > 0)
    {
	for (size_t i = 1; i < sizeof locales / sizeof *locales; ++i)
	{
	    if (!wcscmp(locales[i], lang))
	    {
		texts = locale_texts[i];
		break;
	    }
	}
    }

    HANDLE token;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
    {
        DWORD size = 0;
        GetTokenInformation(token, TokenUser, 0, 0, &size);
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            TOKEN_USER *user = malloc(size);
            if (GetTokenInformation(token, TokenUser, user, size, &size))
            {
                if (IsValidSid(user->User.Sid))
                {
                    DWORD sidSize = GetLengthSid(user->User.Sid);
                    sid = malloc(sidSize);
                    CopySid(sidSize, sid, user->User.Sid);
                }
            }
            free(user);
        }
        CloseHandle(token);
    }
}

static int getCommanderPath(void)
{
    GetModuleFileNameW(instance, commanderPath, MAX_PATH);
    WCHAR *pos = wcsrchr(commanderPath, L'\\');
    if (!pos) return 0;
    wcscpy(pos+1, L"v1541commander.exe");
    int rc = PathFileExistsW(commanderPath);
    if (rc)
    {
        size_t pathlen = wcslen(commanderPath);
        commanderQuotedPath[0] = L'"';
        wcscpy(commanderQuotedPath+1, commanderPath);
        commanderQuotedPath[pathlen+1] = L'"';
        commanderQuotedPath[pathlen+2] = L'\0';
        wcscpy(commanderRegCommand, commanderQuotedPath);
        wcscpy(commanderRegCommand + pathlen + 2, L" \"%1\"");
        commanderRegCommandSize = (pathlen + 8)
            * sizeof *commanderRegCommand;
    }
    return rc;
}

static void regTreeDel(HKEY key, LPCWSTR subKey)
{
    HKEY sub;
    if (RegOpenKeyExW(key, subKey, 0, DELETE|KEY_ENUMERATE_SUB_KEYS, &sub)
	    == ERROR_SUCCESS)
    {
	WCHAR subName[256];
	DWORD subNameLen;
	while (subNameLen = 256,
		RegEnumKeyExW(sub, 0, subName, &subNameLen, 0, 0, 0, 0)
		== ERROR_SUCCESS)
	{
	    regTreeDel(sub, subName);
	}
	RegCloseKey(sub);
	RegDeleteKeyW(key, subKey);
    }
}

static void regUnprotect(LPCWSTR subKey)
{
    if (!sid) return;
    SECURITY_DESCRIPTOR sd;
    HKEY sub;
    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
    {
        return;
    }
    if (RegOpenKeyExW(HKEY_CURRENT_USER, subKey, 0, WRITE_DAC, &sub)
            != ERROR_SUCCESS) return;
    EXPLICIT_ACCESS_W ea[1] = { 0 };
    PACL acl = 0;
    ea[0].grfAccessPermissions = GENERIC_ALL;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance = SUB_OBJECTS_ONLY_INHERIT;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
    ea[0].Trustee.ptstrName = (void *)sid;
    if (SetEntriesInAclW(1, ea, 0, &acl) != ERROR_SUCCESS)
    {
        RegCloseKey(sub);
        return;
    }
    if (!SetSecurityDescriptorDacl(&sd, 1, acl, 0))
    {
        LocalFree(acl);
        RegCloseKey(sub);
        return;
    }
    RegSetKeySecurity(sub, DACL_SECURITY_INFORMATION, &sd);
    LocalFree(acl);
    RegCloseKey(sub);
}

static int setFriendlyTypeNameAndIcon(HKEY tkey, int nameId)
{
    DWORD valSize = wcslen(commanderQuotedPath) + 2;
    regValTmp[0] = L'@';
    wcscpy(regValTmp+1, commanderQuotedPath);
    int numlen = swprintf(regValTmp + valSize - 1, PATH_MAX - valSize,
            L",-%d", nameId);
    if (numlen < 1) return 0;
    valSize += numlen;
    valSize *= sizeof *regValTmp;
    if (RegSetValueExW(tkey, L"FriendlyTypeName", 0, REG_SZ,
		(const BYTE *)regValTmp, valSize) != ERROR_SUCCESS)
    {
	return 0;
    }

    HKEY tmp;
    if (RegCreateKeyExW(tkey, L"DefaultIcon", 0, 0, 0, KEY_WRITE, 0, &tmp, 0)
	    != ERROR_SUCCESS) return 0;

    valSize = wcslen(commanderQuotedPath) + 1;
    wcscpy(regValTmp, commanderQuotedPath);
    numlen = swprintf(regValTmp + valSize - 1, PATH_MAX - valSize,
            L",%d", nameId);
    if (numlen < 1)
    {
        RegCloseKey(tmp);
        return 0;
    }
    valSize += numlen;
    valSize *= sizeof *regValTmp;
    if (RegSetValueExW(tmp, 0, 0, REG_SZ,
                (const BYTE *)regValTmp, valSize) != ERROR_SUCCESS)
    {
        RegCloseKey(tmp);
        return 0;
    }
    RegCloseKey(tmp);
    return 1;
}

static void registerAutoType(HKEY classes, LPCWSTR ext, int nameId)
{
    HKEY tmp;
    if (RegOpenKeyExW(classes, ext, 0, KEY_QUERY_VALUE, &tmp)
            != ERROR_SUCCESS) return;

    WCHAR value[128];
    DWORD len = 128;
    DWORD valueType;
    if (RegQueryValueExW(tmp, 0, 0, &valueType, (LPBYTE)&value, &len)
            == ERROR_SUCCESS)
    {
        if (valueType != REG_SZ)
        {
            RegCloseKey(tmp);
            return;
        }
        value[len] = L'\0';
        WCHAR autoTypeKeyName[18];
        wcscpy(autoTypeKeyName, ext+1);
        wcscat(autoTypeKeyName, L"_auto_file");
        if (!wcscmp(value, autoTypeKeyName))
        {
            RegCloseKey(tmp);
            if (RegOpenKeyExW(classes, autoTypeKeyName, 0, KEY_WRITE, &tmp)
                    != ERROR_SUCCESS) return;
            HKEY ico;
            if (RegOpenKeyExW(tmp, L"DefaultIcon", 0, KEY_WRITE, &ico)
                    == ERROR_SUCCESS)
            {
                RegCloseKey(ico);
            }
            else
            {
                setFriendlyTypeNameAndIcon(tmp, nameId);
            }
        }
    }
    RegCloseKey(tmp);
}

static int registerType(HKEY classes, HKEY suppTypes, LPCWSTR ext,
        LPCWSTR name, LPCWSTR desc, LPCWSTR contentType, int nameId,
        int associate)
{
    HKEY tkey = 0;
    HKEY ekey = 0;
    HKEY tmp;
    int rc = 0;

    if (RegCreateKeyExW(suppTypes, ext, 0, 0, 0, KEY_WRITE,
                0, &tmp, 0) == ERROR_SUCCESS)
    {
        RegCloseKey(tmp);
    }
    else goto done;

    if (RegCreateKeyExW(classes, ext, 0, 0, 0, KEY_WRITE, 0, &ekey, 0)
            != ERROR_SUCCESS) goto done;

    if (RegCreateKeyExW(classes, name, 0, 0, 0, KEY_WRITE, 0, &tkey, 0)
            != ERROR_SUCCESS) goto done;

    if (RegCreateKeyExW(tkey, L"shell\\open\\command", 0, 0, 0, KEY_WRITE,
                0, &tmp, 0) == ERROR_SUCCESS)
    {
	if (RegSetValueExW(tmp, 0, 0, REG_SZ,
                    (const BYTE *)commanderRegCommand,
                    commanderRegCommandSize)
                != ERROR_SUCCESS)
	{
	    RegCloseKey(tmp);
	    goto done;
	}
	RegCloseKey(tmp);
    }
    else goto done;

    DWORD valSize = (wcslen(desc)+1) * sizeof *desc;
    if (RegSetValueExW(tkey, 0, 0, REG_SZ,
                (const BYTE *)desc, valSize) != ERROR_SUCCESS)
    {
	goto done;
    }
    valSize = (wcslen(contentType)+1) * sizeof *contentType;
    if (RegSetValueExW(tkey, L"Content Type", 0, REG_SZ,
		(const BYTE *)contentType, valSize) != ERROR_SUCCESS)
    {
	goto done;
    }

    if (!setFriendlyTypeNameAndIcon(tkey, nameId)) goto done;

    if (RegOpenKeyExW(tkey, L"shell", 0, KEY_WRITE, &tmp) == ERROR_SUCCESS)
    {
        if (RegSetValueExW(tmp, 0, 0, REG_SZ, (const BYTE *)L"open",
                    sizeof L"open") != ERROR_SUCCESS)
        {
            RegCloseKey(tmp);
            goto done;
        }
        RegCloseKey(tmp);
    }
    else goto done;

    if (RegCreateKeyExW(ekey, L"OpenWithProgids", 0, 0, 0,
                KEY_WRITE, 0, &tmp, 0) == ERROR_SUCCESS)
    {
        if (RegSetValueExW(tmp, name, 0, REG_NONE, 0, 0) != ERROR_SUCCESS)
        {
            RegCloseKey(tmp);
            goto done;
        }
        RegCloseKey(tmp);
    }
    else goto done;

    registerAutoType(classes, ext, nameId);
    if (!associate)
    {
        rc = 1;
        goto done;
    }

    valSize = (wcslen(name)+1) * sizeof *name;
    if (RegSetValueExW(ekey, 0, 0, REG_SZ, (const BYTE *)name, valSize)
            != ERROR_SUCCESS)
    {
	goto done;
    }
    rc = 1;

    size_t extLen = wcslen(ext);
    wcscpy(userChoiceName + FEXIDX, ext);
    userChoiceName[FEXIDX + extLen] = L'\\';
    wcscpy(userChoiceName + FEXIDX + extLen + 1, L"UserChoice");
    regUnprotect(userChoiceName);
    userChoiceName[FEXIDX + extLen] = L'\0';
    if (RegOpenKeyExW(HKEY_CURRENT_USER, userChoiceName, 0,
                DELETE|KEY_ENUMERATE_SUB_KEYS, &tmp) == ERROR_SUCCESS)
    {
        regTreeDel(tmp, L"UserChoice");
        RegCloseKey(tmp);
    }

done:
    if (ekey) RegCloseKey(ekey);
    if (tkey) RegCloseKey(tkey);
    return rc;
}

static void registerTypes(HWND w)
{
    int success = 0;
    HKEY regkey = 0;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\"
                "CurrentVersion\\App Paths\\v1541commander.exe", 0,
                0, 0, KEY_WRITE, 0, &regkey, 0) != ERROR_SUCCESS)
    {
        goto done;
    }
    DWORD valSize = (wcslen(commanderPath)+1) * sizeof *commanderPath;
    if (RegSetValueExW(regkey, 0, 0, REG_SZ,
                (const BYTE *)commanderPath, valSize) != ERROR_SUCCESS)
    {
        goto done;
    }
    RegCloseKey(regkey);
    regkey = 0;

    HKEY classes = 0;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Classes", 0,
		KEY_WRITE, &classes) == ERROR_SUCCESS)
    {
        if (RegCreateKeyExW(classes,
                    L"Applications\\v1541commander.exe\\shell\\open\\command",
                    0, 0, 0, KEY_WRITE, 0, &regkey, 0) == ERROR_SUCCESS)
        {
            if (RegSetValueExW(regkey, 0, 0, REG_SZ,
                        (const BYTE *)commanderRegCommand,
                        commanderRegCommandSize)
                    != ERROR_SUCCESS)
            {
                goto done;
	    }
	    RegCloseKey(regkey);
            regkey = 0;
        }
        else goto done;
        if (RegCreateKeyExW(classes,
                    L"Applications\\v1541commander.exe\\SupportedTypes", 0,
                    0, 0, KEY_WRITE, 0, &regkey, 0) == ERROR_SUCCESS)
        {
            if (!registerType(classes, regkey,
                        L".d64", L"V1541Commander.D64", L"D64 disk image",
                        L"application/vnd.cbm.d64-disk-image", 1, ftD64))
            {
                goto done;
            }
            if (!registerType(classes, regkey,
                        L".lnx", L"V1541Commander.LyNX", L"C64 LyNX archive",
                        L"application/x.willcorley.lynx-archive", 2, ftLynx))
            {
                goto done;
            }
            if (!registerType(classes, regkey,
                        L".prg", ftZipcode ? L"V1541Commander.Zipcode" :
			L"V1541Commander.PRG", ftZipcode ?
			L"C64 Zip-Code archive file" : L"Commodore program",
                        ftZipcode ? L"application/x.c64.zip-code" :
			L"application/vnd.cbm.program-file",
			ftZipcode ? 3 : 4, ftZipcode))
            {
                goto done;
            }
            success = 1;
        }
    }

done:
    if (regkey) RegCloseKey(regkey);
    if (classes) RegCloseKey(classes);
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
    if (success)
    {
	MessageBoxW(w, texts[TID_regsuccess_message],
		texts[TID_regsuccess_title], MB_OK|MB_ICONINFORMATION);
    }
    else
    {
	MessageBoxW(w, texts[TID_regfailure_message],
		texts[TID_regfailure_title], MB_OK|MB_ICONERROR);
    }
}

static void addFileTypeCheckbox(HWND w, int cid, LPCWSTR desc, int checked,
	HDC dc, int padding, int *ypos, int *fullwidth)
{
    SIZE size;
    GetTextExtentExPointW(dc, desc, wcslen(desc), 0, 0, 0, &size);
    size.cx += 2*padding;
    *fullwidth = size.cx > *fullwidth ? size.cx : *fullwidth;
    HWND cb = CreateWindowExW(0, L"Button", desc,
	    WS_CHILD|WS_VISIBLE|BS_CHECKBOX, padding, *ypos,
	    size.cx, size.cy, w, (HMENU)cid, instance, 0);
    *ypos += size.cy + padding;
    SendMessageW(cb, WM_SETFONT, (WPARAM)messageFont, 0);
    if (checked) CheckDlgButton(w, cid, BST_CHECKED);
}

static LRESULT CALLBACK wproc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
	{
	    HDC dc = GetDC(0);
	    SelectObject(dc, (HGDIOBJ) messageFont);
	    RECT textrect = {0, 0, 0, 0};
	    int padding = messageFontMetrics.tmAveCharWidth * 3 / 2;
	    int ypos = padding;

	    const WCHAR *text = texts[TID_select_types];
	    DrawTextExW(dc, (WCHAR *)text, -1, &textrect, DT_CALCRECT, 0);
	    int fullwidth = textrect.right;
	    HWND ctrl = CreateWindowExW(0, L"Static", text,
		    WS_CHILD|WS_VISIBLE, padding, ypos,
		    textrect.right, textrect.bottom, w, 0, instance, 0);
	    SendMessageW(ctrl, WM_SETFONT, (WPARAM)messageFont, 0);
	    ypos += textrect.bottom + padding;

	    addFileTypeCheckbox(w, CID_d64, texts[TID_d64], 1,
		    dc, padding, &ypos, &fullwidth);
	    addFileTypeCheckbox(w, CID_lynx, texts[TID_lynx], 1,
		    dc, padding, &ypos, &fullwidth);
	    addFileTypeCheckbox(w, CID_zipcode, texts[TID_zipcode], 0,
		    dc, padding, &ypos, &fullwidth);

	    text = texts[TID_prg_warn];
            memset(&textrect, 0, sizeof textrect);
	    DrawTextExW(dc, (WCHAR *)text, -1, &textrect, DT_CALCRECT, 0);
	    fullwidth = textrect.right > fullwidth
                ? textrect.right : fullwidth;
	    ctrl = CreateWindowExW(0, L"Static", text,
		    WS_CHILD|WS_VISIBLE, padding, ypos,
		    textrect.right, textrect.bottom, w, 0, instance, 0);
	    SendMessageW(ctrl, WM_SETFONT, (WPARAM)messageFont, 0);
	    ypos += textrect.bottom + padding;

	    ctrl = CreateWindowExW(0, L"Button", texts[TID_register],
		    WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,
		    fullwidth - 2*buttonWidth, ypos,
		    buttonWidth, buttonHeight,
		    w, (HMENU)CID_register, instance, 0);
	    SendMessageW(ctrl, WM_SETFONT, (WPARAM)messageFont, 0);
	    ctrl = CreateWindowExW(0, L"Button", texts[TID_cancel],
		    WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
		    padding + fullwidth - buttonWidth, ypos,
		    buttonWidth, buttonHeight,
		    w, (HMENU)CID_cancel, instance, 0);
	    SendMessageW(ctrl, WM_SETFONT, (WPARAM)messageFont, 0);
	    ypos += buttonHeight + padding;

	    ReleaseDC(0, dc);
	    RECT winRect = {0, 0, fullwidth + 2*padding, ypos};
	    AdjustWindowRect(&winRect, WS_CAPTION|WS_SYSMENU, 0);
	    SetWindowPos(w, HWND_TOP, 0, 0,
		    winRect.right - winRect.left,
		    winRect.bottom - winRect.top, SWP_NOMOVE);
	}
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_KEYDOWN:
        if (wp == VK_RETURN)
        {
            registerTypes(w);
            DestroyWindow(w);
        }
        else if (wp == VK_ESCAPE)
        {
            DestroyWindow(w);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wp))
        {
	case CID_d64:
	    ftD64 = !ftD64;
	    CheckDlgButton(w, CID_d64,
		    ftD64 ? BST_CHECKED : BST_UNCHECKED);
	    break;

	case CID_lynx:
	    ftLynx = !ftLynx;
	    CheckDlgButton(w, CID_lynx,
		    ftLynx ? BST_CHECKED : BST_UNCHECKED);
	    break;

	case CID_zipcode:
	    ftZipcode = !ftZipcode;
	    CheckDlgButton(w, CID_zipcode,
		    ftZipcode ? BST_CHECKED : BST_UNCHECKED);
	    break;

        case CID_register:
	    registerTypes(w);
            /* fall through */
        case CID_cancel:
	    DestroyWindow(w);
            break;
        }
        break;
    }
    return DefWindowProcW(w, msg, wp, lp);
}

int main(void)
{
    init();
    if (!getCommanderPath())
    {
	MessageBoxW(0, texts[TID_notfound_title], texts[TID_notfound_message],
		MB_OK|MB_ICONERROR);
	return 0;
    }

    WNDCLASSEXW wc;
    memset(&wc, 0, sizeof wc);
    wc.cbSize = sizeof wc;
    wc.hInstance = instance;
    wc.lpszClassName = WC_mainWindow;
    wc.lpfnWndProc = wproc;
    wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
    wc.hCursor = (HCURSOR) LoadImageW(0, MAKEINTRESOURCEW(OCR_NORMAL),
	    IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE|LR_SHARED);
    RegisterClassExW(&wc);

    mainWindow = CreateWindowExW(0, WC_mainWindow, texts[TID_title],
            WS_CAPTION|WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT,
	    320, 100, 0, 0, instance, 0);
    ShowWindow(mainWindow, SW_SHOWNORMAL);

    MSG msg;
    while (GetMessageW(&msg, 0, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    free(sid);
    return (int)msg.wParam;
}

