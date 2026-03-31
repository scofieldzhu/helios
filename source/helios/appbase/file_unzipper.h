#ifndef NP_fileunzipper_h__
#define NP_fileunzipper_h__
#include <string>
#include "zlib.h"
#include "unzip.h"

class FileUnzipper
{
public:
    FileUnzipper();
    ~FileUnzipper();

    void setZipPath(const std::string &path);
    void setPassword(const std::string &password);
    void setDestDir(const std::string &destDir);

    bool decompress();

    bool open();
    bool isOpen();
    bool close();

protected:
    bool do_extract();
    int do_extract_currentfile();
private:

    std::string zipPath_;
    unzFile zfile_ = nullptr;

    std::string destDir_;
    
    /// 解压文件时，当文件已经存在时，是否覆盖
    bool overwrite_ = true;
    /// 密码
    std::string password_;
    
    bool logFileDetail_ = false;

};

#endif // NP_fileunzipper_h__
