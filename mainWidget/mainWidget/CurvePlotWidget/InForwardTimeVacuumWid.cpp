#pragma execution_character_set("utf-8")
#include "InForwardTimeVacuumWid.h"


InForwardTimeVacuumWid::InForwardTimeVacuumWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void InForwardTimeVacuumWid::init()
{
	SetTitleName("注药工艺正向设计");
	SetXName("时间（s）");
	SetYName("真空度（MPa）");
}
