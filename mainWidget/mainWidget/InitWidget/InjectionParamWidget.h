#pragma once
#include <QWidget>
#include <QMap>

class QLineEdit;
class QPushButton;
class QGroupBox;
class QFormLayout;
class QVBoxLayout;
class QComboBox;

class InjectionParamWidget : public QWidget
{
	Q_OBJECT

public:
	struct ParamConfig {
		QString label;
		QString placeholder;
		QString envVar;
		QString defaultValue;
		bool required;
		QPair<double, double> range;
	};

	explicit InjectionParamWidget(QWidget* parent = nullptr);
	~InjectionParamWidget();

private:
	void initUI();
	void bindConnect();
	void loadSettings();
	void saveSettings();
	bool validateInputs();

	QGroupBox* createGroupBox(const QString& title);
	QLineEdit* createParamEdit(const ParamConfig& config);
	void buildColumn1(QVBoxLayout* layout);
	void buildColumn2(QVBoxLayout* layout);
	void buildColumn3(QVBoxLayout* layout);
	void buildColumn4(QVBoxLayout* layout);

	QString globalStyleSheet() const;
	QString groupBoxStyleSheet() const;
	QString buttonStyleSheet() const;

	QString defaultButtonStyleSheet() const;

	void setDefaultValues(); 

private:
	QLineEdit* m_pWorkbenchPathEdit = nullptr;
	QPushButton* m_pBrowseBtn = nullptr;
	QComboBox* m_pTypeComboBox = nullptr;
	QMap<QString, QLineEdit*> m_paramEdits;
	QPushButton* m_pStartBtn = nullptr;
	QPushButton* m_pDefaultBtn = nullptr; 

	static const QList < ParamConfig > s_col1Params;
	static const QList < ParamConfig > s_col2Params;
	static const QList < ParamConfig > s_col3Params;
	static const QList < ParamConfig > s_col4Params;
	static const QMap<QString, QString> s_defaultParamValues1;
	static const QMap<QString, QString> s_defaultParamValues2;
	static const QMap<QString, QString> s_defaultParamValues3;
	static const QMap<QString, QString> s_defaultParamValues4;
};