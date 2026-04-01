/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/9/12
*******************************************************/
#include "drill_sprite.h"
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkSTLReader.h>
#include <QFile>
#include "helios/basic/log_service.h"
#include "helios_navigation_typedef.h"

HELIOS_NAMESPACE_BEGIN

extern std::string QStrToStd(const QString& qstr);

namespace{
    vtkSmartPointer<vtkPolyData> ReadModelData(const QString& filename)
    {
        if(!QFile::exists(filename)){
            SPDLOG_ERROR("model file path:{} not exists!", QStrToStd(filename));
            return nullptr;
        }
        vtkNew<vtkSTLReader> reader;
        reader->SetFileName(filename.toUtf8().data());
        reader->Update();
        vtkSmartPointer<vtkPolyData> polydata = reader->GetOutput();
        if(polydata == nullptr){
            SPDLOG_ERROR("read failed:{}", filename.toUtf8().toStdString());
            return nullptr;
        }
        return polydata;
    }
}

DrillSprite::DrillSprite()
    :current_model_data_(vtkSmartPointer<vtkPolyData>::New())
{
}

DrillSprite::~DrillSprite()
{
}

void DrillSprite::rotate()
{
    auto actor_collection = getSceneActors(nullptr);
    actor_collection->InitTraversal();
    auto current_actor = actor_collection->GetNextActor();
    while(current_actor){
        current_actor->RotateZ(72.0);
        current_actor = actor_collection->GetNextActor();
    }
    modified();
}

void DrillSprite::loadConfig(const DrillConfig& cf)
{
    if(current_config_ == cf){
        return;
    }    
    if(!cf){
        current_config_.reset();
        makePolyDataEmpty(current_model_data_);
    }else{
        current_config_ = cf;
        current_model_data_ = ReadModelData(cf.model_filepath);
    }
    if(existsDisplayScene()){
        updateModelData();
    }
}

void DrillSprite::makePolyDataEmpty(vtkPolyData *data)
{
    data->Reset();
    data->SetPoints(vtkSmartPointer<vtkPoints>::New());    
    data->SetVerts(vtkSmartPointer<vtkCellArray>::New());
    data->SetLines(vtkSmartPointer<vtkCellArray>::New());
    data->SetPolys(vtkSmartPointer<vtkCellArray>::New());
    data->SetStrips(vtkSmartPointer<vtkCellArray>::New());
}

void DrillSprite::updateModelData()
{    
    for(auto s : getDisplaySceneList()){
        auto a = getSceneActor("drill", s);
        if(a){
            auto poly_mapper = vtkPolyDataMapper::SafeDownCast(a->GetMapper());
            poly_mapper->SetInputData(current_model_data_);
            poly_mapper->Modified();
        }
    }
    current_model_data_->Modified();
    modified();
}

void DrillSprite::createSource(Scene& scene)
{
}

void DrillSprite::makeActors(Scene& scene)
{
    vtkNew<vtkPolyDataMapper> m;
    m->SetInputData(current_model_data_);
    vtkNew<vtkActor> a;
    a->SetMapper(m);
    addSceneProp(scene, a, "drill");
}

NAMESPACE_END
