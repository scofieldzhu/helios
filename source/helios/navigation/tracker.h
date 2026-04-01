/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/18
*******************************************************/
#ifndef __tracker_h__
#define __tracker_h__

#include <QObject>
#include <vtkTracker.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include "helios/navigation/tracker_config.h"
#include "helios/navigation/tracker_tracking_data.h"
#include "helios/navigation/helios_navigation_export.h"

class QTimer;

HELIOS_NAMESPACE_BEGIN

class HELIOS_NAVIGATION_API Tracker : public QObject
{
    Q_OBJECT

signals:
    void registrationMatrixUpdated();
    void pollingTimeout();
    void trackingStarted();
    void trackingStopped();
    
public:    
    const auto& currentTrackingData()const { return tracking_data_; }
    const auto& config()const{ return config_; }
    bool checkReachable();
    bool isReachable() const { return reachable_; }
    bool isTracking()const;
    bool startTracking(bool start_timer_polling = true);
    bool startTimerPolling();
    void setTimerPollingInterval(int millsecs);
    bool isTimerPolling()const;
    void stopTimerPolling();
    void stopTracking();
    void updateRegistrationMatrix(vtkMatrix4x4* matrix);
    vtkMatrix4x4* getRegistrationMatrix();
    bool isRegistrationDone()const;
    void enableReferencer(const QString& name);
    void disableReferencer();
    bool isReferencerEnabled()const;
    TrackerToolStatus getToolStatus(TrackerToolType t, int id = 0)const;        
    vtkTrackerTool* getTrackerTool(TrackerToolType t, int id = 0)const;    
    vtkTrackerTool* getTrackerTool(const QString& name)const;
    double getToolError(TrackerToolType t, int id = 0)const;
    //vtkSmartPointer<vtkTransform> getTransformBetweenReferenceAndLocator(bool ref_to_locator);
    explicit Tracker(const TrackerConfig& cf);
    Tracker(const Tracker&) = delete;
    Tracker& operator=(const Tracker&) = delete;
    ~Tracker();

private slots:
    void handleTimeOut();

private:
    TrackerToolStatus getToolStatusById(int id)const;
    TrackerToolStatus getToolStatusByPort(int port)const;
    void createTrackerObject();
    void loadToolConfigs();
    void unloadToolConfigs();    
    void updateTrackingData();
    TrackerConfig config_;
    TrackerTrackingData tracking_data_;
    vtkSmartPointer<vtkTracker> vtk_tracker_;
    bool reachable_ = false;    
    QTimer* timer_ = nullptr;
    int polling_interval_ = 50;
};

NAMESPACE_END

#endif