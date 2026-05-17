#pragma execution_character_set("utf-8")
#include "InForwardDensityValveWid.h"


InForwardDensityValveWid::InForwardDensityValveWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void InForwardDensityValveWid::init()
{
	SetTitleName("注药工艺正向设计");
	SetXName("密度（%）");
	SetYName("阀门开度（mm）");
}
