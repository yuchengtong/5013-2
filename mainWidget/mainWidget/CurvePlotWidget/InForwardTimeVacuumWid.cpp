#pragma execution_character_set("utf-8")
#include "InForwardTimeVacuumWid.h"


InForwardTimeVacuumWid::InForwardTimeVacuumWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void InForwardTimeVacuumWid::init()
{
	SetTitleName("注药工艺正向设计时间真空度");
	SetXName("真空度（MPa）");
	SetYName("时间（s）");
}
