#pragma once
#include <QMainWindow>

class QPushButton;

class SoftwareSelectionWidget : public QMainWindow
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
	QPushButton* m_pCustomBtn = nullptr;
	QPushButton* m_pInjectionBtn = nullptr;
	QPushButton* m_pPreheatingBtn = nullptr;
};