# PRK Multitool by Ar1z

A lightweight, high-performance, and modular multi-account launcher and automated login tool designed for **Project Rubi-Ka**. Built entirely in native C and Win32 API, it features an integrated account credential manager, customizable styling, and configurable global hotkeys.

## 🛠️ Build & Compilation

The binaries are built seamlessly using **GCC / MinGW-w64** (such as the `w64devkit` environment). Follow these quick steps to compile the application from source:

### 1. Compile Resources
Compile your resource file (app icon and explorer metadata blocks) into a linker object format:
```bash
windres resource.rc -o resource.o
```

### 2. Compile Executables
Compile your modules into their standalone targeted binaries using the following commands:

```bash
# Compile Main PRKMultitool Module
gcc launcher.c GUI.c logic.c ini_manager.c game_launcher.c image_manager.c resource.o -o PRKMultitool.exe -mwindows -luser32 -lkernel32 -lcomdlg32 -lgdi32 -lgdiplus -lole32

# Compile Settings Manager Module
gcc settings.c GUI.c ini_manager.c logic.c resource.o -o settings.exe -mwindows -luser32 -lkernel32 -lcomdlg32

# Compile Accounts Manager Module
gcc accounts.c GUI.c ini_manager.c logic.c resource.o -o accounts.exe -mwindows -luser32 -lkernel32
```
*Make sure all three compiled `.exe` files sit in the same root folder before running.*

## 📖 How to Use

### 1. Initial Setup
1. Run `PRKMultitool.exe`. On your very first startup, it will prompt you to browse and select your main `Project Rubi-Ka` executable (e.g., `AnarchyOnline.exe`).
2. The Program will automatically generate a clean `settings.ini` profile file in its directory.

### 2. Managing Accounts
1. Launch `accounts.exe` (or access the configuration panel) to open the **Accounts Manager**.
2. Type an account username and password, then click **Save**. Your credentials are securely hidden using a local XOR-obfuscation (`Xcrypt`) layout script.
3. You can manage multiple profiles here. Saved accounts are immediately fed into the main Window dropdown menus.

### 3. Customizing & Adding Tools
1. Open `settings.exe` to manage theme layouts, button labels, execution delays, and external tools.
2. Under **Tools Config**, manage the utilities, change the display name and click **Browse** to point to any utility utility program (like an inventory manager, map tool, or chat overlay) or clear it's settings.
3. Customize window colors, button styles, font families, or choose a custom `.png`/`.jpg` file for a **Background Image**.
4. Click **SAVE SETTINGS** to automatically refresh and restart the main window.

### 4. Running Clients & Keyboard Commands
*   Click **Primary** or **Alt** dropdown names to cycle through your saved accounts, and click their respective **Go** buttons to fire up the client and let the macros log you in automatically.
*   Click their respective **Go** buttons after the client has loaded to select it's window.
*   Click the **Launch Both** button to cycle a sequential automation script that logs in both clients back-to-back.
*   **Keyboard Commands:** You can press keys directly on your keyboard to instantly trigger layout actions! The defaults are:
    *   `1` — Fire Go 1
    *   `2` — Fire Go 2
    *   `0` — Launch Both Macro
    *   `a` — Run Tool Slot 0
    *   `S` — Run Tool Slot 1
    *   `3` through `8` — Run Tool Slots 2 through 7

## ⌨️ Advanced INI Keybindings

Power users can change the custom global hotkey layout directly by updating the `settings.ini` configuration (read INI_Editing.txt):

```ini
[Settings]
Go1Label=Primary Account
Go2Label=Alt Account
BothLabel=Launch Macro
KeyGo1=1
KeyGo2=2
KeyBoth=0

[Tools]
KeyTool0=a
KeyTool1=S
KeyTool2=3
...
```

## 📝 License

Developed by **Ar1z**. This project is provided for personal automation optimization and standalone window layout engineering.
