#pragma once
#include "Fluent/FluentMainWindow.h"
#include "Fluent/FluentMenuBar.h"
#include "Fluent/FluentLabel.h"
#include "Fluent/FluentWidget.h"
#include "Fluent/FluentLineEdit.h"
#include "Fluent/FluentButton.h"
class LoginWindow : public Fluent::FluentMainWindow
{
	Q_OBJECT
public:
	LoginWindow(QWidget* parent = nullptr);
	~LoginWindow();

signals:
	void loginSuccess();

private:
	void init();
	void bindConnect();

private:
	Fluent::FluentMenuBar* m_pMenuBar = nullptr;
	Fluent::FluentLabel* m_pTitleLabel = nullptr;

	Fluent::FluentWidget* m_pCentralWidget = nullptr;
	Fluent::FluentLabel*m_pLeftImageLabel= nullptr;

	Fluent::FluentWidget* m_pRightPanel = nullptr;
	Fluent::FluentLabel* m_pLogoLabel = nullptr;

	Fluent::FluentLineEdit* m_pUseLineEdit = nullptr;
	Fluent::FluentLineEdit* m_pPassportLineEdit = nullptr;

	Fluent::FluentButton*m_pLoginBtn = nullptr;
};

