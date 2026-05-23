#include "shared.h"

HWND hMain, hGo1, hGo2, hBoth, hParams, hPress, hSlotCb, hTCmd;
HWND hWaitTitle, hWaitTab, hTypeDelay;
COLORREF custClrs[16];

LRESULT CALLBACK SetProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

    if (msg == WM_CREATE) {
        CreateSettingsGUI(hwnd);
        return TRUE;
    }
	if (msg == WM_CTLCOLORSTATIC) {
		HDC hdc = (HDC)wp;
		static HBRUSH hStaticBr = NULL;
		
		// If the color changed or brush doesn't exist, recreate it
		if (hStaticBr) DeleteObject(hStaticBr);
		hStaticBr = CreateSolidBrush(formColor);
		
		SetTextColor(hdc, txtColor);
		SetBkColor(hdc, formColor);
		return (LRESULT)hStaticBr;
	}

    if (msg == WM_DRAWITEM) {
        DrawCustomBtn((LPDRAWITEMSTRUCT)lp);
        return TRUE;
    }

    if (msg == WM_COMMAND) {
        int id = LOWORD(wp);
		static int lastSlot = 0; 
        // 1. Auto-save previous slot text on selection shift
        if (id == ID_SET_CB_SLOT && HIWORD(wp) == CBN_SELCHANGE) {
            DirectSaveTool(lastSlot); // Save what they just typed into the old slot
            
            lastSlot = SendMessage(hSlotCb, CB_GETCURSEL, 0, 0); // Lock new index
            LoadToolData(); // Load text for the new slot
        }
        
        // 2. Clear Tool Fields
        if (id == ID_SET_BTN_CLEAR) { 
            SetWindowTextA(hTName, "Empty");
            SetWindowTextA(hTCmd, "");
        }

        // 3. Browse File Path Selection
        if (id == ID_SET_BTN_BROWSE) {
            char f[MAX_PATH] = {0};
            char* n = SimpleFile(hwnd, "Executables\0*.exe\0All Files\0*.*\0", f);
            if (n) {
                SetWindowTextA(hTCmd, f);
                char cl[64]; strcpy(cl, n); char* ex = strrchr(cl, '.'); if(ex) *ex = '\0';
                SetWindowTextA(hTName, cl);
            }
        }


        if (id == ID_SET_BTN_FONT) {
            CHOOSEFONTA cf = {sizeof(cf), hwnd}; LOGFONTA lf = {0};
            strcpy(lf.lfFaceName, fontName); lf.lfHeight = -fontSize;
            cf.lpLogFont = &lf; cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
            cf.rgbColors = txtColor;
            if (ChooseFontA(&cf)) {
                strcpy(fontName, lf.lfFaceName); fontSize = -lf.lfHeight; txtColor = cf.rgbColors;
                WritePrivateProfileStringA("Settings", "FontName", fontName, INI);
                SetInt("FontSize", fontSize);
                SetInt("TxtColor", txtColor);
                InvalidateRect(hwnd, NULL, TRUE); // Refresh UI
            }
        }
		if (id == ID_SET_BTN_FCLR || id == ID_SET_BTN_BCLR) {
			CHOOSECOLORA cc = {sizeof(cc), hwnd}; cc.lpCustColors = custClrs; cc.Flags = CC_RGBINIT | CC_FULLOPEN;
			if (ChooseColorA(&cc)) {
				if (id == ID_SET_BTN_FCLR) { 
					formColor = cc.rgbResult; 
					SetInt("FormColor", formColor); 
					
					// Update the window immediately
					HBRUSH newBr = CreateSolidBrush(formColor);
					HBRUSH oldBr = (HBRUSH)SetClassLongPtrA(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)newBr);
					if (oldBr) DeleteObject(oldBr); // Free the old brush
				}
				if (id == ID_SET_BTN_BCLR) { 
					btnColor = cc.rgbResult; 
					SetInt("BtnColor", btnColor); 
				}
				// Force a repaint
				InvalidateRect(hwnd, NULL, TRUE);
				UpdateWindow(hwnd);
			}
		}
	    if (id == ID_SET_BTN_BGIMG) {
			char f[MAX_PATH] = {0};
			if (SimpleFile(hwnd, "Images\0*.jpg;*.png\0", f)) {
				WritePrivateProfileStringA("Settings", "BgImage", f, INI);
			}
        }
        if (id == ID_SET_BTN_SAVE) {
			int currentSlot = SendMessage(hSlotCb, CB_GETCURSEL, 0, 0);
            DirectSaveTool(currentSlot); // Save the currently active screen edits
            // 1. Commit all fields to settings.ini
            SaveAllToINI();
            LoadToolList();
            
            // 2. Find and gracefully close the running main launcher window
            HWND hOldLauncher = FindWindowA("AOL", NULL);
            if (hOldLauncher) {
                PostMessageA(hOldLauncher, WM_CLOSE, 0, 0);
            }

            // 3. Launch a fresh instance of the main launcher program
            // This assumes the launcher executable is named "launcher.exe" and sits in the same directory
            char launcherPath[MAX_PATH];
            GetModuleFileNameA(NULL, launcherPath, MAX_PATH);
            char* lastSlash = strrchr(launcherPath, '\\');
            if (lastSlash) {
                strcpy(lastSlash + 1, "prkmultitool.exe"); // Change settings.exe to launcher.exe in path
                ShellExecuteA(NULL, "open", launcherPath, NULL, NULL, SW_SHOWNORMAL);
            }
        }
    }
    if (msg == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProcA(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hI, HINSTANCE hP, LPSTR lp, int nS) {
    GetModuleFileNameA(NULL, INI, MAX_PATH); char* s = strrchr(INI, '\\'); if(s) strcpy(s+1, "settings.ini");
    LoadAllSettings();
    
    // Position window based on launcher coordinates
    int wx = GetInt("WinX", CW_USEDEFAULT);
    int wy = GetInt("WinY", CW_USEDEFAULT);

    WNDCLASSA wc = {0}; wc.lpfnWndProc = SetProc; wc.hInstance = hI; wc.lpszClassName = "Sets"; 
    wc.hbrBackground = CreateSolidBrush(formColor); RegisterClassA(&wc);
    
    hMain = CreateWindowA("Sets", "Settings Manager", WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_VISIBLE, wx, wy, 320, 420, 0, 0, hI, 0);
  
    MSG m; while (GetMessageA(&m, 0, 0, 0)) { TranslateMessage(&m); DispatchMessageA(&m); }
    return 0;
}
