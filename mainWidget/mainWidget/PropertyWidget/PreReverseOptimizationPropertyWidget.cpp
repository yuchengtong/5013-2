#pragma once
#pragma execution_character_set("utf-8")
#include "PreReverseOptimizationPropertyWidget.h"
#include "../ProgressDialog.h"
#include "../ReverseOptimizationWorker.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QDir>
#include <QPushButton>
#include <QDialog>
#include "ModelDataManager.h"
#include "../GFTreeModelWidget.h"
#include "../GFImportModelWidget.h"
#include "xlsxdocument.h"


#include <QDateTime>
#include <QApplication>

PreReverseOptimizationPropertyWidget::PreReverseOptimizationPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void PreReverseOptimizationPropertyWidget::initWidget()
{

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);
	{
		m_tableWidget = new QTableWidget(this);

		m_tableWidget->setRowCount(11);
		m_tableWidget->setColumnCount(5);
		// 隐藏表头（如果不需要显示表头文字，可根据需求决定是否隐藏）
		m_tableWidget->horizontalHeader()->setVisible(false);
		m_tableWidget->verticalHeader()->setVisible(false);

		// 设置第一列固定宽度（例如100像素）
		m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
		m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
		m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
		m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
		m_tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
		m_tableWidget->horizontalHeader()->resizeSection(0, 5);
		m_tableWidget->horizontalHeader()->resizeSection(1, 120);
		m_tableWidget->horizontalHeader()->resizeSection(3, 80);
		m_tableWidget->horizontalHeader()->resizeSection(4, 25);

		QStringList cols = { "逆向寻优"," ",  "1", "2", "3","4","5","6"," ","1","2" };
		for (int row = 0; row < cols.size(); ++row) {
			QTableWidgetItem* serialItem = new QTableWidgetItem(cols[row]);
			serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 0, serialItem);
		}

		QStringList labels = { "逆向寻优","工艺设计参数",  "弹体目标温度", "烘箱环境温度", "弹体初始温度","环境对流传热系数","壳体辐射吸收系数","环境发射率","工艺目标参数","弹体预热时间","弹体温度云图与温升曲线" };
		for (int row = 0; row < labels.size(); ++row) {
			QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
			labelItem->setTextAlignment(Qt::AlignCenter); // 文本居中
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 1, labelItem);
		}

		// 设置列宽度
		QTableWidgetItem* colimnItem = m_tableWidget->item(10, 1);
		int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
		m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());

		// 计算按钮
		QPushButton* m_calButton = new QPushButton("计算");
		QPushButton* m_resetButton = new QPushButton("默认");
		m_tableWidget->setCellWidget(0, 2, m_calButton);
		m_tableWidget->setCellWidget(1, 2, m_resetButton);
		connect(m_calButton, &QPushButton::clicked, this, &PreReverseOptimizationPropertyWidget::calculate);
		connect(m_resetButton, &QPushButton::clicked, this, &PreReverseOptimizationPropertyWidget::reset);


		QTableWidgetItem* targetTemperatureValueItem = new QTableWidgetItem(m_targetTemperatureValue);
		targetTemperatureValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		targetTemperatureValueItem->setBackground(QBrush(QColor(2, 253, 254)));

		QTableWidgetItem* environmentalTemperatureValueItem = new QTableWidgetItem(m_environmentalTemperatureValue);
		environmentalTemperatureValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		environmentalTemperatureValueItem->setFlags(environmentalTemperatureValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		environmentalTemperatureValueItem->setBackground(QBrush(QColor(230, 230, 230)));

		QTableWidgetItem* initialTemperatureValueItem = new QTableWidgetItem(m_initialTemperatureValue);
		initialTemperatureValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		initialTemperatureValueItem->setFlags(initialTemperatureValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		initialTemperatureValueItem->setBackground(QBrush(QColor(230, 230, 230)));

		QTableWidgetItem* heatTransferCoefficientValueItem = new QTableWidgetItem(m_heatTransferCoefficientValue);
		heatTransferCoefficientValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		heatTransferCoefficientValueItem->setFlags(heatTransferCoefficientValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		heatTransferCoefficientValueItem->setBackground(QBrush(QColor(230, 230, 230)));

		QTableWidgetItem* absorptionCoefficientValueItem = new QTableWidgetItem(m_absorptionCoefficientValue);
		absorptionCoefficientValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		absorptionCoefficientValueItem->setFlags(absorptionCoefficientValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		absorptionCoefficientValueItem->setBackground(QBrush(QColor(230, 230, 230)));

		QTableWidgetItem* environmentalEmissivityValueItem = new QTableWidgetItem(m_environmentalEmissivityValue);
		environmentalEmissivityValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		environmentalEmissivityValueItem->setFlags(environmentalEmissivityValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		environmentalEmissivityValueItem->setBackground(QBrush(QColor(230, 230, 230)));

		QTableWidgetItem* preheatingTimeValueItem = new QTableWidgetItem(m_preheatingTimeValue);
		preheatingTimeValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		preheatingTimeValueItem->setBackground(QBrush(QColor(255, 254, 195)));



		QTableWidgetItem* curveItem = new QTableWidgetItem(m_preheatingTimeValue);
		curveItem->setTextAlignment(Qt::AlignCenter); // 文本居中

		m_tableWidget->setItem(2, 2, targetTemperatureValueItem);
		m_tableWidget->setItem(3, 2, environmentalTemperatureValueItem);
		m_tableWidget->setItem(4, 2, initialTemperatureValueItem);
		m_tableWidget->setItem(5, 2, heatTransferCoefficientValueItem);
		m_tableWidget->setItem(6, 2, absorptionCoefficientValueItem);
		m_tableWidget->setItem(7, 2, environmentalEmissivityValueItem);
		m_tableWidget->setItem(9, 2, preheatingTimeValueItem);

		connect(m_tableWidget, &QTableWidget::itemChanged, this, [this, preheatingTimeValueItem](QTableWidgetItem* item) {

			if (item == preheatingTimeValueItem)
			{
				auto text = item->text();
				auto value = text.toDouble();
				m_preheatingTimeValue = text;
			}

		});

		// 显示按钮
		m_viewButton = new QPushButton("显示");
		m_tableWidget->setCellWidget(10, 2, m_viewButton);

		// 单位列
		QStringList unitLabels = { " "," ","℃","℃","℃", "W/㎡·k", "1/m"," "," ","s","" };
		for (int row = 0; row < unitLabels.size(); ++row) {
			if (row != 0)
			{
				QTableWidgetItem* labelItem = new QTableWidgetItem(unitLabels[row]);
				labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
				m_tableWidget->setItem(row, 3, labelItem);
			}

		}

		auto createCenteredRadioWidget = [](QRadioButton* radio) -> QWidget* {
			QWidget* container = new QWidget();
			QHBoxLayout* layout = new QHBoxLayout(container);
			layout->setAlignment(Qt::AlignCenter);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(radio);
			return container;
		};

		auto firRadioButtonGroup = new QButtonGroup();
		{
			m_targetempeRadioBtn = new QRadioButton();
			m_targetempeRadioBtn->setChecked(true);
			m_environmentalTempeRadioBtn = new QRadioButton();
			

			m_tableWidget->setCellWidget(2, 4, createCenteredRadioWidget(m_targetempeRadioBtn));
			//m_tableWidget->setCellWidget(3, 4, createCenteredRadioWidget(m_environmentalTempeRadioBtn));
			

			firRadioButtonGroup->addButton(m_targetempeRadioBtn);
			firRadioButtonGroup->addButton(m_environmentalTempeRadioBtn);
			
		}
		firRadioButtonGroup->setExclusive(true);

		auto secRadioButtonGroup = new QButtonGroup();
		{
			m_preheatingTimeRadioBtn = new QRadioButton();
			m_preheatingTimeRadioBtn->setChecked(true);

			m_tableWidget->setCellWidget(9, 4, createCenteredRadioWidget(m_preheatingTimeRadioBtn));

			secRadioButtonGroup->addButton(m_preheatingTimeRadioBtn);
		}
		secRadioButtonGroup->setExclusive(true);

		// 合并单元格
		m_tableWidget->setSpan(0, 0, 1, 2);
		m_tableWidget->setSpan(0, 2, 1, 3);
		m_tableWidget->setSpan(1, 2, 1, 3);
		m_tableWidget->setSpan(10, 2, 1, 3);
	}
	vlayout->addWidget(m_tableWidget);
	setLayout(vlayout);
	
	// 将第0行0列的单元格文本字体加粗
	QTableWidgetItem* headerItem = m_tableWidget->item(0, 0);
	if (headerItem) {
		QFont font = headerItem->font();
		font.setBold(true);
		headerItem->setFont(font);
	}

	

	//文本左对齐
	for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
		for (int col = 0; col < m_tableWidget->columnCount(); ++col) {
			QTableWidgetItem* item = m_tableWidget->item(row, col);
			if (item)
			{
				if (col == 0 && row != 0)
				{
					item->setTextAlignment(Qt::AlignCenter);
				}
				else
				{
					item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				}
			}
		}
	}

	// 遍历第2列（索引为1），将不可编辑单元格背景设置为浅灰色
	for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
		// 遍历行，设置行高
		m_tableWidget->setRowHeight(row, 10);
		QTableWidgetItem* item = m_tableWidget->item(row, 2);
		if (item && !(item->flags() & Qt::ItemIsEditable))
		{
			item->setBackground(QBrush(QColor(230, 230, 230)));
		}
		if (row != 0)
		{
			QTableWidgetItem* unitItem = m_tableWidget->item(row, 3);
			unitItem->setBackground(QBrush(QColor(230, 230, 230)));
		}
	}

}



void PreReverseOptimizationPropertyWidget::calculate() 
{

	if (m_preheatingTimeValue == "")
	{
		QMessageBox::information(this, "提示", "弹体预热时间不能为空！");
		return;
	}
	QWidget* parent = parentWidget();
	while (parent) {
		GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
		if (gfParent)
		{
			QDateTime currentTime = QDateTime::currentDateTime();
			QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
			auto logWidget = gfParent->GetLogWidget();
			auto textEdit = logWidget->GetTextEdit();
			QString text = timeStr + "[信息]>开始预热工艺逆向寻优";
			textEdit->appendPlainText(text);
			logWidget->update();

			QApplication::processEvents();
			break;
		}
		else
		{
			parent = parent->parentWidget();
		}
	}

	// 创建进度对话框
	ProgressDialog* progressDialog = new ProgressDialog("预热工艺工程逆向寻优", this);
	progressDialog->show();

	// 创建工作线程和工作对象
	ReverseOptimizationWorker* calculateWorker = new ReverseOptimizationWorker();
	QThread* calculateThread = new QThread();
	calculateWorker->moveToThread(calculateThread);

	// 连接信号槽
	connect(calculateThread, &QThread::started, calculateWorker, &ReverseOptimizationWorker::DoWork);
	connect(calculateWorker, &ReverseOptimizationWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
	connect(calculateWorker, &ReverseOptimizationWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
	connect(progressDialog, &ProgressDialog::Canceled, calculateWorker, &ReverseOptimizationWorker::RequestInterruption, Qt::DirectConnection);

	// 处理导入结果
	connect(calculateWorker, &ReverseOptimizationWorker::WorkFinished, this,
		[=](bool success, const QString& msg) {
	
		auto ins = ModelDataManager::GetInstance();
		auto modelGeometryInfo = ins->GetModelGeometryInfo();
		auto steelPropertyInfo = ins->GetSteelPropertyInfo();
		auto calculationPropertyInfo = ins->GetCalculationPropertyInfo();

		//auto A = m_targetTemperatureValue.toDouble(); // 弹体目标温度（℃）
		auto B = modelGeometryInfo.shellThickness; // 壳体厚度 (mm)
		auto C = modelGeometryInfo.gasketLayerThickness; // 胶层厚度(mm)
		auto D = steelPropertyInfo.density; // 壳体密度 (kg m^-3)
		auto E = steelPropertyInfo.specificHeatCapacity; // 壳体比热容 (J kg^-1 K^-1)
		auto F = steelPropertyInfo.thermalConductivity; // 壳体导热系数 (W m^-1 K^-1)
		double target = m_tableWidget->item(9, 2)->text().toDouble();


		m_solver = new PreReverseFormulaSolver(this);
		// 绑定求解完成信号
		connect(m_solver, &PreReverseFormulaSolver::solveFinished, this, [=](const std::vector<double>& resList) {
			progressDialog->SetProgress(100);


			QWidget* parent = parentWidget();
			while (parent) {
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{
					QDateTime currentTime = QDateTime::currentDateTime();
					QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
					auto logWidget = gfParent->GetLogWidget();
					auto textEdit = logWidget->GetTextEdit();
					QString text = timeStr + "[信息]>预热工艺逆向寻优完成";
					textEdit->appendPlainText(text);
					logWidget->update();

					QApplication::processEvents();
					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}

			if (resList.empty()) {
				QMessageBox::warning(this, "警告", "无解");
				return;
			}

			for (double v : resList) {
				double value = 35 * v + 50;

				if (value >= 0 && value <= 100) {

					QTableWidgetItem* resultItem = new QTableWidgetItem(QString::number(qRound(value)));
					resultItem->setBackground(QBrush(QColor(2, 253, 254)));
					m_tableWidget->setItem(2, 2, resultItem);
					//QMessageBox::information(this, "成功", "计算完成");
					return;
				}
			}

			QMessageBox::warning(this, "提示", "解超出范围");
			});

		QString formula = calculationPropertyInfo.preForwardCalculateFormula;

		B = (B - 20) / 10;
		C = (C - 1) / 4;
		D = (D - 2160) / 7260;
		E = (E - 368) / 736;
		F = (F - 6) / 150;

		m_solver->solve(formula, target, 0, B, C, D, E, F, 'A');
		if (!success)
		{
			QMessageBox::warning(this, "计算失败", msg);
		}
		// 清理资源
		progressDialog->close();
		calculateThread->quit();
		calculateThread->wait();
		calculateWorker->deleteLater();
		calculateThread->deleteLater();
		progressDialog->deleteLater();
		});

	// 启动线程
	calculateThread->start();
		
}


void PreReverseOptimizationPropertyWidget::reset()
{
	m_targetTemperatureValue = "50";
	QTableWidgetItem* targetTemperatureValueItem = new QTableWidgetItem(m_targetTemperatureValue);
	targetTemperatureValueItem->setBackground(QBrush(QColor(2, 253, 254)));
	m_tableWidget->setItem(2, 2, targetTemperatureValueItem);

	m_preheatingTimeValue = "";
	QTableWidgetItem* resultItem = new QTableWidgetItem(m_preheatingTimeValue);
	resultItem->setBackground(QBrush(QColor(255, 254, 195)));
	m_tableWidget->setItem(9, 2, resultItem);
}