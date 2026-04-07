#pragma execution_character_set("utf-8")
#include "TimeTempPlot.h"


TimeTempPlot::TimeTempPlot(QWidget* parent) : QCustomPlot(parent)
{
    init();



}

void TimeTempPlot::init()
{
    // 初始化十字准星对象
    // 垂直线
    m_vLine = new QCPItemLine(this);
    m_vLine->setPen(QPen(Qt::red, 1, Qt::DashLine));

    // 水平线
    m_hLine = new QCPItemLine(this);
    m_hLine->setPen(QPen(Qt::red, 1, Qt::DashLine));

    // 追踪点
    m_tracer = new QCPItemTracer(this);
    m_tracer->setPen(QPen(Qt::red, 2));
    m_tracer->setBrush(QBrush(Qt::red));
    m_tracer->setSize(6);
    m_tracer->setStyle(QCPItemTracer::tsCircle);
    m_tracer->setGraph(this->graph(0)); // 绑定到第一条曲线

    // 基础图表设置
    this->xAxis->setLabel("时间 (s)");
    this->yAxis->setLabel("温度 (°C)");
    this->addGraph();
    this->graph(0)->setPen(QPen(Qt::blue, 2)); // 蓝色线条
    this->graph(0)->setName("温度曲线");
    this->setInteraction(QCP::iRangeDrag, true);
    this->setInteraction(QCP::iRangeZoom, true);
    this->xAxis->grid()->setVisible(true);
    this->yAxis->grid()->setVisible(true);
}

void TimeTempPlot::AddDataPoint(const QVector<double>& times, const QVector<double>& temperatures)
{
    if (times.isEmpty() || times.size() != temperatures.size())
        return;

    this->graph(0)->setData(times, temperatures);

    // 设置 X 轴范围
    this->xAxis->setRange(times.first(), times.last());

    // 设置 Y 轴范围 (留出一点边距)
    double minY = *std::min_element(temperatures.constBegin(), temperatures.constEnd());
    double maxY = *std::max_element(temperatures.constBegin(), temperatures.constEnd());
    double margin = (maxY - minY) * 0.1;
    this->yAxis->setRange(minY - margin, maxY + margin);

    // 启用重绘
    this->replot();

    // 数据加载后，显示十字线对象
    m_vLine->setVisible(true);
    m_hLine->setVisible(true);
    m_tracer->setVisible(true);
}

void TimeTempPlot::SetCursorPos(double pos)
{
    auto dataContainer = this->graph(0)->data();
    if (!dataContainer || dataContainer->isEmpty())
        return;

    // 1. 获取 X 轴范围 (用于限制滑块范围)
    double minKey = std::numeric_limits<double>::max();
    double maxKey = -std::numeric_limits<double>::max();
    for (auto it = dataContainer->constBegin(); it != dataContainer->constEnd(); ++it)
    {
        minKey = qMin(minKey, it->key);
        maxKey = qMax(maxKey, it->key);
    }
    pos = qBound(minKey, pos, maxKey);

    // 2. 关键步骤：更新追踪点位置
    m_tracer->setGraphKey(pos);       // 告诉追踪器 X 坐标是多少
    m_tracer->updatePosition();       // 【核心】：这一步会强制计算该 X 对应的 Y 坐标

    // 3. 获取计算好的坐标值
    // 因为上面调用了 updatePosition，这里的 value() 才是准确的曲线上的 Y 值
    double curveY = m_tracer->position->value();

    // 4. 设置垂直线：从底部指向曲线
    m_vLine->start->setCoords(pos, this->yAxis->range().lower);
    m_vLine->end->setCoords(pos, curveY); // 终点是计算出的 Y

    // 5. 设置水平线：从 Y 轴指向曲线
    m_hLine->start->setCoords(this->xAxis->range().lower, curveY); // Y 坐标是计算出的 Y
    m_hLine->end->setCoords(pos, curveY);       // 终点是计算出的 Y

    this->replot();
}

