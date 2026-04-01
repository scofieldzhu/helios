/******************************************************** 
* author: scofieldzhu
* time:2025/11/24
*******************************************************/
#ifndef __plugin_manager_h__
#define __plugin_manager_h__

#include <QPluginLoader>
#include <QVector>
#include <QSharedPointer>
#include "helios/appbase/helios_appbase_export.h"

HELIOS_NAMESPACE_BEGIN

struct PluginRecord 
{
    QString path;              
    QString iid;               
    QString class_name;         
    QJsonObject meta;          
    QString display_name;     
    QString locale_display_name;  
    QString version;           
    QString vendor;            
    int sort_order = -1;
    QSharedPointer<QPluginLoader> loader;
    QObject* instance = nullptr;  
    bool loaded = false;
    QString err_str;        
};

class HELIOS_APPBASE_API PluginManager 
{    
public:    
    void addSearchPath(const QString& dir);
    QStringList searchPaths() const;
    void scan();
    QVector<PluginRecord> records() const;
    QVector<int> indexByIID(const QString& iid) const;
    QVector<int> indexByCapability(const QString& cap) const;
    bool load(int index);
    bool isLoaded(const QString& iid)const;
    bool unload(int index);
    void unloadAll();
    QObject* getObjectInstance(const QString& iid);
    QVector<QObject*> loadAllByIID(const QString& iid);
    QString lastError(int index) const;
    QStringList getPluginIIDList(bool only_loaded)const;
    QVector<QObject*> getLoadedPluginInstList()const;
    void dump() const;
    PluginManager() = default;
    ~PluginManager() = default;

private:    
    bool inRange(int index) const;
    QStringList paths_;
    QVector<PluginRecord> records_;
};

NAMESPACE_END

#endif