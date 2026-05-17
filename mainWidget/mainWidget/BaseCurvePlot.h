#pragma once
#include "QCustomPlot/qcustomplot.h"

class BaseCurvePlot :public QCustomPlot
{
    Q_OBJECT
public:
    explicit BaseCurvePlot(QWidget* parent = nullptr);

    void init();

    void AddDataPoint(const QVector<double>& x, const QVector<double>& y);

    void SetXName(const QString& str);
    void SetYName(const QString& str);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QCPItemLine* m_vLine;
    QCPItemLine* m_hLine;
    QCPItemTracer* m_tracer;
    QCPItemText* m_textLabel;
};

