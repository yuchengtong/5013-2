#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"
#include "PreReverseFormulaSolver.h"

#include <QRadioButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPushButton>

class PreReverseOptimizationPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit PreReverseOptimizationPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private slots:
	void calculate();

	void reset();

private:
	QTableWidget* m_tableWidget = nullptr;

	QPushButton* m_calButton = nullptr;
	QPushButton* m_resetButton = nullptr;
	QPushButton* m_viewButton = nullptr;

	QString m_targetTemperatureValue = "50"; // 弹体目标温度
	QString m_environmentalTemperatureValue = "90"; // 烘箱环境温度
	QString m_initialTemperatureValue = "22"; // 弹体初始温度
	QString m_heatTransferCoefficientValue = "58"; // 环境对流传热系数
	QString m_absorptionCoefficientValue = "0.5"; // 壳体辐射吸收系数
	QString m_environmentalEmissivityValue = "0.5"; // 环境发射率
	QString m_preheatingTimeValue = ""; // 弹体预热时间

	
	QRadioButton* m_targetempeRadioBtn = nullptr; // 弹体目标温度
	QRadioButton* m_environmentalTempeRadioBtn = nullptr; // 烘箱环境温度

	QRadioButton* m_preheatingTimeRadioBtn = nullptr; //弹体预热时间

	PreReverseFormulaSolver* m_solver;

};