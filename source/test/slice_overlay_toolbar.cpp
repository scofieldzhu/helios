#include "slice_overlay_toolbar.h"
#include <QHBoxLayout>

SliceOverlayToolbar::SliceOverlayToolbar(QWidget* parent)
    :QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground,true);
    setStyleSheet("background:rgba(30,30,30,200);");
    auto* h = new QHBoxLayout(this);
    h->setContentsMargins(4,2,4,2);

    maximize_btn = new QPushButton("Maximize",this); 
    maximize_btn->setCheckable(true);
    maximize_btn->setChecked(false);
    maximize_btn->setStyleSheet("background-color:white;");
    maximize_btn->setFixedSize(100, 24);
    slice_slider = new QSlider(Qt::Horizontal,this);
    angle = new QSlider(Qt::Horizontal,this); 
    angle->setMinimumWidth(100);
    angle->setRange(0, 360);

    h->addWidget(maximize_btn);         
    h->addWidget(slice_slider, 1);        // 切片
    h->addWidget(angle);          // 旋转

    slice_slider->setRange(0, 100);
    slice_slider->setValue(50);
}