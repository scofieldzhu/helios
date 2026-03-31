/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/3/4
*******************************************************/
#include "task_progressor_view.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	TaskProgressorView w;
	w.hide();
	return a.exec();
}
