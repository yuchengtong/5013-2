#include "TimeTempPlotWidget.h"


TimeTempPlotWidget::TimeTempPlotWidget(QWidget* parent) : QWidget(parent)
{
    init();
    bindConnect();
}

void TimeTempPlotWidget::init()
{
    // 主布局
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 1. 创建绘图控件
    m_TimeTempPlot = new TimeTempPlot(this);
    m_ToolsAnimationWidget = new ToolsAnimationWidget();
   

    // 4. 组装
    layout->addWidget(m_TimeTempPlot);
    layout->addWidget(m_ToolsAnimationWidget);

    // 初始化假数据
    QVector<double> times, temps;
    for (int i = 0; i <= 1000; ++i)
    {
        double t = i * 0.01;
        times << t;
        // 模拟温度曲线
        temps << (20.0 + 5.0 * sin(t * 0.5) + 0.5 * t);
    }
    m_TimeTempPlot->AddDataPoint(times, temps);
}

void TimeTempPlotWidget::bindConnect()
{

}