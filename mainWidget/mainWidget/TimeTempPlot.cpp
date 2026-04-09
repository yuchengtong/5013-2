#pragma execution_character_set("utf-8")
#include "TimeTempPlot.h"


TimeTempPlot::TimeTempPlot(QWidget* parent) : QCustomPlot(parent)
{
    init();
    this->setMouseTracking(true);
}

void TimeTempPlot::init()
{
    m_vLine = new QCPItemLine(this);
    m_vLine->setPen(QPen(Qt::red, 1, Qt::DashLine));

    m_hLine = new QCPItemLine(this);
    m_hLine->setPen(QPen(Qt::red, 1, Qt::DashLine));

    m_tracer = new QCPItemTracer(this);
    m_tracer->setPen(QPen(Qt::red, 2));
    m_tracer->setBrush(QBrush(Qt::red));
    m_tracer->setSize(6);
    m_tracer->setStyle(QCPItemTracer::tsCircle);
    m_tracer->setGraph(this->graph(0));

    this->xAxis->setLabel("时间 (s)");
    this->yAxis->setLabel("温度 (°C)");
    this->addGraph();
    this->graph(0)->setPen(QPen(Qt::blue, 2));
    this->graph(0)->setName("温度曲线");
    this->setInteraction(QCP::iRangeDrag, true);
    this->setInteraction(QCP::iRangeZoom, true);
    this->xAxis->grid()->setVisible(true);
    this->yAxis->grid()->setVisible(true);

    m_textLabel = new QCPItemText(this);
    m_textLabel->setVisible(false); // 默认隐藏
    m_textLabel->setPositionAlignment(Qt::AlignBottom | Qt::AlignHCenter); // 对齐方式：底部居中
    m_textLabel->setPen(QPen(Qt::black)); // 边框颜色
    m_textLabel->setBrush(QBrush(QColor(255, 255, 255, 200))); // 背景颜色 (白色半透明)
    m_textLabel->setFont(QFont("Microsoft YaHei", 9, QFont::Bold)); // 字体
    m_textLabel->setPadding(QMargins(5, 3, 5, 3)); // 内边距
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

void TimeTempPlot::mouseMoveEvent(QMouseEvent* event)
{
    double mouseX = event->pos().x();
    double mouseY = event->pos().y();

    double key = this->xAxis->pixelToCoord(mouseX);
    double value = this->yAxis->pixelToCoord(mouseY);

    bool hoverOnCurve = false;

    if (this->graphCount() > 0)
    {
        QCPGraph* graph = this->graph(0);
        QCPGraphDataContainer::const_iterator it = graph->data()->findBegin(key);

        if (it != graph->data()->constEnd())
        {
            double pointX = it->key;
            double pointY = it->value;

            double pointYPixel = this->yAxis->coordToPixel(pointY);
            double distance = qAbs(pointYPixel - mouseY);

            if (distance < 15.0)
            {
                hoverOnCurve = true;

                m_vLine->start->setCoords(pointX, this->yAxis->range().lower);
                m_vLine->end->setCoords(pointX, pointY);

                m_hLine->start->setCoords(this->xAxis->range().lower, pointY);
                m_hLine->end->setCoords(pointX, pointY);

                m_tracer->setVisible(true);
                m_tracer->setGraphKey(pointX);
                m_tracer->updatePosition();

                m_textLabel->setVisible(true);
                m_textLabel->setText(QString("X: %1\nY: %2").arg(pointX, 0, 'f', 2).arg(pointY, 0, 'f', 2));
                m_textLabel->position->setCoords(pointX, pointY);
            }
        }
    }

    if (hoverOnCurve)
    {
        m_vLine->setVisible(true);
        m_hLine->setVisible(true);
    }
    else
    {
        m_vLine->setVisible(false);
        m_hLine->setVisible(false);
        m_tracer->setVisible(false);
        m_textLabel->setVisible(false);
    }

    this->replot();
    QCustomPlot::mouseMoveEvent(event);
}

