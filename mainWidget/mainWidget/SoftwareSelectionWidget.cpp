#pragma execution_character_set("utf-8")
#include "SoftwareSelectionWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProcess>
#include <QDebug>
#include "mainWidget.h"

SoftwareSelectionWidget::SoftwareSelectionWidget(QWidget* parent) : Fluent::FluentMainWindow(parent)
{
	init();
	bindConnect();
}

SoftwareSelectionWidget::~SoftwareSelectionWidget()
{
}

void SoftwareSelectionWidget::init()
{
	m_pMenuBar = new Fluent::FluentMenuBar();
	this->setFluentMenuBar(m_pMenuBar);

	QHBoxLayout* titleLayout = new QHBoxLayout();
	titleLayout->setSpacing(4);
	titleLayout->setContentsMargins(0, 0, 0, 0);

	// Logo 图标
	Fluent::FluentLabel* logoIcon = new Fluent::FluentLabel();
	logoIcon->setPixmap(QPixmap("C:/Users/yu_ch/Desktop/AllProject/TestCase/QtWidgetsApplication1/QtWidgetsApplication1/src/icon.jpg").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	logoIcon->setFixedSize(32, 32);
	titleLayout->addWidget(logoIcon);

	// 平台名称
	m_pTitleLabel = new Fluent::FluentLabel("软件选择");
	titleLayout->addWidget(m_pTitleLabel);

	QWidget* titleWidget = new QWidget();
	titleWidget->setLayout(titleLayout);
	titleWidget->setFixedWidth(350);
	titleWidget->setFixedHeight(32);
	this->setFluentTitleBarLeftWidget(titleWidget);

	// 主内容
	m_pCentralWidget = new Fluent::FluentWidget();
	m_pCentralWidget->setBackgroundRole(Fluent::FluentWidget::BackgroundRole::WindowBackground);
	this->setCentralWidget(m_pCentralWidget);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->setContentsMargins(80, 40, 80, 40);
	mainLayout->setSpacing(30);
	mainLayout->setAlignment(Qt::AlignCenter);

	// 页面标题
	Fluent::FluentLabel* title = new Fluent::FluentLabel(("请选择要启动的软件"));
	title->setAlignment(Qt::AlignCenter);
	title->setStyleSheet(
		("font-size: 22px; font-weight: bold; color: #333; background: transparent;"));
	mainLayout->addWidget(title);

	mainLayout->addSpacing(10);

	// ========== 卡片容器 ==========
	QHBoxLayout* cardsLayout = new QHBoxLayout();
	cardsLayout->setSpacing(20);
	cardsLayout->setAlignment(Qt::AlignCenter);

	// ----- Workbench 卡片 -----
	m_pWorkbenchCard = new Fluent::FluentWidget();
	m_pWorkbenchCard->setFixedSize(280, 420);
	m_pWorkbenchCard->setStyleSheet(
		"Fluent::FluentWidget {"
		"  background-color: white;"
		"  border: 1px solid #e0e0e0;"
		"  border-radius: 16px;"
		"}"
		"Fluent::FluentWidget:hover {"
		"  border: 2px solid #28afe9;"
		"  background-color: #f0f9ff;"
		"}"
	);

	QVBoxLayout* wbLayout = new QVBoxLayout(m_pWorkbenchCard);
	wbLayout->setContentsMargins(20, 20, 20, 20);
	wbLayout->setSpacing(4);
	wbLayout->setAlignment(Qt::AlignCenter);

	// 图标
	Fluent::FluentLabel* wbIcon = new Fluent::FluentLabel();
	wbIcon->setPixmap(QPixmap("C:/Users/yu_ch/Desktop/AllProject/TestCase/QtWidgetsApplication1/QtWidgetsApplication1/src/Workbench_48.bmp").scaled(56, 56, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	wbIcon->setAlignment(Qt::AlignCenter);
	wbIcon->setFixedHeight(60);
	wbLayout->addWidget(wbIcon, 0, Qt::AlignHCenter);

	// 名称
	Fluent::FluentLabel* wbName = new Fluent::FluentLabel("ANSYS Workbench");
	wbName->setAlignment(Qt::AlignCenter);
	wbName->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; background: transparent;");
	wbLayout->addWidget(wbName, 0, Qt::AlignHCenter);

	// 描述
	Fluent::FluentLabel* wbDesc = new Fluent::FluentLabel("多物理场仿真平台");
	wbDesc->setAlignment(Qt::AlignCenter);
	wbDesc->setStyleSheet("font-size: 11px; color: #888; background: transparent;");
	wbLayout->addWidget(wbDesc, 0, Qt::AlignHCenter);

	wbLayout->addSpacing(6);

	// ========== 参数区域（固定宽度 + 居中） ==========
	QWidget* paramBox = new QWidget();
	paramBox->setFixedWidth(230);
	QVBoxLayout* paramLayout = new QVBoxLayout(paramBox);
	paramLayout->setContentsMargins(0, 0, 0, 0);
	paramLayout->setSpacing(6);

	// 软件路径
	Fluent::FluentLabel* pathLabel = new Fluent::FluentLabel("软件路径");
	pathLabel->setStyleSheet("font-size: 11px; color: #666; background: transparent;");
	paramLayout->addWidget(pathLabel);
	m_pSoftwarePathEdit = new Fluent::FluentLineEdit();
	m_pSoftwarePathEdit->setPlaceholderText("请输入软件路径");
	m_pSoftwarePathEdit->setFixedHeight(26);
	paramLayout->addWidget(m_pSoftwarePathEdit);

	// 阀门开度
	Fluent::FluentLabel* valveLabel = new Fluent::FluentLabel("阀门开度");
	valveLabel->setStyleSheet("font-size: 11px; color: #666; background: transparent;");
	paramLayout->addWidget(valveLabel);
	m_pValveOpeningEdit = new Fluent::FluentLineEdit();
	m_pValveOpeningEdit->setPlaceholderText("请输入阀门开度");
	m_pValveOpeningEdit->setFixedHeight(26);
	paramLayout->addWidget(m_pValveOpeningEdit);

	// 壁厚
	Fluent::FluentLabel* wallLabel = new Fluent::FluentLabel("壁厚");
	wallLabel->setStyleSheet("font-size: 11px; color: #666; background: transparent;");
	paramLayout->addWidget(wallLabel);
	m_pWallThicknessEdit = new Fluent::FluentLineEdit();
	m_pWallThicknessEdit->setPlaceholderText("请输入壁厚");
	m_pWallThicknessEdit->setFixedHeight(26);
	paramLayout->addWidget(m_pWallThicknessEdit);

	// 胶层厚度
	Fluent::FluentLabel* bondLabel = new Fluent::FluentLabel("胶层厚度");
	bondLabel->setStyleSheet("font-size: 11px; color: #666; background: transparent;");
	paramLayout->addWidget(bondLabel);
	m_pBondlineThicknessEdit = new Fluent::FluentLineEdit();
	m_pBondlineThicknessEdit->setPlaceholderText("请输入胶层厚度");
	m_pBondlineThicknessEdit->setFixedHeight(26);
	paramLayout->addWidget(m_pBondlineThicknessEdit);

	// 保温温度
	Fluent::FluentLabel* tempLabel = new Fluent::FluentLabel("保温温度");
	tempLabel->setStyleSheet("font-size: 11px; color: #666; background: transparent;");
	paramLayout->addWidget(tempLabel);
	m_pInsulationTempEdit = new Fluent::FluentLineEdit();
	m_pInsulationTempEdit->setPlaceholderText("请输入保温温度");
	m_pInsulationTempEdit->setFixedHeight(26);
	paramLayout->addWidget(m_pInsulationTempEdit);

	wbLayout->addWidget(paramBox, 0, Qt::AlignHCenter);
	// ==============================================

	wbLayout->addStretch();

	// 启动按钮
	Fluent::FluentButton* wbBtn = new Fluent::FluentButton("启动");
	wbBtn->setFixedHeight(36);
	wbBtn->setStyleSheet(
		"Fluent::FluentButton {"
		"  background-color: #0078D4;"
		"  color: white;"
		"  border: none;"
		"  border-radius: 8px;"
		"  font-size: 14px;"
		"  font-weight: 500;"
		"}"
		"Fluent::FluentButton:hover { background-color: #106EBE; }"
		"Fluent::FluentButton:pressed { background-color: #005A9E; }"
	);
	wbLayout->addWidget(wbBtn, 0, Qt::AlignHCenter);
	connect(wbBtn, &Fluent::FluentButton::clicked, this, &SoftwareSelectionWidget::startWorkbench);

	cardsLayout->addWidget(m_pWorkbenchCard, 0, Qt::AlignBottom);

	// ----- Custom 平台卡片 -----
	m_pCustomCard = new Fluent::FluentWidget();
	m_pCustomCard->setFixedSize(280, 320);
	m_pCustomCard->setStyleSheet(
		(
			"Fluent::FluentWidget {"
			"  background-color: white;"
			"  border: 1px solid #e0e0e0;"
			"  border-radius: 16px;"
			"}"
			"Fluent::FluentWidget:hover {"
			"  border: 2px solid #00d4aa;"
			"  background-color: #f0fff9;"
			"}"
			)
	);

	QVBoxLayout* customLayout = new QVBoxLayout(m_pCustomCard);
	customLayout->setContentsMargins(30, 30, 30, 30);
	customLayout->setSpacing(15);
	customLayout->setAlignment(Qt::AlignCenter);

	// Custom 图标
	Fluent::FluentLabel* customIcon = new Fluent::FluentLabel();
	customIcon->setPixmap(QPixmap((":/images/custom_icon.png")).scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	customIcon->setAlignment(Qt::AlignCenter);
	customIcon->setFixedHeight(90);
	customLayout->addWidget(customIcon);

	// 名称
	Fluent::FluentLabel* customName = new Fluent::FluentLabel(("流体仿真平台"));
	customName->setAlignment(Qt::AlignCenter);
	customName->setStyleSheet(("font-size: 18px; font-weight: bold; color: #333; background: transparent;"));
	customLayout->addWidget(customName);

	// 描述
	Fluent::FluentLabel* customDesc = new Fluent::FluentLabel(("自主开发仿真系统"));
	customDesc->setAlignment(Qt::AlignCenter);
	customDesc->setStyleSheet(("font-size: 12px; color: #888; background: transparent;"));
	customLayout->addWidget(customDesc);

	customLayout->addStretch();

	// 启动按钮
	m_pCustomBtn = new Fluent::FluentButton(("启动"));
	m_pCustomBtn->setFixedHeight(42);
	m_pCustomBtn->setStyleSheet(
		(
			"Fluent::FluentButton {"
			"  background-color: #00a86b;"
			"  color: white;"
			"  border: none;"
			"  border-radius: 8px;"
			"  font-size: 14px;"
			"  font-weight: 500;"
			"}"
			"Fluent::FluentButton:hover { background-color: #00995e; }"
			"Fluent::FluentButton:pressed { background-color: #008a52; }"
			)
	);
	customLayout->addWidget(m_pCustomBtn);
	connect(m_pCustomBtn, &Fluent::FluentButton::clicked, this, [this]() {
		m_pCustomBtn->setEnabled(false);

		mainWidget* w = new mainWidget();
		w->resize(1200, 600);
		w->setAttribute(Qt::WA_DeleteOnClose);

		QObject::connect(w, &QObject::destroyed, m_pCustomBtn, [this]() {
			m_pCustomBtn->setEnabled(true);
			});

		w->show();
		});

	cardsLayout->addWidget(m_pCustomCard, 0, Qt::AlignBottom);

	mainLayout->addLayout(cardsLayout);
	mainLayout->addStretch();

	m_pCentralWidget->setLayout(mainLayout);
}

void SoftwareSelectionWidget::bindConnect()
{
}

void SoftwareSelectionWidget::startWorkbench()
{
	QString runwb2 = ("D:/Program Files/ANSYS Inc/v222/Framework/bin/Win64/runwb2.exe");
	QString scriptPath = ("D:/wb/C1-1.wbjn");

	QProcess* process = new QProcess(this);
	process->setWorkingDirectory(("D:/Program Files/ANSYS Inc/v222/Framework/bin/Win64"));
	process->setProgram(runwb2);
	process->setArguments(QStringList() << ("-R") << scriptPath);
	process->start();

	if (!process->waitForStarted(10000)) {
		qDebug() << ("启动失败:") << process->errorString();
		return;
	}

	qDebug() << ("Workbench 启动成功 PID:") << process->processId();

	connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		this, [](int exitCode, QProcess::ExitStatus status) {
			qDebug() << ("退出码:") << exitCode
				<< ("状态:") << (status == QProcess::NormalExit ? ("正常") : ("崩溃"));
		});
}