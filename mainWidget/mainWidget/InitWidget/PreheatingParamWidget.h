#pragma once
#include <QWidget>

class QLineEdit;
class QPushButton;

class PreheatingParamWidget : public QWidget
{
	Q_OBJECT
public:
	explicit PreheatingParamWidget(QWidget* parent = nullptr);
	~PreheatingParamWidget();

private:
	void initUI();
	void bindConnect();

private:
	// Workbench 路径
	QLineEdit* m_pWorkbenchPathEdit = nullptr;
	QPushButton* m_pBrowseBtn = nullptr;

	// 工艺参数
	QLineEdit* m_pValveOpeningEdit = nullptr;      // 阀门开度
	QLineEdit* m_pWallThicknessEdit = nullptr;     // 壁厚
	QLineEdit* m_pBondlineThicknessEdit = nullptr; // 胶层厚度
	QLineEdit* m_pLiquidTempEdit = nullptr;        // 药液温度
	QLineEdit* m_pInsulationTempEdit = nullptr;    // 保温温度

	QPushButton* m_pStartBtn = nullptr;
};