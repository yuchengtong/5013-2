#pragma execution_character_set("utf-8")
#include "PreheatingParamWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

PreheatingParamWidget::PreheatingParamWidget(QWidget* parent) : QWidget(parent)
{
	initUI();
	bindConnect();
}

PreheatingParamWidget::~PreheatingParamWidget()
{
}

void PreheatingParamWidget::initUI()
{
	setWindowTitle("预热工艺参数设置");
	this->setWindowIcon(QIcon(":/selectWidget/src/selectWidget/PreheatingProcess.jpg"));
	setFixedSize(520, 420);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(30, 24, 30, 24);
	mainLayout->setSpacing(16);

	// ========== 标题 ==========
	QLabel* title = new QLabel("预热工艺参数配置");
	title->setAlignment(Qt::AlignCenter);
	title->setStyleSheet("font-size: 18px; font-weight: bold; color: #333333; background: transparent;");
	mainLayout->addWidget(title);

	mainLayout->addSpacing(8);

	// ========== 参数表单区域 ==========
	QWidget* formWidget = new QWidget();
	formWidget->setStyleSheet(R"(
		QWidget { background-color: #ffffff; border: 1px solid #e0e0e0; border-radius: 8px; }
		QLineEdit {
			background-color: #ffffff;
			border: 1px solid #d0d0d0;
			border-radius: 4px;
			padding: 6px;
			font-size: 13px;
			color: #333333;
		}
		QLineEdit:focus { border: 1px solid #0078D4; }
	)");

	QGridLayout* formLayout = new QGridLayout(formWidget);
	formLayout->setContentsMargins(20, 16, 20, 16);
	formLayout->setSpacing(12);
	formLayout->setColumnStretch(1, 1);

	int row = 0;

	// Workbench 路径
	QLabel* pathLabel = new QLabel("Workbench 路径:");
	pathLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	pathLabel->setStyleSheet("background: transparent; color: #555555; font-size: 13px;");

	m_pWorkbenchPathEdit = new QLineEdit();
	m_pWorkbenchPathEdit->setPlaceholderText("请选择 runwb2.exe 路径");
	m_pWorkbenchPathEdit->setReadOnly(true);
	m_pBrowseBtn = new QPushButton("浏览...");
	m_pBrowseBtn->setFixedWidth(60);
	m_pBrowseBtn->setFixedHeight(28);
	m_pBrowseBtn->setStyleSheet(R"(
		QPushButton {
			background-color: #f0f0f0;
			border: 1px solid #d0d0d0;
			border-radius: 4px;
			font-size: 12px;
			color: #333333;
		}
		QPushButton:hover { background-color: #e0e0e0; }
		QPushButton:pressed { background-color: #d0d0d0; }
	)");

	QHBoxLayout* pathLayout = new QHBoxLayout();
	pathLayout->setSpacing(8);
	pathLayout->setContentsMargins(0, 0, 0, 0);
	pathLayout->addWidget(m_pWorkbenchPathEdit);
	pathLayout->addWidget(m_pBrowseBtn);

	formLayout->addWidget(pathLabel, row, 0, Qt::AlignRight | Qt::AlignVCenter);
	formLayout->addLayout(pathLayout, row, 1);
	++row;

	// 阀门开度
	QLabel* valveLabel = new QLabel("阀门开度:");
	valveLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	valveLabel->setStyleSheet("background: transparent; color: #555555; font-size: 13px;");

	m_pValveOpeningEdit = new QLineEdit();
	m_pValveOpeningEdit->setPlaceholderText("例如: 0.75");
	formLayout->addWidget(valveLabel, row, 0, Qt::AlignRight | Qt::AlignVCenter);
	formLayout->addWidget(m_pValveOpeningEdit, row, 1);
	++row;

	// 壁厚
	QLabel* wallLabel = new QLabel("壁厚 (mm):");
	wallLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	wallLabel->setStyleSheet("background: transparent; color: #555555; font-size: 13px;");

	m_pWallThicknessEdit = new QLineEdit();
	m_pWallThicknessEdit->setPlaceholderText("例如: 12.5");
	formLayout->addWidget(wallLabel, row, 0, Qt::AlignRight | Qt::AlignVCenter);
	formLayout->addWidget(m_pWallThicknessEdit, row, 1);
	++row;

	// 胶层厚度
	QLabel* bondLabel = new QLabel("胶层厚度 (mm):");
	bondLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	bondLabel->setStyleSheet("background: transparent; color: #555555; font-size: 13px;");

	m_pBondlineThicknessEdit = new QLineEdit();
	m_pBondlineThicknessEdit->setPlaceholderText("例如: 2.0");
	formLayout->addWidget(bondLabel, row, 0, Qt::AlignRight | Qt::AlignVCenter);
	formLayout->addWidget(m_pBondlineThicknessEdit, row, 1);
	++row;

	// 药液温度
	QLabel* liquidLabel = new QLabel("药液温度 (℃):");
	liquidLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	liquidLabel->setStyleSheet("background: transparent; color: #555555; font-size: 13px;");

	m_pLiquidTempEdit = new QLineEdit();
	m_pLiquidTempEdit->setPlaceholderText("例如: 85");
	formLayout->addWidget(liquidLabel, row, 0, Qt::AlignRight | Qt::AlignVCenter);
	formLayout->addWidget(m_pLiquidTempEdit, row, 1);
	++row;

	// 保温温度
	QLabel* tempLabel = new QLabel("保温温度 (℃):");
	tempLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	tempLabel->setStyleSheet("background: transparent; color: #555555; font-size: 13px;");

	m_pInsulationTempEdit = new QLineEdit();
	m_pInsulationTempEdit->setPlaceholderText("例如: 65");
	formLayout->addWidget(tempLabel, row, 0, Qt::AlignRight | Qt::AlignVCenter);
	formLayout->addWidget(m_pInsulationTempEdit, row, 1);
	++row;

	mainLayout->addWidget(formWidget);

	mainLayout->addStretch();

	// ========== 启动按钮 ==========
	m_pStartBtn = new QPushButton("启动");
	m_pStartBtn->setFixedHeight(40);
	m_pStartBtn->setCursor(Qt::PointingHandCursor);
	m_pStartBtn->setStyleSheet(R"(
		QPushButton {
			background-color: #0078D4;
			color: white;
			border: none;
			border-radius: 6px;
			font-size: 14px;
			font-weight: bold;
		}
		QPushButton:hover { background-color: #106EBE; }
		QPushButton:pressed { background-color: #005A9E; }
		QPushButton:disabled {
			background-color: #cccccc;
			color: #888888;
		}
	)");
	mainLayout->addWidget(m_pStartBtn, 0, Qt::AlignHCenter);

	// 窗口背景
	setStyleSheet("PreheatingParamWidget { background-color: #f5f6f7; }");
}

void PreheatingParamWidget::bindConnect()
{
	// 浏览按钮：选择 Workbench 路径
	connect(m_pBrowseBtn, &QPushButton::clicked, this, [this]() {
		QString filePath = QFileDialog::getOpenFileName(this,
			"选择 Workbench 启动程序",
			QDir::homePath(),
			"Executable Files (*.exe);;All Files (*.*)");
		if (!filePath.isEmpty()) {
			m_pWorkbenchPathEdit->setText(filePath);
		}
		});

	// 启动按钮
	connect(m_pStartBtn, &QPushButton::clicked, this, [this]() {
		// 简单校验
		if (m_pWorkbenchPathEdit->text().isEmpty()) {
			QMessageBox::warning(this, "提示", "请先选择 Workbench 路径！");
			return;
		}
		if (m_pValveOpeningEdit->text().isEmpty() ||
			m_pWallThicknessEdit->text().isEmpty() ||
			m_pBondlineThicknessEdit->text().isEmpty() ||
			m_pLiquidTempEdit->text().isEmpty() ||
			m_pInsulationTempEdit->text().isEmpty()) {
			QMessageBox::warning(this, "提示", "请填写完整的工艺参数！");
			return;
		}

		QMessageBox::information(this, "启动", "参数已确认，准备启动 Workbench...");
		});
}