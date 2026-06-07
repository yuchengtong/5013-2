#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"
#include "ModelDataManager.h"

class InForwardDesignPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit InForwardDesignPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

	void calculateTable(InForwardPropertyInfo& inForwardPropertyInfo, ModelGeometryInfo& modelGeometryInfo, SteelPropertyInfo& steelPropertyInfo, CalculationPropertyInfo& calculationPropertyInfo);

private slots:
	void inForwardCalculate();

	void reset();

private:
	QTableWidget* m_tableWidget = nullptr;


	QString m_insulationTemperatureValue = "60"; // 弹体保温温度
	QString m_pouringTemperatureValue = "102"; // 药液浇注温度
	QString m_valveOpeningValue = "15"; // 阀门开度
	QString m_vacuumDegreeValue = "0.05"; // 真空度
	QString m_relativeDensityValue = ""; // 相对密度
	QString m_injectionTimeValue = ""; // 弹体注药时间


};