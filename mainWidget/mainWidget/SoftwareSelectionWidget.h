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

	// 两个软件卡片
	Fluent::FluentWidget* m_pWorkbenchCard = nullptr;
	Fluent::FluentWidget* m_pCustomCard = nullptr;

	// Workbench 参数输入
	Fluent::FluentLineEdit* m_pDensityEdit = nullptr;
	Fluent::FluentLineEdit* m_pLengthEdit = nullptr;
	Fluent::FluentLineEdit* m_pWidthEdit = nullptr;
	Fluent::FluentLineEdit* m_pHeightEdit = nullptr;
	Fluent::FluentLineEdit* m_pVolumeEdit = nullptr;
};