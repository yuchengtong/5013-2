#pragma execution_character_set("utf-8")
#include "GFTreeModelWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QApplication>
#include <QIcon>
#include <QFileDialog>
#include <QDateTime>
#include <QRegExp>
#include <QRegularExpression> 
#include <QValidator>
#include <QThread>
#include <algorithm>

#include <AIS_Shape.hxx>
#include <AIS_ColorScale.hxx>

#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepProj_Projection.hxx>
#include <BRepGProp.hxx>


#include <GProp_GProps.hxx>

#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>

#include <MeshVS_Mesh.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>

#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Drawer.hxx>
#include <Quantity_ColorRGBA.hxx>

#include <RWStl.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Reader.hxx>

#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopExp_Explorer.hxx>
#include <TColStd_HArray2OfInteger.hxx>
#include <TColStd_HArray2OfReal.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>


#include <V3d_View.hxx>
#include <V3d_Viewer.hxx> 




#include "GFImportModelWidget.h"
#include "TriangleStructure.h"
#include "occView.h"
#include "ModelDataManager.h"
#include "ProgressDialog.h"
#include "GeometryImportWorker.h"
#include "WordExporterWorker.h"
#include "APICreateMidSurfaceHelper.h"




#include <QScreen>
#include <QtCore/qstandardpaths.h>
#include "TriangulationWorker.h"
#include "APICalculateHepler.h"
#include "CalculateWorker.h"
#include <BRepAlgoAPI_Fuse.hxx>
#include <ShapeFix_Shape.hxx>


GFTreeModelWidget::GFTreeModelWidget(QWidget*parent)
	:QWidget(parent)
{
	m_WordExporter = new WordExporter(this);

	qRegisterMetaType<ModelGeometryInfo>("ModelGeometryInfo");
	qRegisterMetaType<ModelMeshInfo>("ModelMeshInfo");

	init();
	bindConnect();
}

GFTreeModelWidget::~GFTreeModelWidget()
{
}

void GFTreeModelWidget::init()
{
	QIcon error_icon(":/src/Error.svg");
	QIcon checked_icon(":/src/Checked.svg");

	m_TreeWidget = new GFTreeWidget(this);
	{
		m_TreeWidget->setColumnCount(1);
		m_TreeWidget->setHeaderLabels({ "项目结构" });
		m_TreeWidget->setHeaderHidden(true);
	}

	// 创建根节点
	QTreeWidgetItem* rootItem = new QTreeWidgetItem(m_TreeWidget);
	rootItem->setText(0, "工程文件");
	rootItem->setData(0, Qt::UserRole, "Project");
	rootItem->setExpanded(true);
	//rootItem->setIcon(0, icon);

	// 几何模型节点
	QTreeWidgetItem* geometryNode = new QTreeWidgetItem(rootItem);
	geometryNode->setText(0, "几何模型");
	geometryNode->setData(0, Qt::UserRole, "Geometry");
	geometryNode->setIcon(0, error_icon);

	// 数据库
	QTreeWidgetItem* dataBase = new QTreeWidgetItem(rootItem);
	{
		dataBase->setText(0, "数据库");
		dataBase->setData(0, Qt::UserRole, "DataBase");
		dataBase->setIcon(0, error_icon);
		dataBase->setExpanded(true);
	}

	QTreeWidgetItem* phyProperty = new QTreeWidgetItem();
	{
		phyProperty->setText(0, "物性数据库");
		phyProperty->setData(0, Qt::UserRole, "PhysicalProperty");
		phyProperty->setIcon(0, error_icon);
		phyProperty->setExpanded(true);
	}
	dataBase->addChild(phyProperty);

	QTreeWidgetItem* steel = new QTreeWidgetItem();
	{
		steel->setText(0, "壳体物性");
		steel->setData(0, Qt::UserRole, "Steel");
		steel->setIcon(0, error_icon);
	}
	QTreeWidgetItem* propellant = new QTreeWidgetItem();
	{
		propellant->setText(0, "药液物性");
		propellant->setData(0, Qt::UserRole, "Propellant");
		propellant->setIcon(0, error_icon);
	}
	QTreeWidgetItem* gelatin = new QTreeWidgetItem();
	{
		gelatin->setText(0, "明胶物性");
		gelatin->setData(0, Qt::UserRole, "Gelatin");
		gelatin->setIcon(0, error_icon);
	}
	phyProperty->addChild(steel);
	phyProperty->addChild(propellant);
	phyProperty->addChild(gelatin);	

	//计算模型
	QTreeWidgetItem* calculationItem = new QTreeWidgetItem(rootItem);
	{
		calculationItem->setText(0, "计算模型数据库");
		calculationItem->setData(0, Qt::UserRole, "Calculation");
		calculationItem->setIcon(0, error_icon);
	}

	//网格节点
	QTreeWidgetItem* meshItem = new QTreeWidgetItem(rootItem);
	{
		meshItem->setText(0, "网格");
		meshItem->setData(0, Qt::UserRole, "Mesh");
		meshItem->setIcon(0, error_icon);
	}

	// 预热工艺工程计算
	QTreeWidgetItem* preheatingProcessCal = new QTreeWidgetItem(rootItem);
	{
		preheatingProcessCal->setText(0, "预热工艺工程计算");
		preheatingProcessCal->setData(0, Qt::UserRole, "PreheatingProcessCal");
		preheatingProcessCal->setIcon(0, error_icon);
		preheatingProcessCal->setExpanded(true);
	}

	QTreeWidgetItem* preForwardDesign = new QTreeWidgetItem();
	{
		preForwardDesign->setText(0, "正向设计");
		preForwardDesign->setData(0, Qt::UserRole, "PreForwardDesign");
		preForwardDesign->setIcon(0, error_icon);
	}
	QTreeWidgetItem* preReverseOptimization = new QTreeWidgetItem();
	{
		preReverseOptimization->setText(0, "逆向寻优");
		preReverseOptimization->setData(0, Qt::UserRole, "PreReverseOptimization");
		preReverseOptimization->setIcon(0, error_icon);
	}
	preheatingProcessCal->addChild(preForwardDesign);
	preheatingProcessCal->addChild(preReverseOptimization);

	// 注药工艺工程计算
	QTreeWidgetItem* injectionProcessCal = new QTreeWidgetItem(rootItem);
	{
		injectionProcessCal->setText(0, "注药工艺工程计算");
		injectionProcessCal->setData(0, Qt::UserRole, "InjectionProcessCal");
		injectionProcessCal->setIcon(0, error_icon);
		injectionProcessCal->setExpanded(true);
	}

	QTreeWidgetItem* inForwardDesign = new QTreeWidgetItem();
	{
		inForwardDesign->setText(0, "正向设计");
		inForwardDesign->setData(0, Qt::UserRole, "InForwardDesign");
		inForwardDesign->setIcon(0, error_icon);
	}
	QTreeWidgetItem* inReverseOptimization = new QTreeWidgetItem();
	{
		inReverseOptimization->setText(0, "逆向寻优");
		inReverseOptimization->setData(0, Qt::UserRole, "InReverseOptimization");
		inReverseOptimization->setIcon(0, error_icon);
	}
	injectionProcessCal->addChild(inForwardDesign);
	injectionProcessCal->addChild(inReverseOptimization);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(m_TreeWidget);
	layout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(layout);
}

void GFTreeModelWidget::bindConnect()
{
	connect(m_TreeWidget, &QTreeWidget::itemClicked, this, &GFTreeModelWidget::onTreeItemClicked);
}

void GFTreeModelWidget::onTreeItemClicked(QTreeWidgetItem* item, int column)
{
	QString itemData = item->data(0, Qt::UserRole).toString();
	emit itemClicked(itemData);

	if (itemData.contains("StressResult")|| itemData.contains("StrainResult") || itemData.contains("TemperatureResult") || itemData.contains("OverpressureResult") )
	{
		QWidget* parent = parentWidget();
		while (parent) 
		{
			GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
			if (gfParent)
			{
				// 截图结果云图
				QString m_privateDirPath = "";
				if (itemData == "StressResult")
				{
					m_privateDirPath = "src/template/fall/Stress.png";
				}
				else if (itemData == "StrainResult")
				{
					m_privateDirPath = "src/template/fall/Strain.png";
				}
				else if (itemData == "TemperatureResult")
				{
					m_privateDirPath = "src/template/fall/Temperature.png";
				}
				else if (itemData == "OverpressureResult")
				{
					m_privateDirPath = "src/template/fall/Overpressure.png";
				}

				else if (itemData == "FastCombustionTemperatureResult")
				{
					m_privateDirPath = "src/template/fastCombustion/Temperature.png";
				}

				else if (itemData == "SlowCombustionTemperatureResult")
				{
					m_privateDirPath = "src/template/slowCombustion/Temperature.png";
				}

				if (itemData == "ShootStressResult")
				{
					m_privateDirPath = "src/template/shoot/Stress.png";
				}
				else if (itemData == "ShootStrainResult")
				{
					m_privateDirPath = "src/template/shoot/Strain.png";
				}
				else if (itemData == "ShootTemperatureResult")
				{
					m_privateDirPath = "src/template/shoot/Temperature.png";
				}
				else if (itemData == "ShootOverpressureResult")
				{
					m_privateDirPath = "src/template/shoot/Overpressure.png";
				}

				if (itemData == "JetImpactStressResult")
				{
					m_privateDirPath = "src/template/jetImpact/Stress.png";
				}
				else if (itemData == "JetImpactStrainResult")
				{
					m_privateDirPath = "src/template/jetImpact/Strain.png";
				}
				else if (itemData == "JetImpactTemperatureResult")
				{
					m_privateDirPath = "src/template/jetImpact/Temperature.png";
				}
				else if (itemData == "JetImpactOverpressureResult")
				{
					m_privateDirPath = "src/template/jetImpact/Overpressure.png";
				}

				if (itemData == "FragmentationImpactStressResult")
				{
					m_privateDirPath = "src/template/fragmentationImpact/Stress.png";
				}
				else if (itemData == "FragmentationImpactStrainResult")
				{
					m_privateDirPath = "src/template/fragmentationImpact/Strain.png";
				}
				else if (itemData == "FragmentationImpactTemperatureResult")
				{
					m_privateDirPath = "src/template/fragmentationImpact/Temperature.png";
				}
				else if (itemData == "FragmentationImpactOverpressureResult")
				{
					m_privateDirPath = "src/template/fragmentationImpact/Overpressure.png";
				}

				if (itemData == "ExplosiveBlastStressResult")
				{
					m_privateDirPath = "src/template/explosiveBlast/Stress.png";
				}
				else if (itemData == "ExplosiveBlastStrainResult")
				{
					m_privateDirPath = "src/template/explosiveBlast/Strain.png";
				}
				else if (itemData == "ExplosiveBlastTemperatureResult")
				{
					m_privateDirPath = "src/template/explosiveBlast/Temperature.png";
				}
				else if (itemData == "ExplosiveBlastOverpressureResult")
				{
					m_privateDirPath = "src/template/explosiveBlast/Overpressure.png";
				}

				if (itemData == "SacrificeExplosioStressResult")
				{
					m_privateDirPath = "src/template/sacrificeExplosio/Stress.png";
				}
				else if (itemData == "SacrificeExplosioStrainResult")
				{
					m_privateDirPath = "src/template/sacrificeExplosio/Strain.png";
				}
				else if (itemData == "SacrificeExplosioTemperatureResult")
				{
					m_privateDirPath = "src/template/sacrificeExplosio/Temperature.png";
				}
				else if (itemData == "SacrificeExplosioOverpressureResult")
				{
					m_privateDirPath = "src/template/sacrificeExplosio/Overpressure.png";
				}
				
				QDir privateDir(m_privateDirPath);
				m_WordExporter->captureWidgetToFile(gfParent->GetOccView(), m_privateDirPath);
				break;
			}
			else
			{
				parent = parent->parentWidget();
			}
		}
	}
}

void GFTreeModelWidget::updataIcon()
{
	QIcon error_icon(":/src/Error.svg");
	QIcon checked_icon(":/src/Checked.svg");

	auto ins=ModelDataManager::GetInstance();
	auto geomInfo = ins->GetModelGeometryInfo();
	auto meshInfo = ins->GetModelMeshInfo();

	auto steelInfo = ins->GetSteelPropertyInfo();
	auto propellantInfo = ins->GetPropellantPropertyInfo();
	auto calculationInfo = ins->GetCalculationPropertyInfo();
	auto gelatinPropertyInfo = ins->GetGelatinPropertyInfo();
	auto judgementPropertyInfo = ins->GetJudgementPropertyInfo();

	int size = m_TreeWidget->topLevelItemCount();
	QTreeWidgetItem *child;
	for (int i = 0; i < size; i++)
	{
		child = m_TreeWidget->topLevelItem(i);
		int childCount = child->childCount();
		for (int j = 0; j < childCount; ++j)
		{
			if (child->child(j)->text(0).contains("几何模型"))
			{
				if (geomInfo.path.isEmpty())
				{
					child->child(j)->setIcon(0, error_icon);
				}
				else
				{
					child->child(j)->setIcon(0, checked_icon);
				}
			}
			else if (child->child(j)->text(0).contains("网格"))
			{
				if (!meshInfo.isChecked)
				{
					child->child(j)->setIcon(0, error_icon);
				}
				else
				{
					child->child(j)->setIcon(0, checked_icon);
				}
			}
			else if (child->child(j)->text(0).contains("数据库"))
			{
				QTreeWidgetItem *clChild = child->child(j);
				int clChildCount = clChild->childCount();
				for (int m = 0; m < clChildCount; ++m) {

					if (clChild->child(m)->text(0).contains("标准数据库"))
					{
						if (!judgementPropertyInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					else if (clChild->child(m)->text(0).contains("物性数据库"))
					{
						QTreeWidgetItem *clChild_child = clChild->child(m);
						int clChildCount = clChild_child->childCount();
						for (int n = 0; n < clChildCount; ++n) {
							if (clChild_child->child(n)->text(0).contains("壳体物性"))
							{
								if (!steelInfo.isChecked)
								{
									clChild_child->child(n)->setIcon(0, error_icon);
								}
								else
								{
									clChild_child->child(n)->setIcon(0, checked_icon);
								}
							}
							if (clChild_child->child(n)->text(0).contains("药液物性"))
							{
								if (!propellantInfo.isChecked)
								{
									clChild_child->child(n)->setIcon(0, error_icon);
								}
								else
								{
									clChild_child->child(n)->setIcon(0, checked_icon);
								}
							}
							if (clChild_child->child(n)->text(0).contains("明胶物性"))
							{
								if (!gelatinPropertyInfo.isChecked)
								{
									clChild_child->child(n)->setIcon(0, error_icon);
								}
								else
								{
									clChild_child->child(n)->setIcon(0, checked_icon);
								}
							}
													}
						if (gelatinPropertyInfo.isChecked && propellantInfo.isChecked && steelInfo.isChecked)
						{
							clChild_child->setIcon(0, checked_icon);
						}
					}
				}
				if (judgementPropertyInfo.isChecked && gelatinPropertyInfo.isChecked && propellantInfo.isChecked && steelInfo.isChecked)
				{
					clChild->setIcon(0, checked_icon);
				}
			}
			else if (child->child(j)->text(0).contains("物性数据库"))
			{
				QTreeWidgetItem *clChild = child->child(j);
				int clChildCount = clChild->childCount();
				for (int m = 0; m < clChildCount; ++m) {
					if (clChild->child(m)->text(0).contains("壳体物性"))
					{
						if (!steelInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					if (clChild->child(m)->text(0).contains("药液物性"))
					{
						if (!propellantInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					if (clChild->child(m)->text(0).contains("明胶物性"))
					{
						if (!gelatinPropertyInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					if (clChild->child(m)->text(0).contains("标准数据库"))
					{
						if (!judgementPropertyInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
				}
				
			}
		}
	}
}

void GFTreeModelWidget::contextMenuEvent(QContextMenuEvent *event)
{
	QTreeWidgetItem *item = m_TreeWidget->itemAt(event->pos());
	if (!item) {
		return;
	}
	//QString type = item->data(0, Qt::UserRole).toString();
	QString text = item->text(0);
	if (text == "正向设计")
	{
		m_ContextMenu = new QMenu(this);
		QAction* calAction = new QAction("计算", this);
		QAction* exportAction = new QAction("导出报告", this);

		int childCount = item->childCount();
		QList<QTreeWidgetItem*> checkedChildItems;
		for (int i = 0; i < childCount; ++i) {
			QTreeWidgetItem* childItem = item->child(i);
			if (childItem->checkState(0) == Qt::Checked)
			{
				checkedChildItems.append(childItem);
			}
		}

		connect(calAction, &QAction::triggered, this, [item, this]() {
			QWidget* parent = parentWidget();
			while (parent)
			{
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{
					QDateTime currentTime = QDateTime::currentDateTime();
					QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
					auto logWidget = gfParent->GetLogWidget();
					auto textEdit = logWidget->GetTextEdit();

					auto occView = gfParent->GetOccView();
					Handle(AIS_InteractiveContext) context = occView->getContext();
					Handle(V3d_View) view = occView->getView();

					// 创建进度对话框
					ProgressDialog* progressDialog = new ProgressDialog("计算", gfParent);
					progressDialog->show();

					// 创建工作线程和工作对象
					auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
					CalculateWorker* worker = new CalculateWorker();
					QThread* workerThread = new QThread();
					worker->moveToThread(workerThread);

					// 连接信号槽
					connect(workerThread, &QThread::started, worker, &CalculateWorker::DoWork);
					connect(worker, &CalculateWorker::ProgressUpdated,
						progressDialog, &ProgressDialog::SetProgress);
					connect(worker, &CalculateWorker::StatusUpdated,
						progressDialog, &ProgressDialog::SetStatusText);
					connect(progressDialog, &ProgressDialog::Canceled,
						worker, &CalculateWorker::RequestInterruption,
						Qt::DirectConnection);

					// 处理导入结果
					connect(worker, &CalculateWorker::WorkFinished, this,
						[=](bool success, const QString& msg) {
							// 更新日志
							QDateTime finishTime = QDateTime::currentDateTime();
							QString finishTimeStr = finishTime.toString("yyyy-MM-dd hh:mm:ss");
							textEdit->appendPlainText(finishTimeStr + "[" + (success ? "信息" : "错误") + "]>" + msg);
							if (success)
							{
								std::vector<double> resultValue;
								resultValue.reserve(8);
								bool success = APICalculateHepler::CalculatePreForwardDesignResult(occView, resultValue);

								QDateTime currentTime = QDateTime::currentDateTime();
								QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
								if (success)
								{
									QString text = timeStr + "[信息]>跌落安全性分析计算完成";
									textEdit->appendPlainText(text);

									context->EraseAll(true);
									view->SetProj(V3d_Yneg);
									view->Redraw();

									auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
									auto oriShape = geomInfo.shape;
								}


							}
							else if (!success)
							{
								QMessageBox::warning(this, "计算", msg);
							}

							// 清理资源
							progressDialog->close();
							workerThread->quit();
							workerThread->wait();
							worker->deleteLater();
							workerThread->deleteLater();
							progressDialog->deleteLater();
						});

					// 启动线程
					workerThread->start();
					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
			});
			
	
		connect(exportAction, &QAction::triggered, [this, item]() {
			QString directory = QFileDialog::getExistingDirectory(nullptr,
				tr("选择文件夹"),
				"/home", // 默认的起始目录
				QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks); // 选项
			if (!directory.isEmpty()) {
				exportWord(directory, item); // 直接在Lambda中传递参数
			}
		});
		m_ContextMenu->addAction(calAction); // 将动作添加到菜单中
		m_ContextMenu->addAction(exportAction);
		m_ContextMenu->exec(event->globalPos()); // 在鼠标位置显示菜单
	}
	else if (text == "几何模型")
	{
		m_ContextMenu = new QMenu(this); // 创建菜单对象
		QAction *customAction = new QAction("导入", this); // 创建动作对象并添加到菜单中
		connect(customAction, &QAction::triggered, this, [item, this]() {
			QWidget* parent = parentWidget();
			while (parent) {
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{
					QString filePath = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(),
						"STEP Files (*.stp *.step);;IGES Files (*.iges *.igs);;VTK Files (*.vtk);;X_T Files (*.x_t);;All Files (*.*)");

					if (filePath.isEmpty())
					{
						return;
					}
				
					QDateTime currentTime = QDateTime::currentDateTime();
					QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
					auto logWidget = gfParent->GetLogWidget();
					auto textEdit = logWidget->GetTextEdit();
					QString text = timeStr + "[信息]>开始导入几何模型";
					textEdit->appendPlainText(text);
					logWidget->update();

					// 关键：强制刷新UI，确保日志立即显示
					QApplication::processEvents();
					

					// 创建进度对话框
					ProgressDialog* progressDialog = new ProgressDialog("固体发动机三维模型导入", gfParent);
					progressDialog->show();

					// 创建工作线程和工作对象
					GeometryImportWorker* worker = new GeometryImportWorker(filePath);
					QThread* workerThread = new QThread();
					worker->moveToThread(workerThread);

					// 连接信号槽
					connect(workerThread, &QThread::started, worker, &GeometryImportWorker::DoWork);
					connect(worker, &GeometryImportWorker::ProgressUpdated,
						progressDialog, &ProgressDialog::SetProgress);
					connect(worker, &GeometryImportWorker::StatusUpdated,
						progressDialog, &ProgressDialog::SetStatusText);
					connect(progressDialog, &ProgressDialog::Canceled,
						worker, &GeometryImportWorker::RequestInterruption,
						Qt::DirectConnection); 

					// 处理导入结果
					connect(worker, &GeometryImportWorker::WorkFinished, this,
						[=](bool success, const QString& msg, ModelGeometryInfo info) {
							// 更新日志
							QDateTime finishTime = QDateTime::currentDateTime();
							QString finishTimeStr = finishTime.toString("yyyy-MM-dd hh:mm:ss");
							textEdit->appendPlainText(finishTimeStr + "[" + (success ? "信息" : "错误") + "]>" + msg);

							if (success && !info.shape.IsNull())
							{
								double minX = DBL_MAX;
								double minY = DBL_MAX;
								double maxY = -DBL_MAX; // 新增：用于记录最大Y值，辅助判断底部边
								double maxX = -DBL_MAX;
								gp_Pnt bottomP1, bottomP2; // 修改：记录底部边线的两个端点
								bool hasBottomEdge = false; // 修改：标记是否找到底部边

								TopExp_Explorer exp(info.shape, TopAbs_EDGE);
								for (; exp.More(); exp.Next())
								{
									TopoDS_Edge edge = TopoDS::Edge(exp.Current());
									TopoDS_Vertex v1, v2;
									TopExp::Vertices(edge, v1, v2);
									gp_Pnt p1 = BRep_Tool::Pnt(v1);
									gp_Pnt p2 = BRep_Tool::Pnt(v2);

									bool vertical = (fabs(p1.X() - p2.X()) < 1e-3);
									bool horizontal = (fabs(p1.Y() - p2.Y()) < 1e-3);

									// 寻找最下侧的水平边（作为对称轴）
									if (horizontal) {
										double currentY = p1.Y();
										if (currentY < minY) {
											minY = currentY;
											bottomP1 = p1; bottomP2 = p2; // 记录底部边的两个端点
											hasBottomEdge = true;
										}
									}
									// 顺便记录最大Y值，方便后续逻辑使用
									maxY = std::max(maxY, std::max(p1.Y(), p2.Y()));

									// 寻找最左侧的垂直边（如果后续还需要用到）
									if (vertical) {
										minX = std::min(minX, p1.X());
									}
									maxX = std::max(maxX, std::max(p1.X(), p2.X()));
								}

								// 沿底部水平边对称生成完整模型
								if (hasBottomEdge)
								{
									// 构建对称轴（底部水平边）
									gp_Ax1 mirrorAxis(bottomP1, gp_Dir(bottomP2.XYZ() - bottomP1.XYZ()));

									// 设置镜像变换
									gp_Trsf mirrorTrsf;
									mirrorTrsf.SetMirror(mirrorAxis);

									// 执行镜像
									BRepBuilderAPI_Transform mirrorBRep(info.shape, mirrorTrsf, false);
									TopoDS_Shape mirroredShape = mirrorBRep.Shape();

									// 将原模型与镜像模型进行布尔并集（Fuse）融合
									BRepAlgoAPI_Fuse fuseOp(info.shape, mirroredShape);
									if (fuseOp.IsDone())
									{
										info.symmetricalShape = fuseOp.Shape();
									}
								}


								// 保存模型信息
								ModelDataManager::GetInstance()->SetModelGeometryInfo(info);
								updataIcon();

								// 更新显示
								auto occView = gfParent->GetOccView();
								Handle(AIS_InteractiveContext) context = occView->getContext();
								context->EraseAll(true);

								Handle(AIS_Shape) modelPresentation = new AIS_Shape(info.shape);
								context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
								context->SetColor(modelPresentation, Quantity_Color(0.0, 1.0, 1.0, Quantity_TOC_RGB), true);
								context->Display(modelPresentation, false);
								occView->fitAll();

								// 更新属性窗口
								auto geomProWid = gfParent->findChild<GeomPropertyWidget*>();
								geomProWid->UpdataPropertyInfo();

							}
							else if (!success)
							{
								QMessageBox::warning(this, "导入失败", msg);
							}

							// 清理资源
							progressDialog->close();
							workerThread->quit();
							if (!workerThread->wait(500)) 
							{  
								workerThread->terminate();
							}
							worker->deleteLater();
							workerThread->deleteLater();
							progressDialog->deleteLater();

							// 截图计算模型
							QString m_privateDirPath = "src/template/main.png";
							QDir privateDir(m_privateDirPath);
							m_WordExporter->captureWidgetToFile(gfParent->GetOccView(), m_privateDirPath);
						});

					// 启动线程
					workerThread->start();
					break;					
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
		});
		m_ContextMenu->addAction(customAction); // 将动作添加到菜单中
		m_ContextMenu->exec(event->globalPos()); // 在鼠标位置显示菜单
	}
	else if (text == "网格")
	{
		m_ContextMenu = new QMenu(this);
		QAction *meshAction = new QAction("网格划分", this);
		connect(meshAction, &QAction::triggered, this, [item, this]() {
			QWidget* parent = parentWidget();
			while (parent)
			{
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{								
					QDateTime currentTime = QDateTime::currentDateTime();
					QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
					auto logWidget = gfParent->GetLogWidget();
					auto textEdit = logWidget->GetTextEdit();
					QString text = timeStr + "[信息]>启动网格划分引擎，采用自适应尺寸控制算法";
					textEdit->appendPlainText(text);
					logWidget->update();
					
					auto occView = gfParent->GetOccView();
					Handle(AIS_InteractiveContext) context = occView->getContext();
					auto view = occView->getView();
					context->EraseAll(true);

					// 创建进度对话框
					ProgressDialog* progressDialog = new ProgressDialog("网格划分", gfParent);
					progressDialog->show();

					// 创建工作线程和工作对象
					auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
					TriangulationWorker* worker = new TriangulationWorker(geomInfo.symmetricalShape);
					QThread* workerThread = new QThread();
					worker->moveToThread(workerThread);

					// 连接信号槽
					connect(workerThread, &QThread::started, worker, &TriangulationWorker::DoWork);
					connect(worker, &TriangulationWorker::ProgressUpdated,
						progressDialog, &ProgressDialog::SetProgress);
					connect(worker, &TriangulationWorker::StatusUpdated,
						progressDialog, &ProgressDialog::SetStatusText);
					connect(progressDialog, &ProgressDialog::Canceled,
						worker, &TriangulationWorker::RequestInterruption,
						Qt::DirectConnection);

					// 处理导入结果
					connect(worker, &TriangulationWorker::WorkFinished, this,
						[=](bool success, const QString& msg, const ModelMeshInfo& info) {
							// 更新日志
							QDateTime finishTime = QDateTime::currentDateTime();
							QString finishTimeStr = finishTime.toString("yyyy-MM-dd hh:mm:ss");
							textEdit->appendPlainText(finishTimeStr + "[" + (success ? "信息" : "错误") + "]>" + msg);
							if (success)
							{
								ModelDataManager::GetInstance()->SetModelMeshInfo(info);
								BRep_Builder builder;
								TopoDS_Compound compound;
								builder.MakeCompound(compound);

								auto aDataSource = info.triangleStructure;
								auto myEdges = aDataSource.GetMyEdge();
								auto myNodeCoords = aDataSource.GetmyNodeCoords();
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
								context->Display(aisCompound, Standard_True);

								updataIcon();

								QDateTime currentTime = QDateTime::currentDateTime();
								QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
								QString text = timeStr + "[信息]>网格划分完成";
								textEdit->appendPlainText(text);

								auto meshProWid = gfParent->findChild<MeshPropertyWidget*>();
								meshProWid->UpdataPropertyInfo();
							}
							else if (!success)
							{
								QMessageBox::warning(this, "导入失败", msg);
							}

							// 清理资源
							progressDialog->close();
							workerThread->quit();
							workerThread->wait();
							worker->deleteLater();
							workerThread->deleteLater();
							progressDialog->deleteLater();
						});

					// 启动线程
					workerThread->start();

					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
		});
		m_ContextMenu->addAction(meshAction);
		m_ContextMenu->exec(event->globalPos());
	}
}

void GFTreeModelWidget::exportWord(const QString& directory, QTreeWidgetItem* item)
{
	
}


