/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/12/25
*******************************************************/
#ifndef __project_package_h__
#define __project_package_h__

#include <QObject>
#include <QString>
#include <QMap>
#include <vtkSmartPointer.h>
#include "helios/appbase/helios_appbase_export.h"

class vtkImageData;
class vtkPolyData;

HELIOS_NAMESPACE_BEGIN

class ModelElementSerializer;

class HELIOS_APPBASE_API ProjectPackage : public QObject
{
    Q_OBJECT

signals:
    void progressing(double p, const QString& desp);

public:
    enum Status{
        PPS_IDLE,
        PPS_LOADING,
        PPS_SAVING
    };
    using ImageDataPtr = vtkSmartPointer<vtkImageData>;
    using PolyDataPtr = vtkSmartPointer<vtkPolyData>;        
    bool load(const QString& source_path);
    bool save(const QString& dest_path);
    void resetNew();
    void close();
    bool hasDocData()const;
    void setVolumeData(ImageDataPtr data);
    ImageDataPtr getVolumeData()const;
    void setImageData(const QString& filename, ImageDataPtr data);
    ImageDataPtr getImageData(const QString& filename)const;
    void setPolyData(const QString& filename, PolyDataPtr data);
    PolyDataPtr getPolyData(const QString& filename)const;
    void addCustomFile(const QString& filepath);    
    const QString& currentFilePath()const { return current_filepath_; }
    const QString& getDocDir()const { return kDocDir_; }
    auto modelElementSerializer(){ return model_elem_serializer_; }
    auto status()const{ return status_; }
    ProjectPackage& operator=(const ProjectPackage&) = delete;
    ProjectPackage(ModelElementSerializer& serializer, const QString& temp_work_dir, const QString& pwd);
    ProjectPackage(const ProjectPackage&) = delete;
    ~ProjectPackage();

private:
    void clearCacheData();
    void makeValidTempWorkDir();
    void makeCurrentDocDir();
    bool doLoad(const QString& source_path);
    bool doSave(const QString& dest_path);
    PolyDataPtr readPolyDataFromFile(const QString& filepath)const;
    bool writePolyDataToFile(const QString& filepath, vtkPolyData* data)const;
    ImageDataPtr readImageDataFromFile(const QString& filepath)const;
    bool writeImageDataToFile(const QString& filepath, vtkImageData* data, double progress_amount, const QString& progress_text);
    bool saveModelData(double progress_amount);
    QString getAbsDocFilePath(const QString& filename)const;
    bool decompressFileToDir()const;
    bool compressDirToProjectFile(const QString& doc_dir, const QString& filepath)const;
    bool loadModelData();
    bool loadModelElements(const QString& filepath);
    bool saveModelElements(const QString& filepath)const;
    void cleanUpDocDir(bool rm_root = false)const;
    void notifyProgress(double p, const QString& text);
    QMap<QString, ImageDataPtr> image_data_dict_;
    QMap<QString, PolyDataPtr> poly_data_dict_;
    QStringList custom_filepaths_;
    const QString kDocDir_;
    const QString kTempWorkDir_;
    const QString kUserPwd_;
    QString current_filepath_;
    double current_progress_ = 0.0;
    Status status_ = PPS_IDLE;    
    ModelElementSerializer* model_elem_serializer_;
};

NAMESPACE_END

#endif