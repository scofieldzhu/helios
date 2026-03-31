/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/3/5
*******************************************************/
#ifndef __progress_control_view_h__
#define __progress_control_view_h__

#include "mirfak/mirfak_nsp.h"
#include <QString>

MIRFAK_NAMESPACE_BEGIN

class ProgressControlView
{
public:
	virtual void takeSnapshot() = 0;
	virtual void start(bool has_progress, bool auto_snapshot) = 0;
	virtual void tellProgressing(int p, const QString& text) = 0;
	virtual void stop() = 0;    
	virtual ~ProgressControlView() = default;
};

NAMESPACE_END

#endif