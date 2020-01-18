#include <wchar.h>

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>

#define WC_mainWindow L"V1541CommanderSetup"
#define CID_register 0x101
#define CID_d64 0x102
#define CID_lynx 0x103
#define CID_zipcode 0x104

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

static WCHAR opencmd[MAX_PATH];

static void init(void)
{
    INITCOMMONCONTROLSEX icx;
    icx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icx.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icx);
    ncm.cbSize = sizeof(ncm);
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
}

static int checkForCommander(void)
{
    HMODULE hModule = GetModuleHandleW(0);
    GetModuleFileNameW(hModule, opencmd, MAX_PATH);
    WCHAR *pos = wcsrchr(opencmd, L'\\');
    if (!pos) return 0;
    wcscpy(pos+1, L"v1541commander.exe");
    int rc = PathFileExistsW(opencmd);
    if (!rc) return rc;
    wcscpy(pos+sizeof "v1541commander.exe", L" \"%1\"");
    return rc;
}

static int registerType(HKEY classes, LPCWSTR ext, LPCWSTR name,
	LPCWSTR desc, LPCWSTR contentType)
{
    HKEY tkey;
    HKEY ekey;
    HKEY tmp;
    int rc = 0;

    if (RegCreateKeyW(classes, name, &tkey) != ERROR_SUCCESS) return 0;
    if (RegCreateKeyW(classes, ext, &ekey) != ERROR_SUCCESS) goto openexterror;

    if (RegCreateKeyW(tkey, L"shell\\open\\command", &tmp) == ERROR_SUCCESS)
    {
	if (RegSetValueW(tmp, 0, REG_SZ, opencmd, 0) != ERROR_SUCCESS)
	{
	    RegCloseKey(tmp);
	    goto error;
	}
	RegCloseKey(tmp);
    }
    else goto error;

    if (RegSetValueW(tkey, 0, REG_SZ, desc, 0) != ERROR_SUCCESS)
    {
	goto error;
    }
    DWORD ctSize = (wcslen(contentType)+1) * sizeof (WCHAR);
    if (RegSetValueExW(tkey, L"Content Type", 0, REG_SZ,
		(const BYTE *)contentType, ctSize) != ERROR_SUCCESS)
    {
	goto error;
    }
    if (RegSetValueW(tkey, L"shell", REG_SZ, L"open", 0) != ERROR_SUCCESS)
    {
	goto error;
    }
    if (RegSetValueW(ekey, 0, REG_SZ, name, 0) != ERROR_SUCCESS)
    {
	goto error;
    }
    rc = 1;

error:
    RegCloseKey(ekey);
openexterror:
    RegCloseKey(tkey);
    return rc;
}

static void registerTypes(HWND w)
{
    int success = 0;
    HKEY classes;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Classes", 0,
		KEY_WRITE, &classes) == ERROR_SUCCESS)
    {
	if (ftD64 && !registerType(classes,
			L".d64", L"D64", L"D64 disk image",
			L"application/vnd.cbm.d64-disk-image"))
	{
	    goto done;
	}
	if (ftLynx && !registerType(classes,
			L".lnx", L"LyNX", L"C64 LyNX archive",
			L"application/x.willcorley.lynx-archive"))
	{
	    goto done;
	}
	if (ftZipcode && !registerType(classes,
			L".prg", L"Zipcode", L"C64 Zip-Code archive file",
			L"application/x.c64.zip-code"))
	{
	    goto done;
	}
	RegCloseKey(classes);
	success = 1;
    }

done:
    if (success)
    {
	MessageBoxW(w, L"The selected file types were associated\n"
		L"to V1541Commander successfully.",
		L"File types registered",
		MB_OK|MB_ICONINFORMATION);
    }
    else
    {
	MessageBoxW(w, L"There was an unexpected error registering\n"
		L"the selected file types to V1541Commander.",
		L"File type registration failed",
		MB_OK|MB_ICONERROR);
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
	    SIZE size;
	    int padding = messageFontMetrics.tmAveCharWidth * 3 / 2;
	    int ypos = padding;
	    WCHAR *text =
		L"Select the file types to register V1541Commander for:";
	    GetTextExtentExPointW(dc, text, wcslen(text), 0, 0, 0, &size);
	    int fullwidth = size.cx;
	    HWND ctrl = CreateWindowExW(0, L"Static", text,
		    WS_CHILD|WS_VISIBLE, padding, ypos,
		    size.cx, size.cy, w, 0, instance, 0);
	    SendMessageW(ctrl, WM_SETFONT, (WPARAM)messageFont, 0);
	    ypos += size.cy + padding;
	    text = L"D64 disk images (.d64)";
	    GetTextExtentExPointW(dc, text, wcslen(text), 0, 0, 0, &size);
	    size.cx += 2*padding;
	    fullwidth = size.cx > fullwidth ? size.cx : fullwidth;
	    ctrl = CreateWindowExW(0, L"Button", text,
		    WS_CHILD|WS_VISIBLE|BS_CHECKBOX, padding, ypos,
		    size.cx, size.cy, w, (HMENU)CID_d64, instance, 0);
	    ypos += size.cy + padding;
	    SendMessageW(ctrl, WM_SETFONT, (WPARAM)messageFont, 0);
	    CheckDlgButton(w, CID_d64, BST_CHECKED);
	    text = L"LyNX archives (.lnx)";
	    GetTextExtentExPointW(dc, text, wcslen(text), 0, 0, 0, &size);
	    size.cx += 2*padding;
	    fullwidth = size.cx > fullwidth ? size.cx : fullwidth;
	    ctrl = CreateWindowExW(0, L"Button", text,
		    WS_CHILD|WS_VISIBLE|BS_CHECKBOX, padding, ypos,
		    size.cx, size.cy, w, (HMENU)CID_lynx, instance, 0);
	    ypos += size.cy + padding;
	    SendMessageW(ctrl, WM_SETFONT, (WPARAM)messageFont, 0);
	    CheckDlgButton(w, CID_lynx, BST_CHECKED);
	    text = L"Zipcode files (.prg)";
	    GetTextExtentExPointW(dc, text, wcslen(text), 0, 0, 0, &size);
	    size.cx += 2*padding;
	    fullwidth = size.cx > fullwidth ? size.cx : fullwidth;
	    ctrl = CreateWindowExW(0, L"Button", text,
		    WS_CHILD|WS_VISIBLE|BS_CHECKBOX, padding, ypos,
		    size.cx, size.cy, w, (HMENU)CID_zipcode, instance, 0);
	    ypos += size.cy + padding;
	    SendMessageW(ctrl, WM_SETFONT, (WPARAM)messageFont, 0);
	    ctrl = CreateWindowExW(0, L"Button", L"Register",
		    WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
		    padding + fullwidth - buttonWidth, ypos,
		    buttonWidth, buttonHeight,
		    w, (HMENU)CID_register, instance, 0);
	    ypos += buttonHeight + padding;
	    SendMessageW(ctrl, WM_SETFONT, (WPARAM)messageFont, 0);
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
    if (!checkForCommander())
    {
	MessageBoxW(0, L"Please run this from the same directory\n"
		L"where v1541commander.exe is located.",
		L"v1541commander.exe not found",
		MB_OK|MB_ICONERROR);
	return 0;
    }

    WNDCLASSEXW wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.hInstance = instance;
    wc.lpszClassName = WC_mainWindow;
    wc.lpfnWndProc = wproc;
    wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
    wc.hCursor = LoadCursorA(0, IDC_ARROW);
    RegisterClassExW(&wc);

    mainWindow = CreateWindowExW(0, WC_mainWindow, L"V1541Commander Setup",
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

