#include "BaseCurvePlotWidget.h"

BaseCurvePlotWidget::BaseCurvePlotWidget(QWidget* parent) : QWidget(parent)
{
    init();
    bindConnect();
}

void BaseCurvePlotWidget::init()
{
    m_TitleLabel = new QLabel("aa");
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    {
        m_BaseCurvePlot = new BaseCurvePlot(this);
        m_BaseCurvePlot->setFixedHeight(350);
    }
    layout->addWidget(m_TitleLabel);
    layout->addWidget(m_BaseCurvePlot);
    layout->addStretch(1);
}

void BaseCurvePlotWidget::bindConnect()
{
}

BaseCurvePlot* BaseCurvePlotWidget::GetCurvePlot()
{
    return m_BaseCurvePlot;
}

void BaseCurvePlotWidget::SetXName(const QString& str)
{
    m_BaseCurvePlot->SetXName(str);
}

void BaseCurvePlotWidget::SetYName(const QString& str)
{
    m_BaseCurvePlot->SetYName(str);
}

void BaseCurvePlotWidget::SetTitleName(const QString& str)
{
    m_TitleLabel->setText(str);
}

void BaseCurvePlotWidget::AddDataPoint(const QVector<double>& x, const QVector<double>& y)
{
    m_BaseCurvePlot->AddDataPoint(x, y);
}
