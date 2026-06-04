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
	paramLayout->setSpacing(3);

	// 参数标题
	Fluent::FluentLabel* paramTitle = new Fluent::FluentLabel("模型参数");
	paramTitle->setAlignment(Qt::AlignCenter);
	paramTitle->setStyleSheet("font-size: 12px; font-weight: bold; color: #555; background: transparent;");
	paramLayout->addWidget(paramTitle);

	// 密度
	Fluent::FluentLabel* densityLabel = new Fluent::FluentLabel("密度 (kg/m³)");
	densityLabel->setStyleSheet("font-size: 11px; color: #666; background: transparent;");
	paramLayout->addWidget(densityLabel);
	m_pDensityEdit = new Fluent::FluentLineEdit();
	m_pDensityEdit->setPlaceholderText("请输入密度");
	m_pDensityEdit->setFixedHeight(26);
	paramLayout->addWidget(m_pDensityEdit);

	// 尺寸
	Fluent::FluentLabel* sizeLabel = new Fluent::FluentLabel("尺寸 (mm)");
	sizeLabel->setStyleSheet("font-size: 11px; color: #666; background: transparent;");
	paramLayout->addWidget(sizeLabel);
	QHBoxLayout* sizeLayout = new QHBoxLayout();
	sizeLayout->setSpacing(4);
	m_pLengthEdit = new Fluent::FluentLineEdit(); m_pLengthEdit->setPlaceholderText("长"); m_pLengthEdit->setFixedHeight(26);
	m_pWidthEdit = new Fluent::FluentLineEdit(); m_pWidthEdit->setPlaceholderText("宽");  m_pWidthEdit->setFixedHeight(26);
	m_pHeightEdit = new Fluent::FluentLineEdit(); m_pHeightEdit->setPlaceholderText("高"); m_pHeightEdit->setFixedHeight(26);
	sizeLayout->addWidget(m_pLengthEdit);
	sizeLayout->addWidget(m_pWidthEdit);
	sizeLayout->addWidget(m_pHeightEdit);
	paramLayout->addLayout(sizeLayout);

	// 体积
	Fluent::FluentLabel* volumeLabel = new Fluent::FluentLabel("体积 (m³)");
	volumeLabel->setStyleSheet("font-size: 11px; color: #666; background: transparent;");
	paramLayout->addWidget(volumeLabel);
	m_pVolumeEdit = new Fluent::FluentLineEdit();
	m_pVolumeEdit->setPlaceholderText("自动计算");
	m_pVolumeEdit->setFixedHeight(26);
	m_pVolumeEdit->setReadOnly(true);
	paramLayout->addWidget(m_pVolumeEdit);

	// 自动计算体积
	auto calcVolume = [&]() {
		double l = m_pLengthEdit->text().toDouble();
		double w = m_pWidthEdit->text().toDouble();
		double h = m_pHeightEdit->text().toDouble();
		if (l > 0 && w > 0 && h > 0) {
			m_pVolumeEdit->setText(QString::number(l * w * h * 1e-9, 'f', 6));
		}
		else {
			m_pVolumeEdit->clear();
		}
	};
	connect(m_pLengthEdit, &Fluent::FluentLineEdit::textChanged, this, calcVolume);
	connect(m_pWidthEdit, &Fluent::FluentLineEdit::textChanged, this, calcVolume);
	connect(m_pHeightEdit, &Fluent::FluentLineEdit::textChanged, this, calcVolume);

	// 参数容器整体水平居中
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
	Fluent::FluentButton* customBtn = new Fluent::FluentButton(("启动"));
	customBtn->setFixedHeight(42);
	customBtn->setStyleSheet(
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
	customLayout->addWidget(customBtn);
	connect(customBtn, &Fluent::FluentButton::clicked, this, [customBtn]() {
		// 禁用按钮，防止重复点击
		customBtn->setEnabled(false);

		// 堆上创建，关闭时自动销毁
		mainWidget* w = new mainWidget();
		w->setAttribute(Qt::WA_DeleteOnClose);

		// 窗口关闭后恢复按钮
		QObject::connect(w, &QObject::destroyed, customBtn, [customBtn]() {
			customBtn->setEnabled(true);
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