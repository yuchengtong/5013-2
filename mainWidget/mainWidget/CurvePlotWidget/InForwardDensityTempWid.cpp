#pragma execution_character_set("utf-8")
#include "InForwardDensityTempWid.h"


InForwardDensityTempWid::InForwardDensityTempWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void InForwardDensityTempWid::init()
{
	SetTitleName("注药工艺正向设计密度温度");
	SetXName("温度（℃）");
	SetYName("密度（%）");
}
