#include "BaseCurvePlotWidget.h"

BaseCurvePlotWidget::BaseCurvePlotWidget(QWidget* parent) : QWidget(parent)
{
	init();
	bindConnect();
}

void BaseCurvePlotWidget::init()
{
	m_TitleLabel = new QLabel("");
	m_TitleLabel->setAlignment(Qt::AlignCenter);
	m_TitleLabel->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
	m_TitleLabel->setStyleSheet("color: #333333; padding: 2px;");  // padding 从 4px 减到 2px
	m_TitleLabel->setVisible(false);

	auto layout = new QVBoxLayout(this);
	layout->setContentsMargins(4, 2, 4, 2);  // 顶部 margin 从 6 减到 2
	layout->setSpacing(2);                   // 标题和图表间距从 4 减到 2

	m_BaseCurvePlot = new BaseCurvePlot(this);
	m_BaseCurvePlot->setMinimumHeight(120);
	m_BaseCurvePlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	layout->addWidget(m_TitleLabel);
	layout->addWidget(m_BaseCurvePlot, 1);
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
	m_TitleLabel->setVisible(!str.isEmpty());
}

void BaseCurvePlotWidget::AddDataPoint(const QVector<double>& x, const QVector<double>& y)
{
	m_BaseCurvePlot->AddDataPoint(x, y);
}