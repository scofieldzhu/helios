/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/11/24
*******************************************************/
#ifndef __IDICOMReader_h__
#define __IDICOMReader_h__

#include <QObject>
#include <QString>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <functional>
#include <optional>

struct _tagDICOMMetaData
{
    std::string patient_name;
    uint32_t patient_age = 0;
    std::string patient_gender;
    std::string acquistion_date;
    std::string acquistion_time;
    std::string manufacturer;
    double win_width = 0.0;
    double win_level = 0.0;
    double slice_thickness = 0.0;
};

class IDICOMReader
{
public:
    using CallbackType = std::function<void(IDICOMReader*)>;
    virtual void setDICOMDirectory(const QString& dir) = 0;
    virtual void setFinishReadCallback(CallbackType cb) = 0;
    virtual vtkSmartPointer<vtkImageData> getSeriesData() = 0;
    virtual bool readSeriesData(int series_id) = 0;
    virtual std::optional<_tagDICOMMetaData> readMetaData()const = 0;
    virtual ~IDICOMReader() = default;    
};

#define IDICOM_READER_IID "helios.plugin.IDICOMReader/1.0"

Q_DECLARE_INTERFACE(IDICOMReader, IDICOM_READER_IID)

#endif