/******************************************************** 
* author: scofieldzhu
* time:2025/12/25
*******************************************************/
#include "default_image_data_serializer.h"
#include <QFileInfo>
#include <QDir>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkNew.h>
#include <vtkCallbackCommand.h>
#include "helios/basic/log_service.h"
#include "log_misc.h"

HELIOS_NAMESPACE_BEGIN

bool DefaultImageDataSerializer::writeFile(vtkImageData* data, const QString& filepath)
{
    if(data == nullptr){
        SPDLOG_ERROR("The data is null pointer!");
        return false;
    }
    if(!QFileInfo(filepath).dir().exists()){
        SPDLOG_ERROR("The filepath:\"{}\" is invalid!", QStrToLogStr(filepath));
        return false;
    }
    vtkNew<vtkCallbackCommand> cmd;
    cmd->SetCallback(&DefaultImageDataSerializer::OnCallback);
    cmd->SetClientData(this);
    vtkNew<vtkXMLImageDataWriter> writer;
    writer->AddObserver(vtkCommand::ProgressEvent, cmd);
    writer->SetFileName(filepath.toUtf8().constData());
    writer->SetInputData(data);
    writer->SetCompressorTypeToZLib();
    writer->Write();
    return true;
}

vtkSmartPointer<vtkImageData> DefaultImageDataSerializer::readFile(const QString& filepath)
{
    if(!QFileInfo(filepath).exists()){
        SPDLOG_ERROR("The filepath:\"{}\" is invalid!", QStrToLogStr(filepath));
        return nullptr;
    }
    vtkNew<vtkXMLImageDataReader> reader;
    if(reader->CanReadFile(filepath.toUtf8().constData()) == 0){
        SPDLOG_ERROR("The file:\"{}\" is not well-formed!", QStrToLogStr(filepath));
        return nullptr;
    }
    reader->SetFileName(filepath.toUtf8().constData());
    reader->Update();
    return reader->GetOutput();
}

void DefaultImageDataSerializer::OnCallback(vtkObject* caller, unsigned long e_id, void* client_data, void* call_data)
{
    auto s = static_cast<DefaultImageDataSerializer*>(client_data);
    if(s){
        s->handleCallback(caller, e_id, call_data);
    }    
}

void DefaultImageDataSerializer::handleCallback(vtkObject* caller, unsigned long e_id, void* call_data)
{
    auto algo = vtkAlgorithm::SafeDownCast(caller);
    if(algo){
        Progress.invoke(algo->GetProgress(), algo->GetProgressText());
    }
}

NAMESPACE_END