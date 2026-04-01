/*******************************************************
* author: scofieldzhu
* time:2025/12/18
*******************************************************/
#ifndef __tracker_tracking_data_h__
#define __tracker_tracking_data_h__

#include "helios/navigation/tool_tracking_data.h"

HELIOS_NAMESPACE_BEGIN

struct TrackerTrackingData
{    
    //bool isValid()const { 
    //    return connected && handpieceinfo.isvalid && (!referencer_enabled || referenceboardinfo.isvalid); 
    //}
    void reset(){
        connected = false;
        referencer_enabled = false;
        for(auto& td : tool_tracking_datas){
            td.reset();
        }
        marker_number = 0;
    }
    bool connected = false;        
    bool referencer_enabled = false;     

    ToolTrackingDataList tool_tracking_datas;
    int marker_number = 0;
    static const int kMaxTrackingMarkerNumber = 50;
    Point3 marker_positions[kMaxTrackingMarkerNumber];
};

NAMESPACE_END

#endif
