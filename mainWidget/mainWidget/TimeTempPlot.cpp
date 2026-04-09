#pragma execution_character_set("utf-8")
#include "TimeTempPlot.h"


TimeTempPlot::TimeTempPlot(QWidget* parent) : QCustomPlot(parent)
{
    init();
    this->setMouseTracking(true);


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
    m_tracer->setGraph(this->graph(0));

    // 基础图表设置
    this->xAxis->setLabel("时间 (s)");
    this->yAxis->setLabel("温度 (°C)");
    this->addGraph();
    this->graph(0)->setPen(QPen(Qt::blue, 2));
    this->graph(0)->setName("温度曲线");
    this->setInteraction(QCP::iRangeDrag, true);
    this->setInteraction(QCP::iRangeZoom, true);
    this->xAxis->grid()->setVisible(true);
    this->yAxis->grid()->setVisible(true);
}

void TimeTempPlot::AddDataPoint(const QVector<double>& times, const QVector<double>& temperatures)
{
    if (times.isEmpty() || times.size() != temperatures.size())
    {
        return;
    }

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
    {
        return;
    }

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

void TimeTempPlot::mouseMoveEvent(QMouseEvent* event)
{
    // 1. 获取鼠标在绘图区域的像素坐标
    double mouseX = event->pos().x();
    double mouseY = event->pos().y();

    // 2. 将像素坐标转换为图表的逻辑坐标
    double key = this->xAxis->pixelToCoord(mouseX);
    double value = this->yAxis->pixelToCoord(mouseY);

    bool hoverOnCurve = false; // 标记是否悬停在曲线上

    // 3. 查找曲线上最接近鼠标 X 坐标的点
    if (this->graphCount() > 0)
    {
        QCPGraph* graph = this->graph(0);

        // 使用 QCustomPlot 的查找功能找到最接近的点
        // findBegin 需要一个搜索键值，这里用 key
        QCPGraphDataContainer::const_iterator it = graph->data()->findBegin(key);

        // 确保迭代器有效（数据不为空）
        if (it != graph->data()->constEnd())
        {
            // 获取该数据点的坐标
            double pointX = it->key;
            double pointY = it->value;

            // 4. 计算鼠标到该点的垂直像素距离
            // 先把数据点的 Y 坐标转回像素坐标，才能和鼠标的 Y (像素) 进行比较
            double pointYPixel = this->yAxis->coordToPixel(pointY);
            double distance = qAbs(pointYPixel - mouseY);

            // 【关键】设定阈值，例如 15 像素。
            // 只有当鼠标距离曲线小于 15 像素时，才认为悬停
            if (distance < 15.0)
            {
                hoverOnCurve = true;

                // 5. 更新十字准星和垂线的位置
                // 使用数据点的精确 Y 值 (pointY)，而不是鼠标的 Y 值

                // 垂直线 (X轴上下贯穿)
                m_vLine->start->setCoords(pointX, this->yAxis->range().lower);
                m_vLine->end->setCoords(pointX, this->yAxis->range().upper);

                // 水平线 (Y轴左右贯穿)
                m_hLine->start->setCoords(this->xAxis->range().lower, pointY);
                m_hLine->end->setCoords(this->xAxis->range().upper, pointY);

                // 追踪点 (吸附到曲线上)
                m_tracer->setVisible(true);
                m_tracer->setGraphKey(pointX);
                m_tracer->updatePosition();
            }
        }
    }

    // 6. 根据判断结果显示或隐藏线条
    if (hoverOnCurve)
    {
        m_vLine->setVisible(true);
        m_hLine->setVisible(true);
    }
    else
    {
        // 如果没悬停在曲线上，隐藏所有辅助线
        m_vLine->setVisible(false);
        m_hLine->setVisible(false);
        m_tracer->setVisible(false);
    }

    this->replot();
    // 调用父类事件
    QCustomPlot::mouseMoveEvent(event);
}

