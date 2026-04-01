/*******************************************************
* author: scofieldzhu
* time:2026/3/4
*******************************************************/
#include "task_progressor_view.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <QTimer>
#include <QMouseEvent>
#include <QProcess>
#include <TLHELP32.H>
#include <QBitmap>
#include <QSharedMemory>
#include <QDebug>
#include "movie_label.h"

TaskProgressorView::TaskProgressorView(QWidget *parent)
	:QMainWindow(parent)
{
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_NoSystemBackground, false);
	setWindowFlags(Qt::FramelessWindowHint|Qt::Tool);
	setWindowModality(Qt::ApplicationModal);

	movie_label_ = new MovieLabel(this);
	movie_label_->setAutoFillBackground(true);
	movie_label_->setWindowFlags(Qt::FramelessWindowHint);
	movie_label_->setAttribute(Qt::WA_TranslucentBackground, true);
	movie_label_->setAttribute(Qt::WA_NoSystemBackground, false);
	movie_label_->setStyleSheet("background-color: transparent;");
	
	shared_memory_ = new QSharedMemory(this);
	shared_memory_->setKey(kTaskProgressorSHMName);
	shared_memory_->attach(QSharedMemory::ReadWrite);

	this->startTimer(30);
	this->hide();
}

TaskProgressorView::~TaskProgressorView()
{
	if(shared_memory_->isAttached()){
		shared_memory_->detach();
	}
}

void TaskProgressorView::startPlayingMovie()
{
	QRect rect = this->geometry();
	int height = rect.height();
	int width = rect.width();
	movie_label_->setGeometry(width/2-300/2, height/2 - 300 / 2, 300, 300);
	QPixmap pixmap = QPixmap(main_win_snapshot_path_);
	QPalette palette(this->palette());
	palette.setBrush(QPalette::Background, QBrush(pixmap));
	this->setPalette(palette);
	movie_label_->startPlaying();
	//qDebug() << "startPlayingMovie ...";
}

void TaskProgressorView::stopMoviePlaying()
{
	movie_label_->stop();
	//qDebug() << "stopMoviePlaying ...";
}

void TaskProgressorView::execMovieCommand(int flag, int x, int y, int width, int height)
{
	if(flag){
		this->setGeometry(QRect(x,y,width,height));
		this->move(x, y);
		startPlayingMovie();
		this->show();
		this->setFocus();
		this->update();
	}else{
		this->hide();
		stopMoviePlaying();
	}
}

bool TaskProgressorView::checkProcessAlive(__int64 process_id)
{
	bool res = false;
	HANDLE hToolHelp32Snapshot;
	hToolHelp32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
	BOOL is_success = Process32First(hToolHelp32Snapshot, &pe);
	while(is_success){
		if(pe.th32ProcessID == process_id){
			res = true;
			break;
		}
		is_success = Process32Next(hToolHelp32Snapshot, &pe);
	}
	CloseHandle(hToolHelp32Snapshot);
	return res;
}

void TaskProgressorView::timerEvent(QTimerEvent *event)
{
	processIPCData();
	if(this->isVisible()){
        setWindowFlag(Qt::WindowStaysOnTopHint, true);  
        show();
        setFocus();
        ::SetActiveWindow((HWND)winId());
	}
	if(master_process_id_ != 0 && !checkProcessAlive(master_process_id_)){
		exit(0);
	}else if (0 == master_process_id_){
		static int stCheckMasterProcessAliveCnt = 0;
		if(++stCheckMasterProcessAliveCnt > 30){
			exit(0);
		}
	}
}

void TaskProgressorView::updateProgressText(const wchar_t *wstr)
{
    if(wstr == nullptr){
        received_progress_text_.clear();
        return;
    }
    wchar_t pro_text[256];
    wmemset(pro_text, 0, 256);
    wmemcpy(pro_text, wstr, wcslen(wstr));
    pro_text[wcslen(wstr) + 1] = L'\0';
    received_progress_text_ = QString::fromStdWString(pro_text);
}

void TaskProgressorView::processIPCData()
{
	shared_memory_->lock();
	TaskProgressorIpcData* ipc_data = reinterpret_cast<TaskProgressorIpcData*>(shared_memory_->data());
	CmdType status = ipc_data->cmd_type;
	if(status != last_cmd_type_ || status == CmdType::CT_PROGRESSING){
		last_cmd_type_ = status;
		if(ipc_data->master_process_id > 0){
			master_process_id_ = ipc_data->master_process_id;
		}
		if(status == CmdType::CT_IDLE){
			execMovieCommand(0, ipc_data->x, ipc_data->y, ipc_data->width, ipc_data->height);
		}else if (status == CmdType::CT_ACTIVE){            
			main_win_snapshot_path_ = ipc_data->win_snapshot_image_path;
            movie_label_->setDisplayProgress(ipc_data->has_detail_progress);			            
			execMovieCommand(1, ipc_data->x, ipc_data->y, ipc_data->width, ipc_data->height);
			ipc_data->active_finish = 1;
		}else if( status == CmdType::CT_PROGRESSING){
            updateProgressText(ipc_data->progress_text);
            movie_label_->setProgress(ipc_data->progress, received_progress_text_);            
        }else if(status == CmdType::CT_INACTIVE){
		}else if(status == CmdType::CT_START){
			master_process_id_ = ipc_data->master_process_id;
		}
	}
	shared_memory_->unlock();
}

void TaskProgressorView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape) {
        qDebug() << "ESC key pressed";
        exit(0);
        return;
    }
    QMainWindow::keyPressEvent(event); 
}