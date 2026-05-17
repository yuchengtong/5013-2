#pragma once
#include <QWidget>
#include "BaseCurvePlot.h"

class BaseCurvePlotWidget : public QWidget
{
	Q_OBJECT
public:
	explicit BaseCurvePlotWidget(QWidget* parent = nullptr);

	void init();
	void bindConnect();

	BaseCurvePlot* GetCurvePlot();

	void SetXName(const QString& str);

	void SetYName(const QString& str);

	void SetTitleName(const QString& str);

	void AddDataPoint(const QVector<double>& x, const QVector<double>& y);

private:
	BaseCurvePlot* m_BaseCurvePlot = nullptr;
	QLabel* m_TitleLabel;

};

