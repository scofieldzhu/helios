/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/2/25
*******************************************************/
#ifndef __process_factory_interface_h__
#define __process_factory_interface_h__

#include <memory>
#include <QObject>
#include "helios/helios_nsp.h"

class QWidget;

HELIOS_NAMESPACE_BEGIN

class Session;
class ProcessSession;
class ProcessViewModel;
class ProcessView;

class IProcessFactory
{
public:
	virtual std::unique_ptr<ProcessSession> createSession(Session* parent) = 0;
	virtual std::unique_ptr<ProcessViewModel> createViewModel(ProcessSession& s) = 0;
	virtual ProcessView* createView(QWidget* parent) = 0;
	virtual ~IProcessFactory() = default;
};

NAMESPACE_END

#define IProcessFactory_IID "com.helios.plugin.IProcessFactory/1.0"

Q_DECLARE_INTERFACE(helios::IProcessFactory, IProcessFactory_IID)

#endif