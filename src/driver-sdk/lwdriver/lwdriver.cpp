/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: lwdriver.cpp .
*
* Date: 2025-05-28
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include <cerrno>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <dlfcn.h>

#include <vector>
#include <sstream>

using namespace std;

#define LOAD_LIBRARY_FAILED -1
#define EXPORT_SYMBOL_FROM_LIBRARY_FAILED -2
#define MAX_FILE_PATH 1024

#ifdef WIN32
#define CVDRIVER_COMMON_DLL_NAME "lwdrvcmn"
#define OS_DIR_SEP_STR		"\\"
#define OS_DIR_SEP_CHAR		'\\'
#else
#define CVDRIVER_COMMON_DLL_NAME "liblwdrvcmn.so"
#define OS_DIR_SEP_STR		"/"
#define OS_DIR_SEP_CHAR		'/'
#endif

extern "C" int drvMain(char *szDrvName, char *szDrvPath);
typedef int(*PFN_Main)(const char* szDrvName, const char *szDrvDir);

std::string process_name;

int ExitWithWait(int nCode)
{
#ifdef WIN32
	Sleep(5000);
#else
	sleep(5); // 5 seconds
#endif

	return nCode;
}

/*
 * Get full path of the driver executable.
 * This function retrieves the full path of the currently running driver executable that loaded.
 */
int GetDrvFullPath(string &strDrvFullPath)
{
	char szDrvExePath[MAX_FILE_PATH] = { 0 };
#if defined SYLIXOS
    pid_t pid = getpid();
    char szExefile[MAX_FILE_PATH] = {0};
    sprintf(szExefile, "/proc/%d/exe", pid);
    readlink(szExefile, szDrvExePath, sizeof(szDrvExePath));
#elif defined _WIN32
    GetModuleFileName(NULL, szDrvExePath, sizeof(szDrvExePath));
#else
    readlink("/proc/self/exe", szDrvExePath, sizeof(szDrvExePath));
#endif
	strDrvFullPath = szDrvExePath;
	return 0;
}

/* 
 * Get the libdrvcmn.dll path by the driver full path.
 */
int GetDrvCmnDir(string strDrvFullPath, string &strDrvCmdDir)
{
	strDrvCmdDir = strDrvFullPath;
	int nPos = strDrvCmdDir.find_last_of(OS_DIR_SEP_CHAR);
	if (nPos >= 0)
		strDrvCmdDir = strDrvCmdDir.substr(0, nPos);
	strDrvCmdDir += OS_DIR_SEP_STR;
	strDrvCmdDir += "..";
	strDrvCmdDir += OS_DIR_SEP_STR;
	strDrvCmdDir += "..";
	strDrvCmdDir += OS_DIR_SEP_STR;
	strDrvCmdDir += "..";
	return 0;
}

/*
 * Get the driver directory.
 */
int GetDriverDir(string strDrvExeFullPath, string & strDriverDir)
{
	strDriverDir = strDrvExeFullPath;
	std::size_t pos = strDriverDir.rfind(OS_DIR_SEP_STR);
	if (pos != string::npos) {
		strDriverDir = strDriverDir.substr(0, pos);
    }
	return 0;
}


/*
 * Get the driver name from the full path.
 * The driver name is the file name without extension.
 */
int GetDrvName(string strDrvFullPath, string &strDrvName)
{
	strDrvName = strDrvFullPath;
	size_t nPos = std::string::npos;
	if ((nPos = strDrvName.find_last_of(OS_DIR_SEP_CHAR)) != string::npos)
		strDrvName = strDrvName.substr(nPos + 1);

	if ((nPos = strDrvName.find_last_of('.')) != string::npos)
		strDrvName = strDrvName.substr(0, nPos);

	return 0;
}
// 拆分字符串函数
std::vector<std::string> split(const std::string &str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

bool IsPathInEnv(const std::string &path, const std::string &env) {
	std::vector<std::string> paths = split(env, ':');
	for (const auto& p : paths) {
		if (p == path) {
			return true;
		}
	}
	return false;
}

int SetWorkspaceEnv (std::vector<std::string> &libdirs)
{
	string strSysPath;

	strSysPath = getenv("LD_LIBRARY_PATH") == nullptr ? "" : getenv("LD_LIBRARY_PATH");
	if (strSysPath.empty()) {
		for (const auto& lib_path : libdirs) {
			strSysPath = strSysPath + lib_path + ":";
		}
		strSysPath = strSysPath.substr(0, strSysPath.length() - 1); // 去掉最后的冒号
	} else {
		for (const auto& lib_path : libdirs) {
			if (!IsPathInEnv(lib_path, strSysPath)) {
				strSysPath = lib_path + ":" + strSysPath;
			}
		}
	}

	setenv("LD_LIBRARY_PATH", strSysPath.c_str(), 1);
	const char* env_tmp = getenv("LD_LIBRARY_PATH");
	return 0;
}

int main(int argc, char** argv)
{
	process_name = argv[0];
	std::string drv_exec_fullpath, drv_dir, drv_name, drv_fullpath;
	std::string homedir, bindir, libdir;
	GetDrvFullPath(drv_exec_fullpath); // full path of exe. if is symbol link, then to final file
	printf("Driver exec full path is %s\n", drv_exec_fullpath.c_str());
	GetDriverDir(drv_exec_fullpath, drv_dir);		// driver path
    printf("Driver dir is %s\n", drv_dir.c_str());
	GetDrvCmnDir(drv_exec_fullpath, homedir);	// drvcmn.dll path
	bindir = homedir + OS_DIR_SEP_STR + "bin";
	libdir = homedir + OS_DIR_SEP_STR + "lib";
	GetDrvName(drv_exec_fullpath, drv_name);
	printf("Driver name is %s\n", drv_name.c_str());
	drv_fullpath = libdir + OS_DIR_SEP_STR + CVDRIVER_COMMON_DLL_NAME;
	printf("Driver full path is %s\n", drv_fullpath.c_str());

	char szOldCurDir[1024] = { 0 };
	getcwd(szOldCurDir, sizeof(szOldCurDir));

	std::vector<std::string> libdirs({
		libdir
	});
	SetWorkspaceEnv(libdirs);
	
	chdir(bindir.c_str());

	// void* hDrvComonDll = dlopen(strDrvCmnPath.c_str(), RTLD_LAZY | RTLD_NOLOAD);
	printf("Loading driver common library: %s\n", drv_fullpath.c_str());
	if (access(drv_fullpath.c_str(), F_OK) != 0) {
		fprintf(stderr, "Driver common library %s not found!\n", drv_fullpath.c_str());
		return ExitWithWait(LOAD_LIBRARY_FAILED);
	}

	// Load the driver common library
	void* hDrvComonDll = dlopen(drv_fullpath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (hDrvComonDll == NULL)
    {
        fprintf(stderr, "Load %s failed!, %s\n", drv_fullpath.c_str(), dlerror());
        return ExitWithWait(LOAD_LIBRARY_FAILED);
    }
	printf("Load %s successfull!\n", drv_fullpath.c_str());

	PFN_Main pfnMain = (PFN_Main)dlsym(hDrvComonDll, "drvMain");
	if (pfnMain == NULL)
	{
		printf("Driver base: %s, failed to Get Function:drvMain\n", drv_fullpath.c_str());
		return ExitWithWait(EXPORT_SYMBOL_FROM_LIBRARY_FAILED);
	}

	pfnMain(drv_name.c_str(), drv_dir.c_str());
	dlclose(hDrvComonDll);
    chdir(szOldCurDir); // restore current work dir
	return 0;
};
