#include "file_zipper.h"


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <Shlwapi.h> 
#include "unzip.h"

#pragma comment(lib, "Shlwapi.lib")



#if (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifndef _FILE_OFFSET_BIT
#define _FILE_OFFSET_BIT 64
#endif
#endif


#define WRITEBUFFERSIZE (16384)



#ifdef _WIN32
/* f : name of file to get info on */
/* tmzip : return value: access, modific. and creation times */
/* dt: dostime */
uLong filetime(const char *f, tm_zip *tmzip, uLong *dt)
{
    int ret = 0;
    {
        FILETIME ftLocal;
        HANDLE hFind;
        WIN32_FIND_DATAA ff32;

        hFind = FindFirstFileA(f, &ff32);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            FileTimeToLocalFileTime(&(ff32.ftLastWriteTime), &ftLocal);
            FileTimeToDosDateTime(&ftLocal, ((LPWORD)dt) + 1, ((LPWORD)dt) + 0);
            FindClose(hFind);
            ret = 1;
        }
    }
    return ret;
}
#else
#ifdef unix || __APPLE__
uLong filetime(f, tmzip, dt)
const char *f;               /* name of file to get info on */
tm_zip *tmzip;         /* return value: access, modific. and creation times */
uLong *dt;             /* dostime */
{
    int ret = 0;
    struct stat s;        /* results of stat() */
    struct tm* filedate;
    time_t tm_t = 0;

    if (strcmp(f, "-") != 0)
    {
        char name[MAXFILENAME + 1];
        int len = strlen(f);
        if (len > MAXFILENAME)
            len = MAXFILENAME;

        strncpy(name, f, MAXFILENAME - 1);
        /* strncpy doesnt append the trailing NULL, of the string is too long. */
        name[MAXFILENAME] = '\0';

        if (name[len - 1] == '/')
            name[len - 1] = '\0';
        /* not all systems allow stat'ing a file with / appended */
        if (stat(name, &s) == 0)
        {
            tm_t = s.st_mtime;
            ret = 1;
        }
    }
    filedate = localtime(&tm_t);

    tmzip->tm_sec = filedate->tm_sec;
    tmzip->tm_min = filedate->tm_min;
    tmzip->tm_hour = filedate->tm_hour;
    tmzip->tm_mday = filedate->tm_mday;
    tmzip->tm_mon = filedate->tm_mon;
    tmzip->tm_year = filedate->tm_year;

    return ret;
}
#else
uLong filetime(f, tmzip, dt)
char *f;                /* name of file to get info on */
tm_zip *tmzip;             /* return value: access, modific. and creation times */
uLong *dt;             /* dostime */
{
    return 0;
}
#endif
#endif

#ifndef LOG_INFO
#define LOG_INFO printf("\nINFO:");printf
#define LOG_ERROR printf("\nERROR:");printf
#endif

FileZipper::FileZipper()
{
    
}

FileZipper::~FileZipper()
{
    close();
}

void FileZipper::setZipPath(const std::string& path)
{
    zipPath_ = path;
}

void FileZipper::setPassword(const std::string &password)
{
    password_ = password;
}

void FileZipper::setCompressLevel(int compress_level)
{
    compress_level_ = compress_level;
}

bool FileZipper::open()
{
    close();

    if (zipPath_.empty())
    {
        LOG_ERROR("open zip failed. zipPath_ is empty!");
        return false;
    }

    bool isAppend = false;
    if (::PathFileExistsA(zipPath_.c_str()))
    {
        LOG_INFO("zip file already exists: %s", zipPath_.c_str());
        isAppend = append_;
    }

#ifdef USEWIN32IOAPI
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64A(&ffunc);
    zfile_ = zipOpen2_64(zipPath_.c_str(), (opt_overwrite == 2) ? 2 : 0, NULL, &ffunc);
#else
    zfile_ = zipOpen64(zipPath_.c_str(), isAppend ? APPEND_STATUS_ADDINZIP : APPEND_STATUS_CREATE);
#endif

    if (nullptr == zfile_)
    {
        LOG_ERROR("error opening %s", zipPath_.c_str());
        return false;
    }

    LOG_INFO("open zip: %s", zipPath_.c_str());

    return true;
}

bool FileZipper::isOpen()
{   
    return nullptr != zfile_;
}

bool FileZipper::close()
{
    if (nullptr == zfile_)
    {
        return true;
    }

    int err = zipClose(zfile_, NULL);
    if (err != ZIP_OK)
    {
        LOG_ERROR("error in closing %s", zipPath_.c_str());
    }

    zfile_ = nullptr;
    LOG_INFO("close zip: %s", zipPath_.c_str());

    return ZIP_OK == err;
}


bool FileZipper::compressDir(const std::string& path)
{
    if (!isOpen())
    {
        if (!open())
        {
            LOG_ERROR("open zip failed.");
            return false;
        }        
    }

    bool isOK = collectfileInDirtoZip(path, "");

    LOG_INFO("compressDir OK, path: %s", path.c_str());
    return isOK;
}


bool FileZipper::collectfileInDirtoZip(const std::string& filePath, const std::string& parentDirName)
{
    if (NULL == zfile_ || filePath.empty())
    {
        LOG_ERROR("zfile is null or filePath is empty.");
        return false;
    }
   
    bool isFile = false;
    std::string relativepath = "";
    WIN32_FIND_DATAA findFileData;

    std::string szpath(filePath);

    if (::PathIsDirectoryA(filePath.c_str()))
    {
        szpath += "/*.*";             
    }
    else
    {
        isFile = true;        
    }

    HANDLE hFile = ::FindFirstFileA(szpath.c_str(), &findFileData);
    if (NULL == hFile)
    {
        LOG_ERROR("FindFirstFileA failed. szpath:%s", szpath.c_str());
        return false;
    }
    do
    {
        if (parentDirName.empty())
        {
            relativepath = findFileData.cFileName;
        }
        else
        {
            relativepath = parentDirName + "/" + findFileData.cFileName;//生成zip文件中的相对路径
        }

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0)
            {
                addfiletoZip(relativepath, "");

                std::string szTemp(filePath);
                szTemp += "/";
                szTemp += findFileData.cFileName;

                collectfileInDirtoZip(szTemp, relativepath);
            }

            continue;
        }

        std::string szTemp(filePath);
        if (!isFile)
        {           
            // 处理目录文件的压缩
            szTemp += "/";
            szTemp += findFileData.cFileName;
        }

        addfiletoZip(relativepath, szTemp);

    } while (::FindNextFileA(hFile, &findFileData));
    FindClose(hFile);
    return true;
}


/*
* 函数功能 :解压zip文件
* 参数strFilePath表示zip压缩文件的路径
* 参数strTempPath表示要解压到的文件目录
*/
bool FileZipper::addfiletoZip(const std::string& fileNameinZip, const std::string& srcfile)
{
    int err = ZIP_OK;
    zip_fileinfo zi;
    unsigned long crcFile = 0;
    int zip64 = 0;

    zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
        zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
    zi.dosDate = 0;
    zi.internal_fa = 0;
    zi.external_fa = 0;
    filetime(srcfile.c_str(), &zi.tmz_date, &zi.dosDate);

    std::string newFileNameInZip = fileNameinZip;

    if (srcfile.empty()) /// 表示是目录
    {
        newFileNameInZip += "\\";
        
        err = zipOpenNewFileInZip(zfile_, newFileNameInZip.c_str(), &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
        if (err != ZIP_OK)
        {
            return false;
        }

        zipCloseFileInZip(zfile_);
        return true;
    }
    

    int size_buf = WRITEBUFFERSIZE;

    void *buf = (void*)malloc(size_buf);
    if (buf == NULL)
    {
        LOG_ERROR("Error allocating memory, size_buf: %d", size_buf);
        return false;
    }
    
    FILE * fin = nullptr;
    int size_read;   

    

    /*
    err = zipOpenNewFileInZip(zf,filenameinzip,&zi,
    NULL,0,NULL,0,NULL / * comment * /,
    (opt_compress_level != 0) ? Z_DEFLATED : 0,
    opt_compress_level);
    */
    char *password = nullptr;
    if (!password_.empty())
    {
        err = getFileCrc(srcfile.c_str(), buf, size_buf, &crcFile);

        password = new char[password_.length() + 1];
        memset(password, 0, password_.length() + 1);
        strcpy_s(password, password_.length() + 1, password_.c_str());
    }

    zip64 = isLargeFile(srcfile.c_str());
    
    /**/
    err = zipOpenNewFileInZip3_64(zfile_, fileNameinZip.c_str(), &zi,
        NULL, 0, NULL, 0, NULL /* comment*/,
        (compress_level_ != 0) ? Z_DEFLATED : 0,
        compress_level_, 0,
        /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
        -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
        password, crcFile, zip64);

    if (err != ZIP_OK)
    {
        LOG_ERROR("error in opening %s in zipfile", fileNameinZip.c_str());
    }
    else
    {
        err = fopen_s(&fin, srcfile.c_str(), "rb");
        if (NULL == fin)
        {
            err = ZIP_ERRNO;
            LOG_ERROR("error in opening %s for reading, err:%d", srcfile.c_str(), err);
        }
    }

    if (ZIP_OK == err)
    {
        do
        {
            err = ZIP_OK;
            size_read = (int)fread(buf, 1, size_buf, fin);
            if (size_read < size_buf)
                if (feof(fin) == 0)
                {
                    LOG_ERROR("error in reading %s", srcfile.c_str());
                    err = ZIP_ERRNO;
                }

            if (size_read > 0)
            {
                err = zipWriteInFileInZip(zfile_, buf, size_read);
                if (err < 0)
                {
                    LOG_ERROR("error in writing %s in the zipfile", srcfile.c_str());
                }

            }
        } while ((ZIP_OK == err) && (size_read > 0));
    }

    if (fin)
    {
        fclose(fin);
    }

    if (err < 0)
    {
        err = ZIP_ERRNO;
    }
    else
    {
        err = zipCloseFileInZip(zfile_);
        if (err != ZIP_OK)
        {
            LOG_ERROR("error in closing %s in the zipfile", srcfile.c_str());
        }
    }

    free(buf);

    if (password)
    {
        delete []password;
    }

    if (logFileDetail_)
    {
        LOG_INFO("add file to zip OK, fileNameinZip:%s, srcfile:%s", fileNameinZip.c_str(), srcfile.c_str());
    }
    return ZIP_OK == err;
}

/* calculate the CRC32 of a file,
because to encrypt a file, we need known the CRC32 of the file before */
int FileZipper::getFileCrc(const char* srcFilePath, void*buf, unsigned long size_buf, unsigned long* result_crc)
{
    unsigned long calculate_crc = 0;
    int err = ZIP_OK;
    FILE * fin = nullptr;
    err = fopen_s(&fin, srcFilePath, "rb");
    if (err != 0)
    {
        LOG_ERROR("fopen_s failed. srcFilePath:%s", srcFilePath);
        return err;
    }

    unsigned long size_read = 0;
    unsigned long total_read = 0;
    if (fin == NULL)
    {
        err = ZIP_ERRNO;
    }

    if (err == ZIP_OK)
        do
        {
            err = ZIP_OK;
            size_read = (int)fread(buf, 1, size_buf, fin);
            if (size_read < size_buf)
                if (feof(fin) == 0)
                {
                    LOG_ERROR("error in reading %s", srcFilePath);
                    err = ZIP_ERRNO;
                }

            if (size_read>0)
                calculate_crc = crc32(calculate_crc, (Bytef *)buf, size_read);
            total_read += size_read;

        } while ((err == ZIP_OK) && (size_read>0));

        if (fin)
            fclose(fin);

        *result_crc = calculate_crc;
        if (logFileDetail_)
        {
            LOG_INFO("file %s crc %lx", srcFilePath, calculate_crc);
        }

        return err;
}


int FileZipper::isLargeFile(const char* filename)
{
    int largeFile = 0;
    ZPOS64_T pos = 0;
    FILE* pFile = nullptr;
    fopen_s(&pFile, filename, "rb");

    if (pFile != NULL)
    {
        int n = fseeko64(pFile, 0, SEEK_END);
        pos = ftello64(pFile);

        if (logFileDetail_)
        {
            LOG_INFO("File : %s is %lld bytes", filename, pos);
        }

        if (pos >= 0xffffffff)
            largeFile = 1;

        fclose(pFile);
    }

    return largeFile;
}

