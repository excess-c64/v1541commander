#include <wchar.h>

#define OEMRESOURCE
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>

#define WC_mainWindow L"V1541CommanderUninstall"
#define CID_uninstall 0x101
#define CID_cancel 0x102

static HINSTANCE instance;
static HWND mainWindow;
static NONCLIENTMETRICSW ncm;
static HFONT messageFont;
static TEXTMETRICW messageFontMetrics;
static int buttonWidth;
static int buttonHeight;

static const WCHAR *locales[] = {
    L"C",
    L"de",
};

enum textid {
    TID_title,
    TID_success_title,
    TID_success_message,
    TID_failure_title,
    TID_failure_message,
    TID_message,
    TID_uninstall,
    TID_cancel,

    TID_N_texts
};

static const WCHAR *locale_texts[][TID_N_texts] = {
    {
	L"V1541Commander Uninstall",
	L"Uninstall completed",
	L"All filetype associations for V1541Commander\n"
	    L"were removed successfully.",
	L"Uninstall failed",
	L"There was an unexpected error removing\n"
	    L"file type associations for V1541Commander.",
	L"This tool unregisters V1541Commander with windows\n"
            L"and removes all file type associations created by\n"
            L"setup.exe.",
	L"Uninstall",
        L"Cancel",
    },
    {
	L"V1541Commander entfernen",
	L"Deinstallation erfolgreich",
	L"Alle Zuordnungen von Dateitypen für V1541Commander\n"
	    L"wurden erfolgreich entfernt.",
	L"Deinstallation fehlgeschlagen",
	L"Beim Entfernen von Dateityp-Zuordnungen für V1541Commander\n"
	    L"ist ein unerwarteter Fehler aufgetreten.",
	L"Dieses Programm deregistriert V1541Commander in Windows\n"
            L"und entfernt alle Dateityp-Zuordnungen, die von setup.exe\n"
            L"erstellt wurden.\n",
	L"Entfernen",
        L"Abbrechen",
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

static int unregisterType(HKEY classes, LPCWSTR ext, LPCWSTR name)
{
    int success = 1;

    HKEY ekey;

    if (RegOpenKeyExW(classes, ext, 0, KEY_WRITE|KEY_QUERY_VALUE, &ekey)
	    == ERROR_SUCCESS)
    {
	WCHAR value[128];
	DWORD len = 128;
	DWORD valueType;
	if (RegQueryValueExW(ekey, 0, 0, &valueType, (LPBYTE)&value, &len)
		== ERROR_SUCCESS)
	{
	    if (valueType != REG_SZ)
	    {
		success = 0;
	    }
	    else
	    {
		value[len] = L'\0';
		if (!wcscmp(value, name))
		{
		    if (RegDeleteValueW(ekey, 0) != ERROR_SUCCESS)
		    {
			success = 0;
		    }
		}
	    }
	}

        HKEY owkey;
        if (RegOpenKeyExW(ekey, L"OpenWithProgids", 0, KEY_WRITE, &owkey)
                == ERROR_SUCCESS)
        {
            RegDeleteValueW(owkey, name);
            RegCloseKey(owkey);
        }
        RegCloseKey(ekey);
    }

    regTreeDel(classes, name);

    return success;
}

static void unregister(HWND w)
{
    int success = 1;
    
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\"
		"CurrentVersion\\App Paths", 0,
		DELETE|KEY_ENUMERATE_SUB_KEYS, &key) == ERROR_SUCCESS)
    {
	regTreeDel(key, L"v1541commander.exe");
	RegCloseKey(key);
    }
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Classes\\Applications",
		0, DELETE|KEY_ENUMERATE_SUB_KEYS, &key) == ERROR_SUCCESS)
    {
	regTreeDel(key, L"v1541commander.exe");
	RegCloseKey(key);
    }
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Classes", 0,
		KEY_WRITE|KEY_ENUMERATE_SUB_KEYS, &key) == ERROR_SUCCESS)
    {
        if (!unregisterType(key, L".prg", L"V1541Commander.Zipcode"))
        {
            success = 0;
        }
        if (!unregisterType(key, L".prg", L"V1541Commander.PRG"))
        {
            success = 0;
        }
        if (!unregisterType(key, L".lnx", L"V1541Commander.LyNX"))
        {
            success = 0;
        }
        if (!unregisterType(key, L".d64", L"V1541Commander.D64"))
        {
            success = 0;
        }
        RegCloseKey(key);
    }
    else success = 0;

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
    if (success)
    {
	MessageBoxW(w, texts[TID_success_message],
		texts[TID_success_title], MB_OK|MB_ICONINFORMATION);
    }
    else
    {
	MessageBoxW(w, texts[TID_failure_message],
		texts[TID_failure_title], MB_OK|MB_ICONERROR);
    }
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

	    const WCHAR *text = texts[TID_message];
	    DrawTextExW(dc, (WCHAR *)text, -1, &textrect, DT_CALCRECT, 0);
	    int fullwidth = textrect.right;
	    HWND ctrl = CreateWindowExW(0, L"Static", text,
		    WS_CHILD|WS_VISIBLE, padding, ypos,
		    textrect.right, textrect.bottom, w, 0, instance, 0);
	    SendMessageW(ctrl, WM_SETFONT, (WPARAM)messageFont, 0);
	    ypos += textrect.bottom + padding;

	    ctrl = CreateWindowExW(0, L"Button", texts[TID_uninstall],
		    WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,
		    fullwidth - 2*buttonWidth, ypos,
		    buttonWidth, buttonHeight,
		    w, (HMENU)CID_uninstall, instance, 0);
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
            unregister(w);
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
        case CID_uninstall:
	    unregister(w);
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
    return (int)msg.wParam;
}

