/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/3/3
*******************************************************/
#ifndef __dicom_meta_data_h__
#define __dicom_meta_data_h__

#include <string>
#include "helios/helios_nsp.h"

HELIOS_NAMESPACE_BEGIN

struct DICOMMetaData
{
    std::string patient_name;
    unsigned int patient_age = 0;
    std::string patient_gender;
    std::string acquistion_date;
    std::string acquistion_time;
    std::string manufacturer;
    double win_width = 0.0;
    double win_level = 0.0;
    double slice_thickness = 0.0;
};

NAMESPACE_END

#endif
