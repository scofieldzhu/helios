/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/11/24
*******************************************************/
#include "plugin_manager.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QLocale>
#include "mirfak/basic/log_service.h"
#include "log_misc.h"

MIRFAK_NAMESPACE_BEGIN

namespace{
	QStringList FindPluginFiles(const QString& dir_path)
	{
	#if defined(Q_OS_WIN)
		#ifdef BUILD_TYPE_DEBUG
			const QString pattern = "*-d.dll";
		#elif BUILD_TYPE_RELWITHDEBINFO
			const QString pattern = "*-rd.dll";
		#elif BUILD_TYPE_RELEASE
			const QString pattern = "*.dll";
		#endif
	#elif defined(Q_OS_MAC)
		const QString pattern = "*.dylib";
	#else
		const QString pattern = "*.so";
	#endif
		QDir dir(dir_path);
		auto files = dir.entryList({pattern}, QDir::Files);
	#ifdef BUILD_TYPE_RELEASE
		files.erase(
			std::remove_if(files.begin(), files.end(),
				[](const QString& file) {
					return file.endsWith("-rd.dll", Qt::CaseInsensitive) ||
						file.endsWith("-d.dll",  Qt::CaseInsensitive);
				}),
			files.end()
		);
	#endif
		return files;
	}

	QString LocalizedField(const QJsonObject& meta, const QString& base_key, const QLocale& locale = QLocale::system())
	{
		auto strs =  meta.keys();
		//for(auto s : strs){
		//    SPDLOG_INFO("s:{}", s.toStdString());
		//}
		const QString i18n_key = base_key + ".i18n";
		const QJsonObject i18n = meta.value(i18n_key).toObject();
		if(!i18n.isEmpty()){
			const QString full = locale.name(); // "zh_CN"
			if (i18n.contains(full)){
				return i18n.value(full).toString();
			}
		}
		return "";
	}

	PluginRecord NewPluginRecord(const QString& abs_path)
	{
		PluginRecord r;
		r.path = abs_path;
		r.loader.reset(new QPluginLoader(abs_path));
		const auto root_meta = r.loader->metaData(); 
		r.iid = root_meta.value("IID").toString();
		r.class_name = root_meta.value("className").toString();
		auto meta_data_obj = root_meta.value("MetaData").toObject();
		r.meta = meta_data_obj;
		r.display_name = meta_data_obj.value("DisplayName").toString();
		r.version = meta_data_obj.value("Version").toString();
		r.vendor = meta_data_obj.value("Vendor").toString();
		if(r.display_name.isEmpty()){
			r.display_name = QFileInfo(abs_path).baseName();
		}
		auto localized_display_name = LocalizedField(meta_data_obj, "DisplayName");
		if(!localized_display_name.isEmpty()){
			r.locale_display_name = localized_display_name;
		}else{
			r.locale_display_name = r.display_name;
		}
		if(meta_data_obj.contains("SortOrder")){
			r.sort_order = meta_data_obj.value("SortOrder").toInt();
		}
		return r;
	}
}

void PluginManager::addSearchPath(const QString& dir)
{
	QDir d(dir);
	if(d.exists()){
		const QString abs = d.absolutePath();
		if(!paths_.contains(abs)){
			paths_.push_back(abs); 
		}
	}
}

QStringList PluginManager::searchPaths() const
{
	return paths_;
}

void PluginManager::scan()
{
	records_.clear ();
	for(const auto& dir : paths_){
		for(const QString& f : FindPluginFiles(dir)){
			const QString absPath = QDir(dir).absoluteFilePath(f);
			auto rec = NewPluginRecord(absPath);
			records_.push_back(rec);
		}
	}
	std::sort(records_.begin(), records_.end(), [](const PluginRecord& lhs, const PluginRecord& rhs){
		return lhs.sort_order < rhs.sort_order;
	});
}

QVector<PluginRecord> PluginManager::records() const
{
	return records_;
}

QVector<int> PluginManager::indexByIID(const QString& iid) const
{
	QVector<int> idx;
	for(int i = 0; i < records_.size(); ++i){
		if(records_[i].iid == iid){
			idx.push_back (i);
		}
	}
	return idx;
}

QVector<int> PluginManager::indexByCapability(const QString& cap) const
{
	QVector<int> idx;
	for(int i = 0; i < records_.size(); ++i){
		const auto caps = records_[i].meta.value("Capabilities").toArray();
		for(const auto& v : caps){
			if(v.toString() == cap){ 
				idx.push_back(i); 
				break; 
			}
		}
	}
	return idx;
}

bool PluginManager::load(int index)
{
	if(!inRange(index)){
		return false;
	}
	auto& r = records_[index];
	if(r.loaded && r.instance){
		return true;
	}
	if(!r.loader){
		r.loader.reset(new QPluginLoader(r.path));
	}
	QObject* obj = r.loader->instance();
	if(!obj){
		r.loaded = false;
		r.instance = nullptr;
		r.err_str = r.loader->errorString();
		return false;
	}
	r.loaded = true;
	r.instance = obj;
	r.err_str.clear();
	return true;
}

bool PluginManager::unload(int index)
{
	if(!inRange(index)){
		return false;
	}
	auto& r = records_[index];
	if(!r.loader){
		return true; 
	}
	if(!r.loaded){
		return true;
	}
	const bool ok = r.loader->unload();
	if(ok){
		r.loaded = false;
		r.instance = nullptr;
	}else{
		r.err_str = r.loader->errorString();
	}
	return ok;
}

void PluginManager::unloadAll()
{
	for(auto i = 0; i < records_.size(); ++i){
		unload(i);
	}
}

QVector<QObject*> PluginManager::loadAllByIID(const QString& iid)
{
	QVector<QObject*> out;
	for(int idx : indexByIID(iid)){
		if(load(idx)){
			out.push_back(records_[idx].instance);
		}else{
			SPDLOG_WARN("Load plugin file:\"{}\" failed! detail reason:{}.", QStrToLogStr(records_[idx].path), QStrToLogStr(records_[idx].err_str));
		}
	}
	return out;
}

QString PluginManager::lastError(int index) const
{
	if(!inRange(index)){
		return "index out of range";
	}
	return records_[index].err_str;
}

void PluginManager::dump() const
{
	spdlog::info("Plugin count:{}", records_.size());
	for(const auto& r : records_){
		spdlog::info(QStrToLogStr(QString("- %1 | IID=%2 | Name=%3 | Ver=%4 | Vendor=%5").arg(r.path, r.iid, r.display_name, r.version, r.vendor)));
	}
}

bool PluginManager::inRange(int index) const
{
	return index >= 0 && index < records_.size();
}

bool PluginManager::isLoaded(const QString& iid) const
{
	auto indexes = indexByIID(iid);
	if(indexes.empty()){
		return false;
	}
	auto idx = indexes[0];
	auto& r = records_[idx];
	return r.loaded && r.instance;
}

QObject* PluginManager::getObjectInstance(const QString& iid)
{
	auto indexes = indexByIID(iid);
	if(indexes.empty()){
		return nullptr;
	}
	auto idx = indexes[0];
	return records_[idx].instance;
}

QStringList PluginManager::getPluginIIDList(bool only_loaded) const
{
	QStringList iid_list;
	for(auto& rd : records_){
		if(only_loaded){
			if(rd.loaded && rd.instance){
				iid_list.push_back(rd.iid);
			}
		}else{
			iid_list.push_back(rd.iid);
		}
	}
	return iid_list;
}

QVector<QObject*> PluginManager::getLoadedPluginInstList() const
{
	QVector<QObject*> loaded_insts;
	for(auto& rd : records_){
		if(rd.loaded && rd.instance){
			loaded_insts.push_back(rd.instance);
		}
	}
	return loaded_insts;
}

NAMESPACE_END
