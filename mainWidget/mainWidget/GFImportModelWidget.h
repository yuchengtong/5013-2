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
#include "TimeTempPlotWidget.h"

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

	TimeTempPlotWidget* m_TimeTempPlotWidget = nullptr;
};

