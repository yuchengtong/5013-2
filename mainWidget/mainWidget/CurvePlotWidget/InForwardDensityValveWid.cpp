#pragma execution_character_set("utf-8")
#include "InForwardDensityValveWid.h"


InForwardDensityValveWid::InForwardDensityValveWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void InForwardDensityValveWid::init()
{
	SetTitleName("真空环境注药速度温度耦合工程分析正向设计密度阀门开度");
	SetXName("阀门开度（mm）");
	SetYName("密度（%）");
}
