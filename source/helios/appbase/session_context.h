/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/1/15
*******************************************************/
#ifndef __session_context_h__
#define __session_context_h__

#include "helios/basic/mobject.h"

HELIOS_NAMESPACE_BEGIN

class SessionContext : public MObject
{
public:
	virtual ~SessionContext() = default;
};

NAMESPACE_END

#define SESSION_CONTEXT_DECL(TheClass, SuperClass) MOBJECT_DECL(TheClass, SuperClass, helios::SessionContext)

#endif
