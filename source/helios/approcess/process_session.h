/******************************************************** 
* author: scofieldzhu
* time:2026/1/22
*******************************************************/
#ifndef __process_session_h__
#define __process_session_h__

#include "helios/appbase/session.h"
#include "helios/approcess/process_session_context.h"
#include "helios/approcess/helios_approcess_export.h"

HELIOS_NAMESPACE_BEGIN

class MetaDataSerializer;

class HELIOS_APPROCESS_API ProcessSession : public helios::Session
{
	SESSION_DECL(ProcessSession, helios::Session)
public:	
	ProcessSessionContext* processContext()const;
	virtual ProcessIIDType processIID()const = 0;
	virtual MetaDataSerializer* getDataSerializer() = 0;
	virtual ~ProcessSession();

protected:
	ProcessSession(Session* parent);
	virtual bool handleEnterAction() = 0;
	virtual void handleLeaveAction() = 0;
	virtual void handleDeadAction() = 0;
	virtual bool handleActivateAction() = 0;
	virtual void handleDeactivateAction() = 0;

private:
	bool onEnter() override final;
	bool onActivate() override final;
	void onDeactivate() override final;
	void onLeave() override final;
	void onDead() override final;
};

NAMESPACE_END

#endif
