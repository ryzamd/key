#pragma once
#include "common.h"

// Callback function cho hook bàn phím
LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);

// Công việc hook chính
void HookJob();

// Dừng hook
void RemoveHookThread(int code);

// Tạo thread hook
void CreateHookThread();

// Thread kiểm tra an toàn
void SecurityCheckJob();