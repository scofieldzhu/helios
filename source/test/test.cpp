#include <QApplication>
#include <QTranslator>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkMatrix3x3.h>
#include <vtkMetaImageReader.h>
#include "helios/basic/helios_basic_typedef.h"
#include "helios/basic/signal.hpp"
#include "helios/basic/log_service.h"
#include "main_widget.h"
using namespace helios;

vtkSmartPointer<vtkImageData> MakeDefaultVolume()
{
    vtkNew<vtkImageData> image;
    image->SetDimensions(100, 100, 50);
    image->SetSpacing(1.0, 1.0, 1.0);
    image->SetOrigin(0.0, 0.0, 0.0);
    vtkNew<vtkMatrix3x3> dir;
    dir->Identity();
    image->SetDirectionMatrix(dir);
    image->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    int* dims = image->GetDimensions();
    int nx = dims[0];
    int ny = dims[1];
    int nz = dims[2];
    int numComponents = image->GetNumberOfScalarComponents();
    vtkIdType numVoxels = static_cast<vtkIdType>(nx) * ny * nz;
    vtkIdType numValues = numVoxels * numComponents;
    unsigned char* ptr = static_cast<unsigned char*>(image->GetScalarPointer());
    std::memset(ptr, 0, static_cast<size_t>(numValues) * sizeof(unsigned char));
    return image;
}

vtkSmartPointer<vtkImageData> ReadTestVolume()
{
    vtkSmartPointer<vtkImageData> image;
    vtkNew<vtkMetaImageReader> reader;
    reader->SetFileName("test_image_1209.mha");
    reader->Update();
    image = reader->GetOutput();
    auto origin = image->GetOrigin();
    auto spacings = image->GetSpacing();
    auto extents = image->GetExtent();
    SPDLOG_DEBUG("Volume data:");
    SPDLOG_DEBUG("Origin:{} {} {}", origin[0], origin[1], origin[2]);
    SPDLOG_DEBUG("Spacing:{} {} {}", spacings[0], spacings[1], spacings[2]);
    SPDLOG_DEBUG("Extents:{} {} {} {} {} {}", extents[0], extents[1], extents[2], extents[3], extents[4], extents[5]);
    return image;
}

int main(int argc, char *argv[])
{
    //spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%# %!] %v");
    vtkObject::GlobalWarningDisplayOff();
    spdlog::set_level(spdlog::level::trace); 
    SPDLOG_INFO("Test application started...");    
    auto default_image = MakeDefaultVolume();
    QApplication app(argc, argv);
    QTranslator translator;
    if(translator.load("helios_Demo_zh_CN.qm", "./res/conf/lang")){
        app.installTranslator(&translator);
    }
    MainWidget mw;
    mw.setGlobalImage(default_image);
    if(!mw.initUI()){
        SPDLOG_ERROR("Init main window failed!");
        return 0;
    }
    return app.exec();
}