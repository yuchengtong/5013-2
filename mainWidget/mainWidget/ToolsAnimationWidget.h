#pragma once
#include <QWidget>
#include <QEvent>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>

class ToolsAnimationWidget : public QWidget
{
	Q_OBJECT
public:
	explicit ToolsAnimationWidget(QWidget* parent = nullptr);

	void SetAnimationRange(double min, double max);

	void SetAnimationSteps(const QStringList& names, const QVector<double>& times);

	void UpdateUI(double time);

	int GetCurrentStepIndex();

private:
	void init();

	void bindConnect();

	void setProcessButtonEnabled(double value);


	enum EnumButtonState
	{
		Normal,
		Hover,
		Pressed,
		Checked,
		Disabled,
	};
	void setButtonIcon(QPushButton* pBtn, EnumButtonState eState);


protected:
	bool eventFilter(QObject* obj,QEvent* ev) override;

signals:
	void animationTimeChanged(double);

private:
	QComboBox* m_comboBox;
	QPushButton* m_firstBtn;
	QPushButton* m_prevBtn;
	QPushButton* m_nextBtn;
	QPushButton* m_lastBtn;
	QPushButton* m_playBtn;
	QWidget* m_animToolBtnWidget = nullptr; 
	QSlider* m_slider;
	QTimer* m_playTimer = nullptr;
	int m_substepIdx = 0;

private:
	QVector<double> m_timeSteps;
	QVector<double> m_subSteps;
	double m_minValue = 0.0;
	double m_maxValue = 0.0;
	double m_currentTime = 0.0;


};

