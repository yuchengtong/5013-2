#pragma once
#include <QWidget>
#include "TimeTempPlot.h"
#include <QSlider>

class TimeTempPlotWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TimeTempPlotWidget(QWidget* parent = nullptr);

	void init();
	void bindConnect();

private:
	TimeTempPlot* m_TimeTempPlot = nullptr;
	QSlider* m_Slider = nullptr;

};

