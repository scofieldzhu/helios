/******************************************************** 
* author: scofieldzhu
* time:2026/1/30
*******************************************************/
#ifndef __scene_layout_manager_h__
#define __scene_layout_manager_h__

#include <QObject>
#include "helios/appbase/scene_layout.h"
#include "helios/appbase/helios_appbase_export.h"

HELIOS_NAMESPACE_BEGIN

class SceneRepository;

class HELIOS_APPBASE_API SceneLayoutManager : public QObject
{
	Q_OBJECT

public:
	void maximizeScene(const QString& scene_name);
	const QString& maximizedScene()const{ return maximized_scene_name_; }
	void restoreScene();
	void updateActiveLayout();
	void makeActiveLayout(const QString& layout_name);
	const QString& activeLayoutName()const{ return active_layout_name_; }
	void setSceneRepository(const SceneRepository* s);
	SceneLayout* getLayout(const QString& name)const;
	void addLayout(SceneLayoutUPtr layout);
	SceneLayoutManager(QObject* parent = nullptr);

signals:
	void activeLayoutUpdated();

public slots:
	void onMinmaxSceneView(const QString& name, bool maxmized);

private:
	void applyLayout(SceneLayout* layout)const;
	const SceneRepository* scene_repos_ = nullptr;
	std::vector<SceneLayoutUPtr> layouts_;
	QString active_layout_name_;
	QString maximized_scene_name_;
};

NAMESPACE_END

#endif
