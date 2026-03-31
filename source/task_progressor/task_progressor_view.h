/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/3/4
*******************************************************/
#ifndef __task_progressor_view_h__
#define __task_progressor_view_h__

#include <QMainWindow>
#include "task_progressor_ipc_data.h"

class MovieLabel;
class QSharedMemory;

class TaskProgressorView : public QMainWindow
{
	Q_OBJECT

public:
	TaskProgressorView(QWidget* parent = Q_NULLPTR);
	~TaskProgressorView();

private:
	void startPlayingMovie();
	void stopMoviePlaying();
	void execMovieCommand(int flag, int x, int y, int width, int height);
	void timerEvent(QTimerEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
    void updateProgressText(const wchar_t* wstr);
	bool checkProcessAlive(__int64 processid);	
	void processIPCData();
	int64_t master_process_id_ = 0;
	MovieLabel* movie_label_ = nullptr;
	QString main_win_snapshot_path_; 
	QSharedMemory* shared_memory_ = nullptr;
	CmdType last_cmd_type_ = CmdType::CT_UNK; 
    QString received_progress_text_;
};

#endif
