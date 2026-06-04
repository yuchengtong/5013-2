#pragma once
#include "QCustomPlot/qcustomplot.h"
#include <QPushButton>

class BaseCurvePlot : public QCustomPlot
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
	void resizeEvent(QResizeEvent* event) override;

private:
	void updateBtnPosition();

	QCPItemLine* m_vLine = nullptr;
	QCPItemLine* m_hLine = nullptr;
	QCPItemTracer* m_tracer = nullptr;
	QCPItemText* m_textLabel = nullptr;
	QPushButton* m_resetBtn = nullptr;
};