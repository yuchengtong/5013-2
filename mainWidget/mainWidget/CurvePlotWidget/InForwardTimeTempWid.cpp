#pragma execution_character_set("utf-8")
#include "InForwardTimeTempWid.h"


InForwardTimeTempWid::InForwardTimeTempWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void InForwardTimeTempWid::init()
{
	SetTitleName("注药工艺正向设计时间温度");
	SetXName("温度（℃）");
	SetYName("时间（s）");
}
