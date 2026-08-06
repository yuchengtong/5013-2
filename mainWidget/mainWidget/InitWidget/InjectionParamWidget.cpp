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
#include <QIntValidator>
#include <QSettings>
#include <QCoreApplication>

// ==================== 第1列：药液 ====================
const QList < InjectionParamWidget::ParamConfig > InjectionParamWidget::s_col1Params = {
	{"药液密度 (kg/m³)", "密度", "LIQUID_DENSITY", "", false, {0.0, 0.0}},
	{"药液比热容 (J/(kg·K))", "比热容", "LIQUID_SPECIFIC_HEAT", "", false, {0.0, 0.0}},
	{"药液导热系数 (W/(m·K))", "导热系数", "LIQUID_THERMAL_CONDUCTIVITY", "", false, {0.0, 0.0}},
};

// ==================== 第2列：胶层 ====================
const QList < InjectionParamWidget::ParamConfig > InjectionParamWidget::s_col2Params = {
	{"胶层密度 (kg/m³)", "密度", "BONDLINE_DENSITY", "", false, {0.0, 0.0}},
	{"胶层比热容 (J/(kg·K))", "比热容", "BONDLINE_SPECIFIC_HEAT", "", false, {0.0, 0.0}},
	{"胶层导热系数 (W/(m·K))", "导热系数", "BONDLINE_THERMAL_CONDUCTIVITY", "", false, {0.0, 0.0}},
};

// ==================== 第3列：壳体 ====================
const QList < InjectionParamWidget::ParamConfig > InjectionParamWidget::s_col3Params = {
	{"壳体密度 (kg/m³)", "密度", "SHELL_DENSITY", "", false, {0.0, 0.0}},
	{"壳体比热容 (J/(kg·K))", "比热容", "SHELL_SPECIFIC_HEAT", "", false, {0.0, 0.0}},
	{"壳体导热系数 (W/(m·K))", "导热系数", "SHELL_THERMAL_CONDUCTIVITY", "", false, {0.0, 0.0}},
};

// ==================== 第4列：边界条件 ====================
const QList < InjectionParamWidget::ParamConfig > InjectionParamWidget::s_col4Params = {
	{"进气口总压 (Pa)", "Pa", "INLET_TOTAL_PRESSURE", "", false, {0.0, 0.0}},
	{"进气口总温度 (K)", "K", "INLET_TOTAL_TEMPERATURE", "", false, {0.0, 0.0}},
	{"壁温 (K)", "K", "WALL_TEMPERATURE", "", false, {0.0, 0.0}},
};

// ==================== 第5列：几何参数 ====================
const QList < InjectionParamWidget::ParamConfig > InjectionParamWidget::s_col5Params = {
	{"总长度 P1 (mm)", "mm", "GEOM_P1", "", false, {0.0, 0.0}},
	{"特征长度 P2 (mm)", "mm", "GEOM_P2", "", false, {0.0, 0.0}},
	{"特征高度 P3 (mm)", "mm", "GEOM_P3", "", false, {0.0, 0.0}},
	{"壁厚 P5 (mm)", "mm", "GEOM_P5", "", false, {0.0, 0.0}},
	{"间隙 P6 (mm)", "mm", "GEOM_P6", "", false, {0.0, 0.0}},
};

// ==================== 参数默认值 ====================
// HQ-9B 对应 C1
const QMap<QString, QString> InjectionParamWidget::s_defaultParamValues1 = {
	{"LIQUID_DENSITY",          "1855.0"},
	{"LIQUID_SPECIFIC_HEAT",    "1500.0"},
	{"LIQUID_THERMAL_CONDUCTIVITY", "0.6"},

	{"BONDLINE_DENSITY",        "900.0"},
	{"BONDLINE_SPECIFIC_HEAT",  "1800.0"},
	{"BONDLINE_THERMAL_CONDUCTIVITY", "1.0"},

	{"SHELL_DENSITY",           "7850.0"},
	{"SHELL_SPECIFIC_HEAT",     "460.0"},
	{"SHELL_THERMAL_CONDUCTIVITY", "43.0"},

	{"INLET_TOTAL_PRESSURE",    "80000.0"},
	{"INLET_TOTAL_TEMPERATURE", "375.1"},
	{"WALL_TEMPERATURE",        "343.1"},

	{"GEOM_P1", "780.0"},
	{"GEOM_P2", "39.0"},
	{"GEOM_P3", "391.0"},
	{"GEOM_P5", "5.0"},
	{"GEOM_P6", "30.0"}
};

// YJ-20 对应 C2
const QMap<QString, QString> InjectionParamWidget::s_defaultParamValues2 = {
	{"LIQUID_DENSITY",          "1805.0"},
	{"LIQUID_SPECIFIC_HEAT",    "1500.0"},
	{"LIQUID_THERMAL_CONDUCTIVITY", "0.6"},

	{"BONDLINE_DENSITY",        "900.0"},
	{"BONDLINE_SPECIFIC_HEAT",  "1800.0"},
	{"BONDLINE_THERMAL_CONDUCTIVITY", "1.0"},

	{"SHELL_DENSITY",           "4400.0"},
	{"SHELL_SPECIFIC_HEAT",     "560.0"},
	{"SHELL_THERMAL_CONDUCTIVITY", "7.50"},

	{"INLET_TOTAL_PRESSURE",    "80000.0"},
	{"INLET_TOTAL_TEMPERATURE", "375.1"},
	{"WALL_TEMPERATURE",        "343.1"},

	{"GEOM_P1", "1075.0"},
	{"GEOM_P2", "370.0"},
	{"GEOM_P3", "39.0"},
	{"GEOM_P5", "5.0"},
	{"GEOM_P6", "30.0"}
};

// YJ-91A 对应 C3
const QMap<QString, QString> InjectionParamWidget::s_defaultParamValues3 = {
	{"LIQUID_DENSITY",          "1790.0"},
	{"LIQUID_SPECIFIC_HEAT",    "1500.0"},
	{"LIQUID_THERMAL_CONDUCTIVITY", "0.6"},

	{"BONDLINE_DENSITY",        "900.0"},
	{"BONDLINE_SPECIFIC_HEAT",  "1800.0"},
	{"BONDLINE_THERMAL_CONDUCTIVITY", "1.0"},

	{"SHELL_DENSITY",           "2700.0"},
	{"SHELL_SPECIFIC_HEAT",     "920.0"},
	{"SHELL_THERMAL_CONDUCTIVITY", "130.0"},

	{"INLET_TOTAL_PRESSURE",    "80000.0"},
	{"INLET_TOTAL_TEMPERATURE", "375.1"},
	{"WALL_TEMPERATURE",        "343.1"},

	{"GEOM_P1", "930.0"},
	{"GEOM_P2", "39.0"},
	{"GEOM_P3", "330.0"},
	{"GEOM_P5", "5.0"},
	{"GEOM_P6", "30.0"}
};

// CJ-20A 对应 C4
const QMap<QString, QString> InjectionParamWidget::s_defaultParamValues4 = {
	{"LIQUID_DENSITY",          "1730.0"},
	{"LIQUID_SPECIFIC_HEAT",    "1500.0"},
	{"LIQUID_THERMAL_CONDUCTIVITY", "0.6"},

	{"BONDLINE_DENSITY",        "900.0"},
	{"BONDLINE_SPECIFIC_HEAT",  "1800.0"},
	{"BONDLINE_THERMAL_CONDUCTIVITY", "1.0"},

	{"SHELL_DENSITY",           "7850.0"},
	{"SHELL_SPECIFIC_HEAT",     "460.0"},
	{"SHELL_THERMAL_CONDUCTIVITY", "43.0"},

	{"INLET_TOTAL_PRESSURE",    "80000.0"},
	{"INLET_TOTAL_TEMPERATURE", "375.1"},
	{"WALL_TEMPERATURE",        "343.1"},

	{"GEOM_P1", "1120.0"},
	{"GEOM_P2", "400.0"},
	{"GEOM_P3", "39.0"},
	{"GEOM_P5", "5.0"},
	{"GEOM_P6", "30.0"}
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
	setFixedSize(1500, 660);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(24, 16, 24, 16);
	mainLayout->setSpacing(12);

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

	// --- 工况类型选择 ---
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
	typeLayout->addSpacing(80);
	mainLayout->addLayout(typeLayout);

	// --- 五列参数区 ---
	QHBoxLayout* columnsLayout = new QHBoxLayout();
	columnsLayout->setSpacing(14);

	QGroupBox* col5Group = createGroupBox("几何参数");
	QVBoxLayout* col5VBox = new QVBoxLayout(col5Group);
	col5VBox->setContentsMargins(12, 16, 12, 16);
	buildColumn5(col5VBox);

	QGroupBox* col1Group = createGroupBox("药液");
	QVBoxLayout* col1VBox = new QVBoxLayout(col1Group);
	col1VBox->setContentsMargins(12, 16, 12, 16);
	buildColumn1(col1VBox);

	QGroupBox* col2Group = createGroupBox("胶层");
	QVBoxLayout* col2VBox = new QVBoxLayout(col2Group);
	col2VBox->setContentsMargins(12, 16, 12, 16);
	buildColumn2(col2VBox);

	QGroupBox* col3Group = createGroupBox("壳体");
	QVBoxLayout* col3VBox = new QVBoxLayout(col3Group);
	col3VBox->setContentsMargins(12, 16, 12, 16);
	buildColumn3(col3VBox);

	QGroupBox* col4Group = createGroupBox("边界条件");
	QVBoxLayout* col4VBox = new QVBoxLayout(col4Group);
	col4VBox->setContentsMargins(12, 16, 12, 16);
	buildColumn4(col4VBox);

	columnsLayout->addWidget(col5Group, 1);
	columnsLayout->addWidget(col1Group, 1);
	columnsLayout->addWidget(col2Group, 1);
	columnsLayout->addWidget(col3Group, 1);
	columnsLayout->addWidget(col4Group, 1);
	mainLayout->addLayout(columnsLayout, 1);

	// --- 仿真设置区域（步数 + 启动模式 并排） ---
	QGroupBox* simGroup = createGroupBox("仿真设置");
	QHBoxLayout* simLayout = new QHBoxLayout(simGroup);
	simLayout->setContentsMargins(16, 12, 16, 12);
	simLayout->setSpacing(24);

	// 仿真步数
	QHBoxLayout* stepLayout = new QHBoxLayout();
	stepLayout->setSpacing(8);
	QLabel* stepLabel = new QLabel("仿真步数:");
	stepLabel->setMinimumWidth(70);
	stepLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_pStepCountEdit = new QLineEdit();
	m_pStepCountEdit->setPlaceholderText("正整数");
	m_pStepCountEdit->setMinimumHeight(32);
	m_pStepCountEdit->setFixedWidth(120);
	m_pStepCountEdit->setValidator(new QIntValidator(1, 99999999, this));
	m_pStepCountEdit->setText(s_defaultStepCount);  // 【新增】默认步数 11
	stepLayout->addWidget(stepLabel);
	stepLayout->addWidget(m_pStepCountEdit);
	stepLayout->addStretch();

	// 启动模式
	QHBoxLayout* modeLayout = new QHBoxLayout();
	modeLayout->setSpacing(8);
	QLabel* modeLabel = new QLabel("启动模式:");
	modeLabel->setMinimumWidth(70);
	modeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_pLaunchModeComboBox = new QComboBox();
	m_pLaunchModeComboBox->addItems(QStringList() << "界面启动" << "无界面启动");
	m_pLaunchModeComboBox->setMinimumHeight(32);
	m_pLaunchModeComboBox->setFixedWidth(120);
	modeLayout->addWidget(modeLabel);
	modeLayout->addWidget(m_pLaunchModeComboBox);
	modeLayout->addStretch();

	simLayout->addLayout(stepLayout);
	simLayout->addLayout(modeLayout);
	simLayout->addStretch();
	mainLayout->addWidget(simGroup);

	// --- 底部按钮 ---
	QHBoxLayout* bottomBtnLayout = new QHBoxLayout();
	bottomBtnLayout->setSpacing(20);
	bottomBtnLayout->setAlignment(Qt::AlignCenter);

	m_pDefaultBtn = new QPushButton("恢复默认");
	m_pDefaultBtn->setFixedSize(160, 40);
	m_pDefaultBtn->setCursor(Qt::PointingHandCursor);
	m_pDefaultBtn->setStyleSheet(buttonStyleSheet());

	m_pStartBtn = new QPushButton("启动计算");
	m_pStartBtn->setFixedSize(160, 40);
	m_pStartBtn->setCursor(Qt::PointingHandCursor);
	m_pStartBtn->setStyleSheet(buttonStyleSheet());

	bottomBtnLayout->addStretch();
	bottomBtnLayout->addWidget(m_pDefaultBtn);
	bottomBtnLayout->addWidget(m_pStartBtn);
	bottomBtnLayout->addStretch();
	mainLayout->addLayout(bottomBtnLayout);
}

// ==================== 构建第1列：药液 ====================
void InjectionParamWidget::buildColumn1(QVBoxLayout * layout)
{
	QFormLayout* form = new QFormLayout();
	form->setLabelAlignment(Qt::AlignRight);
	form->setFormAlignment(Qt::AlignLeft);
	form->setSpacing(14);
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
	form->setSpacing(14);
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
	form->setSpacing(14);
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
	form->setSpacing(14);
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

// ==================== 构建第5列：几何参数 ====================
void InjectionParamWidget::buildColumn5(QVBoxLayout * layout)
{
	QFormLayout* form = new QFormLayout();
	form->setLabelAlignment(Qt::AlignRight);
	form->setFormAlignment(Qt::AlignLeft);
	form->setSpacing(14);
	form->setContentsMargins(0, 0, 0, 0);
	form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	form->setRowWrapPolicy(QFormLayout::DontWrapRows);

	for (int i = 0; i < s_col5Params.size(); ++i)
	{
		QLineEdit* edit = createParamEdit(s_col5Params[i]);
		form->addRow(s_col5Params[i].label + ":", edit);
		m_paramEdits.insert(s_col5Params[i].envVar, edit);
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
		if (!validateInputs()) return;

		QString basePath, batPath;
		QString caseType = m_pTypeComboBox->currentText();

		if (caseType == "HQ-9B") {
			basePath = QDir::currentPath() + "/module/C1";
			batPath = basePath + "/C1.bat";
		}
		else if (caseType == "YJ-20") {
			basePath = QDir::currentPath() + "/module/C2";
			batPath = basePath + "/C2.bat";
		}
		else if (caseType == "YJ-91A") {
			basePath = QDir::currentPath() + "/module/C3";
			batPath = basePath + "/C3.bat";
		}
		else if (caseType == "CJ-20A") {
			basePath = QDir::currentPath() + "/module/C4";
			batPath = basePath + "/C4.bat";
		}

		if (!QFile::exists(batPath)) {
			QMessageBox::critical(this, "错误",
				QString("找不到批处理文件:\n%1").arg(batPath));
			return;
		}

		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert("WORKBENCH_PATH", m_pWorkbenchPathEdit->text());
		env.insert("CASE_TYPE", caseType);
		env.insert("NUMBER_OF_TIME_STEPS", m_pStepCountEdit->text().trimmed());

		// ========== 启动模式判断 ==========
		QString launchMode = m_pLaunchModeComboBox->currentText();
		if (launchMode == "界面启动") {
			env.insert("LAUNCH_MODE", "gui");
		}
		else {
			env.insert("LAUNCH_MODE", "batch");
		}
		// ==================================

		// 传递所有参数
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
		process.setWorkingDirectory(basePath);

		bool ok = process.startDetached();
		if (!ok) {
			QMessageBox::critical(this, "错误",
				"启动计算失败，请检查系统权限或命令配置。");
		}
		else {
			saveSettings();
		}
		});

	connect(m_pDefaultBtn, &QPushButton::clicked, this, &InjectionParamWidget::setDefaultValues);
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

	// 验证仿真步数
	QString stepText = m_pStepCountEdit->text().trimmed();
	if (stepText.isEmpty())
	{
		QMessageBox::warning(this, "提示", "请填写仿真步数");
		m_pStepCountEdit->setFocus();
		return false;
	}
	bool ok;
	int steps = stepText.toInt(&ok);
	if (!ok || steps <= 0)
	{
		QMessageBox::warning(this, "输入错误", "仿真步数必须为正整数");
		m_pStepCountEdit->setFocus();
		m_pStepCountEdit->selectAll();
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
	checkMissing(s_col5Params);

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

	if (!checkRange(s_col1Params)) return false;
	if (!checkRange(s_col2Params)) return false;
	if (!checkRange(s_col3Params)) return false;
	if (!checkRange(s_col4Params)) return false;
	if (!checkRange(s_col5Params)) return false;

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

	// 加载仿真步数，无记录则默认 11
	m_pStepCountEdit->setText(settings.value("simulation_steps", s_defaultStepCount).toString());
	m_pLaunchModeComboBox->setCurrentText(settings.value("launch_mode", "界面启动").toString());
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

	settings.setValue("simulation_steps", m_pStepCountEdit->text());
	settings.setValue("launch_mode", m_pLaunchModeComboBox->currentText());
}

void InjectionParamWidget::setDefaultValues()
{
	QString caseType = m_pTypeComboBox->currentText();
	QMap<QString, QString> s_defaultParamValues;
	if (caseType == "HQ-9B")
	{
		s_defaultParamValues = s_defaultParamValues1;
	}
	else if (caseType == "YJ-20")
	{
		s_defaultParamValues = s_defaultParamValues2;
	}
	else if (caseType == "YJ-91A")
	{
		s_defaultParamValues = s_defaultParamValues3;
	}
	else
	{
		s_defaultParamValues = s_defaultParamValues4;
	}

	for (auto it = m_paramEdits.begin(); it != m_paramEdits.end(); ++it)
	{
		QString key = it.key();
		if (s_defaultParamValues.contains(key))
		{
			it.value()->setText(s_defaultParamValues[key]);
		}
	}


	m_pStepCountEdit->setText(s_defaultStepCount);
}