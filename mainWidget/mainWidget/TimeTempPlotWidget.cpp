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

    // 2. 创建滑块控件
    m_Slider = new QSlider(Qt::Horizontal);
    {
        m_Slider->setMinimum(0);
        m_Slider->setMaximum(1000); // 0-10秒，精度0.01
        m_Slider->setValue(0);
    }

    // 3. 创建时间标签
    auto labelLayout = new QHBoxLayout();
    QLabel* labelStart = new QLabel("0s");
    QLabel* labelEnd = new QLabel("10s");
    labelLayout->addWidget(labelStart);
    labelLayout->addStretch();
    labelLayout->addWidget(labelEnd);

    // 4. 组装
    layout->addWidget(m_TimeTempPlot);
    layout->addLayout(labelLayout);
    layout->addWidget(m_Slider);

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
    // 连接滑块信号到槽函数
    connect(m_Slider, &QSlider::valueChanged, this, [this](int value) {
        double time = value * 0.01; // 转换为秒
        m_TimeTempPlot->SetCursorPos(time);
        });
}