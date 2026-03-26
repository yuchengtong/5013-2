#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class ShellPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit ShellPropertyWidget(QWidget* parent = nullptr);

private:
	void initWidget() override;

private slots:
	void showTableDialog();

private:
	QTableWidget* m_tableWidget = nullptr;
};