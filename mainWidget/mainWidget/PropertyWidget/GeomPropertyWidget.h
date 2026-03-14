#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"
#include "ModelDataManager.h"

class GeomPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit GeomPropertyWidget(QWidget* parent = nullptr);

	void UpdataPropertyInfo();

	QTableWidget* GetQTableWidget() { return m_tableWidget; }

private:
	void initWidget() override;

private:
	QTableWidget* m_tableWidget = nullptr;

	QString m_boreDiameterValue = "391";
	QString m_equaldiameterSectionHeightValue = "560";
	QString m_shellThicknessValue = "20";
	QString m_gasketLayerThicknessValue = "1";

};