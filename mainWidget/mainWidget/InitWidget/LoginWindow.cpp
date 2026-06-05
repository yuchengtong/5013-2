#pragma execution_character_set("utf-8")
#include "LoginWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

LoginWindow::LoginWindow(QWidget* parent) : QMainWindow(parent)
{
	init();
	bindConnect();
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::init()
{
	this->setWindowIcon(QIcon(":/src/HongYuLogo.jpg"));
	// ========== 设置窗口固定大小与背景图片一致（1278×638），不压缩图片 ==========
	this->setFixedSize(1278, 638);
	this->setWindowTitle("软件平台登录页面");

	// 设置窗口背景图片：使用原始尺寸，不缩放、不压缩
	// border-image 会拉伸填充，background-image 配合 no-repeat 保持原图
	this->setStyleSheet(R"(
		QMainWindow {
			background-image: url(:/src/HongYuBg.jpg);
			background-repeat: no-repeat;
			background-position: center;
		}
	)");

	// ========== 主页面 ==========
	m_pCentralWidget = new QWidget();
	this->setCentralWidget(m_pCentralWidget);

	// ========== 右侧登录面板（半透明悬浮在背景之上） ==========
	m_pRightPanel = new QWidget();
	m_pRightPanel->setFixedWidth(420);
	m_pRightPanel->setFixedHeight(500);
	// 半透明白色背景 + 圆角，既能看清背景图，又能保证文字可读性
	m_pRightPanel->setStyleSheet(
		"background-color: rgba(255, 255, 255, 0.90);"
		"border-radius: 12px;"
	);

	QVBoxLayout* rightLayout = new QVBoxLayout(m_pRightPanel);
	rightLayout->setContentsMargins(50, 30, 50, 40);
	rightLayout->setSpacing(12);
	rightLayout->setAlignment(Qt::AlignCenter);

	// Logo
	m_pLogoLabel = new QLabel();
	m_pLogoLabel->setPixmap(QPixmap(":/src/HongYuLogo.jpg").scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	m_pLogoLabel->setFixedSize(128, 128);
	m_pLogoLabel->setAlignment(Qt::AlignCenter);
	rightLayout->addWidget(m_pLogoLabel, 0, Qt::AlignHCenter);
	rightLayout->addSpacing(20);

	// 用户名
	QLabel* userLabel = new QLabel("用户名");
	userLabel->setStyleSheet("background-color: transparent; color: #333333; font-size: 14px;");
	m_pUseLineEdit = new QLineEdit();
	m_pUseLineEdit->setPlaceholderText("example@example.com");
	m_pUseLineEdit->setStyleSheet(R"(
		QLineEdit {
			background-color: #ffffff;
			border: 1px solid #d0d0d0;
			border-radius: 6px;
			padding: 8px;
			font-size: 14px;
		}
		QLineEdit:focus {
			border: 1px solid #0066cc;
		}
	)");
	m_pUseLineEdit->setFixedHeight(36);
	rightLayout->addWidget(userLabel);
	rightLayout->addWidget(m_pUseLineEdit);

	// 密码
	QLabel* passportLabel = new QLabel("密码");
	passportLabel->setStyleSheet("background-color: transparent; color: #333333; font-size: 14px;");
	m_pPassportLineEdit = new QLineEdit();
	m_pPassportLineEdit->setPlaceholderText("••••••••••••");
	m_pPassportLineEdit->setEchoMode(QLineEdit::Password);
	m_pPassportLineEdit->setStyleSheet(R"(
		QLineEdit {
			background-color: #ffffff;
			border: 1px solid #d0d0d0;
			border-radius: 6px;
			padding: 8px;
			font-size: 14px;
		}
		QLineEdit:focus {
			border: 1px solid #0066cc;
		}
	)");
	m_pPassportLineEdit->setFixedHeight(36);
	rightLayout->addWidget(passportLabel);
	rightLayout->addWidget(m_pPassportLineEdit);

	// 登录按钮
	m_pLoginBtn = new QPushButton("登录");
	m_pLoginBtn->setCursor(Qt::PointingHandCursor);
	m_pLoginBtn->setFixedHeight(40);
	m_pLoginBtn->setStyleSheet(R"(
		QPushButton {
			background-color: #0066cc;
			color: white;
			border: none;
			border-radius: 6px;
			font-size: 15px;
			font-weight: bold;
		}
		QPushButton:hover {
			background-color: #0052a3;
		}
		QPushButton:pressed {
			background-color: #003d7a;
		}
	)");
	rightLayout->addSpacing(10);
	rightLayout->addWidget(m_pLoginBtn);
	rightLayout->addStretch();

	// ========== 主布局：左侧全部留给背景，右侧放置登录面板 ==========
	QHBoxLayout* mainLayout = new QHBoxLayout(m_pCentralWidget);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	mainLayout->addStretch(1);                       // 左侧弹性空间，把面板推到右侧
	mainLayout->addWidget(m_pRightPanel, 0, Qt::AlignVCenter);
	mainLayout->addStretch(1);

}

void LoginWindow::bindConnect()
{
	connect(m_pLoginBtn, &QPushButton::clicked, this, [this]() {
		// 这里可以添加用户名密码验证
		// if (m_pUseLineEdit->text().isEmpty()) return;

		emit loginSuccess();
		});
}