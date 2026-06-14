#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainWidget.h"
#include <QAction>
#include <QTabWidget>
#include <QLabel>
#include <QTimer>
#include <QPushButton>

#include <Windows.h>
#include <functional>

#include "GFImportModelWidget.h"
#include "DatabaseWidget.h"


class mainWidget : public QMainWindow
{
	Q_OBJECT
public:
	mainWidget(QWidget* parent = nullptr);
	~mainWidget();

private:
	void init();
	void bindConnect();
	void refreshMemoryUsage(QLabel* statusLabel);
	void getMemoryUsage(QLabel* statusLabel);

	// 工具栏设置
	void setupGeomWidget();
	void setupOperationWidget();
	//void setupViewWidget();

	// 视图方向设置
	enum ViewDirection {
		View_Xpos, View_Ypos, View_Zpos,
		View_Xneg, View_Yneg, View_Zneg
	};
	void setViewDirection(ViewDirection dir);

	// 文件导入处理
	void handleModelImport();
	void handleExcelImport();
	bool loadStepFile(const QString& filePath, TopoDS_Shape& outShape, ModelGeometryInfo& outInfo);
	bool loadStlFile(const QString& filePath, TopoDS_Shape& outShape, ModelGeometryInfo& outInfo);
	bool computeBBox(const TopoDS_Shape& shape, const QString& filePath, ModelGeometryInfo& outInfo);

	// 辅助函数
	ULONGLONG fileTimeToULL(const FILETIME& ft);

private:
	// 由 setupUi 创建的 UI 元素
	QMenuBar* m_menuBar = nullptr;
	QToolBar* m_mainToolBar = nullptr;

	// Tab页面指针
	GFImportModelWidget* m_importModelWid = nullptr;
	DatabaseWidget* m_dataBaseWid = nullptr;

	// 工具栏按钮
	QPushButton* m_importBtn = nullptr;
	QPushButton* m_saveBtn = nullptr;
	QPushButton* m_saveAsBtn = nullptr;
	QPushButton* m_exportBtn = nullptr;
	//QPushButton* m_moveBtn = nullptr;
	//QPushButton* m_rotateBtn = nullptr;
	QPushButton* m_zoomBtn = nullptr;
	QPushButton* m_fitAllBtn = nullptr;
	//QPushButton* m_resetBtn = nullptr;
	//QPushButton* m_xBtn = nullptr;
	//QPushButton* m_yBtn = nullptr;
	//QPushButton* m_zBtn = nullptr;
	//QPushButton* m_xNegBtn = nullptr;
	//QPushButton* m_yNegBtn = nullptr;
	//QPushButton* m_zNegBtn = nullptr;

	// 顶部导航TabWidget（替代MenuBar）
	QTabWidget* m_navTabWidget = nullptr;

	QTabWidget* m_pMainTabWidget = nullptr;

	// 状态栏
	QLabel* m_statusLabel = nullptr;

	// 定时器和系统时间
	QTimer* m_timer = nullptr;
	FILETIME m_prevIdleTime = { 0 };
	FILETIME m_prevKernelTime = { 0 };
	FILETIME m_prevUserTime = { 0 };
	bool m_isFirstSample = true;

	int m_prevValidIndex = 0;
};