#pragma execution_character_set("utf-8")
#include "PreForwardTimeTempWid.h"


PreForwardTimeTempWid::PreForwardTimeTempWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void PreForwardTimeTempWid::init()
{
	SetTitleName("预热工艺正向设计时间温度");
	SetXName("温度（℃）");
	SetYName("时间（s）");
}
