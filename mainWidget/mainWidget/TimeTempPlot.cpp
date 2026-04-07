#pragma execution_character_set("utf-8")
#include "TimeTempPlot.h"


TimeTempPlot::TimeTempPlot(QWidget* parent) : QCustomPlot(parent)
{
    this->xAxis->setLabel("时间");
    this->yAxis->setLabel("温度 (°C)");



    this->addGraph();
    this->graph(0)->setPen(QPen(Qt::blue, 2)); // 蓝色线条
    this->graph(0)->setName("温度曲线");


    this->setInteraction(QCP::iRangeDrag, true);
    this->setInteraction(QCP::iRangeZoom, true);

}

void TimeTempPlot::AddDataPoint(const QVector<double>& times, const QVector<double>& temperatures)
{
    this->graph(0)->setData(times, temperatures);
    if (!times.empty())
    {
        this->xAxis->setRange(times.front(), times.back());
        // this->yAxis->rescale(); 
    }
    this->replot();
}
