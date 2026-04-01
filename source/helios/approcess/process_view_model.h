/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/1/26
*******************************************************/
#ifndef __processViewModel_h__
#define __processViewModel_h__

#include <QObject>
#include "helios/approcess/helios_approcess_export.h"
#include "helios/approcess/helios_approcess_typedef.h"

HELIOS_NAMESPACE_BEGIN

class HELIOS_APPROCESS_API ProcessViewModel : public QObject
{
	Q_OBJECT

signals:
	void sessionEnter();
	void sessionActivate();
	void sessionDeactivate();
	void sessionLeave();
	void sessionDead();

public:
	void requestRenderSceneViews();
	ProcessIIDType processIID()const;	
	~ProcessViewModel();

protected:
	ProcessViewModel(QObject* p, ProcessSession& s);
	ProcessSessionContext* processSessionContext()const;	
	ProcessSession& process_session_;
};

NAMESPACE_END

#endif
