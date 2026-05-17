#pragma execution_character_set("utf-8")
#include "InForwardDensityVacuumWid.h"


InForwardDensityVacuumWid::InForwardDensityVacuumWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void InForwardDensityVacuumWid::init()
{
	SetTitleName("注药工艺正向设计");
	SetXName("密度（%）");
	SetYName("真空度（MPa）");
}
