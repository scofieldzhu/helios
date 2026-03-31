/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/2/2
*******************************************************/
#ifndef __scene_layout_h__
#define __scene_layout_h__

#include <string_view>
#include <QString>
#include "mirfak/basic/mirfak_basic_typedef.h"

MIRFAK_NAMESPACE_BEGIN

class SceneLayout
{
public:
	void setBorder(double b){ border_ = b; } //used on maximized
	void addViewport(std::string_view v, Arry4d vp){
		scene_vp_dict_[v] = vp;
	}
	const QString& name()const{ return name_; }
	explicit SceneLayout(const QString& name)
		:name_(name)
	{}

private:
	friend class SceneLayoutManager;
	std::map<std::string_view, Arry4d> scene_vp_dict_;
	const QString name_;
	double border_ = 0.0; 
};

using SceneLayoutUPtr = std::unique_ptr<SceneLayout>;

NAMESPACE_END

#endif
