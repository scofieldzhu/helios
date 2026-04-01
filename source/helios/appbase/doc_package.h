/******************************************************** 
* author: scofieldzhu
* time:2025/12/25
*******************************************************/
#ifndef __doc_package_h__
#define __doc_package_h__

#include <QString>
#include <QMap>
#include <vtkSmartPointer.h>
#include "helios/appbase/helios_appbase_export.h"

class vtkImageData;
class vtkPolyData;

HELIOS_NAMESPACE_BEGIN

class MetaDataSerializer;

class HELIOS_APPBASE_API DocPackage
{
public:
    using ProgressCallback = std::function<void(double, const QString&)>;

    enum Status{
        PPS_IDLE,
        PPS_LOADING,
        PPS_SAVING
    };
    using ImageDataPtr = vtkSmartPointer<vtkImageData>;
    using PolyDataPtr = vtkSmartPointer<vtkPolyData>;        
    bool load(const QString& source_path);
    bool save(const QString& dest_path);
    void setVolumeData(ImageDataPtr data);
    ImageDataPtr getVolumeData()const;
    void setImageData(const QString& filename, ImageDataPtr data);
    ImageDataPtr getImageData(const QString& filename)const;
    void setPolyData(const QString& filename, PolyDataPtr data);
    PolyDataPtr getPolyData(const QString& filename)const;
    void addCustomFile(const QString& filepath);    
    const QString& currentFilePath()const { return current_filepath_; }
    const QString& getDocDir()const { return kDocDir_; }
    MetaDataSerializer* getRootDocMetaDataSerializer()const;
    void setProgressCallback(ProgressCallback cb){ progress_cb_ = cb; }
    auto status()const{ return status_; }    
    DocPackage& operator=(const DocPackage&) = delete;
    DocPackage(std::unique_ptr<MetaDataSerializer> mds, const QString& temp_work_dir, const QString& pwd);
    DocPackage(const DocPackage&) = delete;
    virtual ~DocPackage();

private:
    void makeCurrentDocDir(const QString& root_dir);
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
    const QString kUserPwd_;
    QString current_filepath_;
    double current_progress_ = 0.0;
    Status status_ = PPS_IDLE;    
    std::unique_ptr<MetaDataSerializer> root_mds_;
    ProgressCallback progress_cb_;
};

NAMESPACE_END

#endif