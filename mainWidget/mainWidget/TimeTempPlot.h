#pragma once
#include "QCustomPlot/qcustomplot.h"


class TimeTempPlot :public QCustomPlot
{
    Q_OBJECT
public:
    explicit TimeTempPlot(QWidget* parent = nullptr);

    void init();

    void AddDataPoint(const QVector<double>& times, const QVector<double>& temperatures);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QCPItemLine* m_vLine;
    QCPItemLine* m_hLine;
    QCPItemTracer* m_tracer;
    QCPItemText* m_textLabel;
};

