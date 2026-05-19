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

		QStringList labels = { "","工艺输入参数",  "弹体保温温度(50～70)", "药液浇注温度", "阀门开度(5～39)","真空度(0.02～0.08)","工艺输出参数","相对密度","弹体注药时间","弹体温度云图与温升曲线" };
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
		
		QTableWidgetItem* insulationTemperatureValueItem = new QTableWidgetItem(m_insulationTemperatureValue);
		insulationTemperatureValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		insulationTemperatureValueItem->setBackground(QBrush(QColor(255, 254, 195)));

		QTableWidgetItem* pouringTemperatureValueItem = new QTableWidgetItem(m_pouringTemperatureValue);
		pouringTemperatureValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		pouringTemperatureValueItem->setFlags(pouringTemperatureValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		pouringTemperatureValueItem->setBackground(QBrush(QColor(230, 230, 230)));

		QTableWidgetItem* valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
		valveOpeningValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		valveOpeningValueItem->setBackground(QBrush(QColor(2, 253, 254)));

		QTableWidgetItem* vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
		vacuumDegreeValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		vacuumDegreeValueItem->setBackground(QBrush(QColor(2, 253, 254)));

		QTableWidgetItem* relativeDensityValueItem = new QTableWidgetItem(m_relativeDensityValue);
		relativeDensityValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		relativeDensityValueItem->setBackground(QBrush(QColor(255, 254, 195)));

		QTableWidgetItem* injectionTimeValueItem = new QTableWidgetItem(m_injectionTimeValue);
		injectionTimeValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		injectionTimeValueItem->setBackground(QBrush(QColor(255, 254, 195)));

		m_tableWidget->setItem(2, 2, insulationTemperatureValueItem);
		m_tableWidget->setItem(3, 2, pouringTemperatureValueItem);
		m_tableWidget->setItem(4, 2, valveOpeningValueItem);
		m_tableWidget->setItem(5, 2, vacuumDegreeValueItem);
		m_tableWidget->setItem(7, 2, relativeDensityValueItem);
		m_tableWidget->setItem(8, 2, injectionTimeValueItem);


		connect(m_tableWidget, &QTableWidget::itemChanged, this, [this, insulationTemperatureValueItem, pouringTemperatureValueItem, valveOpeningValueItem, vacuumDegreeValueItem, relativeDensityValueItem, injectionTimeValueItem](QTableWidgetItem* item) {

			if (item == insulationTemperatureValueItem)
			{
				auto text = item->text();
				auto value = text.toDouble();
				if (value >= 0 && value <= 110)
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
				if (value >= 0 && value <= 30)
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
				m_vacuumDegreeValue = text;
			}

			if (item == relativeDensityValueItem)
			{
				auto text = item->text();
				auto value = text.toDouble();
				m_relativeDensityValue = text;
			}

			if (item == injectionTimeValueItem)
			{
				auto text = item->text();
				auto value = text.toDouble();
				m_injectionTimeValue = text;
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

					auto ins = ModelDataManager::GetInstance();
					auto modelGeometryInfo = ins->GetModelGeometryInfo();
					auto steelPropertyInfo = ins->GetSteelPropertyInfo();
					auto calculationPropertyInfo = ins->GetCalculationPropertyInfo();

					auto A = m_valveOpeningValue.toDouble(); // 阀门开度（mm）
					auto B = modelGeometryInfo.gasketLayerThickness; // 胶层厚度(mm)
					auto C = modelGeometryInfo.shellThickness; // 壳体厚度 (mm)
					auto D = m_vacuumDegreeValue.toDouble() * 1000; // 真空度(KPa)
					auto E = m_insulationTemperatureValue.toDouble(); // 保温温度（℃）

					// 获取模型类型
					QString model = "产品一";
					QWidget* parent = parentWidget();
					while (parent) {
						GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
						if (gfParent)
						{
							auto* modelComboBox = gfParent->GetGeomPropertyWidget()->GetModelComboBox();
							model = modelComboBox->currentText();
							break;
						}
						else
						{
							parent = parent->parentWidget();
						}
					}

					

					m_solver = new InReverseFormulaSolver(this);

					double density = ins->GetSteelPropertyInfo().density;
					QString formula = calculationPropertyInfo.oneGasRateCalculateFormula; // 方程

					QMap<char, double> known; // 计算入参值


					if (model == "产品一")
					{
						known['A'] = (A - 6.740741) / 11.555556;
						known['B'] = (B - 1.074074) / 3.851852;
						known['C'] = (C - 20.185185) / 9.629630;
						known['D'] = (D - 21.111111) / 57.777778;
						known['E'] = (E - 50.370370) / 19.259259;

						double target = 0;

						if (relativeDensityBool)
						{
							///////////////////////////// 已知相对密度/////////////////////////////////////
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
										value = 11.555556 * v + 6.740741;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_valveOpeningValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (57.777778 * v + 21.111111) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_vacuumDegreeValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
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
										value = 11.555556 * v + 6.740741;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_valveOpeningValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (57.777778 * v + 21.111111) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_vacuumDegreeValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
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
					else if (model == "产品二")
					{
						known['A'] = (A - 6.500000) / 13.000000;
						known['B'] = (B - 1.000000) / 4.000000;
						known['C'] = (C - 20.000000) / 10.000000;
						known['D'] = (D - 20.000000) / 60.000000;
						known['E'] = (E - 50.000000) / 20.000000;

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
										value = 13.000000 * v + 6.500000;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_valveOpeningValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (60.000000 * v + 20.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_vacuumDegreeValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
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
										value = 13.000000 * v + 6.500000;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_valveOpeningValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (60.000000 * v + 20.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_vacuumDegreeValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
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
					else if (model == "产品三")
					{
						known['A'] = (A - 6.500000) / 12.277778;
						known['B'] = (B - 1.000000) / 4.000000;
						known['C'] = (C - 20.000000) / 10.000000;
						known['D'] = (D - 20.000000) / 60.000000;
						known['E'] = (E - 50.000000) / 20.000000;

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
										value = 2.277778 * v + 6.500000;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_valveOpeningValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (60.000000 * v + 20.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_vacuumDegreeValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
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
										value = 2.277778 * v + 6.500000;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_valveOpeningValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (60.000000 * v + 20.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_vacuumDegreeValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
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
						known['A'] = (A - 8.500000) / 9.796296;
						known['B'] = (B - 1.000000) / 4.000000;
						known['C'] = (C - 20.000000) / 10.000000;
						known['D'] = (D - 20.000000) / 60.000000;
						known['E'] = (E - 50.000000) / 20.000000;

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
										value = 9.796296 * v + 8.500000;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_valveOpeningValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (60.000000 * v + 20.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_vacuumDegreeValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
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
										value = 9.796296 * v + 8.500000;
										if (value >= 0 && value <= 100) {
											m_valveOpeningValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_valveOpeningValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(4, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
											return;
										}
									}
									else
									{
										// 对真空度寻优
										double value = v;
										value = (60.000000 * v + 20.000000) / 1000;

										if (value >= 0 && value <= 100) {
											m_vacuumDegreeValue = QString::number(value, 'f', 2);
											QTableWidgetItem* item = new QTableWidgetItem(m_vacuumDegreeValue);
											item->setBackground(QBrush(QColor(2, 253, 254)));
											m_tableWidget->setItem(5, 2, item);
											QMessageBox::information(this, "成功", "计算完成");
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
						QMessageBox::warning(this, "计算失败", msg);
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
	m_insulationTemperatureValue = "50";
	QTableWidgetItem* insulationTemperatureValueItem = new QTableWidgetItem(m_insulationTemperatureValue);
	insulationTemperatureValueItem->setBackground(QBrush(QColor(2, 253, 254)));
	m_tableWidget->setItem(2, 2, insulationTemperatureValueItem);

	m_valveOpeningValue = "5";
	QTableWidgetItem* valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
	valveOpeningValueItem->setBackground(QBrush(QColor(2, 253, 254)));
	m_tableWidget->setItem(4, 2, valveOpeningValueItem);

	m_vacuumDegreeValue = "0.02";
	QTableWidgetItem* vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
	vacuumDegreeValueItem->setBackground(QBrush(QColor(2, 253, 254)));
	m_tableWidget->setItem(5, 2, vacuumDegreeValueItem);



	m_relativeDensityValue = "";
	QTableWidgetItem* relativeDensityItem = new QTableWidgetItem(m_relativeDensityValue);
	relativeDensityItem->setBackground(QBrush(QColor(255, 254, 195)));
	m_tableWidget->setItem(7, 2, relativeDensityItem);

	m_injectionTimeValue = "";
	QTableWidgetItem* injectionTimeItem = new QTableWidgetItem(m_injectionTimeValue);
	injectionTimeItem->setBackground(QBrush(QColor(255, 254, 195)));
	m_tableWidget->setItem(8, 2, injectionTimeItem);
}


void InReverseOptimizationPropertyWidget::forwardCalculate()
{
	auto ins = ModelDataManager::GetInstance();
	auto modelGeometryInfo = ins->GetModelGeometryInfo();
	auto steelPropertyInfo = ins->GetSteelPropertyInfo();
	auto calculationPropertyInfo = ins->GetCalculationPropertyInfo();

	auto A = m_valveOpeningValue.toDouble(); // 阀门开度（mm）
	auto B = modelGeometryInfo.gasketLayerThickness; // 胶层厚度(mm)
	auto C = modelGeometryInfo.shellThickness; // 壳体厚度 (mm)
	auto D = m_vacuumDegreeValue.toDouble() * 1000; // 真空度(KPa)
	auto E = m_insulationTemperatureValue.toDouble(); // 保温温度（℃）

	
	auto relativeDensityBool = m_relativeDensityRadioBtn->isChecked(); // 相对密度
	auto injectionTimeBool = m_injectionTimeRadioBtn->isChecked(); // 弹体注药时间

	// 获取模型类型
	QString model = "产品一";
	QWidget* parent = parentWidget();
	while (parent) {
		GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
		if (gfParent)
		{
			auto* modelComboBox = gfParent->GetGeomPropertyWidget()->GetModelComboBox();
			model = modelComboBox->currentText();
			break;
		}
		else
		{
			parent = parent->parentWidget();
		}
	}

	QString formula = "";
	if (model == "产品一" )
	{
		A = (A - 6.740741) / 11.555556;
		B = (B - 1.074074) / 3.851852;
		C = (C - 20.185185) / 9.629630;
		D = (D - 21.111111) / 57.777778;
		E = (E - 50.370370) / 19.259259;
		if (!relativeDensityBool)
		{
			// 气含率
			formula = calculationPropertyInfo.oneGasRateCalculateFormula;
		}
		else
		{
			formula = calculationPropertyInfo.oneInjectionTimeCalculateFormula;
		}
	}
	else if (model == "产品二")
	{
		A = (A - 6.500000) / 13.000000;
		B = (B - 1.000000) / 4.000000;
		C = (C - 20.000000) / 10.000000;
		D = (D - 20.000000) / 60.000000;
		E = (E - 50.000000) / 20.000000;
		
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
	else if (model == "产品三")
	{
		A = (A - 6.500000) / 12.277778;
		B = (B - 1.000000) / 4.000000;
		C = (C - 20.000000) / 10.000000;
		D = (D - 20.000000) / 60.000000;
		E = (E - 50.000000) / 20.000000;

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

		A = (A - 8.500000) / 9.796296;
		B = (B - 1.000000) / 4.000000;
		C = (C - 20.000000) / 10.000000;
		D = (D - 20.000000) / 60.000000;
		E = (E - 50.000000) / 20.000000;

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
		QTableWidgetItem* relativeDensityItem = new QTableWidgetItem(relativeDensityResult);
		relativeDensityItem->setBackground(QBrush(QColor(255, 254, 195)));
		m_tableWidget->setItem(7, 2, relativeDensityItem);
	}
	else
	{
		if (model == "产品一") 
		{ 
			value = value * 21.33;
		}
		else if (model == "产品二")
		{
			value = value * 26.67;
		}
		else if (model == "产品三")
		{
			value = value * 26.33;
		}
		else 
		{
			value = value * 27.33;
		}

		QString injectionTimeResult = QString::number(qRound(value));
		QTableWidgetItem* injectionTimeItem = new QTableWidgetItem(injectionTimeResult);
		injectionTimeItem->setBackground(QBrush(QColor(255, 254, 195)));
		m_tableWidget->setItem(8, 2, injectionTimeItem);
	}

	


	

	

}