#include "GFImportModelWidget.h"

#include <AIS_Shape.hxx>
#include <AIS_ColorScale.hxx>
#include <STEPControl_Reader.hxx>
#include <Prs3d_LineAspect.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Edge.hxx>
#include <StlAPI_Reader.hxx>
#include <RWStl.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <Quantity_NameOfColor.hxx>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>

#include <QSplitter>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QGridLayout>

#include <V3d_View.hxx>
#include <V3d_TypeOfOrientation.hxx>

#include "OccView.h"
#include "GFLogWidget.h"
#include "GFTreeModelWidget.h"
#include "colour_change_algrithm.h"
#include "APISetNodeValue.h"
#include "BaseCurvePlotWidget.h"
#include "ModelDataManager.h"

GFImportModelWidget::GFImportModelWidget(QWidget* parent)
	: QWidget(parent)
{
	initPropertyWidgets();
	initLayout();
	initConnections();
	setupPropertyMapping();
}

GFImportModelWidget::~GFImportModelWidget()
{
}

// ============================================================
// 初始化属性控件
// ============================================================
void GFImportModelWidget::initPropertyWidgets()
{
	m_treeModelWidget = new GFTreeModelWidget(this);
	m_PropertyStackWidget = new QStackedWidget(this);

	// 初始化所有 PropertyWidget
	m_geomPropertyWidget = new GeomPropertyWidget(this);
	m_materialPropertyWidget = new MaterialPropertyWidget(this);
	m_meshPropertyWidget = new MeshPropertyWidget(this);
	m_shellPropertyWidget = new ShellPropertyWidget(this);
	m_propellantPropertyWidget = new PropellantPropertyWidget(this);
	m_projectPropertyWidge = new ProjectPropertyWidge(this);
	m_calculationPropertyWidget = new CalculationPropertyWidget(this);
	m_gelatinPropertyWidget = new GelatinPropertyWidget(this);
	m_preForwardDesignPropertyWidget = new PreForwardDesignPropertyWidget(this);
	m_preReverseOptimizationPropertyWidget = new PreReverseOptimizationPropertyWidget(this);
	m_inForwardDesignPropertyWidget = new InForwardDesignPropertyWidget(this);
	m_inReverseOptimizationPropertyWidget = new InReverseOptimizationPropertyWidget(this);
	m_databasePropertyWidget = new DatabasePropertyWidget(this);

	// 添加到 QStackedWidget
	m_PropertyStackWidget->addWidget(m_geomPropertyWidget);
	m_PropertyStackWidget->addWidget(m_materialPropertyWidget);
	m_PropertyStackWidget->addWidget(m_meshPropertyWidget);
	m_PropertyStackWidget->addWidget(m_shellPropertyWidget);
	m_PropertyStackWidget->addWidget(m_propellantPropertyWidget);
	m_PropertyStackWidget->addWidget(m_projectPropertyWidge);
	m_PropertyStackWidget->addWidget(m_calculationPropertyWidget);
	m_PropertyStackWidget->addWidget(m_gelatinPropertyWidget);
	m_PropertyStackWidget->addWidget(m_preForwardDesignPropertyWidget);
	m_PropertyStackWidget->addWidget(m_preReverseOptimizationPropertyWidget);
	m_PropertyStackWidget->addWidget(m_inForwardDesignPropertyWidget);
	m_PropertyStackWidget->addWidget(m_inReverseOptimizationPropertyWidget);
	m_PropertyStackWidget->addWidget(m_databasePropertyWidget);
}

// ============================================================
// 设置属性页映射表
// ============================================================
void GFImportModelWidget::setupPropertyMapping()
{
	m_propertyMap.insert("Geometry", m_geomPropertyWidget);
	m_propertyMap.insert("PhysicalProperty", m_materialPropertyWidget);
	m_propertyMap.insert("Steel", m_shellPropertyWidget);
	m_propertyMap.insert("Propellant", m_propellantPropertyWidget);
	m_propertyMap.insert("ComputationalModel", m_calculationPropertyWidget);
	m_propertyMap.insert("Project", m_projectPropertyWidge);
	m_propertyMap.insert("Gelatin", m_gelatinPropertyWidget);
	m_propertyMap.insert("DataBase", m_databasePropertyWidget);
	m_propertyMap.insert("Calculation", m_calculationPropertyWidget);
	m_propertyMap.insert("Mesh", m_meshPropertyWidget);
	m_propertyMap.insert("PreForwardDesign", m_preForwardDesignPropertyWidget);
	m_propertyMap.insert("PreReverseOptimization", m_preReverseOptimizationPropertyWidget);
	m_propertyMap.insert("InForwardDesign", m_inForwardDesignPropertyWidget);
	m_propertyMap.insert("InReverseOptimization", m_inReverseOptimizationPropertyWidget);
}

// ============================================================
// 根据 itemData 获取对应的属性页
// ============================================================
QWidget* GFImportModelWidget::getPropertyWidget(const QString& itemData)
{
	return m_propertyMap.value(itemData, nullptr);
}

// ============================================================
// 初始化布局
// ============================================================
void GFImportModelWidget::initLayout()
{
	// ========== 左侧分割器（树 + 属性表） ==========
	auto leftSplitter = new QSplitter(Qt::Vertical, this);
	leftSplitter->setMinimumWidth(360);
	leftSplitter->setHandleWidth(2);
	leftSplitter->setContentsMargins(0, 0, 0, 0);
	leftSplitter->setStyleSheet("QSplitter::handle { background: #cccccc; }");
	leftSplitter->addWidget(m_treeModelWidget);
	leftSplitter->addWidget(m_PropertyStackWidget);
	leftSplitter->setStretchFactor(0, 2);
	leftSplitter->setStretchFactor(1, 1);

	// ========== TabWidget ==========
	m_pMainTabWidget = new QTabWidget();
	m_pMainTabWidget->setDocumentMode(true);

	// ----- 主场景 -----
	auto mainSceneWidget = new QWidget();
	auto vLayout = new QVBoxLayout();
	vLayout->setContentsMargins(0, 0, 0, 0);
	vLayout->setSpacing(2);

	m_OccView = new OccView(this);
	m_OccView->setMinimumSize(320, 320);
	m_OccView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_ToolsAnimationWidget = new ToolsAnimationWidget(this);
	m_ToolsAnimationWidget->setMaximumHeight(100);

	vLayout->addWidget(m_OccView, 1);
	vLayout->addWidget(m_ToolsAnimationWidget);
	mainSceneWidget->setLayout(vLayout);
	m_pMainTabWidget->addTab(mainSceneWidget, QString::fromLocal8Bit("主场景"));

	// ----- 预热工艺（单图表，用容器包裹） -----
	auto preForwardWidget = new QWidget(this);
	auto preForwardLayout = new QVBoxLayout(preForwardWidget);
	preForwardLayout->setContentsMargins(10, 10, 10, 10);
	preForwardLayout->setSpacing(0);

	m_PreForwardTimeTempWid = new PreForwardTimeTempWid(this);
	m_PreForwardTimeTempWid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	preForwardLayout->addWidget(m_PreForwardTimeTempWid);

	m_pMainTabWidget->addTab(preForwardWidget, QString::fromLocal8Bit("预热工艺"));

	// ----- 注药工艺（6 图表网格） -----
	auto inForwardWidget = new QWidget(this);
	auto inForwardGridLayout = new QGridLayout(inForwardWidget);
	inForwardGridLayout->setSpacing(10);           // 增大间距
	inForwardGridLayout->setContentsMargins(10, 10, 10, 10); // 增大边距

	m_InForwardDensityTempWid = new InForwardDensityTempWid(this);
	m_InForwardDensityValveWid = new InForwardDensityValveWid(this);
	m_InForwardDensityVacuumWid = new InForwardDensityVacuumWid(this);
	m_InForwardTimeTempWid = new InForwardTimeTempWid(this);
	m_InForwardTimeValveWid = new InForwardTimeValveWid(this);
	m_InForwardTimeVacuumWid = new InForwardTimeVacuumWid(this);

	// 左列：密度相关
	inForwardGridLayout->addWidget(m_InForwardDensityTempWid, 0, 0);
	inForwardGridLayout->addWidget(m_InForwardDensityValveWid, 1, 0);
	inForwardGridLayout->addWidget(m_InForwardDensityVacuumWid, 2, 0);

	// 右列：时间相关
	inForwardGridLayout->addWidget(m_InForwardTimeTempWid, 0, 1);
	inForwardGridLayout->addWidget(m_InForwardTimeValveWid, 1, 1);
	inForwardGridLayout->addWidget(m_InForwardTimeVacuumWid, 2, 1);

	// 列等宽
	inForwardGridLayout->setColumnStretch(0, 1);
	inForwardGridLayout->setColumnStretch(1, 1);
	// 行等高（关键！让三行均分垂直空间）
	inForwardGridLayout->setRowStretch(0, 1);
	inForwardGridLayout->setRowStretch(1, 1);
	inForwardGridLayout->setRowStretch(2, 1);

	m_pMainTabWidget->addTab(inForwardWidget, QString::fromLocal8Bit("注药工艺"));

	// ========== 日志窗口 ==========
	m_LogWidget = new GFLogWidget(this);
	m_LogWidget->setMinimumHeight(100);
	m_LogWidget->setMaximumHeight(250);

	// 右侧垂直分割器（TabWidget + 日志）
	auto rightSplitter = new QSplitter(Qt::Vertical, this);
	rightSplitter->setHandleWidth(3);
	rightSplitter->setStyleSheet("QSplitter::handle { background: #aaaaaa; }");
	rightSplitter->setContentsMargins(0, 0, 0, 0);
	rightSplitter->addWidget(m_pMainTabWidget);
	rightSplitter->addWidget(m_LogWidget);
	rightSplitter->setStretchFactor(0, 5);
	rightSplitter->setStretchFactor(1, 2);
	rightSplitter->setCollapsible(0, false);
	rightSplitter->setCollapsible(1, false);  // 日志区不允许折叠到 0

	// ========== 主水平分割器（左侧 + 右侧） ==========
	auto mainSplitter = new QSplitter(Qt::Horizontal, this);
	mainSplitter->setHandleWidth(3);
	mainSplitter->setStyleSheet("QSplitter::handle { background: #aaaaaa; }");
	mainSplitter->setContentsMargins(0, 0, 0, 0);
	mainSplitter->addWidget(leftSplitter);
	mainSplitter->addWidget(rightSplitter);
	mainSplitter->setStretchFactor(0, 1);
	mainSplitter->setStretchFactor(1, 3);
	mainSplitter->setCollapsible(0, false);
	mainSplitter->setCollapsible(1, false);

	auto mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(mainSplitter);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(mainLayout);
}

// ============================================================
// 初始化信号连接
// ============================================================
void GFImportModelWidget::initConnections()
{
	connect(m_treeModelWidget, &GFTreeModelWidget::itemClicked,
		this, &GFImportModelWidget::onTreeItemClicked);
}

// ============================================================
// 显示模型形状（提取的公共函数）
// ============================================================
void GFImportModelWidget::displayModelShape(OccView* occView)
{
	if (!occView) return;

	auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	if (modelInfo.shape.IsNull()) return;

	Handle(AIS_InteractiveContext) context = occView->getContext();
	if (context.IsNull()) return;

	context->EraseAll(true);
	Handle(AIS_Shape) modelPresentation = new AIS_Shape(modelInfo.shape);
	context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
	context->SetColor(modelPresentation, Quantity_Color(0.0, 1.0, 1.0, Quantity_TOC_RGB), true);
	context->Display(modelPresentation, false);
	occView->fitAll();
}

// ============================================================
// 显示网格
// ============================================================
void GFImportModelWidget::displayMeshEdges(OccView* occView)
{
	if (!occView) return;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	if (!meshInfo.isChecked) return;

	Handle(AIS_InteractiveContext) context = occView->getContext();
	if (context.IsNull()) return;

	BRep_Builder builder;
	TopoDS_Compound compound;
	builder.MakeCompound(compound);

	auto tri = meshInfo.triangleStructure;
	auto myEdges = tri.GetMyEdge();
	auto myNodeCoords = tri.GetmyNodeCoords();

	for (const auto& edge : myEdges)
	{
		Standard_Integer node1ID = edge.first;
		Standard_Integer node2ID = edge.second;

		Standard_Real x1 = myNodeCoords->Value(node1ID, 1);
		Standard_Real y1 = myNodeCoords->Value(node1ID, 2);
		Standard_Real z1 = myNodeCoords->Value(node1ID, 3);

		Standard_Real x2 = myNodeCoords->Value(node2ID, 1);
		Standard_Real y2 = myNodeCoords->Value(node2ID, 2);
		Standard_Real z2 = myNodeCoords->Value(node2ID, 3);

		gp_Pnt p1(x1, y1, z1);
		gp_Pnt p2(x2, y2, z2);

		TopoDS_Vertex v1 = BRepBuilderAPI_MakeVertex(p1);
		TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(p2);
		TopoDS_Edge edgeShape = BRepBuilderAPI_MakeEdge(v1, v2);

		builder.Add(compound, edgeShape);
	}

	Handle(AIS_Shape) aisCompound = new AIS_Shape(compound);
	context->EraseAll(true);
	context->Display(aisCompound, Standard_True);
}

// ============================================================
// 显示正向设计结果
// ============================================================
void GFImportModelWidget::displayInForwardDesignResult(OccView* occView)
{
	if (!occView) return;

	auto inForwardInfo = ModelDataManager::GetInstance()->GetInForwardPropertyInfo();
	if (!inForwardInfo.isChecked) return;

	auto toolsAnimationWidget = GetToolsAnimationWidget();
	if (!toolsAnimationWidget) return;

	std::vector<double> nodeValues;
	APISetNodeValue::SetPreForwardDesignResult(occView, nodeValues, toolsAnimationWidget->GetCurrentFrameIndex());

	Handle(AIS_InteractiveContext) context = occView->getContext();
	if (context.IsNull()) return;

	if (!inForwardInfo.m_ColorScale.IsNull())
	{
		Graphic3d_Vec2i offset(0, Standard_Integer(200));
		context->SetTransformPersistence(inForwardInfo.m_ColorScale,
			new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, offset));
		context->SetDisplayMode(inForwardInfo.m_ColorScale, 1, Standard_False);
		context->Display(inForwardInfo.m_ColorScale, Standard_True);
	}
}

// ============================================================
// 树节点点击处理（使用映射表 + 策略模式）
// ============================================================
void GFImportModelWidget::onTreeItemClicked(const QString& itemData)
{
	auto occView = GetOccView();
	if (!occView) 
	{
		return;
	}

	// 获取对应的属性页
	QWidget* propertyWidget = getPropertyWidget(itemData);
	if (!propertyWidget) 
	{
		return;
	}

	// 切换到对应的属性页
	m_PropertyStackWidget->setCurrentWidget(propertyWidget);

	// 根据节点类型执行特定的显示逻辑
	if (itemData == "Geometry" ||
		itemData == "PreForwardDesign" ||
		itemData == "PreReverseOptimization" ||
		itemData == "InReverseOptimization")
	{
		displayModelShape(occView);
	}
	else if (itemData == "Mesh")
	{
		displayMeshEdges(occView);
		m_meshPropertyWidget->UpdataPropertyInfo();
	}
	else if (itemData == "InForwardDesign")
	{
		displayInForwardDesignResult(occView);
	}

	if (itemData == "Geometry")
	{
		m_geomPropertyWidget->UpdataPropertyInfo();
	}
}