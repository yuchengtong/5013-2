#include "TimeTempPlotWidget.h"


TimeTempPlotWidget::TimeTempPlotWidget(QWidget* parent) : QWidget(parent)
{
    init();
    bindConnect();
}

void TimeTempPlotWidget::init()
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    {
        m_TimeTempPlot = new TimeTempPlot(this);
        m_TimeTempPlot->setFixedHeight(350);
        m_ToolsAnimationWidget = new ToolsAnimationWidget();
    }
    layout->addWidget(m_TimeTempPlot);
    layout->addWidget(m_ToolsAnimationWidget);
    layout->addStretch(1);

    QVector<double> times, temps;
    for (int i = 0; i <= 1000; ++i)
    {
        double t = i * 0.01;
        times << t;
        temps << (20.0 + 5.0 * sin(t * 0.5) + 0.5 * t);
    }
    m_TimeTempPlot->AddDataPoint(times, temps);
}

void TimeTempPlotWidget::bindConnect()
{

}