/******************************************************** 
* author: scofieldzhu
* time:2026/1/26
*******************************************************/
#ifndef __process_view_h__
#define __process_view_h__

#include <QWidget>
#include "helios/approcess/helios_approcess_export.h"
#include "helios/approcess/helios_approcess_typedef.h"

HELIOS_NAMESPACE_BEGIN

class ProcessViewModel;

class HELIOS_APPROCESS_API ProcessView : public QWidget
{
	Q_OBJECT
public:
	void setViewModel(ProcessViewModel* vm);
	auto viewModel(){ return viewmodel_; }
	void setSharedRenderWidget(QWidget* rw){ shared_rw_ = rw; }
	auto sharedRenderWidget()const{ return shared_rw_; }
	void setTitle(const QString& title);
	const QString& title()const{ return title_; }
	virtual ProcessIIDType processIID()const = 0;
	ProcessView(QWidget* p);
	virtual ~ProcessView();

protected slots:
	virtual void onSessionActivate();
	virtual void onSessionDeactivate();
	virtual void onSessionEnter();
	virtual void onSessionLeave();
	virtual void onSessionDead();

protected:
	void addSignalConnection(QMetaObject::Connection c);
	void releaseActiveConnections();
	virtual void onViewModelSet();
	QWidget* shared_rw_ = nullptr;
	ProcessViewModel* viewmodel_ = nullptr;
	QString title_ = "Untitled";
	QVector<QMetaObject::Connection> sig_conns_;
};

NAMESPACE_END

#endif
