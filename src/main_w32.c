/*
*       main_w32.c
*       
*       This file is part of Piskworks game.
*       https://bitbucket.org/berk76/piskworks
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
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include "piskworks.h"
#include "resource.h"

#define _MainClassName TEXT("WinAPIMainClass")
#define MARGIN 15
#define GRID_FIELD_SIZE 20
#define TEXT_BUFF 256

TCHAR _AppName[TEXT_BUFF];
TCHAR _AboutText[TEXT_BUFF];
TCHAR _StatusText[TEXT_BUFF];
HINSTANCE g_hInstance;
HWND g_hwndMain;
HWND g_hwndStatusBar;
MSG msg;
PISKWORKS_T pisk;
HPEN hPen;
HPEN hPenGrid;
HBRUSH hBrush;
HBRUSH hBrushHighlited;
HBRUSH hBrushDisabled;


static BOOL InitApp();
static BOOL DeleteApp();
static LRESULT CALLBACK WindowProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void OnWM_MOUSEDOWN(WPARAM wParam, LPARAM lParam);
static void onPaint();
static void draw_mesh(HDC hdc, PISKWORKS_T *game);
static void draw_stone(HDC hdc, int x, int y, STONE stone, int last);
static void update_statusbar();


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShow) {
        g_hInstance = hInstance;
        
        if (!InitApp())
                return FALSE;
                
        while (GetMessage(&msg, NULL, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
        
        DeleteApp();
        return msg.wParam;
}

BOOL InitApp() {
        _stprintf(_AppName, "Piskworks %s", VERSION);
        _stprintf(_AboutText, "Piskworks %s\nhttps://bitbucket.org/berk76/piskworks\n(c) 2016 Jaroslav Beran", VERSION);

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
        
        g_hwndMain = CreateWindowEx(0, // rozšíøený styl okna    
                _MainClassName, // jméno tøídy
                _AppName, // text okna
                WS_OVERLAPPEDWINDOW | WS_VISIBLE, // styl okna
                CW_USEDEFAULT, CW_USEDEFAULT, // souøadnice na obraziovce
                600, 600, // rozmìry - šíøka a výška
                (HWND)NULL, // okna pøedka
                LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MAINMENU)), // handle hlavní nabídky
                g_hInstance, // handle instance
                NULL); // další "uživatelská" data
                
        if (g_hwndMain == NULL)
                return FALSE;
                
                
        g_hwndStatusBar = CreateWindowEx(0,
                TEXT("msctls_statusbar32"),
                TEXT(" "),
                WS_CHILD | WS_VISIBLE,
                0, 0, 0, 0,
                g_hwndMain,
                (HMENU)NULL,
                g_hInstance,
                NULL);
        if ( g_hwndStatusBar == NULL )
                return FALSE;

        
        hPen = CreatePen(PS_SOLID | PS_INSIDEFRAME, 2, 0x000000);
        hPenGrid = CreatePen(PS_SOLID | PS_INSIDEFRAME, 2, 0xFFFF00);
        hBrush = CreateSolidBrush(0xFFFFFF);
        hBrushHighlited = CreateSolidBrush(0x00FFFF);
        hBrushDisabled = CreateSolidBrush(0x808080);
        
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(g_hwndStatusBar, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
        
        p_create_new_game(&pisk);
        update_statusbar();                
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
                        break;
                case WM_DESTROY:
                        PostQuitMessage(0);
                        break;
                case WM_CLOSE:
                        if (MessageBox(hwnd, "Do you want quit?", _AppName,
                                MB_YESNO | MB_ICONQUESTION) != IDYES) {
                                        return 0;
                                }
                        DestroyWindow(hwnd);
                        break;        
                default:
                        return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        
        return 0;
}

void OnWM_MOUSEDOWN(WPARAM wParam, LPARAM lParam) {
        int x, y;
        HDC hdc;
        RECT rect;
        int result;
        
        x = (GET_X_LPARAM(lParam) -  MARGIN) / GRID_FIELD_SIZE;
        y = (GET_Y_LPARAM(lParam) -  MARGIN) / GRID_FIELD_SIZE;
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
                p_create_new_game(&pisk);
                update_statusbar();
                GetClientRect(g_hwndMain, &rect);
                InvalidateRect(g_hwndMain, &rect, TRUE);
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
                MARGIN, 
                MARGIN, 
                MARGIN + GRID_FIELD_SIZE * fields_x, 
                MARGIN + GRID_FIELD_SIZE * fields_y);
        
        /* Draw grid */
        for (i = 0; i <= fields_x; i++) {
                MoveToEx(hdc, MARGIN + i * GRID_FIELD_SIZE, MARGIN, NULL);
                LineTo(hdc, MARGIN + i * GRID_FIELD_SIZE, MARGIN + fields_y * GRID_FIELD_SIZE);
        }
        for (i = 0; i <= fields_y; i++) {
                MoveToEx(hdc, MARGIN, MARGIN + i * GRID_FIELD_SIZE, NULL);
                LineTo(hdc, MARGIN + fields_x * GRID_FIELD_SIZE, MARGIN + i * GRID_FIELD_SIZE);
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
        
        x_pos = MARGIN + x * GRID_FIELD_SIZE + GRID_FIELD_SIZE / 2; 
        y_pos = MARGIN + y * GRID_FIELD_SIZE + GRID_FIELD_SIZE / 2;
        SelectObject(hdc, hPen);
        SelectObject(hdc, hBrush);
        
        if (last) {
                SelectObject(hdc, hPenGrid);
                SelectObject(hdc, hBrushHighlited);
                Rectangle(hdc, 
                        MARGIN + x * GRID_FIELD_SIZE, 
                        MARGIN + y * GRID_FIELD_SIZE, 
                        MARGIN + x * GRID_FIELD_SIZE + GRID_FIELD_SIZE, 
                        MARGIN + y * GRID_FIELD_SIZE + GRID_FIELD_SIZE);
                SelectObject(hdc, hPen);
        }
        
        if (stone == CIRCLE) {
                Ellipse(hdc, 
                        x_pos - GRID_FIELD_SIZE / 2 + 2,
                        y_pos - GRID_FIELD_SIZE / 2 + 2,
                        x_pos + GRID_FIELD_SIZE / 2 - 2,
                        y_pos + GRID_FIELD_SIZE / 2 - 2
                );
        } else
        if (stone == CROSS) {
                MoveToEx(hdc, x_pos - GRID_FIELD_SIZE / 2 + 3, y_pos - GRID_FIELD_SIZE / 2 + 3, NULL);
                LineTo(hdc, x_pos + GRID_FIELD_SIZE / 2 - 3, y_pos + GRID_FIELD_SIZE / 2 - 3);
                MoveToEx(hdc, x_pos + GRID_FIELD_SIZE / 2 - 3, y_pos - GRID_FIELD_SIZE / 2 + 3, NULL);
                LineTo(hdc, x_pos - GRID_FIELD_SIZE / 2 + 3, y_pos + GRID_FIELD_SIZE / 2 - 3);
        } else 
        if (stone == NA) {
                SelectObject(hdc, hPenGrid);
                SelectObject(hdc, hBrushDisabled);
                Rectangle(hdc, 
                        MARGIN + x * GRID_FIELD_SIZE, 
                        MARGIN + y * GRID_FIELD_SIZE, 
                        MARGIN + x * GRID_FIELD_SIZE + GRID_FIELD_SIZE, 
                        MARGIN + y * GRID_FIELD_SIZE + GRID_FIELD_SIZE);
                SelectObject(hdc, hBrush);
                SelectObject(hdc, hPen);
        }
}

void update_statusbar() {
        _stprintf(_StatusText, "Move: %d", pisk.move_cnt);
        SetWindowText(g_hwndStatusBar, _StatusText);
}
