#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QTabWidget>
#include "QCustomPlot/qcustomplot.h"

#include "GFTreeModelWidget.h"
#include "OccView.h"
#include "GFLogWidget.h"
#include "GeomPropertyWidget.h"
#include "MaterialPropertyWidget.h"
#include "MeshPropertyWidget.h"
#include "ShellPropertyWidget.h"
#include "PropellantPropertyWidget.h"
#include "ProjectPropertyWidge.h"
#include "CalculationPropertyWidget.h"
#include "GelatinPropertyWidget.h"
#include "DatabasePropertyWidget.h"

#include "PreForwardDesignPropertyWidget.h"
#include "PreReverseOptimizationPropertyWidget.h"
#include "InForwardDesignPropertyWidget.h"
#include "InReverseOptimizationPropertyWidget.h"
#include "ToolsAnimationWidget.h"
#include "PreForwardTimeTempWid.h"
#include "InForwardDensityTempWid.h"
#include "InForwardDensityVacuumWid.h"
#include "InForwardDensityValveWid.h"
#include "InForwardTimeTempWid.h"
#include "InForwardTimeVacuumWid.h"
#include "InForwardTimeValveWid.h"
#include "InForwardMassTimeWid.h"

class GFImportModelWidget : public QWidget
{
	Q_OBJECT

public:
	explicit GFImportModelWidget(QWidget* parent = nullptr);
	~GFImportModelWidget();

	OccView* GetOccView() const { return m_OccView; }
	GFLogWidget* GetLogWidget() const { return m_LogWidget; }
	GFTreeModelWidget* GetGFTreeModelWidget() const { return m_treeModelWidget; }

	ProjectPropertyWidge* GetProjectPropertyWidget() const { return m_projectPropertyWidge; }
	GeomPropertyWidget* GetGeomPropertyWidget() const { return m_geomPropertyWidget; }
	MaterialPropertyWidget* GetMaterialPropertyWidget() const { return m_materialPropertyWidget; }
	DatabasePropertyWidget* GetDatabasePropertyWidget() const { return m_databasePropertyWidget; }
	CalculationPropertyWidget* GetCalculationPropertyWidget() const { return m_calculationPropertyWidget; }

	PreForwardDesignPropertyWidget* GetPreForwardDesignPropertyWidget() const { return m_preForwardDesignPropertyWidget; }
	PreReverseOptimizationPropertyWidget* GetPreReverseOptimizationPropertyWidget() const { return m_preReverseOptimizationPropertyWidget; }
	InForwardDesignPropertyWidget* GetInForwardDesignPropertyWidget() const { return m_inForwardDesignPropertyWidget; }
	InReverseOptimizationPropertyWidget* GetInReverseOptimizationPropertyWidget() const { return m_inReverseOptimizationPropertyWidget; }

	QTabWidget* GetMainTabWidget() const {return m_pMainTabWidget;}
	ToolsAnimationWidget* GetToolsAnimationWidget() const { return m_ToolsAnimationWidget; }

	QStackedWidget* GetRightStackedWidget() const { return m_pRightStackedWidget; }

	PreForwardTimeTempWid* GetPreForwardTimeTempWid() const { return m_PreForwardTimeTempWid; }
	InForwardDensityTempWid* GetInForwardDensityTempWid() const { return m_InForwardDensityTempWid; }
	InForwardDensityValveWid* GetInForwardDensityValveWid() const { return m_InForwardDensityValveWid; }
	InForwardDensityVacuumWid* GetInForwardDensityVacuumWid() const { return m_InForwardDensityVacuumWid; }
	InForwardTimeTempWid* GetInForwardTimeTempWid() const { return m_InForwardTimeTempWid; }
	InForwardTimeValveWid* GetInForwardTimeValveWid() const { return m_InForwardTimeValveWid; }
	InForwardTimeVacuumWid* GetInForwardTimeVacuumWid() const { return m_InForwardTimeVacuumWid; }
	InForwardMassTimeWid* GetInForwardMassTimeWid()const {return m_InForwardMassTimeWid;}

private slots:
	void onTreeItemClicked(const QString& itemData);

private:
	void initLayout();
	void initPropertyWidgets();
	void initConnections();

	// 模型显示辅助函数
	void displayModelShape(OccView* occView);
	void displayMeshEdges(OccView* occView);
	bool displayPreForwardDesignResult(OccView* occView);
	bool displayInForwardDesignResult(OccView* occView);

	// 使用映射表管理属性页切换
	void setupPropertyMapping();
	QWidget* getPropertyWidget(const QString& itemData);

private:
	OccView* m_OccView = nullptr;
	QFrame* m_plotFrame = nullptr;
	GFLogWidget* m_LogWidget = nullptr;
	QStackedWidget* m_PropertyStackWidget = nullptr;
	GFTreeModelWidget* m_treeModelWidget = nullptr;
	QTabWidget* m_pMainTabWidget = nullptr;
	// 属性页映射表
	QHash<QString, QWidget*> m_propertyMap;

	GeomPropertyWidget* m_geomPropertyWidget = nullptr;
	MaterialPropertyWidget* m_materialPropertyWidget = nullptr;
	MeshPropertyWidget* m_meshPropertyWidget = nullptr;
	ShellPropertyWidget* m_shellPropertyWidget = nullptr;
	PropellantPropertyWidget* m_propellantPropertyWidget = nullptr;
	ProjectPropertyWidge* m_projectPropertyWidge = nullptr;
	CalculationPropertyWidget* m_calculationPropertyWidget = nullptr;
	GelatinPropertyWidget* m_gelatinPropertyWidget = nullptr;
	DatabasePropertyWidget* m_databasePropertyWidget = nullptr;
	PreForwardDesignPropertyWidget* m_preForwardDesignPropertyWidget = nullptr;
	PreReverseOptimizationPropertyWidget* m_preReverseOptimizationPropertyWidget = nullptr;
	InForwardDesignPropertyWidget* m_inForwardDesignPropertyWidget = nullptr;
	InReverseOptimizationPropertyWidget* m_inReverseOptimizationPropertyWidget = nullptr;

	ToolsAnimationWidget* m_ToolsAnimationWidget = nullptr;

	QStackedWidget* m_pRightStackedWidget = nullptr;
	PreForwardTimeTempWid* m_PreForwardTimeTempWid = nullptr;
	InForwardMassTimeWid* m_InForwardMassTimeWid = nullptr;

	InForwardDensityTempWid* m_InForwardDensityTempWid = nullptr;
	InForwardDensityValveWid* m_InForwardDensityValveWid = nullptr;
	InForwardDensityVacuumWid* m_InForwardDensityVacuumWid = nullptr;
	InForwardTimeTempWid* m_InForwardTimeTempWid = nullptr;
	InForwardTimeValveWid* m_InForwardTimeValveWid = nullptr;
	InForwardTimeVacuumWid* m_InForwardTimeVacuumWid = nullptr;
};