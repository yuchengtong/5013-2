#pragma execution_character_set("utf-8")
#pragma once
#include "Fluent/FluentMainWindow.h"
#include "Fluent/FluentMenuBar.h"
#include "Fluent/FluentLabel.h"
#include "Fluent/FluentWidget.h"
#include "Fluent/FluentButton.h"
#include "Fluent/FluentLineEdit.h"

class SoftwareSelectionWidget : public Fluent::FluentMainWindow
{
	Q_OBJECT
public:
	SoftwareSelectionWidget(QWidget* parent = nullptr);
	~SoftwareSelectionWidget();

private:
	void init();
	void bindConnect();
	void startWorkbench();

private:
	Fluent::FluentMenuBar* m_pMenuBar = nullptr;
	Fluent::FluentLabel* m_pTitleLabel = nullptr;
	Fluent::FluentWidget* m_pCentralWidget = nullptr;

	// 平台卡片
	Fluent::FluentWidget* m_pWorkbenchCard = nullptr;
	Fluent::FluentWidget* m_pCustomCard = nullptr;

	// Workbench 输入控件
	Fluent::FluentLineEdit* m_pSoftwarePathEdit = nullptr;
	Fluent::FluentLineEdit* m_pValveOpeningEdit = nullptr;
	Fluent::FluentLineEdit* m_pWallThicknessEdit = nullptr;
	Fluent::FluentLineEdit* m_pBondlineThicknessEdit = nullptr;
	Fluent::FluentLineEdit* m_pInsulationTempEdit = nullptr;

	Fluent::FluentButton* m_pCustomBtn = nullptr;
};