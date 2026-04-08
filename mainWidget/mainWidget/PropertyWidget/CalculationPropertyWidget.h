#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

#include <QString>

class CalculationPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit CalculationPropertyWidget(QWidget* parent = nullptr);

	void updateData(QString text);

private:
	void initWidget() override;


private:
	QTableWidget* m_tableWidget = nullptr;
};