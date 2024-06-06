// TestConsole.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include <string>
#include <codecvt>
#include <locale>
#include <ctime>
#include <cwchar>
#include <Windows.h>

// ================================
void StringToLower(std::string &str) {
    for (int i = 0; i < str.length(); i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') str[i] = str[i] - 'A' + 'a';
    }
}
void StringToLower(std::wstring& str) {
    for (int i = 0; i < str.length(); i++) {
        if (str[i] >= L'A' && str[i] <= L'Z') str[i] = str[i] - L'A' + L'a';
    }
}
std::wstring UTF8ToUTF16(const std::string& u8) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
    std::wstring utf16_str = converter.from_bytes(u8);
    return utf16_str;
}
std::string UTF16ToUTF8(const std::wstring& u16) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
    std::string utf8_str = converter.to_bytes(u16);
    return utf8_str;
}

std::string ToString(const std::wstring &str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(str);
}
std::wstring ToWString(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.from_bytes(str);
}

bool GetVersionString(int mainVersion, int subVersion, std::wstring& douhao, std::wstring& dianhao) {
    time_t t = time(NULL);
    tm now;
    localtime_s(&now, &t);
    WCHAR szVersion[32] = { 0 };
    swprintf_s(szVersion, L"%d.%d.%02d%02d.%02d%02d", mainVersion, subVersion, now.tm_year - 100, now.tm_mon + 1, now.tm_mday, now.tm_hour);
    dianhao = szVersion;
    swprintf_s(szVersion, L"%d,%d,%02d%02d,%02d%02d", mainVersion, subVersion, now.tm_year - 100, now.tm_mon + 1, now.tm_mday, now.tm_hour);
    douhao = szVersion;
    return true;
}
bool GetVersionString(int mainVersion, int subVersion, std::string& douhao, std::string& dianhao) {
    time_t t = time(NULL);
    tm now;
    localtime_s(&now, &t);
    char szVersion[32] = { 0 };
    sprintf_s(szVersion, "%d.%d.%02d%02d.%02d%02d", mainVersion, subVersion, now.tm_year - 100, now.tm_mon + 1, now.tm_mday, now.tm_hour);
    dianhao = szVersion;
    sprintf_s(szVersion, "%d,%d,%02d%02d,%02d%02d", mainVersion, subVersion, now.tm_year - 100, now.tm_mon + 1, now.tm_mday, now.tm_hour);
    douhao = szVersion;
    return true;
}
std::string GetTimeString() {
    time_t t = time(NULL);
    tm now;
    localtime_s(&now, &t);
    char szVersion[32] = { 0 };
    sprintf_s(szVersion, "%04d%02d%02d_%02d%02d%02d", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
    return szVersion;
}


bool IsStringNumber(const std::wstring &str) {
    for (int i = 0; i < str.length(); i++) {
        if (str[i] < L'0' || str[i] > L'9') return false;
    }
    return true;
}
bool IsStringNumber(const std::string& str) {
    for (int i = 0; i < str.length(); i++) {
        if (str[i] < '0' || str[i] > '9') return false;
    }
    return true;
}

void ShowMessage(std::wstring data) {
    MessageBoxW(NULL, data.c_str(), L"错误", MB_OK | MB_ICONERROR);
}
// ================================

void UpdateFileVersion(int mainVersion, int subVersion, const std::wstring& filename, bool bChangeProductVersion) {
    std::wifstream fin(filename, std::ios::binary);
    if (!fin) {
        std::cerr << "Error opening file for reading!" << std::endl;
        ShowMessage(L"打开资源文件失败, 请检查或手动修改版本信息");
        return ;
    }

    // 根据文件开头两个字节判断是大端还是小端
    wchar_t bom[2];
    fin.read(reinterpret_cast<wchar_t*>(bom), 2);
    printf("%02X, %02X", bom[0], bom[1]);
    std::wifstream::traits_type::int_type bom_value = bom[0] << 8 | bom[1];

    // 设置正确的字节序
    if (bom_value == 0xFEFF) {
        // 大端, 0, 默认
        fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf16<wchar_t, 0x10FFFE>));
        std::cout << "大端序" << std::endl;
    }
    else if (bom_value == 0xFFFE) {
        // 小端, 1
        fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf16<wchar_t, 0x10FFFE, std::little_endian>));
        std::cout << "小端序" << std::endl;
    }
    else {
        std::cerr << "Unknown byte order!" << std::endl;
        fin.close();
        ShowMessage(L"资源文件中包含不支持的字符序列, 请检查或手动修改版本信息");
        return ;
    }
    // 生成版本信息
    std::string douhao = "", dianhao = "";
    GetVersionString(mainVersion, subVersion, douhao, dianhao);
    std::wcout << L"version: " << douhao.c_str() << std::endl;
    std::wcout << L"version: " << dianhao.c_str() << std::endl;
    // 备份资源文件, 避免修改异常后无法回退
    std::cout << "备份资源文件, 如果编译有异常, 请使用备份后的文件覆盖原资源文件 (不会自动删除历史备份的文件, 需要手动清理)" << std::endl;
    std::wcout << L"原资源文件: " << filename << std::endl;
    // 先备份
    std::wstring backFile = filename;
    backFile += L".back.";
    backFile += ToWString(GetTimeString());
    BOOL bSuccess = CopyFileW(filename.c_str(), backFile.c_str(), TRUE);
    std::wcout << L"备份后的资源文件: " << backFile << L" " << (bSuccess?L"成功":L"失败");
    if (bSuccess) {
        std::wcout << std::endl;
    }
    else {
        std::wcout << L", 错误码: " << GetLastError() << std::endl;
    }

    std::wstring fulldata = L"";
    std::wstring line = L"";
    while (std::getline(fin, line)) {
        std::string s = UTF16ToUTF8(line);
        StringToLower(s);
        if (s.find("value \"fileversion\"") != std::string::npos) {
            std::string appendLine = "            VALUE \"FileVersion\", ";
            appendLine += "\"";
            appendLine += dianhao;
            appendLine += "\"\r\n";
            std::cout << "set file version2: " << appendLine << std::endl;
            fulldata += UTF8ToUTF16(appendLine);
        }
        else if (s.find("value \"productversion\"") != std::string::npos) {
            if (bChangeProductVersion) {
                std::string appendLine = "            VALUE \"ProductVersion\", ";
                appendLine += "\"";
                appendLine += dianhao;
                appendLine += "\"\r\n";
                std::cout << "set product version2: " << appendLine << std::endl;
                fulldata += UTF8ToUTF16(appendLine);
            }
            else {
                fulldata += line;
                fulldata += UTF8ToUTF16("\n");
            }
        }else if (s.find("fileversion") != std::string::npos) {
            std::string appendLine = " FILEVERSION ";
            appendLine += douhao;
            appendLine += "\r\n";
            std::cout << "set file version: " << appendLine << std::endl;
            fulldata += UTF8ToUTF16(appendLine);
        }
        else if (s.find("productversion") != std::string::npos) {
            if (bChangeProductVersion) {
                std::string appendLine = " PRODUCTVERSION ";
                appendLine += douhao;
                appendLine += "\r\n";
                std::cout << "set product version: " << appendLine << std::endl;
                fulldata += UTF8ToUTF16(appendLine);
            }
            else {
                fulldata += line;
                fulldata += UTF8ToUTF16("\n");
            }
        }else {
            fulldata += line;
            fulldata += UTF8ToUTF16("\n");
        }
    }
    fin.close();
    // write
    std::wofstream fout(filename, std::ios::binary);
    if (!fout) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    // 写入BOM（Byte Order Mark）
    // 按照小端序
    wchar_t bom2[] = { wchar_t(0xFF), wchar_t(0xFE) };
    fout.write((wchar_t*)bom2, 2);

    // 设置编码转换器为UTF-16小端
    fout.imbue(std::locale(fout.getloc(), new std::codecvt_utf16<wchar_t, 0x10FFFE, std::little_endian>));
    fout << fulldata;
    fout.close();
    std::cout << "版本信息更新完成." << std::endl;
}

// 使用说明: 首先在项目右键上添加资源, 选择Version, 然后修改好自定义的版本其他信息, 一般资源文件和项目同名
// 
// 在 <项目属性-->生成事件-->预先生成事件> 中指定命令行, path/UpdateFileVersion.exe 1 0 $(ProjectName).rc
// 
// 参数列表: 主版本号 次版本号 资源文件名
int wmain(int argc, WCHAR** argv){
    // 支持输出wstring
    std::wcout.imbue(std::locale("chs"));

    if (argc < 4) {
        ShowMessage(L"参数不足: 主版本号 次版本号 资源文件名 (请检查或手动修改版本信息)");
        return 1;
    }
    
    if (!IsStringNumber(argv[1]) || !IsStringNumber(argv[2])) {
        ShowMessage(L"参数不足: 主版本号 次版本号 资源文件名");
        return 2;
    }
    // 检测文件是否存在
    if (_waccess(argv[3], 0) == -1) {
        if (errno == ENOENT) {
            std::cerr << "文件不存在." << std::endl;
            ShowMessage(L"资源文件不存在, 请检查或手动修改版本信息");
        }
        else if (errno == EACCES) {
            std::cerr << "无权限访问." << std::endl;
            ShowMessage(L"资源文件无权限访问, 请检查或手动修改版本信息");
        }
        return 3;
    }

    // 
    std::wcout << L"主版本号: " << argv[1] << std::endl;
    std::wcout << L"次版本号: " << argv[2] << std::endl;
    std::wcout << L"资源文件: " << argv[3] << std::endl;
    bool bChangeProductVersion = false;
    if (argc > 4) {
        bChangeProductVersion = std::stoi(argv[4]);
    }
    std::wcout << L"是否同步修改产品版本: " << bChangeProductVersion << std::endl;
    UpdateFileVersion(std::stoi(argv[1]), std::stoi(argv[2]), argv[3], bChangeProductVersion);
    return 0;
}
