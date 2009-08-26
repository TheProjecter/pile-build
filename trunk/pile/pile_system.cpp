/*
System-specific functions
*/

#include "pile_global.h"


#ifdef PILE_WIN32

#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0500
#include <windows.h>
#include <shlobj.h>

#endif





string getHomeDir()
{
    #ifdef PILE_WIN32
    char path[PATH_MAX];
    
    if(SHGetSpecialFolderPath(HWND_DESKTOP, path, CSIDL_PERSONAL, FALSE))
        return path;
    
    // Error...
    
    return "";
    #endif
    
    #ifdef PILE_LINUX
    return getenv("HOME");
    #endif
}


void convertSlashes(string& str)
{
    #ifdef PILE_WIN32
    size_t i = str.find('/');
    while(i != string::npos)
    {
        str[i] = '\\';
        i = str.find('/');
    }
    #endif
}


void systemCall(string command)
{
    // Append stdout and stderr to file
    string tempname = "pile.tmp";
    command += " >> " + tempname + " 2>&1";
    
    #ifdef PILE_LINUX
    system(command.c_str());
    #endif
    
    #ifdef PILE_WIN32
    //ShellExecute(NULL, "open", "cmd.exe", ("/C " + command).c_str(), NULL, SW_HIDE);
    //delay(100);
    char buff[command.size() + 10];
    
    sprintf(buff, "%s", ("/C " + command).c_str());
    
    SHELLEXECUTEINFO ShExecInfo = {0};
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = "open";
    ShExecInfo.lpFile = "cmd.exe";
    ShExecInfo.lpParameters = buff;
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_HIDE;
    ShExecInfo.hInstApp = NULL;
    
    ShellExecuteEx(&ShExecInfo);
    
    WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
    #endif
}

void delay(unsigned int milliseconds)
{
    #ifdef PILE_WIN32
    Sleep(milliseconds);
    #endif
}
