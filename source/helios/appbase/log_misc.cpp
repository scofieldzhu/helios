/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/1/5
*******************************************************/
#include "log_misc.h"
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <vtkObject.h>
#include <sstream>
#include <vtkIndent.h>

MIRFAK_NAMESPACE_BEGIN

bool InVSDebugger()
{
    return ::IsDebuggerPresent() != FALSE;
}

std::string QStrToLogStr(const QString& qstr)
{
    if(InVSDebugger()){
        return qstr.toLocal8Bit().toStdString();
    }
    return qstr.toUtf8().toStdString();
}

NAMESPACE_END