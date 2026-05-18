#pragma once
#include <QWidget>
#include <QEvent>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QTimer>

class ToolsAnimationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToolsAnimationWidget(QWidget* parent = nullptr);
    void SetAnimationSteps(const QStringList& names);
    void UpdateUI(int frameIndex);
    int GetCurrentFrameIndex();

private:
    void init();
    void bindConnect();
    void setButtonState(int frameIndex);
    void goToFrame(int frameIndex);  // ¡û ÐÂÔö

    enum EnumButtonState
    {
        Normal, Hover, Pressed, Checked, Disabled,
    };
    void setButtonIcon(QPushButton* pBtn, EnumButtonState eState);

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;

signals:
    void animationFrameChanged(int frameIndex);

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

    int m_frameCount = 0;
    int m_currentFrame = 0;
    QVector<int> m_subSteps;
};