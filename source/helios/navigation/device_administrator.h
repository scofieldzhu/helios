/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/22
*******************************************************/
#ifndef __device_administrator_h__
#define __device_administrator_h__

#include "helios/navigation/tracker.h"

HELIOS_NAMESPACE_BEGIN

class RoboticArm;

class HELIOS_NAVIGATION_API DeviceAdministrator
{
public:
	static DeviceAdministrator& GetInst();
	bool init(const QString& config_file);
	Tracker* getDefaultTracker()const;
	RoboticArm* getDefaultRoboticArm()const;
	DeviceAdministrator(const DeviceAdministrator&) = delete;
	DeviceAdministrator& operator=(const DeviceAdministrator&) = delete;

private:
	DeviceAdministrator();
	~DeviceAdministrator();	
	std::unique_ptr<Tracker> active_tracker_;
};

NAMESPACE_END

#endif