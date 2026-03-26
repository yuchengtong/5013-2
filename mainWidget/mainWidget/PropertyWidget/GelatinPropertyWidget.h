#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class GelatinPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit GelatinPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private slots:
	void showTableDialog();

private:
	QTableWidget* m_tableWidget = nullptr;
};