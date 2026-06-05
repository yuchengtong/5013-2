#pragma execution_character_set("utf-8")
#include "LoginWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

LoginWindow::LoginWindow(QWidget* parent): FluentMainWindow(parent)
{
	init();
	bindConnect();
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::init()
{
	m_pMenuBar = new Fluent::FluentMenuBar();
	this->setFluentMenuBar(m_pMenuBar);
	m_pMenuBar->setFixedHeight(32);
	//平台名称
	{
		QHBoxLayout* titleLayout = new QHBoxLayout();
		titleLayout->setSpacing(4);
		titleLayout->setContentsMargins(0, 0, 0, 0);
		// Logo 图标
		Fluent::FluentLabel* logoIcon = new Fluent::FluentLabel();
		logoIcon->setPixmap(QPixmap(":/src/HongYuLogo.jpg").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		logoIcon->setFixedSize(32, 32);
		titleLayout->addWidget(logoIcon);

		// 平台名称
		m_pTitleLabel = new Fluent::FluentLabel("软件平台登录页面");
		titleLayout->addWidget(m_pTitleLabel);

		QWidget* titleWidget = new QWidget();
		titleWidget->setLayout(titleLayout);
		titleWidget->setFixedWidth(350);
		titleWidget->setFixedHeight(32);
		this->setFluentTitleBarLeftWidget(titleWidget);
	}

	//主页面
	m_pCentralWidget = new Fluent::FluentWidget();
	m_pCentralWidget->setBackgroundRole(Fluent::FluentWidget::BackgroundRole::WindowBackground);
	this->setCentralWidget(m_pCentralWidget);

	// 左侧插图
	m_pLeftImageLabel = new Fluent::FluentLabel();
	m_pLeftImageLabel->setScaledContents(true);
	m_pLeftImageLabel->setMinimumWidth(400);  // 最小宽度，避免过窄

	QPixmap bg(":/src/HongYuBg.jpg");
	m_pLeftImageLabel->setPixmap(bg);



	// 右侧登录面板
	m_pRightPanel = new Fluent::FluentWidget();
	m_pRightPanel->setFixedWidth(420);


	QVBoxLayout* rightLayout = new QVBoxLayout(m_pRightPanel);
	rightLayout->setContentsMargins(50, 30, 50, 40);
	rightLayout->setSpacing(12);
	rightLayout->setAlignment(Qt::AlignCenter);

	// Logo
	m_pLogoLabel = new Fluent::FluentLabel();
	m_pLogoLabel->setPixmap(QPixmap(":/src/HongYuLogo.jpg").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	m_pLogoLabel->setFixedSize(64, 64);
	m_pLogoLabel->setAlignment(Qt::AlignCenter);
	rightLayout->addWidget(m_pLogoLabel, 0, Qt::AlignHCenter);
	rightLayout->addSpacing(20);


	// 用户名
	Fluent::FluentLabel* userLabel = new Fluent::FluentLabel("用户名");
	m_pUseLineEdit = new Fluent::FluentLineEdit();
	m_pUseLineEdit->setPlaceholderText("example@example.com");
	rightLayout->addWidget(userLabel);
	rightLayout->addWidget(m_pUseLineEdit);

	// 密码
	Fluent::FluentLabel* passportLabel = new Fluent::FluentLabel("密码");
	m_pPassportLineEdit = new Fluent::FluentLineEdit();
	m_pPassportLineEdit->setPlaceholderText("••••••••••••");
	m_pPassportLineEdit->setEchoMode(QLineEdit::Password);
	rightLayout->addWidget(passportLabel);
	rightLayout->addWidget(m_pPassportLineEdit);

	// 登录按钮
	m_pLoginBtn = new Fluent::FluentButton("登录");
	m_pLoginBtn->setCursor(Qt::PointingHandCursor);
	m_pLoginBtn->setFixedHeight(38);
	rightLayout->addWidget(m_pLoginBtn);
	rightLayout->addStretch();

	// 主布局：左侧拉伸(1)，右侧固定(0)
	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(m_pLeftImageLabel, 1);
	mainLayout->addWidget(m_pRightPanel, 0); 

	m_pCentralWidget->setLayout(mainLayout);
}

void LoginWindow::bindConnect()
{
	connect(m_pLoginBtn, &Fluent::FluentButton::clicked, this, [this]() {
		// 这里可以添加用户名密码验证
		// if (m_pUseLineEdit->text().isEmpty()) return;

		emit loginSuccess();
		});
}
