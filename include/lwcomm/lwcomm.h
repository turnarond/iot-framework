/*
 * Copyright (c) 2024 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: lwcomm.h .
 *
 * Date: 2024-03-09
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#pragma once

#ifdef _WIN32
#ifdef lwcomm_EXPORTS
#define LWCOMM_API __declspec(dllexport)
#else
#define LWCOMM_API __declspec(dllimport)
#endif
#else //_WIN32
#define LWCOMM_API __attribute__((visibility("default")))
#include <sys/types.h>
#endif //_WIN32

#include <list>
#include <string>
#include <time.h>
#include <vector>

using namespace std;

#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
  #define LWFS_HAS_CPP17 1
  #include <string_view>
#else
  #define LWFS_HAS_CPP17 0
#endif

#ifndef LW_SUCCESS
#define LW_SUCCESS 0
#endif

#define SAFE_FREE(p) if (p) { \
                         free(p); \
                         p = NULL;\
                     }

#ifndef MAX_PATH
#define MAX_PATH         260
#endif

#define LW_LONGFILENAME_MAXLEN  1024
#define LW_SHORTFILENAME_MAXLEN 260

#ifdef _WIN32
#define LW_OS_DIR_SEPARATOR "\\"
#else
#define LW_OS_DIR_SEPARATOR "/"
#endif

#ifdef _WIN32
#define LW_OS_DIR_SEPARATOR_CHAR '\\'
#else
#define LW_OS_DIR_SEPARATOR_CHAR '/'
#endif

/*
 * mutex and spin
 */
#ifdef _WIN32
#include <windows.h>
#define hmutex_t CRITICAL_SECTION
#define hmutex_init(mtx) (InitializeCriticalSectionEx(mtx, 8192, CRITICAL_SECTION_NO_DEBUG_INFO) ? 0 : -1)
#define hmutex_destroy(mtx) DeleteCriticalSection(mtx)
#define hmutex_lock(mtx) EnterCriticalSection(mtx)
#define hmutex_unlock(mtx) LeaveCriticalSection(mtx)
#define hmutex_trylock(mtx) (TryEnterCriticalSection(mtx) ? VEOK : VEFAIL)

#define hspin_t CRITICAL_SECTION
#define hspin_init(spin) (InitializeCriticalSectionEx(spin, 81920, CRITICAL_SECTION_NO_DEBUG_INFO) - 1)
#define hspin_destroy(spin) DeleteCriticalSection(spin)
#define hspin_lock(spin) EnterCriticalSection(spin)
#define hspin_unlock(spin) LeaveCriticalSection(spin)
#define hspin_trylock(spin) (TryEnterCriticalSection(spin) - 1)
#else
#include <pthread.h>
#include <sys/time.h>  
#define hmutex_t pthread_mutex_t
#define hmutex_init(mtx) (pthread_mutex_init(mtx, 0) ? -1 : 0)
#define hmutex_destroy(mtx) (pthread_mutex_destroy(mtx) ? -1 : 0)
#define hmutex_lock(mtx) pthread_mutex_lock(mtx)
#define hmutex_unlock(mtx) pthread_mutex_unlock(mtx)
#define hmutex_trylock(mtx) (pthread_mutex_trylock(mtx) ? -1 : 0)

#define hspin_t pthread_spinlock_t
#define hspin_init(spin) pthread_spin_init(spin, 0)
#define hspin_destroy(spin) pthread_spin_destroy(spin)
#define hspin_lock(spin) pthread_spin_lock(spin)
#define hspin_unlock(spin) pthread_spin_unlock(spin)
#define hspin_trylock(spin) pthread_spin_trylock(spin)
#endif

#define EC_ICV_CC_PATHNAMETOOLLONG 11201     /* Path name too long */
#define EC_ICV_CC_FAILTOGETCWD 11202         /* Fail to get current working directory */
#define EC_ICV_COMM_INVALIDHEXSTR 11203      /* Invalid hex string */
#define EC_ICV_CC_FILENOTEXIST 11204         /* File not exist */
#define EC_ICV_COMM_BUFFERTOOSHORT 11205      /* Buffer too short */
#define EC_ICV_CC_PARAMINVALID 11210         /* Invalid parameter */
#define EC_ICV_CC_FAILTOCREATEFILEPATH 11233 /* Fail to create directory */
#define EC_ICV_CC_FAILTODELETEFILEPATH 11234 /* Fail to delete directory */
#define EC_ICV_CC_FAILTORENAMEFILEPATH 11235 /* Fail to rename directory */
#define EC_ICV_CC_FAILTODELETEFILE 11236     /* Fail to remove file */
#define EC_ICV_CC_FILEPATHNOTEXIST 11237     /* File path not exist */
#define EC_ICV_CC_PATHNAMECONVERTFAIL 11250  /* Fail to convert path name */
#define EC_ICV_CC_CANNOTWRITEFILE 11251      /* Can not write file */
#define EC_ICV_CC_FILEEMPTY 11252            /* File empty */
#define EC_ICV_CC_FILEREADFAILED 11253       /* Fail to read file */

class LWComm
{
private:
    LWComm(void);

    public:
    ~LWComm();

    LWCOMM_API static const char *GetHomePath(void);

    LWCOMM_API static const char *GetConfigPath(void);

    LWCOMM_API static const char *GetBinPath(void);

    LWCOMM_API static const char *GetLibPath(void);

    LWCOMM_API static const char *GetModelPath(void);

    LWCOMM_API static const char *GetDataPath(void);

    LWCOMM_API static const char *GetImagePath(void);

    LWCOMM_API static const char *GetProcessName(void);

    LWCOMM_API static const char *GetLogPath(void);

    LWCOMM_API static const char *GetRunTimeDataPath(void);

    /**
     * @brief 毫秒级睡眠
     * @param ms 毫秒数
     */
    LWCOMM_API static void Sleep(unsigned int ms);
};

class LWFileHelper
{
public:

struct dir_entry {
    bool is_directory;
    std::string name;
};
class dir_entry_set  {
private:
    std::vector<dir_entry> entries;
public:
    void add(const dir_entry& e) { entries.push_back(e); }
    size_t size() const { return entries.size(); }
    const dir_entry& at(size_t i) const { return entries.at(i); }
    const dir_entry* get(size_t i) const { 
        return i < entries.size() ? &entries[i] : nullptr; 
    }
    const dir_entry& operator[](size_t index) const {
        return entries[index];  // 可能越界，但 STL 风格
    }
    auto begin() const -> decltype(entries.begin()) {
        return entries.begin();
    }

    auto end() const -> decltype(entries.end()) {
        return entries.end();
    }

    friend int ListDirEntry(const char*, dir_entry_set&);
    void clear() { entries.clear(); }

    dir_entry_set() = default;
    ~dir_entry_set() = default;
};


    //////////////////////////////////////////////////////////////////////////
    // File Dir operation
    LWCOMM_API static int WriteToFile(const char *szAbsPathName, const char *pFileContent, int lFileLen);

    // Directory operation
    // Concat directory
    LWCOMM_API static int ConCatDir(const char *szDirName, const char *szAbsParentDir, char *szResultDir,
                                    int lResultDirBufLen);

    // Concat directory
    LWCOMM_API static int ConCatDirAndFile(const char *szFileName, const char *szAbsParentDir, char *szResultPathName,
                                           int lResultDirBufLen);

    // create directory recursively
    // LWCOMM_API static int CreateDir(const char * szDirName, const char * szAbsParentDir);

    // create directory recursively
    LWCOMM_API static int CreateDir(const char *szAbsDirPath);

    // is directory empty
    LWCOMM_API static bool IsDirectoryEmpty (const char *szAbsDirName);

    // Rename a dir.
    LWCOMM_API static int RenameDir(const char *szOldDirName, const char *szNewDirName, const char *szAbsParentDir);

    // is given pathname a directory
    LWCOMM_API static bool IsDirectoryExist(const char *szAbsDir);

    // Delete directory recursively
    LWCOMM_API static int DeleteDir(const char * szAbsParentDir);

    // Rename directory
    // LWCOMM_API static int RenameDir(const char * szOldDirName, const char * szNewDirName, const char *
    // szAbsParentDir);

    // list sub directory.
    LWCOMM_API static int ListSubDir(const char *szAbsParentDir, std::list<std::string> &dirNameList);

    // list files in directory.
    LWCOMM_API static int ListFilesInDir(const char *szAbsParentDir, std::list<std::string> &fileNameList);

    // Delete a file
    LWCOMM_API static int DeleteAFile(const char *szFileName, const char *szAbsParentDir);

    // Copy Files in Dir recursively
    LWCOMM_API static int CopyDir(const char *szAbsSrcFilePath, const char *szAbsDsFilePath);

    // Copy File to other
    LWCOMM_API static int CopyAFile(const char *szAbsSrcFilePath, const char *szAbsDsFilePath);

    // 需要自己释放pFileContent的内存 [废弃]
    LWCOMM_API static int ReadFile(const char *pFileName, char **pFileContent, int *lFileSize);

    // Readfile
    LWCOMM_API static int ReadFile(const char *pFileName, std::vector<char>& buffer);

    // file is exist
    LWCOMM_API static bool IsFileExist(const char *szFileName, const char *szAbsParentDir);

    // file is exis::
    LWCOMM_API static bool IsFileExist(const char *szFullPathName);

    // Regulate file path
    LWCOMM_API static int RegulatePathName(char *szPathName, int lPathNameLen);

    LWCOMM_API static bool IsDirectory(const char *szAbsPathNameIn);

    LWCOMM_API static int RelativeToFullPathName(const char *pszRelativePathName, char *pszFullPathName,
                                                 int lFullPathSize);

    LWCOMM_API static int RelativeToFullPathNameEx(const char *pszRelativePathName, const char *pszParentPath,
                                                   char *pszFullPathName, int lFullPathSize);

    LWCOMM_API static int GetWorkingDirectory(char *szWorkDirBuff, int lWorkDirBuffLen);

    LWCOMM_API static int SetWorkingDirectory(const char *szWorkDirBuff);

    LWCOMM_API static int SeparatePathName(const char *szPathName, char *szPath, int lPathLen, char *szName,
                                           int NameLen);

    LWCOMM_API static bool IsAbsFilePath(string strFilePath);

    LWCOMM_API static int ListDirEntry(const char *szAbsParentDir, dir_entry_set &dirEntrySet);
};

#if LWFS_HAS_CPP17
class LWFileSystem
{
public:
    struct Entry  {
        bool is_directory;
        std::string name;
        std::string path; // 完成的路径
    };

    /////////////////////////////////////Read & Write file////////////////////////////////
    LWCOMM_API static bool WriteToFile(std::string_view file_path, std::string_view content);
    LWCOMM_API static bool WriteToFile(std::string_view path, const std::vector<char>& data);
    LWCOMM_API static std::vector<char> ReadFile(std::string_view path);

    //////////////////////////////////////File Operation//////////////////////////////////
    LWCOMM_API static bool CreateFile(std::string_view path);
    LWCOMM_API static bool DeleteFile(std::string_view path);
    LWCOMM_API static bool FileExist(std::string_view path);
    LWCOMM_API static bool IsFile(std::string_view path);

    ////////////////////////////////////////Dir Operation/////////////////////////////////
    LWCOMM_API static bool CreateDirectory(std::string_view path, bool recursive = true);
    LWCOMM_API static bool RemoveDirectory(std::string_view path, bool recursive = true);
    LWCOMM_API static bool RenamePath(std::string_view oldPath, std::string_view newPath);
    LWCOMM_API static bool IsDirectory(std::string_view path);
    LWCOMM_API static bool IsDirectoryEmpty(std::string_view path);

    ///////////////////////////////////////Concat Path/////////////////////////////////
    LWCOMM_API static std::string ConcatPath(std::string_view parent, std::string_view child);

    /////////////////////////////////////////Path Operation///////////////////////////////
    LWCOMM_API static std::vector<Entry> ListDirectory(std::string_view path);
    LWCOMM_API static std::string AbsolutePath(std::string_view path);
    LWCOMM_API static std::string CurrentWorkingDirectory();
    LWCOMM_API static bool ChangeWorkingDirectory(std::string_view path);
    /* Spite path to dir and filename */
    LWCOMM_API static std::pair<std::string, std::string> SplitPath(std::string_view fullPath); // {dir, filename}
    LWCOMM_API static bool IsAbsolutePath(std::string_view path);
};
#endif

class LWStringHelper
{
public:
    LWCOMM_API static int SafeStrNCpy(char *szDest, const char *szSource, int nDestBuffLen);

    LWCOMM_API static int StringToInt(const char *szValue);

    LWCOMM_API static int StringToIntEx(const char *szValue, int nDefault);

    LWCOMM_API static double StringToDouble(const char *szValue);

    LWCOMM_API static int StriCmp(const char *szCmp1, const char *szCmp2);

    LWCOMM_API static int StrEscape(const char *szSrc, char *szEscape, size_t nEscapeBuffLen, char cEscaped,
                                    char cEscaping);

    LWCOMM_API static vector<string> StriSplit(const string str, const string pattern);

    LWCOMM_API static void Replace(std::string &sBig, const std::string &sSrc, const std::string &sDst);

    LWCOMM_API static int HexDumpBuf(const char *szBuffer, unsigned int nCharBuffLen, char *szHexBuf,
                                     unsigned int nHexBufLen, unsigned int *pnRetHexBufLen);

    LWCOMM_API static int HexStr2Buffer(const char *szHexStr, char *szBuffer, unsigned int *pnBufLen);
};

class LWTimeHelper {
public:
    /**
     * @brief 将时间字符串（含毫秒）转换为时间戳（毫秒）
     * @param time_str 时间字符串，格式: "2025-08-05 12:34:56.789"
     * @param timestamp_ms 输出：毫秒级时间戳
     * @return LW_SUCCESS 成功，EC_ICV_INVALID_PARAM 格式错误
     */
    LWCOMM_API static int String2Timestamp(const char* time_str, int64_t* timestamp_ms);

    /**
     * @brief 将时间戳（毫秒）转换为格式化时间字符串
     * @param timestamp_ms 毫秒级时间戳
     * @param buffer 输出缓冲区
     * @param buf_len 缓冲区长度
     * @param include_ms 是否包含毫秒 ".000"
     * @return LW_SUCCESS 成功，EC_ICV_INVALID_PARAM 参数错误，EC_ICV_COMM_BUFFERTOOSHORT 缓冲区不足
     */
    LWCOMM_API static int Timestamp2String(int64_t timestamp_ms, char* buffer, size_t buf_len, bool include_ms = false);

    /**
     * @brief 将 UTC 时间字符串（含毫秒）转换为时间戳（毫秒）
     * @param time_str 时间字符串，格式: "2025-08-05 12:34:56.789"
     * @param timestamp_ms 输出：毫秒级时间戳
     * @return LW_SUCCESS 成功，EC_ICV_INVALID_PARAM 格式错误
     */
    LWCOMM_API static int64_t UtcString2Timestamp(const char* time_str);

    /**
     * @brief 将 UTC 时间戳（毫秒）转换为格式化时间字符串
     * @param timestamp_ms 毫秒级时间戳
     * @param buffer 输出缓冲区
     * @param buf_len 缓冲区长度
     * @param include_ms 是否包含毫秒 ".000"
     * @return LW_SUCCESS 成功，EC_ICV_INVALID_PARAM 参数错误，EC_ICV_COMM_BUFFERTOOSHORT 缓冲区不足
     */
    LWCOMM_API static int Timestamp2UtcString(int64_t timestamp_ms, char* buffer, size_t buf_len, bool include_ms = false);

    /**
     * @brief 获取当前时间戳（毫秒）
     * @return 当前时间戳（毫秒）
     */
    LWCOMM_API static int64_t GetCurrentTimestampMs();

    /**
     * @brief 获取当前时间字符串
     * @param buffer 输出缓冲区
     * @param buf_len 缓冲区长度
     * @param include_ms 是否包含毫秒
     * @return 错误码
     */
    LWCOMM_API static int GetCurrentTimeString(char* buffer, size_t buf_len, bool include_ms = false);

    /**
     * @brief 获取当前 UTC 时间字符串
     * @param buffer 输出缓冲区
     * @param buf_len 缓冲区长度
     * @param include_ms 是否包含毫秒
     * @return 错误码
     */
    LWCOMM_API static int GetCurrentUtcString(char* buffer, size_t buf_len, bool include_ms = false);
};

class LWBase64
{
private:
    LWBase64();
public:
    ~LWBase64();

    LWCOMM_API static int base64_encode(const char *in_str, int in_len, std::string &out_str);
    LWCOMM_API static int base64_decode(const char *in_str, int in_len, std::string &out_str);
};

class LWSystemInfo
{
public:
    /* Get Ip list */
    LWCOMM_API static int SysInfoIpList(std::list<std::string> &ip_list);

    /* Check Ip valid */
    LWCOMM_API static bool Ipv4Valid(std::string &ip);
};

/*
 * end
 */
