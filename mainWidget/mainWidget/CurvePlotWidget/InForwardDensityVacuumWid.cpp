#pragma execution_character_set("utf-8")
#include "InForwardDensityVacuumWid.h"


InForwardDensityVacuumWid::InForwardDensityVacuumWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void InForwardDensityVacuumWid::init()
{
	SetTitleName("注药工艺正向设计密度真空度");
	SetXName("真空度（MPa）");
	SetYName("密度（%）");
}
