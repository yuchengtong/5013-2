#pragma execution_character_set("utf-8")
#include "PreForwardTimeTempWid.h"


PreForwardTimeTempWid::PreForwardTimeTempWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void PreForwardTimeTempWid::init()
{
	SetTitleName("预热工艺正向设计");
	SetXName("时间（s）");
	SetYName("温度（℃）");
}
