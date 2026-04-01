/******************************************************** 
* author: scofieldzhu
* time:2026/3/5
*******************************************************/
#ifndef __task_progressor_controller_h__
#define __task_progressor_controller_h__

#include "helios/appbase/helios_appbase_export.h"
#include "helios/appbase/progress_control_view.h"

class QSharedMemory;
class QWidget;

HELIOS_NAMESPACE_BEGIN

class HELIOS_APPBASE_API TaskProgressorController final : public ProgressControlView 
{
public:
    static TaskProgressorController* GetInstance();    
    bool init(QWidget* master_window, const QString& snapshot_save_location, const QString& progressor_filename);
    void destroy();
    void takeSnapshot() override;
    void start(bool has_progress, bool auto_snapshot) override;
    void tellProgressing(int p, const QString& text) override;
    bool isStarted()const { return started_; }
    void stop() override;    
    TaskProgressorController(const TaskProgressorController&) = delete;
    TaskProgressorController& operator=(const TaskProgressorController&) = delete;     

private:
    TaskProgressorController();
    ~TaskProgressorController();    
    void startIPC();
    void startProgressorProcess(const QString& abs_progressor_file_path);
    QString getSnapshotImagePath()const;
    QString getProgressorAbsFilePath(const QString& fn)const;
    static TaskProgressorController* stInstance_;
	QSharedMemory* ipc_shm_ = nullptr;
    bool started_ = false;
    QString snapshot_save_dir_;
    QWidget* master_window_ = nullptr;
};

NAMESPACE_END

#endif
