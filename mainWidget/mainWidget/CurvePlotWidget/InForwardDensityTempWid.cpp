#pragma execution_character_set("utf-8")
#include "InForwardDensityTempWid.h"


InForwardDensityTempWid::InForwardDensityTempWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void InForwardDensityTempWid::init()
{
	SetTitleName("真空环境注药速度温度耦合工程分析正向设计密度温度");
	SetXName("温度（℃）");
	SetYName("密度（%）");
}
