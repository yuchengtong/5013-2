#pragma once
#pragma execution_character_set("utf-8")
#include "InReverseOptimizationPropertyWidget.h"
#include "../ProgressDialog.h"
#include "../ReverseOptimizationWorker.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QDir>
#include <QDialog>
#include <QDateTime>
#include <QApplication>
#include "../GFTreeModelWidget.h"
#include "../GFImportModelWidget.h"
#include "xlsxdocument.h"

#include "ModelDataManager.h"


//计算
double inReverseCalculateForm(const QString& formula,
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


InReverseOptimizationPropertyWidget::InReverseOptimizationPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
}

void InReverseOptimizationPropertyWidget::initWidget()
{
	QVBoxLayout* vlayout = new QVBoxLayout();
	vlayout->setContentsMargins(0, 0, 0, 0);
	{
		m_tableWidget = new QTableWidget(this);
		m_tableWidget->setRowCount(10);
		m_tableWidget->setColumnCount(5);
		// 隐藏表头
		m_tableWidget->horizontalHeader()->setVisible(false);
		m_tableWidget->verticalHeader()->setVisible(false);

		// 设置固定宽度
		m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
		m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
		m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
		m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
		m_tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
		m_tableWidget->horizontalHeader()->resizeSection(0, 25);
		m_tableWidget->horizontalHeader()->resizeSection(1, 150);
		m_tableWidget->horizontalHeader()->resizeSection(3, 50);
		m_tableWidget->horizontalHeader()->resizeSection(4, 25);

		QStringList cols = { "逆向寻优"," ",  "1", "2", "3","4"," ","1","2","3" };
		for (int row = 0; row < cols.size(); ++row)
		{
			QTableWidgetItem* serialItem = new QTableWidgetItem(cols[row]);
			if (row == 0)
			{
				serialItem->setTextAlignment(Qt::AlignLeft);
			}
			serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 0, serialItem);
		}

		QStringList labels = { "","工艺输入参数",  "弹体保温温度(50～70)", "药液浇注温度", "阀门开度(13～39)","真空度(0.02～0.08)","工艺输出参数","相对密度","弹体注药时间","弹体温度云图与温升曲线" };
		for (int row = 0; row < labels.size(); ++row)
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
			labelItem->setTextAlignment(Qt::AlignCenter); // 文本居中	
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 1, labelItem);
		}

		// 计算按钮
		QPushButton* m_calButton = new QPushButton("计算");
		QPushButton* m_resetButton = new QPushButton("默认");
		m_tableWidget->setCellWidget(0, 2, m_calButton);
		m_tableWidget->setCellWidget(1, 2, m_resetButton);
		connect(m_calButton, &QPushButton::clicked, this, &InReverseOptimizationPropertyWidget::calculate);
		connect(m_resetButton, &QPushButton::clicked, this, &InReverseOptimizationPropertyWidget::reset);
		
		m_insulationTemperatureValueItem = new QTableWidgetItem(m_insulationTemperatureValue);
		m_insulationTemperatureValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		m_insulationTemperatureValueItem->setBackground(QBrush(QColor(255, 254, 195)));

		m_pouringTemperatureValueItem = new QTableWidgetItem(m_pouringTemperatureValue);
		m_pouringTemperatureValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		m_pouringTemperatureValueItem->setFlags(m_pouringTemperatureValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_pouringTemperatureValueItem->setBackground(QBrush(QColor(230, 230, 230)));

		m_valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
		m_valveOpeningValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		m_valveOpeningValueItem->setBackground(QBrush(QColor(2, 253, 254)));

		m_vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
		m_vacuumDegreeValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		m_vacuumDegreeValueItem->setBackground(QBrush(QColor(2, 253, 254)));

		m_relativeDensityValueItem = new QTableWidgetItem(m_relativeDensityValue);
		m_relativeDensityValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		m_relativeDensityValueItem->setBackground(QBrush(QColor(255, 254, 195)));

		m_injectionTimeValueItem = new QTableWidgetItem(m_injectionTimeValue);
		m_injectionTimeValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		m_injectionTimeValueItem->setBackground(QBrush(QColor(255, 254, 195)));

		m_tableWidget->setItem(2, 2, m_insulationTemperatureValueItem);
		m_tableWidget->setItem(3, 2, m_pouringTemperatureValueItem);
		m_tableWidget->setItem(4, 2, m_valveOpeningValueItem);
		m_tableWidget->setItem(5, 2, m_vacuumDegreeValueItem);
		m_tableWidget->setItem(7, 2, m_relativeDensityValueItem);
		m_tableWidget->setItem(8, 2, m_injectionTimeValueItem);


		connect(m_tableWidget, &QTableWidget::itemChanged, this, [this](QTableWidgetItem* item) {

			if (item == m_insulationTemperatureValueItem)
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

			if (item == m_pouringTemperatureValueItem)
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

			if (item == m_valveOpeningValueItem)
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

			if (item == m_vacuumDegreeValueItem)
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

			if (item == m_relativeDensityValueItem)
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

			if (item == m_injectionTimeValueItem)
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

		// 显示按钮
		m_viewButton = new QPushButton("显示");
		m_tableWidget->setCellWidget(9, 2, m_viewButton);

		// 设置列宽度
		QTableWidgetItem* colimnItem = m_tableWidget->item(9, 1);
		int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
		m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());

		QStringList unitLabels = { " "," ","℃", "℃", "mm","MPa"," ","%","s"," " };
		for (int row = 0; row < unitLabels.size(); ++row)
		{
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
			m_insulationTempeRadioBtn = new QRadioButton();
			m_pouringTempeRadioBtn = new QRadioButton();
			m_valveOpeningRadioBtn = new QRadioButton();
			m_valveOpeningRadioBtn->setChecked(true);
			m_vacuumDegreeRadioBtn = new QRadioButton();

			//m_tableWidget->setCellWidget(2, 4, createCenteredRadioWidget(m_insulationTempeRadioBtn));
			//m_tableWidget->setCellWidget(3, 4, createCenteredRadioWidget(m_pouringTempeRadioBtn));
			m_tableWidget->setCellWidget(4, 4, createCenteredRadioWidget(m_valveOpeningRadioBtn));
			m_tableWidget->setCellWidget(5, 4, createCenteredRadioWidget(m_vacuumDegreeRadioBtn));

			firRadioButtonGroup->addButton(m_insulationTempeRadioBtn);
			firRadioButtonGroup->addButton(m_pouringTempeRadioBtn);
			firRadioButtonGroup->addButton(m_valveOpeningRadioBtn);
			firRadioButtonGroup->addButton(m_vacuumDegreeRadioBtn);
		}
		firRadioButtonGroup->setExclusive(true);

		auto secRadioButtonGroup = new QButtonGroup();
		{
			m_relativeDensityRadioBtn = new QRadioButton();
			m_relativeDensityRadioBtn->setChecked(true);
			m_injectionTimeRadioBtn = new QRadioButton();

			m_tableWidget->setCellWidget(7, 4, createCenteredRadioWidget(m_relativeDensityRadioBtn));
			m_tableWidget->setCellWidget(8, 4, createCenteredRadioWidget(m_injectionTimeRadioBtn));

			secRadioButtonGroup->addButton(m_relativeDensityRadioBtn);
			secRadioButtonGroup->addButton(m_injectionTimeRadioBtn);
		}
		secRadioButtonGroup->setExclusive(true);

		// 合并单元格
		m_tableWidget->setSpan(0, 0, 1, 2);
		m_tableWidget->setSpan(0, 2, 1, 3);
		m_tableWidget->setSpan(1, 2, 1, 3);
		m_tableWidget->setSpan(9, 2, 1, 3);
	}
	vlayout->addWidget(m_tableWidget);
	setLayout(vlayout);

	// 将第0行0列的单元格文本字体加粗
	QTableWidgetItem* headerItem = m_tableWidget->item(0, 0);
	QFont font = headerItem->font();
	font.setBold(true);
	headerItem->setFont(font);
	
	//文本左对齐
	for (int row = 0; row < m_tableWidget->rowCount(); ++row) 
	{
		for (int col = 0; col < m_tableWidget->columnCount(); ++col) 
		{
			QTableWidgetItem* item = m_tableWidget->item(row, col);
			if (item)
			{
				if (col == 0)
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
	for (int row = 0; row < m_tableWidget->rowCount(); ++row) 
	{
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


void InReverseOptimizationPropertyWidget::calculate()
{

	auto ins = ModelDataManager::GetInstance();
	auto modelGeometryInfo = ins->GetModelGeometryInfo();
	auto steelPropertyInfo = ins->GetSteelPropertyInfo();
	auto propellantPropertyInfo = ins->GetPropellantPropertyInfo();
	auto gelatinPropertyInfo = ins->GetGelatinPropertyInfo();
	auto calculationPropertyInfo = ins->GetCalculationPropertyInfo();

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

	auto valveOpeningBool = m_valveOpeningRadioBtn->isChecked(); // 阀门开度
	auto vacuumDegreeBool = m_vacuumDegreeRadioBtn->isChecked(); // 真空度

	auto relativeDensityBool = m_relativeDensityRadioBtn->isChecked(); // 相对密度
	auto injectionTimeBool = m_injectionTimeRadioBtn->isChecked(); // 弹体注药时间

	if (relativeDensityBool)
	{

		if (m_relativeDensityValue == "")
		{
			QMessageBox::information(this, "提示", "相对密度不能为空！");
			return;
		}
	}
	else
	{
		if (m_injectionTimeValue == "")
		{
			QMessageBox::information(this, "提示", "弹体注药时间不能为空！");
			return;
		}
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
			QString text = timeStr + "[信息]>开始注药工艺逆向寻优";
			textEdit->appendPlainText(text);
			logWidget->update();

			QApplication::processEvents();




			// 创建进度对话框
			ProgressDialog* progressDialog = new ProgressDialog("注药工艺工程逆向寻优", this);
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

					auto A = m_valveOpeningValue.toDouble() / 2.0; // 阀门开度（mm）
					auto B = modelGeometryInfo.gasketLayerThickness; // 胶层厚度(mm)
					auto C = modelGeometryInfo.shellThickness; // 壳体厚度 (mm)
					auto D = m_vacuumDegreeValue.toDouble() * 1000; // 真空度(KPa)
					auto E = m_insulationTemperatureValue.toDouble(); // 保温温度（℃）

					// 获取模型类型
					QString model = ModelDataManager::GetInstance()->GetModelGeometryInfo().model;
					m_solver = new InReverseFormulaSolver(this);

					double density = ins->GetSteelPropertyInfo().density;
					QString formula = calculationPropertyInfo.oneGasRateCalculateFormula; // 方程

					QMap<char, double> known; // 计算入参值


					if (model == "HQ-9B")
					{
						

						double target = 0;

						if (relativeDensityBool)
						{
							///////////////////////////// 已知相对密度/////////////////////////////////////
							known['A'] = (A - 12.509291) / 2.218704;
							known['B'] = (B - 3.01888) / 0.702157;
							known['C'] = (C - 25.076659) / 1.740173;
							known['D'] = (D - 50.037481) / 10.504812;
							known['E'] = (E - 60.043506) / 3.582356;
							// 相对密度转为气含率
							auto relativeDensity = m_relativeDensityValue.toDouble();
							auto gasRateValue = ((1 - relativeDensity / 100) * density) / (density - 1.205);
							QString formula = calculationPropertyInfo.oneGasRateCalculateFormula; // 方程
							target = gasRateValue;

							connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
								progressDialog->SetProgress(100);
								if (resList.empty()) {
									QMessageBox::warning(this, "警告", "无解");
									return;
								}
								for (double v : resList) {
									if (valveOpeningBool)
									{
										// 对阀门开度寻优
										double value = v;
											value = 2.218704 * v + 12.509291;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value * 2.0, 'f', 2);
											m_valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
											m_valveOpeningValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, m_valveOpeningValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (10.504812 * v + 50.037481) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											m_vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
											m_vacuumDegreeValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, m_vacuumDegreeValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}

								}
								QMessageBox::warning(this, "提示", "解超出范围");
								});

							if (valveOpeningBool)
							{
								// 对阀门开度寻优
								known.remove('A');
								m_solver->solveReverse(formula, target, known, 'A');
							}
							else
							{
								// 对真空度寻优
								known.remove('D');
								m_solver->solveReverse(formula, target, known, 'D');
							}
						}
						else
						{
							known['A'] = (A - 12.518519) / 2.177802;
							known['B'] = (B - 2.999897) / 0.725859;
							known['C'] = (C - 25.001792) / 1.8102640;
							known['D'] = (D - 50.000000) / 10.889012;
							known['E'] = (E - 60.005435) / 3.621028;

							//////////////////////////// 已知弹体注药时间//////////////////////////////////////////

							auto injectionTimeValue = m_injectionTimeValue.toDouble() / 21.33;
							formula = calculationPropertyInfo.oneInjectionTimeCalculateFormula;
							target = injectionTimeValue;

							connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
								progressDialog->SetProgress(100);
								if (resList.empty()) {
									QMessageBox::warning(this, "警告", "无解");
									return;
								}
								for (double v : resList) {
									if (valveOpeningBool)
									{
										// 对阀门开度寻优
										double value = v;
										value = 2.177802 * v + 12.518519;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value * 2.0, 'f', 2);
											m_valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
											m_valveOpeningValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, m_valveOpeningValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (10.889012 * v + 50.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											m_vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
											m_vacuumDegreeValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, m_vacuumDegreeValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}

								}
								QMessageBox::warning(this, "提示", "解超出范围");
								});

							if (valveOpeningBool)
							{
								// 对阀门开度寻优
								known.remove('A');
								m_solver->solveReverse(formula, target, known, 'A');
							}
							else
							{
								// 对真空度寻优
								known.remove('D');
								m_solver->solveReverse(formula, target, known, 'D');
							}
						}
					}
					else if (model == "YJ-20")
					{
						known['A'] = (A - 13.000000) / 2.44883;
						known['B'] = (B - 3.000000) / 0.753487;
						known['C'] = (C - 25.000000) / 1.883716;
						known['D'] = (D - 50.000000) / 11.302298;
						known['E'] = (E - 60.000000) / 3.767433;

						double target = 0;

						if (relativeDensityBool)
						{
							///////////////////////////// 已知相对密度/////////////////////////////////////
							// 相对密度转为气含率
							auto relativeDensity = m_relativeDensityValue.toDouble();
							auto gasRateValue = ((1 - relativeDensity / 100) * density) / (density - 1.205);
							QString formula = calculationPropertyInfo.twoGasRateCalculateFormula; // 方程
							target = gasRateValue;

							connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
								progressDialog->SetProgress(100);
								if (resList.empty()) {
									QMessageBox::warning(this, "警告", "无解");
									return;
								}
								for (double v : resList) {
									if (valveOpeningBool)
									{
										// 对阀门开度寻优
										double value = v;
										value = 2.44883 * v + 13.000000;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value * 2.0, 'f', 2);
											m_valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
											m_valveOpeningValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, m_valveOpeningValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (11.302298 * v + 50.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											m_vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
											m_vacuumDegreeValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, m_vacuumDegreeValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}

								}
								QMessageBox::warning(this, "提示", "解超出范围");
								});

							if (valveOpeningBool)
							{
								// 对阀门开度寻优
								known.remove('A');
								m_solver->solveReverse(formula, target, known, 'A');
							}
							else
							{
								// 对真空度寻优
								known.remove('D');
								m_solver->solveReverse(formula, target, known, 'D');
							}
						}
						else
						{
							//////////////////////////// 已知弹体注药时间//////////////////////////////////////////


							auto injectionTimeValue = m_injectionTimeValue.toDouble() / 26.67;
							formula = calculationPropertyInfo.twoInjectionTimeCalculateFormula;
							target = injectionTimeValue;

							connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
								progressDialog->SetProgress(100);
								if (resList.empty()) {
									QMessageBox::warning(this, "警告", "无解");
									return;
								}
								for (double v : resList) {
									if (valveOpeningBool)
									{
										// 对阀门开度寻优
										double value = v;
										value = 2.44883 * v + 13.000000;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value * 2.0, 'f', 2);
											m_valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
											m_valveOpeningValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, m_valveOpeningValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (11.302298 * v + 50.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											m_vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
											m_vacuumDegreeValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, m_vacuumDegreeValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}

								}
								QMessageBox::warning(this, "提示", "解超出范围");
								});

							if (valveOpeningBool)
							{
								// 对阀门开度寻优
								known.remove('A');
								m_solver->solveReverse(formula, target, known, 'A');
							}
							else
							{
								// 对真空度寻优
								known.remove('D');
								m_solver->solveReverse(formula, target, known, 'D');
							}
						}
					}
					else if (model == "YJ-91A")
					{
						known['A'] = (A - 12.638889) / 2.312785;
						known['B'] = (B - 3.000000) / 0.753487;
						known['C'] = (C - 25.000000) / 1.883716;
						known['D'] = (D - 50.000000) / 11.302298;
						known['E'] = (E - 60.000000) / 3.767433;

						double target = 0;

						if (relativeDensityBool)
						{
							///////////////////////////// 已知相对密度/////////////////////////////////////
							// 相对密度转为气含率
							auto relativeDensity = m_relativeDensityValue.toDouble();
							auto gasRateValue = ((1 - relativeDensity / 100) * density) / (density - 1.205);
							QString formula = calculationPropertyInfo.threeGasRateCalculateFormula; // 方程
							target = gasRateValue;

							connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
								progressDialog->SetProgress(100);
								if (resList.empty()) {
									QMessageBox::warning(this, "警告", "无解");
									return;
								}
								for (double v : resList) {
									if (valveOpeningBool)
									{
										// 对阀门开度寻优
										double value = v;
										value = 2.312785 * v + 12.638889;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value * 2.0, 'f', 2);
											m_valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
											m_valveOpeningValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, m_valveOpeningValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (11.302298 * v + 50.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											m_vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
											m_vacuumDegreeValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, m_vacuumDegreeValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}

								}
								QMessageBox::warning(this, "提示", "解超出范围");
								});

							if (valveOpeningBool)
							{
								// 对阀门开度寻优
								known.remove('A');
								m_solver->solveReverse(formula, target, known, 'A');
							}
							else
							{
								// 对真空度寻优
								known.remove('D');
								m_solver->solveReverse(formula, target, known, 'D');
							}
						}
						else
						{
							//////////////////////////// 已知弹体注药时间//////////////////////////////////////////


							auto injectionTimeValue = m_injectionTimeValue.toDouble() / 27.33;
							formula = calculationPropertyInfo.threeInjectionTimeCalculateFormula;
							target = injectionTimeValue;

							connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
								progressDialog->SetProgress(100);
								if (resList.empty()) {
									QMessageBox::warning(this, "警告", "无解");
									return;
								}
								for (double v : resList) {
									if (valveOpeningBool)
									{
										// 对阀门开度寻优
										double value = v;
										value = 2.312785 * v + 12.638889;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value * 2.0, 'f', 2);
											m_valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
											m_valveOpeningValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, m_valveOpeningValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (11.302298 * v + 50.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											m_vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
											m_vacuumDegreeValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, m_vacuumDegreeValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}

								}
								QMessageBox::warning(this, "提示", "解超出范围");
								});

							if (valveOpeningBool)
							{
								// 对阀门开度寻优
								known.remove('A');
								m_solver->solveReverse(formula, target, known, 'A');
							}
							else
							{
								// 对真空度寻优
								known.remove('D');
								m_solver->solveReverse(formula, target, known, 'D');
							}
						}
					}
					else
					{
						known['A'] = (A - 13.398148) / 1.845344;
						known['B'] = (B - 3.000000) / 0.753487;
						known['C'] = (C - 25.000000) / 1.883716;
						known['D'] = (D - 50.000000) / 11.302298;
						known['E'] = (E - 60.000000) / 3.767433;

						double target = 0;

						if (relativeDensityBool)
						{
							///////////////////////////// 已知相对密度/////////////////////////////////////
							// 相对密度转为气含率
							auto relativeDensity = m_relativeDensityValue.toDouble();
							auto gasRateValue = ((1 - relativeDensity / 100) * density) / (density - 1.205);
							QString formula = calculationPropertyInfo.fourGasRateCalculateFormula; // 方程
							target = gasRateValue;


							connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
								progressDialog->SetProgress(100);
								if (resList.empty()) {
									QMessageBox::warning(this, "警告", "无解");
									return;
								}
								for (double v : resList) {
									if (valveOpeningBool)
									{
										// 对阀门开度寻优
										double value = v;
										value = 1.845344 * v + 13.398148;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value * 2.0, 'f', 2);
											m_valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
											m_valveOpeningValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, m_valveOpeningValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (11.302298 * v + 50.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											m_vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
											m_vacuumDegreeValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, m_vacuumDegreeValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}

								}
								QMessageBox::warning(this, "提示", "解超出范围");
								});

							if (valveOpeningBool)
							{
								known.remove('A');
								// 对阀门开度寻优
								m_solver->solveReverse(formula, target, known, 'A');
							}
							else
							{
								known.remove('D');
								// 对真空度寻优
								m_solver->solveReverse(formula, target, known, 'D');
							}
						}
						else
						{
							//////////////////////////// 已知弹体注药时间//////////////////////////////////////////


							auto injectionTimeValue = m_injectionTimeValue.toDouble() / 21.33;
							formula = calculationPropertyInfo.fourInjectionTimeCalculateFormula;
							target = injectionTimeValue;

							connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
								progressDialog->SetProgress(100);
								if (resList.empty()) {
									QMessageBox::warning(this, "警告", "无解");
									return;
								}
								for (double v : resList) {
									if (valveOpeningBool)
									{
										// 对阀门开度寻优
										double value = v;
										value = 1.845344 * v + 13.398148;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value * 2.0, 'f', 2);
											m_valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
											m_valveOpeningValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, m_valveOpeningValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (11.302298 * v + 50.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											m_vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
											m_vacuumDegreeValueItem->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, m_vacuumDegreeValueItem);
											//QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}

								}
								QMessageBox::warning(this, "提示", "解超出范围");
								});

							if (valveOpeningBool)
							{
								known.remove('A');
								// 对阀门开度寻优
								m_solver->solveReverse(formula, target, known, 'A');
							}
							else
							{
								known.remove('D');
								// 对真空度寻优
								m_solver->solveReverse(formula, target, known, 'D');
							}
						}

					}
					if (!success)
					{
						//QMessageBox::warning(this, "计算失败", msg);
					}
					// 正向计算
					forwardCalculate();

					QString text = timeStr + "[信息]>注药工艺逆向寻优完成";
					textEdit->appendPlainText(text);
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


void InReverseOptimizationPropertyWidget::reset()
{
	m_insulationTemperatureValue = "60";
	m_insulationTemperatureValueItem = new QTableWidgetItem(m_insulationTemperatureValue);
	m_insulationTemperatureValueItem->setBackground(QBrush(QColor(2, 253, 254)));
	m_tableWidget->setItem(2, 2, m_insulationTemperatureValueItem);

	m_valveOpeningValue = "15";
	m_valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
	m_valveOpeningValueItem->setBackground(QBrush(QColor(2, 253, 254)));
	m_tableWidget->setItem(4, 2, m_valveOpeningValueItem);

	m_vacuumDegreeValue = "0.05";
	m_vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
	m_vacuumDegreeValueItem->setBackground(QBrush(QColor(2, 253, 254)));
	m_tableWidget->setItem(5, 2, m_vacuumDegreeValueItem);



	m_relativeDensityValue = "";
	m_relativeDensityValueItem = new QTableWidgetItem(m_relativeDensityValue);
	m_relativeDensityValueItem->setBackground(QBrush(QColor(255, 254, 195)));
	m_tableWidget->setItem(7, 2, m_relativeDensityValueItem);

	m_injectionTimeValue = "";
	m_injectionTimeValueItem = new QTableWidgetItem(m_injectionTimeValue);
	m_injectionTimeValueItem->setBackground(QBrush(QColor(255, 254, 195)));
	m_tableWidget->setItem(8, 2, m_injectionTimeValueItem);
}


void InReverseOptimizationPropertyWidget::forwardCalculate()
{
	auto ins = ModelDataManager::GetInstance();
	auto modelGeometryInfo = ins->GetModelGeometryInfo();
	auto steelPropertyInfo = ins->GetSteelPropertyInfo();
	auto calculationPropertyInfo = ins->GetCalculationPropertyInfo();

	auto A = m_valveOpeningValue.toDouble()/2.0; // 阀门开度（mm）
	auto B = modelGeometryInfo.gasketLayerThickness; // 胶层厚度(mm)
	auto C = modelGeometryInfo.shellThickness; // 壳体厚度 (mm)
	auto D = m_vacuumDegreeValue.toDouble() * 1000; // 真空度(KPa)
	auto E = m_insulationTemperatureValue.toDouble(); // 保温温度（℃）

	
	auto relativeDensityBool = m_relativeDensityRadioBtn->isChecked(); // 相对密度
	auto injectionTimeBool = m_injectionTimeRadioBtn->isChecked(); // 弹体注药时间

	// 获取模型类型
	QString model = ModelDataManager::GetInstance()->GetModelGeometryInfo().model;
	QString formula = "";
	if (model == "HQ-9B" )
	{
		if (!relativeDensityBool)
		{
			A = (A - 12.509291) / 2.218704;
			B = (B - 3.01888) / 0.702157;
			C = (C - 25.076659) / 1.740173;
			D = (D - 50.037481) / 10.504812;
			E = (E - 60.043506) / 3.582356;
			// 气含率
			formula = calculationPropertyInfo.oneGasRateCalculateFormula;
		}
		else
		{
			A = (A - 12.518519) / 2.177802;
			B = (B - 2.999897) / 0.725859;
			C = (C - 25.001792) / 1.8102640;
			D = (D - 50.000000) / 10.889012;
			E = (E - 60.005435) / 3.621028;
			formula = calculationPropertyInfo.oneInjectionTimeCalculateFormula;
		}
	}
	else if (model == "YJ-20")
	{
		A = (A - 13.000000) / 2.44883;
		B = (B - 3.000000) / 0.753487;
		C = (C - 25.000000) / 1.883716;
		D = (D - 50.000000) / 11.302298;
		E = (E - 60.000000) / 3.767433;
		
		if (!relativeDensityBool)
		{
			// 气含率
			formula = calculationPropertyInfo.twoGasRateCalculateFormula;
		}
		else
		{
			formula = calculationPropertyInfo.twoInjectionTimeCalculateFormula;
		}
	}
	else if (model == "YJ-91A")
	{
		A = (A - 12.638889) / 2.312785;
		B = (B - 3.000000) / 0.753487;
		C = (C - 25.000000) / 1.883716;
		D = (D - 50.000000) / 11.302298;
		E = (E - 60.000000) / 3.767433;

		if (!relativeDensityBool)
		{
			// 气含率
			formula = calculationPropertyInfo.threeGasRateCalculateFormula;
		}
		else
		{
			formula = calculationPropertyInfo.threeInjectionTimeCalculateFormula;
		}
	}
	else
	{

		A = (A - 13.398148) / 1.845344;
		B = (B - 3.000000) / 0.753487;
		C = (C - 25.000000) / 1.883716;
		D = (D - 50.000000) / 11.302298;
		E = (E - 60.000000) / 3.767433;

		if (!relativeDensityBool)
		{
			// 气含率
			formula = calculationPropertyInfo.fourGasRateCalculateFormula;
		}
		else
		{
			formula = calculationPropertyInfo.fourInjectionTimeCalculateFormula;
		}
	}

	double value = inReverseCalculateForm(formula, A, B, C, D, E);

	if (!relativeDensityBool)
	{
		// 气含率转相对密度
		double density = ins->GetSteelPropertyInfo().density;
		double gas = 1.205 * value; // 气体质量
		double solid = density * (1 - value);
		double relativeDensity = (gas + solid) / density;

		QString relativeDensityResult = QString::number(relativeDensity * 100, 'f', 2);
		m_relativeDensityValueItem = new QTableWidgetItem(relativeDensityResult);
		m_relativeDensityValueItem->setBackground(QBrush(QColor(255, 254, 195)));
		m_tableWidget->setItem(7, 2, m_relativeDensityValueItem);
		m_relativeDensityValue = relativeDensityResult;
	}
	else
	{
		if (model == "HQ-9B") 
		{ 
			value = value * 21.33;
		}
		else if (model == "YJ-20")
		{
			value = value * 26.67;
		}
		else if (model == "YJ-91A")
		{
			value = value * 26.33;
		}
		else 
		{
			value = value * 27.33;
		}

		QString injectionTimeResult = QString::number(qRound(value));
		m_injectionTimeValueItem = new QTableWidgetItem(injectionTimeResult);
		m_injectionTimeValueItem->setBackground(QBrush(QColor(255, 254, 195)));
		m_tableWidget->setItem(8, 2, m_injectionTimeValueItem);
		m_injectionTimeValue = injectionTimeResult;
	}

	


	

	

}