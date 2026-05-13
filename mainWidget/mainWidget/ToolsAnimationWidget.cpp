#pragma once
#pragma execution_character_set("utf-8")
#include "ToolsAnimationWidget.h"
#include <AIS_Shape.hxx>
#include <AIS_ColorScale.hxx>
#include <V3d_View.hxx>

#include <QVBoxLayout>
#include <QObject>
#include <QTimer>
#include <QMouseEvent>




#include "GFImportModelWidget.h"
#include "APISetNodeValue.h"


const double XFM_TooL_ANIMATION_TIME_TOLERANCE = 1e-4;


QString xfmKeyFrameSlider_style_sheet = R"(
QSlider{
	padding-left : 10px;
	padding-right: 10px;
	max-height:22px;
	min-height:22px;
/* border:lpx solid black; */
}

QSlider::groove {
	background-color:rgba(0, 0, 0, 128) ;
	height:8px;
	padding-left:-7px;
	padding-right:-7px;
}

QSlider::handle{
	width:15px;
	height:26px;
	margin-top:-10px;
	margin-bottom:-10px;
	border-image:url(:/animation/Animation/slider_cursor.svg) ;
	border-image:url(:/animation/transparent_handle.png) ;
}

QSlider::sub-page {
	height:10px;
	background-color:#c5c5c5;
	margin-top:-lpx;
	margin-bottom:-lpx;
	border:1px solid #727272;
}

)";

ToolsAnimationWidget::ToolsAnimationWidget(QWidget* parent)
	:QWidget{ parent }
{
	init();
	bindConnect();

	setStyleSheet(xfmKeyFrameSlider_style_sheet);
}

void ToolsAnimationWidget::init()
{
	//主布局
	QVBoxLayout* pVLayout = new QVBoxLayout(this);
	pVLayout->setSpacing(0);
	pVLayout->setContentsMargins(0, 0, 0, 0);
	{
		m_animToolBtnWidget = new QWidget(this);
		m_animToolBtnWidget->setFixedHeight(20);
		QHBoxLayout* pBtnLayout = new QHBoxLayout(m_animToolBtnWidget);
		pBtnLayout->setSpacing(3);
		pBtnLayout->setContentsMargins(0, 0, 0, 0);
		{
			m_comboBox = new QComboBox();
			{
				m_comboBox->addItem("0");
				m_comboBox->addItem("1");
				m_comboBox->addItem("2");
				m_comboBox->addItem("3");
				m_comboBox->addItem("4");
				m_comboBox->addItem("5");
				m_comboBox->addItem("6");
				m_comboBox->addItem("7");
				m_comboBox->addItem("8");
				m_comboBox->addItem("9");
				m_comboBox->addItem("10");
				m_comboBox->addItem("11");
			}



			m_comboBox->setFixedHeight(20);
			m_comboBox->setFixedWidth(100);

			QWidget* btnWidget = new QWidget(this);
			QHBoxLayout* hLayout = new QHBoxLayout(btnWidget);
			hLayout->setAlignment(Qt::AlignCenter);
			hLayout->setSpacing(3);
			hLayout->setContentsMargins(0, 0, 0, 0);
			{
				m_firstBtn = new QPushButton();
				m_firstBtn->setFixedSize(22, 20);
				m_firstBtn->setIconSize(QSize(12, 12));

				m_prevBtn = new QPushButton();
				m_prevBtn->setFixedSize(22, 20);
				m_prevBtn->setIconSize(QSize(12, 12));

				m_nextBtn = new QPushButton();
				m_nextBtn->setFixedSize(22, 20);
				m_nextBtn->setIconSize(QSize(12, 12));

				m_lastBtn = new QPushButton();
				m_lastBtn->setFixedSize(22, 20);
				m_lastBtn->setIconSize(QSize(12, 12));

				m_playBtn = new QPushButton();
				m_playBtn->setFixedSize(22, 20);
				m_playBtn->setIconSize(QSize(12, 12));
			}
			hLayout->addWidget(m_firstBtn);
			hLayout->addWidget(m_prevBtn);
			hLayout->addWidget(m_playBtn);
			hLayout->addWidget(m_nextBtn);
			hLayout->addWidget(m_lastBtn);

			pBtnLayout->addWidget(m_comboBox);
			pBtnLayout->addWidget(btnWidget);
		}

		{
			m_slider = new QSlider(Qt::Horizontal);
			m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			m_slider->setContentsMargins(0, 0, 0, 0);
			m_slider->setRange(0, 100);
			m_slider->installEventFilter(this);
		}

		pVLayout->addWidget(m_animToolBtnWidget);
		pVLayout->addWidget(m_slider);
	}

	QVector<QPushButton*> btns;
	btns << m_firstBtn << m_prevBtn << m_nextBtn << m_lastBtn << m_playBtn;
	for (auto pBtn : btns)
	{
		pBtn->installEventFilter(this);
		setButtonIcon(pBtn, EnumButtonState::Normal);
	}
}

void ToolsAnimationWidget::bindConnect()
{
	connect(m_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index) {
		double time = m_comboBox->currentData().toDouble();
		emit animationTimeChanged(time);

		QWidget* parent = parentWidget();
		while (parent) {
			GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
			if (gfParent)
			{
				double min_value = 0;
				double max_value = 10;
				auto occView = gfParent->GetOccView();
				//occView->SetCameraRotationState(false);
				Handle(AIS_InteractiveContext) context = occView->getContext();
				Handle(V3d_View) view = occView->getView();
				view->SetProj(V3d_Zneg);
				view->SetTwist(-M_PI / 2.0);
				std::vector<double> nodeValues;
				APISetNodeValue::SetPreForwardDesignResult(occView, nodeValues, index);

				// 颜色条显示（与原逻辑一致）
				TCollection_ExtendedString tostr("体积分数", true);
				Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
				aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
				aColorScale->SetSize(100, 400);
				aColorScale->SetRange(min_value, max_value);
				aColorScale->SetNumberOfIntervals(9);
				aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
				aColorScale->SetTextHeight(14);
				aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
				aColorScale->SetTitle(tostr);
				aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
				aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
				aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
				Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
				context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
				context->SetDisplayMode(aColorScale, 1, Standard_False);
				context->Display(aColorScale, Standard_True);


				break;
			}
			else
			{
				parent = parent->parentWidget();
			}
		}
		/*
		if (index == 0)
		{
			QWidget* parent = parentWidget();
			while (parent) {
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{
					double min_value = 0;
					double max_value = 10;
					auto occView=gfParent->GetOccView();
					//occView->SetCameraRotationState(false);
					Handle(AIS_InteractiveContext) context = occView->getContext();
					Handle(V3d_View) view = occView->getView();
					view->SetProj(V3d_Zneg);
					std::vector<double> nodeValues;
					APISetNodeValue::SetPreForwardDesignResult0(occView, nodeValues);

					// 颜色条显示（与原逻辑一致）
					TCollection_ExtendedString tostr("跌落试验\n应力分析\n单位:MPa", true);
					Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
					aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
					aColorScale->SetSize(100, 400);
					aColorScale->SetRange(min_value, max_value);
					aColorScale->SetNumberOfIntervals(9);
					aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
					aColorScale->SetTextHeight(14);
					aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
					aColorScale->SetTitle(tostr);
					aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
					aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
					aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
					Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
					context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
					context->SetDisplayMode(aColorScale, 1, Standard_False);
					context->Display(aColorScale, Standard_True);


					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}

		}
		else if (index == 1)
		{
			QWidget* parent = parentWidget();
			while (parent) {
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{
					double min_value = 0;
					double max_value = 10;
					auto occView = gfParent->GetOccView();
					//occView->SetCameraRotationState(false);
					Handle(AIS_InteractiveContext) context = occView->getContext();
					Handle(V3d_View) view = occView->getView();
					view->SetProj(V3d_Zneg);
					std::vector<double> nodeValues;
					APISetNodeValue::SetPreForwardDesignResult1(occView, nodeValues);

					// 颜色条显示（与原逻辑一致）
					TCollection_ExtendedString tostr("跌落试验\n应力分析\n单位:MPa", true);
					Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
					aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
					aColorScale->SetSize(100, 400);
					aColorScale->SetRange(min_value, max_value);
					aColorScale->SetNumberOfIntervals(9);
					aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
					aColorScale->SetTextHeight(14);
					aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
					aColorScale->SetTitle(tostr);
					aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
					aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
					aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
					Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
					context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
					context->SetDisplayMode(aColorScale, 1, Standard_False);
					context->Display(aColorScale, Standard_True);

					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
		}
		else if (index == 2)
		{
			QWidget* parent = parentWidget();
			while (parent) {
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{
					double min_value = 0;
					double max_value = 10;
					auto occView = gfParent->GetOccView();
					//occView->SetCameraRotationState(false);
					Handle(AIS_InteractiveContext) context = occView->getContext();
					Handle(V3d_View) view = occView->getView();
					view->SetProj(V3d_Zneg);
					std::vector<double> nodeValues;
					APISetNodeValue::SetPreForwardDesignResult2(occView, nodeValues);

					// 颜色条显示（与原逻辑一致）
					TCollection_ExtendedString tostr("跌落试验\n应力分析\n单位:MPa", true);
					Handle(AIS_ColorScale) aColorScale = new AIS_ColorScale();
					aColorScale->SetFormat(TCollection_AsciiString("%.2f"));
					aColorScale->SetSize(100, 400);
					aColorScale->SetRange(min_value, max_value);
					aColorScale->SetNumberOfIntervals(9);
					aColorScale->SetLabelPosition(Aspect_TOCSP_RIGHT);
					aColorScale->SetTextHeight(14);
					aColorScale->SetColor(Quantity_Color(Quantity_NOC_BLACK));
					aColorScale->SetTitle(tostr);
					aColorScale->SetColorRange(Quantity_Color(Quantity_NOC_BLUE1), Quantity_Color(Quantity_NOC_RED));
					aColorScale->SetLabelType(Aspect_TOCSD_AUTO);
					aColorScale->SetZLayer(Graphic3d_ZLayerId_TopOSD);
					Graphic3d_Vec2i anoffset(0, Standard_Integer(450));
					context->SetTransformPersistence(aColorScale, new Graphic3d_TransformPers(Graphic3d_TMF_2d, Aspect_TOTP_LEFT_UPPER, anoffset));
					context->SetDisplayMode(aColorScale, 1, Standard_False);
					context->Display(aColorScale, Standard_True);


					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
		}
		*/

		});

	connect(m_slider, &QSlider::valueChanged, this, [&](int value) {
		double dValue = value / 100.0;
		double time = dValue * (m_maxValue - m_minValue) + m_minValue;
		emit animationTimeChanged(time);
		});

	connect(m_firstBtn, &QPushButton::clicked, this, [&]() {
		int curIdx = m_comboBox->currentIndex();
		if (curIdx == 0)
		{
			return;
		}
		else
		{
			m_comboBox->setCurrentIndex(--curIdx);
		}
		});

	connect(m_lastBtn, &QPushButton::clicked, this, [&]() {
		int curIdx = m_comboBox->currentIndex();
		if (abs(m_timeSteps[curIdx] - m_currentTime) < XFM_TooL_ANIMATION_TIME_TOLERANCE)
		{
			if (curIdx == m_timeSteps.size() - 1)
			{
				return;
			}
			else
			{
				m_comboBox->setCurrentIndex(++curIdx);
			}
		}
		else
		{
			emit animationTimeChanged(m_timeSteps[curIdx]);
		}
		});

	connect(m_prevBtn, &QPushButton::clicked, this, [&]() {
		if (m_substepIdx == 0)
		{
			return;
		}
		else
		{
			emit animationTimeChanged(m_subSteps[--m_substepIdx]);
		}
		});

	connect(m_nextBtn, &QPushButton::clicked, this, [&]() {
		if (abs(m_subSteps[m_substepIdx] - m_currentTime) < XFM_TooL_ANIMATION_TIME_TOLERANCE)
		{
			if (m_substepIdx = m_subSteps.size() - 1)
			{
				return;
			}
			else
			{
				emit animationTimeChanged(m_subSteps[m_substepIdx + 1]);
			}
		}
		else
		{
			emit animationTimeChanged(m_subSteps[m_substepIdx]);
		}
		});


	connect(m_playBtn, &QPushButton::clicked, this, [&]() {
		bool bPause = m_playBtn->property("bPause").toBool();
		m_playBtn->setProperty("bPause", !bPause);
		if (!bPause)
		{
			if (m_slider->value() == m_slider->maximum())
			{
				m_slider->setValue(0);
			}
			if (m_playTimer == nullptr)
			{
				m_playTimer = new QTimer();
				connect(m_playTimer, &QTimer::timeout, this, [&]() {
					int value = m_slider->value();
					int step = 100.0 / 8 + 0.5;
					value += step;
					if (value > 100)
					{
						value = 100;
					}
					m_slider->setValue(value);
					if (value == 100)
					{
						m_playBtn->setProperty("bPause", false);

						m_playTimer->stop();
						delete m_playTimer;
						m_playTimer = nullptr;

					}
					});
				m_playTimer->start(200);
			}
		}
		else
		{
			if (m_playTimer != nullptr)
			{
				m_playTimer->stop();
				delete m_playTimer;
				m_playTimer = nullptr;
			}
		}

		});
}

void ToolsAnimationWidget::SetAnimationRange(double min, double max)
{
	m_minValue = min; 
	m_maxValue = max;
}

void ToolsAnimationWidget::SetAnimationSteps(const QStringList& names, const QVector<double>& times)
{
	m_timeSteps = times;
	double range = times.last() - times.front();
	m_subSteps.clear();
	m_subSteps.push_back(times.front());
	for (int i = 1; i < times.size(); ++i)
	{
		double distance = times[i] - times[i - 1];
		double percent = distance / range;
		int insertSubStepNumber = 1;

		if (percent > 0.7)
		{
			insertSubStepNumber = 7;
		}
		else if (percent > 0.3)
		{
			insertSubStepNumber = 3;
		}

		for (int j = 0; j < insertSubStepNumber; ++j)
		{
			m_subSteps.push_back(times[i - 1] + (j + 1.) / (insertSubStepNumber + 1.) * distance);
		}
		m_subSteps.push_back(times[i]);
	}

	m_comboBox->blockSignals(true);
	m_comboBox->clear();
	m_comboBox->blockSignals(false);

	if (names.isEmpty() || times.isEmpty())
	{
		return;
	}

	if (names.count() != times.count())
	{
		return;
	}

	m_comboBox->blockSignals(true);
	for (int i = 0; i < names.count(); ++i)
	{
		QString name = names[i];
		double value = times[i];
		m_comboBox->addItem(name, value);
	}
	m_comboBox->blockSignals(false);

	m_comboBox->setCurrentIndex(0);
	setProcessButtonEnabled(m_comboBox->currentData().toDouble());

	m_slider->update();
}

void ToolsAnimationWidget::UpdateUI(double time)
{
	m_currentTime = time;
	int index = 0;
	
	for (int i = 0; i < m_comboBox->count(); ++i)
	{
		double value = m_comboBox->itemData(i).toDouble();
		if (time < value + XFM_TooL_ANIMATION_TIME_TOLERANCE)
		{
			index = i;
			break;
		}
	}

	m_substepIdx = 0;

	for (int j = 0; j < m_subSteps.size(); ++j)
	{
		if (time < m_subSteps[j] + XFM_TooL_ANIMATION_TIME_TOLERANCE)
		{
			m_substepIdx = j;
			break;
		}
	}
	
	m_comboBox->blockSignals(true); 
	m_comboBox->setCurrentIndex(index); 
	m_comboBox->blockSignals(false);
	//m_comboBox->AdjustWidthByCurrentText();

	m_animToolBtnWidget->setUpdatesEnabled(false);
	m_animToolBtnWidget->update();
	m_animToolBtnWidget->setUpdatesEnabled(true);

	double value = (time - m_minValue) / (m_maxValue - m_minValue) * 100; 
	m_slider->blockSignals(true);
	m_slider->setValue(value); 
	m_slider->blockSignals(false);

	setProcessButtonEnabled(time);

}

int ToolsAnimationWidget::GetCurrentStepIndex()
{
	return m_comboBox->currentIndex();
}



void ToolsAnimationWidget::setProcessButtonEnabled(double value)
{
	if (fabs(value - m_minValue) <= XFM_TooL_ANIMATION_TIME_TOLERANCE)
	{
		m_firstBtn->setEnabled(false);
		m_prevBtn->setEnabled(false);
		m_nextBtn->setEnabled(true);
		m_lastBtn->setEnabled(true);
	}
	else if (fabs(value - m_maxValue) <= XFM_TooL_ANIMATION_TIME_TOLERANCE)
	{
		m_firstBtn->setEnabled(true);
		m_prevBtn->setEnabled(true);
		m_nextBtn->setEnabled(false);
		m_lastBtn->setEnabled(false);
	}
	else
	{
		m_firstBtn->setEnabled(true); 
		m_prevBtn->setEnabled(true); 
		m_nextBtn->setEnabled(true); 
		m_lastBtn->setEnabled(true);
	}
}

void ToolsAnimationWidget::setButtonIcon(QPushButton* pBtn, EnumButtonState eState)
{
	if (pBtn == nullptr)
	{
		return;
	}

	if (pBtn == m_firstBtn)
	{
		QIcon icon(":/animation/Animation/first_frame.svg");
		if (eState == EnumButtonState::Pressed)
		{
			icon = QIcon(":/animation/Animation/first_frame_press.svg");
		}	
		else if (eState = EnumButtonState::Disabled)
		{
			icon = QIcon(":/animation/Animation/first_frame_disable.svg");
		}
		pBtn->setIcon(icon);
	}
	else if (pBtn == m_prevBtn)
	{
		QIcon icon(":/animation/Animation/prev_frame.svg"); 
		if (eState == EnumButtonState::Pressed)
		{
			icon = QIcon(":/animation/Animation/prev_frame_press.svg");
		}
		else if (eState = EnumButtonState::Disabled)
		{
			icon = QIcon(":/animation/Animation/prev_frame_disable.svg");
		}
		pBtn->setIcon(icon);
	}
	else if (pBtn == m_nextBtn)
	{
		QIcon icon(":/animation/Animation/next_frame.svg"); 
		if (eState == EnumButtonState::Pressed)
		{
			icon = QIcon(":/animation/Animation/next_frame_press.svg");
		}
		else if (eState = EnumButtonState::Disabled)
		{
			icon = QIcon(":/animation/Animation/next_frame_disable.svg");
		}
		pBtn->setIcon(icon);
	}
	else if (pBtn == m_lastBtn)
	{
		QIcon icon(":/animation/Animation/last_frame.svg"); 
		if (eState == EnumButtonState::Pressed)
		{
			icon = QIcon(":/animation/Animation/last_frame_press.svg");
		}
		else if (eState == EnumButtonState::Disabled)
		{
			icon = QIcon(":/animation/Animation/last_frame_disable.svg");
		}
		pBtn->setIcon(icon);
	}
	else if (pBtn == m_playBtn)
	{
		bool bPause = m_playBtn->property("bPause").toBool();
		if (bPause)
		{
			QIcon icon(":/animation/Animation/stop_frame.svg"); 
			if (eState == EnumButtonState::Pressed)
			{
				icon = QIcon(":/animation/Animation/stop_frame_press.svg");
			}
			else if (eState == EnumButtonState:: Disabled)
			{
				icon = QIcon(":/animation/Animation/stop_frame_disable.svg");
			}
			pBtn->setIcon(icon);
		}
		else
		{
			QIcon icon(":/animation/Animation/play_frame.svg");
			if (eState == EnumButtonState::Pressed)
			{
				icon = QIcon(":/animation/Animation/play_frame_press.svg");
			}
			else if (eState == EnumButtonState::Disabled)
			{
				icon = QIcon(":/animation/Animation/play_frame_disable.svg");
			}
			pBtn->setIcon(icon);
		}
	}
}

bool ToolsAnimationWidget::eventFilter(QObject* obj, QEvent* ev)
{
	if (obj == m_firstBtn || obj == m_prevBtn || obj == m_nextBtn || obj == m_lastBtn || obj == m_playBtn)
	{
		//QPushButton* pBtn = dynamic_cast<QPushButton*>(obj);
		//if (ev->type() == QEvent::MouseButtonPress)
		//{
		//	if (pBtn->isEnabled())
		//	{
		//		setButtonIcon(pBtn, EnumButtonState::Pressed);
		//	}
		//}
		//else if (ev->type() == QEvent::MouseButtonRelease)
		//{
		//	if (pBtn->isEnabled())
		//	{
		//		setButtonIcon(pBtn, EnumButtonState::Normal);
		//	}
		//}
		//else if (ev->type() == QEvent::EnabledChange)
		//{
		//	setButtonIcon(pBtn, pBtn->isEnabled() ? EnumButtonState :: Normal : EnumButtonState : : Disabled);
		//}
	}
	else if (obj = m_slider)
	{
		if (ev->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(ev);
			if (mouseEvent->button() == Qt::LeftButton)
			{
				int value = QStyle::sliderValueFromPosition(
					m_slider->minimum(), m_slider->maximum(), mouseEvent->pos().x(), m_slider->width());

				m_slider->setValue(value);
			}
		}
	}
	return QWidget::eventFilter(obj, ev);
}
