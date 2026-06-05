#pragma execution_character_set("utf-8")
#include "SoftwareSelectionWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QProcess>
#include <QDebug>
#include "mainWidget.h"
#include "PreheatingParamWidget.h"
//#include "InjectionParamWidget.h"

SoftwareSelectionWidget::SoftwareSelectionWidget(QWidget* parent) : QMainWindow(parent)
{
	init();
	bindConnect();
}

SoftwareSelectionWidget::~SoftwareSelectionWidget()
{
}

void SoftwareSelectionWidget::init()
{
	setWindowIcon(QIcon(":/src/HongYuLogo.jpg"));
	setWindowTitle("软件选择");
	// 窗口缩小，刚好容纳 3 个按钮 + 标题
	setFixedSize(560, 280);

	// 中央容器
	QWidget* centralWidget = new QWidget(this);
	setCentralWidget(centralWidget);

	QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
	mainLayout->setContentsMargins(20, 16, 20, 16);
	mainLayout->setSpacing(0);

	// 标题
	QLabel* title = new QLabel("请选择要启动的软件");
	title->setAlignment(Qt::AlignCenter);
	title->setStyleSheet("font-size: 16px; font-weight: bold; color: #333333; background: transparent;");
	mainLayout->addWidget(title);

	mainLayout->addSpacing(20);  // 标题和按钮之间间距缩小

	// 3 个按钮的水平布局
	QHBoxLayout* btnLayout = new QHBoxLayout();
	btnLayout->setSpacing(24);   // 按钮间距缩小
	btnLayout->setAlignment(Qt::AlignCenter);

	// ---------- 按钮1：预热工艺 ----------
	m_pPreheatingBtn = new QPushButton();
	m_pPreheatingBtn->setFixedSize(128, 128);
	m_pPreheatingBtn->setIcon(QIcon(":/selectWidget/src/selectWidget/PreheatingProcess.jpg"));
	m_pPreheatingBtn->setCursor(Qt::PointingHandCursor);
	m_pPreheatingBtn->setStyleSheet(R"(
		QPushButton {
			background-color: transparent;
			border: 2px solid #e0e0e0;
			border-radius: 0px;
			padding: 0px;
			margin: 0px;
			icon-size: 124px 124px;
		}
		QPushButton:hover {
			border: 2px solid #e67e22;
		}
		QPushButton:pressed {
			border: 2px solid #d35400;
		}
	)");
	btnLayout->addWidget(m_pPreheatingBtn);

	// ---------- 按钮2：注装工艺 ----------
	m_pInjectionBtn = new QPushButton();
	m_pInjectionBtn->setFixedSize(128, 128);
	m_pInjectionBtn->setIcon(QIcon(":/selectWidget/src/selectWidget/InjectionProcess.jpg"));
	m_pInjectionBtn->setCursor(Qt::PointingHandCursor);
	m_pInjectionBtn->setStyleSheet(R"(
		QPushButton {
			background-color: transparent;
			border: 2px solid #e0e0e0;
			border-radius: 0px;
			padding: 0px;
			margin: 0px;
			icon-size: 124px 124px;
		}
		QPushButton:hover {
			border: 2px solid #0078D4;
		}
		QPushButton:pressed {
			border: 2px solid #005A9E;
		}
	)");
	btnLayout->addWidget(m_pInjectionBtn);

	// ---------- 按钮3：流体仿真平台 ----------
	m_pCustomBtn = new QPushButton();
	m_pCustomBtn->setFixedSize(128, 128);
	m_pCustomBtn->setIcon(QIcon(":/selectWidget/src/selectWidget/Custom.png"));
	m_pCustomBtn->setCursor(Qt::PointingHandCursor);
	m_pCustomBtn->setStyleSheet(R"(
		QPushButton {
			background-color: transparent;
			border: 2px solid #e0e0e0;
			border-radius: 0px;
			padding: 0px;
			margin: 0px;
			icon-size: 124px 124px;
		}
		QPushButton:hover {
			border: 2px solid #00a86b;
		}
		QPushButton:pressed {
			border: 2px solid #008a52;
		}
	)");
	btnLayout->addWidget(m_pCustomBtn);

	mainLayout->addLayout(btnLayout);
	mainLayout->addStretch();

	// 窗口整体背景
	setStyleSheet("QMainWindow { background-color: #f5f6f7; }");
}

void SoftwareSelectionWidget::bindConnect()
{
	connect(m_pPreheatingBtn, &QPushButton::clicked, this, []() {
		PreheatingParamWidget* w = new PreheatingParamWidget();
		w->setAttribute(Qt::WA_DeleteOnClose);
		w->show();
		});

	// 注装工艺（预留）
	connect(m_pInjectionBtn, &QPushButton::clicked, this, []() {
		//InjectionParamWidget* w = new InjectionParamWidget();
		//w->setAttribute(Qt::WA_DeleteOnClose);
		//w->show();
		});

	// 流体仿真平台
	connect(m_pCustomBtn, &QPushButton::clicked, this, [this]() {
		m_pCustomBtn->setEnabled(false);

		mainWidget* w = new mainWidget();
		w->resize(1200, 600);
		w->setAttribute(Qt::WA_DeleteOnClose);

		connect(w, &QObject::destroyed, m_pCustomBtn, [this]() {
			m_pCustomBtn->setEnabled(true);
			});

		w->show();
		});
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