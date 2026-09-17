#pragma execution_character_set("utf-8")
#include "InForwardDensityVacuumWid.h"


InForwardDensityVacuumWid::InForwardDensityVacuumWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void InForwardDensityVacuumWid::init()
{
	SetTitleName("真空环境注药速度温度耦合工程分析正向设计密度真空度");
	SetXName("真空度（MPa）");
	SetYName("密度（%）");
}
