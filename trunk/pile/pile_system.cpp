/*
Pile, a truly cross-platform automatic build tool.
--------------------------------------------------

pile_system.cpp

Copyright Jonathan Dearborn 2009

Licensed under the GNU Public License (GPL)
See COPYING.txt

This file contains all system-specific functions used in Pile.
*/

#include "pile_global.h"


#ifdef PILE_WIN32

#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0500
#include <windows.h>
#include <shlobj.h>

#endif

#ifdef PILE_LINUX
/*#include <Xm/Xm.h>
#include <Xm/PushB.h>*/
#endif

#include "pile_ui.h"


// Motif stuff from http://stackoverflow.com/questions/1384125/c-messagebox-for-linux-like-in-ms-windows
/*void pushed_fn(Widget w, XtPointer client_data,
               XmPushButtonCallbackStruct *cbs)
  {
     printf("Don't Push Me!!\n");
  }*/

void SYS_alert(const char* text)
{
    #ifdef PILE_WIN32
    ;
    #endif
    #ifdef PILE_LINUX
    /*Widget top_wid, button;
    XtAppContext  app;

    top_wid = XtVaAppInitialize(&app, "Push", NULL, 0,
        &argc, argv, NULL, NULL);

    button = XmCreatePushButton(top_wid, "Push_me", NULL, 0);

    // tell Xt to manage button
                        XtManageChild(button);

                        // attach fn to widget
    XtAddCallback(button, XmNactivateCallback, pushed_fn, NULL);

    XtRealizeWidget(top_wid); // display widget hierarchy
    XtAppMainLoop(app); // enter processing loop
    */
    #endif
}

string getSystemName()
{
    #ifdef PILE_WIN32
    return "Win32";
    #endif
    #ifdef PILE_LINUX
    return "Linux";
    #endif
    return "Unknown";
}


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
    string tempname = ".pile.tmp";
    command += " >> " + tempname + " 2>&1";

    #ifdef PILE_LINUX
    int result = system(command.c_str());
    if(result != 0)
        UI_warning("System call signalled failure with value %d: '%s'\n", result, command.c_str());
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
