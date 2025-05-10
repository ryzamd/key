#include "../encryption.h"

// Khởi tạo key mã hóa
std::string encryptionKey;

std::string GenerateEncryptionKey() {
    std::string baseKey = "3301Kir@" + std::to_string(2021);

    char computerName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
    DWORD size = sizeof(computerName);
    GetComputerNameA(computerName, &size);

    char userName[256] = { 0 };
    DWORD userSize = sizeof(userName);
    GetUserNameA(userName, &userSize);

    return baseKey + computerName + userName;
}

std::string AdvancedEncrypt(const std::string& data) {
    // Sử dụng AES thay vì XOR đơn giản
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;

    if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        // Fallback to simple encryption if CryptoAPI fails
        std::string encrypted = data;
        for (size_t i = 0; i < encrypted.length(); i++) {
            encrypted[i] = encrypted[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        return encrypted;
    }

    // Create hash of the encryption key
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        // Fallback
        std::string encrypted = data;
        for (size_t i = 0; i < encrypted.length(); i++) {
            encrypted[i] = encrypted[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        return encrypted;
    }

    // Hash the encryption key
    if (!CryptHashData(hHash, (BYTE*)encryptionKey.c_str(), encryptionKey.length(), 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        // Fallback
        std::string encrypted = data;
        for (size_t i = 0; i < encrypted.length(); i++) {
            encrypted[i] = encrypted[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        return encrypted;
    }

    // Derive encryption key from hash
    if (!CryptDeriveKey(hProv, CALG_AES_256, hHash, CRYPT_EXPORTABLE, &hKey)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        // Fallback
        std::string encrypted = data;
        for (size_t i = 0; i < encrypted.length(); i++) {
            encrypted[i] = encrypted[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        return encrypted;
    }

    // Copy data to buffer and encrypt
    std::vector<BYTE> buffer(data.begin(), data.end());
    // Ensure space for padding
    buffer.resize(((data.size() + 15) / 16) * 16);

    DWORD dataLen = static_cast<DWORD>(data.size());
    if (!CryptEncrypt(hKey, 0, TRUE, 0, buffer.data(), &dataLen, buffer.size())) {
        CryptDestroyKey(hKey);
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        // Fallback
        std::string encrypted = data;
        for (size_t i = 0; i < encrypted.length(); i++) {
            encrypted[i] = encrypted[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        return encrypted;
    }

    // Convert encrypted data to string
    std::string result(reinterpret_cast<char*>(buffer.data()), dataLen);

    // Clean up
    CryptDestroyKey(hKey);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    return result;
}

std::string AdvancedDecrypt(const std::string& encryptedData) {
    // Sử dụng AES 
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;

    if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        // Fallback to simple decryption
        std::string decrypted = encryptedData;
        for (size_t i = 0; i < decrypted.length(); i++) {
            decrypted[i] = decrypted[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        return decrypted;
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        // Fallback
        std::string decrypted = encryptedData;
        for (size_t i = 0; i < decrypted.length(); i++) {
            decrypted[i] = decrypted[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        return decrypted;
    }

    if (!CryptHashData(hHash, (BYTE*)encryptionKey.c_str(), encryptionKey.length(), 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        // Fallback
        std::string decrypted = encryptedData;
        for (size_t i = 0; i < decrypted.length(); i++) {
            decrypted[i] = decrypted[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        return decrypted;
    }

    if (!CryptDeriveKey(hProv, CALG_AES_256, hHash, CRYPT_EXPORTABLE, &hKey)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        // Fallback
        std::string decrypted = encryptedData;
        for (size_t i = 0; i < decrypted.length(); i++) {
            decrypted[i] = decrypted[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        return decrypted;
    }

    std::vector<BYTE> buffer(encryptedData.begin(), encryptedData.end());

    DWORD dataLen = static_cast<DWORD>(buffer.size());
    if (!CryptDecrypt(hKey, 0, TRUE, 0, buffer.data(), &dataLen)) {
        CryptDestroyKey(hKey);
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        // Fallback
        std::string decrypted = encryptedData;
        for (size_t i = 0; i < decrypted.length(); i++) {
            decrypted[i] = decrypted[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        return decrypted;
    }

    std::string result(reinterpret_cast<char*>(buffer.data()), dataLen);

    CryptDestroyKey(hKey);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    return result;
}

std::string base64_encode(const std::string& data) {
    DWORD flags = CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF;
    DWORD bufferSize = 0;

    // Lấy kích thước buffer cần thiết
    if (!CryptBinaryToStringA((BYTE*)data.c_str(), data.size(), flags, NULL, &bufferSize)) {
        // Fallback to manual implementation if CryptoAPI fails
        static const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string encoded;
        int val = 0, valb = -6;

        for (unsigned char c : data) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }

        if (valb > -6) {
            encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }

        while (encoded.size() % 4) {
            encoded.push_back('=');
        }

        return encoded;
    }

    // Tạo buffer và chuyển đổi
    std::vector<char> buffer(bufferSize);

    if (!CryptBinaryToStringA((BYTE*)data.c_str(), data.size(), flags, buffer.data(), &bufferSize)) {
        // Fallback to manual implementation
        static const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string encoded;
        int val = 0, valb = -6;

        for (unsigned char c : data) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }

        if (valb > -6) {
            encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }

        while (encoded.size() % 4) {
            encoded.push_back('=');
        }

        return encoded;
    }

    return std::string(buffer.data(), bufferSize - 1); // -1 để loại bỏ null terminator
}