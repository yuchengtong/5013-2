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

		QStringList labels = { "","工艺输入参数",  "弹体保温温度", "药液浇注温度", "药液浇注速度","真空度","工艺输出参数","相对密度","弹体注药时间","弹体温度云图与温升曲线" };
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
		QTableWidgetItem* pouringSpeedValueItem = new QTableWidgetItem(m_pouringSpeedValue);
		pouringSpeedValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		QTableWidgetItem* vacuumDegreeValueItem = new QTableWidgetItem(m_vacuumDegreeValue);
		vacuumDegreeValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		QTableWidgetItem* relativeDensityValueItem = new QTableWidgetItem(m_relativeDensityValue);
		relativeDensityValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中
		QTableWidgetItem* injectionTimeValueItem = new QTableWidgetItem(m_injectionTimeValue);
		injectionTimeValueItem->setTextAlignment(Qt::AlignCenter); // 文本居中

		m_tableWidget->setItem(2, 2, insulationTemperatureValueItem);
		m_tableWidget->setItem(3, 2, pouringTemperatureValueItem);
		m_tableWidget->setItem(4, 2, pouringSpeedValueItem);
		m_tableWidget->setItem(5, 2, vacuumDegreeValueItem);
		m_tableWidget->setItem(7, 2, relativeDensityValueItem);
		m_tableWidget->setItem(8, 2, injectionTimeValueItem);

		// 显示按钮
		m_viewButton = new QPushButton("显示");
		m_tableWidget->setCellWidget(9, 2, m_viewButton);

		QStringList unitLabels = { " "," ","℃", "℃", "kg/min","MPa"," ","%","s"," " };
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
			m_insulationTempeRadioBtn->setChecked(true);
			m_pouringTempeRadioBtn = new QRadioButton();
			m_pouringSpeedRadioBtn = new QRadioButton();
			m_vacuumDegreeRadioBtn = new QRadioButton();

			m_tableWidget->setCellWidget(2, 4, createCenteredRadioWidget(m_insulationTempeRadioBtn));
			m_tableWidget->setCellWidget(3, 4, createCenteredRadioWidget(m_pouringTempeRadioBtn));
			m_tableWidget->setCellWidget(4, 4, createCenteredRadioWidget(m_pouringSpeedRadioBtn));
			m_tableWidget->setCellWidget(5, 4, createCenteredRadioWidget(m_vacuumDegreeRadioBtn));

			firRadioButtonGroup->addButton(m_insulationTempeRadioBtn);
			firRadioButtonGroup->addButton(m_pouringTempeRadioBtn);
			firRadioButtonGroup->addButton(m_pouringSpeedRadioBtn);
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
	connect(m_calButton, &QPushButton::clicked, this, &InReverseOptimizationPropertyWidget::showTableDialog);


}

void InReverseOptimizationPropertyWidget::showTableDialog() 
{
	QDialog* dialog = new QDialog();
	dialog->setWindowTitle("壳体材料");
	dialog->resize(1000, 500);
	QVBoxLayout* layout = new QVBoxLayout(this);

	QTableWidget* diaTableWidget = new QTableWidget();
	// 隐藏行号
	diaTableWidget->verticalHeader()->setVisible(false);
	// 隐藏列号
	diaTableWidget->horizontalHeader()->setVisible(false);
	QDir dir;
	QString filepath = dir.absoluteFilePath("src/database/壳体物性材料.xlsx");
	int m_rowCount = 0;

	if (!filepath.isEmpty()) {
		QXlsx::Document xlsx(filepath);
		int rowcount = xlsx.dimension().lastRow(); // 获取总行数
		int colcount = xlsx.dimension().lastColumn(); // 获取总列数
		m_rowCount = rowcount;

		diaTableWidget->setRowCount(rowcount);
		diaTableWidget->setColumnCount(colcount);

		for (int row = 1; row <= rowcount; ++row) {
			for (int col = 1; col <= colcount; ++col) {
				QTableWidgetItem* item = new QTableWidgetItem(xlsx.read(row, col).toString());
				item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
				diaTableWidget->setItem(row - 1, col - 1, item);
			}
		}
	}

	// 私有库
	auto ins = ModelDataManager::GetInstance();
	UserInfo info = ins->GetUserInfo();
	QString privateFilePath = dir.absoluteFilePath("src/database/" + info.username + "/壳体物性材料.xlsx");


	QFile file(privateFilePath);

	if (!privateFilePath.isEmpty() && file.exists()) {
		QXlsx::Document xlsx(privateFilePath);
		int rowcount = xlsx.dimension().lastRow(); // 获取总行数
		int colcount = xlsx.dimension().lastColumn(); // 获取总列数

		diaTableWidget->setRowCount(m_rowCount + rowcount - 1);

		int xlsxrow = m_rowCount;
		for (int row = 2; row <= rowcount; ++row) {
			for (int col = 1; col <= colcount; ++col) {
				QTableWidgetItem* item = new QTableWidgetItem(xlsx.read(row, col).toString());
				diaTableWidget->setItem(xlsxrow, col - 1, item);
			}
			xlsxrow++;
		}
	}

	//设置点击事件，双击单元格
	connect(diaTableWidget, &QTableWidget::cellDoubleClicked, this, [this, dialog, diaTableWidget](int row, int column) {
		if (row != 0)
		{
			int colcount = diaTableWidget->columnCount();
			QString value = "";
			for (int col = 1; col < colcount; ++col) {

				QString content = diaTableWidget->item(row, col)->text();
				if (col == 1)
				{
					value = content;
				}
				QTableWidgetItem* valueItem = new QTableWidgetItem(content);
				valueItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
				valueItem->setBackground(QBrush(QColor(230, 230, 230)));
				m_tableWidget->setItem(col, 2, valueItem);
			}
			auto ins = ModelDataManager::GetInstance();

			SteelPropertyInfo info;
			info.name = m_tableWidget->item(1, 2)->text();
			info.type = m_tableWidget->item(2, 2)->text();
			info.density = m_tableWidget->item(3, 2)->text().toDouble();
			info.thermalConductivity = m_tableWidget->item(4, 2)->text().toDouble();
			info.specificHeatCapacity = m_tableWidget->item(5, 2)->text().toDouble();
			info.isChecked = true;
			ins->SetSteelPropertyInfo(info);

			// 更新icon
			QWidget* parent = parentWidget();
			while (parent) {
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{
					gfParent->GetGFTreeModelWidget()->updataIcon();
					// 写入日志
					QDateTime currentTime = QDateTime::currentDateTime();
					QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
					auto logWidget = gfParent->GetLogWidget();
					auto textEdit = logWidget->GetTextEdit();
					QString text = timeStr + "[信息]>开始导入壳体物性材料数据";
					textEdit->appendPlainText(text);
					logWidget->update();

					// 关键：强制刷新UI，确保日志立即显示
					QApplication::processEvents();

					// 写入数据库模块
					MaterialPropertyWidget* m_materialPropertyWidget = gfParent->GetMaterialPropertyWidget();
					QTableWidget* materialTableWid = m_materialPropertyWidget->GetQTableWidget();
					QTableWidgetItem* valueItem = new QTableWidgetItem(value);
					valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable); // 不可编辑
					valueItem->setBackground(QBrush(QColor(230, 230, 230)));
					materialTableWid->setItem(1, 2, valueItem);
					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
		}
		dialog->close();

		});
	//双击单元格选中一行
	 //设置选中整行
	diaTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	diaTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	layout->addWidget(diaTableWidget);
	dialog->setLayout(layout);
	dialog->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除
	dialog->exec();
}


