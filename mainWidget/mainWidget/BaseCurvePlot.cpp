#pragma execution_character_set("utf-8")
#include "BaseCurvePlot.h"
#include <algorithm>

BaseCurvePlot::BaseCurvePlot(QWidget* parent) : QCustomPlot(parent)
{
	init();
	this->setMouseTracking(true);
}

void BaseCurvePlot::init()
{
	this->addGraph();
	this->graph(0)->setPen(QPen(Qt::blue, 2));
	this->graph(0)->setName(QString::fromLocal8Bit(""));

	this->setInteraction(QCP::iRangeDrag, true);
	this->setInteraction(QCP::iRangeZoom, true);
	this->xAxis->grid()->setVisible(true);
	this->yAxis->grid()->setVisible(true);

	// === 修复：增加底部边距防止 X 轴标签被截断，减小顶部边距 ===
	this->axisRect()->setMinimumMargins(QMargins(10, 4, 10, 18));
	this->xAxis->setLabelPadding(2);

	// === 修复：隐藏图例，消除顶部多余留白 ===
	this->legend->setVisible(false);

	m_vLine = new QCPItemLine(this);
	m_vLine->setPen(QPen(Qt::red, 1, Qt::DashLine));
	m_vLine->setVisible(false);

	m_hLine = new QCPItemLine(this);
	m_hLine->setPen(QPen(Qt::red, 1, Qt::DashLine));
	m_hLine->setVisible(false);

	m_tracer = new QCPItemTracer(this);
	m_tracer->setPen(QPen(Qt::red, 2));
	m_tracer->setBrush(QBrush(Qt::red));
	m_tracer->setSize(6);
	m_tracer->setStyle(QCPItemTracer::tsCircle);
	m_tracer->setGraph(this->graph(0));
	m_tracer->setVisible(false);

	m_textLabel = new QCPItemText(this);
	m_textLabel->setVisible(false);
	m_textLabel->setPositionAlignment(Qt::AlignBottom | Qt::AlignHCenter);
	m_textLabel->setPen(QPen(Qt::black));
	m_textLabel->setBrush(QBrush(QColor(255, 255, 255, 200)));
	m_textLabel->setFont(QFont("Microsoft YaHei", 9, QFont::Bold));
	m_textLabel->setPadding(QMargins(5, 3, 5, 3));

	// 右上角自适应按钮
	m_resetBtn = new QPushButton(this);
	m_resetBtn->setIcon(QIcon(":/plot/src/icon/Plot/reset.svg"));
	m_resetBtn->setFixedSize(24, 24);
	m_resetBtn->setCursor(Qt::PointingHandCursor);
	m_resetBtn->setStyleSheet(
		"QPushButton {"
		"  background-color: rgba(255, 255, 255, 200);"
		"  border: 1px solid #cccccc;"
		"  border-radius: 4px;"
		"  padding: 2px;"
		"}"
		"QPushButton:hover {"
		"  background-color: rgba(240, 240, 240, 220);"
		"  border: 1px solid #999999;"
		"}"
		"QPushButton:pressed {"
		"  background-color: rgba(220, 220, 220, 240);"
		"}"
	);
	m_resetBtn->setToolTip(QString::fromLocal8Bit("自适应坐标轴"));

	connect(m_resetBtn, &QPushButton::clicked, this, [this]() {
		if (graphCount() > 0 && !graph(0)->data()->isEmpty())
		{
			this->rescaleAxes();
			this->xAxis->scaleRange(1.05, this->xAxis->range().center());
			this->yAxis->scaleRange(1.1, this->yAxis->range().center());
			this->replot();
		}
		});
	updateBtnPosition();
}

void BaseCurvePlot::updateBtnPosition()
{
	if (m_resetBtn)
	{
		int margin = 8;
		int x = this->width() - m_resetBtn->width() - margin;
		int y = margin;
		m_resetBtn->move(x, y);
		m_resetBtn->raise();  // 确保按钮在最上层
	}
}

void BaseCurvePlot::resizeEvent(QResizeEvent* event)
{
	QCustomPlot::resizeEvent(event);
	updateBtnPosition();
}

void BaseCurvePlot::AddDataPoint(const QVector<double>& x, const QVector<double>& y)
{
	if (x.isEmpty() || x.size() != y.size())
	{
		return;
	}

	this->graph(0)->setData(x, y);

	// 设置 X 轴范围
	this->xAxis->setRange(x.first(), x.last());

	// 设置 Y 轴范围（增加边距，防止所有 Y 相同导致 margin 为 0）
	double minY = *std::min_element(y.constBegin(), y.constEnd());
	double maxY = *std::max_element(y.constBegin(), y.constEnd());
	double margin = (maxY - minY) * 0.1;
	if (margin < 1e-10) margin = 1.0;
	this->yAxis->setRange(minY - margin, maxY + margin);

	this->replot();
}

void BaseCurvePlot::SetXName(const QString& str)
{
	this->xAxis->setLabel(str);
}

void BaseCurvePlot::SetYName(const QString& str)
{
	this->yAxis->setLabel(str);
}

void BaseCurvePlot::mouseMoveEvent(QMouseEvent* event)
{
	double mouseX = event->pos().x();
	double mouseY = event->pos().y();

	double key = this->xAxis->pixelToCoord(mouseX);
	double value = this->yAxis->pixelToCoord(mouseY);

	bool hoverOnCurve = false;

	if (this->graphCount() > 0 && !this->graph(0)->data()->isEmpty())
	{
		QCPGraph* graph = this->graph(0);
		auto data = graph->data();

		// 使用 findBegin 查找起始位置，然后比较前后点找到最近点
		auto it = data->findBegin(key, false);

		if (it != data->constEnd())
		{
			// 候选点：it
			double bestKey = it->key;
			double bestValue = it->value;
			double bestDist = qAbs(it->key - key);

			// 比较前一个点
			if (it != data->constBegin())
			{
				auto itPrev = it - 1;
				double prevDist = qAbs(itPrev->key - key);
				if (prevDist < bestDist)
				{
					bestKey = itPrev->key;
					bestValue = itPrev->value;
					bestDist = prevDist;
				}
			}

			// 比较后一个点
			auto itNext = it + 1;
			if (itNext != data->constEnd())
			{
				double nextDist = qAbs(itNext->key - key);
				if (nextDist < bestDist)
				{
					bestKey = itNext->key;
					bestValue = itNext->value;
					bestDist = nextDist;
				}
			}

			double pointX = bestKey;
			double pointY = bestValue;

			// 计算像素距离（欧几里得距离，同时考虑 X 和 Y 方向）
			double pointXPixel = this->xAxis->coordToPixel(pointX);
			double pointYPixel = this->yAxis->coordToPixel(pointY);
			double pixelDist = qSqrt(qPow(pointXPixel - mouseX, 2) + qPow(pointYPixel - mouseY, 2));

			if (pixelDist < 15.0)
			{
				hoverOnCurve = true;

				m_vLine->start->setCoords(pointX, this->yAxis->range().lower);
				m_vLine->end->setCoords(pointX, pointY);

				m_hLine->start->setCoords(this->xAxis->range().lower, pointY);
				m_hLine->end->setCoords(pointX, pointY);

				m_tracer->setVisible(true);
				m_tracer->setGraphKey(pointX);
				m_tracer->updatePosition();

				m_textLabel->setVisible(true);
				m_textLabel->setText(QString("X: %1\nY: %2").arg(pointX, 0, 'f', 2).arg(pointY, 0, 'f', 2));
				m_textLabel->position->setCoords(pointX, pointY);
			}
		}
	}

	if (hoverOnCurve)
	{
		m_vLine->setVisible(true);
		m_hLine->setVisible(true);
	}
	else
	{
		m_vLine->setVisible(false);
		m_hLine->setVisible(false);
		m_tracer->setVisible(false);
		m_textLabel->setVisible(false);
	}

	this->replot();
	QCustomPlot::mouseMoveEvent(event);
}