#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

#include <QRadioButton>
#include <QButtonGroup>
#include <QHBoxLayout>

class PreReverseOptimizationPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit PreReverseOptimizationPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private slots:
	void showTableDialog();

	// 单选按钮选中事件处理槽函数
	void onRadioSelected(int btnId);

private:
	QTableWidget* m_tableWidget = nullptr;

	QString m_targetTemperatureValue = "50"; // 弹体目标温度
	QString m_environmentalTemperatureValue = "90"; // 烘箱环境温度
	QString m_initialTemperatureValue = "22"; // 弹体初始温度
	QString m_heatTransferCoefficientValue = "3"; // 环境对流传热系数
	QString m_absorptionCoefficientValue = "0.5"; // 壳体辐射吸收系数
	QString m_environmentalEmissivityValue = "0.5"; // 环境发射率
	QString m_preheatingTimeValue = ""; // 弹体预热时间

	QButtonGroup* m_radioButtonGroup = nullptr; // 单选按钮组
};