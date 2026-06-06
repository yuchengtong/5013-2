#include "mainWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QGraphicsView>
#include <QBrush>
#include <QLinearGradient>
#include <QPen>
#include <QtCharts>
#include <QLineSeries>
#include <QBarSeries>
#include <QtCharts/qchartview.h>
#include <QFileDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QMenuBar>
#include <QToolBar>
#include "xlsxdocument.h"

#include <AIS_Shape.hxx>
#include <STEPControl_Reader.hxx>
#include <Prs3d_LineAspect.hxx>
#include <V3d_View.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepMesh_Context.hxx>  
#include <BRepBndLib.hxx>
#include <StlAPI_Reader.hxx>
#include <Bnd_Box.hxx>
#include <gp_Pnt.hxx>
#include <Quantity_Color.hxx>

#include "GFImportModelWidget.h"
#include "GFLogWidget.h"
#include "DatabaseWidget.h"
#include "OccView.h"
#include "GFTreeModelWidget.h"
#include "ModelDataManager.h"

mainWidget::mainWidget(QWidget* parent)
	: QMainWindow(parent)
{
	Ui::mainWidgetClass uiSetup;
	uiSetup.setupUi(this);

	m_menuBar = uiSetup.menuBar;
	m_mainToolBar = uiSetup.mainToolBar;

	// 隐藏原来的菜单栏，用自定义TabWidget替代
	m_menuBar->setVisible(false);

	init();
	bindConnect();
}

mainWidget::~mainWidget()
{
	if (m_timer) {
		m_timer->stop();
		delete m_timer;
		m_timer = nullptr;
	}
}

void mainWidget::init()
{
	setWindowIcon(QIcon(":/selectWidget/src/selectWidget/Custom.png"));
	setWindowTitle(QString::fromLocal8Bit("TNT基/DNAN基熔铸炸药注装工艺参数匹配设计工具软件"));

	// ========== 创建顶部导航TabWidget（替代MenuBar）==========
	m_navTabWidget = new QTabWidget(this);
	m_navTabWidget->setDocumentMode(true);
	m_navTabWidget->setTabPosition(QTabWidget::North);
	// 关键：去掉边框和pane背景，让它看起来像菜单栏
	

	// ========== 导航TabWidget样式表修改 ==========
	m_navTabWidget->setStyleSheet(R"(
    /* --- 核心修复区域 --- */
    QTabWidget::pane {
        border: none;           /* 关键：移除默认的边框 */
        top: -1px;              /* 关键：向上偏移 1px，覆盖掉 TabBar 底部的空隙 */
        background-color: transparent; /* 保持背景透明或设为白色 */
        margin: 0px;            /* 确保没有外边距 */
        padding: 0px;           /* 确保没有内边距 */
    }

    QTabBar {
        -qproperty-expanding: false;
        -qproperty-alignment: AlignLeft;
    }

    QTabBar::tab {
        background-color: #E4E7E9;
        border: none;
        padding: 8px 16px;
        margin-right: 2px;
        /* 这里的 border-bottom 是选中时的下划线，不要在这里加实线边框 */
    }

    QTabBar::tab:selected {
        background-color: white;
        color: #1890ff;
        font-weight: bold;
border-bottom: 2px solid white; /* 改成白色，看起来就没有线了 */
    }
)");


	m_navTabWidget->addTab(new QWidget(), QIcon(":/src/database.svg"), QString::fromLocal8Bit("数据库"));
	m_navTabWidget->addTab(new QWidget(), QIcon(":/src/craft.svg"), QString::fromLocal8Bit("预热与注药工艺"));
	m_navTabWidget->addTab(new QWidget(), QIcon(":/src/help.svg"), QString::fromLocal8Bit("帮助"));
	m_navTabWidget->setFixedHeight(48);
	m_navTabWidget->setIconSize(QSize(32, 32));


	// ========== 全局样式设置 ==========
	this->setStyleSheet(R"(
		/* 工具栏背景白色 */
		QToolBar {
			background-color: #ffffff;
			border: none;
			border-bottom: 1px solid #E4E7E9;
			spacing: 2px;
			padding: 4px;
		}
		QToolBar::separator {
			background-color: #E4E7E9;
			width: 1px;
			margin: 4px 2px;
		}
		/* 工具栏按钮 */
		QToolButton {
			background-color: transparent;
			border: 1px solid transparent;
			border-radius: 3px;
			padding: 4px;
			color: #333333;
		}
		QToolButton:hover {
			background-color: #f5f5f5;
			border: 1px solid #d0d0d0;
		}
		QToolButton:pressed {
			background-color: #e8e8e8;
		}

		/* 状态栏 */
		QStatusBar {
			background-color: #f0f0f0;
			border-top: 1px solid #E4E7E9;
			color: #333333;
			font-size: 12px;
		}
	)");

	// 状态栏
	QStatusBar* statusbar = statusBar();
	m_statusLabel = new QLabel(QString::fromLocal8Bit("内存使用：0%，CPU使用：0%"));
	statusbar->addPermanentWidget(m_statusLabel);
	refreshMemoryUsage(m_statusLabel);

	// 工具栏设置
	m_mainToolBar->setMovable(false);
	m_mainToolBar->setFloatable(false);

	// 设置工具栏各组件
	setupGeomWidget();
	setupOperationWidget();
	//setupViewWidget();

	// ========== 主内容TabWidget ==========
	m_pMainTabWidget = new QTabWidget(this);
	m_importModelWid = new GFImportModelWidget(m_pMainTabWidget);
	m_dataBaseWid = new DatabaseWidget(m_pMainTabWidget);

	m_pMainTabWidget->addTab(m_importModelWid, "importModelWid");
	m_pMainTabWidget->addTab(m_dataBaseWid, "dataBaseWid");
	m_pMainTabWidget->tabBar()->setVisible(false);

	// ========== 总布局 ==========
	// 创建中心widget来容纳导航Tab + 工具栏 + 内容
	auto* centralWidget = new QWidget(this);
	auto* centralLayout = new QVBoxLayout(centralWidget);
	centralLayout->setContentsMargins(0, 0, 0, 0);
	centralLayout->setSpacing(0);

	// 1. 顶部导航Tab
	centralLayout->addWidget(m_navTabWidget);

	// 2. 工具栏（放在导航下方）
	// 注意：QMainWindow的toolBar是独立的，这里需要把它移到布局中
	// 或者保持QMainWindow的toolBar，但设置其位置
	// 方案：移除QMainWindow的toolBar，手动添加到布局
	removeToolBar(m_mainToolBar);
	centralLayout->addWidget(m_mainToolBar);

	// 3. 主内容区
	centralLayout->addWidget(m_pMainTabWidget, 1);

	setCentralWidget(centralWidget);

	m_navTabWidget->setCurrentIndex(1);
	// 主内容显示模型页面（索引0）
	m_pMainTabWidget->setCurrentIndex(0);
	// 工具栏可见
	m_mainToolBar->setVisible(true);


}

// ============================================================
// 工具栏设置函数（setupGeomWidget, setupOperationWidget, setupViewWidget 不变）
// ============================================================
void mainWidget::setupGeomWidget()
{
	m_importBtn = new QPushButton();
	m_saveBtn = new QPushButton();
	m_saveAsBtn = new QPushButton();
	m_exportBtn = new QPushButton();

	m_importBtn->setIcon(QIcon(":/src/Import.svg"));
	m_saveBtn->setIcon(QIcon(":/src/Save_1.svg"));
	m_saveAsBtn->setIcon(QIcon(":/src/Save_as.svg"));
	m_exportBtn->setIcon(QIcon(":/src/Export.svg"));

	const int btnSize = 32;
	QSize iconSize(btnSize, btnSize); 
	m_importBtn->setIconSize(iconSize);
	m_saveBtn->setIconSize(iconSize);
	m_saveAsBtn->setIconSize(iconSize);
	m_exportBtn->setIconSize(iconSize);

	m_importBtn->setFlat(true);
	m_saveBtn->setFlat(true);
	m_saveAsBtn->setFlat(true);
	m_exportBtn->setFlat(true);

	auto importLabel = new QLabel(QString::fromLocal8Bit("导入"));
	auto saveLabel = new QLabel(QString::fromLocal8Bit("保存"));
	auto saveAsLabel = new QLabel(QString::fromLocal8Bit("另存为..."));
	auto exportLabel = new QLabel(QString::fromLocal8Bit("导出"));

	auto importVBox = new QVBoxLayout();
	importVBox->addWidget(m_importBtn, 0, Qt::AlignHCenter);
	importVBox->addWidget(importLabel, 0, Qt::AlignHCenter);
	importVBox->setSpacing(2);
	importVBox->setContentsMargins(4, 0, 4, 0);

	auto saveAsVBox = new QVBoxLayout();
	saveAsVBox->addWidget(m_saveAsBtn, 0, Qt::AlignHCenter);
	saveAsVBox->addWidget(saveAsLabel, 0, Qt::AlignHCenter);
	saveAsVBox->setSpacing(2);
	saveAsVBox->setContentsMargins(4, 0, 4, 0);

	auto saveVBox = new QVBoxLayout();
	saveVBox->addWidget(m_saveBtn, 0, Qt::AlignHCenter);
	saveVBox->addWidget(saveLabel, 0, Qt::AlignHCenter);
	saveVBox->setSpacing(2);
	saveVBox->setContentsMargins(4, 0, 4, 0);

	auto exportVBox = new QVBoxLayout();
	exportVBox->addWidget(m_exportBtn, 0, Qt::AlignHCenter);
	exportVBox->addWidget(exportLabel, 0, Qt::AlignHCenter);
	exportVBox->setSpacing(2);
	exportVBox->setContentsMargins(4, 0, 4, 0);

	auto hLayout = new QHBoxLayout();
	hLayout->addLayout(importVBox);
	hLayout->addLayout(saveAsVBox);
	hLayout->addLayout(saveVBox);
	hLayout->addLayout(exportVBox);
	hLayout->addStretch();
	hLayout->setSpacing(4);
	hLayout->setContentsMargins(4, 0, 4, 0);

	auto vLayout = new QVBoxLayout();
	vLayout->addLayout(hLayout);
	vLayout->setContentsMargins(0, 0, 0, 0);
	vLayout->setSpacing(2);

	auto geomWidget = new QWidget();
	geomWidget->setFixedWidth(280);
	geomWidget->setLayout(vLayout);

	m_mainToolBar->addWidget(geomWidget);
	m_mainToolBar->addSeparator();
}

void mainWidget::setupOperationWidget()
{
	//m_moveBtn = new QPushButton();
	//m_rotateBtn = new QPushButton();
	m_zoomBtn = new QPushButton();
	m_fitAllBtn = new QPushButton();
	//m_resetBtn = new QPushButton();

	//m_moveBtn->setIcon(QIcon(":/src/Move.svg"));
	//m_rotateBtn->setIcon(QIcon(":/src/Rotate.svg"));
	m_zoomBtn->setIcon(QIcon(":/src/Zoom.svg"));
	m_fitAllBtn->setIcon(QIcon(":/src/FitAll.svg"));
	//m_resetBtn->setIcon(QIcon(":/src/Reset.svg"));

	const int btnSize = 32;
	QSize iconSize(btnSize, btnSize);
	//m_moveBtn->setFixedSize(btnSize, btnSize);
	//m_rotateBtn->setFixedSize(btnSize, btnSize);
	m_zoomBtn->setIconSize(iconSize);
	m_fitAllBtn->setIconSize(iconSize);
	//m_resetBtn->setFixedSize(btnSize, btnSize);

	m_zoomBtn->setFlat(true);
	m_fitAllBtn->setFlat(true);

	//auto moveLabel = new QLabel(QString::fromLocal8Bit("移动"));
	//auto rotateLabel = new QLabel(QString::fromLocal8Bit("旋转"));
	auto zoomLabel = new QLabel(QString::fromLocal8Bit("缩放"));
	auto fitAllLabel = new QLabel(QString::fromLocal8Bit("聚焦"));
	//auto resetLabel = new QLabel(QString::fromLocal8Bit("重置"));

	//auto moveVBox = new QVBoxLayout();
	//moveVBox->addWidget(m_moveBtn, 0, Qt::AlignHCenter);
	//moveVBox->addWidget(moveLabel, 0, Qt::AlignHCenter);
	//moveVBox->setSpacing(2);
	//moveVBox->setContentsMargins(4, 2, 4, 2);

	//auto rotateVBox = new QVBoxLayout();
	//rotateVBox->addWidget(m_rotateBtn, 0, Qt::AlignHCenter);
	//rotateVBox->addWidget(rotateLabel, 0, Qt::AlignHCenter);
	//rotateVBox->setSpacing(2);
	//rotateVBox->setContentsMargins(4, 2, 4, 2);

	auto zoomVBox = new QVBoxLayout();
	zoomVBox->addWidget(m_zoomBtn, 0, Qt::AlignHCenter);
	zoomVBox->addWidget(zoomLabel, 0, Qt::AlignHCenter);
	zoomVBox->setSpacing(2);
	zoomVBox->setContentsMargins(4, 2, 4, 2);

	auto fitAllVBox = new QVBoxLayout();
	fitAllVBox->addWidget(m_fitAllBtn, 0, Qt::AlignHCenter);
	fitAllVBox->addWidget(fitAllLabel, 0, Qt::AlignHCenter);
	fitAllVBox->setSpacing(2);
	fitAllVBox->setContentsMargins(4, 2, 4, 2);

	//auto resetVBox = new QVBoxLayout();
	//resetVBox->addWidget(m_resetBtn, 0, Qt::AlignHCenter);
	//resetVBox->addWidget(resetLabel, 0, Qt::AlignHCenter);
	//resetVBox->setSpacing(2);
	//resetVBox->setContentsMargins(4, 2, 4, 2);

	auto hLayout = new QHBoxLayout();
	//hLayout->addLayout(moveVBox);
	//hLayout->addLayout(rotateVBox);
	hLayout->addLayout(zoomVBox);
	hLayout->addLayout(fitAllVBox);
	//hLayout->addLayout(resetVBox);
	hLayout->addStretch();
	hLayout->setSpacing(4);
	hLayout->setContentsMargins(4, 2, 4, 2);

	auto vLayout = new QVBoxLayout();
	vLayout->addLayout(hLayout);
	vLayout->setContentsMargins(0, 0, 0, 0);
	vLayout->setSpacing(2);

	auto operationWidget = new QWidget();
	operationWidget->setFixedWidth(260);
	operationWidget->setLayout(vLayout);

	m_mainToolBar->addWidget(operationWidget);
	m_mainToolBar->addSeparator();
}

//void mainWidget::setupViewWidget()
//{
//	m_xBtn = new QPushButton();
//	m_yBtn = new QPushButton();
//	m_zBtn = new QPushButton();
//	m_xNegBtn = new QPushButton();
//	m_yNegBtn = new QPushButton();
//	m_zNegBtn = new QPushButton();
//
//	const int btnSize = 32;
//	m_xBtn->setFixedSize(btnSize, btnSize);
//	m_yBtn->setFixedSize(btnSize, btnSize);
//	m_zBtn->setFixedSize(btnSize, btnSize);
//	m_xNegBtn->setFixedSize(btnSize, btnSize);
//	m_yNegBtn->setFixedSize(btnSize, btnSize);
//	m_zNegBtn->setFixedSize(btnSize, btnSize);
//
//	m_xBtn->setIcon(QIcon(":/src/View all From +X.png"));
//	m_yBtn->setIcon(QIcon(":/src/View all From +Y.png"));
//	m_zBtn->setIcon(QIcon(":/src/View all From +Z.png"));
//	m_xNegBtn->setIcon(QIcon(":/src/View all From -X.png"));
//	m_yNegBtn->setIcon(QIcon(":/src/View all From -Y.png"));
//	m_zNegBtn->setIcon(QIcon(":/src/View all From -Z.png"));
//
//	auto xLabel = new QLabel(QString::fromLocal8Bit("+X"));
//	auto yLabel = new QLabel(QString::fromLocal8Bit("+Y"));
//	auto zLabel = new QLabel(QString::fromLocal8Bit("+Z"));
//	auto xNegLabel = new QLabel(QString::fromLocal8Bit("-X"));
//	auto yNegLabel = new QLabel(QString::fromLocal8Bit("-Y"));
//	auto zNegLabel = new QLabel(QString::fromLocal8Bit("-Z"));
//
//	auto xVBox = new QVBoxLayout();
//	xVBox->addWidget(m_xBtn, 0, Qt::AlignHCenter);
//	xVBox->addWidget(xLabel, 0, Qt::AlignHCenter);
//	xVBox->setSpacing(2);
//	xVBox->setContentsMargins(4, 2, 4, 2);
//
//	auto yVBox = new QVBoxLayout();
//	yVBox->addWidget(m_yBtn, 0, Qt::AlignHCenter);
//	yVBox->addWidget(yLabel, 0, Qt::AlignHCenter);
//	yVBox->setSpacing(2);
//	yVBox->setContentsMargins(4, 2, 4, 2);
//
//	auto zVBox = new QVBoxLayout();
//	zVBox->addWidget(m_zBtn, 0, Qt::AlignHCenter);
//	zVBox->addWidget(zLabel, 0, Qt::AlignHCenter);
//	zVBox->setSpacing(2);
//	zVBox->setContentsMargins(4, 2, 4, 2);
//
//	auto xNegVBox = new QVBoxLayout();
//	xNegVBox->addWidget(m_xNegBtn, 0, Qt::AlignHCenter);
//	xNegVBox->addWidget(xNegLabel, 0, Qt::AlignHCenter);
//	xNegVBox->setSpacing(2);
//	xNegVBox->setContentsMargins(4, 2, 4, 2);
//
//	auto yNegVBox = new QVBoxLayout();
//	yNegVBox->addWidget(m_yNegBtn, 0, Qt::AlignHCenter);
//	yNegVBox->addWidget(yNegLabel, 0, Qt::AlignHCenter);
//	yNegVBox->setSpacing(2);
//	yNegVBox->setContentsMargins(4, 2, 4, 2);
//
//	auto zNegVBox = new QVBoxLayout();
//	zNegVBox->addWidget(m_zNegBtn, 0, Qt::AlignHCenter);
//	zNegVBox->addWidget(zNegLabel, 0, Qt::AlignHCenter);
//	zNegVBox->setSpacing(2);
//	zNegVBox->setContentsMargins(4, 2, 4, 2);
//
//	auto hLayout1 = new QHBoxLayout();
//	hLayout1->addLayout(xVBox);
//	hLayout1->addLayout(yVBox);
//	hLayout1->addLayout(zVBox);
//	hLayout1->addLayout(xNegVBox);
//	hLayout1->addLayout(yNegVBox);
//	hLayout1->addLayout(zNegVBox);
//	hLayout1->addStretch();
//	hLayout1->setSpacing(4);
//	hLayout1->setContentsMargins(4, 2, 4, 2);
//
//	auto vLayout = new QVBoxLayout();
//	vLayout->addLayout(hLayout1);
//	vLayout->setContentsMargins(0, 0, 0, 0);
//	vLayout->setSpacing(2);
//
//	auto viewWidget = new QWidget();
//	viewWidget->setFixedWidth(280);
//	viewWidget->setLayout(vLayout);
//
//	m_mainToolBar->addWidget(viewWidget);
//	m_mainToolBar->addSeparator();
//}

void mainWidget::setViewDirection(ViewDirection dir)
{
	if (!m_importModelWid)
		return;

	auto occView = m_importModelWid->GetOccView();
	if (!occView)
		return;

	auto state = occView->GetCameraRotationState();
	if (!state)
		return;

	Handle(V3d_View) view = occView->getView();
	if (view.IsNull())
		return;

	switch (dir)
	{
	case View_Xpos: view->SetProj(V3d_Xpos); break;
	case View_Ypos: view->SetProj(V3d_Ypos); break;
	case View_Zpos: view->SetProj(V3d_Zpos); break;
	case View_Xneg: view->SetProj(V3d_Xneg); break;
	case View_Yneg: view->SetProj(V3d_Yneg); break;
	case View_Zneg: view->SetProj(V3d_Zneg); break;
	}
	occView->fitAll();
}

void mainWidget::bindConnect()
{
	// 导航Tab切换
	// 记录上一个有效的页面索引（排除帮助页）
	int m_prevValidIndex = 0;

	connect(m_navTabWidget, &QTabWidget::currentChanged, [this, &m_prevValidIndex](int index) {
		if (index == 0) {
			// 数据库
			m_pMainTabWidget->setCurrentIndex(1);
			m_mainToolBar->setVisible(false);
			m_prevValidIndex = 0;

			// 非admin用户隐藏用户数据库
			if (!m_dataBaseWid) return;
			QTreeWidget* treeWidget = m_dataBaseWid->getQTreeWid();
			if (!treeWidget) return;
			auto ins = ModelDataManager::GetInstance();
			if (!ins) return;
			UserInfo info = ins->GetUserInfo();
			if (info.username != "admin") {
				int size = treeWidget->topLevelItemCount();
				for (int i = 0; i < size; ++i) {
					QTreeWidgetItem* child = treeWidget->topLevelItem(i);
					if (child && child->text(0).contains(QString::fromLocal8Bit("用户数据库"))) {
						child->setHidden(true);
					}
				}
			}
		}
		else if (index == 1) {
			// 预置装注药模型
			m_pMainTabWidget->setCurrentIndex(0);
			m_mainToolBar->setVisible(true);
			m_prevValidIndex = 1;
		}
		else if (index == 2) {
			// 帮助 - 只弹出对话框，不改变当前界面
			QString aboutText = QString::fromLocal8Bit(
				"软件名称：TNT弹/DNAN粒状工业炸药注装药型罩参数匹配设计软件\n"
				"软件版本：V1.0.0\n"
				"版权所有：南京理工大学\n"
				"开发团队：南京理工大学\n"
				"联系邮箱：wuxingliang94@njust.edu.cn\n"
				"官方网站：https://www.njust.edu.cn\n"
				"版权声明：\n"
				"    本软件受版权保护，未经明确授权不得以任何形式复制、分发、修改或用于商业目的。\n"
				"    如有任何问题，请联系我们。"
			);
			QMessageBox::about(this,
				QString::fromLocal8Bit("TNT弹/DNAN粒状工业炸药注装药型罩参数匹配设计软件"),
				aboutText);

			// 帮助弹窗后，切回之前有效的页面，不改变主内容
			m_navTabWidget->setCurrentIndex(m_prevValidIndex);
		}
		});

	connect(m_importBtn, &QPushButton::clicked, [this]() {
		if (m_pMainTabWidget->currentIndex() == 0)
		{
			handleModelImport();
		}
		else if (m_pMainTabWidget->currentIndex() == 1)
		{
			handleExcelImport();
		}
		});

	if (m_importModelWid)
	{
		auto occView = m_importModelWid->GetOccView();
		if (occView) {
			//connect(m_moveBtn, &QPushButton::clicked, occView, &OccView::pan);
			//connect(m_rotateBtn, &QPushButton::clicked, occView, &OccView::rotate);
			connect(m_zoomBtn, &QPushButton::clicked, occView, &OccView::zoom);
			connect(m_fitAllBtn, &QPushButton::clicked, occView, &OccView::fitAll);
			//connect(m_resetBtn, &QPushButton::clicked, occView, &OccView::reset);
		}
	}

	//connect(m_xBtn, &QPushButton::clicked, [this]() { setViewDirection(View_Xpos); });
	//connect(m_yBtn, &QPushButton::clicked, [this]() { setViewDirection(View_Ypos); });
	//connect(m_zBtn, &QPushButton::clicked, [this]() { setViewDirection(View_Zpos); });
	//connect(m_xNegBtn, &QPushButton::clicked, [this]() { setViewDirection(View_Xneg); });
	//connect(m_yNegBtn, &QPushButton::clicked, [this]() { setViewDirection(View_Yneg); });
	//connect(m_zNegBtn, &QPushButton::clicked, [this]() { setViewDirection(View_Zneg); });
}

// ============================================================
// 处理模型导入
// ============================================================
void mainWidget::handleModelImport()
{
	if (!m_importModelWid) return;

	QString filePath = QFileDialog::getOpenFileName(this,
		QString::fromLocal8Bit("打开文件"),
		QDir::homePath(),
		QString::fromLocal8Bit("STEP Files (*.stp *.step);;IGES Files (*.iges *.igs);;STL Files (*.stl);;All Files (*.*)"));

	if (filePath.isEmpty()) return;

	// 记录日志
	auto logWidget = m_importModelWid->GetLogWidget();
	if (!logWidget) return;

	auto textEdit = logWidget->GetTextEdit();
	if (!textEdit) return;

	QDateTime currentTime = QDateTime::currentDateTime();
	QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
	textEdit->appendPlainText(timeStr + QString::fromLocal8Bit("[信息]>开始导入几何模型"));
	logWidget->update();

	TopoDS_Shape aShape;
	bool loadSuccess = false;
	ModelGeometryInfo info;

	try {
		if (filePath.endsWith(".stp", Qt::CaseInsensitive) ||
			filePath.endsWith(".step", Qt::CaseInsensitive)) {
			loadSuccess = loadStepFile(filePath, aShape, info);
		}
		else if (filePath.endsWith(".stl", Qt::CaseInsensitive)) {
			loadSuccess = loadStlFile(filePath, aShape, info);
		}
		else {
			QMessageBox::warning(this, QString::fromLocal8Bit("错误"),
				QString::fromLocal8Bit("不支持的文件格式"));
			return;
		}
	}
	catch (const Standard_Failure& e) {
		QMessageBox::critical(this, QString::fromLocal8Bit("导入错误"),
			QString::fromLocal8Bit("导入失败：") + QString(e.GetMessageString()));
		return;
	}
	catch (...) {
		QMessageBox::critical(this, QString::fromLocal8Bit("导入错误"),
			QString::fromLocal8Bit("导入过程中发生未知错误"));
		return;
	}

	if (!loadSuccess || aShape.IsNull()) {
		QMessageBox::warning(this, QString::fromLocal8Bit("错误"),
			QString::fromLocal8Bit("加载模型失败"));
		return;
	}

	// 保存模型信息
	auto manager = ModelDataManager::GetInstance();
	if (manager) {
		manager->SetModelGeometryInfo(info);
	}

	auto treeWidget = m_importModelWid->GetGFTreeModelWidget();
	if (treeWidget) {
		treeWidget->updataIcon();
	}

	// 显示到OCC视图
	auto occView = m_importModelWid->GetOccView();
	if (occView) {
		Handle(AIS_InteractiveContext) context = occView->getContext();
		if (!context.IsNull()) {
			context->EraseAll(true);
			Handle(AIS_Shape) modelPresentation = new AIS_Shape(aShape);
			context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
			context->SetColor(modelPresentation, Quantity_Color(0.0, 1.0, 1.0, Quantity_TOC_RGB), true);
			context->Display(modelPresentation, false);
			occView->fitAll();
		}
	}

	// 记录成功日志
	currentTime = QDateTime::currentDateTime();
	timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
	QString text = timeStr + QString::fromLocal8Bit("[信息]>导入几何模型,路径为：") + filePath;
	textEdit->appendPlainText(text);
}

// ============================================================
// 加载STEP文件
// ============================================================
bool mainWidget::loadStepFile(const QString& filePath, TopoDS_Shape& outShape, ModelGeometryInfo& outInfo)
{
	STEPControl_Reader reader;
	if (reader.ReadFile(filePath.toStdString().c_str()) != IFSelect_RetDone) {
		return false;
	}

	reader.PrintCheckLoad(Standard_False, IFSelect_ItemsByEntity);
	Standard_Integer nbRoots = reader.NbRootsForTransfer();
	if (nbRoots <= 0) return false;

	reader.TransferRoots();
	outShape = reader.OneShape();

	if (outShape.IsNull()) return false;

	return computeBBox(outShape, filePath, outInfo);
}

// ============================================================
// 加载STL文件
// ============================================================
bool mainWidget::loadStlFile(const QString& filePath, TopoDS_Shape& outShape, ModelGeometryInfo& outInfo)
{
	StlAPI_Reader reader;
	if (!reader.Read(outShape, filePath.toStdString().c_str())) {
		return false;
	}

	if (outShape.IsNull()) return false;

	return computeBBox(outShape, filePath, outInfo);
}

// ============================================================
// 计算边界框（消除重复代码）
// ============================================================
bool mainWidget::computeBBox(const TopoDS_Shape& shape, const QString& filePath, ModelGeometryInfo& outInfo)
{
	Bnd_Box bbox;
	BRepBndLib::Add(shape, bbox);
	bbox.SetGap(0.0);

	Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
	bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

	outInfo.shape = shape;
	outInfo.path = filePath;
	outInfo.theXmin = xmin;
	outInfo.theYmin = ymin;
	outInfo.theZmin = zmin;
	outInfo.theXmax = xmax;
	outInfo.theYmax = ymax;
	outInfo.theZmax = zmax;
	outInfo.length = static_cast<double>(xmax - xmin);
	outInfo.width = static_cast<double>(ymax - ymin);
	outInfo.height = static_cast<double>(zmax - zmin);

	return true;
}

// ============================================================
// 处理Excel导入
// ============================================================
void mainWidget::handleExcelImport()
{
	QString filter = "Excel files (*.xlsx *.xls)";
	QString filePath = QFileDialog::getOpenFileName(this,
		QString::fromLocal8Bit("打开Excel"),
		QDir::currentPath(), filter);

	if (filePath.isEmpty()) return;

	// TODO: 实现Excel导入逻辑
	qDebug() << "Excel file selected:" << filePath;
}

// ============================================================
// 内存和CPU监控
// ============================================================
void mainWidget::refreshMemoryUsage(QLabel* statusLabel)
{
	// 安全地重置定时器
	if (m_timer) {
		m_timer->stop();
		disconnect(m_timer, nullptr, nullptr, nullptr);
		delete m_timer;
	}

	m_timer = new QTimer(this);
	m_timer->setInterval(5000); // 5秒刷新
	connect(m_timer, &QTimer::timeout, [this, statusLabel]() {
		getMemoryUsage(statusLabel);
		});

	GetSystemTimes(&m_prevIdleTime, &m_prevKernelTime, &m_prevUserTime);
	m_isFirstSample = true;
	m_timer->start();
	getMemoryUsage(statusLabel);
}

void mainWidget::getMemoryUsage(QLabel* statusLabel)
{
	QString memoryText = "0.00";
	QString cpuText = "0.00";

	// 内存使用率
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	if (GlobalMemoryStatusEx(&statex)) {
		ULONGLONG totalPhys = statex.ullTotalPhys;
		ULONGLONG availPhys = statex.ullAvailPhys;
		if (totalPhys > 0) {
			double memoryUsage = ((totalPhys - availPhys) / static_cast<double>(totalPhys)) * 100.0;
			memoryText = QString::number(memoryUsage, 'f', 2);
		}
	}
	else {
		qWarning() << "Failed to get memory info, error:" << GetLastError();
		memoryText = QString::fromLocal8Bit("获取失败");
	}

	// CPU使用率
	FILETIME currIdleTime, currKernelTime, currUserTime;
	if (!GetSystemTimes(&currIdleTime, &currKernelTime, &currUserTime)) {
		qWarning() << "Failed to get system times, error:" << GetLastError();
		cpuText = QString::fromLocal8Bit("获取失败");
	}
	else {
		if (m_isFirstSample) {
			m_prevIdleTime = currIdleTime;
			m_prevKernelTime = currKernelTime;
			m_prevUserTime = currUserTime;
			m_isFirstSample = false;
			cpuText = "0.00";
		}
		else {
			ULONGLONG idleDiff = fileTimeToULL(currIdleTime) - fileTimeToULL(m_prevIdleTime);
			ULONGLONG kernelDiff = fileTimeToULL(currKernelTime) - fileTimeToULL(m_prevKernelTime);
			ULONGLONG userDiff = fileTimeToULL(currUserTime) - fileTimeToULL(m_prevUserTime);

			ULONGLONG totalSysDiff = kernelDiff + userDiff;

			if (totalSysDiff == 0) {
				cpuText = "0.00";
			}
			else {
				double cpuUsage = (1.0 - static_cast<double>(idleDiff) / totalSysDiff) * 100.0;
				cpuUsage = qBound(0.0, cpuUsage, 100.0);
				cpuText = QString::number(cpuUsage, 'f', 2);
			}

			m_prevIdleTime = currIdleTime;
			m_prevKernelTime = currKernelTime;
			m_prevUserTime = currUserTime;
		}
	}

	statusLabel->setText(QString::fromLocal8Bit("内存使用：%1%，CPU使用：%2%")
		.arg(memoryText).arg(cpuText));
}

ULONGLONG mainWidget::fileTimeToULL(const FILETIME& ft)
{
	ULARGE_INTEGER ul;
	ul.LowPart = ft.dwLowDateTime;
	ul.HighPart = ft.dwHighDateTime;
	return ul.QuadPart;
}