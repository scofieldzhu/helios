/******************************************************** 
* author: scofieldzhu
* time:2026/2/27
*******************************************************/
#include "zipper.h"
#include <zip.h>
#include <filesystem>
#include <fstream>
#include "helios/basic/log_service.h"

namespace fs = std::filesystem;

HELIOS_NAMESPACE_BEGIN

namespace{
    bool AddDirToZipFile(zip_t* za, const fs::path& root, const std::string& password)
    {
        if(!fs::exists(root) || !fs::is_directory(root)) {
            SPDLOG_ERROR("Source directory not exists or not a directory! \"{}\"", root.generic_string());
            return false;
        }
        for(const auto& entry : fs::recursive_directory_iterator(root)) {
            fs::path full_path = entry.path();
            fs::path relative_path = fs::relative(full_path, root);
            std::string zip_path = relative_path.generic_string();
            if(entry.is_directory()) {
                if(!zip_path.empty() && zip_path.back() != '/'){
                    zip_path += '/';
                }
                if(zip_dir_add(za, zip_path.c_str(), ZIP_FL_ENC_UTF_8) < 0){
                    SPDLOG_ERROR("Add directory:\"{}\" failed! error:{}", zip_path, zip_strerror(za));
                    return false;
                }
            }else if(entry.is_regular_file()){
                zip_source_t* source = zip_source_file(za, full_path.string().c_str(), 0, 0);
                if(source == nullptr){
                    SPDLOG_ERROR("Create file source:\"{}\" failed, error:{}!", full_path.generic_string(), zip_strerror(za));
                    return false;
                }
                auto file_index = zip_file_add(za, zip_path.c_str(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
                if(file_index < 0) {
                    SPDLOG_ERROR("Add file failed:\"{}\" error:{}", zip_path, zip_strerror(za));
                    zip_source_free(source);
                    return false;
                }
                if(zip_set_file_compression(za, file_index, ZIP_CM_DEFLATE, 6) < 0){
                    SPDLOG_ERROR("Set compress method failed! path:\"{}\", error:{}.", zip_path, zip_strerror(za));
                    return false;
                }
                if(!password.empty()){
                    if(zip_file_set_encryption(za, file_index, ZIP_EM_AES_256, password.c_str()) < 0) {
                        SPDLOG_ERROR("Set encryption failed! path:\"{}\", error:{}.", zip_path, zip_strerror(za));
                        return false;
                    }
                }                
            }
        }
        return true;
    }

    bool CreateEncyptedZipFile(const fs::path& source_dir, const fs::path& zip_filepath, const std::string& password)
    {
        int error = 0;
        auto za = zip_open(zip_filepath.string().c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
        if(za == nullptr) {
            zip_error_t zip_err;
            zip_error_init_with_code(&zip_err, error);
            SPDLOG_ERROR("Open zip file:\"{}\" failed! error:{}", zip_filepath.string(), zip_error_strerror(&zip_err));
            zip_error_fini(&zip_err);
            return false;
        }
        if(!AddDirToZipFile(za, source_dir, password)) {
            zip_discard(za);
            return false;
        }
        if(zip_close(za) < 0) {
            SPDLOG_ERROR("Close zip failed! error:{}", zip_strerror(za));
            zip_discard(za);
            return false;
        }
        return true;
    }

    bool WriteFileToLocal(const fs::path& local_file_path, std::vector<char>& buffer, zip_uint64_t file_index, zip_t* za, zip_stat_t* st)
    {
        zip_file_t* zf = zip_fopen_index(za, file_index, 0);
        if(zf == nullptr) {
            SPDLOG_ERROR("zip_fopen_index failed: ", st->name);            
            return false;
        }
        std::ofstream ofs(local_file_path, std::ios::binary);
        if(!ofs){
            SPDLOG_ERROR("open output file failed: ", local_file_path.generic_string());
            zip_fclose(zf);
            return false;
        }
        while(true){
            auto n = zip_fread(zf, buffer.data(), buffer.size());
            if(n < 0){
                SPDLOG_ERROR("zip_fread failed: ", st->name);
                zip_fclose(zf);
                return false;
            }
            if(n == 0) {
                break;
            }
            ofs.write(buffer.data(), n);
            if(!ofs){
                SPDLOG_ERROR("write output file failed: ", local_file_path.generic_string());
                zip_fclose(zf);
                return false;
            }
        }
        zip_fclose(zf);
        return true;
    }

    bool UnzipToDir(const fs::path& zip_path, const fs::path& output_dir, const std::string& password) {
        int err = 0;
        auto za = zip_open(zip_path.string().c_str(), ZIP_RDONLY, &err);
        if(za == nullptr){
            SPDLOG_ERROR("zip_open failed! file path:\"{}\"", zip_path.generic_string());
            return false;
        }
        if(!password.empty()) {
            if(zip_set_default_password(za, password.c_str()) != 0) {
                SPDLOG_ERROR("zip_set_default_password failed!");
                zip_close(za);
                return false;
            }
        }
        try{
            fs::create_directories(output_dir);
            std::vector<char> buffer(64 * 1024);
            for(zip_uint64_t i = 0; i < static_cast<zip_uint64_t>(zip_get_num_entries(za, 0)); ++i) {
                zip_stat_t st;
                zip_stat_init(&st);
                if(zip_stat_index(za, i, 0, &st) != 0) {
                    SPDLOG_ERROR("zip_stat_index failed: i = {}", i);
                    zip_close(za);
                    return false;
                }
                if(st.name == nullptr) {
                    SPDLOG_ERROR("entry name is null, i = ", i);
                    zip_close(za);
                    return false;
                }
                fs::path relative_path = fs::path(st.name).lexically_normal();
                // 防止解压穿越到目标目录之外，例如 ../../xxx
                if(relative_path.is_absolute() || st.name[0] == '/' || st.name[0] == '\\') {
                    SPDLOG_ERROR("invalid entry path: ", st.name);
                    zip_close(za);
                    return false;
                }
                fs::path out_path = output_dir / relative_path;
                std::string name = st.name;
                bool is_dir = !name.empty() && name.back() == '/';
                if(is_dir) {
                    fs::create_directories(out_path);
                    continue;
                }
                if(out_path.has_parent_path()) {
                    fs::create_directories(out_path.parent_path());
                }
                if(!WriteFileToLocal(out_path, buffer, i, za, &st)){
                    zip_close(za);
                    return false;
                }                
            }
            if(zip_close(za) != 0) {
                SPDLOG_ERROR("zip_close failed!");
                return false;
            }
            return true;
        }catch(...){
            zip_discard(za);
            throw;
        }
    }
}

bool Zipper::CompressDirToZipFile(const std::string& target_zip_file, const std::string& source_dir, const std::string& pwd)
{
    return CreateEncyptedZipFile(fs::u8path(source_dir), fs::u8path(target_zip_file), pwd);
}

bool Zipper::DecompressZipFileToDir(const std::string& target_dir, const std::string& source_zip_file, const std::string& pwd)
{
    return UnzipToDir(source_zip_file, target_dir, pwd);
}

NAMESPACE_END
