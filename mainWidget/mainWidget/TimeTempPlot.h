#pragma once
#include "QCustomPlot/qcustomplot.h"


class TimeTempPlot :public QCustomPlot
{
    Q_OBJECT
public:
    explicit TimeTempPlot(QWidget* parent = nullptr);

    void AddDataPoint(const QVector<double>& times, const QVector<double>& temperatures);

private:
    
    void clearData();

private:
    void setupPlotStyle();

};

