/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/9/3
*******************************************************/
#include "label_sprite.h"
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include "mirfak/basic/log_service.h"

MIRFAK_NAMESPACE_BEGIN

LabelSprite::LabelSprite()
    :text_actor_(vtkSmartPointer<vtkTextActor>::New())
{
    initTextProps();
}

LabelSprite::~LabelSprite()
{
}

void LabelSprite::initTextProps()
{
    text_actor_->SetInput("--");
    auto text_prop = text_actor_->GetTextProperty();
    text_prop->SetFontFamilyToArial();
    text_prop->SetFontSize(16);
    text_prop->SetBold(true);
    text_prop->SetShadow(true);
    text_prop->SetColor(1.0, 0.0, 0.0);          
    text_prop->SetBackgroundOpacity(0.0);      
    text_prop->SetJustificationToLeft();
    text_prop->SetVerticalJustificationToTop();    
    text_actor_->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    text_actor_->GetPositionCoordinate()->SetValue(0.02, 0.98); 
}

void LabelSprite::setText(const char* text)
{
    text_actor_->SetInput(text);
    text_actor_->Modified();
    modified();
}

void LabelSprite::setFontFamily(int f)
{
    text_actor_->GetTextProperty()->SetFontFamily(f);
    text_actor_->Modified();
    modified();
}

void LabelSprite::setFontFile (const char* font_file)
{
    text_actor_->GetTextProperty()->SetFontFile(font_file);
    text_actor_->Modified();
    modified();
}

void LabelSprite::setBold(bool b)
{
    text_actor_->GetTextProperty()->SetBold(b);
    text_actor_->Modified();
    modified();
}

void LabelSprite::setFontSize(int size)
{
    text_actor_->GetTextProperty()->SetFontSize(size);
    text_actor_->Modified();
    modified();
}

void LabelSprite::setColor(const Color& color)
{
    Sprite::setColor(color);
    text_actor_->GetTextProperty()->SetColor(color.toValuesR().data());
    text_actor_->Modified();
    modified();
}

void LabelSprite::setPosition(double x, double y)
{
    text_actor_->GetPositionCoordinate()->SetValue(x, y);
    text_actor_->Modified();
    modified();
}

void LabelSprite::setOpacity(double opacity)
{
    Sprite::setOpacity(opacity);
    text_actor_->GetTextProperty()->SetOpacity(opacity);
    text_actor_->Modified();
    modified();
}

void LabelSprite::setJustification(int j)
{
    text_actor_->GetTextProperty()->SetJustification(j);
    text_actor_->Modified();
    modified();
}

bool LabelSprite::addToScene(Scene &scene)
{
    if(existsDisplayScene()){
        SPDLOG_ERROR("Only one scene object can added!");
        return false;
    }
    return Sprite::addToScene(scene);
}

void LabelSprite::makeActors(Scene &scene)
{
    addSceneProp(scene, text_actor_, "text");
}

NAMESPACE_END
