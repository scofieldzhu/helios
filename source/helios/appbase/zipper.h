/******************************************************** 
* author: scofieldzhu
* time:2026/2/27
*******************************************************/
#ifndef __zipper_h__
#define __zipper_h__

#include <string>
#include "helios/helios_nsp.h"

HELIOS_NAMESPACE_BEGIN

class Zipper
{
public:
    static bool CompressDirToZipFile(const std::string& target_zip_file, const std::string& source_dir, const std::string& pwd);
    static bool DecompressZipFileToDir(const std::string& target_dir, const std::string& source_zip_file, const std::string& pwd);
};

NAMESPACE_END

#endif