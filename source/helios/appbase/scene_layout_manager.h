/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/1/30
*******************************************************/
#ifndef __scene_layout_manager_h__
#define __scene_layout_manager_h__

#include <QObject>
#include "mirfak/appbase/scene_layout.h"
#include "mirfak/appbase/mirfak_appbase_export.h"

MIRFAK_NAMESPACE_BEGIN

class SceneRepository;

class MIRFAK_APPBASE_API SceneLayoutManager : public QObject
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
