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


	QStringList labels = { "正向设计","工艺输入参数",  "弹体保温温度(50～70)", "药液浇注温度", "阀门开度(5～39)","真空度(0.02～0.08)","工艺输出参数","相对密度","弹体注药时间","弹体温度云图与温升曲线"};
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

	m_tableWidget->setItem(2, 2, injectionTimeValueItem);
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
			if (value >= 5 && value <= 39)
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

			if (value > 0 && value <= 1)
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
			if (value > 0 )
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

void InForwardDesignPropertyWidget::inForwardCalculate() {

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
						auto *modelComboBox = gfParent->GetGeomPropertyWidget()->GetModelComboBox();
						model = modelComboBox->currentText();
						break;
					}
					else
					{
						parent = parent->parentWidget();
					}
				}

				double gasRateValue = 0.0;
				double injectionTimeValue = 0.0;
				if (model == "产品一")
				{
					// 气含率
					auto gasRateA = (A - 6.740741) / 11.555556;
					auto gasRateB = (B - 1.074074) / 3.851852;
					auto gasRateC = (C - 20.185185) / 9.629630;
					auto gasRateD = (D - 21.111111) / 57.777778;
					auto gasRateE = (E - 50.370370) / 19.259259;
					gasRateValue = inForwardCalculateForm(calculationPropertyInfo.oneGasRateCalculateFormula, gasRateA, gasRateB, gasRateC, gasRateD, gasRateE);


					// 注药时间
					auto injectionTimeA = (A - 6.740741) / 11.555556;
					auto injectionTimeB = (B - 1.074074) / 3.851852;
					auto injectionTimeC = (C - 20.185185) / 9.629630;
					auto injectionTimeD = (D - 21.111111) / 57.777778;
					auto injectionTimeE = (E - 50.370370) / 19.259259;
					injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.oneInjectionTimeCalculateFormula, injectionTimeA, injectionTimeB, injectionTimeC, injectionTimeD, injectionTimeE);
					injectionTimeValue = injectionTimeValue * 21.33;
				}
				else if (model == "产品二")
				{
					// 气含率
					auto gasRateA = (A - 6.500000) / 13.000000;
					auto gasRateB = (B - 1.000000) / 4.000000;
					auto gasRateC = (C - 20.000000) / 10.000000;
					auto gasRateD = (D - 20.000000) / 60.000000;
					auto gasRateE = (E - 50.000000) / 20.000000;
					gasRateValue = inForwardCalculateForm(calculationPropertyInfo.twoGasRateCalculateFormula, gasRateA, gasRateB, gasRateC, gasRateD, gasRateE);


					// 注药时间
					auto injectionTimeA = (A - 6.500000) / 13.000000;
					auto injectionTimeB = (B - 1.000000) / 4.000000;
					auto injectionTimeC = (C - 20.000000) / 10.000000;
					auto injectionTimeD = (D - 20.000000) / 60.000000;
					auto injectionTimeE = (E - 50.000000) / 20.000000;
					injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.twoInjectionTimeCalculateFormula, injectionTimeA, injectionTimeB, injectionTimeC, injectionTimeD, injectionTimeE);
					injectionTimeValue = injectionTimeValue * 26.67;
				}
				else if (model == "产品三")
				{
					// 气含率
					auto gasRateA = (A - 6.500000) / 12.277778;
					auto gasRateB = (B - 1.000000) / 4.000000;
					auto gasRateC = (C - 20.000000) / 10.000000;
					auto gasRateD = (D - 20.000000) / 60.000000;
					auto gasRateE = (E - 50.000000) / 20.000000;
					gasRateValue = inForwardCalculateForm(calculationPropertyInfo.threeGasRateCalculateFormula, gasRateA, gasRateB, gasRateC, gasRateD, gasRateE);


					// 注药时间
					auto injectionTimeA = (A - 6.500000) / 12.277778;
					auto injectionTimeB = (B - 1.000000) / 4.000000;
					auto injectionTimeC = (C - 20.000000) / 10.000000;
					auto injectionTimeD = (D - 20.000000) / 60.000000;
					auto injectionTimeE = (E - 50.000000) / 20.000000;
					injectionTimeValue = inForwardCalculateForm(calculationPropertyInfo.threeInjectionTimeCalculateFormula, injectionTimeA, injectionTimeB, injectionTimeC, injectionTimeD, injectionTimeE);
					injectionTimeValue = injectionTimeValue * 26.33;
				}
				else
				{
					// 气含率
					auto gasRateA = (A - 8.500000) / 9.796296;
					auto gasRateB = (B - 1.000000) / 4.000000;
					auto gasRateC = (C - 20.000000) / 10.000000;
					auto gasRateD = (D - 20.000000) / 60.000000;
					auto gasRateE = (E - 50.000000) / 20.000000;
					gasRateValue = inForwardCalculateForm(calculationPropertyInfo.fourGasRateCalculateFormula, gasRateA, gasRateB, gasRateC, gasRateD, gasRateE);


					// 注药时间
					auto injectionTimeA = (A - 8.500000) / 9.796296;
					auto injectionTimeB = (B - 1.000000) / 4.000000;
					auto injectionTimeC = (C - 20.000000) / 10.000000;
					auto injectionTimeD = (D - 20.000000) / 60.000000;
					auto injectionTimeE = (E - 50.000000) / 20.000000;
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
					auto occView = gfParent->GetOccView();
					std::vector<double> nodeValues;
					APISetNodeValue::SetPreForwardDesignResult(occView, nodeValues, frameIndex);

					// 颜色标尺（从原 comboBox 槽搬过来）
					Handle(AIS_InteractiveContext) context = occView->getContext();
					Handle(V3d_View) view = occView->getView();
					view->SetProj(V3d_Zneg);
					view->SetTwist(-M_PI / 2.0);

					double min_value = 0;
					double max_value = 10;
					TCollection_ExtendedString tostr("温度云图", true);
					Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
					aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
					aColorScale->SetSize(50, 200);
					aColorScale->SetRange(min_value, max_value);
					aColorScale->SetNumberOfIntervals(9);
					aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
					aColorScale->SetTextHeight(14);
					aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
					aColorScale->SetTitle(tostr);
					aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
					aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
					aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
					Graphic3d_Vec2i anoffset(0, Standard_Integer(200));
					context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
					context->SetDisplayMode(aColorScale, 1, Standard_False);
					context->Display(aColorScale, Standard_True);
					});

				// 初始化第 0 帧
				auto occView = gfParent->GetOccView();
				std::vector<double> nodeValues;
				APISetNodeValue::SetPreForwardDesignResult(occView, nodeValues, 0);
				toolsAnimationWidget->UpdateUI(0);



				if (!success)
				{
					QMessageBox::warning(this, "计算失败", msg);
				}
				QMessageBox::information(this, "计算", "计算成功");
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
	m_insulationTemperatureValue = "50";
	QTableWidgetItem* insulationTemperatureValueItem = new QTableWidgetItem(m_insulationTemperatureValue);
	insulationTemperatureValueItem->setBackground(QBrush(QColor(255, 254, 195)));
	m_tableWidget->setItem(2, 2, insulationTemperatureValueItem);

	m_valveOpeningValue = "5";
	QTableWidgetItem* valveOpeningValueItem = new QTableWidgetItem(m_valveOpeningValue);
	valveOpeningValueItem->setBackground(QBrush(QColor(255, 254, 195)));
	m_tableWidget->setItem(4, 2, valveOpeningValueItem);

	m_vacuumDegreeValue = "0.02";
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


			if (model == "产品一")
			{
				//温度密度曲线
				double densityTempXStart = 49.0;  // 弹体保温温度(50～70)
				double densityTempXEnd = m_insulationTemperatureValue.toDouble(); // 弹体目标温度（℃）
				double densityTempStep = (densityTempXEnd - densityTempXStart) / 10;

				for (double i = densityTempXStart; i <= densityTempXEnd; i += densityTempStep) {

					auto tempA = (A - 6.740741) / 11.555556;
					auto tempB = (B - 1.074074) / 3.851852;
					auto tempC = (C - 20.185185) / 9.629630;
					auto tempD = (D - 21.111111) / 57.777778;
					auto tempE = (i - 50.370370) / 19.259259;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.oneGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
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
					densityTempX.push_back(i);
					densityTempY.push_back(relativeDensity * 100.0);

				}


				///阀门开度密度曲线
				double densityValveXStart = 4.9;  // 阀门开度(5～39)
				double densityValveXEnd = m_valveOpeningValue.toDouble(); // 阀门开度(5～39)
				double densityValveStep = (densityValveXEnd - densityValveXStart) / 10;
				for (double i = densityValveXStart; i <= densityValveXEnd; i += densityValveStep) {

					auto tempA = (i - 6.740741) / 11.555556;
					auto tempB = (B - 1.074074) / 3.851852;
					auto tempC = (C - 20.185185) / 9.629630;
					auto tempD = (D - 21.111111) / 57.777778;
					auto tempE = (E - 50.370370) / 19.259259;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.oneGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
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
					densityValveX.push_back(i);
					densityValveY.push_back(relativeDensity * 100.0);

				}

				//真空度密度曲线
				double densityVacuumXStart = 19.0;  // 真空度(20～80)
				double densityVacuumXEnd = m_vacuumDegreeValue.toDouble() * 1000.0; // 真空度(20～80)
				double densityVacuumStep = (densityVacuumXEnd - densityVacuumXStart) / 10;
				for (double i = densityVacuumXStart; i <= densityVacuumXEnd; i += densityVacuumStep) {

					auto tempA = (A - 6.740741) / 11.555556;
					auto tempB = (B - 1.074074) / 3.851852;
					auto tempC = (C - 20.185185) / 9.629630;
					auto tempD = (i - 21.111111) / 57.777778;
					auto tempE = (E - 50.370370) / 19.259259;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.oneGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
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
					densityVacuumX.push_back(i/1000.0);
					densityVacuumY.push_back(relativeDensity * 100.0);

				}


				//温度注药时间曲线
				double timeTempXStart = 49.0;  // 弹体保温温度(50～70)
				double timeTempXEnd = m_insulationTemperatureValue.toDouble(); // 弹体目标温度（℃）
				double timeTempStep = (timeTempXEnd - timeTempXStart) / 10;

				for (double i = timeTempXStart; i <= timeTempXEnd; i += timeTempStep) {

					auto tempA = (A - 6.740741) / 11.555556;
					auto tempB = (B - 1.074074) / 3.851852;
					auto tempC = (C - 20.185185) / 9.629630;
					auto tempD = (D - 21.111111) / 57.777778;
					auto tempE = (i - 50.370370) / 19.259259;
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
				double timeValveXStart = 4.9;  // 阀门开度(5～39)
				double timeValveXEnd = m_valveOpeningValue.toDouble(); // 阀门开度(5～39)
				double timeValveStep = (timeValveXEnd - timeValveXStart) / 10;
				for (double i = timeValveXStart; i <= timeValveXEnd; i += timeValveStep) {

					auto tempA = (i - 6.740741) / 11.555556;
					auto tempB = (B - 1.074074) / 3.851852;
					auto tempC = (C - 20.185185) / 9.629630;
					auto tempD = (D - 21.111111) / 57.777778;
					auto tempE = (E - 50.370370) / 19.259259;
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
				double timeVacuumXStart = 19.0;  // 真空度(20～80)
				double timeVacuumXEnd = m_vacuumDegreeValue.toDouble() * 1000.0; // 真空度(20～80)
				double timeVacuumStep = (timeVacuumXEnd - timeVacuumXStart) / 10;
				for (double i = timeVacuumXStart; i <= timeVacuumXEnd; i += timeVacuumStep) {

					auto tempA = (A - 6.740741) / 11.555556;
					auto tempB = (B - 1.074074) / 3.851852;
					auto tempC = (C - 20.185185) / 9.629630;
					auto tempD = (i - 21.111111) / 57.777778;
					auto tempE = (E - 50.370370) / 19.259259;
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
			else if (model == "产品二")
			{
				//温度密度曲线
				double densityTempXStart = 49.0;  // 弹体保温温度(50～70)
				double densityTempXEnd = m_insulationTemperatureValue.toDouble(); // 弹体目标温度（℃）
				double densityTempStep = (densityTempXEnd - densityTempXStart) / 10;

				for (double i = densityTempXStart; i <= densityTempXEnd; i += densityTempStep) {

					auto tempA = (A - 6.500000) / 13.000000;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (D - 20.000000) / 60.000000;
					auto tempE = (i - 50.000000) / 20.000000;

					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.twoGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
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
					densityTempX.push_back(i);
					densityTempY.push_back(relativeDensity * 100.0);

				}


				///阀门开度密度曲线
				double densityValveXStart = 4.9;  // 阀门开度(5～39)
				double densityValveXEnd = m_valveOpeningValue.toDouble(); // 阀门开度(5～39)
				double densityValveStep = (densityValveXEnd - densityValveXStart) / 10;
				for (double i = densityValveXStart; i <= densityValveXEnd; i += densityValveStep) {

					auto tempA = (i - 6.500000) / 13.000000;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (D - 20.000000) / 60.000000;
					auto tempE = (E - 50.000000) / 20.000000;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.twoGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
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
					densityValveX.push_back(i);
					densityValveY.push_back(relativeDensity * 100.0);

				}

				//真空度密度曲线
				double densityVacuumXStart = 19.0;  // 真空度(20～80)
				double densityVacuumXEnd = m_vacuumDegreeValue.toDouble() * 1000.0; // 真空度(20～80)
				double densityVacuumStep = (densityVacuumXEnd - densityVacuumXStart) / 10;
				for (double i = densityVacuumXStart; i <= densityVacuumXEnd; i += densityVacuumStep) {

					auto tempA = (A - 6.500000) / 13.000000;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (i - 20.000000) / 60.000000;
					auto tempE = (E - 50.000000) / 20.000000;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.twoGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
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
					densityVacuumX.push_back(i / 1000.0);
					densityVacuumY.push_back(relativeDensity * 100.0);

				}


				//温度注药时间曲线
				double timeTempXStart = 49.0;  // 弹体保温温度(50～70)
				double timeTempXEnd = m_insulationTemperatureValue.toDouble(); // 弹体目标温度（℃）
				double timeTempStep = (timeTempXEnd - timeTempXStart) / 10;

				for (double i = timeTempXStart; i <= timeTempXEnd; i += timeTempStep) {

					auto tempA = (A - 6.500000) / 13.000000;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (D - 20.000000) / 60.000000;
					auto tempE = (i - 50.000000) / 20.000000;
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
				double timeValveXStart = 4.9;  // 阀门开度(5～39)
				double timeValveXEnd = m_valveOpeningValue.toDouble(); // 阀门开度(5～39)
				double timeValveStep = (timeValveXEnd - timeValveXStart) / 10;
				for (double i = timeValveXStart; i <= timeValveXEnd; i += timeValveStep) {

					auto tempA = (i - 6.500000) / 13.000000;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (D - 20.000000) / 60.000000;
					auto tempE = (E - 50.000000) / 20.000000;
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
				double timeVacuumXStart = 19.0;  // 真空度(20～80)
				double timeVacuumXEnd = m_vacuumDegreeValue.toDouble() * 1000.0; // 真空度(20～80)
				double timeVacuumStep = (timeVacuumXEnd - timeVacuumXStart) / 10;
				for (double i = timeVacuumXStart; i <= timeVacuumXEnd; i += timeVacuumStep) {

					auto tempA = (A - 6.500000) / 13.000000;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (i - 20.000000) / 60.000000;
					auto tempE = (E - 50.000000) / 20.000000;
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
			else if (model == "产品三")
			{
				//温度密度曲线
				double densityTempXStart = 49.0;  // 弹体保温温度(50～70)
				double densityTempXEnd = m_insulationTemperatureValue.toDouble(); // 弹体目标温度（℃）
				double densityTempStep = (densityTempXEnd - densityTempXStart) / 10;

				for (double i = densityTempXStart; i <= densityTempXEnd; i += densityTempStep) {

					auto tempA = (A - 6.500000) / 12.277778;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (D - 20.000000) / 60.000000;
					auto tempE = (i - 50.000000) / 20.000000;

					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.threeGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
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
					densityTempX.push_back(i);
					densityTempY.push_back(relativeDensity * 100.0);

				}


				///阀门开度密度曲线
				double densityValveXStart = 4.9;  // 阀门开度(5～39)
				double densityValveXEnd = m_valveOpeningValue.toDouble(); // 阀门开度(5～39)
				double densityValveStep = (densityValveXEnd - densityValveXStart) / 10;
				for (double i = densityValveXStart; i <= densityValveXEnd; i += densityValveStep) {

					auto tempA = (i - 6.500000) / 12.277778;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (D - 20.000000) / 60.000000;
					auto tempE = (E - 50.000000) / 20.000000;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.threeGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
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
					densityValveX.push_back(i);
					densityValveY.push_back(relativeDensity * 100.0);

				}

				//真空度密度曲线
				double densityVacuumXStart = 19.0;  // 真空度(20～80)
				double densityVacuumXEnd = m_vacuumDegreeValue.toDouble() * 1000.0; // 真空度(20～80)
				double densityVacuumStep = (densityVacuumXEnd - densityVacuumXStart) / 10;
				for (double i = densityVacuumXStart; i <= densityVacuumXEnd; i += densityVacuumStep) {

					auto tempA = (A - 6.500000) / 12.277778;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (i - 20.000000) / 60.000000;
					auto tempE = (E - 50.000000) / 20.000000;
					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.threeGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
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
					densityVacuumX.push_back(i / 1000.0);
					densityVacuumY.push_back(relativeDensity * 100.0);

				}


				//温度注药时间曲线
				double timeTempXStart = 49.0;  // 弹体保温温度(50～70)
				double timeTempXEnd = m_insulationTemperatureValue.toDouble(); // 弹体目标温度（℃）
				double timeTempStep = (timeTempXEnd - timeTempXStart) / 10;

				for (double i = timeTempXStart; i <= timeTempXEnd; i += timeTempStep) {

					auto tempA = (A - 6.500000) / 12.277778;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (D - 20.000000) / 60.000000;
					auto tempE = (i - 50.000000) / 20.000000;
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
				double timeValveXStart = 4.9;  // 阀门开度(5～39)
				double timeValveXEnd = m_valveOpeningValue.toDouble(); // 阀门开度(5～39)
				double timeValveStep = (timeValveXEnd - timeValveXStart) / 10;
				for (double i = timeValveXStart; i <= timeValveXEnd; i += timeValveStep) {

					auto tempA = (i - 6.500000) / 12.277778;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (D - 20.000000) / 60.000000;
					auto tempE = (E - 50.000000) / 20.000000;
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
				double timeVacuumXStart = 19.0;  // 真空度(20～80)
				double timeVacuumXEnd = m_vacuumDegreeValue.toDouble() * 1000.0; // 真空度(20～80)
				double timeVacuumStep = (timeVacuumXEnd - timeVacuumXStart) / 10;
				for (double i = timeVacuumXStart; i <= timeVacuumXEnd; i += timeVacuumStep) 
				{
					auto tempA = (A - 6.500000) / 12.277778;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (i - 20.000000) / 60.000000;
					auto tempE = (E - 50.000000) / 20.000000;
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
			else if (model == "产品四")
			{
				//温度密度曲线
				double densityTempXStart = 49.0;  // 弹体保温温度(50～70)
				double densityTempXEnd = m_insulationTemperatureValue.toDouble(); // 弹体目标温度（℃）
				double densityTempStep = (densityTempXEnd - densityTempXStart) / 10;

				for (double i = densityTempXStart; i <= densityTempXEnd; i += densityTempStep) {

					auto tempA = (A - 8.500000) / 9.796296;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (D - 20.000000) / 60.000000;
					auto tempE = (i - 50.000000) / 20.000000;

					double gasRateValue = inForwardCalculateForm(calculationPropertyInfo.fourGasRateCalculateFormula, tempA, tempB, tempC, tempD, tempE);
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
					densityTempX.push_back(i);
					densityTempY.push_back(relativeDensity * 100.0);

				}


				///阀门开度密度曲线
				double densityValveXStart = 4.9;  // 阀门开度(5～39)
				double densityValveXEnd = m_valveOpeningValue.toDouble(); // 阀门开度(5～39)
				double densityValveStep = (densityValveXEnd - densityValveXStart) / 10;
				for (double i = densityValveXStart; i <= densityValveXEnd; i += densityValveStep) {

					auto tempA = (i - 8.500000) / 9.796296;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (D - 20.000000) / 60.000000;
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
					if (relativeDensity > 1.0)
					{
						relativeDensity = 1.0;
					}
					densityValveX.push_back(i);
					densityValveY.push_back(relativeDensity * 100.0);

				}

				//真空度密度曲线
				double densityVacuumXStart = 19.0;  // 真空度(20～80)
				double densityVacuumXEnd = m_vacuumDegreeValue.toDouble() * 1000.0; // 真空度(20～80)
				double densityVacuumStep = (densityVacuumXEnd - densityVacuumXStart) / 10;
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
					if (relativeDensity > 1.0)
					{
						relativeDensity = 1.0;
					}
					densityVacuumX.push_back(i / 1000.0);
					densityVacuumY.push_back(relativeDensity * 100.0);

				}


				//温度注药时间曲线
				double timeTempXStart = 49.0;  // 弹体保温温度(50～70)
				double timeTempXEnd = m_insulationTemperatureValue.toDouble(); // 弹体目标温度（℃）
				double timeTempStep = (timeTempXEnd - timeTempXStart) / 10;

				for (double i = timeTempXStart; i <= timeTempXEnd; i += timeTempStep) {

					auto tempA = (A - 8.500000) / 9.796296;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (D - 20.000000) / 60.000000;
					auto tempE = (i - 50.000000) / 20.000000;
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
				double timeValveXStart = 5.0;  // 阀门开度(5～39)
				double timeValveXEnd = m_valveOpeningValue.toDouble(); // 阀门开度(5～39)
				double timeValveStep = (timeValveXEnd - timeValveXStart) / 10;
				for (double i = timeValveXStart; i <= timeValveXEnd; i += timeValveStep) {

					auto tempA = (i - 8.500000) / 9.796296;
					auto tempB = (B - 1.000000) / 4.000000;
					auto tempC = (C - 20.000000) / 10.000000;
					auto tempD = (D - 20.000000) / 60.000000;
					auto tempE = (E - 50.000000) / 20.000000;
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
				double timeVacuumXStart = 19.0;  // 真空度(20～80)
				double timeVacuumXEnd = m_vacuumDegreeValue.toDouble() * 1000.0; // 真空度(20～80)
				double timeVacuumStep = (timeVacuumXEnd - timeVacuumXStart) / 10;
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

			auto inForwardDensityTempWid = gfParent->GetInForwardDensityTempWid();
			auto inForwardDensityValveWid = gfParent->GetInForwardDensityValveWid();
			auto inForwardDensityVacuumWid = gfParent->GetInForwardDensityVacuumWid();
			auto inForwardTimeTempWid = gfParent->GetInForwardTimeTempWid();
			auto inForwardTimeValveWid = gfParent->GetInForwardTimeValveWid();
			auto inForwardTimeVacuumWid = gfParent->GetInForwardTimeVacuumWid();



			inForwardDensityTempWid->AddDataPoint(densityTempX, densityTempY);
			inForwardDensityValveWid->AddDataPoint(densityValveX, densityValveY);
			inForwardDensityVacuumWid->AddDataPoint(densityVacuumX, densityVacuumY);
			inForwardTimeTempWid->AddDataPoint(timeTempX, timeTempY);
			inForwardTimeValveWid->AddDataPoint(timeValveX, timeValveY);
			inForwardTimeVacuumWid->AddDataPoint(timeVacuumX, timeVacuumY);

			break;
		}
		else
		{
			parent = parent->parentWidget();
		}
	}

	
		
}
