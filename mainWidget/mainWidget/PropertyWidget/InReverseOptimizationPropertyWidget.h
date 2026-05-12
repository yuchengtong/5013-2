#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"
#include "InReverseFormulaSolver.h"

#include <QRadioButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPushButton>

class InReverseOptimizationPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit InReverseOptimizationPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;
	void bindConnect();
private slots:
	void calculate();


private:
	QTableWidget* m_tableWidget = nullptr;

	QPushButton* m_calButton = nullptr;
	QPushButton* m_viewButton = nullptr;

	QString m_insulationTemperatureValue = "50"; // 弹体保温温度
	QString m_pouringTemperatureValue = "102"; // 药液浇注温度
	QString m_valveOpeningValue = "6.5"; // 阀门开度
	QString m_vacuumDegreeValue = "0.1"; // 真空度
	QString m_relativeDensityValue = ""; // 相对密度
	QString m_injectionTimeValue = ""; // 弹体注药时间

	
	QRadioButton* m_insulationTempeRadioBtn = nullptr;
	QRadioButton* m_pouringTempeRadioBtn = nullptr;
	QRadioButton* m_valveOpeningRadioBtn = nullptr;
	QRadioButton* m_vacuumDegreeRadioBtn = nullptr;

	QRadioButton* m_relativeDensityRadioBtn = nullptr;
	QRadioButton* m_injectionTimeRadioBtn = nullptr;

	InReverseFormulaSolver* m_solver;

};