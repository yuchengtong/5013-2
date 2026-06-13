#pragma execution_character_set("utf-8")
#include "InjectionParamWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
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

// ==================== 第1列：药液 ====================
const QList < InjectionParamWidget::ParamConfig > InjectionParamWidget::s_col1Params = {
	{"药液密度 (kg/m³)", "密度", "LIQUID_DENSITY", "", false, {0.0, 0.0}},
	{"药液比热容 (J/(kg·K))", "比热容", "LIQUID_SPECIFIC_HEAT", "", false, {0.0, 0.0}},
	{"药液导热系数 (W/(cm·K))", "导热系数", "LIQUID_THERMAL_CONDUCTIVITY", "", false, {0.0, 0.0}},
};

// ==================== 第2列：胶层 ====================
const QList < InjectionParamWidget::ParamConfig > InjectionParamWidget::s_col2Params = {
	{"胶层密度 (kg/m³)", "密度", "BONDLINE_DENSITY", "", false, {0.0, 0.0}},
	{"胶层比热容 (J/(kg·K))", "比热容", "BONDLINE_SPECIFIC_HEAT", "", false, {0.0, 0.0}},
	{"胶层导热系数 (W/(cm·K))", "导热系数", "BONDLINE_THERMAL_CONDUCTIVITY", "", false, {0.0, 0.0}},
};

// ==================== 第3列：壳体 ====================
const QList < InjectionParamWidget::ParamConfig > InjectionParamWidget::s_col3Params = {
	{"壳体密度 (kg/m³)", "密度", "SHELL_DENSITY", "", false, {0.0, 0.0}},
	{"壳体比热容 (J/(kg·K))", "比热容", "SHELL_SPECIFIC_HEAT", "", false, {0.0, 0.0}},
	{"壳体导热系数 (W/(cm·K))", "导热系数", "SHELL_THERMAL_CONDUCTIVITY", "", false, {0.0, 0.0}},
};

// ==================== 第4列：边界条件 ====================
const QList < InjectionParamWidget::ParamConfig > InjectionParamWidget::s_col4Params = {
	{"进气口总压 (Pa)", "Pa", "INLET_TOTAL_PRESSURE", "", false, {0.0, 0.0}},
	{"进气口总温度 (K)", "K", "INLET_TOTAL_TEMPERATURE", "", false, {0.0, 0.0}},
	{"壁温 (K)", "K", "WALL_TEMPERATURE", "", false, {0.0, 0.0}},
};

// ==================== 构造 / 析构 ====================
InjectionParamWidget::InjectionParamWidget(QWidget* parent) : QWidget(parent)
{
	setStyleSheet(globalStyleSheet());
	initUI();
	bindConnect();
	loadSettings();
}

InjectionParamWidget::~InjectionParamWidget() = default;

// ==================== 界面初始化 ====================
void InjectionParamWidget::initUI()
{
	setWindowTitle("注药工艺仿真计算");
	setWindowIcon(QIcon(":/selectWidget/src/selectWidget/InjectionProcess.jpg"));
	setFixedSize(1280, 580);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(24, 20, 24, 20);
	mainLayout->setSpacing(14);

	// --- 标题 ---
	QLabel* title = new QLabel("注药工艺仿真参数配置");
	title->setObjectName("titleLabel");
	title->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(title);

	// --- Workbench 路径 ---
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

	// --- 工况类型选择（全宽） ---
	QHBoxLayout* typeLayout = new QHBoxLayout();
	typeLayout->setSpacing(8);

	m_pTypeComboBox = new QComboBox();
	m_pTypeComboBox->addItems(QStringList() << "HQ-9B" << "YJ-20" << "YJ-91A" << "CJ-20A");
	m_pTypeComboBox->setMinimumHeight(32);
	m_pTypeComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QLabel* typeLabel = new QLabel("工况类型:");
	typeLabel->setMinimumWidth(90);
	typeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	typeLayout->addWidget(typeLabel);
	typeLayout->addWidget(m_pTypeComboBox, 1);
	typeLayout->addSpacing(80); // 与右侧浏览按钮对齐留白
	mainLayout->addLayout(typeLayout);

	// --- 四列参数区 ---
	QHBoxLayout* columnsLayout = new QHBoxLayout();
	columnsLayout->setSpacing(20);

	// 第1列：药液
	QGroupBox* col1Group = createGroupBox("药液");
	QVBoxLayout* col1VBox = new QVBoxLayout(col1Group);
	col1VBox->setContentsMargins(20, 24, 20, 24);
	buildColumn1(col1VBox);

	// 第2列：胶层
	QGroupBox* col2Group = createGroupBox("胶层");
	QVBoxLayout* col2VBox = new QVBoxLayout(col2Group);
	col2VBox->setContentsMargins(20, 24, 20, 24);
	buildColumn2(col2VBox);

	// 第3列：壳体
	QGroupBox* col3Group = createGroupBox("壳体");
	QVBoxLayout* col3VBox = new QVBoxLayout(col3Group);
	col3VBox->setContentsMargins(20, 24, 20, 24);
	buildColumn3(col3VBox);

	// 第4列：边界条件
	QGroupBox* col4Group = createGroupBox("边界条件");
	QVBoxLayout* col4VBox = new QVBoxLayout(col4Group);
	col4VBox->setContentsMargins(20, 24, 20, 24);
	buildColumn4(col4VBox);

	columnsLayout->addWidget(col1Group, 1);
	columnsLayout->addWidget(col2Group, 1);
	columnsLayout->addWidget(col3Group, 1);
	columnsLayout->addWidget(col4Group, 1);
	mainLayout->addLayout(columnsLayout, 1);

	// --- 启动按钮 ---
	m_pStartBtn = new QPushButton("启动计算");
	m_pStartBtn->setFixedSize(180, 42);
	m_pStartBtn->setCursor(Qt::PointingHandCursor);
	m_pStartBtn->setStyleSheet(buttonStyleSheet());
	mainLayout->addWidget(m_pStartBtn, 0, Qt::AlignHCenter);
}

// ==================== 构建第1列：药液 ====================
void InjectionParamWidget::buildColumn1(QVBoxLayout * layout)
{
	QFormLayout* form = new QFormLayout();
	form->setLabelAlignment(Qt::AlignRight);
	form->setFormAlignment(Qt::AlignLeft);
	form->setSpacing(16);
	form->setContentsMargins(0, 0, 0, 0);
	form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	form->setRowWrapPolicy(QFormLayout::DontWrapRows);

	for (int i = 0; i < s_col1Params.size(); ++i) 
	{
		QLineEdit* edit = createParamEdit(s_col1Params[i]);
		form->addRow(s_col1Params[i].label + ":", edit);
		m_paramEdits.insert(s_col1Params[i].envVar, edit);
	}

	layout->addLayout(form);
	layout->addStretch();
}

// ==================== 构建第2列：胶层 ====================
void InjectionParamWidget::buildColumn2(QVBoxLayout * layout)
{
	QFormLayout* form = new QFormLayout();
	form->setLabelAlignment(Qt::AlignRight);
	form->setFormAlignment(Qt::AlignLeft);
	form->setSpacing(16);
	form->setContentsMargins(0, 0, 0, 0);
	form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	form->setRowWrapPolicy(QFormLayout::DontWrapRows);

	for (int i = 0; i < s_col2Params.size(); ++i)
	{
		QLineEdit* edit = createParamEdit(s_col2Params[i]);
		form->addRow(s_col2Params[i].label + ":", edit);
		m_paramEdits.insert(s_col2Params[i].envVar, edit);
	}

	layout->addLayout(form);
	layout->addStretch();
}

// ==================== 构建第3列：壳体 ====================
void InjectionParamWidget::buildColumn3(QVBoxLayout * layout)
{
	QFormLayout* form = new QFormLayout();
	form->setLabelAlignment(Qt::AlignRight);
	form->setFormAlignment(Qt::AlignLeft);
	form->setSpacing(16);
	form->setContentsMargins(0, 0, 0, 0);
	form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	form->setRowWrapPolicy(QFormLayout::DontWrapRows);

	for (int i = 0; i < s_col3Params.size(); ++i)
	{
		QLineEdit* edit = createParamEdit(s_col3Params[i]);
		form->addRow(s_col3Params[i].label + ":", edit);
		m_paramEdits.insert(s_col3Params[i].envVar, edit);
	}

	layout->addLayout(form);
	layout->addStretch();
}

// ==================== 构建第4列：边界条件 ====================
void InjectionParamWidget::buildColumn4(QVBoxLayout * layout)
{
	QFormLayout* form = new QFormLayout();
	form->setLabelAlignment(Qt::AlignRight);
	form->setFormAlignment(Qt::AlignLeft);
	form->setSpacing(16);
	form->setContentsMargins(0, 0, 0, 0);
	form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	form->setRowWrapPolicy(QFormLayout::DontWrapRows);

	for (int i = 0; i < s_col4Params.size(); ++i) 
	{
		QLineEdit* edit = createParamEdit(s_col4Params[i]);
		form->addRow(s_col4Params[i].label + ":", edit);
		m_paramEdits.insert(s_col4Params[i].envVar, edit);
	}

	layout->addLayout(form);
	layout->addStretch();
}

// ==================== 辅助工厂函数 ====================
QGroupBox* InjectionParamWidget::createGroupBox(const QString & title)
{
	QGroupBox* box = new QGroupBox(title);
	box->setStyleSheet(groupBoxStyleSheet());
	return box;
}

QLineEdit* InjectionParamWidget::createParamEdit(const ParamConfig & config)
{
	QLineEdit* edit = new QLineEdit();
	edit->setPlaceholderText(config.placeholder);
	edit->setMinimumHeight(32);
	edit->setMinimumWidth(90);
	edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	edit->setText(config.defaultValue);

	if (config.range.first != 0.0 || config.range.second != 0.0)
	{
		edit->setValidator(new QDoubleValidator(
			config.range.first, config.range.second, 4, this));
	}

	return edit;
}

// ==================== 样式表 ====================
QString InjectionParamWidget::globalStyleSheet() const
{
	return R"(
        InjectionParamWidget {
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
            padding-top: 2px;
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
        QComboBox {
            background-color: #ffffff;
            border: 1px solid #d0d0d0;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 13px;
            color: #333333;
            min-height: 32px;
        }
        QComboBox:focus {
            border: 1px solid #0078D4;
        }
        QComboBox::drop-down {
            border: none;
            width: 24px;
        }
        QComboBox QAbstractItemView {
            border: 1px solid #d0d0d0;
            background-color: #ffffff;
            selection-background-color: #0078D4;
            selection-color: #ffffff;
        }
    )";
}

QString InjectionParamWidget::groupBoxStyleSheet() const
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

QString InjectionParamWidget::buttonStyleSheet() const
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
void InjectionParamWidget::bindConnect()
{
	connect(m_pBrowseBtn, &QPushButton::clicked, this, [this]() {
		QString lastPath = m_pWorkbenchPathEdit->text();
		if (lastPath.isEmpty()) 
		{
			lastPath = QDir::homePath();
		}
		else {
			lastPath = QFileInfo(lastPath).absolutePath();
		}

		QString filePath = QFileDialog::getOpenFileName(this,
			"选择 Workbench 启动程序",
			lastPath,
			"可执行文件 (*.exe);;所有文件 (*.*)");

		if (!filePath.isEmpty()) 
		{
			m_pWorkbenchPathEdit->setText(filePath);
			saveSettings();
		}
		});

	connect(m_pStartBtn, &QPushButton::clicked, this, [this]() {
		if (!validateInputs())
		{
			return;
		}

		QString basePath, batPath;
		QString caseType = m_pTypeComboBox->currentText();

		if (caseType == "HQ-9B")
		{
			basePath = QDir::currentPath() + "/module/C1";
			batPath = basePath + "/C1.bat";
		}
		else if(caseType == "YJ-20")
		{
			basePath = QDir::currentPath() + "/module/C2";
			batPath = basePath + "/C2.bat";
		}
		else if (caseType == "YJ-91A")
		{
			basePath = QDir::currentPath() + "/module/C3";
			batPath = basePath + "/C3.bat";
		}
		else if (caseType == "CJ-20A")
		{
			basePath = QDir::currentPath() + "/module/C4";
			batPath = basePath + "/C4.bat";
		}

		// 如果 applicationDirPath 下找不到，尝试 currentPath
		if (!QFile::exists(batPath)) 
		{
			basePath = QDir::currentPath() + "/module/" + caseType;
			batPath = basePath + "/" + caseType + ".bat";
		}

		if (!QFile::exists(batPath)) 
		{
			QMessageBox::critical(this, "错误",
				QString("找不到批处理文件:\n%1").arg(batPath));
			return;
		}

		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert("WORKBENCH_PATH", m_pWorkbenchPathEdit->text());
		env.insert("CASE_TYPE", caseType);

		for (auto it = m_paramEdits.begin(); it != m_paramEdits.end(); ++it)
		{
			QString value = it.value()->text().trimmed();
			if (!value.isEmpty()) 
			{
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

// ==================== 验证与持久化 ====================
bool InjectionParamWidget::validateInputs()
{
	if (m_pWorkbenchPathEdit->text().isEmpty())
	{
		QMessageBox::warning(this, "提示", "请先选择 Workbench 路径");
		m_pBrowseBtn->setFocus();
		return false;
	}

	QStringList missing;
	auto checkMissing = [&](const QList < ParamConfig >& list) {
		for (const auto& config : list) 
		{
			if (config.required && m_paramEdits[config.envVar]->text().trimmed().isEmpty()) 
			{
				missing.append(config.label);
			}
		}
	};

	checkMissing(s_col1Params);
	checkMissing(s_col2Params);
	checkMissing(s_col3Params);
	checkMissing(s_col4Params);

	if (!missing.isEmpty()) 
	{
		QMessageBox::warning(this, "提示",
			"请填写以下必填参数:\n• " + missing.join("\n• "));
		return false;
	}

	auto checkRange = [&](const QList < ParamConfig >& list) {
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

	if (!checkRange(s_col1Params))
	{
		return false;
	}

	if (!checkRange(s_col2Params)) 
	{
		return false;
	}

	if (!checkRange(s_col3Params)) 
	{
		return false;
	}

	if (!checkRange(s_col4Params)) 
	{
		return false;
	}

	return true;
}

void InjectionParamWidget::loadSettings()
{
	QSettings settings("PreheatingApp", "Params");

	m_pWorkbenchPathEdit->setText(settings.value("workbench_path").toString());
	m_pTypeComboBox->setCurrentText(settings.value("case_type", "HQ-9B").toString());

	for (auto it = m_paramEdits.begin(); it != m_paramEdits.end(); ++it) 
	{
		it.value()->setText(settings.value(it.key()).toString());
	}
}

void InjectionParamWidget::saveSettings()
{
	QSettings settings("PreheatingApp", "Params");

	settings.setValue("workbench_path", m_pWorkbenchPathEdit->text());
	settings.setValue("case_type", m_pTypeComboBox->currentText());

	for (auto it = m_paramEdits.begin(); it != m_paramEdits.end(); ++it) 
	{
		settings.setValue(it.key(), it.value()->text());
	}
}