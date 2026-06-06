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

ToolsAnimationWidget::ToolsAnimationWidget(QWidget* parent)
    : QWidget{ parent }
{
    init();
    bindConnect();
}

void ToolsAnimationWidget::init()
{
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
    // comboBox 变化
    connect(m_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index) {
        m_slider->blockSignals(true);
        m_slider->setValue(index);
        m_slider->blockSignals(false);
        emit animationFrameChanged(index);
        UpdateUI(index);
        });

    // slider 变化
    connect(m_slider, &QSlider::valueChanged, this, [&](int value) {
        m_comboBox->blockSignals(true);
        m_comboBox->setCurrentIndex(value);
        m_comboBox->blockSignals(false);
        emit animationFrameChanged(value);
        UpdateUI(value);
        });

    // 按钮：统一走 goToFrame
    connect(m_firstBtn, &QPushButton::clicked, this, [&]() {
        goToFrame(0);
        });

    connect(m_prevBtn, &QPushButton::clicked, this, [&]() {
        if (m_currentFrame > 0) goToFrame(m_currentFrame - 1);
        });

    connect(m_nextBtn, &QPushButton::clicked, this, [&]() {
        if (m_currentFrame < m_frameCount - 1) goToFrame(m_currentFrame + 1);
        });

    connect(m_lastBtn, &QPushButton::clicked, this, [&]() {
        goToFrame(m_frameCount - 1);
        });

    // 播放
    connect(m_playBtn, &QPushButton::clicked, this, [&]() {
        bool bPause = m_playBtn->property("bPause").toBool();
        m_playBtn->setProperty("bPause", !bPause);
        if (!bPause)
        {
            if (m_currentFrame == m_frameCount - 1)
            {
                goToFrame(0);
            }
            if (m_playTimer == nullptr)
            {
                m_playTimer = new QTimer();
                connect(m_playTimer, &QTimer::timeout, this, [&]() {
                    int nextFrame = m_currentFrame + 1;
                    if (nextFrame >= m_frameCount)
                    {
                        nextFrame = m_frameCount - 1;
                        m_playBtn->setProperty("bPause", false);
                        m_playTimer->stop();
                        delete m_playTimer;
                        m_playTimer = nullptr;
                    }
                    goToFrame(nextFrame);
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

void ToolsAnimationWidget::SetAnimationSteps(const QStringList& names)
{
	m_frameCount = names.count();
	m_subSteps.clear();

	for (int i = 0; i < m_frameCount; ++i)
	{
		m_subSteps.push_back(i);
	}

	m_comboBox->blockSignals(true);
	m_comboBox->clear();
	m_comboBox->blockSignals(false);

	if (names.isEmpty()) return;

	m_comboBox->blockSignals(true);
	for (int i = 0; i < names.count(); ++i)
	{
		m_comboBox->addItem(names[i], i);
	}


	int lastIndex = m_frameCount - 1;

	// comboBox 默认显示最后一项
	m_comboBox->setCurrentIndex(lastIndex);
    m_comboBox->blockSignals(false);

	// slider 也同步到最后一帧
	m_slider->blockSignals(true);
	m_slider->setRange(0, lastIndex);
	m_slider->setSingleStep(1);
	m_slider->setPageStep(1);
	m_slider->setValue(lastIndex);
	m_slider->blockSignals(false);

	// 内部状态同步为最后一帧
	m_currentFrame = lastIndex;
	setButtonState(lastIndex);
	m_slider->update();
}

void ToolsAnimationWidget::goToFrame(int frameIndex)
{
    if (frameIndex < 0 || frameIndex >= m_frameCount)
        return;

    m_comboBox->blockSignals(true);
    m_comboBox->setCurrentIndex(frameIndex);
    m_comboBox->blockSignals(false);

    m_slider->blockSignals(true);
    m_slider->setValue(frameIndex);
    m_slider->blockSignals(false);

    m_currentFrame = frameIndex;
    setButtonState(frameIndex);

    emit animationFrameChanged(frameIndex);
}

void ToolsAnimationWidget::UpdateUI(int frameIndex)
{
    if (frameIndex < 0 || frameIndex >= m_frameCount)
        return;

    m_currentFrame = frameIndex;

    m_comboBox->blockSignals(true);
    m_comboBox->setCurrentIndex(frameIndex);
    m_comboBox->blockSignals(false);

    m_slider->blockSignals(true);
    m_slider->setValue(frameIndex);
    m_slider->blockSignals(false);

    setButtonState(frameIndex);

    m_animToolBtnWidget->setUpdatesEnabled(false);
    m_animToolBtnWidget->update();
    m_animToolBtnWidget->setUpdatesEnabled(true);
}

int ToolsAnimationWidget::GetCurrentFrameIndex()
{
    return m_currentFrame;
}

void ToolsAnimationWidget::setButtonState(int frameIndex)
{
    m_firstBtn->setEnabled(frameIndex > 0);
    m_prevBtn->setEnabled(frameIndex > 0);
    m_nextBtn->setEnabled(frameIndex < m_frameCount - 1);
    m_lastBtn->setEnabled(frameIndex < m_frameCount - 1);
}

void ToolsAnimationWidget::setButtonIcon(QPushButton* pBtn, EnumButtonState eState)
{
    if (pBtn == nullptr) return;

    if (pBtn == m_firstBtn)
    {
        QIcon icon(":/animation/Animation/first_frame.svg");
        if (eState == EnumButtonState::Pressed)
            icon = QIcon(":/animation/Animation/first_frame_press.svg");
        else if (eState == EnumButtonState::Disabled)
            icon = QIcon(":/animation/Animation/first_frame_disable.svg");
        pBtn->setIcon(icon);
    }
    else if (pBtn == m_prevBtn)
    {
        QIcon icon(":/animation/Animation/prev_frame.svg");
        if (eState == EnumButtonState::Pressed)
            icon = QIcon(":/animation/Animation/prev_frame_press.svg");
        else if (eState == EnumButtonState::Disabled)
            icon = QIcon(":/animation/Animation/prev_frame_disable.svg");
        pBtn->setIcon(icon);
    }
    else if (pBtn == m_nextBtn)
    {
        QIcon icon(":/animation/Animation/next_frame.svg");
        if (eState == EnumButtonState::Pressed)
            icon = QIcon(":/animation/Animation/next_frame_press.svg");
        else if (eState == EnumButtonState::Disabled)
            icon = QIcon(":/animation/Animation/next_frame_disable.svg");
        pBtn->setIcon(icon);
    }
    else if (pBtn == m_lastBtn)
    {
        QIcon icon(":/animation/Animation/last_frame.svg");
        if (eState == EnumButtonState::Pressed)
            icon = QIcon(":/animation/Animation/last_frame_press.svg");
        else if (eState == EnumButtonState::Disabled)
            icon = QIcon(":/animation/Animation/last_frame_disable.svg");
        pBtn->setIcon(icon);
    }
    else if (pBtn == m_playBtn)
    {
        bool bPause = m_playBtn->property("bPause").toBool();
        if (bPause)
        {
            QIcon icon(":/animation/Animation/stop_frame.svg");
            if (eState == EnumButtonState::Pressed)
                icon = QIcon(":/animation/Animation/stop_frame_press.svg");
            else if (eState == EnumButtonState::Disabled)
                icon = QIcon(":/animation/Animation/stop_frame_disable.svg");
            pBtn->setIcon(icon);
        }
        else
        {
            QIcon icon(":/animation/Animation/play_frame.svg");
            if (eState == EnumButtonState::Pressed)
                icon = QIcon(":/animation/Animation/play_frame_press.svg");
            else if (eState == EnumButtonState::Disabled)
                icon = QIcon(":/animation/Animation/play_frame_disable.svg");
            pBtn->setIcon(icon);
        }
    }
}

bool ToolsAnimationWidget::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == m_slider)
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