/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2023)
*******************************************************/
#include "task_progressor_controller.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <TLHELP32.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QSharedMemory> 
#include <QDir>
#include <QProcess>
#include <QPainter>
#include "task_progressor_ipc_data.h"
#include "helios/basic/log_service.h"
#include "helios/appbase/log_misc.h"

namespace{
    void GrabWidgetSnapshot(QWidget* widget, const QString& saved_path)
    {
        QScreen *screen = QGuiApplication::primaryScreen();
        QList<QScreen *>screens = QGuiApplication::screens();
        std::vector<QScreen*> screenVec;
        for (int i = 0; i < screens.size(); i++){
            screenVec.push_back(screens.at(i));
        }
        const auto& w_rt = widget->geometry();
        QPixmap newImage(w_rt.width(), w_rt.height());  
        QPainter p(&newImage);
        int startx = 0;
        std::sort(screenVec.begin(), screenVec.end(), [](const QScreen* lhs, const QScreen* rhs){
            return lhs->geometry().left() < rhs->geometry().left(); 
        });
        for(int i = 0; i < screenVec.size(); i++){
            QRect s_rt = screenVec[i]->geometry();
            //LOG_INFO("rect %d:%d %d %d %d", i, s_rt.x(), s_rt.y(), s_rt.width(), s_rt.height());
            if (w_rt.right() < s_rt.left() || w_rt.left() > s_rt.right()){
                continue;
            }
            QPixmap map = screenVec[i]->grabWindow(0);
            if (w_rt.left() >= s_rt.left() && w_rt.right() <= s_rt.right()){
                int leftstart = w_rt.left() - s_rt.left();
                int topstart = w_rt.top() - s_rt.top();
                //int width = w_rt.width() - (w_rt.left() - s_rt.left()) - (w_rt.right()-s_rt.right());
                QRect rectmp = QRect(leftstart, topstart, w_rt.width(), w_rt.height());
                QPixmap cropped = map.copy(rectmp);
                p.drawPixmap(0,0, cropped);
                break;
            }else if (w_rt.left() >= s_rt.left() && w_rt.left() < s_rt.right()&& w_rt.right() > s_rt.right()){
                int leftstart = w_rt.left() - s_rt.left();
                int width = s_rt.width() - leftstart;
                QRect rectmp = QRect(leftstart, w_rt.top(), width, w_rt.height());
                QPixmap cropped = map.copy(leftstart, w_rt.top(), width, w_rt.height());
                p.drawPixmap(0, 0, cropped);
                startx += width;
            }else if (w_rt.left() < s_rt.left() && w_rt.right() > s_rt.left()){
                int width = w_rt.right() - s_rt.left();
                QRect rectmp = QRect(0, w_rt.top(), width, w_rt.height());
                QPixmap cropped = map.copy(0, w_rt.top(), width, w_rt.height());
                p.drawPixmap(startx, 0, cropped);
            }else if (w_rt.left() < s_rt.left() && w_rt.right() > s_rt.right()){
                int width = s_rt.left();
                QRect rectmp = QRect(0, w_rt.top(), width, w_rt.height());
                QPixmap cropped = map.copy(0, w_rt.top(), width, w_rt.height());
                p.drawPixmap(startx, 0, cropped);
                startx += width;
            }
        }
        if(!newImage.isNull()&& !newImage.save(saved_path, "png")){
            SPDLOG_ERROR("save failed!\n");
            return;
        }
        SPDLOG_TRACE("save sucess! path:{}", helios::QStrToLogStr(saved_path)) ;
    }

    bool CheckAliveProcessex(const char* target_proc_name)
    {
        PROCESSENTRY32 proc_entry;
        HANDLE snaphot_handle = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if(snaphot_handle == (HANDLE)-1) {
            SPDLOG_ERROR("CreateToolhelp32Snapshot failed! errcode:{}", GetLastError());
            return FALSE;
        }
        proc_entry.dwSize = sizeof(proc_entry);
        if(Process32First(snaphot_handle, &proc_entry)){
            do{
                if(strcmp(target_proc_name, &(proc_entry.szExeFile[0])) == 0){
                    return TRUE;
                }
            }while(Process32Next(snaphot_handle, &proc_entry));
        }
        return FALSE;
    }

    const QString kSnapshotImageFileName = "snapshot.png";
}

typedef bool(*WFun)(const char* p);
void WaitFunUntil(WFun f, const  char* p, uint64_t millsec)
{
    while(true){
        if(!f(p)){
            ::Sleep(millsec);
            continue;
        }
        break;
    }
}

void WaitNotFunUntil(WFun f, const char* p, uint64_t millsec)
{
    while(true){
        if(f(p)){
            ::Sleep(millsec);
            continue;
        }
        break;
    }
}

HELIOS_NAMESPACE_BEGIN

TaskProgressorController* TaskProgressorController::stInstance_ = nullptr;

TaskProgressorController::TaskProgressorController()
{
    ipc_shm_ = new QSharedMemory();
    ipc_shm_->setKey(kTaskProgressorSHMName);
}

TaskProgressorController::~TaskProgressorController()
{
	destroy();
	if(ipc_shm_){
		delete ipc_shm_;
	}
}

TaskProgressorController* TaskProgressorController::GetInstance()
{
    if(stInstance_ == nullptr){
        stInstance_ = new TaskProgressorController();
    }
    return stInstance_;
}

QString TaskProgressorController::getProgressorAbsFilePath(const QString& fn)const
{
    QDir app_dir = QCoreApplication::applicationDirPath();
    QString progressor_abs_path = app_dir.absoluteFilePath(fn);
    QFileInfo fi(progressor_abs_path);
    if(!fi.exists() || !fi.isFile()){
        return "";
    }
    return progressor_abs_path;
}

bool TaskProgressorController::init(QWidget* master_window, const QString& snapshot_save_location, const QString& progressor_filename)
{   
    if(master_window == nullptr){
        SPDLOG_ERROR("Null master window pointer detected!");
        return false;
    }    
    if(!QFileInfo(snapshot_save_location).isDir()){
        SPDLOG_ERROR("Given snapshot image save location(\"{}\") is invalid directory!", helios::QStrToLogStr(snapshot_save_location));
        return false;
    }
    QString progressor_path = getProgressorAbsFilePath(progressor_filename);
    if(progressor_path.isEmpty()){
        SPDLOG_ERROR("Progressor execute file(\"{}\") not exists!", helios::QStrToLogStr(progressor_filename));
        return false;
    }
    snapshot_save_dir_ = snapshot_save_location;
    master_window_ = master_window;
    destroy();//release last ipc resources
    startIPC();
    startProgressorProcess(progressor_path);
	WaitFunUntil(&(CheckAliveProcessex), progressor_filename.toStdString().c_str(), 10); //wait until start up actually    
    SPDLOG_TRACE("init OK!");
    return true;
}

void TaskProgressorController::startIPC()
{	
    if(ipc_shm_->attach()){
        ipc_shm_->detach();
    }
	if(!ipc_shm_->create(sizeof(TaskProgressorIpcData))){
		SPDLOG_ERROR("create shared memory failed! detail reason: {} \n", helios::QStrToLogStr(ipc_shm_->errorString()));
		return;
	}
    ipc_shm_->lock();
	auto exchange_data = static_cast<TaskProgressorIpcData*>(ipc_shm_->data());
	if(exchange_data){
		exchange_data->construct();
		exchange_data->cmd_type = CmdType::CT_START;
		exchange_data->master_process_id = _getpid();
        QString snapshot_image_path = getSnapshotImagePath();
		strcpy(exchange_data->win_snapshot_image_path, snapshot_image_path.toStdString().c_str());
	}
    ipc_shm_->unlock();
}

void TaskProgressorController::startProgressorProcess(const QString& abs_progressor_file_path)
{
    qint64 pid = 0;
    QStringList start_arguments;
    const bool ok = QProcess::startDetached(
        abs_progressor_file_path,
        start_arguments,
        QString(),   // workingDirectory, optional
        &pid
    );
    if(!ok){
        SPDLOG_ERROR("Start progressor process failed! program:\"{}\"", helios::QStrToLogStr(abs_progressor_file_path));
    }else{
        SPDLOG_INFO("Start progressor process successfully! pid:{}", pid);
    }
}

QString TaskProgressorController::getSnapshotImagePath() const
{
    return QDir(snapshot_save_dir_).filePath(kSnapshotImageFileName);
}

void TaskProgressorController::destroy()
{
    if(ipc_shm_->isAttached()) {
        if(!ipc_shm_->detach()) {
            SPDLOG_WARN("Detach failed! error:{}", helios::QStrToLogStr(ipc_shm_->errorString()));
        }
    }
}

void TaskProgressorController::takeSnapshot()
{
    auto snapshot_image_path = getSnapshotImagePath();
    GrabWidgetSnapshot(master_window_, snapshot_image_path);
}

void TaskProgressorController::start(bool has_progress, bool snapshot)
{   
    if(started_){
        return;
    }
    if(snapshot){
        takeSnapshot();
    }
	QRect rect = master_window_->geometry();                          
	ipc_shm_->lock();
    auto exchange_data = static_cast<TaskProgressorIpcData*>(ipc_shm_->data());
	if(exchange_data){
		exchange_data->x = rect.x();
		exchange_data->y = rect.y();
		exchange_data->width = rect.width();
		exchange_data->master_process_id = _getpid();
		exchange_data->height = rect.height();
		exchange_data->active_finish = 0;
        exchange_data->has_detail_progress = has_progress ? 1 : 0;
		exchange_data->cmd_type = CmdType::CT_ACTIVE;
	}
	ipc_shm_->unlock();
	while(true){ //wait until finished
		if(exchange_data && exchange_data->active_finish == 1){
		    break;
        }
		::Sleep(10);
	}
    started_ = true;
}

void TaskProgressorController::tellProgressing(int p, const QString& text)
{
    if(!started_){
        return;
    }
    ipc_shm_->lock();
    auto exchange_data = static_cast<TaskProgressorIpcData*>(ipc_shm_->data());
	if(exchange_data){
		exchange_data->cmd_type = CmdType::CT_PROGRESSING;
		exchange_data->master_process_id = _getpid();
        exchange_data->progress = p;
        auto wtext = text.toStdWString();
        wmemset(exchange_data->progress_text, 0, 256);
        if(!text.isEmpty()){            
            wmemcpy(exchange_data->progress_text, wtext.c_str(), wtext.size());
        }        
        exchange_data->progress_text[wtext.size()] = 0;
	}
	ipc_shm_->unlock();
    //qDebug() << "tellProgressing:" << p << " text:" << text << "\n";
}

void TaskProgressorController::stop()
{
    //LOG_INFO("enter stop!");
    if(!started_){
        return;
    }
	ipc_shm_->lock();
    auto exchange_data = static_cast<TaskProgressorIpcData*>(ipc_shm_->data());
	if(exchange_data){
		exchange_data->cmd_type = CmdType::CT_IDLE;
		exchange_data->master_process_id = _getpid();
	}
	ipc_shm_->unlock();
    started_ = false;
}

NAMESPACE_END
