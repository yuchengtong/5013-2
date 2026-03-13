#pragma execution_character_set("utf-8")
#include "DatabaseWidget.h"
#include "ModelDataManager.h"
#include "xlsxdocument.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <qmessagebox.h>
#include <QDir>
#include <QToolTip>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QFileDialog>


void setTableCenter(QTableWidget* tableWidget) {
	// 遍历第一列的所有单元格，并设置内容居中
	for (int row = 0; row < tableWidget->rowCount(); ++row) {
		QTableWidgetItem* item = tableWidget->item(row, 0); // 获取第一列的单元格项
		if (item) {
			item->setTextAlignment(Qt::AlignCenter); // 设置文本居中
		}
	}
}


DatabaseWidget::DatabaseWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // tableWidge随窗口大小变化

	QTableWidget* tableWidget = ui.tableWidget;
	tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	tableWidget->setColumnWidth(0, 3);
	tableWidget->horizontalHeader()->setDefaultSectionSize(250); // 设置默认列宽
	tableWidget->verticalHeader()->setDefaultSectionSize(25); // 设置默认行高

	tableWidget->horizontalHeader()->setStretchLastSection(true);
	// 启用右键菜单
	tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	// 设置选择行为 - 选择整行
	tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	// 开启鼠标捕获
	tableWidget->setMouseTracking(true);

	QTreeWidget* treeWidget = ui.treeWidget;
	treeWidget->clear();

	QTreeWidgetItem* material = new QTreeWidgetItem(treeWidget);
	material->setText(0, "物性数据库");
	material->setIcon(0, QIcon(":/src/data_metals.svg"));

	QTreeWidgetItem* shell = new QTreeWidgetItem();
	shell->setText(0, "壳体物性");
	shell->setIcon(0, QIcon(":/src/data_metals.svg"));
	QTreeWidgetItem* medicine = new QTreeWidgetItem();
	medicine->setText(0, "药液物性");
	medicine->setIcon(0, QIcon(":/src/data_propellants.svg"));
	QTreeWidgetItem* gelatin = new QTreeWidgetItem();
	gelatin->setText(0, "明胶物性");
	gelatin->setIcon(0, QIcon(":/src/data_outheat.svg"));

	material->addChild(shell);
	material->addChild(medicine);
	material->addChild(gelatin);

	

	QTreeWidgetItem* calculation = new QTreeWidgetItem(treeWidget);
	calculation->setText(0, "计算模型数据库");
	calculation->setIcon(0, QIcon(":/src/data_calculation.svg"));
	QTreeWidgetItem* preheating = new QTreeWidgetItem();
	preheating->setText(0, "预热工艺模型");
	preheating->setIcon(0, QIcon(":/src/data_calculation.svg"));
	QTreeWidgetItem* intravenousInjection = new QTreeWidgetItem();
	intravenousInjection->setText(0, "注药工艺模型");
	intravenousInjection->setIcon(0, QIcon(":/src/data_calculation.svg"));

	calculation->addChild(preheating);
	calculation->addChild(intravenousInjection);
	

	QPushButton* addBtn = ui.addBtn;
	addBtn->setIcon(QIcon(":/src/data_add.svg"));
	QPushButton* removeBtn = ui.removeBtn;
	removeBtn->setIcon(QIcon(":/src/data_remove.svg"));
	QPushButton* queryBtn = ui.queryBtn;
	queryBtn->setIcon(QIcon(":/src/data_query.svg"));
	QPushButton* saveBtn = ui.saveBtn;
	saveBtn->setIcon(QIcon(":/src/data_save.svg"));
	QPushButton* importBtn = ui.importBtn;
	importBtn->setIcon(QIcon(":/src/data_import.svg"));
	QPushButton* exportBtn = ui.exportBtn;
	exportBtn->setIcon(QIcon(":/src/data_export.svg"));
	QPushButton* resetBtn = ui.resetBtn;
	resetBtn->setIcon(QIcon(":/src/data_reset.svg"));
	QLineEdit* keyword = ui.queryName;


	// TreeItem信号槽
	connect(treeWidget, &QTreeWidget::itemClicked, this, &DatabaseWidget::onTreeItemClicked);

	// tableWidget信号槽
	connect(tableWidget, &QTableWidget::itemEntered, this, &DatabaseWidget::showTooltip);

	// 连接右键菜单信号
	connect(tableWidget, &QTableWidget::customContextMenuRequested,
		this, [=](const QPoint& pos) {
			// 获取右键点击的单元格
			QTableWidgetItem* item = tableWidget->itemAt(pos);
			if (!item) return;
			int row = item->row();
			if (row == 0) return;
			QTableWidgetItem* index = tableWidget->item(row, 0);
			if (!index) return;

			tableWidget->setCurrentCell(row, 0); // 选中整行
			contextMenu = new QMenu(this); // 创建菜单对象
			QAction* addAction = new QAction("新增", this);
			addAction->setIcon(QIcon(":/src/data_add.svg"));
			QAction* copyAction = new QAction("复制", this);
			copyAction->setIcon(QIcon(":/src/data_copy.svg"));
			QAction* removeAction = new QAction("删除", this);
			removeAction->setIcon(QIcon(":/src/data_remove.svg"));
			QAction* importAction = new QAction("导入", this);
			importAction->setIcon(QIcon(":/src/data_import.svg"));
			QAction* exportAction = new QAction("导出", this);
			exportAction->setIcon(QIcon(":/src/data_export.svg"));


			connect(addAction, &QAction::triggered, this, [this, row, tableWidget]() {
				int rowCount = tableWidget->rowCount(); // 获取总行数
				int colCount = tableWidget->columnCount(); // 获取总列数
				tableWidget->insertRow(rowCount);
				QTableWidgetItem* item = new QTableWidgetItem(QString::number(rowCount));
				item->setTextAlignment(Qt::AlignCenter);
				tableWidget->setItem(rowCount, 0, item);
				m_rowCount = m_rowCount + 1;
				// 滚动到新行
				tableWidget->scrollToBottom();
				tableWidget->selectRow(rowCount);
				});

			connect(copyAction, &QAction::triggered, this, [this, row, tableWidget]() {
				int rowCount = tableWidget->rowCount(); // 获取总行数
				int colCount = tableWidget->columnCount(); // 获取总列数
				// 在末尾添加新行
				tableWidget->insertRow(rowCount);
				// 复制数据
				for (int col = 0; col < m_columnCount; ++col) {
					if (col == 0)
					{
						QTableWidgetItem* item = new QTableWidgetItem(QString::number(rowCount));
						item->setTextAlignment(Qt::AlignCenter);
						tableWidget->setItem(rowCount, col, item);
					}
					else
					{
						QTableWidgetItem* sourceItem = tableWidget->item(row, col);
						if (sourceItem) {
							tableWidget->setItem(rowCount, col, new QTableWidgetItem(sourceItem->text()));
						}
					}

				}
				m_rowCount = m_rowCount + 1;
				// 滚动到新行
				tableWidget->scrollToBottom();
				tableWidget->selectRow(m_rowCount);
				});

			connect(removeAction, &QAction::triggered, this, [this, row, tableWidget]() {
				QList<QTableWidgetItem*> items = tableWidget->selectedItems();
				QSet<int> rows;
				foreach(QTableWidgetItem * item, items)
				{
					if (row != 0 || row > m_rowCount)
					{
						rows.insert(item->row());
					}
				}
				foreach(int r, rows) {
					tableWidget->removeRow(r);
				}
				rows.clear();
				});

			connect(importAction, &QAction::triggered, this, &DatabaseWidget::importData);
			connect(exportAction, &QAction::triggered, this, &DatabaseWidget::exportData);


			contextMenu->addAction(addAction); // 将动作添加到菜单中
			contextMenu->addAction(copyAction);
			contextMenu->addAction(removeAction);
			contextMenu->addAction(importAction);
			contextMenu->addAction(exportAction);
			contextMenu->exec(tableWidget->mapToGlobal(pos));

		});

	// 新增
	QObject::connect(addBtn, &QPushButton::clicked, [this, tableWidget]() {
		int rowCount = tableWidget->rowCount(); // 获取总行数
		int colCount = tableWidget->columnCount(); // 获取总列数
		tableWidget->insertRow(rowCount);
		QTableWidgetItem* item = new QTableWidgetItem(QString::number(rowCount));
		item->setTextAlignment(Qt::AlignCenter);
		tableWidget->setItem(rowCount, 0, item);
		m_rowCount = m_rowCount + 1;
		// 滚动到新行
		tableWidget->scrollToBottom();
		tableWidget->selectRow(rowCount);
		});

	// 删除
	QObject::connect(removeBtn, &QPushButton::clicked, [this, tableWidget]() {
		QList<QTableWidgetItem*> items = tableWidget->selectedItems();
		QSet<int> rows;
		foreach(QTableWidgetItem * item, items)
		{
			if (item->row() != 0 || item->row() > m_rowCount)
			{
				rows.insert(item->row());
			}
		}
		foreach(int row, rows) {
			tableWidget->removeRow(row);
		}
		rows.clear();
		m_rowCount = m_rowCount - 1;
		});

	// 导入
	connect(importBtn, &QPushButton::clicked, this, &DatabaseWidget::importData);

	// 导出
	connect(exportBtn, &QPushButton::clicked, this, &DatabaseWidget::exportData);

	// 查询
	QObject::connect(queryBtn, &QPushButton::clicked, [this, tableWidget, keyword]() {
		QString value = keyword->text();
		if (value.isEmpty()) {
			// 显示所有行
			for (int row = 0; row < tableWidget->rowCount(); ++row) {
				tableWidget->setRowHidden(row, false);
			}
			return;
		}
		for (int row = 1; row < tableWidget->rowCount(); ++row) {
			// 获取第二列的内容
			QTableWidgetItem* item = tableWidget->item(row, 1);
			if (!item) continue;

			// 检查是否包含关键词
			bool match = item->text().contains(value, Qt::CaseInsensitive);
			tableWidget->setRowHidden(row, !match);

		}
		});

	// 查询
	QObject::connect(resetBtn, &QPushButton::clicked, [this, tableWidget]() {
		for (int row = 0; row < tableWidget->rowCount(); ++row) {
			tableWidget->setRowHidden(row, false);
		}
		});

	// 保存
	QObject::connect(saveBtn, &QPushButton::clicked, [this, tableWidget]() {
		QString filepath = nullptr;
		QString sheetName = nullptr;
		QDir dir;

		if (currentDataaseType == "物性数据库" || currentDataaseType == "壳体物性")
		{
			filepath = dir.absoluteFilePath(m_privateDirPath + "/壳体物性材料.xlsx");
			sheetName = "壳体物性材料";
		}
		else if (currentDataaseType == "药液物性")
		{
			filepath = dir.absoluteFilePath(m_privateDirPath + "/药液物性材料.xlsx");
			sheetName = "药液物性材料";
		}
		else if (currentDataaseType == "明胶物性")
		{
			filepath = dir.absoluteFilePath(m_privateDirPath + "/明胶物性材料.xlsx");
			sheetName = "明胶物性材料";
		}
		
		else if (currentDataaseType == "预热工艺模型" || currentDataaseType == "计算模型数据库")
		{
			filepath = dir.absoluteFilePath(m_privateDirPath + "/计算模型-预热工艺模型.xlsx");
			sheetName = "预热工艺模型";
		}
		else if (currentDataaseType == "注药工艺模型")
		{
			filepath = dir.absoluteFilePath(m_privateDirPath + "/计算模型-注药工艺模型.xlsx");
			sheetName = "注药工艺模型";
		}
		
		
		int rowCount = tableWidget->rowCount();
		int colCount = tableWidget->columnCount();
		// 检查表格是否有数据
		if (rowCount <= 1) {
			return;
		}

		// 创建Excel文档
		QXlsx::Document xlsx;
		// 设置工作表名称
		xlsx.renameSheet("Sheet1", sheetName);
		// 导出表格数据
		int xlsxRow = 0;
		for (int row = 0; row < rowCount; ++row) {
			if (row == 0 || row >= m_publicRowCount)
			{
				for (int col = 0; col < colCount; ++col) {
					QTableWidgetItem* item = tableWidget->item(row, col);
					if (item) {
						xlsx.write(xlsxRow + 1, col + 1, item->text());

					}
				}
				xlsxRow++;
			}
		}
		// 保存文件
		if (xlsx.saveAs(filepath)) {
			QMessageBox::information(this, "标题", "保存成功");
		}
		else {
			QMessageBox::critical(this, "标题", "保存失败");
		}
		});

}

DatabaseWidget::~DatabaseWidget()
{}

QTableWidget* DatabaseWidget::getTableWid()
{
	return ui.tableWidget;
}
QTreeWidget* DatabaseWidget::getQTreeWid()
{
	return ui.treeWidget;
}

void DatabaseWidget::onTreeItemClicked(QTreeWidgetItem* item) {

	QTableWidget* tableWidge = getTableWid();
	tableWidge->setStyleSheet(R"(
        QTableWidget::item {
            white-space: nowrap; /* 禁止文本换行 */
            text-overflow: ellipsis; /* 可选：文本过长时显示省略号（...） */
        }
    )");

	QString filepath = nullptr;
	QDir dir;
	currentDataaseType = item->text(0);
	if (currentDataaseType == "物性数据库" || currentDataaseType == "壳体物性")
	{
		filepath = dir.absoluteFilePath("src/database/壳体物性材料.xlsx");
		ui.queryTitle->setText("材料名称");
	}
	else if (currentDataaseType == "药液物性")
	{
		filepath = dir.absoluteFilePath("src/database/药液物性材料.xlsx");
		ui.queryTitle->setText("推进剂名称");
	}
	else if (currentDataaseType == "明胶物性")
	{
		filepath = dir.absoluteFilePath("src/database/明胶物性材料.xlsx");
		ui.queryTitle->setText("明胶牌号");
	}
	
	else if (currentDataaseType == "预热工艺模型" || currentDataaseType == "计算模型数据库")
	{
		filepath = dir.absoluteFilePath("src/database/计算模型-预热工艺模型.xlsx");
		ui.queryTitle->setText("材料牌号");
	}

	else if (currentDataaseType == "注药工艺模型")
	{
		filepath = dir.absoluteFilePath("src/database/计算模型-注药工艺模型.xlsx");
		ui.queryTitle->setText("材料牌号");
	}
	

	if (!filepath.isEmpty()) {
		QXlsx::Document xlsx(filepath);
		int rowcount = xlsx.dimension().lastRow(); // 获取总行数
		int colcount = xlsx.dimension().lastColumn(); // 获取总列数

		m_rowCount = rowcount;
		m_columnCount = colcount;
		m_publicRowCount = rowcount;

		tableWidge->setRowCount(rowcount);
		tableWidge->setColumnCount(colcount);

		for (int row = 1; row <= rowcount; ++row) {
			for (int col = 1; col <= colcount; ++col) {
				std::shared_ptr<QXlsx::Cell> cell = xlsx.cellAt(row, col);

				QTableWidgetItem* item = new QTableWidgetItem(xlsx.read(row, col).toString());
				item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 不可编辑
				item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				if (row == 1)
				{
					item->setBackground(QBrush(QColor(0, 237, 252)));
				}
				else
				{
					item->setBackground(QBrush(QColor(230, 230, 230)));
				}
				tableWidge->setItem(row - 1, col - 1, item);
				
			}
		}
	}
	// 私有库
	auto ins = ModelDataManager::GetInstance();
	UserInfo info = ins->GetUserInfo();
	m_privateDirPath = "src/database/" + info.username;
	QDir privateDir(m_privateDirPath);
	if (!privateDir.exists()) {
		privateDir.setPath("src/database/");
		if (!privateDir.mkpath(info.username)) {
			QMessageBox::warning(this, "操作失败", "读取用户私有库失败");
			return;
		}
	}
	QString privateFilePath = "";

	if (currentDataaseType == "物性数据库" || currentDataaseType == "壳体物性")
	{
		privateFilePath = dir.absoluteFilePath(m_privateDirPath + "/壳体物性材料.xlsx");
	}
	else if (currentDataaseType == "药液物性")
	{
		privateFilePath = dir.absoluteFilePath(m_privateDirPath + "/药液物性材料.xlsx");
	}
	else if (currentDataaseType == "明胶物性")
	{
		privateFilePath = dir.absoluteFilePath(m_privateDirPath + "/明胶物性.xlsx");
	}
	else if (currentDataaseType == "预热工艺模型" || currentDataaseType == "计算模型数据库")
	{
		privateFilePath = dir.absoluteFilePath(m_privateDirPath + "/计算模型-预热工艺模型.xlsx");
	}
	
	else if (currentDataaseType == "注药工艺模型")
	{
		privateFilePath = dir.absoluteFilePath(m_privateDirPath + "/计算模型-注药工艺模型.xlsx");
	}
	

	QFile file(privateFilePath);

	if (!privateFilePath.isEmpty() && file.exists()) {
		QXlsx::Document xlsx(privateFilePath);
		int rowcount = xlsx.dimension().lastRow(); // 获取总行数
		int colcount = xlsx.dimension().lastColumn(); // 获取总列数

		tableWidge->setRowCount(m_rowCount + rowcount - 1);

		int xlsxrow = m_rowCount;
		for (int row = 2; row <= rowcount; ++row) {
			for (int col = 1; col <= colcount; ++col) {
				if (col == 1)
				{
					QTableWidgetItem* item = new QTableWidgetItem(QString::number(xlsxrow));
					tableWidge->setItem(xlsxrow, col - 1, item);
				}
				else
				{
					QTableWidgetItem* item = new QTableWidgetItem(xlsx.read(row, col).toString());
					tableWidge->setItem(xlsxrow, col - 1, item);
				}

			}
			xlsxrow++;
		}
	}

	setTableCenter(tableWidge);

}


void DatabaseWidget::showTooltip(QTableWidgetItem* item)
{
	//int columnWidth = this->ui.tableWidget->columnWidth(2);
	if (QToolTip::isVisible())
	{
		QToolTip::hideText();
	}
	if (item == NULL) {
		return;
	}
	QFontMetrics fontMetrics(item->font());
	int fontSize = fontMetrics.width(item->text());
	if (fontSize > (this->ui.tableWidget->columnWidth(this->ui.tableWidget->column(item))))
	{
		QToolTip::showText(QCursor::pos(), item->text());
	}
}

void DatabaseWidget::importData()
{
	QString filePath = QFileDialog::getOpenFileName(
		this, "选择XLSX文件", "", "XLSX文件 (*.xlsx)"
	);

	if (filePath.isEmpty()) return;

	try {
		// 打开XLSX文件
		QXlsx::Document xlsx(filePath);
		QXlsx::Worksheet* sheet = xlsx.currentWorksheet();

		if (!sheet) {
			QMessageBox::warning(this, "错误", "无法打开工作表");
			return;
		}


		// 获取最大行数和列数
		int maxRow = sheet->dimension().rowCount();
		int maxCol = sheet->dimension().columnCount();

		if (maxRow == 0 || maxCol == 0) {
			QMessageBox::information(this, "提示", "文件内容为空");
			return;
		}
		QTableWidget* tableWidget = getTableWid();
		// 设置表格行列数
		tableWidget->setRowCount(maxRow + m_rowCount - 1);

		// 读取数据填充表格
		for (int row = 2; row <= maxRow; row++) {  // XLSX行号从1开始
			for (int col = 1; col <= m_columnCount; col++) {  // XLSX列号从1开始
				std::shared_ptr<QXlsx::Cell> cell = sheet->cellAt(row, col);
				if (cell) {
					if (col == 1)
					{
						QTableWidgetItem* item = new QTableWidgetItem(QString::number(m_rowCount));
						item->setTextAlignment(Qt::AlignCenter);
						tableWidget->setItem(m_rowCount, col - 1, item);
					}
					else
					{
						QTableWidgetItem* item = new QTableWidgetItem(cell->value().toString());
						tableWidget->setItem(m_rowCount, col - 1, item);
					}

				}
			}
			m_rowCount = m_rowCount + 1;
		}

		QMessageBox::information(this, "成功", "导入成功");
	}
	catch (...) {
		QMessageBox::critical(this, "错误", "导入失败，请检查文件格式");
	}
}

void DatabaseWidget::exportData()
{
	QString filePath = QFileDialog::getSaveFileName(
		this, "保存XLSX文件", "data.xlsx", "XLSX文件 (*.xlsx)"
	);

	if (filePath.isEmpty()) return;

	// 确保文件扩展名正确
	if (!filePath.endsWith(".xlsx", Qt::CaseInsensitive)) {
		filePath += ".xlsx";
	}

	try {
		QXlsx::Document xlsx;
		QXlsx::Worksheet* sheet = xlsx.currentWorksheet();

		QTableWidget* tableWidget = getTableWid();
		int rowCount = tableWidget->rowCount();
		int colCount = tableWidget->columnCount();

		// 写入表头
		for (int col = 0; col < colCount; col++) {
			QTableWidgetItem* item = tableWidget->item(0, col);
			if (item) {
				sheet->write(1, col + 1, item->text());
			}
		}
		// 写入表格数据
		auto ins = ModelDataManager::GetInstance();
		UserInfo info = ins->GetUserInfo();
		int realRow = 0;
		int tempPublicRowCount = m_publicRowCount;
		if (info.username == "admin")
		{
			tempPublicRowCount = 1;
		}
		
		for (int row = 0; row < rowCount; row++) {
			if (row == 0 || row >= tempPublicRowCount)
			{
				for (int col = 0; col < colCount; col++) {
					if (col == 0 && row > 0)
					{
						sheet->write(realRow + 1, col + 1, realRow);
					}
					else
					{
						QTableWidgetItem* item = tableWidget->item(row, col);
						if (item) {
							sheet->write(realRow + 1, col + 1, item->text());
						}
					}

				}
				realRow = realRow + 1;
			}

		}


		// 保存文件
		if (xlsx.saveAs(filePath)) {
			QMessageBox::information(this, "成功", "导出成功：" + filePath);
		}
		else {
			QMessageBox::warning(this, "失败", "导出失败");
		}
	}
	catch (...) {
		QMessageBox::critical(this, "错误", "导出失败");
	}
}


// 控件根据窗口大小自动调整控件大小 @{
void DatabaseWidget::resizeEvent(QResizeEvent* event)
{
	double scaleX = event->size().width() * 1.0 / windowOriginalSize.width();
	double scaleY = event->size().height() * 1.0 / windowOriginalSize.height();
	for (auto iter = allLayoutMap.begin(); iter != allLayoutMap.end(); ++iter) {
		QLayout* layout = iter.key();
		QRect originalGeometry = iter.value();
		QRect newGeometry(
			originalGeometry.x() * scaleX,
			originalGeometry.y() * scaleY,
			originalGeometry.width() * scaleX,
			originalGeometry.height() * scaleY
		);
		layout->setGeometry(newGeometry);
	}

	for (auto iter = allWidgetMap.begin(); iter != allWidgetMap.end(); ++iter) {
		QWidget* widget = iter.key();
		QRect originalGeometry = iter.value();
		QRect newGeometry(
			originalGeometry.x() * scaleX,
			originalGeometry.y() * scaleY,
			originalGeometry.width() * scaleX,
			originalGeometry.height() * scaleY
		);
		widget->setGeometry(newGeometry);

		QPushButton* btn = dynamic_cast<QPushButton*>(widget);
		// 调整按钮字体大小
		if (btn != NULL) {
			QFont font = btn->font();
			// 根据按钮的宽度/高度调整字体大小，可以根据默认控件的高度和字体的大小比率进行适当调整
			font.setPointSize(btn->height() / 3);
			btn->setFont(font);
		}
		// QLabel* label = dynamic_cast<QLabel*>(widget);
		// 调整标签字体大小
		// if (label != NULL) {
		//     QFont font = label->font();
		//     // 根据按钮的宽度/高度调整字体大小，可以根据默认控件的高度和字体的大小比率进行适当调整
		//     font.setPointSize(label->height() / 2);
		//     label->setFont(font);
		// }
	}
}

void DatabaseWidget::findAllLayoutAndWidget(QObject* object) {
	QLayout* layout = qobject_cast<QLayout*>(object);
	QWidget* widget = qobject_cast<QWidget*>(object);
	if (layout) {
		if (layout->objectName() != "" && !allLayoutMap.contains(layout)) {
			allLayoutMap.insert(layout, layout->geometry());
		}
		for (int i = 0; i < layout->count(); ++i) {
			findAllLayoutAndWidget(layout->itemAt(i)->widget());
		}
	}
	else if (widget) {
		if (widget != this && widget->objectName() != "" && !allWidgetMap.contains(widget)) {
			allWidgetMap.insert(widget, widget->geometry());
		}
		for (int i = 0; i < widget->children().size(); ++i) {
			findAllLayoutAndWidget(widget->children().at(i));
		}
	}
}
// 控件根据窗口大小自动调整控件大小 @}

