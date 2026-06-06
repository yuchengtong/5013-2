#pragma execution_character_set("utf-8")
#include "InForwardMassTimeWid.h"


InForwardMassTimeWid::InForwardMassTimeWid(QWidget* parent) : BaseCurvePlotWidget(parent)
{
	init();
}

void InForwardMassTimeWid::init()
{
	SetTitleName("注药工艺正向设计注药质量时间");
	SetXName("时间（s）");
	SetYName("注药质量（Kg）");
}
