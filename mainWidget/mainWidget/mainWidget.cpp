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

// ============================================================
// 构造函数 / 析构函数
// ============================================================
mainWidget::mainWidget(QWidget* parent)
	: QMainWindow(parent)
{
	// 使用临时 UI 对象初始化界面，然后释放
	Ui::mainWidgetClass uiSetup;
	uiSetup.setupUi(this);

	// 保存需要后续访问的 UI 元素指针
	m_menuBar = uiSetup.menuBar;
	m_mainToolBar = uiSetup.mainToolBar;

	init();
	bindConnect();
}

mainWidget::~mainWidget()
{
	// 停止并清理定时器
	if (m_timer) {
		m_timer->stop();
		delete m_timer;
		m_timer = nullptr;
	}
}

// ============================================================
// 初始化界面
// ============================================================
void mainWidget::init()
{
	setWindowIcon(QIcon(":/src/engine.svg"));
	setWindowTitle(QString::fromLocal8Bit("TNT弹/DNAN粒状工业炸药注装药型罩参数匹配设计软件"));

	// 状态栏
	QStatusBar* statusbar = statusBar();
	m_statusLabel = new QLabel(QString::fromLocal8Bit("内存使用：0%，CPU使用：0%"));
	statusbar->addPermanentWidget(m_statusLabel);
	refreshMemoryUsage(m_statusLabel);

	// 菜单栏
	m_ImportModelWidAct = new QAction(QString::fromLocal8Bit("预置装注药模型"), m_menuBar);
	m_DataBaseWidAct = new QAction(QString::fromLocal8Bit("数据库"), m_menuBar);
	m_HelpAct = new QAction(QString::fromLocal8Bit("帮助"), m_menuBar);

	m_menuBar->addAction(m_DataBaseWidAct);
	m_menuBar->addAction(m_ImportModelWidAct);
	m_menuBar->addAction(m_HelpAct);

	// 工具栏设置
	m_mainToolBar->setMovable(false);
	m_mainToolBar->setFloatable(false);

	// 设置工具栏各区域
	setupGeomWidget();
	setupOperationWidget();
	setupViewWidget();

	// TabWidget
	m_pMainTabWidget = new QTabWidget(this);
	m_importModelWid = new GFImportModelWidget(m_pMainTabWidget);
	m_dataBaseWid = new DatabaseWidget(m_pMainTabWidget);

	m_pMainTabWidget->addTab(m_importModelWid, "importModelWid");
	m_pMainTabWidget->addTab(m_dataBaseWid, "dataBaseWid");
	m_pMainTabWidget->tabBar()->setVisible(false);

	setCentralWidget(m_pMainTabWidget);
}

// ============================================================
// 设置几何操作工具栏
// ============================================================
void mainWidget::setupGeomWidget()
{
	m_importBtn = new QPushButton();
	m_saveBtn = new QPushButton();
	m_saveAsBtn = new QPushButton();
	m_exportBtn = new QPushButton();

	m_importBtn->setIcon(QIcon(":/src/Import.svg"));
	m_saveBtn->setIcon(QIcon(":/src/Save.svg"));
	m_saveAsBtn->setIcon(QIcon(":/src/Save_as.svg"));
	m_exportBtn->setIcon(QIcon(":/src/Export.svg"));

	const int btnSize = 32;
	m_importBtn->setFixedSize(btnSize, btnSize);
	m_saveBtn->setFixedSize(btnSize, btnSize);
	m_saveAsBtn->setFixedSize(btnSize, btnSize);
	m_exportBtn->setFixedSize(btnSize, btnSize);

	auto importLabel = new QLabel(QString::fromLocal8Bit("导入文件"));
	auto saveLabel = new QLabel(QString::fromLocal8Bit("保存文件"));
	auto saveAsLabel = new QLabel(QString::fromLocal8Bit("另存为..."));
	auto exportLabel = new QLabel(QString::fromLocal8Bit("导出文件"));

	// 图标在上，文字在下的纵向布局
	auto importVBox = new QVBoxLayout();
	importVBox->addWidget(m_importBtn, 0, Qt::AlignHCenter);
	importVBox->addWidget(importLabel, 0, Qt::AlignHCenter);
	importVBox->setSpacing(2);
	importVBox->setContentsMargins(4, 2, 4, 2);

	auto saveAsVBox = new QVBoxLayout();
	saveAsVBox->addWidget(m_saveAsBtn, 0, Qt::AlignHCenter);
	saveAsVBox->addWidget(saveAsLabel, 0, Qt::AlignHCenter);
	saveAsVBox->setSpacing(2);
	saveAsVBox->setContentsMargins(4, 2, 4, 2);

	auto saveVBox = new QVBoxLayout();
	saveVBox->addWidget(m_saveBtn, 0, Qt::AlignHCenter);
	saveVBox->addWidget(saveLabel, 0, Qt::AlignHCenter);
	saveVBox->setSpacing(2);
	saveVBox->setContentsMargins(4, 2, 4, 2);

	auto exportVBox = new QVBoxLayout();
	exportVBox->addWidget(m_exportBtn, 0, Qt::AlignHCenter);
	exportVBox->addWidget(exportLabel, 0, Qt::AlignHCenter);
	exportVBox->setSpacing(2);
	exportVBox->setContentsMargins(4, 2, 4, 2);

	// 横向排列各功能组
	auto hLayout = new QHBoxLayout();
	hLayout->addLayout(importVBox);
	hLayout->addLayout(saveAsVBox);
	hLayout->addLayout(saveVBox);
	hLayout->addLayout(exportVBox);
	hLayout->addStretch();
	hLayout->setSpacing(4);
	hLayout->setContentsMargins(4, 2, 4, 2);

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
	m_moveBtn = new QPushButton();
	m_rotateBtn = new QPushButton();
	m_zoomBtn = new QPushButton();
	m_fitAllBtn = new QPushButton();
	m_resetBtn = new QPushButton();

	m_moveBtn->setIcon(QIcon(":/src/Move.svg"));
	m_rotateBtn->setIcon(QIcon(":/src/Rotate.svg"));
	m_zoomBtn->setIcon(QIcon(":/src/Zoom.png"));
	m_fitAllBtn->setIcon(QIcon(":/src/FitAll.png"));
	m_resetBtn->setIcon(QIcon(":/src/Reset.svg"));

	const int btnSize = 32;
	m_moveBtn->setFixedSize(btnSize, btnSize);
	m_rotateBtn->setFixedSize(btnSize, btnSize);
	m_zoomBtn->setFixedSize(btnSize, btnSize);
	m_fitAllBtn->setFixedSize(btnSize, btnSize);
	m_resetBtn->setFixedSize(btnSize, btnSize);

	auto moveLabel = new QLabel(QString::fromLocal8Bit("移动"));
	auto rotateLabel = new QLabel(QString::fromLocal8Bit("旋转"));
	auto zoomLabel = new QLabel(QString::fromLocal8Bit("缩放"));
	auto fitAllLabel = new QLabel(QString::fromLocal8Bit("聚焦"));
	auto resetLabel = new QLabel(QString::fromLocal8Bit("重置"));

	// 图标在上，文字在下的纵向布局
	auto moveVBox = new QVBoxLayout();
	moveVBox->addWidget(m_moveBtn, 0, Qt::AlignHCenter);
	moveVBox->addWidget(moveLabel, 0, Qt::AlignHCenter);
	moveVBox->setSpacing(2);
	moveVBox->setContentsMargins(4, 2, 4, 2);

	auto rotateVBox = new QVBoxLayout();
	rotateVBox->addWidget(m_rotateBtn, 0, Qt::AlignHCenter);
	rotateVBox->addWidget(rotateLabel, 0, Qt::AlignHCenter);
	rotateVBox->setSpacing(2);
	rotateVBox->setContentsMargins(4, 2, 4, 2);

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

	auto resetVBox = new QVBoxLayout();
	resetVBox->addWidget(m_resetBtn, 0, Qt::AlignHCenter);
	resetVBox->addWidget(resetLabel, 0, Qt::AlignHCenter);
	resetVBox->setSpacing(2);
	resetVBox->setContentsMargins(4, 2, 4, 2);

	// 横向排列各功能组
	auto hLayout = new QHBoxLayout();
	hLayout->addLayout(moveVBox);
	hLayout->addLayout(rotateVBox);
	hLayout->addLayout(zoomVBox);
	hLayout->addLayout(fitAllVBox);
	hLayout->addLayout(resetVBox);
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


void mainWidget::setupViewWidget()
{
	m_xBtn = new QPushButton();
	m_yBtn = new QPushButton();
	m_zBtn = new QPushButton();
	m_xNegBtn = new QPushButton();
	m_yNegBtn = new QPushButton();
	m_zNegBtn = new QPushButton();

	const int btnSize = 32;
	m_xBtn->setFixedSize(btnSize, btnSize);
	m_yBtn->setFixedSize(btnSize, btnSize);
	m_zBtn->setFixedSize(btnSize, btnSize);
	m_xNegBtn->setFixedSize(btnSize, btnSize);
	m_yNegBtn->setFixedSize(btnSize, btnSize);
	m_zNegBtn->setFixedSize(btnSize, btnSize);

	m_xBtn->setIcon(QIcon(":/src/View all From +X.png"));
	m_yBtn->setIcon(QIcon(":/src/View all From +Y.png"));
	m_zBtn->setIcon(QIcon(":/src/View all From +Z.png"));
	m_xNegBtn->setIcon(QIcon(":/src/View all From -X.png"));
	m_yNegBtn->setIcon(QIcon(":/src/View all From -Y.png"));
	m_zNegBtn->setIcon(QIcon(":/src/View all From -Z.png"));

	auto xLabel = new QLabel(QString::fromLocal8Bit("+X"));
	auto yLabel = new QLabel(QString::fromLocal8Bit("+Y"));
	auto zLabel = new QLabel(QString::fromLocal8Bit("+Z"));
	auto xNegLabel = new QLabel(QString::fromLocal8Bit("-X"));
	auto yNegLabel = new QLabel(QString::fromLocal8Bit("-Y"));
	auto zNegLabel = new QLabel(QString::fromLocal8Bit("-Z"));

	// 图标在上，文字在下的纵向布局
	auto xVBox = new QVBoxLayout();
	xVBox->addWidget(m_xBtn, 0, Qt::AlignHCenter);
	xVBox->addWidget(xLabel, 0, Qt::AlignHCenter);
	xVBox->setSpacing(2);
	xVBox->setContentsMargins(4, 2, 4, 2);

	auto yVBox = new QVBoxLayout();
	yVBox->addWidget(m_yBtn, 0, Qt::AlignHCenter);
	yVBox->addWidget(yLabel, 0, Qt::AlignHCenter);
	yVBox->setSpacing(2);
	yVBox->setContentsMargins(4, 2, 4, 2);

	auto zVBox = new QVBoxLayout();
	zVBox->addWidget(m_zBtn, 0, Qt::AlignHCenter);
	zVBox->addWidget(zLabel, 0, Qt::AlignHCenter);
	zVBox->setSpacing(2);
	zVBox->setContentsMargins(4, 2, 4, 2);

	auto xNegVBox = new QVBoxLayout();
	xNegVBox->addWidget(m_xNegBtn, 0, Qt::AlignHCenter);
	xNegVBox->addWidget(xNegLabel, 0, Qt::AlignHCenter);
	xNegVBox->setSpacing(2);
	xNegVBox->setContentsMargins(4, 2, 4, 2);

	auto yNegVBox = new QVBoxLayout();
	yNegVBox->addWidget(m_yNegBtn, 0, Qt::AlignHCenter);
	yNegVBox->addWidget(yNegLabel, 0, Qt::AlignHCenter);
	yNegVBox->setSpacing(2);
	yNegVBox->setContentsMargins(4, 2, 4, 2);

	auto zNegVBox = new QVBoxLayout();
	zNegVBox->addWidget(m_zNegBtn, 0, Qt::AlignHCenter);
	zNegVBox->addWidget(zNegLabel, 0, Qt::AlignHCenter);
	zNegVBox->setSpacing(2);
	zNegVBox->setContentsMargins(4, 2, 4, 2);

	// 第一行：+X, +Y, +Z
	auto hLayout1 = new QHBoxLayout();
	hLayout1->addLayout(xVBox);
	hLayout1->addLayout(yVBox);
	hLayout1->addLayout(zVBox);
	hLayout1->addLayout(xNegVBox);
	hLayout1->addLayout(yNegVBox);
	hLayout1->addLayout(zNegVBox);
	hLayout1->addStretch();
	hLayout1->setSpacing(4);
	hLayout1->setContentsMargins(4, 2, 4, 2);

	auto vLayout = new QVBoxLayout();
	vLayout->addLayout(hLayout1);
	vLayout->setContentsMargins(0, 0, 0, 0);
	vLayout->setSpacing(2);

	auto viewWidget = new QWidget();
	viewWidget->setFixedWidth(280);
	viewWidget->setLayout(vLayout);

	m_mainToolBar->addWidget(viewWidget);
	m_mainToolBar->addSeparator();
}


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
	connect(m_ImportModelWidAct, &QAction::triggered, [this]() {
		m_pMainTabWidget->setCurrentIndex(0);
		m_mainToolBar->setVisible(true);
		});

	connect(m_DataBaseWidAct, &QAction::triggered, [this]() {
		m_pMainTabWidget->setCurrentIndex(1);
		m_mainToolBar->setVisible(false);

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
		});

	connect(m_HelpAct, &QAction::triggered, [this]() {
		QString aboutText = QString::fromLocal8Bit(
			"软件名称：TNT弹/DNAN粒状工业炸药注装药型罩参数匹配设计软件\n"
			"软件版本：V1.0.0\n"
			"版权所有：南京理工大学\n"
			"开发团队：南京理工大学\n"
			"联系邮箱：wuxingliang94@njust.edu.cn\n"
			"官方网站：https://www.njust.edu.cn\n"
			"版权声明：\n"
			"    本软件受版权法保护，未经明确授权，严禁以任何形式复制、分发、修改或用于工程目的。\n"
			"    如需帮助，请随时联系我们。"
		);
		QMessageBox::about(this,
			QString::fromLocal8Bit("TNT弹/DNAN粒状工业炸药注装药型罩参数匹配设计软件"),
			aboutText);
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
			connect(m_moveBtn, &QPushButton::clicked, occView, &OccView::pan);
			connect(m_rotateBtn, &QPushButton::clicked, occView, &OccView::rotate);
			connect(m_zoomBtn, &QPushButton::clicked, occView, &OccView::zoom);
			connect(m_fitAllBtn, &QPushButton::clicked, occView, &OccView::fitAll);
			connect(m_resetBtn, &QPushButton::clicked, occView, &OccView::reset);
		}
	}

	connect(m_xBtn, &QPushButton::clicked, [this]() { setViewDirection(View_Xpos); });
	connect(m_yBtn, &QPushButton::clicked, [this]() { setViewDirection(View_Ypos); });
	connect(m_zBtn, &QPushButton::clicked, [this]() { setViewDirection(View_Zpos); });
	connect(m_xNegBtn, &QPushButton::clicked, [this]() { setViewDirection(View_Xneg); });
	connect(m_yNegBtn, &QPushButton::clicked, [this]() { setViewDirection(View_Yneg); });
	connect(m_zNegBtn, &QPushButton::clicked, [this]() { setViewDirection(View_Zneg); });
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