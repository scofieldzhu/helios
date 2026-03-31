#ifndef NP_filezipper_h__
#define NP_filezipper_h__
#include <string>
#include "zlib.h"
#include "zip.h"

class FileZipper
{
public:
    FileZipper();
    ~FileZipper();

    void setZipPath(const std::string &path);
    void setPassword(const std::string &password);
    void setCompressLevel(int compress_level);

    bool compressDir(const std::string& path);
    bool compressFile(const std::string& path);

    bool open();
    bool isOpen();
    bool close();

protected:
    bool collectfileInDirtoZip(const std::string& filePath, const std::string& parentDirName);
    bool addfiletoZip(const std::string& fileNameinZip, const std::string& srcfile);

    int getFileCrc(const char* filenameinzip, void*buf, unsigned long size_buf, unsigned long* result_crc);
    int isLargeFile(const char* filename);
private:

    std::string zipPath_;
    zipFile zfile_ = nullptr;

    /// 当zip存在时，是继续追加，还是重新创建
    bool append_ = false;

    /// 压缩后是否保留文件的路径
    bool exclude_path_ = true;

    /// 密码
    std::string password_;
    
    /// -0  Store only
    /// -1  Compress faster
    /// -9  Compress better
    int compress_level_ = 3;

    bool logFileDetail_ = false;

};

#endif // NP_filezipper_h__
