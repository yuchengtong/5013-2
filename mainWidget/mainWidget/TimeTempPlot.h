#pragma once
#include "QCustomPlot/qcustomplot.h"


class TimeTempPlot :public QCustomPlot
{
    Q_OBJECT
public:
    explicit TimeTempPlot(QWidget* parent = nullptr);

    void init();

    void AddDataPoint(const QVector<double>& times, const QVector<double>& temperatures);

    void SetCursorPos(double pos);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;



private:
    
    // 十字准星相关对象
    QCPItemLine* m_vLine;     // 垂直线
    QCPItemLine* m_hLine;     // 水平线
    QCPItemTracer* m_tracer;  // 曲线上的追踪点

};

