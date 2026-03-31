/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/3/4
*******************************************************/
#include "movie_label.h"
#include <QCoreApplication>
#include <QTimerEvent>
#include <QPainter>

namespace{
    constexpr size_t kCycleImageCount = 24;
    constexpr int kTimerInterval = 50;
}

MovieLabel::MovieLabel(QWidget* parent) 
    :QLabel(parent)
{

}

MovieLabel::~MovieLabel()
{
}

void MovieLabel::startPlaying()
{
	this->killTimer(timer_id_);
	timer_id_ = this->startTimer(50);
	QString using_image_path = QString(":/image/loding_%1.png").arg(using_image_id_);
	pixmap_.load(using_image_path);
}

void MovieLabel::stop()
{
	this->killTimer(timer_id_);
	timer_id_ = 0;
	using_image_id_ = 0;
}

void MovieLabel::setDisplayProgress(bool b)
{
    display_progress_ = b;
    hope_progress_ = 0;
    current_progress_value_ = 0;
    hope_progress_text_ = "";
}

void MovieLabel::setProgress(int progress, const QString& desp)
{
    if(hope_progress_ != progress || hope_progress_text_ != desp){
        if(current_progress_value_ != hope_progress_){
            current_progress_value_ = hope_progress_;
        }
        hope_progress_ = progress;
        hope_progress_text_ = desp;
    }    
}

void MovieLabel::timerEvent(QTimerEvent *event)
{
	if(event->timerId() == timer_id_){
		using_image_id_ = (++using_image_id_) % kCycleImageCount;
        QString using_image_path = QString(":/image/loding_%1.png").arg(using_image_id_);
		pixmap_.load(using_image_path);
        repaint();  
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	}
}

void MovieLabel::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
	painter.drawPixmap(50, 50, pixmap_.scaled({200, 200}, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    if(!display_progress_){
        return;
    }
    QFont font = painter.font();
    font.setPointSize(24);
    font.setFamily(QString::fromLocal8Bit("微软雅黑"));
    painter.setFont(font);
    painter.setPen(QColor(61, 117, 186));
    QRectF txt_bound(0, 50, 300, 200);
    painter.drawText(txt_bound, Qt::AlignCenter, QString("%1%").arg(current_progress_value_));
    if(current_progress_value_ < hope_progress_){
        ++current_progress_value_;
    }
    font = painter.font();
    font.setPointSize(10);
    font.setBold(false);
    font.setUnderline(true);
    painter.setFont(font);
    if(!hope_progress_text_.isEmpty()){
        QRectF box(0, 250, 300, 50);
        painter.drawText(box, Qt::AlignCenter, hope_progress_text_);
    }
    //qDebug() << "current_progress_value_:" << current_progress_value_ << " hope_progress_:" << hope_progress_ << "\n";
}
