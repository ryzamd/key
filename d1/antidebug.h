#pragma once
#include "common.h"

// Kiểm tra xem ứng dụng có đang bị debug không
bool IsBeingDebugged();

// Kiểm tra xem có đang chạy trong VM không
bool IsRunningInVM();

// Kiểm tra xem có đang bị giám sát không
bool IsBeingMonitored();

// Tự hủy ứng dụng
void SelfDestruct();