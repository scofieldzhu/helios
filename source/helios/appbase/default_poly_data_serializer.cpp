/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/25
*******************************************************/
#include "default_poly_data_serializer.h"
#include <QFileInfo>
#include <QDir>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkNew.h>
#include <vtkCallbackCommand.h>
#include "helios/basic/log_service.h"
#include "helios/appbase/helios_appbase_typedef.h"

HELIOS_NAMESPACE_BEGIN

bool DefaultPolyDataSerializer::writeFile(vtkPolyData* data, const QString& filepath)
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
    cmd->SetCallback(&DefaultPolyDataSerializer::OnCallback);
    cmd->SetClientData(this);    
    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->AddObserver(vtkCommand::ProgressEvent, cmd);
    writer->SetInputData(data);
    writer->SetFileName(filepath.toUtf8().data());
    writer->SetDataModeToBinary();
    return writer->Write() == 1;
}

vtkSmartPointer<vtkPolyData> DefaultPolyDataSerializer::readFile(const QString& filepath)
{
    if(!QFileInfo(filepath).exists()){
        SPDLOG_ERROR("The filepath:\"{}\" is invalid!", QStrToLogStr(filepath));
        return nullptr;
    }
    vtkNew<vtkXMLPolyDataReader> reader;
    if(reader->CanReadFile(filepath.toUtf8().constData()) == 0){
        SPDLOG_ERROR("The file:\"{}\" is not well-formed!", QStrToLogStr(filepath));
        return nullptr;
    }
    reader->SetFileName(filepath.toUtf8().constData());
    reader->Update();
    return reader->GetOutput();
}

void DefaultPolyDataSerializer::OnCallback(vtkObject* caller, unsigned long e_id, void* client_data, void* call_data)
{
    auto s = static_cast<DefaultPolyDataSerializer*>(client_data);
    if(s){
        s->handleCallback(caller, e_id, call_data);
    }    
}

void DefaultPolyDataSerializer::handleCallback(vtkObject* caller, unsigned long e_id, void* call_data)
{
    auto algo = vtkAlgorithm::SafeDownCast(caller);
    if(algo){
        Progress.invoke(algo->GetProgress(), algo->GetProgressText());
    }
}

NAMESPACE_END