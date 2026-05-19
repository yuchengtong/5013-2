#pragma execution_character_set("utf-8")
#include "InForwardTimeValveWid.h"


InForwardTimeValveWid::InForwardTimeValveWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void InForwardTimeValveWid::init()
{
	SetTitleName("注药工艺正向设计时间阀门开度");
	SetXName("阀门开度（mm）");
	SetYName("时间（s）");
}
