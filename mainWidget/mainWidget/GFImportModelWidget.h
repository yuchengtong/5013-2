#pragma once
#include <qwidget.h>
#include <QStackedWidget>
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


class GFImportModelWidget :public QWidget
{
	Q_OBJECT
public:
	GFImportModelWidget(QWidget* parent = nullptr);
	~GFImportModelWidget();


	OccView* GetOccView() { return m_OccView; }

	GFLogWidget* GetLogWidget() { return m_LogWidget; }

	GFTreeModelWidget* GetGFTreeModelWidget() { return m_treeModelWidget; }


	ProjectPropertyWidge* GetProjectPropertyWidget() { return m_projectPropertyWidge; }
	GeomPropertyWidget* GetGeomPropertyWidget() { return m_geomPropertyWidget; }
	MaterialPropertyWidget* GetMaterialPropertyWidget() { return m_materialPropertyWidget; }
	DatabasePropertyWidget* GetDatabasePropertyWidget() { return m_databasePropertyWidget; }
	CalculationPropertyWidget* GetCalculationPropertyWidget() { return m_calculationPropertyWidget; }


	PreForwardDesignPropertyWidget* GetPreForwardDesignPropertyWidget() { return m_preForwardDesignPropertyWidget; }
	PreReverseOptimizationPropertyWidget* GetPreReverseOptimizationPropertyWidget() { return m_preReverseOptimizationPropertyWidget; }
	InForwardDesignPropertyWidget* GetInForwardDesignPropertyWidget() { return m_inForwardDesignPropertyWidget; }
	InReverseOptimizationPropertyWidget* GetInReverseOptimizationPropertyWidget() { return m_inReverseOptimizationPropertyWidget; }

	//ÇúÏßÍ¼
	PreForwardTimeTempWid* GetPreForwardTimeTempWid() { return m_PreForwardTimeTempWid; }
	InForwardDensityTempWid* GetInForwardDensityTempWid() { return m_InForwardDensityTempWid; }
	InForwardDensityValveWid* GetInForwardDensityValveWid() { return m_InForwardDensityValveWid; }
	InForwardDensityVacuumWid* GetInForwardDensityVacuumWid() { return m_InForwardDensityVacuumWid; }
	InForwardTimeTempWid* GetInForwardTimeTempWid() { return m_InForwardTimeTempWid; }
	InForwardTimeValveWid* GetInForwardTimeValveWid() { return m_InForwardTimeValveWid; }
	InForwardTimeVacuumWid* GetInForwardTimeVacuumWid() { return m_InForwardTimeVacuumWid; }

private slots:
	void onTreeItemClicked(const QString& itemData);

private:
	OccView* m_OccView = nullptr;
	QFrame* m_plotFrame = nullptr;
	GFLogWidget* m_LogWidget = nullptr;
	QStackedWidget* m_PropertyStackWidget = nullptr;
	GFTreeModelWidget* m_treeModelWidget = nullptr;

	GeomPropertyWidget* m_geomPropertyWidget = nullptr;
	MaterialPropertyWidget* m_materialPropertyWidget = nullptr;
	MeshPropertyWidget* m_meshPropertyWidget = nullptr;
	ShellPropertyWidget* m_steelPropertyWidgett = nullptr;
	PropellantPropertyWidget* m_propellantPropertyWidget = nullptr;
	ProjectPropertyWidge* m_projectPropertyWidge = nullptr;
	CalculationPropertyWidget* m_calculationPropertyWidget = nullptr;
	GelatinPropertyWidget* m_insulatingheatPropertyWidget = nullptr;
	DatabasePropertyWidget* m_databasePropertyWidget = nullptr;
	PreForwardDesignPropertyWidget* m_preForwardDesignPropertyWidget = nullptr;
	PreReverseOptimizationPropertyWidget* m_preReverseOptimizationPropertyWidget = nullptr;
	InForwardDesignPropertyWidget* m_inForwardDesignPropertyWidget = nullptr;
	InReverseOptimizationPropertyWidget* m_inReverseOptimizationPropertyWidget = nullptr;


	ToolsAnimationWidget* m_ToolsAnimationWidget = nullptr;

	PreForwardTimeTempWid* m_PreForwardTimeTempWid = nullptr;
	InForwardDensityTempWid* m_InForwardDensityTempWid = nullptr;
	InForwardDensityValveWid* m_InForwardDensityValveWid = nullptr;
	InForwardDensityVacuumWid* m_InForwardDensityVacuumWid = nullptr;

	InForwardTimeTempWid* m_InForwardTimeTempWid = nullptr;
	InForwardTimeValveWid* m_InForwardTimeValveWid = nullptr;
	InForwardTimeVacuumWid* m_InForwardTimeVacuumWid = nullptr;

};

