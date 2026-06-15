#pragma once
#include <QWidget>
#include <QMap>
#include <QVBoxLayout>
class QLineEdit;
class QPushButton;
class QGroupBox;
class QFormLayout;

class PreheatingParamWidget : public QWidget
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

	explicit PreheatingParamWidget(QWidget* parent = nullptr);
	~PreheatingParamWidget();

private:
	void initUI();
	void bindConnect();
	void loadSettings();
	void saveSettings();
	bool validateInputs();

	QGroupBox* createGroupBox(const QString& title);
	QLineEdit* createParamEdit(const ParamConfig& config);
	void buildLeftColumn(QVBoxLayout* layout);
	void buildRightColumn(QVBoxLayout* layout);

	QString globalStyleSheet() const;
	QString groupBoxStyleSheet() const;
	QString buttonStyleSheet() const;

	void setDefaultValues();

private:
	QLineEdit* m_pWorkbenchPathEdit = nullptr;
	QPushButton* m_pBrowseBtn = nullptr;
	QMap<QString, QLineEdit*> m_paramEdits;
	QPushButton* m_pStartBtn = nullptr;

	static const QMap<QString, QString> s_defaultParamValues;
	QPushButton* m_pDefaultBtn = nullptr;

	static const QList < ParamConfig > s_leftParams;
	static const QList < ParamConfig > s_rightParams;
};