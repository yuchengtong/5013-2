#pragma once
#pragma execution_character_set("utf-8")
#include "InReverseOptimizationPropertyWidget.h"
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


InReverseOptimizationPropertyWidget::InReverseOptimizationPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
	bindConnect();
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

		QStringList labels = { "","工艺输入参数",  "弹体保温温度", "药液浇注温度", "阀门开度","真空度","工艺输出参数","相对密度","弹体注药时间","弹体温度云图与温升曲线" };
		for (int row = 0; row < labels.size(); ++row)
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
			labelItem->setTextAlignment(Qt::AlignCenter); // 文本居中	
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 1, labelItem);
		}

		// 计算按钮
		m_calButton = new QPushButton("计算");
		m_tableWidget->setCellWidget(0, 2, m_calButton);
		
		QTableWidgetItem* insulationTemperatureValueItem = new QTableWidgetItem(m_insulationTemperatureValue);
		insulationTemperatureValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		QTableWidgetItem* pouringTemperatureValueItem = new QTableWidgetItem(m_pouringTemperatureValue);
		pouringTemperatureValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		QTableWidgetItem* valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
		valveOpeningValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		QTableWidgetItem* vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
		vacuumDegreeValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		QTableWidgetItem* relativeDensityValueItem = new QTableWidgetItem(m_relativeDensityValue);
		relativeDensityValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		QTableWidgetItem* injectionTimeValueItem = new QTableWidgetItem(m_injectionTimeValue);
		injectionTimeValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中

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

void InReverseOptimizationPropertyWidget::bindConnect()
{
	connect(m_calButton, &QPushButton::clicked, this, &InReverseOptimizationPropertyWidget::calculate);


}

void InReverseOptimizationPropertyWidget::calculate()
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

	auto valveOpeningBool =  m_valveOpeningRadioBtn->isChecked(); // 阀门开度
	auto vacuumDegreeBool = m_vacuumDegreeRadioBtn->isChecked(); // 真空度
	
	auto relativeDensityBool = m_relativeDensityRadioBtn->isChecked(); // 相对密度
	auto injectionTimeBool = m_injectionTimeRadioBtn->isChecked(); // 弹体注药时间

	m_solver = new InReverseFormulaSolver(this);
	
	double density = ins->GetSteelPropertyInfo().density;
	QString formula = calculationPropertyInfo.oneGasRateCalculateFormula; // 方程

	QMap<char, double> known; // 计算入参值

	
	if (model == "产品一")
	{
		

		double target = 0;

		if (relativeDensityBool)
		{
			///////////////////////////// 已知相对密度/////////////////////////////////////
			// 相对密度转为气含率
			if (m_relativeDensityValue == "")
			{
				QMessageBox::information(this, "提示", "相对密度不能为空！");
				return;
			}
			known['A'] = (A - 6.740741) / 11.555556;
			known['B'] = (B - 1.074074) / 3.851852;
			known['C'] = (C - 20.185185) / 9.629630;
			known['D'] = (D - 21.111111) / 57.777778;
			known['E'] = (E - 86.370370) / 19.259259;

			auto relativeDensity = m_relativeDensityValue.toDouble();
			auto gasRateValue = ((1 - relativeDensity / 100) * density) / (density - 1.205);
			QString formula = calculationPropertyInfo.oneGasRateCalculateFormula; // 方程
			target = gasRateValue;

			connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
				qDebug() << "解数量：" << resList.size();

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
						qDebug() << "阀门开度原始解：" << v << " 换算后：" << value;
						if (value >= 0 && value <= 100) {
							m_valveOpeningValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(4, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
							QMessageBox::information(this, "成功", "计算完成");
							return;
						}
					}
					else
					{
						// 对真空度寻优
						double value = v;
						value = (57.777778 * v + 21.111111)/1000;
						qDebug() << "真空度原始解：" << v << " 换算后：" << value;

						if (value >= 0 && value <= 100) {
							m_vacuumDegreeValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(5, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
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

			if (m_injectionTimeValue == "")
			{
				QMessageBox::information(this, "提示", "弹体注药时间不能为空！");
				return;
			}
			known['A'] = (A - 6.740741) / 11.555556;
			known['B'] = (B - 1.074074) / 3.851852;
			known['C'] = (C - 20.185185) / 9.629630;
			known['D'] = (D - 21.111111) / 57.777778;
			known['E'] = (E - 50.370370) / 19.259259;
			auto injectionTimeValue = m_injectionTimeValue.toDouble() / 21.33;
			formula = calculationPropertyInfo.oneInjectionTimeCalculateFormula;
			target = injectionTimeValue;

			connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
				qDebug() << "解数量：" << resList.size();

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
						qDebug() << "阀门开度原始解：" << v << " 换算后：" << value;
						if (value >= 0 && value <= 100) {
							m_valveOpeningValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(4, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
							QMessageBox::information(this, "成功", "计算完成");
							return;
						}
					}
					else
					{
						// 对真空度寻优
						double value = v;
						value = (57.777778 * v + 21.111111)/1000;
						qDebug() << "真空度原始解：" << v << " 换算后：" << value;

						if (value >= 0 && value <= 100) {
							m_vacuumDegreeValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(5, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
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
			if (m_relativeDensityValue == "")
			{
				QMessageBox::information(this, "提示", "相对密度不能为空！");
				return;
			}
			auto relativeDensity = m_relativeDensityValue.toDouble();
			auto gasRateValue = ((1 - relativeDensity / 100) * density) / (density - 1.205);
			QString formula = calculationPropertyInfo.twoGasRateCalculateFormula; // 方程
			target = gasRateValue;

			connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
				qDebug() << "解数量：" << resList.size();

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
						qDebug() << "阀门开度原始解：" << v << " 换算后：" << value;
						if (value >= 0 && value <= 100) {
							m_valveOpeningValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(4, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
							QMessageBox::information(this, "成功", "计算完成");
							return;
						}
					}
					else
					{
						// 对真空度寻优
						double value = v;
						value = (60.000000 * v + 20.000000) / 1000;
						qDebug() << "真空度原始解：" << v << " 换算后：" << value;

						if (value >= 0 && value <= 100) {
							m_vacuumDegreeValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(5, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
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

			if (m_injectionTimeValue == "")
			{
				QMessageBox::information(this, "提示", "弹体注药时间不能为空！");
				return;
			}

			auto injectionTimeValue = m_injectionTimeValue.toDouble() / 26.67;
			formula = calculationPropertyInfo.twoInjectionTimeCalculateFormula;
			target = injectionTimeValue;

			connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
				qDebug() << "解数量：" << resList.size();

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
						qDebug() << "阀门开度原始解：" << v << " 换算后：" << value;
						if (value >= 0 && value <= 100) {
							m_valveOpeningValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(4, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
							QMessageBox::information(this, "成功", "计算完成");
							return;
						}
					}
					else
					{
						// 对真空度寻优
						double value = v;
						value = (60.000000 * v + 20.000000) / 1000;
						qDebug() << "真空度原始解：" << v << " 换算后：" << value;

						if (value >= 0 && value <= 100) {
							m_vacuumDegreeValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(5, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
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
			if (m_relativeDensityValue == "")
			{
				QMessageBox::information(this, "提示", "相对密度不能为空！");
				return;
			}
			auto relativeDensity = m_relativeDensityValue.toDouble();
			auto gasRateValue = ((1 - relativeDensity / 100) * density) / (density - 1.205);
			QString formula = calculationPropertyInfo.threeGasRateCalculateFormula; // 方程
			target = gasRateValue;

			connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
				qDebug() << "解数量：" << resList.size();

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
						qDebug() << "阀门开度原始解：" << v << " 换算后：" << value;
						if (value >= 0 && value <= 100) {
							m_valveOpeningValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(4, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
							QMessageBox::information(this, "成功", "计算完成");
							return;
						}
					}
					else
					{
						// 对真空度寻优
						double value = v;
						value = (60.000000 * v + 20.000000) / 1000;
						qDebug() << "真空度原始解：" << v << " 换算后：" << value;

						if (value >= 0 && value <= 100) {
							m_vacuumDegreeValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(5, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
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

			if (m_injectionTimeValue == "")
			{
				QMessageBox::information(this, "提示", "弹体注药时间不能为空！");
				return;
			}

			auto injectionTimeValue = m_injectionTimeValue.toDouble() / 27.33;
			formula = calculationPropertyInfo.threeInjectionTimeCalculateFormula;
			target = injectionTimeValue;

			connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
				qDebug() << "解数量：" << resList.size();

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
						qDebug() << "阀门开度原始解：" << v << " 换算后：" << value;
						if (value >= 0 && value <= 100) {
							m_valveOpeningValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(4, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
							QMessageBox::information(this, "成功", "计算完成");
							return;
						}
					}
					else
					{
						// 对真空度寻优
						double value = v;
						value = (60.000000 * v + 20.000000) / 1000;
						qDebug() << "真空度原始解：" << v << " 换算后：" << value;

						if (value >= 0 && value <= 100) {
							m_vacuumDegreeValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(5, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
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
			if (m_relativeDensityValue == "")
			{
				QMessageBox::information(this, "提示", "相对密度不能为空！");
				return;
			}
			auto relativeDensity = m_relativeDensityValue.toDouble();
			auto gasRateValue = ((1 - relativeDensity / 100) * density) / (density - 1.205);
			QString formula = calculationPropertyInfo.fourGasRateCalculateFormula; // 方程
			target = gasRateValue;
			

			connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
				qDebug() << "解数量：" << resList.size();

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
						qDebug() << "阀门开度原始解：" << v << " 换算后：" << value;
						if (value >= 0 && value <= 100) {
							m_valveOpeningValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(4, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
							QMessageBox::information(this, "成功", "计算完成");
							return;
						}
					}
					else
					{
						// 对真空度寻优
						double value = v;
						value = (60.000000 * v + 20.000000) / 1000;
						qDebug() << "真空度原始解：" << v << " 换算后：" << value;

						if (value >= 0 && value <= 100) {
							m_vacuumDegreeValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(5, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
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

			if (m_injectionTimeValue == "")
			{
				QMessageBox::information(this, "提示", "弹体注药时间不能为空！");
				return;
			}

			auto injectionTimeValue = m_injectionTimeValue.toDouble() / 21.33;
			formula = calculationPropertyInfo.fourInjectionTimeCalculateFormula;
			target = injectionTimeValue;

			connect(m_solver, &InReverseFormulaSolver::solveCompleted, this, [=](const std::vector<double>& resList) {
				qDebug() << "解数量：" << resList.size();

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
						qDebug() << "阀门开度原始解：" << v << " 换算后：" << value;
						if (value >= 0 && value <= 100) {
							m_valveOpeningValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(4, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
							QMessageBox::information(this, "成功", "计算完成");
							return;
						}
					}
					else
					{
						// 对真空度寻优
						double value = v;
						value = (60.000000 * v + 20.000000) / 1000;
						qDebug() << "真空度原始解：" << v << " 换算后：" << value;

						if (value >= 0 && value <= 100) {
							m_vacuumDegreeValue = QString::number(value, 'f', 4);
							m_tableWidget->setItem(5, 2, new QTableWidgetItem(QString::number(value, 'f', 4)));
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

}


