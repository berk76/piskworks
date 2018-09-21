/*
*       main_w32.c
*       
*       This file is part of Piskworks game.
*       https://github.com/berk76/piskworks
*       
*       Piskworks is free software; you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation; either version 3 of the License, or
*       (at your option) any later version. <http://www.gnu.org/licenses/>
*       
*       Written by Jaroslav Beran <jaroslav.beran@gmail.com>, on 24.2.2016        
*/

#include <windows.h>
#include <Windowsx.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include "pisk_lib.h"
#include "resource.h"

#define _MainClassName TEXT("WinAPIMainClass")
#define TOOLBAR_HEIGHT 30
#define MARGIN_X 15
#define MARGIN_Y (15 + TOOLBAR_HEIGHT) 
#define GRID_FIELD_SIZE 20
#define TEXT_BUFF 256
#define EXTENSION ".sav"

TCHAR _AppName[TEXT_BUFF];
TCHAR _AboutText[TEXT_BUFF];
TCHAR _StatusText[TEXT_BUFF];
HINSTANCE g_hInstance;
HWND g_hwndMain;
HWND g_hwndStatusBar;
HWND g_hwndToolBar;
HACCEL g_haccel;
MSG msg;
PISKWORKS_T pisk;
HPEN hPen;
HPEN hPenGrid;
HBRUSH hBrush;
HBRUSH hBrushHighlited;
HBRUSH hBrushDisabled;
int g_ComputerPlaysWithO = 1;


static BOOL InitApp();
static BOOL DeleteApp();
static LRESULT CALLBACK WindowProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SettingsDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void OnWM_MOUSEDOWN(WPARAM wParam, LPARAM lParam);
static void onPaint();
static void draw_mesh(HDC hdc, PISKWORKS_T *game);
static void draw_stone(HDC hdc, int x, int y, STONE stone, int last);
static void update_statusbar();
static void new_game(int reset_counter);
static void load_game_w32();
static void save_game_w32();


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShow) {
        g_hInstance = hInstance;

        InitCommonControls();        
        if (!InitApp())
                return FALSE;
                
        while (GetMessage(&msg, NULL, 0, 0)) {
                if (!TranslateAccelerator(msg.hwnd, g_haccel, &msg)) {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                }
        }
        
        DeleteApp();
        return msg.wParam;
}

BOOL InitApp() {
        _stprintf(_AppName, "Piskworks %s", VERSION);
        _stprintf(_AboutText, "Piskworks %s\nhttps://github.com/berk76/piskworks\n(c) 2016 Jaroslav Beran", VERSION);

        WNDCLASSEX wc;
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDR_MAIN));
        wc.hIconSm = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDR_MAIN));
        wc.hInstance = g_hInstance;
        wc.lpfnWndProc = WindowProcMain;
        wc.lpszClassName = _MainClassName;
        wc.lpszMenuName = NULL;
        wc.style = CS_HREDRAW | CS_VREDRAW;
        
        if (!RegisterClassEx(&wc)) {
                MessageBox(NULL, TEXT("This program requires Windows NT."), _AppName, MB_ICONERROR);
                return FALSE;
        }
        
        g_hwndMain = CreateWindowEx(0,     
                _MainClassName, 
                _AppName, 
                WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
                CW_USEDEFAULT, CW_USEDEFAULT, 
                600, 600, 
                (HWND)NULL, 
                LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MAINMENU)), 
                g_hInstance, 
                NULL); 
                
        if (g_hwndMain == NULL)
                return FALSE;
                
                
        g_hwndStatusBar = CreateWindowEx(0,
                STATUSCLASSNAME,
                TEXT(" "),
                WS_CHILD | WS_VISIBLE,
                0, 0, 0, 0,
                g_hwndMain,
                (HMENU)NULL,
                g_hInstance,
                NULL);
        if (g_hwndStatusBar == NULL)
                return FALSE;

        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(g_hwndStatusBar, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

        g_hwndToolBar = CreateWindowEx(0,
                TOOLBARCLASSNAME,
                NULL, 
                WS_CHILD | WS_VISIBLE,
                0, 0, 0, 0,
                g_hwndMain,
                (HMENU)NULL, 
                GetModuleHandle(NULL), 
                NULL);
        if (g_hwndToolBar == NULL)
                return FALSE;

        // Send the TB_BUTTONSTRUCTSIZE message, which is required for
        // backward compatibility.
        SendMessage(g_hwndToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

        // Four buttons on toolbar
        TBBUTTON tbb[4];
        TBADDBITMAP tbab;

        tbab.hInst = HINST_COMMCTRL;
        tbab.nID = IDB_STD_SMALL_COLOR;
        SendMessage(g_hwndToolBar, TB_ADDBITMAP, 0, (LPARAM)&tbab);

        ZeroMemory(tbb, sizeof(tbb));

        tbb[0].iBitmap = STD_FILENEW;
        tbb[0].fsState = TBSTATE_ENABLED;
        tbb[0].fsStyle = TBSTYLE_BUTTON;
        tbb[0].idCommand = ID_G_NEW;

        tbb[1].iBitmap = STD_FILEOPEN;
        tbb[1].fsState = TBSTATE_ENABLED;
        tbb[1].fsStyle = TBSTYLE_BUTTON;
        tbb[1].idCommand = ID_G_LOAD;

        tbb[2].iBitmap = STD_FILESAVE;
        tbb[2].fsState = TBSTATE_ENABLED;
        tbb[2].fsStyle = TBSTYLE_BUTTON;
        tbb[2].idCommand = ID_G_SAVE;
        
        tbb[3].iBitmap = STD_PROPERTIES;
        tbb[3].fsState = TBSTATE_ENABLED;
        tbb[3].fsStyle = TBSTYLE_BUTTON;
        tbb[3].idCommand = ID_G_SETTINGS;

        SendMessage(g_hwndToolBar, TB_ADDBUTTONS, sizeof(tbb)/sizeof(TBBUTTON), (LPARAM)&tbb);

        hPen = CreatePen(PS_SOLID | PS_INSIDEFRAME, 2, 0x000000);
        hPenGrid = CreatePen(PS_SOLID | PS_INSIDEFRAME, 2, 0xEEEEAF); 
        hBrush = CreateSolidBrush(0xFFFFFF);
        hBrushHighlited = CreateSolidBrush(0x00FFFF);
        hBrushDisabled = CreateSolidBrush(0x808080);

        // https://docs.microsoft.com/en-us/windows/desktop/menurc/using-keyboard-accelerators        
        g_haccel = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_MAINACCEL));
        if (g_haccel == NULL)
                return FALSE; 

        pisk.difficulty = 3;
        pisk.computer_starts_game = 1;
        new_game(1);

        ShowWindow(g_hwndMain, SW_SHOWNORMAL);
        UpdateWindow(g_hwndMain);

        return TRUE;
}

BOOL DeleteApp() {
        DeleteObject(hPen);
        DeleteObject(hPenGrid);
        DeleteObject(hBrush);
        DeleteObject(hBrushHighlited);
        DeleteObject(hBrushDisabled);
        
        return TRUE;
}

LRESULT CALLBACK WindowProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        
        switch (uMsg) {
                case WM_LBUTTONDOWN:
                        OnWM_MOUSEDOWN(wParam, lParam);
                        break;
                case WM_COMMAND:
                        switch (LOWORD(wParam)) {
                                case ID_G_NEW:
                                        new_game(1);
                                        break;
                                case ID_G_LOAD:
                                        load_game_w32();
                                        break;
                                case ID_G_SAVE:
                                        save_game_w32();
                                        break;
                                case ID_G_SETTINGS:
                                        if (DialogBox(g_hInstance, TEXT("SETTINGBOX"), hwnd, SettingsDlgProc))
                                                InvalidateRect(g_hwndMain, NULL, TRUE);
                                        return 0;
                                case ID_END:
                                        SendMessage(hwnd, WM_CLOSE, 0, 0);
                                        break;
                                case ID_ABOUT:
                                        MessageBox(hwnd, _AboutText,
                                                "About Piskworks", MB_ICONINFORMATION);
                                        break;
                        }
                        break;
                case WM_PAINT:
                        onPaint();
                        break;
                case WM_SIZE:
                        SendMessage(g_hwndStatusBar, WM_SIZE, wParam, lParam);
                        SendMessage(g_hwndToolBar, WM_SIZE, wParam, lParam);
                        break;
                case WM_DESTROY:
                        PostQuitMessage(0);
                        break;
                case WM_CLOSE:
                        DestroyWindow(hwnd);
                        break;        
                default:
                        return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        
        return 0;
}

BOOL CALLBACK SettingsDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
        static int lStone, lDifficulty, lStart;
        
        switch (message) {
                
                case WM_INITDIALOG:
                        lStone = (g_ComputerPlaysWithO == 1) ? IDC_STONE_X: IDC_STONE_O; 
                        CheckRadioButton(hDlg, IDC_STONE_X, IDC_STONE_O, lStone);
                        SetFocus(GetDlgItem(hDlg, lStone));
                        
                        lStart = pisk.computer_starts_game;
                        CheckDlgButton(hDlg, IDC_COMP_START, (lStart) ? BST_CHECKED : BST_UNCHECKED);
                        
                        lDifficulty = pisk.difficulty;
                        switch (lDifficulty) {
                                case 1: CheckRadioButton(hDlg, IDC_DIF_EASY, IDC_DIF_HARD, IDC_DIF_EASY);
                                        break;
                                case 2: CheckRadioButton(hDlg, IDC_DIF_EASY, IDC_DIF_HARD, IDC_DIF_MEDIUM);
                                        break;
                                case 3: CheckRadioButton(hDlg, IDC_DIF_EASY, IDC_DIF_HARD, IDC_DIF_HARD);
                                        break;
                        }
                        return FALSE ;
                case WM_COMMAND:
                        switch (LOWORD (wParam)) {
                                case IDC_STONE_X:
                                case IDC_STONE_O:
                                        CheckRadioButton(hDlg, IDC_STONE_X, IDC_STONE_O, LOWORD (wParam));
                                        lStone = LOWORD (wParam); 
                                        return TRUE;
                                case IDC_COMP_START:
                                        if (IsDlgButtonChecked(hDlg, IDC_COMP_START)) {
                                                CheckDlgButton(hDlg, IDC_COMP_START, BST_UNCHECKED);
                                                lStart = 0;
                                        } else {
                                                CheckDlgButton(hDlg, IDC_COMP_START, BST_CHECKED);
                                                lStart = 1;
                                        }
                                        return TRUE;
                                case IDC_DIF_EASY:
                                case IDC_DIF_MEDIUM:
                                case IDC_DIF_HARD:
                                        CheckRadioButton(hDlg, IDC_DIF_EASY, IDC_DIF_HARD, LOWORD (wParam));
                                        switch (LOWORD (wParam)) {
                                                case IDC_DIF_EASY:
                                                        lDifficulty = 1;
                                                        break;
                                                case IDC_DIF_MEDIUM:
                                                        lDifficulty = 2;
                                                        break;
                                                case IDC_DIF_HARD:
                                                        lDifficulty = 3;
                                                        break;
                                        }
                                        return TRUE;
                                case IDC_OK:
                                        g_ComputerPlaysWithO = (lStone == IDC_STONE_X) ? 1 : 0;
                                        pisk.computer_starts_game = (lStart == BST_CHECKED) ? 1 : 0;
                                        pisk.difficulty = lDifficulty;
                                        new_game(1);  
                                        EndDialog(hDlg, TRUE);
                                        return TRUE;
                                case IDC_CANCEL:
                                        EndDialog(hDlg, FALSE);
                                        return TRUE;
                        }
                        break;
        }
        
        return FALSE;
}

void OnWM_MOUSEDOWN(WPARAM wParam, LPARAM lParam) {
        int x, y;
        HDC hdc;
        int result;
        
        if ((GET_X_LPARAM(lParam) < MARGIN_X) || (GET_Y_LPARAM(lParam) < MARGIN_Y)) {
                return;
        } 
        
        x = (GET_X_LPARAM(lParam) -  MARGIN_X) / GRID_FIELD_SIZE;
        y = (GET_Y_LPARAM(lParam) -  MARGIN_Y) / GRID_FIELD_SIZE;
        
        if ((x > (pisk.gs.maxx - pisk.gs.minx)) || 
            (y > (pisk.gs.maxy - pisk.gs.miny))) {
                return;    
        }
        
        if (get_input(&pisk, x, y)) {
                SetWindowText(g_hwndStatusBar, "This field is already occupied");
                return;
        }
        update_statusbar();
        
        hdc = GetDC(g_hwndMain);
        draw_mesh(hdc, &pisk);
        ReleaseDC(g_hwndMain, hdc);
        Sleep(500);
        
        result = check_and_play(&pisk, 0);     
        if (result == 0) {
                result = check_and_play(&pisk, 1);
                hdc = GetDC(g_hwndMain);
                draw_mesh(hdc, &pisk);
                ReleaseDC(g_hwndMain, hdc);
        }
        update_statusbar();
        
        if (result == 0) {
                result = check_and_play(&pisk, 0);
        }
        
        if (result != 0) {
                if (result == 1) {
                        pisk.score_computer++;
                        MessageBox(g_hwndMain, "Computer is winner", "GAME OVER", MB_ICONINFORMATION);
                } else if (result == 2) {
                        pisk.score_player++;
                        MessageBox(g_hwndMain, "You are winner", "GAME OVER", MB_ICONINFORMATION);
                }
                new_game(0);
        }
}

void onPaint() {
        PAINTSTRUCT ps;
        HDC hdc;
        
        hdc = BeginPaint(g_hwndMain, &ps);
        draw_mesh(hdc, &pisk); 
        EndPaint(g_hwndMain, &ps);
}

void draw_mesh(HDC hdc, PISKWORKS_T *game) {
        int i, fields_x, fields_y;
        int x, y;
        STONE stone;
        
        
        fields_x = game->gs.maxx - game->gs.minx + 1;
        fields_y = game->gs.maxy - game->gs.miny + 1; 
        
        SelectObject(hdc, hPenGrid);
        SelectObject(hdc, hBrush);
        
        /* Cls */
        Rectangle(hdc, 
                MARGIN_X, 
                MARGIN_Y, 
                MARGIN_X + GRID_FIELD_SIZE * fields_x, 
                MARGIN_Y + GRID_FIELD_SIZE * fields_y);
        
        /* Draw grid */
        for (i = 0; i <= fields_x; i++) {
                MoveToEx(hdc, MARGIN_X + i * GRID_FIELD_SIZE, MARGIN_Y, NULL);
                LineTo(hdc, MARGIN_X + i * GRID_FIELD_SIZE, MARGIN_Y + fields_y * GRID_FIELD_SIZE);
        }
        for (i = 0; i <= fields_y; i++) {
                MoveToEx(hdc, MARGIN_X, MARGIN_Y + i * GRID_FIELD_SIZE, NULL);
                LineTo(hdc, MARGIN_X + fields_x * GRID_FIELD_SIZE, MARGIN_Y + i * GRID_FIELD_SIZE);
        }

        /* Draw stones */
        for (y = 0; y <= (game->gs.maxy - game->gs.miny); y++) {
                for (x = 0; x <= (game->gs.maxx - game->gs.minx); x++) {
                        stone = get_stone(game, x + game->gs.minx, y + game->gs.miny);
                        if (stone != EMPTY) {
                                draw_stone(hdc, x, y, stone, (game->last_move_x == (x + game->gs.minx)) && (game->last_move_y == (y + game->gs.miny)));
                        }                       
                }
        }
}

void draw_stone(HDC hdc, int x, int y, STONE stone, int last) {
        int x_pos, y_pos;
        
        x_pos = MARGIN_X + x * GRID_FIELD_SIZE + GRID_FIELD_SIZE / 2; 
        y_pos = MARGIN_Y + y * GRID_FIELD_SIZE + GRID_FIELD_SIZE / 2;
        SelectObject(hdc, hPen);
        SelectObject(hdc, hBrush);
        
        if (last) {
                SelectObject(hdc, hPenGrid);
                SelectObject(hdc, hBrushHighlited);
                Rectangle(hdc, 
                        MARGIN_X + x * GRID_FIELD_SIZE, 
                        MARGIN_Y + y * GRID_FIELD_SIZE, 
                        MARGIN_X + x * GRID_FIELD_SIZE + GRID_FIELD_SIZE, 
                        MARGIN_Y + y * GRID_FIELD_SIZE + GRID_FIELD_SIZE);
                SelectObject(hdc, hPen);
        }
        
        if ((g_ComputerPlaysWithO && (stone == CIRCLE)) || (!g_ComputerPlaysWithO && (stone == CROSS))) {
                Ellipse(hdc, 
                        x_pos - GRID_FIELD_SIZE / 2 + 2,
                        y_pos - GRID_FIELD_SIZE / 2 + 2,
                        x_pos + GRID_FIELD_SIZE / 2 - 2,
                        y_pos + GRID_FIELD_SIZE / 2 - 2
                );
        } else
        if ((g_ComputerPlaysWithO && (stone == CROSS)) || (!g_ComputerPlaysWithO && (stone == CIRCLE))) {
                MoveToEx(hdc, x_pos - GRID_FIELD_SIZE / 2 + 3, y_pos - GRID_FIELD_SIZE / 2 + 3, NULL);
                LineTo(hdc, x_pos + GRID_FIELD_SIZE / 2 - 3, y_pos + GRID_FIELD_SIZE / 2 - 3);
                MoveToEx(hdc, x_pos + GRID_FIELD_SIZE / 2 - 3, y_pos - GRID_FIELD_SIZE / 2 + 3, NULL);
                LineTo(hdc, x_pos - GRID_FIELD_SIZE / 2 + 3, y_pos + GRID_FIELD_SIZE / 2 - 3);
        } else 
        if (stone == NA) {
                SelectObject(hdc, hPenGrid);
                SelectObject(hdc, hBrushDisabled);
                Rectangle(hdc, 
                        MARGIN_X + x * GRID_FIELD_SIZE, 
                        MARGIN_Y + y * GRID_FIELD_SIZE, 
                        MARGIN_X + x * GRID_FIELD_SIZE + GRID_FIELD_SIZE, 
                        MARGIN_Y + y * GRID_FIELD_SIZE + GRID_FIELD_SIZE);
                SelectObject(hdc, hBrush);
                SelectObject(hdc, hPen);
        }
}

void update_statusbar() {
        _stprintf(_StatusText, "Computer:You %d:%d, Move: %d", pisk.score_computer, pisk.score_player, pisk.move_cnt);
        SetWindowText(g_hwndStatusBar, _StatusText);
}

void new_game(int reset_counter) {
        p_create_new_game(&pisk);
        if (reset_counter) {
                pisk.score_player = 0;
                pisk.score_computer = 0;
        }
        update_statusbar();
        InvalidateRect(g_hwndMain, NULL, TRUE);
}

void load_game_w32() {
        OPENFILENAME ofn;       // common dialog box structure
        char szFile[TEXT_BUFF]; // buffer for file name
        
        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = g_hwndMain;
        ofn.lpstrFile = szFile;
        // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
        // use the contents of szFile to initialize itself.
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "Piskworks\0*" EXTENSION "\0";
        ofn.nFilterIndex = 0;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        
        if (GetOpenFileName(&ofn) == TRUE) {
                if (load_game(&pisk, ofn.lpstrFile)) {
                        MessageBox(g_hwndMain, "Load failed.",
                                "Error", MB_ICONERROR);
                } else {
                        update_statusbar();
                        InvalidateRect(g_hwndMain, NULL, TRUE);
                }
        }
}

void save_game_w32() {
        OPENFILENAME ofn;       // common dialog box structure
        char szFile[TEXT_BUFF]; // buffer for file name
        char *p;
        
        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = g_hwndMain;
        ofn.lpstrFile = szFile;
        // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
        // use the contents of szFile to initialize itself.
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "Piskworks\0*" EXTENSION "\0";
        ofn.nFilterIndex = 0;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST;
        
        if (GetSaveFileName(&ofn) == TRUE) {
                p = (strlen(ofn.lpstrFile) >= strlen(EXTENSION)) ? (ofn.lpstrFile + strlen(ofn.lpstrFile) - strlen(EXTENSION)) : ofn.lpstrFile;
                if (strstr(p, EXTENSION) != p) { 
                        strncat(ofn.lpstrFile, EXTENSION, TEXT_BUFF - strlen(ofn.lpstrFile) - 1);
                }
                
                if (save_game(&pisk, ofn.lpstrFile)) {
                        MessageBox(g_hwndMain, "Save failed.",
                                "Error", MB_ICONERROR);
                } else {
                        SetWindowText(g_hwndStatusBar, "Saved...");
                }
        }
}
