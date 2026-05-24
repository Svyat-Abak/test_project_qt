#include "SpectrumVisualizer.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QRandomGenerator>
#include <QTimer>

SpectrumVisualizer::SpectrumVisualizer(int barCount, QWidget *parent)
    : QWidget(parent)
    , m_layout(new QHBoxLayout(this))
    , m_timer(new QTimer(this)) {
    setObjectName(QStringLiteral("spectrumVisualizer"));
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);

    buildBars(barCount);

    m_timer->setInterval(90);
    connect(m_timer, &QTimer::timeout, this, &SpectrumVisualizer::tick);
}

void SpectrumVisualizer::setActive(bool active) {
    m_active = active;
    if (m_active) {
        m_timer->start();
    } else {
        m_timer->stop();
        for (QProgressBar *bar : m_bars) {
            bar->setValue(0);
        }
    }
}

void SpectrumVisualizer::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
}

void SpectrumVisualizer::tick() {
    if (!m_active) {
        return;
    }
    for (QProgressBar *bar : m_bars) {
        bar->setValue(QRandomGenerator::global()->bounded(15, 100));
    }
}

void SpectrumVisualizer::buildBars(int count) {
    for (QProgressBar *bar : m_bars) {
        m_layout->removeWidget(bar);
        bar->deleteLater();
    }
    m_bars.clear();

    for (int i = 0; i < count; ++i) {
        auto *bar = new QProgressBar(this);
        bar->setTextVisible(false);
        bar->setRange(0, 100);
        bar->setValue(0);
        bar->setOrientation(Qt::Vertical);
        bar->setFixedWidth(8);
        bar->setFixedHeight(48);
        bar->setObjectName(QStringLiteral("spectrumBar"));
        m_layout->addWidget(bar);
        m_bars.append(bar);
    }
}
