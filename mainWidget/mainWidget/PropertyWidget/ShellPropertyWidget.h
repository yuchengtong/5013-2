#pragma once
#include <QWidget>
#include "BasePropertyWidget.h"

class ShellPropertyWidget : public BasePropertyWidget
{
	Q_OBJECT
public:
	explicit ShellPropertyWidget(QWidget* parent = nullptr);

	void setMasterialData(QString model);

private:
	void initWidget() override;

private slots:
	void showTableDialog();

private:
	QTableWidget* m_tableWidget = nullptr;
};