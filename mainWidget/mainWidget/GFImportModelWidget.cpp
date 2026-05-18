#pragma execution_character_set("utf-8")
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
#include <QSplitter>

#include <V3d_View.hxx>
#include <V3d_TypeOfOrientation.hxx>


#include "OccView.h"
#include "GFLogWidget.h"
#include "GFTreeModelWidget.h"
#include "colour_change_algrithm.h"
#include "APISetNodeValue.h"
#include "BaseCurvePlotWidget.h"


GFImportModelWidget::GFImportModelWidget(QWidget*parent)
	:QWidget(parent)
{
	m_treeModelWidget = new GFTreeModelWidget();
	m_PropertyStackWidget = new QStackedWidget();
	// 设置m_PropertyStackWidget的背景为白色
	m_PropertyStackWidget->setStyleSheet("background-color: white;");

	// 初始化所有的 PropertyWidget
	m_geomPropertyWidget = new GeomPropertyWidget();
	m_materialPropertyWidget = new MaterialPropertyWidget();
	m_meshPropertyWidget = new MeshPropertyWidget();
	m_steelPropertyWidgett = new ShellPropertyWidget();
	m_propellantPropertyWidget = new PropellantPropertyWidget();
	m_projectPropertyWidge = new ProjectPropertyWidge();
	m_calculationPropertyWidget = new CalculationPropertyWidget();
	m_insulatingheatPropertyWidget = new GelatinPropertyWidget();
	m_preForwardDesignPropertyWidget = new PreForwardDesignPropertyWidget();
	m_preReverseOptimizationPropertyWidget = new PreReverseOptimizationPropertyWidget();
	m_inForwardDesignPropertyWidget = new InForwardDesignPropertyWidget();
	m_inReverseOptimizationPropertyWidget = new InReverseOptimizationPropertyWidget();
	m_databasePropertyWidget = new DatabasePropertyWidget();
	
	// 将所有的 PropertyWidget 添加到 QStackedWidget 中
	m_PropertyStackWidget->addWidget(m_geomPropertyWidget);
	m_PropertyStackWidget->addWidget(m_materialPropertyWidget);
	m_PropertyStackWidget->addWidget(m_meshPropertyWidget);
	m_PropertyStackWidget->addWidget(m_steelPropertyWidgett);
	m_PropertyStackWidget->addWidget(m_propellantPropertyWidget);
	m_PropertyStackWidget->addWidget(m_projectPropertyWidge);
	m_PropertyStackWidget->addWidget(m_calculationPropertyWidget);
	m_PropertyStackWidget->addWidget(m_insulatingheatPropertyWidget);
	m_PropertyStackWidget->addWidget(m_preForwardDesignPropertyWidget);
	m_PropertyStackWidget->addWidget(m_preReverseOptimizationPropertyWidget);
	m_PropertyStackWidget->addWidget(m_inForwardDesignPropertyWidget);
	m_PropertyStackWidget->addWidget(m_inReverseOptimizationPropertyWidget);
	m_PropertyStackWidget->addWidget(m_databasePropertyWidget);


	QVBoxLayout* layout = new QVBoxLayout();
	{
		auto mainSplitter = new QSplitter(Qt::Horizontal);
		{
			// ------ 左侧垂直分割器（树结构与属性表） ------
			auto leftSplitter = new QSplitter(Qt::Vertical);
			leftSplitter->setMinimumWidth(360);
			{
				leftSplitter->addWidget(m_treeModelWidget);
				leftSplitter->addWidget(m_PropertyStackWidget);
				leftSplitter->setStretchFactor(0, 2);
				leftSplitter->setStretchFactor(1, 1);
				leftSplitter->setContentsMargins(0, 0, 0, 0);
				// 设置分割器的Handle宽度为0（消除视觉间隙）
				leftSplitter->setHandleWidth(1);
			}

			auto preForwardWidget = new QWidget();
			auto mainLayout1 = new QVBoxLayout();
			mainLayout1->setSpacing(0); 
			mainLayout1->setContentsMargins(0, 0, 0, 0);
			{
				m_OccView = new OccView(this);
				m_OccView->setMinimumSize(300, 345);

				m_ToolsAnimationWidget = new ToolsAnimationWidget();

				m_PreForwardTimeTempWid = new PreForwardTimeTempWid();
				m_PreForwardTimeTempWid->setMinimumHeight(200);

				m_LogWidget = new GFLogWidget();
				m_LogWidget->setMinimumHeight(200);

				mainLayout1->addWidget(m_OccView, 0);
				mainLayout1->addWidget(m_ToolsAnimationWidget, 0);
				mainLayout1->addWidget(m_PreForwardTimeTempWid, 0);
				mainLayout1->addWidget(m_LogWidget, 0); 

				preForwardWidget->setLayout(mainLayout1);
			}
	
			auto InForwardWidget1 = new QWidget();
			auto mainLayout2 = new QVBoxLayout();
			mainLayout2->setSpacing(0);
			mainLayout2->setContentsMargins(0, 0, 0, 0);
			{
				m_InForwardDensityTempWid = new InForwardDensityTempWid();
				m_InForwardDensityValveWid = new InForwardDensityValveWid();
				m_InForwardDensityVacuumWid = new InForwardDensityVacuumWid();

				mainLayout2->addWidget(m_InForwardDensityTempWid);
				mainLayout2->addWidget(m_InForwardDensityValveWid);
				mainLayout2->addWidget(m_InForwardDensityVacuumWid);

				InForwardWidget1->setLayout(mainLayout2);
			}

			auto InForwardWidget2 = new QWidget();
			auto mainLayout3 = new QVBoxLayout();
			mainLayout3->setSpacing(0);
			mainLayout3->setContentsMargins(0, 0, 0, 0);
			{
				m_InForwardTimeTempWid = new InForwardTimeTempWid();
				m_InForwardTimeValveWid = new InForwardTimeValveWid();
				m_InForwardTimeVacuumWid = new InForwardTimeVacuumWid();

				mainLayout3->addWidget(m_InForwardTimeTempWid);
				mainLayout3->addWidget(m_InForwardTimeValveWid);
				mainLayout3->addWidget(m_InForwardTimeVacuumWid);

				InForwardWidget2->setLayout(mainLayout3);
			}
	
			// ------ 主水平分割器（左侧与右侧） ------
			mainSplitter->addWidget(leftSplitter);
			mainSplitter->addWidget(preForwardWidget);
			mainSplitter->addWidget(InForwardWidget1);
			mainSplitter->addWidget(InForwardWidget2);

			mainSplitter->setContentsMargins(0, 0, 0, 0);
			mainSplitter->setHandleWidth(2);

			mainSplitter->setCollapsible(0, false);
			mainSplitter->setCollapsible(1, false);
			mainSplitter->setCollapsible(2, false); 
			mainSplitter->setCollapsible(3, false);

			mainSplitter->setSizes(QList<int>() << 271 << 543 << 543 << 543);
			mainSplitter->setStretchFactor(0, 1);
			mainSplitter->setStretchFactor(1, 2);
			mainSplitter->setStretchFactor(2, 2);
			mainSplitter->setStretchFactor(3, 2);
		}
		layout->addWidget(mainSplitter);
		layout->setContentsMargins(0, 0, 0, 0);
	}

	setLayout(layout);

	

	// 连接信号和槽
	connect(m_treeModelWidget, &GFTreeModelWidget::itemClicked, this, &GFImportModelWidget::onTreeItemClicked);
}

GFImportModelWidget::~GFImportModelWidget()
{
}


void GFImportModelWidget::onTreeItemClicked(const QString& itemData)
{
	auto occView = GetOccView();
	if (itemData == "Geometry")
	{
		occView->SetCameraRotationState(true);

		m_PropertyStackWidget->setCurrentWidget(m_geomPropertyWidget);
		auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
		if (!modelInfo.shape.IsNull())
		{
			Handle(AIS_InteractiveContext) context = occView->getContext();
			context->EraseAll(true);
			Handle(AIS_Shape) modelPresentation = new AIS_Shape(modelInfo.shape);
			context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
			context->SetColor(modelPresentation, Quantity_Color(0.0, 1.0, 1.0, Quantity_TOC_RGB), true);
			context->Display(modelPresentation, false);
			occView->fitAll();
		}
		m_geomPropertyWidget->UpdataPropertyInfo();
	}
	else if (itemData == "PhysicalProperty")
	{
		occView->SetCameraRotationState(true);

		m_PropertyStackWidget->setCurrentWidget(m_materialPropertyWidget);
	}
	else if (itemData == "Steel")
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_steelPropertyWidgett);
	}
	else if (itemData == "Propellant")
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_propellantPropertyWidget);
	}
	else if (itemData == "ComputationalModel")
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_calculationPropertyWidget);
	}
	else if (itemData == "Project")
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_projectPropertyWidge);
	}
	else if (itemData == "Gelatin")
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_insulatingheatPropertyWidget);
	}
	else if (itemData == "DataBase")
	{
		occView->SetCameraRotationState(true);

		m_PropertyStackWidget->setCurrentWidget(m_databasePropertyWidget);
	}
	else if (itemData == "Calculation")
	{
		occView->SetCameraRotationState(true);

		m_PropertyStackWidget->setCurrentWidget(m_calculationPropertyWidget);
	}
	else if (itemData == "Mesh")
	{
		occView->SetCameraRotationState(true);

		m_PropertyStackWidget->setCurrentWidget(m_meshPropertyWidget);

		auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
		if (meshInfo.isChecked)
		{
			Handle(AIS_InteractiveContext) context = occView->getContext();

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

		m_meshPropertyWidget->UpdataPropertyInfo();
	}
	else if (itemData == "PreForwardDesign")
	{
		occView->SetCameraRotationState(true);

		auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
		if (!modelInfo.shape.IsNull())
		{
			Handle(AIS_InteractiveContext) context = occView->getContext();
			context->EraseAll(true);
			Handle(AIS_Shape) modelPresentation = new AIS_Shape(modelInfo.shape);
			context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
			context->SetColor(modelPresentation, Quantity_Color(0.0, 1.0, 1.0, Quantity_TOC_RGB), true);
			context->Display(modelPresentation, false);
			occView->fitAll();
		}

		m_PropertyStackWidget->setCurrentWidget(m_preForwardDesignPropertyWidget);

	}
	else if (itemData == "PreReverseOptimization")
	{
		occView->SetCameraRotationState(true);

		auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
		if (!modelInfo.shape.IsNull())
		{
			Handle(AIS_InteractiveContext) context = occView->getContext();
			context->EraseAll(true);
			Handle(AIS_Shape) modelPresentation = new AIS_Shape(modelInfo.shape);
			context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
			context->SetColor(modelPresentation, Quantity_Color(0.0, 1.0, 1.0, Quantity_TOC_RGB), true);
			context->Display(modelPresentation, false);
			occView->fitAll();
		}

		m_PropertyStackWidget->setCurrentWidget(m_preReverseOptimizationPropertyWidget);
	}
	else if (itemData == "InForwardDesign")
	{
		occView->SetCameraRotationState(true);
		m_PropertyStackWidget->setCurrentWidget(m_inForwardDesignPropertyWidget);
	}
	else if (itemData == "InReverseOptimization")
	{
		occView->SetCameraRotationState(true);

		auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
		if (!modelInfo.shape.IsNull())
		{
			Handle(AIS_InteractiveContext) context = occView->getContext();
			context->EraseAll(true);
			Handle(AIS_Shape) modelPresentation = new AIS_Shape(modelInfo.shape);
			context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
			context->SetColor(modelPresentation, Quantity_Color(0.0, 1.0, 1.0, Quantity_TOC_RGB), true);
			context->Display(modelPresentation, false);
			occView->fitAll();
		}

		m_PropertyStackWidget->setCurrentWidget(m_inReverseOptimizationPropertyWidget);
	}

}






