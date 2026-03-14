#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class InForwardDesignPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit InForwardDesignPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private slots:
	void showTableDialog();

private:
	QTableWidget* m_tableWidget = nullptr;
};