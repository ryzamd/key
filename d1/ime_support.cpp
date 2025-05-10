#include "common.h"
#include "utils.h"
#include "multilanguage.h"
#include <imm.h>

// Hook tất cả cửa sổ để bắt IME
HHOOK g_imeMessageHook = NULL;
HWND g_lastImeWindow = NULL;

// Bắt IME trực tiếp khi người dùng gõ
LRESULT CALLBACK MessageHookProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code < 0)
        return CallNextHookEx(NULL, code, wParam, lParam);

    MSG* msg = (MSG*)lParam;

    if (msg->message == WM_IME_COMPOSITION && (msg->lParam & GCS_RESULTSTR)) {
        HIMC hImc = ImmGetContext(msg->hwnd);
        if (hImc) {
            DWORD len = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, NULL, 0);
            if (len > 0) {
                wchar_t* buffer = new wchar_t[len / 2 + 1];
                ImmGetCompositionStringW(hImc, GCS_RESULTSTR, buffer, len);
                buffer[len / 2] = 0;

                // Ghi kết quả IME trực tiếp
                std::wstring result(buffer);
                WriteUnicode(result);

                delete[] buffer;
            }
            ImmReleaseContext(msg->hwnd, hImc);
        }
    }

    return CallNextHookEx(NULL, code, wParam, lParam);
}

// Khởi tạo hook IME
void InitializeImeSupport() {
    g_imeMessageHook = SetWindowsHookEx(WH_GETMESSAGE, MessageHookProc, NULL, 0);
}

// Giải phóng hook
void CleanupImeSupport() {
    if (g_imeMessageHook)
        UnhookWindowsHookEx(g_imeMessageHook);
}