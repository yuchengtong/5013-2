#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

#include <QRadioButton>
#include <QButtonGroup>
#include <QHBoxLayout>

class InReverseOptimizationPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit InReverseOptimizationPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private slots:
	void showTableDialog();

	// 单选按钮选中事件处理槽函数
	void inOnRadioSelected(int btnId);
	void outOnRadioSelected(int btnId);

private:
	QTableWidget* m_tableWidget = nullptr;


	QString m_insulationTemperatureValue = "50"; // 弹体保温温度
	QString m_pouringTemperatureValue = "102"; // 药液浇注温度
	QString m_pouringSpeedValue = "15"; // 药液浇注速度
	QString m_vacuumDegreeValue = "0.1"; // 真空度
	QString m_relativeDensityValue = ""; // 相对密度
	QString m_injectionTimeValue = ""; // 弹体注药时间

	QButtonGroup* m_inRadioButtonGroup = nullptr; // 单选按钮组
	QButtonGroup* m_outRadioButtonGroup = nullptr; // 单选按钮组
};