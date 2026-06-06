#pragma once
#pragma execution_character_set("utf-8")
#include "InForwardDesignPropertyWidget.h"
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
#include <V3d_TypeOfOrientation.hxx>
#include <V3d_View.hxx>

//计算
double inForwardCalculateForm(const QString& formula,
	double A, double B, double C, double D, double E)
{
	QString processedFormula = formula;
	processedFormula.remove(' '); // 移除所有空格

	// 变量映射：5个变量 A-E
	const QMap<QString, double> varMap = {
		{"A", A}, {"B", B}, {"C", C}, {"D", D}, {"E", E}
	};

	/************************ 核心：支持1-7次项 + 多变量乘积 ************************/
	// 正则适配：变量A-E，次方支持 ^1~^7（可省略^代表1次）
	QRegExp regExp("([+-]?)((?:\\d+(?:\\.\\d*)?)|(?:\\.\\d+))(\\*[A-E](?:\\^[1234567])?(?:\\*[A-E](?:\\^[1234567])?)*)?");
	regExp.setMinimal(false);

	double result = 0.0;
	int pos = 0;
	int matchCount = 0;

	// 补全开头符号，统一逻辑
	if (!processedFormula.isEmpty() && processedFormula[0] != '+' && processedFormula[0] != '-') {
		processedFormula = "+" + processedFormula;
	}

	// 循环匹配所有项
	while ((pos = regExp.indexIn(processedFormula, pos)) != -1) {
		++matchCount;

		QString signStr = regExp.cap(1);    // 符号 +/-
		QString coeffStr = regExp.cap(2);   // 系数
		QString varPart = regExp.cap(3);    // 变量部分（如 *A^7*B^2*E）

		// 1. 符号解析
		double sign = (signStr == "-") ? -1.0 : 1.0;

		// 2. 系数解析
		bool ok = false;
		double coeff = coeffStr.toDouble(&ok);
		if (!ok) {
			throw std::invalid_argument(QString("无效系数: %1").arg(coeffStr).toStdString());
		}

		// 3. 计算变量乘积值（支持 ^1 ~ ^7）
		double varValue = 1.0;
		if (!varPart.isEmpty()) {
			// 去掉开头的 *，按 * 拆分
			QString vars = varPart.mid(1);
			QStringList units = vars.split('*');

			for (const QString& unit : units) {
				if (unit.contains('^')) {
					// 处理次方项 A^7 / C^3
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
					// 单次变量 A/B/C/D/E
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

InForwardDesignPropertyWidget::InForwardDesignPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void InForwardDesignPropertyWidget::initWidget()
{

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	m_tableWidget = new QTableWidget(this);

	m_tableWidget->setRowCount(10);
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

	QStringList cols = { "正向设计"," ",  "1", "2", "3","4"," ","1","2","3" };
	for (int row = 0; row < cols.size(); ++row) {
		QTableWidgetItem* serialItem = new QTableWidgetItem(cols[row]);
		serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 0, serialItem);
	}


	QStringList labels = { "正向设计","工艺输入参数",  "弹体保温温度(50～70)", "药液浇注温度", "阀门开度(13～39)","真空度(0.02～0.08)","工艺输出参数","相对密度","弹体注药时间","弹体温度云图与温升曲线" };
	for (int row = 0; row < labels.size(); ++row) {
		QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
		labelItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 1, labelItem);


	}

	// 导入按钮
	QPushButton* importButton = new QPushButton("计算");
	QPushButton* resetButton = new QPushButton("默认");
	m_tableWidget->setCellWidget(0, 2, importButton);
	m_tableWidget->setCellWidget(1, 2, resetButton);
	connect(importButton, &QPushButton::clicked, this, &InForwardDesignPropertyWidget::inForwardCalculate);
	connect(resetButton, &QPushButton::clicked, this, &InForwardDesignPropertyWidget::reset);


	// 合并第一行的第三和第四列
	m_tableWidget->setSpan(0, 2, 1, 2);
	m_tableWidget->setSpan(1, 2, 1, 2);

	QTableWidgetItem* insulationTemperatureValueItem = new QTableWidgetItem(m_insulationTemperatureValue);
	insulationTemperatureValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	insulationTemperatureValueItem->setBackground(QBrush(QColor(255, 254, 195)));

	QTableWidgetItem* pouringTemperatureValueItem = new QTableWidgetItem(m_pouringTemperatureValue);
	pouringTemperatureValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	pouringTemperatureValueItem->setFlags(pouringTemperatureValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	pouringTemperatureValueItem->setBackground(QBrush(QColor(230, 230, 230)));

	QTableWidgetItem* valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
	valveOpeningValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	valveOpeningValueItem->setBackground(QBrush(QColor(255, 254, 195)));

	QTableWidgetItem* vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
	vacuumDegreeValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	vacuumDegreeValueItem->setBackground(QBrush(QColor(255, 254, 195)));

	QTableWidgetItem* relativeDensityValueItem = new QTableWidgetItem(m_relativeDensityValue);
	relativeDensityValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	relativeDensityValueItem->setBackground(QBrush(QColor(2, 253, 254)));

	QTableWidgetItem* injectionTimeValueItem = new QTableWidgetItem(m_injectionTimeValue);
	injectionTimeValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	injectionTimeValueItem->setBackground(QBrush(QColor(2, 253, 254)));

	m_tableWidget->setItem(2, 2, insulationTemperatureValueItem);
	m_tableWidget->setItem(3, 2, pouringTemperatureValueItem);
	m_tableWidget->setItem(4, 2, valveOpeningValueItem);
	m_tableWidget->setItem(5, 2, vacuumDegreeValueItem);
	m_tableWidget->setItem(7, 2, relativeDensityValueItem);
	m_tableWidget->setItem(8, 2, injectionTimeValueItem);

	// 显示按钮
	QPushButton* viewButton = new QPushButton("显示");
	m_tableWidget->setCellWidget(9, 2, viewButton);
	m_tableWidget->setSpan(9, 2, 1, 2);
	connect(viewButton, &QPushButton::clicked, this, &InForwardDesignPropertyWidget::view);

	// 设置列宽度
	QTableWidgetItem* colimnItem = m_tableWidget->item(9, 1);
	int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
	m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());

	// 单位列
	QStringList unitLabels = { " "," ","℃", "℃", "mm","MPa"," ","%","s"," " };
	for (int row = 0; row < unitLabels.size(); ++row) {
		if (row != 0)
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(unitLabels[row]);
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 3, labelItem);
		}

	}

	QTableWidgetItem* unitColimnItem = m_tableWidget->item(4, 3);
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

	m_tableWidget->setItem(2, 2, insulationTemperatureValueItem);
	m_tableWidget->setItem(3, 2, pouringTemperatureValueItem);
	m_tableWidget->setItem(4, 2, valveOpeningValueItem);
	m_tableWidget->setItem(5, 2, vacuumDegreeValueItem);
	connect(m_tableWidget, &QTableWidget::itemChanged, this, [this, insulationTemperatureValueItem, pouringTemperatureValueItem, valveOpeningValueItem, vacuumDegreeValueItem, relativeDensityValueItem, injectionTimeValueItem](QTableWidgetItem* item) {

		if (item == insulationTemperatureValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();
			if (value >= 50 && value <= 70)
			{
				m_insulationTemperatureValue = text;
			}
			else
			{
				m_tableWidget->blockSignals(true);
				item->setText(m_insulationTemperatureValue);
				m_tableWidget->blockSignals(false);
			}
		}

		if (item == pouringTemperatureValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();
			if (value >= 50 && value <= 85)
			{
				m_pouringTemperatureValue = text;
			}
			else
			{
				m_tableWidget->blockSignals(true);
				item->setText(m_pouringTemperatureValue);
				m_tableWidget->blockSignals(false);
			}
		}

		if (item == valveOpeningValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();
			if (value >= 13 && value <= 39)
			{
				m_valveOpeningValue = text;
			}
			else
			{
				m_tableWidget->blockSignals(true);
				item->setText(m_valveOpeningValue);
				m_tableWidget->blockSignals(false);
			}
		}

		if (item == vacuumDegreeValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();

			if (value >= 0.02 && value <= 0.08)
			{
				m_vacuumDegreeValue = text;
			}
			else
			{
				m_tableWidget->blockSignals(true);
				item->setText(m_vacuumDegreeValue);
				m_tableWidget->blockSignals(false);
			}
		}

		if (item == relativeDensityValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();

			if (value > 0 && value <= 100)
			{
				m_relativeDensityValue = text;
			}
			else
			{
				m_tableWidget->blockSignals(true);
				item->setText(m_relativeDensityValue);
				m_tableWidget->blockSignals(false);
			}
		}

		if (item == injectionTimeValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();
			if (value > 0)
			{
				m_injectionTimeValue = text;
			}
			else
			{
				m_tableWidget->blockSignals(true);
				item->setText(m_injectionTimeValue);
				m_tableWidget->blockSignals(false);
			}

		}
		});

}

void InForwardDesignPropertyWidget::inForwardCalculate()
{

	auto ins = ModelDataManager::GetInstance();
	auto modelGeometryInfo = ins->GetModelGeometryInfo();
	auto steelPropertyInfo = ins->GetSteelPropertyInfo();
	auto propellantPropertyInfo = ins->GetPropellantPropertyInfo();
	auto gelatinPropertyInfo = ins->GetGelatinPropertyInfo();
	auto calculationPropertyInfo = ins->GetCalculationPropertyInfo();
	auto inForwardPropertyInfo = ins->GetInForwardPropertyInfo();
	if (!steelPropertyInfo.isChecked)
	{
		QMessageBox::warning(this, "提示", "壳体物性材料未选择");
		return;
	}
	if (!propellantPropertyInfo.isChecked)
	{
		QMessageBox::warning(this, "提示", "药液物性材料未选择");
		return;
	}
	if (!gelatinPropertyInfo.isChecked)
	{
		QMessageBox::warning(this, "提示", "明胶物性材料未选择");
		return;
	}
	if (steelPropertyInfo.density == 0.0)
	{
		QMessageBox::warning(this, "提示", "壳体物性材料参数数值不能为0");
		return;
	}

	QWidget* parent = parentWidget();
	GFImportModelWidget* gfParent = nullptr;
	while (parent) {
		gfParent = dynamic_cast<GFImportModelWidget*>(parent);
		if (gfParent)
		{
			QDateTime currentTime = QDateTime::currentDateTime();
			QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
			auto logWidget = gfParent->GetLogWidget();
			auto textEdit = logWidget->GetTextEdit();
			QString text = timeStr + "[信息]>开始注药工艺正向计算";
			textEdit->appendPlainText(text);
			logWidget->update();

			QApplication::processEvents();


			// 创建进度对话框
			ProgressDialog* progressDialog = new ProgressDialog("注药工艺正向计算", this);
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
					auto A = m_valveOpeningValue.toDouble() / 2.0; // 阀门开度（mm）
					auto B = modelGeometryInfo.gasketLayerThickness; // 胶层厚度(mm)
					auto C = modelGeometryInfo.shellThickness; // 壳体厚度 (mm)
					auto D = m_vacuumDegreeValue.toDouble() * 1000; // 真空度(KPa)
					auto E = m_insulationTemperatureValue.toDouble(); // 保温温度（℃）

					// 获取模型类型
					QString model = ModelDataManager::GetInstance()->GetModelGeometryInfo().model;;
					
					double gasRateValue = 0.0;
					double injectionTimeValue = 0.0;
					if (model == "HQ-9B")
					{
						if (A < 6.740741)
						{
							A = 6.740741;
						}
						else if (A > 18.296296)
						{
							A = 18.296296;
						}
						if (B < 1.074074)
						{
							B = 1.074074;
						}
						else if (B > 4.9259266)
						{
							B = 4.925926;
						}
						if (C < 20.185185)
						{
							C = 20.185185;
						}
						else if (C > 29.814815)
						{
							C = 29.814815;
						}
						if (D < 21.111111)
						{
							D = 21.111111;
						}
						else if (D > 78.888889)
						{
							D = 78.888889;
						}
						if (E < 53.000000)
						{
							E = 53.000000;
						}
						else if (E > 69.629630)
						{
							E = 69.629630;
						}
						// 气含率
						auto gasRateA = (A - 12.509291) / 2.218704;
						auto gasRateB = (B - 3.01888) / 0.702157;
						auto gasRateC = (C - 25.076659) / 1.740173;
						auto gasRateD = (D - 50.037481) / 10.504812;
						auto gasRateE = (E - 60.043506) / 3.582356;
						gasRateValue = inForwardCalculateForm(calculationPropertyInfo.oneGasRateCalculateFormula, gasRateA, gasRateB, gasRateC, gasRateD, gasRateE);


						// 注药时间
						auto injectionTimeA = (A - 12.518519) / 2.177802;
						auto injectionTimeB = (B - 2.999897) / 0.725859;
						auto injectionTimeC = (C - 25.001792) / 1.8102640;
						auto injectionTimeD = (D - 50.000000) / 10.889012;
						auto injectionTimeE = (E - 60.005435) / 3.621028;
						injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.oneInjectionTimeCalculateFormula, injectionTimeA, injectionTimeB, injectionTimeC, injectionTimeD, injectionTimeE);
						injectionTimeValue = injectionTimeValue * 21.33;
					}
					else if (model == "YJ-20")
					{
						if (A < 6.500000)
						{
							A = 6.500000;
						}
						else if (A > 19.500000)
						{
							A = 19.500000;
						}
						if (B < 1.000000)
						{
							B = 1.000000;
						}
						else if (B > 5.000000)
						{
							B = 5.000000;
						}
						if (C < 20.000000)
						{
							C = 20.000000;
						}
						else if (C > 30.000000)
						{
							C = 30.000000;
						}
						if (D < 20.000000)
						{
							D = 20.000000;
						}
						else if (D > 80.000000)
						{
							D = 80.000000;
						}
						if (E < 50.000000)
						{
							E = 50.000000;
						}
						else if (E > 70.000000)
						{
							E = 70.000000;
						}
						// 气含率
						auto gasRateA = (A - 13.000000) / 2.44883;
						auto gasRateB = (B - 3.000000) / 0.753487;
						auto gasRateC = (C - 25.000000) / 1.883716;
						auto gasRateD = (D - 50.000000) / 11.302298;
						auto gasRateE = (E - 60.000000) / 3.767433;
						gasRateValue = inForwardCalculateForm(calculationPropertyInfo.twoGasRateCalculateFormula, gasRateA, gasRateB, gasRateC, gasRateD, gasRateE);


						// 注药时间
						auto injectionTimeA = (A - 13.000000) / 2.44883;
						auto injectionTimeB = (B - 3.000000) / 0.753487;
						auto injectionTimeC = (C - 25.000000) / 1.883716;
						auto injectionTimeD = (D - 50.000000) / 11.302298;
						auto injectionTimeE = (E - 60.000000) / 3.767433;
						injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.twoInjectionTimeCalculateFormula, injectionTimeA, injectionTimeB, injectionTimeC, injectionTimeD, injectionTimeE);
						injectionTimeValue = injectionTimeValue * 26.67;
					}
					else if (model == "YJ-91A")
					{
						if (A < 6.500000)
						{
							A = 6.500000;
						}
						else if (A > 18.777778)
						{
							A = 18.777778;
						}
						if (B < 1.000000)
						{
							B = 1.000000;
						}
						else if (B > 5.000000)
						{
							B = 5.000000;
						}
						if (C < 20.000000)
						{
							C = 20.000000;
						}
						else if (C > 30.000000)
						{
							C = 30.000000;
						}
						if (D < 20.000000)
						{
							D = 20.000000;
						}
						else if (D > 80.000000)
						{
							D = 80.000000;
						}
						if (E < 50.000000)
						{
							E = 50.000000;
						}
						else if (E > 70.000000)
						{
							E = 70.000000;
						}
						// 气含率
						auto gasRateA = (A - 12.638889) / 2.312785;
						auto gasRateB = (B - 3.000000) / 0.753487;
						auto gasRateC = (C - 25.000000) / 1.883716;
						auto gasRateD = (D - 50.000000) / 11.302298;
						auto gasRateE = (E - 60.000000) / 3.767433;
						gasRateValue = inForwardCalculateForm(calculationPropertyInfo.threeGasRateCalculateFormula, gasRateA, gasRateB, gasRateC, gasRateD, gasRateE);


						// 注药时间
						auto injectionTimeA = (A - 12.638889) / 2.312785;
						auto injectionTimeB = (B - 3.000000) / 0.753487;
						auto injectionTimeC = (C - 25.000000) / 1.883716;
						auto injectionTimeD = (D - 50.000000) / 11.302298;
						auto injectionTimeE = (E - 60.000000) / 3.767433;
						injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.threeInjectionTimeCalculateFormula, injectionTimeA, injectionTimeB, injectionTimeC, injectionTimeD, injectionTimeE);
						injectionTimeValue = injectionTimeValue * 26.33;
					}
					else
					{
						if (A < 8.500000)
						{
							A = 8.500000;
						}
						else if (A > 18.296296)
						{
							A = 18.296296;
						}
						if (B < 1.000000)
						{
							B = 1.000000;
						}
						else if (B > 5.000000)
						{
							B = 5.000000;
						}
						if (C < 20.000000)
						{
							C = 20.000000;
						}
						else if (C > 30.000000)
						{
							C = 30.000000;
						}
						if (D < 20.000000)
						{
							D = 20.000000;
						}
						else if (D > 80.000000)
						{
							D = 80.000000;
						}
						if (E < 50.000000)
						{
							E = 50.000000;
						}
						else if (E > 70.000000)
						{
							E = 70.000000;
						}
						// 气含率
						auto gasRateA = (A - 13.398148) / 1.845344;
						auto gasRateB = (B - 3.000000) / 0.753487;
						auto gasRateC = (C - 25.000000) / 1.883716;
						auto gasRateD = (D - 50.000000) / 11.302298;
						auto gasRateE = (E - 60.000000) / 3.767433;
						gasRateValue = inForwardCalculateForm(calculationPropertyInfo.fourGasRateCalculateFormula, gasRateA, gasRateB, gasRateC, gasRateD, gasRateE);


						// 注药时间
						auto injectionTimeA = (A - 13.398148) / 1.845344;
						auto injectionTimeB = (B - 3.000000) / 0.753487;
						auto injectionTimeC = (C - 25.000000) / 1.883716;
						auto injectionTimeD = (D - 50.000000) / 11.302298;
						auto injectionTimeE = (E - 60.000000) / 3.767433;
						injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.fourInjectionTimeCalculateFormula, injectionTimeA, injectionTimeB, injectionTimeC, injectionTimeD, injectionTimeE);
						injectionTimeValue = injectionTimeValue * 27.33;
					}

					// 气含率转相对密度
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity > 1.0)
					{
						relativeDensity = 1.0;
					}
					QString relativeDensityResult = QString::number(relativeDensity * 100, 'f', 4);
					// 保存结果
					InForwardPropertyInfo inForwardPropertyInfo = ins->GetInForwardPropertyInfo();
					inForwardPropertyInfo.m_relativeDensityValue = relativeDensityResult.toDouble();
					inForwardPropertyInfo.isChecked = true;
					ins->SetInForwardPropertyInfo(inForwardPropertyInfo);

					QTableWidgetItem* relativeDensityItem = new QTableWidgetItem(relativeDensityResult);
					relativeDensityItem->setBackground(QBrush(QColor(2, 253, 254)));
					m_tableWidget->setItem(7, 2, relativeDensityItem);

					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					QString injectionTimeResult = QString::number(qRound(injectionTimeValue));
					QTableWidgetItem* injectionTimeItem = new QTableWidgetItem(injectionTimeResult);
					injectionTimeItem->setBackground(QBrush(QColor(2, 253, 254)));
					m_tableWidget->setItem(8, 2, injectionTimeItem);

					auto toolsAnimationWidget = gfParent->GetToolsAnimationWidget();
					QStringList names = { "第一帧" ,"第二帧" ,"第三帧" ,"第四帧" ,"第五帧" ,"第六帧" ,
					"第七帧" ,"第八帧" ,"第九帧" ,"第十帧" ,"第十一帧" ,"第十二帧" };
					toolsAnimationWidget->SetAnimationSteps(names);

					connect(toolsAnimationWidget, &ToolsAnimationWidget::animationFrameChanged, this, [=](int frameIndex) {
						auto treeModelWidget = gfParent->GetGFTreeModelWidget();
						auto item = treeModelWidget->GetGFTreeWidget()->currentItem();
						auto name = item->text(0);
						bool isForwardDesign = (item->data(0, Qt::UserRole).toString() == "InForwardDesign");
						if (isForwardDesign)
						{
							auto occView = gfParent->GetOccView();
							std::vector<double> nodeValues;
							APISetNodeValue::SetInForwardDesignResult(occView, nodeValues, frameIndex);

							Handle(AIS_InteractiveContext) context = occView->getContext();
							Handle(V3d_View) view = occView->getView();

							auto ins = ModelDataManager::GetInstance();
							auto inForwardPropertyInfo = ins->GetInForwardPropertyInfo();

							if (!inForwardPropertyInfo.m_ColorScale.IsNull()) {
								double timeValue = frameIndex * 5.0;
								double injectedMass = 1.5; // 假设当前已注药质量为 1.5 Kg

								QString titleStr = QString("时间: %1s\n体积分数\n当前已注药质量: %2 Kg")
									.arg(timeValue, 0, 'f', 0)
									.arg(injectedMass, 0, 'f', 2); // 保留两位小数

								TCollection_ExtendedString newTitle(titleStr.toUtf8().constData(), true);
								inForwardPropertyInfo.m_ColorScale->SetTitle(newTitle);

								// 关键：必须调用 Redisplay 才能刷新显示
								context->Redisplay(inForwardPropertyInfo.m_ColorScale, true);
							}

							Graphic3d_Vec2i anoffset(0, Standard_Integer(550));
							context->SetTransformPersistence(inForwardPropertyInfo.m_ColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
							context->SetDisplayMode(inForwardPropertyInfo.m_ColorScale, 1, Standard_False);
							context->Display(inForwardPropertyInfo.m_ColorScale, Standard_True);

							// 强制更新视图
							view->Invalidate();
							view->Redraw();
						}
						});

					// 初始化第 0 帧
					auto occView = gfParent->GetOccView();
					Handle(AIS_InteractiveContext) context = occView->getContext();
					context->EraseAll(true);

					std::vector<double> nodeValues;
					APISetNodeValue::SetInForwardDesignResult(occView, nodeValues, 0);

					double min_value = 0;
					double max_value = 100;

					// 初始化标题包含时间（第0帧 = 0s）
					TCollection_ExtendedString tostr("时间: 0s\n当前已注药质量：0Kg\n体积分数", true);

					Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
					{
						aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
						aColorScale->SetSize(200, 500);
						aColorScale->SetRange(min_value, max_value);
						aColorScale->SetNumberOfIntervals(9);
						aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
						aColorScale->SetTextHeight(30);
						aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
						aColorScale->SetTitle(tostr);
						aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
						aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
						aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
					}
					inForwardPropertyInfo.m_ColorScale = aColorScale;
					ins->SetInForwardPropertyInfo(inForwardPropertyInfo);

					Graphic3d_Vec2i anoffset(0, Standard_Integer(550));
					context->SetTransformPersistence(inForwardPropertyInfo.m_ColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
					context->SetDisplayMode(inForwardPropertyInfo.m_ColorScale, 1, Standard_False);
					context->Display(inForwardPropertyInfo.m_ColorScale, Standard_True);

					// 更新曲线图
					view();

					if (!success)
					{
						//QMessageBox::warning(this, "计算失败", msg);
					}
					//QMessageBox::information(this, "计算", "计算成功");
					QString newTimeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
					QString newText = newTimeStr + "[信息]>注药工艺正向计算完成";
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

void InForwardDesignPropertyWidget::reset()
{
	m_insulationTemperatureValue = "60";
	QTableWidgetItem* insulationTemperatureValueItem = new QTableWidgetItem(m_insulationTemperatureValue);
	insulationTemperatureValueItem->setBackground(QBrush(QColor(255, 254, 195)));
	m_tableWidget->setItem(2, 2, insulationTemperatureValueItem);

	m_valveOpeningValue = "15";
	QTableWidgetItem* valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
	valveOpeningValueItem->setBackground(QBrush(QColor(255, 254, 195)));
	m_tableWidget->setItem(4, 2, valveOpeningValueItem);

	m_vacuumDegreeValue = "0.05";
	QTableWidgetItem* vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
	vacuumDegreeValueItem->setBackground(QBrush(QColor(255, 254, 195)));
	m_tableWidget->setItem(5, 2, vacuumDegreeValueItem);


	m_relativeDensityValue = "";
	QTableWidgetItem* relativeDensityItem = new QTableWidgetItem(m_relativeDensityValue);
	relativeDensityItem->setBackground(QBrush(QColor(2, 253, 254)));
	m_tableWidget->setItem(7, 2, relativeDensityItem);

	m_injectionTimeValue = "";
	QTableWidgetItem* injectionTimeItem = new QTableWidgetItem(m_injectionTimeValue);
	injectionTimeItem->setBackground(QBrush(QColor(2, 253, 254)));
	m_tableWidget->setItem(8, 2, injectionTimeItem);
}

void InForwardDesignPropertyWidget::view()
{
	auto ins = ModelDataManager::GetInstance();
	auto modelGeometryInfo = ins->GetModelGeometryInfo();
	auto steelPropertyInfo = ins->GetSteelPropertyInfo();
	auto calculationPropertyInfo = ins->GetCalculationPropertyInfo();

	auto A = m_valveOpeningValue.toDouble() / 2.0; // 阀门开度（mm）
	auto B = modelGeometryInfo.gasketLayerThickness; // 胶层厚度(mm)
	auto C = modelGeometryInfo.shellThickness; // 壳体厚度 (mm)
	auto D = m_vacuumDegreeValue.toDouble() * 1000; // 真空度(KPa)
	auto E = m_insulationTemperatureValue.toDouble(); // 保温温度（℃）

	// 获取模型类型
	QString model = "HQ-9B";
	QWidget* parent = parentWidget();
	while (parent) {
		GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
		if (gfParent)
		{
			
			model = ModelDataManager::GetInstance()->GetModelGeometryInfo().model;

			QVector<double> densityTempX;
			QVector<double> densityTempY;

			QVector<double> densityValveX;
			QVector<double> densityValveY;

			QVector<double> densityVacuumX;
			QVector<double> densityVacuumY;

			QVector<double> timeTempX;
			QVector<double> timeTempY;

			QVector<double> timeValveX;
			QVector<double> timeValveY;

			QVector<double> timeVacuumX;
			QVector<double> timeVacuumY;

			//质量时间
			QVector<double> timeQualityX;
			QVector<double> timeQualityY;


			if (model == "HQ-9B")
			{
				if (A < 6.740741)
				{
					A = 6.740741;
				}
				else if (A > 18.296296)
				{
					A = 18.296296;
				}
				if (B < 1.074074)
				{
					B = 1.074074;
				}
				else if (B > 4.9259266)
				{
					B = 4.925926;
				}
				if (C < 20.185185)
				{
					C = 20.185185;
				}
				else if (C > 29.814815)
				{
					C = 29.814815;
				}
				if (D < 21.111111)
				{
					D = 21.111111;
				}
				else if (D > 78.888889)
				{
					D = 78.888889;
				}
				if (E < 53.000000)
				{
					E = 53.000000;
				}
				else if (E > 69.629630)
				{
					E = 69.629630;
				}

				//温度密度曲线
				double densityTempXStart = 50.0;  // 弹体保温温度(50～70)
				double densityTempXEnd = 70.0; // 弹体目标温度（℃）
				double densityTempStep = (densityTempXEnd - densityTempXStart) / 30;

				for (double i = densityTempXStart; i <= densityTempXEnd; i += densityTempStep)
				{							
					auto tempA = (A - 12.509291) / 2.218704;
					auto tempB = (B - 3.01888) / 0.702157;
					auto tempC = (C - 25.076659) / 1.740173;
					auto tempD = (D - 50.037481) / 10.504812;
					auto tempE = (i - 60.043506) / 3.582356;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.oneGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity < 1.0)
					{
						densityTempX.push_back(i);
						densityTempY.push_back(relativeDensity * 100.0);
					}				
				}


				///阀门开度密度曲线
				double densityValveXStart = 13.0;  // 阀门开度(13～39)
				double densityValveXEnd = 39.0; // 阀门开度(13～39)
				double densityValveStep = (densityValveXEnd - densityValveXStart) / 30;
				for (double i = densityValveXStart; i <= densityValveXEnd; i += densityValveStep) {

					auto tempA = ((i / 2.0) - 12.509291) / 2.218704;
					auto tempB = (B - 3.01888) / 0.702157;
					auto tempC = (C - 25.076659) / 1.740173;
					auto tempD = (D - 50.037481) / 10.504812;
					auto tempE = (E - 60.043506) / 3.582356;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.oneGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity < 1.0)
					{
						densityValveX.push_back(i);
						densityValveY.push_back(relativeDensity * 100.0);
					}				
				}

				//真空度密度曲线
				double densityVacuumXStart = 20.0;  // 真空度(20～80)
				double densityVacuumXEnd = 80.0; // 真空度(20～80)
				double densityVacuumStep = (densityVacuumXEnd - densityVacuumXStart) / 30;
				for (double i = densityVacuumXStart; i <= densityVacuumXEnd; i += densityVacuumStep) {

					auto tempA = (A - 12.509291) / 2.218704;
					auto tempB = (B - 3.01888) / 0.702157;
					auto tempC = (C - 25.076659) / 1.740173;
					auto tempD = (i - 50.037481) / 10.504812;
					auto tempE = (E - 60.043506) / 3.582356;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.oneGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity < 1.0)
					{
						densityVacuumX.push_back(i / 1000.0);
						densityVacuumY.push_back(relativeDensity * 100.0);
					}			
				}


				//温度注药时间曲线
				double timeTempXStart = 50.0;  // 弹体保温温度(50～70)
				double timeTempXEnd = 70.0; // 弹体目标温度（℃）
				double timeTempStep = (timeTempXEnd - timeTempXStart) / 30;

				for (double i = timeTempXStart; i <= timeTempXEnd; i += timeTempStep) {

					auto tempA = (A - 12.518519) / 2.177802;
					auto tempB = (B - 2.999897) / 0.725859;
					auto tempC = (C - 25.001792) / 1.8102640;
					auto tempD = (D - 50.000000) / 10.889012;
					auto tempE = (i - 60.005435) / 3.621028;
					double injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.oneInjectionTimeCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					injectionTimeValue = injectionTimeValue * 21.33;
					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					timeTempX.push_back(i);
					timeTempY.push_back(injectionTimeValue);

				}


				///阀门开度注药时间曲线
				double timeValveXStart = 13.0;  // 阀门开度(13～39)
				double timeValveXEnd = 39.0; // 阀门开度(13～39)
				double timeValveStep = (timeValveXEnd - timeValveXStart) / 30;
				for (double i = timeValveXStart; i <= timeValveXEnd; i += timeValveStep) {

					auto tempA = ((i / 2.0) - 12.518519) / 2.177802;
					auto tempB = (B - 2.999897) / 0.725859;
					auto tempC = (C - 25.001792) / 1.8102640;
					auto tempD = (D - 50.000000) / 10.889012;
					auto tempE = (E - 60.005435) / 3.621028;
					double injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.oneInjectionTimeCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					injectionTimeValue = injectionTimeValue * 21.33;
					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					timeValveX.push_back(i);
					timeValveY.push_back(injectionTimeValue);

				}

				//真空度注药时间曲线
				double timeVacuumXStart = 20.0;  // 真空度(20～80)
				double timeVacuumXEnd = 80.0; // 真空度(20～80)
				double timeVacuumStep = (timeVacuumXEnd - timeVacuumXStart) / 30;
				for (double i = timeVacuumXStart; i <= timeVacuumXEnd; i += timeVacuumStep) {

					auto tempA = (A - 12.518519) / 2.177802;
					auto tempB = (B - 2.999897) / 0.725859;
					auto tempC = (C - 25.001792) / 1.8102640;
					auto tempD = (i - 50.000000) / 10.889012;
					auto tempE = (E - 60.005435) / 3.621028;
					double injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.oneInjectionTimeCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					injectionTimeValue = injectionTimeValue * 21.33;
					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					timeVacuumX.push_back(i / 1000.0);
					timeVacuumY.push_back(injectionTimeValue);

				}
			}
			else if (model == "YJ-20")
			{
				if (A < 6.500000)
				{
					A = 6.500000;
				}
				else if (A > 19.500000)
				{
					A = 19.500000;
				}
				if (B < 1.000000)
				{
					B = 1.000000;
				}
				else if (B > 5.000000)
				{
					B = 5.000000;
				}
				if (C < 20.000000)
				{
					C = 20.000000;
				}
				else if (C > 30.000000)
				{
					C = 30.000000;
				}
				if (D < 20.000000)
				{
					D = 20.000000;
				}
				else if (D > 80.000000)
				{
					D = 80.000000;
				}
				if (E < 50.000000)
				{
					E = 50.000000;
				}
				else if (E > 70.000000)
				{
					E = 70.000000;
				}
				//温度密度曲线
				double densityTempXStart = 50.0;  // 弹体保温温度(50～70)
				double densityTempXEnd = 70.0; // 弹体目标温度（℃）
				double densityTempStep = (densityTempXEnd - densityTempXStart) / 30;

				for (double i = densityTempXStart; i <= densityTempXEnd; i += densityTempStep) {

					auto tempA = (A - 13.000000) / 2.44883;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (D - 50.000000) / 11.302298;
					auto tempE = (i - 60.000000) / 3.767433;

					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.twoGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity < 1.0)
					{
						densityTempX.push_back(i);
						densityTempY.push_back(relativeDensity * 100.0);
					}
					

				}


				///阀门开度密度曲线
				double densityValveXStart = 13.0;  // 阀门开度(13～39)
				double densityValveXEnd = 39.0; // 阀门开度(5～39)
				double densityValveStep = (densityValveXEnd - densityValveXStart) / 30;
				for (double i = densityValveXStart; i <= densityValveXEnd; i += densityValveStep) {

					auto tempA = ((i / 2.0) - 13.000000) / 2.44883;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (D - 50.000000) / 11.302298;
					auto tempE = (E - 60.000000) / 3.767433;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.twoGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity < 1.0)
					{
						densityValveX.push_back(i);
						densityValveY.push_back(relativeDensity * 100.0);
					}
					

				}

				//真空度密度曲线
				double densityVacuumXStart = 20.0;  // 真空度(20～80)
				double densityVacuumXEnd = 80.0; // 真空度(20～80)
				double densityVacuumStep = (densityVacuumXEnd - densityVacuumXStart) / 30;
				for (double i = densityVacuumXStart; i <= densityVacuumXEnd; i += densityVacuumStep) {

					auto tempA = (A - 13.000000) / 2.44883;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (i - 50.000000) / 11.302298;
					auto tempE = (E - 60.000000) / 3.767433;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.twoGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity < 1.0)
					{
						densityVacuumX.push_back(i / 1000.0);
						densityVacuumY.push_back(relativeDensity * 100.0);
					}				
				}


				//温度注药时间曲线
				double timeTempXStart = 50.0;  // 弹体保温温度(50～70)
				double timeTempXEnd = 70.0; // 弹体目标温度（℃）
				double timeTempStep = (timeTempXEnd - timeTempXStart) / 30;

				for (double i = timeTempXStart; i <= timeTempXEnd; i += timeTempStep) {

					auto tempA = (A - 13.000000) / 2.44883;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (D - 50.000000) / 11.302298;
					auto tempE = (i - 60.000000) / 3.767433;
					double injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.twoInjectionTimeCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					injectionTimeValue = injectionTimeValue * 26.33;
					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					timeTempX.push_back(i);
					timeTempY.push_back(injectionTimeValue);

				}


				///阀门开度注药时间曲线
				double timeValveXStart = 13.0;  // 阀门开度(13～39)
				double timeValveXEnd = 39.0; // 阀门开度(5～39)
				double timeValveStep = (timeValveXEnd - timeValveXStart) / 30;
				for (double i = timeValveXStart; i <= timeValveXEnd; i += timeValveStep) {

					auto tempA = ((i / 2.0) - 13.000000) / 2.44883;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (D - 50.000000) / 11.302298;
					auto tempE = (E - 60.000000) / 3.767433;
					double injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.twoInjectionTimeCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					injectionTimeValue = injectionTimeValue * 26.33;
					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					timeValveX.push_back(i);
					timeValveY.push_back(injectionTimeValue);

				}

				//真空度注药时间曲线
				double timeVacuumXStart = 20.0;  // 真空度(20～80)
				double timeVacuumXEnd = 80.0; // 真空度(20～80)
				double timeVacuumStep = (timeVacuumXEnd - timeVacuumXStart) / 30;
				for (double i = timeVacuumXStart; i <= timeVacuumXEnd; i += timeVacuumStep) {

					auto tempA = (A - 13.000000) / 2.44883;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (i - 50.000000) / 11.302298;
					auto tempE = (E - 60.000000) / 3.767433;
					double injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.twoInjectionTimeCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					injectionTimeValue = injectionTimeValue * 26.33;
					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					timeVacuumX.push_back(i / 1000.0);
					timeVacuumY.push_back(injectionTimeValue);

				}
			}
			else if (model == "YJ-91A")
			{
				if (A < 6.500000)
				{
					A = 6.500000;
				}
				else if (A > 18.777778)
				{
					A = 18.777778;
				}
				if (B < 1.000000)
				{
					B = 1.000000;
				}
				else if (B > 5.000000)
				{
					B = 5.000000;
				}
				if (C < 20.000000)
				{
					C = 20.000000;
				}
				else if (C > 30.000000)
				{
					C = 30.000000;
				}
				if (D < 20.000000)
				{
					D = 20.000000;
				}
				else if (D > 80.000000)
				{
					D = 80.000000;
				}
				if (E < 50.000000)
				{
					E = 50.000000;
				}
				else if (E > 70.000000)
				{
					E = 70.000000;
				}
				//温度密度曲线
				double densityTempXStart = 50.0;  // 弹体保温温度(50～70)
				double densityTempXEnd = 70.0; // 弹体目标温度（℃）
				double densityTempStep = (densityTempXEnd - densityTempXStart) / 30;

				for (double i = densityTempXStart; i <= densityTempXEnd; i += densityTempStep) {

					auto tempA = (A - 12.638889) / 2.312785;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (D - 50.000000) / 11.302298;
					auto tempE = (i - 60.000000) / 3.767433;

					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.threeGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity < 1.0)
					{
						densityTempX.push_back(i);
						densityTempY.push_back(relativeDensity * 100.0);
					}
					

				}


				///阀门开度密度曲线
				double densityValveXStart = 13.0;  // 阀门开度(13～39)
				double densityValveXEnd = 39.0; // 阀门开度(39～39)
				double densityValveStep = (densityValveXEnd - densityValveXStart) / 30;
				for (double i = densityValveXStart; i <= densityValveXEnd; i += densityValveStep) {

					auto tempA = ((i / 2.0) - 12.638889) / 2.312785;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (D - 50.000000) / 11.302298;
					auto tempE = (E - 60.000000) / 3.767433;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.threeGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity < 1.0)
					{
						densityValveX.push_back(i);
						densityValveY.push_back(relativeDensity * 100.0);
					}
					

				}

				//真空度密度曲线
				double densityVacuumXStart = 20.0;  // 真空度(20～80)
				double densityVacuumXEnd = 80.0; // 真空度(20～80)
				double densityVacuumStep = (densityVacuumXEnd - densityVacuumXStart) / 30;
				for (double i = densityVacuumXStart; i <= densityVacuumXEnd; i += densityVacuumStep) {

					auto tempA = (A - 12.638889) / 2.312785;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (i - 50.000000) / 11.302298;
					auto tempE = (E - 60.000000) / 3.767433;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.threeGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity < 1.0)
					{
						densityVacuumX.push_back(i / 1000.0);
						densityVacuumY.push_back(relativeDensity * 100.0);
					}
					

				}


				//温度注药时间曲线
				double timeTempXStart = 50.0;  // 弹体保温温度(50～70)
				double timeTempXEnd = 70.0; // 弹体目标温度（℃）
				double timeTempStep = (timeTempXEnd - timeTempXStart) / 30;

				for (double i = timeTempXStart; i <= timeTempXEnd; i += timeTempStep) {

					auto tempA = (A - 12.638889) / 2.312785;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (D - 50.000000) / 11.302298;
					auto tempE = (i - 60.000000) / 3.767433;
					double injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.threeInjectionTimeCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					injectionTimeValue = injectionTimeValue * 26.33;
					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					timeTempX.push_back(i);
					timeTempY.push_back(injectionTimeValue);

				}


				///阀门开度注药时间曲线
				double timeValveXStart = 13.0;  // 阀门开度(13～39)
				double timeValveXEnd = 39.0; // 阀门开度(13～39)
				double timeValveStep = (timeValveXEnd - timeValveXStart) / 30;
				for (double i = timeValveXStart; i <= timeValveXEnd; i += timeValveStep) {

					auto tempA = ((i / 2.0) - 12.638889) / 2.312785;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (D - 50.000000) / 11.302298;
					auto tempE = (E - 60.000000) / 3.767433;
					double injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.threeInjectionTimeCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					injectionTimeValue = injectionTimeValue * 26.33;
					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					timeValveX.push_back(i);
					timeValveY.push_back(injectionTimeValue);

				}

				//真空度注药时间曲线
				double timeVacuumXStart = 20.0;  // 真空度(20～80)
				double timeVacuumXEnd = 80.0; // 真空度(20～80)
				double timeVacuumStep = (timeVacuumXEnd - timeVacuumXStart) / 30;
				for (double i = timeVacuumXStart; i <= timeVacuumXEnd; i += timeVacuumStep)
				{
					auto tempA = (A - 12.638889) / 2.312785;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (i - 50.000000) / 11.302298;
					auto tempE = (E - 60.000000) / 3.767433;
					double injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.threeInjectionTimeCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					injectionTimeValue = injectionTimeValue * 26.33;
					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					timeVacuumX.push_back(i / 1000.0);
					timeVacuumY.push_back(injectionTimeValue);

				}
			}
			else if (model == "CJ-20A")
			{
				if (A < 8.500000)
				{
					A = 8.500000;
				}
				else if (A > 18.296296)
				{
					A = 18.296296;
				}
				if (B < 1.000000)
				{
					B = 1.000000;
				}
				else if (B > 5.000000)
				{
					B = 5.000000;
				}
				if (C < 20.000000)
				{
					C = 20.000000;
				}
				else if (C > 30.000000)
				{
					C = 30.000000;
				}
				if (D < 20.000000)
				{
					D = 20.000000;
				}
				else if (D > 80.000000)
				{
					D = 80.000000;
				}
				if (E < 50.000000)
				{
					E = 50.000000;
				}
				else if (E > 70.000000)
				{
					E = 70.000000;
				}
				//温度密度曲线
				double densityTempXStart = 50.0;  // 弹体保温温度(50～70)
				double densityTempXEnd = 70.0; // 弹体目标温度（℃）
				double densityTempStep = (densityTempXEnd - densityTempXStart) / 30;

				for (double i = densityTempXStart; i <= densityTempXEnd; i += densityTempStep) {

					auto tempA = (A - 13.398148) / 1.845344;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (D - 50.000000) / 11.302298;
					auto tempE = (i - 60.000000) / 3.767433;

					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.fourGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity < 1.0)
					{
						densityTempX.push_back(i);
						densityTempY.push_back(relativeDensity * 100.0);
					}
					

				}


				///阀门开度密度曲线
				double densityValveXStart = 13.0;  // 阀门开度(13～39)
				double densityValveXEnd = 39.0; // 阀门开度(39～39)
				double densityValveStep = (densityValveXEnd - densityValveXStart) / 30;
				for (double i = densityValveXStart; i <= densityValveXEnd; i += densityValveStep) {

					auto tempA = ((i / 2.0) - 13.398148) / 1.845344;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (D - 50.000000) / 11.302298;
					auto tempE = (E - 60.000000) / 3.767433;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.fourGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity < 1.0)
					{
						densityValveX.push_back(i);
						densityValveY.push_back(relativeDensity * 100.0);
					}
					

				}

				//真空度密度曲线
				double densityVacuumXStart = 20.0;  // 真空度(20～80)
				double densityVacuumXEnd = 80.0; // 真空度(20～80)
				double densityVacuumStep = (densityVacuumXEnd - densityVacuumXStart) / 30;
				for (double i = densityVacuumXStart; i <= densityVacuumXEnd; i += densityVacuumStep) {

					auto tempA = (A - 8.500000) / 9.796296;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (i - 20.000000) / 60.000000;
					auto tempE = (E - 50.000000) / 20.000000;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.fourGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					double density = ins->GetSteelPropertyInfo().density;
					double gas = 1.205 * gasRateValue; // 气体质量
					double solid = density * (1 - gasRateValue);
					double relativeDensity = (gas + solid) / density;
					if (relativeDensity < 0.0)
					{
						relativeDensity = 0;
					}
					if (relativeDensity < 1.0)
					{
						densityVacuumX.push_back(i / 1000.0);
						densityVacuumY.push_back(relativeDensity * 100.0);
					}
					

				}


				//温度注药时间曲线
				double timeTempXStart = 50.0;  // 弹体保温温度(50～70)
				double timeTempXEnd = 70.0; // 弹体目标温度（℃）
				double timeTempStep = (timeTempXEnd - timeTempXStart) / 30;

				for (double i = timeTempXStart; i <= timeTempXEnd; i += timeTempStep) {

					auto tempA = (A - 13.398148) / 1.845344;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (D - 50.000000) / 11.302298;
					auto tempE = (i - 60.000000) / 3.767433;
					double injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.fourInjectionTimeCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					injectionTimeValue = injectionTimeValue * 27.33;
					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					timeTempX.push_back(i);
					timeTempY.push_back(injectionTimeValue);

				}


				///阀门开度注药时间曲线
				double timeValveXStart = 13.0;  // 阀门开度(13～39)
				double timeValveXEnd = 39.0; // 阀门开度(5～39)
				double timeValveStep = (timeValveXEnd - timeValveXStart) / 30;
				for (double i = timeValveXStart; i <= timeValveXEnd; i += timeValveStep) {

					auto tempA = ((i / 2.0) - 13.398148) / 1.845344;
					auto tempB = (B - 3.000000) / 0.753487;
					auto tempC = (C - 25.000000) / 1.883716;
					auto tempD = (D - 50.000000) / 11.302298;
					auto tempE = (E - 60.000000) / 3.767433;
					double injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.fourInjectionTimeCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					injectionTimeValue = injectionTimeValue * 27.33;
					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					timeValveX.push_back(i);
					timeValveY.push_back(injectionTimeValue);
					 
				}

				//真空度注药时间曲线
				double timeVacuumXStart = 20.0;  // 真空度(20～80)
				double timeVacuumXEnd = 80.0; // 真空度(20～80)
				double timeVacuumStep = (timeVacuumXEnd - timeVacuumXStart) / 30;
				for (double i = timeVacuumXStart; i <= timeVacuumXEnd; i += timeVacuumStep)
				{
					auto tempA = (A - 8.500000) / 9.796296;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (i - 20.000000) / 60.000000;
					auto tempE = (E - 50.000000) / 20.000000;
					double injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.fourInjectionTimeCalculateFormula, tempA, tempB, tempC, tempD, tempE);
					injectionTimeValue = injectionTimeValue * 27.33;
					if (injectionTimeValue < 0.0)
					{
						injectionTimeValue = 0;
					}
					timeVacuumX.push_back(i / 1000.0);
					timeVacuumY.push_back(injectionTimeValue);

				}
			}

			// 质量温度
			double injectionTime = 0.0;
			if (m_tableWidget->item(8, 2)->text() != "")
			{
				injectionTime = m_tableWidget->item(8, 2)->text().toDouble();
			}
			double quality = 64;
			if (model == "HQ-9B")
			{
				quality = 64;
			}
			else if (model == "YJ-20")
			{
				quality = 80;
			}
			else if (model == "YJ-91A")
			{
				quality = 79;
			}
			else 
			{
				quality = 82;
			}
			double timeQualityXStart = 0.0;  
			double timeQualityXEnd = injectionTime;
			double timeQualityStep = (timeQualityXEnd - timeQualityXStart) / 30;
			for (double i = timeQualityXStart; i <= timeQualityXEnd; i += timeQualityStep)
			{
				timeQualityX.push_back(i);
				timeQualityY.push_back(quality/30*i);
			}

			auto inForwardDensityTempWid = gfParent->GetInForwardDensityTempWid();
			auto inForwardDensityValveWid = gfParent->GetInForwardDensityValveWid();
			auto inForwardDensityVacuumWid = gfParent->GetInForwardDensityVacuumWid();
			auto inForwardTimeTempWid = gfParent->GetInForwardTimeTempWid();
			auto inForwardTimeValveWid = gfParent->GetInForwardTimeValveWid();
			auto inForwardTimeVacuumWid = gfParent->GetInForwardTimeVacuumWid();
			auto inForwardMassTimeWid = gfParent->GetInForwardMassTimeWid();

			inForwardDensityTempWid->AddDataPoint(densityTempX, densityTempY);
			inForwardDensityValveWid->AddDataPoint(densityValveX, densityValveY);
			inForwardDensityVacuumWid->AddDataPoint(densityVacuumX, densityVacuumY);
			inForwardTimeTempWid->AddDataPoint(timeTempX, timeTempY);
			inForwardTimeValveWid->AddDataPoint(timeValveX, timeValveY);
			inForwardTimeVacuumWid->AddDataPoint(timeVacuumX, timeVacuumY);
			inForwardMassTimeWid->AddDataPoint(timeQualityX, timeQualityY);

			auto inForwardPropertyInfo = ins->GetInForwardPropertyInfo();
			inForwardPropertyInfo.densityTempX = densityTempX;
			inForwardPropertyInfo.densityTempY = densityTempY;

			inForwardPropertyInfo.densityValveX = densityValveX;
			inForwardPropertyInfo.densityValveY = densityValveY;

			inForwardPropertyInfo.densityVacuumX = densityVacuumX;
			inForwardPropertyInfo.densityVacuumY = densityVacuumY;

			inForwardPropertyInfo.timeTempX = timeTempX;
			inForwardPropertyInfo.timeTempY = timeTempY;

			inForwardPropertyInfo.timeValveX = timeValveX;
			inForwardPropertyInfo.timeValveY = timeValveY;

			inForwardPropertyInfo.timeVacuumX = timeVacuumX;
			inForwardPropertyInfo.timeVacuumY = timeVacuumY;

			inForwardPropertyInfo.timeQualityX = timeQualityX;
			inForwardPropertyInfo.timeQualityY = timeQualityY;
			ins->SetInForwardPropertyInfo(inForwardPropertyInfo);

			break;
		}
		else
		{
			parent = parent->parentWidget();
		}
	}
}
