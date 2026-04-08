#pragma once
#include <QWidget>
#include <QSlider>
#include "TimeTempPlot.h"
#include "ToolsAnimationWidget.h"

class TimeTempPlotWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TimeTempPlotWidget(QWidget* parent = nullptr);

	void init();
	void bindConnect();

private:
	TimeTempPlot* m_TimeTempPlot = nullptr;
	ToolsAnimationWidget* m_ToolsAnimationWidget = nullptr;
};

