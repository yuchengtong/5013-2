#pragma execution_character_set("utf-8")
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
#include <QtCharts\qchartview.h>
#include <QFileDialog>
#include "xlsxdocument.h"

#include <AIS_Shape.hxx>
#include <STEPControl_Reader.hxx>
#include <Prs3d_LineAspect.hxx>
#include <V3d_View.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepMesh_Context.hxx>  
#include <BRepBndLib.hxx>
#include <StlAPI_Reader.hxx>

#include "GFImportModelWidget.h"
#include "GFLogWidget.h"
#include "DatabaseWidget.h"
#include "OccView.h"
#include "GFTreeModelWidget.h"



mainWidget::mainWidget(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::mainWidgetClass())
{
	setWindowIcon(QIcon(":/src/engine.svg"));
	setStyleSheet("QPushButton {"
                           "background-color:  rgba(0, 0, 0, 0);"
                           "}"
                           "QPushButton:hover {"
                           "background-color: white;"
		"}");

    ui->setupUi(this);
	setWindowTitle("注装工艺参数设计软件V1.0");
	//showMaximized();
	setMinimumSize(1900, 1300);

	// 状态栏
	QStatusBar* statusbar = this->statusBar();
	this->setStatusBar(statusbar);
	QLabel *m_statusLabel = new QLabel("内存使用：0%，CPU使用：0%");
	statusbar->addPermanentWidget(m_statusLabel);
	refreshMemoryUsage(m_statusLabel);

	m_ImportModelWidAct = new QAction("预热与注药工艺", ui->menuBar);
	m_DataBaseWidAct = new QAction("数据库", ui->menuBar);
	m_HelpAct = new QAction("帮助", ui->menuBar);

	ui->menuBar->addAction(m_DataBaseWidAct);
	ui->menuBar->addAction(m_ImportModelWidAct);
	ui->menuBar->addAction(m_HelpAct);


	ui->mainToolBar->setMovable(false);
	ui->mainToolBar->setFloatable(false);
//////////////////////////////////////////////////////////ToolBar
	auto ImportBtn = new QPushButton();
	auto SaveBtn = new QPushButton();
	auto SaveAsBtn = new QPushButton();
	auto ExportBtn= new QPushButton();
	ImportBtn->setIcon(QIcon(":/src/Import.svg"));
	SaveBtn->setIcon(QIcon(":/src/Save.svg"));
	SaveAsBtn->setIcon(QIcon(":/src/Save_as.svg"));
	ExportBtn->setIcon(QIcon(":/src/Export.svg"));
	ImportBtn->setFixedSize(32,32);
	SaveBtn->setFixedSize(32, 32);
	SaveAsBtn->setFixedSize(32, 32);
	ExportBtn->setFixedSize(32, 32);

	auto ImportLabel = new QLabel("导入文件");
	auto SaveLabel = new QLabel("保存文件");
	auto SaveAsLabel= new QLabel("另存为...");
	auto ExportLabel= new QLabel("导出文件");
	auto bottomTitleLab1 = new QLabel("几何");

	auto geomWidget = new QWidget();
	geomWidget->setFixedWidth(172);
	auto hLayout1 = new QHBoxLayout();
	auto hLayout2 = new QHBoxLayout();
	auto hLayout3 = new QHBoxLayout();
	hLayout1->addWidget(ImportBtn);
	hLayout1->addWidget(ImportLabel);
	hLayout1->setSpacing(0);
	hLayout1->addWidget(SaveAsBtn);
	hLayout1->addWidget(SaveAsLabel);
	hLayout1->setContentsMargins(0, 0, 0, 0);
	hLayout2->addWidget(SaveBtn);
	hLayout2->addWidget(SaveLabel);
	hLayout2->setSpacing(0);
	hLayout2->addWidget(ExportBtn);
	hLayout2->addWidget(ExportLabel);
	hLayout2->setContentsMargins(0, 0, 0, 0);
	hLayout3->addStretch();
	hLayout3->addWidget(bottomTitleLab1);
	hLayout3->addStretch();
	hLayout3->setContentsMargins(0, 0, 0, 0);
	auto vLayout = new QVBoxLayout();
	vLayout->addLayout(hLayout1);
	vLayout->addLayout(hLayout2);
	vLayout->addLayout(hLayout3);
	vLayout->setContentsMargins(0, 0, 0, 0);
	geomWidget->setLayout(vLayout);



	auto MoveBtn = new QPushButton();
	auto RotateBtn = new QPushButton();
	auto ZoomBtn = new QPushButton();
	auto FitAllBtn = new QPushButton();
	auto ResetBtn= new QPushButton();
	auto nullBtn= new QPushButton();
	nullBtn->setEnabled(false);
	MoveBtn->setIcon(QIcon(":/src/Move.svg"));
	RotateBtn->setIcon(QIcon(":/src/Rotate.svg"));
	ZoomBtn->setIcon(QIcon(":/src/Zoom.png"));
	FitAllBtn->setIcon(QIcon(":/src/FitAll.png"));
	ResetBtn->setIcon(QIcon(":/src/Reset.svg"));
	MoveBtn->setFixedSize(32, 32);
	RotateBtn->setFixedSize(32, 32);
	ZoomBtn->setFixedSize(32, 32);
	FitAllBtn->setFixedSize(32, 32);
	ResetBtn->setFixedSize(32, 32);
	auto MoveLabel = new QLabel("移动");
	auto RotateLabel = new QLabel("旋转");
	auto ZoomLabel = new QLabel("缩放");
	auto FitAllLabel = new QLabel("聚焦");
	auto ResetLabel= new QLabel("重置");
	auto nullLabel = new QLabel("");

	auto bottomTitleLab_o = new QLabel("操作");

	auto operationWidget = new QWidget();
	operationWidget->setFixedWidth(184);
	auto hLayout_o1 = new QHBoxLayout();
	auto hLayout_o2 = new QHBoxLayout();
	auto hLayout_o3 = new QHBoxLayout();
	hLayout_o1->addWidget(MoveBtn);
	hLayout_o1->addWidget(MoveLabel);
	hLayout_o1->setSpacing(0);
	hLayout_o1->addWidget(ZoomBtn);
	hLayout_o1->addWidget(ZoomLabel);
	hLayout_o1->setSpacing(0);
	hLayout_o1->addWidget(ResetBtn);
	hLayout_o1->addWidget(ResetLabel);
	hLayout_o1->setContentsMargins(0, 0, 0, 0);
	hLayout_o2->addWidget(RotateBtn);
	hLayout_o2->addWidget(RotateLabel);
	hLayout_o2->setSpacing(0);
	hLayout_o2->addWidget(FitAllBtn);
	hLayout_o2->addWidget(FitAllLabel);
	hLayout_o2->setSpacing(0);
	hLayout_o2->addWidget(nullBtn);
	hLayout_o2->addWidget(nullLabel);

	hLayout_o2->setContentsMargins(0, 0, 0, 0);
	hLayout_o3->addStretch();
	hLayout_o3->addWidget(bottomTitleLab_o);
	hLayout_o3->addStretch();
	hLayout_o3->setContentsMargins(0, 0, 0, 0);
	auto vLayout_o = new QVBoxLayout();
	vLayout_o->addLayout(hLayout_o1);
	vLayout_o->addLayout(hLayout_o2);
	vLayout_o->addLayout(hLayout_o3);
	vLayout_o->setContentsMargins(0, 0, 0, 0);
	operationWidget->setLayout(vLayout_o);

	auto XBtn = new QPushButton();
	auto YBtn = new QPushButton();
	auto ZBtn = new QPushButton();
	auto _XBtn = new QPushButton();
	auto _YBtn = new QPushButton();
	auto _ZBtn = new QPushButton();
	XBtn->setFixedSize(32, 32);
	YBtn->setFixedSize(32, 32);
	ZBtn->setFixedSize(32, 32);
	_XBtn->setFixedSize(32, 32);
	_YBtn->setFixedSize(32, 32);
	_ZBtn->setFixedSize(32, 32);

	XBtn->setIcon(QIcon(":/src/View all From +X.png"));
	YBtn->setIcon(QIcon(":/src/View all From +Y.png"));
	ZBtn->setIcon(QIcon(":/src/View all From +Z.png"));
	_XBtn->setIcon(QIcon(":/src/View all From -X.png"));
	_YBtn->setIcon(QIcon(":/src/View all From -Y.png"));
	_ZBtn->setIcon(QIcon(":/src/View all From -Z.png"));
	auto XLabel = new QLabel("X轴方向");
	auto YLabel = new QLabel("Y轴方向");
	auto ZLabel = new QLabel("Z轴方向");
	auto _XLabel = new QLabel("负X轴方向");
	auto _YLabel = new QLabel("负Y轴方向");
	auto _ZLabel = new QLabel("负Z轴方向");
	auto bottomTitleLab2 = new QLabel("视图");

	auto viewWidget = new QWidget();
	viewWidget->setFixedWidth(265);
	auto hLayout_v1 = new QHBoxLayout();
	auto hLayout_v2 = new QHBoxLayout();
	auto hLayout_v3 = new QHBoxLayout();
	hLayout_v1->addWidget(XBtn);
	hLayout_v1->addWidget(XLabel);
	hLayout_v1->setSpacing(0);
	hLayout_v1->addWidget(YBtn);
	hLayout_v1->addWidget(YLabel);
	hLayout_v1->setSpacing(0);
	hLayout_v1->addWidget(ZBtn);
	hLayout_v1->addWidget(ZLabel);
	hLayout_v1->setContentsMargins(0,0,0,0);
	hLayout_v2->addWidget(_XBtn);
	hLayout_v2->addWidget(_XLabel);
	hLayout_v2->setSpacing(0);
	hLayout_v2->addWidget(_YBtn);
	hLayout_v2->addWidget(_YLabel);
	hLayout_v2->setSpacing(0);
	hLayout_v2->addWidget(_ZBtn);
	hLayout_v2->addWidget(_ZLabel);
	hLayout_v2->setContentsMargins(0, 0, 0, 0);
	hLayout_v3->addStretch();
	hLayout_v3->addWidget(bottomTitleLab2);
	hLayout_v3->addStretch();
	auto vLayout_v = new QVBoxLayout();
	vLayout_v->addLayout(hLayout_v1);
	vLayout_v->addLayout(hLayout_v2);
	vLayout_v->addLayout(hLayout_v3);
	vLayout_v->setContentsMargins(0, 0, 0, 0);
	viewWidget->setLayout(vLayout_v);


	/*auto SettingBtn = new QPushButton();
	SettingBtn->setFixedSize(67, 67);
	SettingBtn->setIcon(QIcon(":/src/Setting.svg"));
	SettingBtn->setIconSize(QSize(50, 50));
	auto SettingLabel = new QLabel("设置");

	auto settingWidget = new QWidget();
	settingWidget->setFixedWidth(69);
	auto hLayout_s = new QHBoxLayout();
	hLayout_s->addStretch();
	hLayout_s->addWidget(SettingLabel);
	hLayout_s->addStretch();

	auto vLayout_s = new QVBoxLayout();
	vLayout_s->addWidget(SettingBtn);
	vLayout_s->addLayout(hLayout_s);
	vLayout_s->setContentsMargins(0, 0, 0, 0);
	settingWidget->setLayout(vLayout_s);*/

	ui->mainToolBar->addWidget(geomWidget);
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addWidget(operationWidget);
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addWidget(viewWidget);
	ui->mainToolBar->addSeparator();
	//ui->mainToolBar->addWidget(settingWidget);
	//ui->mainToolBar->addSeparator();


	m_TabWidget = new QTabWidget(this);

	GFImportModelWidget*importModelWid = new GFImportModelWidget(m_TabWidget);
	{
	}

	DatabaseWidget*dataBaseWid = new DatabaseWidget(m_TabWidget);
	{
	}

	


	m_TabWidget->addTab(importModelWid, "importModelWid");
	m_TabWidget->addTab(dataBaseWid, "dataBaseWid");
	m_TabWidget->tabBar()->setVisible(false);


	setCentralWidget(m_TabWidget);


	QObject::connect(m_ImportModelWidAct, &QAction::triggered, [=]() {
		m_TabWidget->setCurrentIndex(0);
		// 显示工具栏
		ui->mainToolBar->setVisible(true);
	});
		
	QObject::connect(m_DataBaseWidAct, &QAction::triggered, [=]() {
		m_TabWidget->setCurrentIndex(1);
		// 隐藏工具栏
		ui->mainToolBar->setVisible(false);
		// 非admin用户，隐藏用户数据库
		QTreeWidget* treeWidget = dataBaseWid->getQTreeWid();
		auto ins = ModelDataManager::GetInstance();
		UserInfo info = ins->GetUserInfo();
		if (info.username != "admin")
		{
			QTreeWidgetItem *child;
			int size = treeWidget->topLevelItemCount();
			for (int i = 0; i < size; i++)
			{
				child = treeWidget->topLevelItem(i);
				if (child->text(0).contains("用户数据库"))
				{
					child->setHidden(true);
				}
			}
		}
	});

	


	QObject::connect(m_HelpAct, &QAction::triggered, [=]() {
		QString aboutText = "**[软件名称] - 注装工艺参数设计软件V1.0**\n\n"
			"**版本信息**:\n"
			"- **软件版本**: [v1.0.0]\n"
			"**版权信息**:\n"
			"- **版权所有**: [版权所有者或公司名称]\n"
			"- **版权声明**:\n"
			"本软件受版权法和国际条约的保护。未经版权所有者的明确书面许可，严禁对本软件进行任何形式的复制、分发、修改或逆向工程。本软件的部分组件可能使用了第三方的开源软件，这些软件遵循各自的开源许可协议，具体信息可在软件的 LICENSE 文件或相应组件的文档中找到。\n\n"
			"**开发团队**:\n"
			"- **开发团队**: [开发团队或组织的名称]\n"
			"**联系我们**:\n"
			"如果您对本软件有任何问题、建议或反馈，请随时联系我们:\n"
			"- **电子邮件**: [联系邮箱]\n"
			"- **网站**: [官方网站]";
		QMessageBox::about(nullptr, "注装工艺参数设计软件V1.0", aboutText);
	});
	


	QObject::connect(ImportBtn, &QPushButton::clicked, [this,importModelWid]() {
		if (m_TabWidget->currentIndex() == 0) {
			// 打开文件对话框
			QString filePath = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(),
				"STEP Files (*.stp *.step);;IGES Files (*.iges *.igs);;VTK Files (*.vtk);;X_T Files (*.x_t);;All Files (*.*)");

			if (filePath.isEmpty()) 
				return;
			QDateTime currentTime = QDateTime::currentDateTime();
			QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
			auto logWidget = importModelWid->GetLogWidget();
			auto textEdit = logWidget->GetTextEdit();
			textEdit->appendPlainText(timeStr + "[信息]>开始导入几何模型");
			logWidget->update();

			TopoDS_Shape aShape;

			bool loadSuccess = false;

			ModelGeometryInfo info;

			// 根据文件扩展名选择适当的读取器
			if (filePath.endsWith(".stp", Qt::CaseInsensitive) || filePath.endsWith(".step", Qt::CaseInsensitive))
			{
				STEPControl_Reader aReader_Step;
				if (aReader_Step.ReadFile(filePath.toStdString().c_str()) == IFSelect_RetDone) {
					aReader_Step.PrintCheckLoad(Standard_False, IFSelect_ItemsByEntity);
					Standard_Integer NbRoots = aReader_Step.NbRootsForTransfer();
					Standard_Integer num = aReader_Step.TransferRoots();
					aShape = aReader_Step.OneShape();

					Bnd_Box bbox;
					BRepBndLib::Add(aShape, bbox);
					bbox.SetGap(0.0); // 消除间隙

					gp_Pnt bboxMin, bboxMax;
					Standard_Real theXmin, theYmin, theZmin, theXmax, theYmax, theZmax;
					bbox.Get(theXmin, theYmin, theZmin, theXmax, theYmax, theZmax); // 获取边界盒最小/最大点(包围盒)
					auto length = double(theXmax - theXmin);
					auto width = double(theYmax - theYmin);
					auto height = double(theZmax - theZmin);

					info.shape = aShape;
					info.path = filePath;
					info.theXmin = theXmin;
					info.theYmin = theYmin;
					info.theZmin = theZmin;
					info.theXmax = theXmax;
					info.theYmax = theYmax;
					info.theZmax = theZmax;

					info.length = length;
					info.width = width;
					info.height = height;
					ModelDataManager::GetInstance()->SetModelGeometryInfo(info);

					importModelWid->GetGFTreeModelWidget()->updataIcon();

					loadSuccess = true;
				}
			}
			else if (filePath.endsWith(".stl", Qt::CaseInsensitive)) {
				StlAPI_Reader aReader_Stl;
				// 读取STL文件
				if (aReader_Stl.Read(aShape, filePath.toStdString().c_str()))
				{
					// 计算边界盒（与STEP处理方式一致）
					Bnd_Box bbox;
					BRepBndLib::Add(aShape, bbox);
					bbox.SetGap(0.0); // 消除间隙

					Standard_Real theXmin, theYmin, theZmin, theXmax, theYmax, theZmax;
					bbox.Get(theXmin, theYmin, theZmin, theXmax, theYmax, theZmax);

					// 计算尺寸（与STEP处理方式一致）
					auto length = double(theXmax - theXmin);
					auto width = double(theYmax - theYmin);
					auto height = double(theZmax - theZmin);

					// 统一的信息存储（与STEP使用相同的数据结构）
					info.shape = aShape;
					info.path = filePath;
					info.length = length;
					info.width = width;
					info.height = height;
					ModelDataManager::GetInstance()->SetModelGeometryInfo(info);

					importModelWid->GetGFTreeModelWidget()->updataIcon();

					loadSuccess = true;
				}
			}
		//	else if (filePath.endsWith(".vtk", Qt::CaseInsensitive)) {
		//		vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
		//		reader->SetFileName(filePath.toStdString().c_str());
		//		reader->Update();

		//		vtkPolyData* polyData = reader->GetOutput();
		//		if (!polyData || polyData->GetNumberOfPoints() == 0) {
		//			QMessageBox::warning(this, "Error", "Failed to read VTK file or empty data.");
		//			return;
		//		}

		//		aShape = VtkPolyDataToOCCShape(polyData);
		//		if (aShape.IsNull()) {
		//			QMessageBox::warning(this, "Error", "Failed to convert VTK to OCC shape.");
		//			return;
		//		}

		//		// 计算包围盒等（同前）
		//		Bnd_Box bbox;
		//		BRepBndLib::Add(aShape, bbox);
		//		bbox.SetGap(0.0);
		//		Standard_Real theXmin, theYmin, theZmin, theXmax, theYmax, theZmax;
		//		bbox.Get(theXmin, theYmin, theZmin, theXmax, theYmax, theZmax);

		//		info.shape = aShape;
		//		info.path = filePath;
		//		info.theXmin = theXmin; info.theYmin = theYmin; info.theZmin = theZmin;
		//		info.theXmax = theXmax; info.theYmax = theYmax; info.theZmax = theZmax;
		//		info.length = theXmax - theXmin;
		//		info.width = theYmax - theYmin;
		//		info.height = theZmax - theZmin;

		//		ModelDataManager::GetInstance()->SetModelGeometryInfo(info);
		//		importModelWid->GetGFTreeModelWidget()->updataIcon();
		//		loadSuccess = true;
		//	}
		//	else if (filePath.endsWith(".x_t", Qt::CaseInsensitive)) {
		//	XSControl_Reader aReader_XT;
		//	// 设置为 Parasolid 模式（关键！）
		//	aReader_XT.SetMode("XSTEP"); // 或尝试 "DEFAULT"

		//	IFSelect_ReturnStatus status = aReader_XT.ReadFile(filePath.toStdString().c_str());
		//	if (status == IFSelect_RetDone) {
		//		aReader_XT.PrintCheckLoad(Standard_False, IFSelect_ItemsByEntity);
		//		Standard_Integer nbRoots = aReader_XT.NbRootsForTransfer();
		//		if (nbRoots > 0) {
		//			aReader_XT.TransferRoots();
		//			aShape = aReader_XT.OneShape();

		//			if (!aShape.IsNull()) {
		//				// 计算包围盒
		//				Bnd_Box bbox;
		//				BRepBndLib::Add(aShape, bbox);
		//				bbox.SetGap(0.0);

		//				Standard_Real theXmin, theYmin, theZmin, theXmax, theYmax, theZmax;
		//				bbox.Get(theXmin, theYmin, theZmin, theXmax, theYmax, theZmax);

		//				info.shape = aShape;
		//				info.path = filePath;
		//				info.theXmin = theXmin; info.theYmin = theYmin; info.theZmin = theZmin;
		//				info.theXmax = theXmax; info.theYmax = theYmax; info.theZmax = theZmax;
		//				info.length = double(theXmax - theXmin);
		//				info.width = double(theYmax - theYmin);
		//				info.height = double(theZmax - theZmin);

		//				ModelDataManager::GetInstance()->SetModelGeometryInfo(info);
		//				importModelWid->GetGFTreeModelWidget()->updataIcon();
		//				loadSuccess = true;
		//			}
		//		}
		//	}
		//	if (!loadSuccess || aShape.IsNull()) {
		//		QMessageBox::warning(this, "Error", "Failed to load model");
		//		return;
		//	}

		//	// 获取OCC视图和上下文
		//	auto occView = importModelWid->GetOccView();
		//	Handle(AIS_InteractiveContext) context = occView->getContext();

		//	// 清除之前的显示
		//	context->EraseAll(true);

		//	// 创建模型的AIS表示
		//	Handle(AIS_Shape) modelPresentation = new AIS_Shape(aShape);

		//	// 设置模型显示属性
		//	context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
		//	context->SetColor(modelPresentation, Quantity_Color(0.0, 1.0, 1.0, Quantity_TOC_RGB), true);
		//	context->Display(modelPresentation, false);

		//			
		//	// 调整视图以适应模型
		//	occView->fitAll();


		//	currentTime = QDateTime::currentDateTime();
		//	timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
		//	QString text = timeStr + "[信息]>导入几何模型,路径为：" + filePath;
		//	textEdit->appendPlainText(text);
		}
		else if (m_TabWidget->currentIndex() == 1)
		{
			QString filter = "Image files (*.xlsx *.xlx )";
			QString filePath = QFileDialog::getOpenFileName(nullptr, QObject::tr("Open Excle"),
				QDir::currentPath(), filter);
		}

	});

	auto occView = importModelWid->GetOccView();
	connect(MoveBtn, &QPushButton::clicked, occView, &OccView::pan);
	connect(RotateBtn, &QPushButton::clicked, occView, &OccView::rotate);
	connect(ZoomBtn, &QPushButton::clicked, occView, &OccView::zoom);
	connect(FitAllBtn, &QPushButton::clicked, occView, &OccView::fitAll);
	connect(ResetBtn, &QPushButton::clicked, occView, &OccView::reset);

	QObject::connect(XBtn, &QPushButton::clicked, [occView]() {		
		auto state=occView->GetCameraRotationState();
		if (state)
		{
			Handle(V3d_View) view = occView->getView();
			view->SetProj(V3d_Xpos);
			occView->fitAll();
		}
		});
	QObject::connect(YBtn, &QPushButton::clicked, [occView]() {
		auto state = occView->GetCameraRotationState();
		if (state)
		{
			Handle(V3d_View) view = occView->getView();
			view->SetProj(V3d_Ypos);
			occView->fitAll();
		}
		});
	QObject::connect(ZBtn, &QPushButton::clicked, [occView]() {
		auto state = occView->GetCameraRotationState();
		if (state)
		{
			Handle(V3d_View) view = occView->getView();
			view->SetProj(V3d_Zpos);
			occView->fitAll();
		}
		});
	QObject::connect(_XBtn, &QPushButton::clicked, [occView]() {
		auto state = occView->GetCameraRotationState();
		if (state)
		{
			Handle(V3d_View) view = occView->getView();
			view->SetProj(V3d_Xneg);
			occView->fitAll();
		}
		});
	QObject::connect(_YBtn, &QPushButton::clicked, [occView]() {
		auto state = occView->GetCameraRotationState();
		if (state)
		{
			Handle(V3d_View) view = occView->getView();
			view->SetProj(V3d_Yneg);
			occView->fitAll();
		}
		});
	QObject::connect(_ZBtn, &QPushButton::clicked, [occView]() {
		auto state = occView->GetCameraRotationState();
		if (state)
		{
			Handle(V3d_View) view = occView->getView();
			view->SetProj(V3d_Zneg);
			occView->fitAll();
		}
		});
}

mainWidget::~mainWidget()
{
    delete ui;
}


void deleteWidget(QLayout *layout)
{
	if (layout) {
		for (int i = layout->count() - 1; i >= 0; --i) {
			QLayoutItem *item = layout->itemAt(i);
			QWidget *widget = item->widget();
			if (widget) {
				delete widget;
			}
			else {
				delete item;
			}
		}
	}
}

void mainWidget::deleteWidget(QLayout *layout)
{
	if (layout) {
		for (int i = layout->count() - 1; i >= 0; --i) {
			QLayoutItem *item = layout->itemAt(i);
			QWidget *widget = item->widget();
			if (widget) {
				delete widget;
			}
			else {
				delete item;
			}
		}
	}
}

void mainWidget::refreshMemoryUsage(QLabel *m_statusLabel) {
	// 避免重复创建定时器（防止内存泄漏和多次触发）
	if (timer) {
		timer->stop();
		delete timer;
	}

	timer = new QTimer(this);
	timer->setInterval(5000); // 5秒采样一次（合理间隔，平衡实时性和性能）
	connect(timer, &QTimer::timeout, [this, m_statusLabel]() {
		getMemoryUsage(m_statusLabel);
		});

	// 初始化首次采样的基准时间（关键：提前获取初始时间，避免首次计算异常）
	GetSystemTimes(&prevIdleTime, &prevKernelTime, &prevUserTime);
	isFirstSample = true; // 标记首次采样
	timer->start();
	getMemoryUsage(m_statusLabel); // 首次调用（此时CPU显示为0%，避免异常值）
}

void mainWidget::getMemoryUsage(QLabel *m_statusLabel) {
	QString memoryText = "0.00";
	QString cpuText = "0.00";

	// 内存
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	if (GlobalMemoryStatusEx(&statex)) {
		ULONGLONG totalPhys = statex.ullTotalPhys;
		ULONGLONG availPhys = statex.ullAvailPhys;
		double memoryUsage = ((totalPhys - availPhys) / static_cast<double>(totalPhys)) * 100.0;
		memoryText = QString::number(memoryUsage, 'f', 2);
	}
	else {
		qWarning() << "获取内存信息失败，错误码：" << GetLastError();
		memoryText = "获取失败";
	}

	// CPU
	FILETIME currIdleTime, currKernelTime, currUserTime;
	if (!GetSystemTimes(&currIdleTime, &currKernelTime, &currUserTime)) {
		qWarning() << "获取系统时间失败，错误码：" << GetLastError();
		cpuText = "获取失败";
	}
	else {
		// 首次采样：仅更新基准时间，不计算使用率（避免异常值）
		if (isFirstSample) {
			prevIdleTime = currIdleTime;
			prevKernelTime = currKernelTime;
			prevUserTime = currUserTime;
			isFirstSample = false;
			cpuText = "0.00"; // 首次显示0%
		}
		else {
			// 计算时间差（64位整数，无溢出）
			ULONGLONG idleDiff = fileTimeToULL(currIdleTime) - fileTimeToULL(prevIdleTime);
			ULONGLONG kernelDiff = fileTimeToULL(currKernelTime) - fileTimeToULL(prevKernelTime);
			ULONGLONG userDiff = fileTimeToULL(currUserTime) - fileTimeToULL(prevUserTime);

			// 总系统时间 = 内核时间 + 用户时间（所有CPU核心的总运行时间）
			ULONGLONG totalSysDiff = kernelDiff + userDiff;

			// 避免除零（极端情况，如系统无任何操作）
			if (totalSysDiff == 0) {
				cpuText = "0.00";
			}
			else {
				// 计算CPU使用率：(总时间 - 空闲时间) / 总时间 × 100%
				double cpuUsage = (1.0 - static_cast<double>(idleDiff) / totalSysDiff) * 100.0;
				// 边界限制：确保数值在0%~100%之间（避免计算误差导致的超界）
				cpuUsage = qBound(0.0, cpuUsage, 100.0);
				cpuText = QString::number(cpuUsage, 'f', 2);
			}

			// 更新基准时间（为下一次计算做准备）
			prevIdleTime = currIdleTime;
			prevKernelTime = currKernelTime;
			prevUserTime = currUserTime;
		}
	}

	// 更新QLabel显示
	m_statusLabel->setText(QString("内存使用：%1%, CPU使用：%2%").arg(memoryText).arg(cpuText));

}


// 辅助函数：FILETIME 转 64位整数（核心修复：正确合并高低位）
ULONGLONG mainWidget::fileTimeToULL(const FILETIME& ft)
{
	ULARGE_INTEGER ul;
	ul.LowPart = ft.dwLowDateTime;
	ul.HighPart = ft.dwHighDateTime;
	return ul.QuadPart; // 返回完整的64位时间戳（100纳秒为单位）
}