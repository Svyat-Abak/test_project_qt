#include "ParticleBackground.h"

#include <QPaintEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QTimer>

ParticleBackground::ParticleBackground(QWidget *parent)
    : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ParticleBackground::step);
    m_timer->start(33);
}

void ParticleBackground::initParticles() {
    m_particles.clear();
    const int w = qMax(width(), 1);
    const int h = qMax(height(), 1);
    const int count = 36;
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = QPointF(QRandomGenerator::global()->bounded(w),
                        QRandomGenerator::global()->bounded(h));
        p.velocity = QPointF(QRandomGenerator::global()->bounded(-30, 30) / 10.0,
                             QRandomGenerator::global()->bounded(-30, 30) / 10.0);
        if (qFuzzyIsNull(p.velocity.x()) && qFuzzyIsNull(p.velocity.y())) {
            p.velocity = QPointF(0.8, 0.5);
        }
        p.radius = QRandomGenerator::global()->bounded(24, 70);
        p.color = (i % 2 == 0) ? QColor(0, 229, 255, 70) : QColor(255, 0, 204, 55);
        m_particles.append(p);
    }
}

void ParticleBackground::step() {
    if (m_particles.isEmpty()) {
        initParticles();
    }
    const float w = qMax(1, width());
    const float h = qMax(1, height());
    for (Particle &p : m_particles) {
        p.pos += p.velocity;
        if (p.pos.x() < -p.radius) {
            p.pos.setX(w + p.radius);
        } else if (p.pos.x() > w + p.radius) {
            p.pos.setX(-p.radius);
        }
        if (p.pos.y() < -p.radius) {
            p.pos.setY(h + p.radius);
        } else if (p.pos.y() > h + p.radius) {
            p.pos.setY(-p.radius);
        }
    }
    update();
}

void ParticleBackground::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient bg(0, 0, width(), height());
    bg.setColorAt(0.0, QColor(12, 12, 20));
    bg.setColorAt(0.5, QColor(18, 10, 28));
    bg.setColorAt(1.0, QColor(10, 14, 22));
    painter.fillRect(rect(), bg);

    for (const Particle &p : m_particles) {
        QRadialGradient grad(p.pos, p.radius);
        grad.setColorAt(0.0, p.color);
        grad.setColorAt(0.45, QColor(p.color.red(), p.color.green(), p.color.blue(), p.color.alpha() / 2));
        grad.setColorAt(1.0, Qt::transparent);
        painter.setBrush(grad);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(p.pos, p.radius, p.radius);
    }
}

void ParticleBackground::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (width() > 100 && height() > 100) {
        initParticles();
    }
}
