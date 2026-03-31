/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/25
*******************************************************/
#include "path_util.h"
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <QDir>
#include "mirfak/basic/log_service.h"

MIRFAK_NAMESPACE_BEGIN

static void DoRmTree(const QString &path, bool remove_self)
{
    QDir dir(path);
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); 
    QFileInfoList fileList = dir.entryInfoList(); 
    foreach(QFileInfo file, fileList) {
        QString fp = file.filePath(); 
        if(file.isFile()){ 
            file.dir().remove(file.fileName());
        }else{ 
            DoRmTree(file.absoluteFilePath(), remove_self);
        }
    }
    if(remove_self){
        dir.rmdir(dir.absolutePath());
        if(dir.exists()){
            SPDLOG_ERROR("Remove directory:\"{}\" failed!", QStrToLogStr(dir.absolutePath()));
            return;
        }
    }
}

bool path_util::RemoveTree(const QString& path, bool remove_self)
{
    if(path.isEmpty()){
        return false;
    }
    QDir dir(path);
    if(!dir.exists()) {
        return true;
    }
    DoRmTree(path, remove_self);
    if(remove_self){
        dir.rmdir(dir.absolutePath());
        if(dir.exists()){
            SPDLOG_ERROR("Remove directory:\"{}\" failed!", QStrToLogStr(dir.absolutePath()));
            return false;
        }
    }
    return true; 
}

QStringList path_util::SearchDirFiles(const QString& dir, const QStringList& required_extension_names)
{
    QDir root_dir(dir);
    if(!root_dir.exists()){
        SPDLOG_WARN("\"{}\" is not existing directory!", QStrToLogStr(dir));
        return {};
    }
    root_dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); 
    QStringList result_filenames;
    for(const auto& file_info : root_dir.entryInfoList()) { 
        if(!file_info.isFile()) { 
            continue;
        }
        auto suffix = file_info.suffix();
        auto it = std::find(required_extension_names.begin(), required_extension_names.end(), suffix);
        if(it != required_extension_names.end()){
            result_filenames.push_back(file_info.fileName());
        }
    }
    return result_filenames;
}

void path_util::SetDirHidden(const QString& dir)
{
    QFileInfo folder(dir);  
    if(!folder.exists() || !folder.isDir()) {  
        SPDLOG_WARN("Folder:\"{}\" not exist or can't access!", QStrToLogStr(dir));  
        return;  
    }  
    DWORD attributes = GetFileAttributesA(dir.toLocal8Bit().constData());  
    if(attributes == INVALID_FILE_ATTRIBUTES) {  
        SPDLOG_WARN("Error retrieving folder:\"{}\" attributes, error code:{}!", QStrToLogStr(dir), GetLastError());  
        return;  
    }  
    if(!(attributes & FILE_ATTRIBUTE_HIDDEN)) {  
        attributes |= FILE_ATTRIBUTE_HIDDEN; 
        if(SetFileAttributesA(dir.toLocal8Bit().constData(), attributes)) {  
            return;  
        }else{  
            SPDLOG_WARN("Error setting folder:\"{}\" attributes, error code:{}!", QStrToLogStr(dir), GetLastError());  
            return;  
        }  
    }  
}

NAMESPACE_END
