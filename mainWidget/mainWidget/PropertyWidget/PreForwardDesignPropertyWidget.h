#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class PreForwardDesignPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit PreForwardDesignPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private slots:
	void showTableDialog();

private:
	QTableWidget* m_tableWidget = nullptr;
};