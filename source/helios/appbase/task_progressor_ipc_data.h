/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/3/4
*******************************************************/
#ifndef __task_progressor_ipc_data_h__
#define __task_progressor_ipc_data_h__

enum CmdType
{
	CT_UNK,
	CT_IDLE,
	CT_ACTIVE,
	CT_START,
	CT_INACTIVE,
    CT_PROGRESSING,
	CT_EXIT
};

struct TaskProgressorIpcData
{
    CmdType cmd_type = CT_IDLE;  
	int64_t master_process_id = 0;
	int active_finish = 0;//
	int x = 0; 
	int y = 0; 
	int width = 0;
	int height = 0;
	char win_snapshot_image_path[256] = {'\0'};//截图保存路径
    int progress = 0;
    int has_detail_progress = 0;
    wchar_t progress_text[256] = {L'\0'};
    
    void construct()
    {
        cmd_type = CT_IDLE;
        master_process_id = 0;
		active_finish = 0;
		x = y = width = height = 0;
		memset(win_snapshot_image_path, 0, sizeof(win_snapshot_image_path) * sizeof(char));
        memset(progress_text, 0, sizeof(progress_text) * sizeof(wchar_t));
        progress = 0;
        has_detail_progress = 0;
    }
};

inline const char* kTaskProgressorSHMName = "__sysbot_taskprogressor__";

#endif
