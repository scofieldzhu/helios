/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/12/25
*******************************************************/
#include "project_package.h"
#include <QFileInfo>
#include <QDir>
#include <QUuid>
#include "file_zipper.h"
#include "file_unzipper.h"
#include "path_util.h"
#include "default_image_data_serializer.h"
#include "default_poly_data_serializer.h"
#include "mirfak/basic/log_service.h"
#include "model_element_serializer.h"

MIRFAK_NAMESPACE_BEGIN

namespace 
{    
    const QString kElementFileName = "model_element";
    const QString kVolumeDataFileName = "volume_data";
    const QString kElementFileSuffixName = "db";
    const QString kImageFileSuffixName = "img";
    const QString kPolyFileSuffixName = "plys";

    enum FileType
    {
        FT_IMAGE,
        FT_POLYS,
        FT_ELEMENT        
    };

    inline QString GetFileName(const QString& fn, FileType ft)
    {
        QString ext;
        if(ft == FT_IMAGE){
            ext = kImageFileSuffixName;
        }else if(ft == FT_POLYS){
            ext = kPolyFileSuffixName;
        }else if(ft == FT_ELEMENT){
            ext = kElementFileSuffixName;
        }
        return fn + "." + ext;
    }
}

ProjectPackage::ProjectPackage(ModelElementSerializer& serializer, const QString& temp_work_dir, const QString& pwd)
    :model_elem_serializer_(&serializer),
    kTempWorkDir_(temp_work_dir),
    kUserPwd_(pwd)
{
    makeValidTempWorkDir();
    makeCurrentDocDir();
}

ProjectPackage::~ProjectPackage()
{
    cleanUpDocDir(true);
}

void ProjectPackage::setImageData(const QString& filename_without_ext, ImageDataPtr data)
{
    auto full_filename = GetFileName(filename_without_ext, FT_IMAGE);
    image_data_dict_[full_filename] = data;
}

ProjectPackage::ImageDataPtr ProjectPackage::getImageData(const QString& filename_without_ext) const
{
    auto full_filename = GetFileName(filename_without_ext, FT_IMAGE);
    auto it = image_data_dict_.find(full_filename);
    return it != image_data_dict_.end() ? it.value() : nullptr;
}

void ProjectPackage::setPolyData(const QString &filename_without_ext, PolyDataPtr data)
{
    auto full_filename = GetFileName(filename_without_ext, FT_POLYS);
    poly_data_dict_[full_filename] = data;
}

ProjectPackage::PolyDataPtr ProjectPackage::getPolyData(const QString& filename_without_ext) const
{
    auto full_filename = GetFileName(filename_without_ext, FT_POLYS);
    auto it = poly_data_dict_.find(full_filename);
    return it != poly_data_dict_.end() ? it.value() : nullptr;
}

void ProjectPackage::addCustomFile(const QString& filepath)
{
    custom_filepaths_.append(filepath);
}

bool ProjectPackage::doLoad(const QString& source_path)
{
    notifyProgress(0.0, "Ready for loading");
    auto doc_root_dir = getDocDir();
    if(!QDir(doc_root_dir).exists()){
        SPDLOG_ERROR("Doc directory:\"{}\" not exists!", QStrToLogStr(kDocDir_));
        return false;
    }
    QFileInfo finf(source_path);
    if(!finf.exists() || !finf.isFile()){
        SPDLOG_ERROR("Source file path:\"{}\" not exists!", QStrToLogStr(source_path));
        return false;
    }
    notifyProgress(0.1, "Decompress package file");
    current_filepath_ = source_path;
    if(!decompressFileToDir()){
        goto cleanup_label;
    }
    notifyProgress(0.2, "Reading model data from package");
    if(!loadModelData()){
        goto cleanup_label;
    }
    notifyProgress(0.8, "Clean dirty files");
    cleanUpDocDir();
    notifyProgress(1.0, "Finish loading");
    SPDLOG_INFO("Load project file:\"{}\" successfully!", QStrToLogStr(current_filepath_));
    return true;

cleanup_label:
    SPDLOG_ERROR("Load project file:\"{}\" failed!", QStrToLogStr(current_filepath_));
    current_filepath_.clear();
    cleanUpDocDir();
    return false;
}

bool ProjectPackage::doSave(const QString& dest_path)
{
    notifyProgress(0.0, tr("Ready for saving"));
    auto doc_root_dir = getDocDir();
    if(!QDir(kDocDir_).exists()){
        SPDLOG_ERROR("Doc root directory:\"{}\" not exists!", QStrToLogStr(kDocDir_));
        return false;
    }    
    if(!QFileInfo(dest_path).absoluteDir().exists()){
        SPDLOG_ERROR("It's invalid path:\"{}\"!", QStrToLogStr(dest_path));
        return false;
    }
    notifyProgress(0.1, tr("Clean history files"));
    cleanUpDocDir();

    notifyProgress(0.2, tr("Start saving model data"));
    if(!saveModelData(0.6)){
        goto cleanup_label;
    }
    notifyProgress(0.8, tr("Compress files"));
    if(!compressDirToProjectFile(kDocDir_, dest_path)){
        goto cleanup_label;
    }
    notifyProgress(0.95, tr("Clean rubbish file"));
    current_filepath_ = dest_path;
    cleanUpDocDir();
    notifyProgress(1.0, tr("Finish saving package"));
    SPDLOG_INFO("Save project file:\"{}\" successfully!", QStrToLogStr(current_filepath_));
    return true;

cleanup_label:
    cleanUpDocDir();
    SPDLOG_ERROR("Save project file:\"{}\" failed!", QStrToLogStr(current_filepath_));
    return false;
}

void ProjectPackage::setVolumeData(ImageDataPtr data)
{
    setImageData(kVolumeDataFileName, data);
}

ProjectPackage::ImageDataPtr ProjectPackage::getVolumeData() const
{
    return getImageData(kVolumeDataFileName);
}

ProjectPackage::PolyDataPtr ProjectPackage::readPolyDataFromFile(const QString& filepath)const
{
    DefaultPolyDataSerializer pds;
    auto result_polydata = pds.readFile(filepath);
    if(result_polydata == nullptr || !result_polydata->GetNumberOfPoints()){
        SPDLOG_ERROR("Read poly data from file:\"{}\" failed!", QStrToLogStr(filepath));
    }else{
        SPDLOG_TRACE("Read poly data from file:\"{}\" successfully!", QStrToLogStr(filepath));
    }
    return result_polydata;
}

bool ProjectPackage::writePolyDataToFile(const QString& filepath, vtkPolyData* data)const
{
    DefaultPolyDataSerializer pds;
    if(!pds.writeFile(data, filepath)){
        SPDLOG_ERROR("Write poly data to file:\"{}\" failed!", QStrToLogStr(filepath));
        return false;
    }
    SPDLOG_TRACE("Write poly data to file:\"{}\" successfully!", QStrToLogStr(filepath));
    return true;
}

ProjectPackage::ImageDataPtr ProjectPackage::readImageDataFromFile(const QString& filepath)const
{
    DefaultImageDataSerializer ds;
    auto result_image_data = ds.readFile(filepath);
    if(result_image_data){
        SPDLOG_TRACE("Read image data file:\"{}\" successfully!", QStrToLogStr(filepath));
    }else{
        SPDLOG_ERROR("Read image data file:\"{}\" failed!", QStrToLogStr(filepath));
    }
    return result_image_data;
}

bool ProjectPackage::writeImageDataToFile(const QString& filepath, vtkImageData* data, double progress_amount, const QString& progress_text)
{
    Q_CHECK_PTR(data);
    current_progress_ += progress_amount * 0.4;
    notifyProgress(current_progress_, progress_text);
    DefaultImageDataSerializer ds;
    if(!ds.writeFile(data, filepath)){
        SPDLOG_ERROR("Write image data file:\"{}\" failed!", QStrToLogStr(filepath));
        return false;
    }    
    current_progress_ += progress_amount * 0.6;
    emit progressing(current_progress_, progress_text);   
    SPDLOG_TRACE("Write image file:\"{}\" successfully!", QStrToLogStr(filepath));
    return true;
}

bool ProjectPackage::saveModelData(double progress_amount)
{    
    auto model_element_filename = GetFileName(kElementFileName, FT_ELEMENT);
    auto abs_model_filepath = getAbsDocFilePath(model_element_filename);
    double step_progress = progress_amount * 0.1;
    current_progress_ += step_progress;
    emit progressing(current_progress_, tr("Save project document data"));
    if(!saveModelElements(abs_model_filepath)){
        return false;
    }    
    QStringList current_filenames;    
    current_filenames.push_back(model_element_filename);
    //write other large data files!    
    step_progress = progress_amount * 0.5;
    double file_step_progress = step_progress / (int)image_data_dict_.size();
    int file_id = 0;
    for(auto it = image_data_dict_.cbegin(); it != image_data_dict_.cend(); ++it){
        auto image_data = it.value();
        if(image_data == nullptr){
            continue;
        }
        ++file_id;
        QString progress_text = QString(tr("Save the %1th structure image data")).arg(file_id);
        const QString& fn = it.key();
        auto abs_filepath = getAbsDocFilePath(fn);
        if(!writeImageDataToFile(abs_filepath, image_data, file_step_progress, progress_text)){
            return false;
        }
        current_filenames.push_back(fn);
    }
    step_progress = progress_amount * 0.2;
    for(auto it = poly_data_dict_.cbegin(); it != poly_data_dict_.cend(); ++it){        
        auto poly_data = it.value();
        if(poly_data == nullptr){
            continue;
        }
        current_progress_ += step_progress / (int)poly_data_dict_.size();
        emit progressing(current_progress_, tr("Save poly data to file"));
        const QString& fn = it.key();
        auto abs_filepath = getAbsDocFilePath(fn);        
        if(!writePolyDataToFile(abs_filepath, poly_data)){
            return false;
        }
        current_filenames.push_back(fn);
    }
    auto is_locate_func = [&current_filenames](const QString& fn)->bool{
        return std::find(current_filenames.begin(), current_filenames.end(), fn) != current_filenames.end();
    };    
    for(auto fn : QStringList{GetFileName(kElementFileName, FT_ELEMENT), GetFileName(kVolumeDataFileName, FT_IMAGE)}){
        if(!is_locate_func(fn)){
            SPDLOG_ERROR("Necessary filename:\"{}\" not found!", QStrToLogStr(fn));
            return false;
        }
    }
    step_progress = progress_amount * 0.1;
    current_progress_ += step_progress;
    emit progressing(current_progress_, tr("Save custom file data"));
    for(auto src_filepath : custom_filepaths_){
        QFileInfo fi(src_filepath);
        if(!fi.exists()){
            SPDLOG_WARN("Custom filepath:\"{}\" not exists and neglect it!", QStrToLogStr(src_filepath));
            continue;
        }
        QString dst_filepath = getAbsDocFilePath(fi.fileName());
        if(!QFile::copy(src_filepath, dst_filepath)){
            SPDLOG_WARN("Copy file:\"{}\" to \"{}\" failed, and reason: \"{}\"", 
                QStrToLogStr(src_filepath), 
                QStrToLogStr(dst_filepath), 
                QStrToLogStr(QFile(src_filepath).errorString())
            ); 
            continue;
        }
    }
    step_progress = progress_amount * 0.1;
    current_progress_ += step_progress;
    emit progressing(current_progress_, tr("Save test data file"));
    return true;
}

QString ProjectPackage::getAbsDocFilePath(const QString &filename) const
{
    return QDir(kDocDir_).filePath(filename);
}

bool ProjectPackage::decompressFileToDir()const
{
    FileUnzipper zf;
    zf.setZipPath(current_filepath_.toLocal8Bit().toStdString());
    zf.setPassword(kUserPwd_.toStdString());
    zf.setDestDir(kDocDir_.toLocal8Bit().toStdString());
    if(!zf.decompress()){
        SPDLOG_ERROR("Package file:\"{}\" cannot resolved!", QStrToLogStr(current_filepath_));
        return false;
    }
    zf.close();
    SPDLOG_TRACE("Decompress package file:\"{}\" to dir:\"{}\" successfully!", QStrToLogStr(current_filepath_), QStrToLogStr(kDocDir_));
    return true;
}

bool ProjectPackage::compressDirToProjectFile(const QString& doc_dir, const QString& filepath) const
{
    FileZipper zf;
    zf.setZipPath(filepath.toLocal8Bit().data());
    zf.setPassword(kUserPwd_.toStdString());
    if(!zf.compressDir(doc_dir.toLocal8Bit().data())){
        SPDLOG_ERROR("Compress dir to project file failed! filepath:\"{}\".", QStrToLogStr(filepath));
        return false;
    }
    return true;
}

bool ProjectPackage::loadModelData()
{
    const double STEP_PROGRESS_AMOUNT = 0.7;
    image_data_dict_.clear();
    poly_data_dict_.clear();

    auto element_file_path = QDir(kDocDir_).filePath(GetFileName(kElementFileName, FT_ELEMENT));
    if(!QFileInfo(element_file_path).exists()){
        SPDLOG_ERROR("Lack of \"model_element.db\" in package file:\"{}\"!", QStrToLogStr(current_filepath_));
        return false;
    }
    if(!loadModelElements(element_file_path)){
        SPDLOG_ERROR("Load model elements from file:\"{}\" failed!", QStrToLogStr(element_file_path));
        return false;
    }
    current_progress_ += 0.1;
    notifyProgress(current_progress_, "Reading model elements finished!");

    auto volume_filename = GetFileName(kVolumeDataFileName, FT_IMAGE);
    auto volume_file_path = QDir(kDocDir_).filePath(volume_filename);
    if(!QFileInfo(volume_file_path).exists()){
        SPDLOG_ERROR("Lack of \"volume_data.img\" in package file:\"{}\"!", QStrToLogStr(current_filepath_));
        return false;
    }
    auto volum_data = readImageDataFromFile(volume_file_path);
    if(volum_data == nullptr){
        SPDLOG_ERROR("Load \"volume_data.img\" from file:\"{}\" failed!", QStrToLogStr(element_file_path));
        return false;
    }
    image_data_dict_[volume_filename] = volum_data;
    current_progress_ += 0.4;
    notifyProgress(current_progress_, "Reading volume data finished!");

    auto filenames = path_util::SearchDirFiles(kDocDir_, {kImageFileSuffixName, kPolyFileSuffixName});
    const double FILE_PROGRESS = 0.2 / (int)filenames.size();
    auto get_filename_ext = [](const QString& fn){
        return fn.section('.', -1);
    };
    for(const auto& fn : filenames){
        if(fn == volume_filename){ //already handled!
            continue;
        }
        auto abs_filepath = getAbsDocFilePath(fn);
        QString ext = get_filename_ext(fn);
        if(ext == kImageFileSuffixName){
            auto image_data = readImageDataFromFile(abs_filepath);
            if(image_data){
                image_data_dict_[fn] = image_data;
            }
        }else if(ext == kPolyFileSuffixName){
            auto polys_data = readPolyDataFromFile(abs_filepath);
            if(polys_data){
                poly_data_dict_[fn] = polys_data;
            }
        }
        current_progress_ += FILE_PROGRESS;
        notifyProgress(current_progress_, "Read other data");
    }
    return true;
}

bool ProjectPackage::loadModelElements(const QString& filepath)
{
    if(!model_elem_serializer_->readFile(filepath)){
        SPDLOG_ERROR("Parse model elements from file:\"{}\" failed!", QStrToLogStr(filepath));
        return false;
    }
    SPDLOG_TRACE("Parse model elements to file:\"{}\" successfully!", QStrToLogStr(filepath));
    return true;
}

bool ProjectPackage::saveModelElements(const QString& filepath)const
{
    if(!model_elem_serializer_->writeFile(filepath)){
        SPDLOG_ERROR("Serialize model elements to file:\"{}\" failed!", QStrToLogStr(filepath));
        return false;
    }
    SPDLOG_TRACE("Serialize model elements to file:\"{}\" successfully!", QStrToLogStr(filepath)); 
    return true;
}

void ProjectPackage::cleanUpDocDir(bool rm_root)const
{
    if(!path_util::RemoveTree(kDocDir_, rm_root)){
        SPDLOG_ERROR("Clean doc root dir:\"{}\" failed!", QStrToLogStr(kDocDir_));
        return;
    }
    SPDLOG_TRACE("Clean current document directory:\"{}\" successfully!", QStrToLogStr(kDocDir_));
}

void ProjectPackage::notifyProgress(double p, const QString& text)
{
    current_progress_ = p;
    emit progressing(current_progress_, text);
}

namespace{
    struct _AutoState{
        ProjectPackage::Status& ref_status;
        ProjectPackage::Status end_v;
        _AutoState(ProjectPackage::Status& s, ProjectPackage::Status start_v, ProjectPackage::Status ev)
            :ref_status(s){
            ref_status = start_v;
            end_v = ev;
        }
        ~_AutoState(){
            ref_status = end_v;
        }
    };
}

bool ProjectPackage::load(const QString& source_path)
{
    _AutoState s(status_, PPS_LOADING, PPS_IDLE);
    return doLoad(source_path);
}

bool ProjectPackage::save(const QString& dest_path)
{
    _AutoState s(status_, PPS_SAVING, PPS_IDLE);
    if(!hasDocData()){
        SPDLOG_ERROR("No document data exists!");
        return false;
    }
    return doSave(dest_path);
}

void ProjectPackage::resetNew()
{
    if(hasDocData()){
        SPDLOG_ERROR("Still exists package data loaded, please call close() at first!");
        return;
    }
    if(status_ != PPS_IDLE){
        SPDLOG_ERROR("Current package status is not idle!");
        return;
    }
    model_elem_serializer_->resetNewRootElem();
    current_filepath_.clear();
    cleanUpDocDir();
    clearCacheData();
}

void ProjectPackage::close()
{
    if(!hasDocData()){
        SPDLOG_ERROR("No any package data loaded at present!");
        return;
    }
    if(status_ != PPS_IDLE){
        SPDLOG_ERROR("Current package status is not idle!");
        return;
    }
    model_elem_serializer_->resetRootElemToNull();    
    current_filepath_.clear();
    cleanUpDocDir();
    clearCacheData();
}

bool ProjectPackage::hasDocData() const
{
    return model_elem_serializer_->existRootElem();
}

void ProjectPackage::makeValidTempWorkDir()
{
    QFileInfo fi(kTempWorkDir_);
    if(fi.exists() && fi.isDir()){
        return;
    }    
    QString new_temp_dir = "_temp"; //set temporary work directory by myself.
    if(!QDir(new_temp_dir).exists()){
        QDir::current().mkpath(new_temp_dir);
    } 
    const_cast<QString&>(kTempWorkDir_) = QDir(new_temp_dir).absolutePath();
    SPDLOG_TRACE("Temporary work directory:\"{}\" is reset automatically!", QStrToLogStr(kTempWorkDir_));
}

void ProjectPackage::clearCacheData()
{
    image_data_dict_.clear();
    poly_data_dict_.clear();
    custom_filepaths_.clear();
}

void ProjectPackage::makeCurrentDocDir()
{
    QDir root_dir(kTempWorkDir_);
    const_cast<QString&>(kDocDir_) = root_dir.filePath(QUuid::createUuid().toString());
    QDir().mkpath(kDocDir_);
    //set hidden attribute for safety
    path_util::SetDirHidden(kDocDir_);
    SPDLOG_TRACE("Create doc directory:\"{}\" successfully!", QStrToLogStr(kDocDir_));    
}

NAMESPACE_END
