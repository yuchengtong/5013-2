#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"
#include "ModelDataManager.h"

#include <QComboBox>

class GeomPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit GeomPropertyWidget(QWidget* parent = nullptr);

	void UpdataPropertyInfo();

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private slots:
	void onComboBoxChanged(int index);

private:
	QTableWidget* m_tableWidget = nullptr;

	QComboBox* m_modelComboBox = nullptr;

	QString m_boreDiameterValue = "391";
	QString m_equaldiameterSectionHeightValue = "560";
	QString m_boreRadiusValue = "0";
	QString m_variableDiameterSectionHeightValue = "0";
	QString m_shellThicknessValue = "20";
	QString m_gasketLayerThicknessValue = "1";
	QString m_injectionHoleDiameterValue = "20";
	QString m_vacuumHoleDiameterValue= "8";

};