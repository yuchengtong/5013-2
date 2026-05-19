#pragma once
#pragma execution_character_set("utf-8")
#include "PreForwardDesignPropertyWidget.h"
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
#include "../ForwardDesignWorker.h"
#include "../ProgressDialog.h"


#include <QDateTime>
#include <QApplication>


#include <QString>
#include <QMap>
#include <QRegExp>
#include <stdexcept>
#include <cmath>

// 计算
double preForwardCalculateForm(const QString& formula,
	double A, double B, double C, double D, double E, double F)
{
	A = (A - 50) / 35;
	B = (B - 20) / 10;
	C = (C - 1) / 4;
	D = (D - 2160) / 7260;
	E = (E - 368) / 736;
	F = (F - 6) / 150;

	QString processedFormula = formula;
	processedFormula.remove(' '); // 移除所有空格

	// 变量映射：公式二使用 A-F
	const QMap<QString, double> varMap = {
		{"A", A}, {"B", B}, {"C", C}, {"D", D}, {"E", E}, {"F", F}
	};

	/************************ 核心：支持 1/2/3/4次项 + 多变量乘积 ************************/
	QRegExp regExp("([+-]?)((?:\\d+(?:\\.\\d*)?)|(?:\\.\\d+))(\\*[A-F](?:\\^[234])?(?:\\*[A-F](?:\\^[234])?)*)?");
	regExp.setMinimal(false);

	double result = 0.0;
	int pos = 0;
	int matchCount = 0;

	// 补全开头符号，统一逻辑（和代码一完全一致）
	if (!processedFormula.isEmpty() && processedFormula[0] != '+' && processedFormula[0] != '-') {
		processedFormula = "+" + processedFormula;
	}

	// 循环匹配所有项（结构和代码一完全一致）
	while ((pos = regExp.indexIn(processedFormula, pos)) != -1) {
		++matchCount;

		QString signStr = regExp.cap(1);    // 符号 +/-
		QString coeffStr = regExp.cap(2);   // 系数
		QString varPart = regExp.cap(3);    // 变量部分（如 *A^2*B^3）

		// 1. 符号解析（和代码一一致）
		double sign = (signStr == "-") ? -1.0 : 1.0;

		// 2. 系数解析（和代码一一致）
		bool ok = false;
		double coeff = coeffStr.toDouble(&ok);
		if (!ok) {
			throw std::invalid_argument(QString("无效系数: %1").arg(coeffStr).toStdString());
		}

		// 3. 计算变量乘积值（支持 ^2 ^3 ^4）
		double varValue = 1.0;
		if (!varPart.isEmpty()) {
			// 去掉开头的 *，按 * 拆分
			QString vars = varPart.mid(1);
			QStringList units = vars.split('*');

			for (const QString& unit : units) {
				if (unit.contains('^')) {
					// 处理次方项 A^3 / F^4
					QStringList parts = unit.split('^');
					QString vName = parts[0];
					int power = parts[1].toInt();

					if (!varMap.contains(vName)) {
						throw std::invalid_argument(QString("未知变量: %1").arg(vName).toStdString());
					}
					double val = varMap[vName];
					varValue *= std::pow(val, power);
				}
				else {
					// 单次变量 A/B/C
					if (!varMap.contains(unit)) {
						throw std::invalid_argument(QString("未知变量: %1").arg(unit).toStdString());
					}
					varValue *= varMap[unit];
				}
			}
		}

		// 累加当前项
		result += sign * coeff * varValue;
		pos += regExp.matchedLength();
	}

	// 公式校验
	if (matchCount == 0) {
		throw std::invalid_argument(QString("公式格式错误: %1").arg(formula).toStdString());
	}

	return result;
}



PreForwardDesignPropertyWidget::PreForwardDesignPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void PreForwardDesignPropertyWidget::initWidget()
{
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	m_tableWidget = new QTableWidget(this);

	m_tableWidget->setRowCount(11);
	m_tableWidget->setColumnCount(4);
	// 隐藏表头（如果不需要显示表头文字，可根据需求决定是否隐藏）
	m_tableWidget->horizontalHeader()->setVisible(false);
	m_tableWidget->verticalHeader()->setVisible(false);

	// 设置第一列固定宽度（例如100像素）
	m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->resizeSection(0, 5);
	m_tableWidget->horizontalHeader()->resizeSection(1, 120);
	m_tableWidget->horizontalHeader()->resizeSection(3, 80);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
	// 让表格充满布局，自动调整行列大小
	m_tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_tableWidget->setColumnWidth(0, 5);
	// 合并第一行的第一和第二列
	m_tableWidget->setSpan(0, 0, 1, 2);

	vlayout->addWidget(m_tableWidget);
	setLayout(vlayout);

	QStringList cols = { "正向设计"," ",  "1", "2", "3","4","5","6"," ","1","2" };
	for (int row = 0; row < cols.size(); ++row) {
		QTableWidgetItem* serialItem = new QTableWidgetItem(cols[row]);
		serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 0, serialItem);
	}

	QStringList labels = { "正向设计","工艺输入参数",  "弹体目标温度(50～90)", "烘箱环境温度", "弹体初始温度","环境对流传热系数","壳体辐射吸收系数","环境发射率","工艺输出参数","弹体预热时间","弹体温度云图与温升曲线" };
	for (int row = 0; row < labels.size(); ++row) {
		QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
		labelItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 1, labelItem);

	}

	// 设置第一列宽度
	QTableWidgetItem* colimnItem = m_tableWidget->item(10, 1);
	int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
	m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());


	// 导入按钮
	QPushButton* importButton = new QPushButton("计算");
	QPushButton* resetButton = new QPushButton("默认");
	m_tableWidget->setCellWidget(0, 2, importButton);
	m_tableWidget->setCellWidget(1, 2, resetButton);
	connect(importButton, &QPushButton::clicked, this, &PreForwardDesignPropertyWidget::preForwardCalculate);
	connect(resetButton, &QPushButton::clicked, this, &PreForwardDesignPropertyWidget::reset);
	// 合并第一行的第三和第四列
	m_tableWidget->setSpan(0, 2, 1, 2);
	m_tableWidget->setSpan(1, 2, 1, 2);


	QTableWidgetItem* targetTemperatureValueItem = new QTableWidgetItem(m_targetTemperatureValue);
	targetTemperatureValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	targetTemperatureValueItem->setBackground(QBrush(QColor(255, 254, 195)));

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
	preheatingTimeValueItem->setBackground(QBrush(QColor(2, 253, 254)));


	QTableWidgetItem* curveItem = new QTableWidgetItem(m_preheatingTimeValue);
	curveItem->setTextAlignment(Qt::AlignCenter); // 文本居中


	m_tableWidget->setItem(2, 2, targetTemperatureValueItem);
	m_tableWidget->setItem(3, 2, environmentalTemperatureValueItem);
	m_tableWidget->setItem(4, 2, initialTemperatureValueItem);
	m_tableWidget->setItem(5, 2, heatTransferCoefficientValueItem);
	m_tableWidget->setItem(6, 2, absorptionCoefficientValueItem);
	m_tableWidget->setItem(7, 2, environmentalEmissivityValueItem);
	m_tableWidget->setItem(9, 2, preheatingTimeValueItem);

	// 显示按钮
	QPushButton* viewButton = new QPushButton("显示");
	m_tableWidget->setCellWidget(10, 2, viewButton);
	m_tableWidget->setSpan(10, 2, 1, 2);
	connect(viewButton, &QPushButton::clicked, this, &PreForwardDesignPropertyWidget::view);

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

	QTableWidgetItem* unitColimnItem = m_tableWidget->item(5, 3);
	int unitItemWidth = QFontMetrics(m_tableWidget->font()).width(unitColimnItem->text());
	m_tableWidget->setColumnWidth(3, unitItemWidth + m_tableWidget->verticalHeader()->width());

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

	connect(m_tableWidget, &QTableWidget::itemChanged, this, [this, targetTemperatureValueItem](QTableWidgetItem* item) {

		if (item == targetTemperatureValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();
			if (value >=50 && value <= 90)
			{
				m_targetTemperatureValue = text;
			}
			else
			{
				m_tableWidget->blockSignals(true);
				item->setText(m_targetTemperatureValue);
				m_tableWidget->blockSignals(false);
			}

		}
	});

}

void PreForwardDesignPropertyWidget::preForwardCalculate() 
{

	QWidget* parent = parentWidget();
	while (parent) {
		GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
		if (gfParent)
		{
			QDateTime currentTime = QDateTime::currentDateTime();
			QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
			auto logWidget = gfParent->GetLogWidget();
			auto textEdit = logWidget->GetTextEdit();
			QString text = timeStr + "[信息]>开始预热工艺正向计算";
			textEdit->appendPlainText(text);
			logWidget->update();

			QApplication::processEvents();


			// 创建进度对话框
			ProgressDialog* progressDialog = new ProgressDialog("预热工艺正向计算", this);
			progressDialog->show();

			// 创建工作线程和工作对象
			ForwardDesignWorker* calculateWorker = new ForwardDesignWorker();
			QThread* calculateThread = new QThread();
			calculateWorker->moveToThread(calculateThread);

			// 连接信号槽
			connect(calculateThread, &QThread::started, calculateWorker, &ForwardDesignWorker::DoWork);
			connect(calculateWorker, &ForwardDesignWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
			connect(calculateWorker, &ForwardDesignWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
			connect(progressDialog, &ProgressDialog::Canceled, calculateWorker, &ForwardDesignWorker::RequestInterruption, Qt::DirectConnection);

			// 处理导入结果
			connect(calculateWorker, &ForwardDesignWorker::WorkFinished, this,
				[=](bool success, const QString& msg) {

					auto ins = ModelDataManager::GetInstance();
					auto modelGeometryInfo = ins->GetModelGeometryInfo();
					auto steelPropertyInfo = ins->GetSteelPropertyInfo();
					auto calculationPropertyInfo = ins->GetCalculationPropertyInfo();

					auto A = m_targetTemperatureValue.toDouble(); // 弹体目标温度（℃）
					auto B = modelGeometryInfo.shellThickness; // 壳体厚度 (mm)
					auto C = modelGeometryInfo.gasketLayerThickness; // 胶层厚度(mm)
					auto D = steelPropertyInfo.density; // 壳体密度 (kg m^-3)
					auto E = steelPropertyInfo.specificHeatCapacity; // 壳体比热容 (J kg^-1 K^-1)
					auto F = steelPropertyInfo.thermalConductivity; // 壳体导热系数 (W m^-1 K^-1)


					double value = preForwardCalculateForm(calculationPropertyInfo.preForwardCalculateFormula, A, B, C, D, E, F);
					QString result = QString::number(qRound(value));
					m_preheatingTimeValue = result;
					QTableWidgetItem* resultItem = new QTableWidgetItem(m_preheatingTimeValue);
					resultItem->setBackground(QBrush(QColor(2, 253, 254)));
					m_tableWidget->setItem(9, 2, resultItem);

					if (!success)
					{
						QMessageBox::warning(this, "计算失败", msg);
					}
					QMessageBox::information(this, "计算", "计算成功");

					QString newTimeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
					QString newText = newTimeStr + "[信息]>预热工艺正向计算完成";
					textEdit->appendPlainText(newText);
					logWidget->update();
					QApplication::processEvents();

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



			break;
		}
		else
		{
			parent = parent->parentWidget();
		}
	}

	
}

void PreForwardDesignPropertyWidget::reset()
{
	m_targetTemperatureValue = "50";
	QTableWidgetItem* targetTemperatureValueItem = new QTableWidgetItem(m_targetTemperatureValue);
	targetTemperatureValueItem->setBackground(QBrush(QColor(255, 254, 195)));
	m_tableWidget->setItem(2, 2, targetTemperatureValueItem);

	m_preheatingTimeValue = "";
	QTableWidgetItem* resultItem = new QTableWidgetItem(m_preheatingTimeValue);
	resultItem->setBackground(QBrush(QColor(2, 253, 254)));
	m_tableWidget->setItem(9, 2, resultItem);
}



void PreForwardDesignPropertyWidget::view()
{

	double start = 49.0;  // 初始温度
	double end = m_targetTemperatureValue.toDouble(); // 弹体目标温度（℃）
	double step = (end-start)/10;

	auto ins = ModelDataManager::GetInstance();
	auto modelGeometryInfo = ins->GetModelGeometryInfo();
	auto steelPropertyInfo = ins->GetSteelPropertyInfo();
	auto calculationPropertyInfo = ins->GetCalculationPropertyInfo();

	auto A = m_targetTemperatureValue.toDouble(); // 弹体目标温度（℃）
	auto B = modelGeometryInfo.shellThickness; // 壳体厚度 (mm)
	auto C = modelGeometryInfo.gasketLayerThickness; // 胶层厚度(mm)
	auto D = steelPropertyInfo.density; // 壳体密度 (kg m^-3)
	auto E = steelPropertyInfo.specificHeatCapacity; // 壳体比热容 (J kg^-1 K^-1)
	auto F = steelPropertyInfo.thermalConductivity; // 壳体导热系数 (W m^-1 K^-1)

	QVector<double> x;
	QVector<double> y;
	for (double i = start; i <= end; i += step) {
		x.push_back(i);
		double value = preForwardCalculateForm(calculationPropertyInfo.preForwardCalculateFormula, i, B, C, D, E, F);
		QString result = QString::number(qRound(value));
		y.push_back(result.toDouble());

	}
	x.push_back(end);
	double value = preForwardCalculateForm(calculationPropertyInfo.preForwardCalculateFormula, end, B, C, D, E, F);
	QString result = QString::number(qRound(value));
	y.push_back(result.toDouble());


	
	// 更新曲线图
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


			auto preForwardTimeTempWid = gfParent->GetPreForwardTimeTempWid();

			/*QVector<double> x;
			x.push_back(0.0);
			x.push_back(300.0);
			x.push_back(600.0);
			QVector<double> y;
			y.push_back(22.0);
			y.push_back(40.0);
			y.push_back(60.0);*/


			preForwardTimeTempWid->AddDataPoint(x, y);

			break;
		}
		else
		{
			parent = parent->parentWidget();
		}
	}
}