#include "ortho_toolbar.h"
#include <QHBoxLayout>

OrthoToolbar::OrthoToolbar(QWidget* parent)
    :QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background:rgba(30,30,30,200);");
    auto* h = new QHBoxLayout(this);
    h->setContentsMargins(4, 2, 4, 2);
    maximize_btn= new QPushButton("Maximize",this);
    maximize_btn->setCheckable(true);
    maximize_btn->setChecked(false);
    maximize_btn->setStyleSheet("background-color:white;");
    maximize_btn->setFixedSize(100, 24);
    h->addWidget(maximize_btn);       
    h->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));  
}