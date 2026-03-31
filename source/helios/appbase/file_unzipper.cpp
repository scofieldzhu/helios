#include "file_unzipper.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#else
# include <unistd.h>
# include <utime.h>
#endif

#include "unzip.h"



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

#ifdef __APPLE__
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#else
#define FOPEN_FUNC(f, filename, mode) fopen_s(f, filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (512)

#ifdef _WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif


#ifndef LOG_INFO
#define LOG_INFO printf("\nINFO:");printf
#define LOG_ERROR printf("\nERROR:");printf
#endif



/* change_file_date : change the date/time of a file
filename : the filename of the file where date/time must be modified
dosdate : the new date at the MSDos format (4 bytes)
tmu_date : the SAME new date at the tm_unz format */
void changeFileDate(const char *filename, uLong dosdate, tm_unz tmu_date)
{
#ifdef _WIN32
    HANDLE hFile;
    FILETIME ftm, ftLocal, ftCreate, ftLastAcc, ftLastWrite;

    hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL);
    GetFileTime(hFile, &ftCreate, &ftLastAcc, &ftLastWrite);
    DosDateTimeToFileTime((WORD)(dosdate >> 16), (WORD)dosdate, &ftLocal);
    LocalFileTimeToFileTime(&ftLocal, &ftm);
    SetFileTime(hFile, &ftm, &ftLastAcc, &ftm);
    CloseHandle(hFile);
#else
#ifdef unix || __APPLE__
    struct utimbuf ut;
    struct tm newdate;
    newdate.tm_sec = tmu_date.tm_sec;
    newdate.tm_min = tmu_date.tm_min;
    newdate.tm_hour = tmu_date.tm_hour;
    newdate.tm_mday = tmu_date.tm_mday;
    newdate.tm_mon = tmu_date.tm_mon;
    if (tmu_date.tm_year > 1900)
        newdate.tm_year = tmu_date.tm_year - 1900;
    else
        newdate.tm_year = tmu_date.tm_year;
    newdate.tm_isdst = -1;

    ut.actime = ut.modtime = mktime(&newdate);
    utime(filename, &ut);
#endif
#endif
}

int mkOneDir(const char* dirname)
{
    int ret = 0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#elif unix
    ret = mkdir(dirname, 0775);
#elif __APPLE__
    ret = mkdir(dirname, 0775);
#endif
    return ret;
}

/// 创建目录树
int makedir(const char *newdir)
{
    char *buffer;
    char *p;
    int  len = (int)strlen(newdir);

    if (len <= 0)
        return 0;

    len += 1;

    buffer = (char*)malloc(len);
    if (buffer == NULL)
    {
        LOG_ERROR("Error allocating memory, len:%d", len);
        return UNZ_INTERNALERROR;
    }

    strcpy_s(buffer, len, newdir);

    /// 如果最后一个字符是斜杆，则去掉
    if (buffer[len - 2] == '/' || buffer[len - 2] == '\\')
    {
        buffer[len - 2] = '\0';
    }

    if (mkOneDir(buffer) == 0)
    {
        free(buffer);
        return 1;
    }

    p = buffer + 1;
    while (1)
    {
        char hold;

        while (*p && *p != '\\' && *p != '/')
            p++;
        hold = *p;
        *p = 0;
        if ((mkOneDir(buffer) < 0 ) && (ENOENT == errno))
        {
            printf("couldn't create directory %s\n", buffer);
            free(buffer);
            return 0;
        }
        if (hold == 0)
            break;
        *p++ = hold;
    }
    free(buffer);
    return 1;
}

std::string joinPath(const std::string& path1, const std::string &path2)
{
    if (path1.empty())
    {
        return path2;
    }


    char lastChar = path1[path1.length() - 1];
    if (lastChar == '/' || lastChar == '\\')
    {
        return path1 + path2;
    }
    else
    {
        return path1 + "/" + path2;
    }
}

FileUnzipper::FileUnzipper()
{
    
}

FileUnzipper::~FileUnzipper()
{
    close();
}

void FileUnzipper::setZipPath(const std::string& path)
{
    zipPath_ = path;
}

void FileUnzipper::setPassword(const std::string &password)
{
    password_ = password;
}


void FileUnzipper::setDestDir(const std::string &destDir)
{
    destDir_ = destDir;
}

bool FileUnzipper::decompress()
{
    if (!isOpen())
    {
        if (!open())
        {            
            return false;
        }        
    }

    if (!destDir_.empty())
    {
        if (!makedir(destDir_.c_str()))
        {
            return false;
        }        
    }
    
    if (!do_extract())
    {
        return false;
    }

    LOG_INFO("decompress zipfile OK. zip:%s, destDir:%s"
                        , zipPath_.c_str()
                        , destDir_.c_str());

    return true;
}

bool FileUnzipper::open()
{
    close();

    if (zipPath_.empty())
    {
        LOG_ERROR("open zip failed. zipPath_ is empty!");
        return false;
    }

#ifdef USEWIN32IOAPI
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64A(&ffunc);
    zfile_ = unzOpen2_64(zipPath_.c_str(), &ffunc);
#else
    uf = unzOpen64(zipfilename);
#endif
    if (nullptr == zfile_)
    {
        LOG_ERROR("error opening %s", zipPath_.c_str());
        return false;
    }


    LOG_INFO("open zip: %s", zipPath_.c_str());

    return true;
}

bool FileUnzipper::isOpen()
{   
    return nullptr != zfile_;
}

bool FileUnzipper::close()
{
    if (nullptr == zfile_)
    {
        return true;
    }

    int err = unzClose(zfile_);
    if (err != UNZ_OK)
    {
        LOG_ERROR("error in closing %s", zipPath_.c_str());
    }

    zfile_ = nullptr;
    LOG_INFO("close zip: %s", zipPath_.c_str());

    return UNZ_OK == err;
}

bool FileUnzipper::do_extract()
{
    uLong i;
    unz_global_info64 gi;
    int err;
    FILE* fout = NULL;

    err = unzGetGlobalInfo64(zfile_, &gi);
    if (err != UNZ_OK)
    {
        LOG_ERROR("error %d with zipfile in unzGetGlobalInfo.", err);
    }

    for (i = 0; i < gi.number_entry; i++)
    {
        err = do_extract_currentfile();
        if (err != UNZ_OK)
            break;

        if ((i + 1) < gi.number_entry)
        {
            err = unzGoToNextFile(zfile_);
            if (err != UNZ_OK)
            {
                LOG_ERROR("error %d with zipfile in unzGoToNextFile", err);
                break;
            }
        }
    }

    return err == UNZ_OK;
}

#include <QString>

int FileUnzipper::do_extract_currentfile()
{
    char filename_inzip[MAXFILENAME] = {'\0'};
    char* filename_withoutpath;
    std::string filePathInDisk; /// 磁盘上的路径

    int err = UNZ_OK;
    
    unz_file_info64 file_info;
    uLong ratio = 0;
    err = unzGetCurrentFileInfo64(zfile_, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

    if (err != UNZ_OK)
    {
        LOG_ERROR("error %d with zipfile in unzGetCurrentFileInfo", err);
        return err;
    }

    // 判断文件名是否按 UTF-8 编码  added by scofieldzhu 2025/01/07
    bool isUtf8 = ((file_info.flag & (1 << 11)) != 0); // 检查 General Purpose Bit Flag 第 11 位  
    QString zfn;
    if (isUtf8)  
    {  
        zfn = QString::fromUtf8(filename_inzip, MAXFILENAME);
    }  
    else  
    {  
        zfn = QString::fromLocal8Bit(filename_inzip, MAXFILENAME);
    }  
    auto wstr = zfn.toStdWString();
    if(lstrcmpW(wstr.c_str(), L"..") == 0 || lstrcmpW(wstr.c_str(), L".") == 0){ //neglect this file.
        return UNZ_OK;
    }

    /// 得到磁盘上的文件路径
    filePathInDisk = joinPath(destDir_, filename_inzip);
        
    /// 得到文件名，不包含路径
    char* p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p) == '/') || ((*p) == '\\'))
            filename_withoutpath = p + 1;
        p++;
    }

    /// 目录，那么仅创建目录即可
    if ((*filename_withoutpath) == '\0')
    {
        if (logFileDetail_)
        {
            LOG_INFO("creating directory: %s", filename_inzip);
        }
        
        mkOneDir(filePathInDisk.c_str());
        return err;
    }
    
    bool skip = false; /// 是否跳过当前文件

    if (password_.empty())
    {
        err = unzOpenCurrentFilePassword(zfile_, NULL);
    }
    else
    {
        err = unzOpenCurrentFilePassword(zfile_, password_.c_str());
    }
       
    if (err != UNZ_OK)
    {
        LOG_ERROR("error %d with zipfile in unzOpenCurrentFilePassword, filename_inzip:%s", err, filename_inzip);
    }

    if (err == UNZ_OK)
    {
        char rep = 0;
        FILE* ftestexist = nullptr;
        FOPEN_FUNC(&ftestexist, filePathInDisk.c_str(), "rb");
        if (ftestexist != NULL)
        {
            fclose(ftestexist);
            
            if (!overwrite_)
            {
                skip = true;
            }
        }       
    }

    FILE *fout = nullptr;

    if ((!skip) && (err == UNZ_OK))
    {
        FOPEN_FUNC(&fout, filePathInDisk.c_str(), "wb");
        /// 如果打开失败，则判断是否需要创建目录
        if ((nullptr == fout) && (filename_withoutpath != (char*)filename_inzip))
        {
            std::string fileDirInZip;  ///zip中的文件目录（不含文件名）
            char c = *(filename_withoutpath - 1);
            *(filename_withoutpath - 1) = '\0';
            fileDirInZip = filename_inzip;
            *(filename_withoutpath - 1) = c;

            std::string dir = joinPath(destDir_, fileDirInZip);
            makedir(dir.c_str());
            
            FOPEN_FUNC(&fout, filePathInDisk.c_str(), "wb");
        }

        if (fout == NULL)
        {
            LOG_ERROR("error opening %s", filePathInDisk.c_str());
            err = UNZ_ERRNO;
        }
    }

    if (fout != NULL)
    {
        if (logFileDetail_)
        {
            LOG_INFO(" extracting: %s\n", filename_inzip);
        }        

        uInt size_buf = WRITEBUFFERSIZE;
        void* buf = (void*)malloc(size_buf);
        if (NULL == buf)
        {
            LOG_ERROR("Error allocating memory, size_buf:%d", size_buf);
            err =  UNZ_INTERNALERROR;
        }
        else
        {
            int byteNum = 0;

            do
            {
                /// return the number of byte copied if somes bytes are copied
                /// return 0 if the end of file was reached
                /// return < 0 with error code if there is an error
                byteNum = unzReadCurrentFile(zfile_, buf, size_buf);
                if (byteNum < 0)
                {
                    LOG_ERROR("error %d with zipfile in unzReadCurrentFile, filename_inzip:%s", byteNum, filename_inzip);
                    break;
                }
                if (byteNum > 0)
                {
                    if (fwrite(buf, byteNum, 1, fout) != 1)
                    {
                        LOG_ERROR("error in writing extracted file");
                        err = UNZ_ERRNO;
                        break;
                    }
                }
            } while (byteNum > 0);
        }
        
        if (fout)
        {
            fclose(fout);
        }

        if (UNZ_OK == err)
        {
            changeFileDate(filePathInDisk.c_str(), file_info.dosDate, file_info.tmu_date);
        }

        free(buf);
    }

    if (err == UNZ_OK)
    {
        err = unzCloseCurrentFile(zfile_);
        if (err != UNZ_OK)
        {
            LOG_ERROR("error %d with zipfile in unzCloseCurrentFile, filename_inzip:%s", err, filename_inzip);
        }
    }
    else
    {
        unzCloseCurrentFile(zfile_); /* don't lose the error */
    }
    
    
    return err;
}


