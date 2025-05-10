#pragma once
#include "common.h"

// Tạo key mã hóa dựa trên thông tin máy tính
std::string GenerateEncryptionKey();

// Mã hóa dữ liệu sử dụng Windows CryptoAPI
std::string AdvancedEncrypt(const std::string& data);

// Giải mã dữ liệu
std::string AdvancedDecrypt(const std::string& encryptedData);

// Encode dữ liệu thành Base64
std::string base64_encode(const std::string& data);