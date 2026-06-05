#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class LoginWindow : public QMainWindow
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
	QLabel* m_pTitleLabel = nullptr;

	QWidget* m_pCentralWidget = nullptr;

	QWidget* m_pRightPanel = nullptr;
	QLabel* m_pLogoLabel = nullptr;

	QLineEdit* m_pUseLineEdit = nullptr;
	QLineEdit* m_pPassportLineEdit = nullptr;

	QPushButton* m_pLoginBtn = nullptr;
};