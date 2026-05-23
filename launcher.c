#include "shared.h"

HWND hMain, hGo1, hGo2, hBoth, hParams, hPress, hSlotCb, hTCmd;
HWND hWaitTitle, hWaitTab, hTypeDelay;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE: {
			CreateMainGUI(hwnd);
            return 0;
        }

        case WM_MOVE: {
            WINDOWPLACEMENT wp = {sizeof(wp)};
            if(GetWindowPlacement(hwnd, &wp) && wp.showCmd == SW_SHOWNORMAL) {
                SetInt("WinX", wp.rcNormalPosition.left); SetInt("WinY", wp.rcNormalPosition.top);
            }
            return 0;
        }
		case WM_ERASEBKGND:
			if (gpBitmap) {
				DrawBackgroundImage(hwnd, (HDC)wp);
				return 1; // Load image
			}
			break; // If no image, let Windows draw the default Color

		case WM_CTLCOLOREDIT:   // This handles the ComboBox text area
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORBTN: {
			HDC hdc = (HDC)wp;
			SetTextColor(hdc, txtColor);
			SetBkMode(hdc, TRANSPARENT);
			return (LRESULT)hNullBrush;
		}
        case WM_CHAR: {
            char typedChar = (char)wp;

            // 1. Check Primary and Alt Actions
            if (typedChar == keyGo1) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_GO1, STN_CLICKED), 0);
                return 0;
            }
            if (typedChar == keyGo2) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_GO2, STN_CLICKED), 0);
                return 0;
            }
            if (typedChar == keyBoth) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_BOTH, STN_CLICKED), 0);
                return 0;
            }

            // 2. Check Tool Button Bindings
            for (int i = 0; i < 8; i++) {
                if (typedChar == keyTools[i]) {
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BASE_TOOL + i, STN_CLICKED), 0);
                    return 0;
                }
            }
            break;
        }
		case WM_COMMAND: {
			int id = LOWORD(wp);
			int code = HIWORD(wp);
			if (code != STN_CLICKED) break;

			if (id == ID_CB_MAIN) ShowAccountMenu(hwnd, hCbMain, "LastMain");
			if (id == ID_CB_ALT)  ShowAccountMenu(hwnd, hCbAlt, "LastAlt");
			if (id == ID_CB_MAIN || id == ID_BTN_ARROW_MAIN) 
				ShowAccountMenu(hwnd, hCbMain, "LastMain");
			
			if (id == ID_CB_ALT || id == ID_BTN_ARROW_ALT) 
				ShowAccountMenu(hwnd, hCbAlt, "LastAlt");
			
            // Static controls use STN_CLICKED instead of BN_CLICKED
            if (id == ID_BTN_GO1) LaunchGame(hCbMain, "Primary", "PID1");
            if (id == ID_BTN_GO2) LaunchGame(hCbAlt, "Alt", "PID2");
            if (id == ID_BTN_BOTH) { LaunchGame(hCbMain, "Primary", "PID1"); Sleep(waitTitle); LaunchGame(hCbAlt, "Alt", "PID2"); }
            
			if (id >= ID_BASE_TOOL && id < ID_BASE_TOOL + 8) {
				char cmd[MAX_PATH], k[32]; 
				int toolIndex = id - ID_BASE_TOOL;
				sprintf(k, "T%d_Cmd", toolIndex);
				GetPrivateProfileStringA("Tools", k, "", cmd, MAX_PATH, INI);
				
				if (strlen(cmd) == 0) {
					char f[MAX_PATH] = {0};
					char* name = SimpleFile(hwnd, "Executables\0*.exe\0All Files\0*.*\0", f);
					
					if (name && strlen(f) > 0) {
						// 1. Save executable path to INI
						WritePrivateProfileStringA("Tools", k, f, INI);
						
						// 2. Format and save clean name to INI
						char kn[32];
						sprintf(kn, "T%d_Name", toolIndex);
						char cleanName[64]; strcpy(cleanName, name);
						char* dot = strrchr(cleanName, '.'); if(dot) *dot = '\0';
						WritePrivateProfileStringA("Tools", kn, cleanName, INI);
						
						// 3. Update the launcher button text string
						HWND hBtn = GetDlgItem(hwnd, id);
						SetWindowTextA(hBtn, cleanName);
						
						RECT rcBtn;
						GetWindowRect(hBtn, &rcBtn);
						MapWindowPoints(NULL, hwnd, (LPPOINT)&rcBtn, 2);
						InvalidateRect(hwnd, &rcBtn, TRUE);
						UpdateWindow(hwnd);
					}
					break; // FIX: Stop execution here so it DOES NOT run immediately
				}

				// Only runs if the tool already had a path mapped to it before clicking
				if (strlen(cmd) > 0) {
					char workDir[MAX_PATH];
					strcpy(workDir, cmd);
					char* lastSlash = strrchr(workDir, '\\');
					if (lastSlash) {
						*lastSlash = '\0'; 
						ShellExecuteA(NULL, "open", cmd, NULL, workDir, SW_SHOWNORMAL);
					} else {
						ShellExecuteA(NULL, "open", cmd, NULL, NULL, SW_SHOWNORMAL);
					}
				}
			}

            break;
        }

        case WM_DESTROY: PostQuitMessage(0); break;
		
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hI, HINSTANCE hP, LPSTR lp, int nS) {
    //Setup INI path
    GetModuleFileNameA(NULL, INI, MAX_PATH); 
    char* s = strrchr(INI, '\\'); 
    if(s) strcpy(s + 1, "settings.ini");
    // Initialize GDI+
    InitImageSystem();

    //Load Configuration from ini_manager.c
    InitDefaults();
    LoadAllSettings();

    //Setup Paths and Background
    GetPrivateProfileStringA("Settings", "BgImage", "", BG_PATH, MAX_PATH, INI);
    if (strlen(BG_PATH) > 0) LoadBackgroundImage(BG_PATH);

    GetPrivateProfileStringA("Settings", "GamePath", "", GAME_EXE, MAX_PATH, INI);
    if (!strlen(GAME_EXE)) {
			if (SimpleFile(NULL, "Anarchy Online\0*.exe\0", GAME_EXE)) {
			WritePrivateProfileStringA("Settings", "GamePath", GAME_EXE, INI);
		}
    }
    strcpy(GAME_DIR, GAME_EXE); 
    char* dE = strrchr(GAME_DIR, '\\'); 
    if (dE) *dE = '\0';

    //Handle Window Position
    int x = GetInt("WinX", -32000);
    int y = GetInt("WinY", -32000);

    //Register and Create Window
    WNDCLASSA wc = {0}; 
    wc.lpfnWndProc = WindowProc; 
    wc.hInstance = hI; 
    wc.lpszClassName = "AOL";
    wc.hbrBackground = CreateSolidBrush(formColor);
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA("AOL", "PRK Multitool by Ar1z", 
                             WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 
                             (x == -32000) ? CW_USEDEFAULT : x, 
                             (y == -32000) ? CW_USEDEFAULT : y, 
                             340, 325, 0, 0, hI, 0);

    // Wine/Multi-monitor fix: force position if saved
    if (x != -32000) SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    ShowWindow(hwnd, nS);
    
    //Message Loop
    MSG m; 
    while (GetMessageA(&m, 0, 0, 0)) { 
        TranslateMessage(&m); 
        DispatchMessageA(&m);
		
    }

    //Cleanup before exit
    ShutdownImageSystem();
    return (int)m.wParam;
}

