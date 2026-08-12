#pragma once

#include <QWidget>
#include <QMap>
#include <QList>
#include <QString>

class QLineEdit;
class QPushButton;
class QComboBox;
class QGroupBox;
class QVBoxLayout;
class QFormLayout;

class InjectionParamWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InjectionParamWidget(QWidget* parent = nullptr);
    ~InjectionParamWidget();

protected:
    void initUI();
    void bindConnect();

private:
    // 参数配置结构
    struct ParamConfig {
        QString label;
        QString placeholder;
        QString envVar;
        QString defaultValue;
        bool required;
        QPair<double, double> range;  // 最小值, 最大值
    };

    // 静态参数列表
    static const QList<ParamConfig> s_col1Params;   // 药液
    static const QList<ParamConfig> s_col2Params;   // 胶层
    static const QList<ParamConfig> s_col3Params;   // 壳体
    static const QList<ParamConfig> s_col4Params;   // 边界条件
    static const QList<ParamConfig> s_col5Params;   // 几何参数

    // 默认值映射（四种工况）
    static const QMap<QString, QString> s_defaultParamValues1; // HQ-9B
    static const QMap<QString, QString> s_defaultParamValues2; // YJ-20
    static const QMap<QString, QString> s_defaultParamValues3; // YJ-91A
    static const QMap<QString, QString> s_defaultParamValues4; // CJ-20A

    static const QString s_defaultStepCount; // 默认迭代步数

    // UI 控件
    QLineEdit* m_pWorkbenchPathEdit = nullptr;
    QPushButton* m_pBrowseBtn = nullptr;
    QComboBox* m_pTypeComboBox = nullptr;
    QLineEdit* m_pStepCountEdit = nullptr;
    QComboBox* m_pLaunchModeComboBox = nullptr;
    QPushButton* m_pDefaultBtn = nullptr;
    QPushButton* m_pStartBtn = nullptr;

    QMap<QString, QLineEdit*> m_paramEdits; // envVar -> QLineEdit*

    // 辅助函数
    QGroupBox* createGroupBox(const QString& title);
    QLineEdit* createParamEdit(const ParamConfig& config);
    void buildColumn4(QVBoxLayout* layout);     // 边界条件
    void buildColumn5(QVBoxLayout* layout);     // 几何参数
    void buildMaterialColumn(QVBoxLayout* layout); // 材料参数（合并药液、胶层、壳体）

    // 样式表
    QString globalStyleSheet() const;
    QString groupBoxStyleSheet() const;
    QString buttonStyleSheet() const;

    // 验证与持久化
    bool validateInputs();
    void loadSettings();
    void saveSettings();

private slots:
    void setDefaultValues();
};