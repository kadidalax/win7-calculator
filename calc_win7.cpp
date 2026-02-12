// Windows 7 Style Calculator - Native Win32 C++ with Calendar & Date Calc
// Minimal size, no dependencies, single EXE
// Compile with: 
// rc.exe resource.rc
// cl.exe /O2 /MT /Fe:Calculator_Win7.exe calc_win7.cpp resource.res user32.lib gdi32.lib comctl32.lib dwmapi.lib shell32.lib comdlg32.lib Msimg32.lib

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <strsafe.h>
#include <cmath>
#include <cstring>
#include <ctime>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "Msimg32.lib")

// Enable visual styles
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Constants
#define IDI_ICON        101
#define WINDOW_WIDTH    700  // Increased for history sidebar
#define WINDOW_HEIGHT   520
#define HISTORY_WIDTH   260
#define CALC_WIDTH      360
#define BUTTON_WIDTH    52
#define BUTTON_HEIGHT   40
#define DISPLAY_HEIGHT  60

// Colors
#define CLR_TEXT_NORMAL RGB(0, 0, 0)
#define CLR_TEXT_OP     RGB(0, 0, 150)

// Tab IDs
#define TAB_CALC        0
#define TAB_CALENDAR    1
#define TAB_DATECALC    2

// Control IDs
#define IDC_TAB         1
#define IDC_MONTHCAL    10
#define IDC_DATEINFO    11
#define IDC_DTP_START   20
#define IDC_DTP_END     21
#define IDC_BTN_CALCDIFF 22
#define IDC_COMBO_UNIT  23
#define IDC_EDIT_VALUE  24
#define IDC_SPIN_VALUE  25
#define IDC_BTN_CALCADD 26
#define IDC_RESULT      27
#define IDC_DTP_BASE    28
#define IDC_LIST_HISTORY 30

// Button IDs
enum ButtonID {
    BTN_0 = 100, BTN_1, BTN_2, BTN_3, BTN_4,
    BTN_5, BTN_6, BTN_7, BTN_8, BTN_9,
    BTN_ADD, BTN_SUB, BTN_MUL, BTN_DIV,
    BTN_EQUAL, BTN_DOT,
    BTN_C, BTN_CE, BTN_BACK, BTN_NEG, BTN_SQRT,
    BTN_PERCENT, BTN_RECIP,
    BTN_MC, BTN_MR, BTN_MS, BTN_MPLUS, BTN_MMINUS,
    BTN_TODAY
};

// Forward declarations
void InitFonts();

// Globals - Moved up for visibility
static HWND hTab = NULL;
static HWND hDisplay = NULL;
static HWND hMemoryIndicator = NULL;
static HWND hHistoryList = NULL;
static HFONT hFontDisplay = NULL;
static HFONT hFontButton = NULL;
static HFONT hFontNormal = NULL;

// Calendar state
struct CalendarState {
    SYSTEMTIME selectedDate;
    SYSTEMTIME today;
    HWND hMonthCal;
    HWND hInfoLabel;
    HWND hBtnToday;
    bool initialized;
    CalendarState() : initialized(false) {}
};

// Date calc state
struct DateCalcState {
    int calcMode; // 0=diff, 1=add
    HWND hResult;
    DateCalcState() : calcMode(0) {}
};

// Calculator state
struct CalcState {
    double currentValue;
    double previousValue;
    double memoryValue;
    char currentOp;
    bool waitingForOperand;
    bool hasMemory;
    char displayText[256];
    
    CalcState() : currentValue(0), previousValue(0), memoryValue(0),
                  currentOp(0), waitingForOperand(false), hasMemory(false) {
        displayText[0] = '0';
        displayText[1] = '\0';
    }
};

static CalcState g_state;
static CalendarState g_calState;
static DateCalcState g_dateState;
static int g_curTab = TAB_CALC;

void InitFonts() {
    hFontDisplay = CreateFontW(28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        
    hFontButton = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        
    hFontNormal = hFontButton; // Reuse button font for generic UI
}

// Calculator controls (for show/hide)
static HWND hCalcControls[50];
static int hCalcCount = 0;

// Forward declarations
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateTabControl(HWND hwnd);
void CreateCalculatorUI(HWND hwnd);
void CreateCalendarUI(HWND hwnd);
void CreateDateCalcUI(HWND hwnd);
void SwitchTab(int tab);
void UpdateDisplay();
void HandleButton(int id);
void Calculate();
void UpdateCalendarInfo();
void CalcDateDiff();
void CalcDateAdd();
int GetISOWeek(const SYSTEMTIME& st);
int GetDayOfYear(const SYSTEMTIME& st);
bool IsLeapYear(int year);
int DaysInMonth(int year, int month);

// Draw rounded rectangle
void RoundRect(HDC hdc, RECT* rect, int radius) {
    BeginPath(hdc);
    RoundRect(hdc, rect->left, rect->top, rect->right, rect->bottom, radius, radius);
    EndPath(hdc);
    FillPath(hdc);
}

// Draw gradient button
void DrawGradientButton(HDC hdc, RECT* rect, COLORREF start, COLORREF end, BOOL pressed) {
    TRIVERTEX vert[2];
    GRADIENT_RECT gRect;
    
    // Adjust colors for pressed state
    if (pressed) {
        COLORREF temp = start;
        start = end;
        end = temp;
    }
    
    vert[0].x = rect->left;
    vert[0].y = rect->top;
    vert[0].Red = GetRValue(start) << 8;
    vert[0].Green = GetGValue(start) << 8;
    vert[0].Blue = GetBValue(start) << 8;
    vert[0].Alpha = 0;
    
    vert[1].x = rect->right;
    vert[1].y = rect->bottom;
    vert[1].Red = GetRValue(end) << 8;
    vert[1].Green = GetGValue(end) << 8;
    vert[1].Blue = GetBValue(end) << 8;
    vert[1].Alpha = 0;
    
    gRect.UpperLeft = 0;
    gRect.LowerRight = 1;
    
    GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
    
    // Border
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, rect->left, rect->top, rect->right, rect->bottom, 4, 4);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
}

// Custom button window procedure
LRESULT CALLBACK ButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                           UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            // Determine button type from ID
            int id = (int)dwRefData;
            COLORREF start, end;
            
            if (id == BTN_EQUAL) {
                start = RGB(255, 220, 180);
                end = RGB(255, 180, 100);
            } else if (id >= BTN_MC && id <= BTN_MMINUS) {
                start = RGB(240, 240, 250);
                end = RGB(220, 220, 240);
            } else if (id >= BTN_ADD && id <= BTN_DIV) {
                start = RGB(240, 248, 255);
                end = RGB(200, 230, 255);
            } else if (id >= BTN_C && id <= BTN_RECIP) {
                start = RGB(245, 245, 245);
                end = RGB(230, 230, 230);
            } else {
                start = RGB(255, 255, 255);
                end = RGB(240, 240, 240);
            }
            
            BOOL pressed = (SendMessage(hwnd, BM_GETSTATE, 0, 0) & BST_PUSHED) != 0;
            DrawGradientButton(hdc, &rect, start, end, pressed);
            
            // Draw text
            WCHAR text[32];
            GetWindowTextW(hwnd, text, 32);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, (id >= BTN_ADD && id <= BTN_DIV) ? CLR_TEXT_OP : CLR_TEXT_NORMAL);
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFontButton);
            DrawTextW(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(hdc, hOldFont);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

// Create button helper
HWND CreateCalcButton(HWND parent, int id, const WCHAR* text, int x, int y, int w, int h) {
    DWORD style = WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON;

    // 保留系统按压反馈前提下做视觉分层
    if (id == BTN_EQUAL) style |= BS_DEFPUSHBUTTON;                  // 主操作
    else if (id >= BTN_ADD && id <= BTN_DIV) style |= BS_FLAT;       // 运算符区分

    // Remove WS_TABSTOP to keep focus on main window for keyboard input
    style &= ~WS_TABSTOP;

    HWND btn = CreateWindowW(L"BUTTON", text,
        style,
        x, y, w, h, parent, (HMENU)(UINT_PTR)id,
        GetModuleHandle(NULL), NULL);

    SendMessage(btn, WM_SETFONT, (WPARAM)hFontButton, TRUE);
    
    // Enable custom drawing
    SetWindowSubclass(btn, ButtonProc, id, (DWORD_PTR)id);
    
    if (hCalcCount < 50) hCalcControls[hCalcCount++] = btn;
    return btn;
}

// Create calculator UI
void CreateCalculatorUI(HWND hwnd) {
    // Create display
    // Width limited to allow space for history sidebar
    hDisplay = CreateWindowW(L"STATIC", L"0",
        WS_VISIBLE | WS_CHILD | SS_RIGHT | SS_NOTIFY | WS_BORDER | SS_SUNKEN,
        12, 45, 370, DISPLAY_HEIGHT, 
        hwnd, NULL, GetModuleHandle(NULL), NULL);
    if (hCalcCount < 50) hCalcControls[hCalcCount++] = hDisplay;

    // Set display font
    SendMessage(hDisplay, WM_SETFONT, (WPARAM)hFontDisplay, TRUE);

    // Memory indicator
    hMemoryIndicator = CreateWindowW(L"STATIC", L"",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        12, 110, 300, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    SendMessage(hMemoryIndicator, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
    if (hCalcCount < 50) hCalcControls[hCalcCount++] = hMemoryIndicator;

    // History ListBox (Sidebar)
    // Positioned at X=390 (original width - padding), Y=45
    hHistoryList = CreateWindowW(L"LISTBOX", NULL,
        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        390, 45, HISTORY_WIDTH, 435, 
        hwnd, (HMENU)IDC_LIST_HISTORY, GetModuleHandle(NULL), NULL);
    SendMessage(hHistoryList, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
    if (hCalcCount < 50) hCalcControls[hCalcCount++] = hHistoryList;

    int startX = 12;
    int startY = 140;
    int gap = 6;

    // Row 1: Backspace, CE, C, +/-, sqrt
    CreateCalcButton(hwnd, BTN_BACK, L"\u2190", startX, startY, BUTTON_WIDTH, 32);
    CreateCalcButton(hwnd, BTN_CE, L"CE", startX + (BUTTON_WIDTH+gap), startY, BUTTON_WIDTH, 32);
    CreateCalcButton(hwnd, BTN_C, L"C", startX + 2*(BUTTON_WIDTH+gap), startY, BUTTON_WIDTH, 32);
    CreateCalcButton(hwnd, BTN_NEG, L"\u00b1", startX + 3*(BUTTON_WIDTH+gap), startY, BUTTON_WIDTH, 32);
    CreateCalcButton(hwnd, BTN_SQRT, L"\u221a", startX + 4*(BUTTON_WIDTH+gap), startY, BUTTON_WIDTH, 32);

    // Memory row
    int memY = startY + 38;
    CreateCalcButton(hwnd, BTN_MC, L"MC", startX, memY, BUTTON_WIDTH, 32);
    CreateCalcButton(hwnd, BTN_MR, L"MR", startX + (BUTTON_WIDTH+gap), memY, BUTTON_WIDTH, 32);
    CreateCalcButton(hwnd, BTN_MS, L"MS", startX + 2*(BUTTON_WIDTH+gap), memY, BUTTON_WIDTH, 32);
    CreateCalcButton(hwnd, BTN_MPLUS, L"M+", startX + 3*(BUTTON_WIDTH+gap), memY, BUTTON_WIDTH, 32);
    CreateCalcButton(hwnd, BTN_MMINUS, L"M-", startX + 4*(BUTTON_WIDTH+gap), memY, BUTTON_WIDTH, 32);

    // Number pad
    int numStartY = memY + 40;
    const WCHAR* nums[] = {L"7", L"8", L"9", L"4", L"5", L"6", L"1", L"2", L"3", L"0"};
    int numIds[] = {BTN_7, BTN_8, BTN_9, BTN_4, BTN_5, BTN_6, BTN_1, BTN_2, BTN_3, BTN_0};

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            int idx = row * 3 + col;
            int x = startX + col * (BUTTON_WIDTH + gap);
            int y = numStartY + row * (BUTTON_HEIGHT + gap);
            CreateCalcButton(hwnd, numIds[idx], nums[idx], x, y, BUTTON_WIDTH, BUTTON_HEIGHT);
        }
    }

    // Zero button (double width)
    CreateCalcButton(hwnd, BTN_0, L"0", startX, numStartY + 3*(BUTTON_HEIGHT+gap), BUTTON_WIDTH*2+gap, BUTTON_HEIGHT);

    // Decimal
    CreateCalcButton(hwnd, BTN_DOT, L".", startX + 2*(BUTTON_WIDTH+gap), numStartY + 3*(BUTTON_HEIGHT+gap), BUTTON_WIDTH, BUTTON_HEIGHT);

    // Operators
    const WCHAR* ops[] = {L"/", L"*", L"-", L"+"};
    int opIds[] = {BTN_DIV, BTN_MUL, BTN_SUB, BTN_ADD};
    for (int i = 0; i < 4; i++) {
        CreateCalcButton(hwnd, opIds[i], ops[i], startX + 3*(BUTTON_WIDTH+gap),
            numStartY + i*(BUTTON_HEIGHT+gap), BUTTON_WIDTH, BUTTON_HEIGHT);
    }

    // Special functions
    CreateCalcButton(hwnd, BTN_PERCENT, L"%", startX + 4*(BUTTON_WIDTH+gap), numStartY, BUTTON_WIDTH, BUTTON_HEIGHT);
    CreateCalcButton(hwnd, BTN_RECIP, L"1/x", startX + 4*(BUTTON_WIDTH+gap), numStartY + (BUTTON_HEIGHT+gap), BUTTON_WIDTH, BUTTON_HEIGHT);

    // Equal button
    CreateCalcButton(hwnd, BTN_EQUAL, L"=", startX + 4*(BUTTON_WIDTH+gap),
        numStartY + 2*(BUTTON_HEIGHT+gap), BUTTON_WIDTH, BUTTON_HEIGHT*2+gap);
}

static bool IsErrorDisplay() {
    return strcmp(g_state.displayText, "Error") == 0;
}

static double GetDisplayNumber() {
    return atof(g_state.displayText);
}

static void SetDisplayNumber(double value) {
    if (value == floor(value)) sprintf(g_state.displayText, "%.0f", value);
    else sprintf(g_state.displayText, "%.12g", value);
}

static char g_lastHistory[160] = "";

static void PushHistory(const char* expr) {
    if (!expr) return;
    strncpy(g_lastHistory, expr, sizeof(g_lastHistory) - 1);
    g_lastHistory[sizeof(g_lastHistory) - 1] = '\0';

    if (hHistoryList) {
        WCHAR wExpr[256];
        MultiByteToWideChar(CP_ACP, 0, expr, -1, wExpr, 256);
        int idx = SendMessage(hHistoryList, LB_ADDSTRING, 0, (LPARAM)wExpr);
        SendMessage(hHistoryList, LB_SETCURSEL, idx, 0); // Auto scroll to bottom
    }
}

static bool CopyTextToClipboard(HWND hwnd, const char* textA) {
    if (!textA) return false;

    WCHAR wbuf[256];
    MultiByteToWideChar(CP_ACP, 0, textA, -1, wbuf, 256);
    SIZE_T bytes = (wcslen(wbuf) + 1) * sizeof(WCHAR);

    if (!OpenClipboard(hwnd)) return false;
    EmptyClipboard();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!hMem) { CloseClipboard(); return false; }

    void* p = GlobalLock(hMem);
    memcpy(p, wbuf, bytes);
    GlobalUnlock(hMem);

    SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
    return true;
}

static bool PasteTextFromClipboard(HWND hwnd, char* outA, int outSize) {
    if (!outA || outSize <= 1) return false;
    outA[0] = '\0';

    if (!OpenClipboard(hwnd)) return false;
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (!hData) { CloseClipboard(); return false; }

    WCHAR* wtext = (WCHAR*)GlobalLock(hData);
    if (!wtext) { CloseClipboard(); return false; }

    char buf[256] = {0};
    WideCharToMultiByte(CP_ACP, 0, wtext, -1, buf, sizeof(buf), NULL, NULL);
    GlobalUnlock(hData);
    CloseClipboard();

    // 简单数值合法性过滤
    int j = 0;
    for (int i = 0; buf[i] != '\0' && j < outSize - 1; ++i) {
        char c = buf[i];
        if ((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+'
            || c == 'e' || c == 'E') {
            outA[j++] = c;
        }
    }
    outA[j] = '\0';

    if (j == 0) return false;
    return true;
}

// Update display
void UpdateDisplay() {
    WCHAR wtext[256];
    MultiByteToWideChar(CP_ACP, 0, g_state.displayText, -1, wtext, 256);
    SetWindowTextW(hDisplay, wtext);

    // Update memory indicator + latest history
    WCHAR wh[256];
    if (g_lastHistory[0] != '\0') {
        WCHAR whis[180];
        MultiByteToWideChar(CP_ACP, 0, g_lastHistory, -1, whis, 180);
        StringCchPrintfW(wh, 256, L"%s%s%s",
            g_state.hasMemory ? L"M  |  " : L"",
            whis,
            L"");
        SetWindowTextW(hMemoryIndicator, wh);
    } else {
        SetWindowTextW(hMemoryIndicator, g_state.hasMemory ? L"M" : L"");
    }
}

// Handle digit input
void InputDigit(int digit) {
    if (digit < 0 || digit > 9) return;

    if (g_state.waitingForOperand || IsErrorDisplay()) {
        g_state.displayText[0] = '0' + digit;
        g_state.displayText[1] = '\0';
        g_state.waitingForOperand = false;
    } else {
        int len = (int)strlen(g_state.displayText);
        if (strcmp(g_state.displayText, "0") == 0) {
            g_state.displayText[0] = '0' + digit;
            g_state.displayText[1] = '\0';
        } else if (len < 30) {
            g_state.displayText[len] = '0' + digit;
            g_state.displayText[len + 1] = '\0';
        }
    }
    UpdateDisplay();
}

// Calculate result
void Calculate() {
    if (g_state.currentOp == 0) return;

    double left = g_state.previousValue;
    double right = GetDisplayNumber();
    double result = 0.0;

    switch (g_state.currentOp) {
        case '+': result = left + right; break;
        case '-': result = left - right; break;
        case '*': result = left * right; break;
        case '/':
            if (right == 0.0) {
                strcpy(g_state.displayText, "Error");
                PushHistory("Divide by zero");
                g_state.waitingForOperand = true;
                UpdateDisplay();
                return;
            }
            result = left / right;
            break;
        default: return;
    }

    g_state.previousValue = result;
    SetDisplayNumber(result);

    char expr[160];
    snprintf(expr, sizeof(expr), "%.12g %c %.12g = %s", left, g_state.currentOp, right, g_state.displayText);
    PushHistory(expr);

    g_state.waitingForOperand = true;
    UpdateDisplay();
}

// Handle button click
void HandleButton(int id) {
    if (id >= BTN_0 && id <= BTN_9) {
        InputDigit(id - BTN_0);
    }
    else if (id >= BTN_ADD && id <= BTN_DIV) {
        if (IsErrorDisplay()) strcpy(g_state.displayText, "0");

        if (g_state.currentOp != 0 && !g_state.waitingForOperand) {
            Calculate(); // 连续运算
        } else {
            g_state.previousValue = GetDisplayNumber();
        }

        g_state.currentOp = (id == BTN_ADD) ? '+' :
                            (id == BTN_SUB) ? '-' :
                            (id == BTN_MUL) ? '*' : '/';
        g_state.waitingForOperand = true;
    }
    else if (id == BTN_EQUAL) {
        Calculate();
        g_state.currentOp = 0;
    }
    else if (id == BTN_C) {
        g_state = CalcState();
        g_lastHistory[0] = '\0';
        UpdateDisplay();
    }
    else if (id == BTN_CE) {
        strcpy(g_state.displayText, "0");
        g_state.waitingForOperand = false;
        UpdateDisplay();
    }
    else if (id == BTN_BACK) {
        if (g_state.waitingForOperand || IsErrorDisplay()) {
            strcpy(g_state.displayText, "0");
        } else {
            int len = (int)strlen(g_state.displayText);
            if (len > 1) g_state.displayText[len - 1] = '\0';
            else strcpy(g_state.displayText, "0");
        }
        UpdateDisplay();
    }
    else if (id == BTN_DOT) {
        if (g_state.waitingForOperand || IsErrorDisplay()) {
            strcpy(g_state.displayText, "0.");
            g_state.waitingForOperand = false;
        } else if (strchr(g_state.displayText, '.') == NULL) {
            int len = (int)strlen(g_state.displayText);
            if (len < 30) {
                g_state.displayText[len] = '.';
                g_state.displayText[len + 1] = '\0';
            }
        }
        UpdateDisplay();
    }
    else if (id == BTN_NEG) {
        if (!IsErrorDisplay()) {
            if (g_state.displayText[0] == '-') {
                memmove(g_state.displayText, g_state.displayText + 1, strlen(g_state.displayText));
            } else if (strcmp(g_state.displayText, "0") != 0) {
                memmove(g_state.displayText + 1, g_state.displayText, strlen(g_state.displayText) + 1);
                g_state.displayText[0] = '-';
            }
            UpdateDisplay();
        }
    }
    else if (id == BTN_MC) {
        g_state.memoryValue = 0;
        g_state.hasMemory = false;
        UpdateDisplay();
    }
    else if (id == BTN_MR) {
        SetDisplayNumber(g_state.memoryValue);
        g_state.waitingForOperand = true;
        UpdateDisplay();
    }
    else if (id == BTN_MS) {
        g_state.memoryValue = GetDisplayNumber();
        g_state.hasMemory = (g_state.memoryValue != 0);
        g_state.waitingForOperand = true;
        UpdateDisplay();
    }
    else if (id == BTN_MPLUS) {
        g_state.memoryValue += GetDisplayNumber();
        g_state.hasMemory = (g_state.memoryValue != 0);
        g_state.waitingForOperand = true;
        UpdateDisplay();
    }
    else if (id == BTN_MMINUS) {
        g_state.memoryValue -= GetDisplayNumber();
        g_state.hasMemory = (g_state.memoryValue != 0);
        g_state.waitingForOperand = true;
        UpdateDisplay();
    }
    else if (id == BTN_SQRT) {
        double val = GetDisplayNumber();
        if (val >= 0) {
            SetDisplayNumber(sqrt(val));
            char expr[160];
            snprintf(expr, sizeof(expr), "sqrt(%.12g) = %s", val, g_state.displayText);
            PushHistory(expr);
        } else {
            strcpy(g_state.displayText, "Error");
            PushHistory("sqrt of negative");
        }
        g_state.waitingForOperand = true;
        UpdateDisplay();
    }
    else if (id == BTN_PERCENT) {
        double val = GetDisplayNumber();
        SetDisplayNumber(val / 100.0);
        char expr[160];
        snprintf(expr, sizeof(expr), "%.12g%% = %s", val, g_state.displayText);
        PushHistory(expr);
        g_state.waitingForOperand = true;
        UpdateDisplay();
    }
    else if (id == BTN_RECIP) {
        double val = GetDisplayNumber();
        if (val != 0) {
            SetDisplayNumber(1.0 / val);
            char expr[160];
            snprintf(expr, sizeof(expr), "1/(%.12g) = %s", val, g_state.displayText);
            PushHistory(expr);
        } else {
            strcpy(g_state.displayText, "Error");
            PushHistory("1/0");
        }
        g_state.waitingForOperand = true;
        UpdateDisplay();
    }
}

// --- Date Helpers ---
bool IsLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int DaysInMonth(int year, int month) {
    static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && IsLeapYear(year)) return 29;
    return days[month];
}

int GetDayOfYear(const SYSTEMTIME& st) {
    int day = 0;
    for (int i = 1; i < st.wMonth; i++) day += DaysInMonth(st.wYear, i);
    day += st.wDay;
    return day;
}

int GetISOWeek(const SYSTEMTIME& st) {
    // Simplified ISO 8601 week calculation
    int a = (14 - st.wMonth) / 12;
    int y = st.wYear + 4800 - a;
    int m = st.wMonth + 12 * a - 3;
    int J = st.wDay + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
    int d4 = (((J + 31741 - (J % 7)) % 146097) % 36524) % 1461;
    int L = d4 / 1460;
    int d1 = ((d4 - L) % 365) + L;
    return d1 / 7 + 1;
}

// --- Tab Control ---
void CreateTabControl(HWND hwnd) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES | ICC_DATE_CLASSES;
    InitCommonControlsEx(&icex);

    hTab = CreateWindowW(WC_TABCONTROL, L"",
        WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
        0, 0, WINDOW_WIDTH, 28, hwnd, (HMENU)IDC_TAB, GetModuleHandle(NULL), NULL);

    SendMessage(hTab, WM_SETFONT, (WPARAM)hFontNormal, 0);

    TCITEMW tie;
    tie.mask = TCIF_TEXT;

    tie.pszText = (LPWSTR)L"标准";
    TabCtrl_InsertItem(hTab, TAB_CALC, &tie);

    tie.pszText = (LPWSTR)L"日历";
    TabCtrl_InsertItem(hTab, TAB_CALENDAR, &tie);

    tie.pszText = (LPWSTR)L"日期计算";
    TabCtrl_InsertItem(hTab, TAB_DATECALC, &tie);
}

// --- Calendar UI ---
void CreateCalendarUI(HWND hwnd) {
    // Month Calendar
    g_calState.hMonthCal = CreateWindowW(MONTHCAL_CLASS, L"",
        WS_CHILD | WS_BORDER | MCS_DAYSTATE | MCS_WEEKNUMBERS,
        10, 40, 385, 300,
        hwnd, (HMENU)IDC_MONTHCAL, GetModuleHandle(NULL), NULL);

    // Info Label
    g_calState.hInfoLabel = CreateWindowW(L"STATIC", L"选择一个日期...",
        WS_CHILD | SS_LEFT,
        15, 360, 380, 100,
        hwnd, (HMENU)IDC_DATEINFO, GetModuleHandle(NULL), NULL);
    SendMessage(g_calState.hInfoLabel, WM_SETFONT, (WPARAM)hFontNormal, 0);

    // Today Button
    g_calState.hBtnToday = CreateWindowW(L"BUTTON", L"今天",
        WS_CHILD | BS_PUSHBUTTON,
        300, 450, 100, 30,
        hwnd, (HMENU)BTN_TODAY, GetModuleHandle(NULL), NULL);
    SendMessage(g_calState.hBtnToday, WM_SETFONT, (WPARAM)hFontNormal, 0);

    g_calState.initialized = true;
    UpdateCalendarInfo();
}

void UpdateCalendarInfo() {
    SYSTEMTIME st;
    MonthCal_GetCurSel(g_calState.hMonthCal, &st);
    
    WCHAR buf[256];
    const WCHAR* days[] = {L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday"};
    
    StringCchPrintfW(buf, 256, L"%d-%02d-%02d\n%s\nWeek: %d\nDay of Year: %d", 
        st.wYear, st.wMonth, st.wDay,
        days[st.wDayOfWeek],
        GetISOWeek(st),
        GetDayOfYear(st));
        
    SetWindowTextW(g_calState.hInfoLabel, buf);
}

// --- Date Calc UI ---
static HWND hDateCtrls[30];
static int hDateCount = 0;
static HWND hDtpStart, hDtpEnd, hDtpBase, hComboOp, hEditVal, hComboUnit, hResDiff, hResAdd;

void AddDateCtrl(HWND h) { if(hDateCount < 30) hDateCtrls[hDateCount++] = h; }

void CreateDateCalcUI(HWND hwnd) {
    // 1. Date Difference
    AddDateCtrl(CreateWindowW(L"BUTTON", L"Calculate Difference", WS_CHILD|BS_GROUPBOX, 10, 40, 385, 200, hwnd, NULL, GetModuleHandle(NULL), NULL));

    AddDateCtrl(CreateWindowW(L"STATIC", L"From:", WS_CHILD|SS_CENTERIMAGE, 20, 70, 40, 25, hwnd, NULL, NULL, NULL));
    hDtpStart = CreateWindowW(DATETIMEPICK_CLASS, L"", WS_CHILD|WS_BORDER|DTS_SHORTDATEFORMAT, 70, 70, 120, 25, hwnd, (HMENU)IDC_DTP_START, NULL, NULL);
    AddDateCtrl(hDtpStart);
        
    AddDateCtrl(CreateWindowW(L"STATIC", L"To:", WS_CHILD|SS_CENTERIMAGE, 200, 70, 30, 25, hwnd, NULL, NULL, NULL));
    hDtpEnd = CreateWindowW(DATETIMEPICK_CLASS, L"", WS_CHILD|WS_BORDER|DTS_SHORTDATEFORMAT, 240, 70, 120, 25, hwnd, (HMENU)IDC_DTP_END, NULL, NULL);
    AddDateCtrl(hDtpEnd);

    AddDateCtrl(CreateWindowW(L"BUTTON", L"Calculate Interval", WS_CHILD|BS_PUSHBUTTON, 130, 110, 140, 30, hwnd, (HMENU)IDC_BTN_CALCDIFF, NULL, NULL));

    hResDiff = CreateWindowW(L"STATIC", L"", WS_CHILD|SS_CENTER, 20, 150, 365, 80, hwnd, NULL, NULL, NULL);
    AddDateCtrl(hResDiff);

    // 2. Date Add/Sub
    AddDateCtrl(CreateWindowW(L"BUTTON", L"Add/Subtract Date", WS_CHILD|BS_GROUPBOX, 10, 260, 385, 200, hwnd, NULL, GetModuleHandle(NULL), NULL));

    hDtpBase = CreateWindowW(DATETIMEPICK_CLASS, L"", WS_CHILD|WS_BORDER|DTS_SHORTDATEFORMAT, 20, 290, 120, 25, hwnd, (HMENU)IDC_DTP_BASE, NULL, NULL);
    AddDateCtrl(hDtpBase);

    hComboOp = CreateWindowW(L"COMBOBOX", L"", WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, 150, 290, 40, 100, hwnd, NULL, NULL, NULL);
    AddDateCtrl(hComboOp);
    SendMessage(hComboOp, CB_ADDSTRING, 0, (LPARAM)L"+");
    SendMessage(hComboOp, CB_ADDSTRING, 0, (LPARAM)L"-");
    SendMessage(hComboOp, CB_SETCURSEL, 0, 0);

    hEditVal = CreateWindowW(L"EDIT", L"1", WS_CHILD|WS_BORDER|ES_NUMBER|ES_CENTER, 200, 290, 50, 25, hwnd, (HMENU)IDC_EDIT_VALUE, NULL, NULL);
    AddDateCtrl(hEditVal);

    hComboUnit = CreateWindowW(L"COMBOBOX", L"", WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, 260, 290, 80, 100, hwnd, (HMENU)IDC_COMBO_UNIT, NULL, NULL);
    AddDateCtrl(hComboUnit);
    SendMessage(hComboUnit, CB_ADDSTRING, 0, (LPARAM)L"Days");
    SendMessage(hComboUnit, CB_ADDSTRING, 0, (LPARAM)L"Weeks");
    SendMessage(hComboUnit, CB_ADDSTRING, 0, (LPARAM)L"Months");
    SendMessage(hComboUnit, CB_ADDSTRING, 0, (LPARAM)L"Years");
    SendMessage(hComboUnit, CB_SETCURSEL, 0, 0);

    AddDateCtrl(CreateWindowW(L"BUTTON", L"Calculate Date", WS_CHILD|BS_PUSHBUTTON, 130, 330, 140, 30, hwnd, (HMENU)IDC_BTN_CALCADD, NULL, NULL));

    hResAdd = CreateWindowW(L"STATIC", L"", WS_CHILD|SS_CENTER, 20, 370, 365, 80, hwnd, NULL, NULL, NULL);
    AddDateCtrl(hResAdd);
}

void CalcDateDiff() {
    SYSTEMTIME st1, st2;
    DateTime_GetSystemtime(hDtpStart, &st1);
    DateTime_GetSystemtime(hDtpEnd, &st2);

    FILETIME ft1, ft2;
    SystemTimeToFileTime(&st1, &ft1);
    SystemTimeToFileTime(&st2, &ft2);

    ULARGE_INTEGER u1, u2;
    u1.LowPart = ft1.dwLowDateTime; u1.HighPart = ft1.dwHighDateTime;
    u2.LowPart = ft2.dwLowDateTime; u2.HighPart = ft2.dwHighDateTime;

    long long diff = (long long)(u2.QuadPart - u1.QuadPart);
    if (diff < 0) diff = -diff;
    
    // 100ns units to days: / 10,000,000 / 3600 / 24
    long long days = diff / 864000000000LL;
    long long weeks = days / 7;
    int remDays = days % 7;

    WCHAR buf[128];
    StringCchPrintfW(buf, 128, L"Difference:\n%lld days\n(%lld weeks, %d days)", days, weeks, remDays);
    SetWindowTextW(hResDiff, buf);
}

void CalcDateAdd() {
    SYSTEMTIME st;
    DateTime_GetSystemtime(hDtpBase, &st);
    
    WCHAR buf[128];
    GetWindowTextW(hEditVal, buf, 128);
    int val = _wtoi(buf);
    
    int op = SendMessage(hComboOp, CB_GETCURSEL, 0, 0); // 0=+, 1=-
    if (op == 1) val = -val;
    
    int unit = SendMessage(hComboUnit, CB_GETCURSEL, 0, 0); // Day, Week, Month, Year

    // Use FILETIME for Days/Weeks
    if (unit == 0 || unit == 1) {
        if (unit == 1) val *= 7;
        
        FILETIME ft;
        SystemTimeToFileTime(&st, &ft);
        ULARGE_INTEGER u;
        u.LowPart = ft.dwLowDateTime; u.HighPart = ft.dwHighDateTime;
        u.QuadPart += (long long)val * 864000000000LL;
        
        ft.dwLowDateTime = u.LowPart; ft.dwHighDateTime = u.HighPart;
        FileTimeToSystemTime(&ft, &st);
    } 
    else if (unit == 2) { // Months
        int totalMonths = st.wYear * 12 + (st.wMonth - 1) + val;
        st.wYear = totalMonths / 12;
        st.wMonth = (totalMonths % 12) + 1;
        // Adjust days (e.g. Jan 31 + 1 month -> Feb 28/29)
        int dim = DaysInMonth(st.wYear, st.wMonth);
        if (st.wDay > dim) st.wDay = dim;
    }
    else if (unit == 3) { // Years
        st.wYear += val;
        // Adjust leap day
        if (st.wMonth == 2 && st.wDay == 29 && !IsLeapYear(st.wYear)) st.wDay = 28;
    }

    const WCHAR* dayNames[] = {L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat"};
    StringCchPrintfW(buf, 128, L"Result:\n%d-%02d-%02d (%s)\nWeek %d", 
        st.wYear, st.wMonth, st.wDay, dayNames[st.wDayOfWeek], GetISOWeek(st));
    SetWindowTextW(hResAdd, buf);
}

void SwitchTab(int tab) {
    g_curTab = tab;
    
    // 1. Calculator Controls
    int showCalc = (tab == TAB_CALC) ? SW_SHOW : SW_HIDE;
    for (int i = 0; i < hCalcCount; i++) ShowWindow(hCalcControls[i], showCalc);

    // 2. Calendar Controls
    int showCal = (tab == TAB_CALENDAR) ? SW_SHOW : SW_HIDE;
    ShowWindow(g_calState.hMonthCal, showCal);
    ShowWindow(g_calState.hInfoLabel, showCal);
    ShowWindow(g_calState.hBtnToday, showCal);

    // 3. Date Calc Controls
    int showDate = (tab == TAB_DATECALC) ? SW_SHOW : SW_HIDE;
    for (int i = 0; i < hDateCount; i++) ShowWindow(hDateCtrls[i], showDate);
    
    // Refresh to clean artifacts
    InvalidateRect(GetParent(g_calState.hMonthCal), NULL, TRUE);
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Enable DWM
            DWMNCRENDERINGPOLICY policy = DWMNCRP_ENABLED;
            DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &policy, sizeof(policy));
            MARGINS margins = {0, 0, 30, 0};
            DwmExtendFrameIntoClientArea(hwnd, &margins);
            
            InitFonts(); // Initialize fonts first
            CreateTabControl(hwnd);
            CreateCalculatorUI(hwnd);
            CreateCalendarUI(hwnd);
            CreateDateCalcUI(hwnd);
            
            // Initial state: Show calc, hide others
            SwitchTab(TAB_CALC);
            return 0;
        }
        
        case WM_NOTIFY: {
            NMHDR* pnm = (NMHDR*)lParam;
            if (pnm->idFrom == IDC_TAB && pnm->code == TCN_SELCHANGE) {
                SwitchTab(TabCtrl_GetCurSel(hTab));
            }
            else if (pnm->idFrom == IDC_MONTHCAL && (pnm->code == MCN_SELECT || pnm->code == MCN_SELCHANGE)) {
                UpdateCalendarInfo();
            }
            return 0;
        }
        
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hCtl = (HWND)lParam;
            SetBkMode(hdc, TRANSPARENT);

            if (hCtl == hDisplay && IsErrorDisplay()) {
                SetTextColor(hdc, RGB(200, 20, 20)); // Error 红色高亮
            } else {
                SetTextColor(hdc, RGB(50, 50, 50));
            }
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }
        
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
            if (dis->CtlType == ODT_BUTTON) return TRUE;
            return FALSE;
        }
        
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);
            
            if (g_curTab == TAB_CALC) {
                if (code == BN_CLICKED) HandleButton(id);
                else if (id == IDC_LIST_HISTORY && code == LBN_DBLCLK) {
                    // Double click on history item to recall value
                    int idx = SendMessage(hHistoryList, LB_GETCURSEL, 0, 0);
                    if (idx != LB_ERR) {
                        WCHAR wtext[256];
                        SendMessage(hHistoryList, LB_GETTEXT, idx, (LPARAM)wtext);
                        // Parse result (after last '=')
                        WCHAR* res = wcsrchr(wtext, L'=');
                        if (res) {
                            res++; // Skip '='
                            while (*res == L' ') res++; // Skip spaces
                            char buf[256];
                            WideCharToMultiByte(CP_ACP, 0, res, -1, buf, sizeof(buf), NULL, NULL);
                            strncpy(g_state.displayText, buf, sizeof(g_state.displayText)-1);
                            g_state.displayText[sizeof(g_state.displayText)-1] = '\0';
                            g_state.waitingForOperand = false;
                            UpdateDisplay();
                        }
                    }
                }
            }
            else if (g_curTab == TAB_CALENDAR) {
                if (id == BTN_TODAY && code == BN_CLICKED) {
                    SYSTEMTIME st;
                    GetLocalTime(&st);
                    MonthCal_SetCurSel(g_calState.hMonthCal, &st);
                    UpdateCalendarInfo();
                }
            }
            else if (g_curTab == TAB_DATECALC) {
                if (code == BN_CLICKED) {
                    if (id == IDC_BTN_CALCDIFF) CalcDateDiff();
                    else if (id == IDC_BTN_CALCADD) CalcDateAdd();
                }
            }
            return 0;
        }
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            // Gradient Background
            TRIVERTEX vert[2];
            GRADIENT_RECT gRect;
            vert[0].x = 0; vert[0].y = 0;
            vert[0].Red = 232 << 8; vert[0].Green = 244 << 8; vert[0].Blue = 252 << 8; vert[0].Alpha = 0;
            vert[1].x = rect.right; vert[1].y = rect.bottom;
            vert[1].Red = 196 << 8; vert[1].Green = 224 << 8; vert[1].Blue = 240 << 8; vert[1].Alpha = 0;
            gRect.UpperLeft = 0; gRect.LowerRight = 1;
            GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
            
            // Only draw "Calculator" title if in Calc tab? No, keep generic or update based on tab?
            // The Tab control covers the top, so we probably don't need the manually drawn title anymore.
            // But if DwmExtendFrameIntoClientArea is used, we have a glass top area.
            // Let's remove the "Calculator" text drawing to avoid clashing with Tab control.
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_KEYDOWN: {
            if (g_curTab == TAB_CALC) {
                // Ctrl+C / Ctrl+V
                if (GetKeyState(VK_CONTROL) < 0) {
                    if (wParam == 'C') {
                        CopyTextToClipboard(hwnd, g_state.displayText);
                        return 0;
                    } else if (wParam == 'V') {
                        char pasted[128];
                        if (PasteTextFromClipboard(hwnd, pasted, sizeof(pasted))) {
                            strncpy(g_state.displayText, pasted, sizeof(g_state.displayText) - 1);
                            g_state.displayText[sizeof(g_state.displayText) - 1] = '\0';
                            g_state.waitingForOperand = false;
                            PushHistory("Paste value");
                            UpdateDisplay();
                        }
                        return 0;
                    }
                }

                // 运算（先处理 Shift+8 的 *，避免被当作数字 8）
                if (wParam == VK_MULTIPLY || (wParam == '8' && GetKeyState(VK_SHIFT) < 0)) HandleButton(BTN_MUL);
                else if (wParam == VK_ADD || wParam == VK_OEM_PLUS) HandleButton(BTN_ADD);
                else if (wParam == VK_SUBTRACT || wParam == VK_OEM_MINUS) HandleButton(BTN_SUB);
                else if (wParam == VK_DIVIDE || wParam == VK_OEM_2) HandleButton(BTN_DIV);

                // 顶排数字
                else if (wParam >= '0' && wParam <= '9') InputDigit((int)(wParam - '0'));
                // 小键盘数字
                else if (wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD9) InputDigit((int)(wParam - VK_NUMPAD0));

                // 结果与编辑
                else if (wParam == VK_RETURN) HandleButton(BTN_EQUAL);
                else if (wParam == VK_BACK) HandleButton(BTN_BACK);
                else if (wParam == VK_DELETE) HandleButton(BTN_CE);
                else if (wParam == VK_ESCAPE) HandleButton(BTN_C);

                // 小数点
                else if (wParam == VK_DECIMAL || wParam == VK_OEM_PERIOD) HandleButton(BTN_DOT);

                // 快捷功能键
                else if (wParam == 'C') HandleButton(BTN_C);
                else if (wParam == 'N') HandleButton(BTN_NEG);
                else if (wParam == 'R') HandleButton(BTN_SQRT);
                else if (wParam == 'P' || (wParam == '5' && GetKeyState(VK_SHIFT) < 0)) HandleButton(BTN_PERCENT);
            }
            return 0;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// Entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    // Register window class
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"Win7CalcClass";
    
    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Calculate required window size based on client area
    RECT rc = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRectEx(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE, WS_EX_CLIENTEDGE | WS_EX_COMPOSITED);

    // Create window
    HWND hwnd = CreateWindowExW(
        WS_EX_CLIENTEDGE | WS_EX_COMPOSITED,
        L"Win7CalcClass",
        L"Calculator",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, hInstance, NULL
    );
    
    if (!hwnd) {
        MessageBoxW(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
