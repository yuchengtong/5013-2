#pragma once
#pragma execution_character_set("utf-8")
#include "GeomPropertyWidget.h"
#include "../GFImportModelWidget.h"
#include <QTableWidget>
#include <QHeaderView>

GeomPropertyWidget::GeomPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
	m_tableWidget->setStyleSheet(
		"QTableWidget {"
		"	background-color: #ffffff;"
		"   border: 2px solid #999999;"
		"   border-radius: 12px;"
		"}"

		"QPushButton {"
		"   background-color: #f0f0f0;"
		"   border: 1px solid #ccc;"
		"   border-radius: 8px;"
		"   padding: 4px 8px;"
		"   min-width: 60px;"
		"}"
		"QPushButton:hover {"
		"   background-color: #e0e0e0;"
		"}"
		"QPushButton:pressed {"
		"   background-color: #d0d0d0;"
		"}"
	);

}

void GeomPropertyWidget::UpdataPropertyInfo()
{
	auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	QTableWidgetItem* path_item = m_tableWidget->item(2, 2);
	//QTableWidgetItem* length_item = m_tableWidget->item(3, 2);
	//QTableWidgetItem* width_item = m_tableWidget->item(4, 2);
	////QTableWidgetItem* height_item = m_tableWidget->item(5, 2);
	//QTableWidgetItem* thickness_item = m_tableWidget->item(5, 2);

	if (path_item )
	{
		path_item->setText(modelInfo.path);
		//length_item->setText(QString::number(modelInfo.length, 'f', 3));
		//width_item->setText(QString::number(modelInfo.width, 'f', 3));
		////height_item->setText(QString::number(modelInfo.height, 'f', 3));
		//thickness_item->setText(QString::number(3, 'f', 3));
	}
	// 更新厚度值
	//modelInfo.thickness = thickness_item->text().toDouble();
	//ModelDataManager::GetInstance()->SetModelGeometryInfo(modelInfo);
}

void GeomPropertyWidget::initWidget()
{
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);

	m_tableWidget = new QTableWidget(this);
	// 设置行列数，这里固定 5 行 4 列
	m_tableWidget->setRowCount(11);
	m_tableWidget->setColumnCount(4);
	// 隐藏表头（如果不需要显示表头文字，可根据需求决定是否隐藏）
	m_tableWidget->horizontalHeader()->setVisible(false);
	m_tableWidget->verticalHeader()->setVisible(false);
	// 让表格充满布局，自动调整行列大小


	// 设置第一列固定宽度（例如100像素）
	m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	m_tableWidget->horizontalHeader()->resizeSection(0, 5);
	m_tableWidget->horizontalHeader()->resizeSection(1, 60);
	m_tableWidget->horizontalHeader()->resizeSection(3, 5);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
	// 让表格充满布局，自动调整行列大小
	m_tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_tableWidget->setColumnWidth(0, 5);
	// 合并第一行的第一和第二列
	m_tableWidget->setSpan(0, 0, 1, 4);

	vlayout->addWidget(m_tableWidget);
	setLayout(vlayout);

	QStringList labels = { "几何模型","产品型号","来源","弹体大径","等径段高度", "弹体小径", "变径段高度", "壳体厚度(20～30)", "胶层厚度(1～5)", "注药孔孔径(5～39)", "真空孔孔径" };
	for (int row = 0; row < labels.size(); ++row) {
		QTableWidgetItem* serialItem = new QTableWidgetItem(QString::number(row));
		if (row == 0) {
			serialItem = new QTableWidgetItem("几何模型");
		}
		serialItem->setFlags(serialItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 0, serialItem);

		QTableWidgetItem* labelItem = new QTableWidgetItem(labels[row]);
		labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
		m_tableWidget->setItem(row, 1, labelItem);
	}

	//第2列用空Label
	QStringList emptyLabels = { " "," ","", "", "", "","","","","","" };
	for (int row = 0; row < emptyLabels.size(); ++row) {
		if (row == 1 || (row >= 3 && row <= 8))
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(emptyLabels[row]);
			m_tableWidget->setItem(row, 2, labelItem);
		}
		else if (row != 0)
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(emptyLabels[row]);
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 2, labelItem);
		}
	}
	

	/*m_modelComboBox = new QComboBox();
	m_modelComboBox->addItems({ "HQ-9B", "YJ-20", "YJ-91A", "CJ-20A"});
	m_modelComboBox->setEditable(false);*/

	QTableWidgetItem* modelValueItem = new QTableWidgetItem(m_modelValue);
	modelValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	modelValueItem->setFlags(modelValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	modelValueItem->setBackground(QBrush(QColor(230, 230, 230)));

	QTableWidgetItem* sourceValueItem = new QTableWidgetItem("");
	sourceValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中

	QTableWidgetItem* boreDiameterValueItem = new QTableWidgetItem(m_boreDiameterValue);
	boreDiameterValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	boreDiameterValueItem->setFlags(boreDiameterValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	boreDiameterValueItem->setBackground(QBrush(QColor(230, 230, 230)));

	QTableWidgetItem* equaldiameterSectionHeightValueItem = new QTableWidgetItem(m_equaldiameterSectionHeightValue);
	equaldiameterSectionHeightValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	equaldiameterSectionHeightValueItem->setFlags(equaldiameterSectionHeightValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	equaldiameterSectionHeightValueItem->setBackground(QBrush(QColor(230, 230, 230)));

	QTableWidgetItem* boreRadiusValueItem = new QTableWidgetItem(m_boreRadiusValue);
	boreRadiusValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	boreRadiusValueItem->setFlags(boreRadiusValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	boreRadiusValueItem->setBackground(QBrush(QColor(230, 230, 230)));

	QTableWidgetItem* variableDiameterSectionHeightValueItem = new QTableWidgetItem(m_variableDiameterSectionHeightValue);
	variableDiameterSectionHeightValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	variableDiameterSectionHeightValueItem->setFlags(variableDiameterSectionHeightValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	variableDiameterSectionHeightValueItem->setBackground(QBrush(QColor(230, 230, 230)));

	QTableWidgetItem* shellThicknessValueItem = new QTableWidgetItem(m_shellThicknessValue);
	shellThicknessValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	shellThicknessValueItem->setBackground(QBrush(QColor(255, 254, 195)));

	QTableWidgetItem* gasketLayerThicknessValueItem = new QTableWidgetItem(m_gasketLayerThicknessValue);
	gasketLayerThicknessValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	gasketLayerThicknessValueItem->setBackground(QBrush(QColor(255, 254, 195)));

	QTableWidgetItem* injectionHoleDiameterValueItem = new QTableWidgetItem(m_injectionHoleDiameterValue);
	injectionHoleDiameterValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	injectionHoleDiameterValueItem->setBackground(QBrush(QColor(255, 254, 195)));

	QTableWidgetItem* vacuumHoleDiameterValueItem = new QTableWidgetItem(m_vacuumHoleDiameterValue);
	vacuumHoleDiameterValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	vacuumHoleDiameterValueItem->setFlags(vacuumHoleDiameterValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
	vacuumHoleDiameterValueItem->setBackground(QBrush(QColor(230, 230, 230)));



	//m_tableWidget->setCellWidget(1, 2, m_modelComboBox);
	m_tableWidget->setItem(1, 2, modelValueItem);
	m_tableWidget->setItem(2, 2, sourceValueItem);
	m_tableWidget->setItem(3, 2, boreDiameterValueItem);
	m_tableWidget->setItem(4, 2, equaldiameterSectionHeightValueItem);
	m_tableWidget->setItem(5, 2, boreRadiusValueItem);
	m_tableWidget->setItem(6, 2, variableDiameterSectionHeightValueItem);
	m_tableWidget->setItem(7, 2, shellThicknessValueItem);
	m_tableWidget->setItem(8, 2, gasketLayerThicknessValueItem);
	m_tableWidget->setItem(9, 2, injectionHoleDiameterValueItem);
	m_tableWidget->setItem(10, 2, vacuumHoleDiameterValueItem);

	/*connect(m_modelComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this, &GeomPropertyWidget::onComboBoxChanged);*/

	QStringList unitLabels = { " "," ","", "mm", "mm", "mm", "mm", "mm", "mm", "mm", "mm" };
	for (int row = 0; row < unitLabels.size(); ++row) {
		if (row != 0)
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(unitLabels[row]);
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			labelItem->setBackground(QBrush(QColor(230, 230, 230)));
			m_tableWidget->setItem(row, 3, labelItem);
		}
	}

	// 设置列宽度
	QTableWidgetItem* colimnItem = m_tableWidget->item(9, 1);
	int itemWidth = QFontMetrics(m_tableWidget->font()).width(colimnItem->text());
	m_tableWidget->setColumnWidth(1, itemWidth + m_tableWidget->verticalHeader()->width());

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

	
	

	connect(m_tableWidget, &QTableWidget::itemChanged, this, [this, boreDiameterValueItem, equaldiameterSectionHeightValueItem, boreRadiusValueItem, variableDiameterSectionHeightValueItem, shellThicknessValueItem, gasketLayerThicknessValueItem, injectionHoleDiameterValueItem, vacuumHoleDiameterValueItem](QTableWidgetItem* item) {
		
		auto ins = ModelDataManager::GetInstance();
		auto modelGeometryInfo = ins->GetModelGeometryInfo();

		if (item == boreDiameterValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();
			if (value >= 200 && value <= 1000)
			{
				m_boreDiameterValue = text;
			}
			else
			{
				m_tableWidget->blockSignals(true);
				item->setText(m_boreDiameterValue);
				m_tableWidget->blockSignals(false);
			}
			modelGeometryInfo.boreDiameter = m_boreDiameterValue.toDouble();
		}
		if (item == equaldiameterSectionHeightValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();
			if (value >= 200 && value <= 1000)
			{
				m_equaldiameterSectionHeightValue = text;
			}
			else
			{
				m_tableWidget->blockSignals(true);
				item->setText(m_equaldiameterSectionHeightValue);
				m_tableWidget->blockSignals(false);
			}
			modelGeometryInfo.equaldiameterSection = m_equaldiameterSectionHeightValue.toDouble();
		}
		if (item == boreRadiusValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();
			m_boreRadiusValue = text;
			modelGeometryInfo.boreRadius = m_boreRadiusValue.toDouble();
		}
		if (item == variableDiameterSectionHeightValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();
			m_variableDiameterSectionHeightValue = text;
			modelGeometryInfo.variableDiameterSectionHeight = m_variableDiameterSectionHeightValue.toDouble();
		}
		if (item == shellThicknessValueItem )
		{
			auto text = item->text();
			auto value = text.toDouble();
			if (value >= 20 && value <= 30)
			{
				m_shellThicknessValue = text;
			}
			else
			{
				m_tableWidget->blockSignals(true);
				item->setText(m_shellThicknessValue);
				m_tableWidget->blockSignals(false);
			}
			modelGeometryInfo.shellThickness = m_shellThicknessValue.toDouble();
		}

		if (item == gasketLayerThicknessValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();
			if (value >= 1 && value <= 5)
			{
				m_gasketLayerThicknessValue = text;
			}
			else
			{
				m_tableWidget->blockSignals(true);
				item->setText(m_gasketLayerThicknessValue);
				m_tableWidget->blockSignals(false);
			}
			modelGeometryInfo.gasketLayerThickness = m_gasketLayerThicknessValue.toDouble();
		}
		if (item == injectionHoleDiameterValueItem)
		{
			auto text = item->text();
			auto value = text.toDouble();
			if (value >= 5 && value <= 39)
			{
				m_injectionHoleDiameterValue = text;
			}
			else
			{
				m_tableWidget->blockSignals(true);
				item->setText(m_injectionHoleDiameterValue);
				m_tableWidget->blockSignals(false);
			}
			modelGeometryInfo.injectionHoleDiameter = m_injectionHoleDiameterValue.toDouble();
		}
		ins->SetModelGeometryInfo(modelGeometryInfo);
	});

}

void GeomPropertyWidget::onComboBoxChanged(int index) {
	/*QWidget* parent = parentWidget();
	while (parent)
	{
		GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
		if (gfParent)
		{
			CalculationPropertyWidget* calculationPropertyWidget = gfParent->GetCalculationPropertyWidget();
			calculationPropertyWidget->updateData(m_modelComboBox->currentText());
			break;
		}
		else
		{
			parent = parent->parentWidget();
		}
	}*/
}
