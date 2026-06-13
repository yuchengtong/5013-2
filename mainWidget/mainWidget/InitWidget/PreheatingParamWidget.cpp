#pragma execution_character_set("utf-8")
#include "PreheatingParamWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDoubleValidator>
#include <QSettings>
#include <QCoreApplication>

// ==================== 左列：材料参数（胶层 + 壳体）====================
const QList < PreheatingParamWidget::ParamConfig > PreheatingParamWidget::s_leftParams = {
	// 胶层
	{"胶层密度 (kg/m³)", "密度", "BONDLINE_DENSITY", "", false, {0.0, 0.0}},
	{"胶层比热 (J/(kg·K))", "比热", "BONDLINE_SPECIFIC_HEAT", "", false, {0.0, 0.0}},
	{"胶层热导率 (W/(m·K))", "热导率", "BONDLINE_THERMAL_CONDUCTIVITY", "", false, {0.0, 0.0}},
	{"胶层吸收系数 (m⁻¹)", "吸收系数", "BONDLINE_ABSORPTION_COEFFICIENT", "", false, {0.0, 0.0}},
	// 壳体
	{"壳体密度 (kg/m³)", "密度", "SHELL_DENSITY", "", false, {0.0, 0.0}},
	{"壳体比热 (J/(kg·K))", "比热", "SHELL_SPECIFIC_HEAT", "", false, {0.0, 0.0}},
	{"壳体热导率 (W/(m·K))", "热导率", "SHELL_THERMAL_CONDUCTIVITY", "", false, {0.0, 0.0}},
	{"壳体吸收系数 (m⁻¹)", "吸收系数", "SHELL_ABSORPTION_COEFFICIENT", "", false, {0.0, 0.0}},
};

// ==================== 右列：边界条件 ====================
const QList < PreheatingParamWidget::ParamConfig > PreheatingParamWidget::s_rightParams = {
	{"传热系数 (W/(m²·K))", "传热系数", "HEAT_TRANSFER_COEFFICIENT", "", false, {0.0, 0.0}},
	{"来流温度 (K)", "温度", "FREE_STREAM_TEMPERATURE", "", false, {0.0, 0.0}},
	{"外部辐射系数", "辐射系数", "EXTERNAL_EMISSIVITY", "", false, {0.0, 1.0}},
	{"外部辐射温度 (K)", "温度", "EXTERNAL_RADIATION_TEMPERATURE", "", false, {0.0, 0.0}},
	{"内部辐射系数", "辐射系数", "INTERNAL_EMISSIVITY", "", false, {0.0, 1.0}},
};

// ==================== 构造 / 析构 ====================
PreheatingParamWidget::PreheatingParamWidget(QWidget* parent) : QWidget(parent)
{
	setStyleSheet(globalStyleSheet());
	initUI();
	bindConnect();
	loadSettings();
}

PreheatingParamWidget::~PreheatingParamWidget() = default;

// ==================== 界面初始化 ====================
void PreheatingParamWidget::initUI()
{
	setWindowTitle("预热工艺仿真计算");
	setWindowIcon(QIcon(":/selectWidget/src/selectWidget/PreheatingProcess.jpg"));
	setFixedSize(900, 520);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(24, 20, 24, 20);
	mainLayout->setSpacing(16);

	// --- 标题 ---
	QLabel* title = new QLabel("预热工艺仿真参数配置");
	title->setObjectName("titleLabel");
	title->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(title);

	// --- Workbench 路径（全宽） ---
	QHBoxLayout* pathLayout = new QHBoxLayout();
	pathLayout->setSpacing(8);

	m_pWorkbenchPathEdit = new QLineEdit();
	m_pWorkbenchPathEdit->setPlaceholderText("请选择 runwb2.exe 路径");
	m_pWorkbenchPathEdit->setReadOnly(true);
	m_pWorkbenchPathEdit->setMinimumHeight(32);

	m_pBrowseBtn = new QPushButton("浏览...");
	m_pBrowseBtn->setFixedSize(72, 32);
	m_pBrowseBtn->setCursor(Qt::PointingHandCursor);

	pathLayout->addWidget(new QLabel("Workbench 路径:"));
	pathLayout->addWidget(m_pWorkbenchPathEdit, 1);
	pathLayout->addWidget(m_pBrowseBtn);
	mainLayout->addLayout(pathLayout);

	// --- 双列参数区 ---
	QHBoxLayout* columnsLayout = new QHBoxLayout();
	columnsLayout->setSpacing(20);

	// 左列：材料参数
	QGroupBox* leftGroup = createGroupBox("材料参数");
	QVBoxLayout* leftVBox = new QVBoxLayout(leftGroup);
	leftVBox->setContentsMargins(20, 20, 20, 20);
	buildLeftColumn(leftVBox);

	// 右列：边界条件
	QGroupBox* rightGroup = createGroupBox("边界条件");
	QVBoxLayout* rightVBox = new QVBoxLayout(rightGroup);
	rightVBox->setContentsMargins(20, 20, 20, 20);
	buildRightColumn(rightVBox);

	columnsLayout->addWidget(leftGroup, 1);
	columnsLayout->addWidget(rightGroup, 1);
	mainLayout->addLayout(columnsLayout, 1);

	// --- 启动按钮 ---
	m_pStartBtn = new QPushButton("启动计算");
	m_pStartBtn->setFixedSize(180, 42);
	m_pStartBtn->setCursor(Qt::PointingHandCursor);
	m_pStartBtn->setStyleSheet(buttonStyleSheet());
	mainLayout->addWidget(m_pStartBtn, 0, Qt::AlignHCenter);
}

// ==================== 构建左列：材料参数 ====================
void PreheatingParamWidget::buildLeftColumn(QVBoxLayout * layout)
{
	QFormLayout* form = new QFormLayout();
	form->setLabelAlignment(Qt::AlignRight);
	form->setFormAlignment(Qt::AlignLeft);
	form->setSpacing(12);
	form->setContentsMargins(0, 0, 0, 0);
	form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

	// 胶层
	QLabel* bondTip = new QLabel("— 胶层 —");
	bondTip->setObjectName("groupTip");
	form->addRow(bondTip);

	for (int i = 0; i < 4; ++i) 
	{
		QLineEdit* edit = createParamEdit(s_leftParams[i]);
		form->addRow(s_leftParams[i].label + ":", edit);
		m_paramEdits.insert(s_leftParams[i].envVar, edit);
	}

	// 壳体
	QLabel* shellTip = new QLabel("— 壳体 —");
	shellTip->setObjectName("groupTip");
	form->addRow(shellTip);

	for (int i = 4; i < s_leftParams.size(); ++i)
	{
		QLineEdit* edit = createParamEdit(s_leftParams[i]);
		form->addRow(s_leftParams[i].label + ":", edit);
		m_paramEdits.insert(s_leftParams[i].envVar, edit);
	}

	layout->addLayout(form);
	layout->addStretch();
}

// ==================== 构建右列：边界条件 ====================
void PreheatingParamWidget::buildRightColumn(QVBoxLayout * layout)
{
	QFormLayout* form = new QFormLayout();
	form->setLabelAlignment(Qt::AlignRight);
	form->setFormAlignment(Qt::AlignLeft);
	form->setSpacing(14);
	form->setContentsMargins(0, 0, 0, 0);
	form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

	for (int i = 0; i < s_rightParams.size(); ++i)
	{
		QLineEdit* edit = createParamEdit(s_rightParams[i]);
		form->addRow(s_rightParams[i].label + ":", edit);
		m_paramEdits.insert(s_rightParams[i].envVar, edit);
	}

	layout->addLayout(form);
	layout->addStretch();
}

// ==================== 辅助工厂函数 ====================
QGroupBox* PreheatingParamWidget::createGroupBox(const QString & title)
{
	QGroupBox* box = new QGroupBox(title);
	box->setStyleSheet(groupBoxStyleSheet());
	return box;
}

QLineEdit* PreheatingParamWidget::createParamEdit(const ParamConfig & config)
{
	QLineEdit* edit = new QLineEdit();
	edit->setPlaceholderText(config.placeholder);
	edit->setMinimumHeight(32);
	edit->setMinimumWidth(140);
	edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	edit->setText(config.defaultValue);

	if (config.range.first != 0.0 || config.range.second != 0.0) {
		edit->setValidator(new QDoubleValidator(
			config.range.first, config.range.second, 4, this));
	}

	return edit;
}

// ==================== 样式表 ====================
QString PreheatingParamWidget::globalStyleSheet() const
{
	return R"(
        PreheatingParamWidget {
            background-color: #f0f2f5;
        }
        #titleLabel {
            font-size: 18px;
            font-weight: bold;
            color: #1a1a1a;
            background: transparent;
            padding: 4px;
        }
        #groupTip {
            color: #0078D4;
            font-size: 11px;
            font-weight: bold;
            background: transparent;
            padding-top: 4px;
            padding-bottom: 2px;
        }
        QLabel {
            color: #333333;
            font-size: 13px;
            background: transparent;
        }
        QLineEdit {
            background-color: #ffffff;
            border: 1px solid #d0d0d0;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 13px;
            color: #333333;
        }
        QLineEdit:focus {
            border: 1px solid #0078D4;
            background-color: #f8fbff;
        }
        QLineEdit::placeholder {
            color: #aaaaaa;
        }
        QLineEdit[readOnly="true"] {
            background-color: #f5f5f5;
            color: #666666;
        }
        QPushButton {
            border-radius: 4px;
            font-size: 13px;
        }
    )";
}

QString PreheatingParamWidget::groupBoxStyleSheet() const
{
	return R"(
        QGroupBox {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            margin-top: 12px;
            font-size: 13px;
            font-weight: bold;
            color: #333333;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
            color: #0078D4;
        }
    )";
}

QString PreheatingParamWidget::buttonStyleSheet() const
{
	return R"(
        QPushButton {
            background-color: #0078D4;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #106EBE;
        }
        QPushButton:pressed {
            background-color: #005A9E;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #888888;
        }
    )";
}

// ==================== 信号绑定 ====================
void PreheatingParamWidget::bindConnect()
{
	connect(m_pBrowseBtn, &QPushButton::clicked, this, [this]() {
		QString lastPath = m_pWorkbenchPathEdit->text();
		if (lastPath.isEmpty()) {
			lastPath = QDir::homePath();
		}
		else {
			lastPath = QFileInfo(lastPath).absolutePath();
		}

		QString filePath = QFileDialog::getOpenFileName(this,
			"选择 Workbench 启动程序",
			lastPath,
			"可执行文件 (*.exe);;所有文件 (*.*)");

		if (!filePath.isEmpty()) {
			m_pWorkbenchPathEdit->setText(filePath);
			saveSettings();
		}
		});

	connect(m_pStartBtn, &QPushButton::clicked, this, [this]() {
		if (!validateInputs()) return;

		QString batPath = QDir::currentPath() + "/module/yr/yr.bat";
		//if (!QFile::exists(batPath)) {
		//	batPath = QDir::currentPath() + "/module/yr/yr.bat";
		//}

		if (!QFile::exists(batPath)) {
			QMessageBox::critical(this, "错误",
				QString("找不到批处理文件:\n%1").arg(batPath));
			return;
		}

		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert("WORKBENCH_PATH", m_pWorkbenchPathEdit->text());

		for (auto it = m_paramEdits.begin(); it != m_paramEdits.end(); ++it) {
			QString value = it.value()->text().trimmed();
			if (!value.isEmpty()) {
				env.insert(it.key(), value);
			}
		}

		QProcess process;
		process.setProgram("cmd.exe");
		process.setArguments(QStringList() << "/c" << batPath);
		process.setProcessEnvironment(env);
		process.setWorkingDirectory(QCoreApplication::applicationDirPath());

		bool ok = process.startDetached();
		if (!ok) 
		{
			QMessageBox::critical(this, "错误",
				"启动计算失败，请检查系统权限或命令配置。");
		}
		else 
		{
			saveSettings();
		}
		});
}

bool PreheatingParamWidget::validateInputs()
{
	if (m_pWorkbenchPathEdit->text().isEmpty()) 
	{
		QMessageBox::warning(this, "提示", "请先选择 Workbench 路径");
		m_pBrowseBtn->setFocus();
		return false;
	}

	QStringList missing;
	auto checkMissing = [&](const QList < ParamConfig > &list) {
		for (const auto& config : list) 
		{
			if (config.required && m_paramEdits[config.envVar]->text().trimmed().isEmpty()) 
			{
				missing.append(config.label);
			}
		}
	};

	checkMissing(s_leftParams);
	checkMissing(s_rightParams);

	if (!missing.isEmpty()) 
	{
		QMessageBox::warning(this, "提示",
			"请填写以下必填参数:\n• " + missing.join("\n• "));
		return false;
	}

	auto checkRange = [&](const QList < ParamConfig > &list) {
		for (const auto& config : list) 
		{
			if (config.range.first == 0.0 && config.range.second == 0.0)
			{
				continue;
			}

			QString text = m_paramEdits[config.envVar]->text().trimmed();
			if (text.isEmpty())
			{
				continue;
			}

			bool ok;
			double val = text.toDouble(&ok);
			if (!ok || val < config.range.first || val > config.range.second) 
			{
				QMessageBox::warning(this, "输入错误",
					QString("%1 必须在 %2 ~ %3 之间")
					.arg(config.label)
					.arg(config.range.first)
					.arg(config.range.second));
				m_paramEdits[config.envVar]->setFocus();
				m_paramEdits[config.envVar]->selectAll();
				return false;
			}
		}
		return true;
	};

	if (!checkRange(s_leftParams))
	{
		return false;
	}

	if (!checkRange(s_rightParams)) 
	{
		return false;
	}

	return true;
}

void PreheatingParamWidget::loadSettings()
{
	QSettings settings("PreheatingApp", "Params");

	m_pWorkbenchPathEdit->setText(settings.value("workbench_path").toString());

	for (auto it = m_paramEdits.begin(); it != m_paramEdits.end(); ++it) 
	{
		it.value()->setText(settings.value(it.key()).toString());
	}
}

void PreheatingParamWidget::saveSettings()
{
	QSettings settings("PreheatingApp", "Params");

	settings.setValue("workbench_path", m_pWorkbenchPathEdit->text());

	for (auto it = m_paramEdits.begin(); it != m_paramEdits.end(); ++it) 
	{
		settings.setValue(it.key(), it.value()->text());
	}
}