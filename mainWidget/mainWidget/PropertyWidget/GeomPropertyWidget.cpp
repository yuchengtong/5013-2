#pragma once
#pragma execution_character_set("utf-8")
#include "GeomPropertyWidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>

GeomPropertyWidget::GeomPropertyWidget(QWidget* parent)
	:BasePropertyWidget(parent)
{
	initWidget();
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

	QStringList labels = { "几何模型","产品型号","来源","弹体大径","等径段高度", "弹体小径", "变径段高度", "壳体厚度", "胶层厚度", "注药孔孔径", "真空孔孔径" };
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
	

	QComboBox* modelComboBox = new QComboBox();
	modelComboBox->addItems({ "产品一", "产品二", "产品三", "产品四"});

	QTableWidgetItem* sourceValueItem = new QTableWidgetItem("");
	sourceValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中

	QTableWidgetItem* boreDiameterValueItem = new QTableWidgetItem("391");
	boreDiameterValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中

	QTableWidgetItem* equaldiameterSectionHeightValueItem = new QTableWidgetItem("560");
	equaldiameterSectionHeightValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中

	QTableWidgetItem* boreRadiusValueItem = new QTableWidgetItem("0");
	boreRadiusValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中

	QTableWidgetItem* variableDiameterSectionHeightValueItem = new QTableWidgetItem("0");
	variableDiameterSectionHeightValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中

	QTableWidgetItem* shellThicknessValueItem = new QTableWidgetItem("20");
	shellThicknessValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中

	QTableWidgetItem* gasketLayerThicknessValueItem = new QTableWidgetItem("1");
	gasketLayerThicknessValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中

	QTableWidgetItem* injectionHoleDiameterValueItem = new QTableWidgetItem("20");
	injectionHoleDiameterValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	injectionHoleDiameterValueItem->setFlags(injectionHoleDiameterValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑

	QTableWidgetItem* vacuumHoleDiameterValueItem = new QTableWidgetItem("8");
	vacuumHoleDiameterValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
	vacuumHoleDiameterValueItem->setFlags(vacuumHoleDiameterValueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑



	m_tableWidget->setCellWidget(1, 2, modelComboBox);
	m_tableWidget->setItem(2, 2, sourceValueItem);
	m_tableWidget->setItem(3, 2, boreDiameterValueItem);
	m_tableWidget->setItem(4, 2, equaldiameterSectionHeightValueItem);
	m_tableWidget->setItem(5, 2, boreRadiusValueItem);
	m_tableWidget->setItem(6, 2, variableDiameterSectionHeightValueItem);
	m_tableWidget->setItem(7, 2, shellThicknessValueItem);
	m_tableWidget->setItem(8, 2, gasketLayerThicknessValueItem);
	m_tableWidget->setItem(9, 2, injectionHoleDiameterValueItem);
	m_tableWidget->setItem(10, 2, vacuumHoleDiameterValueItem);
	

	QStringList unitLabels = { " "," ","", "mm", "mm", "mm", "mm", "mm", "mm", "mm", "mm" };
	for (int row = 0; row < unitLabels.size(); ++row) {
		if (row != 0)
		{
			QTableWidgetItem* labelItem = new QTableWidgetItem(unitLabels[row]);
			labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
			m_tableWidget->setItem(row, 3, labelItem);
		}
	}

	// 设置列宽度
	QTableWidgetItem* colimnItem = m_tableWidget->item(4, 1);
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

	
	

	connect(m_tableWidget, &QTableWidget::itemChanged, this, [this, boreDiameterValueItem, equaldiameterSectionHeightValueItem, shellThicknessValueItem, gasketLayerThicknessValueItem](QTableWidgetItem* item) {
		
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

		}
	});

}
