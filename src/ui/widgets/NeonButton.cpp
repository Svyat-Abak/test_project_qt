#include "NeonButton.h"

#include <QEnterEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

NeonButton::NeonButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent) {
    setCursor(Qt::PointingHandCursor);
    setFixedSize(48, 48);
    setObjectName(QStringLiteral("neonButton"));
    m_glowAnimation = new QPropertyAnimation(this, "glowOpacity", this);
    m_glowAnimation->setDuration(180);
}

NeonButton::NeonButton(const QIcon &icon, QWidget *parent)
    : NeonButton(QString(), parent) {
    setIcon(icon);
    setIconSize(QSize(22, 22));
}

qreal NeonButton::glowOpacity() const { return m_glowOpacity; }

void NeonButton::setGlowOpacity(qreal value) {
    m_glowOpacity = value;
    update();
}

void NeonButton::enterEvent(QEnterEvent *event) {
    QPushButton::enterEvent(event);
    animateGlow(1.0);
}

void NeonButton::leaveEvent(QEvent *event) {
    QPushButton::leaveEvent(event);
    animateGlow(0.35);
    m_pressed = false;
}

void NeonButton::mousePressEvent(QMouseEvent *event) {
    m_pressed = true;
    setGlowOpacity(0.15);
    QPushButton::mousePressEvent(event);
}

void NeonButton::mouseReleaseEvent(QMouseEvent *event) {
    m_pressed = false;
    animateGlow(underMouse() ? 1.0 : 0.35);
    QPushButton::mouseReleaseEvent(event);
}

void NeonButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF circleRect = rect().adjusted(4, 4, -4, -4);
    QPainterPath path;
    path.addEllipse(circleRect);

    QColor fill(21, 21, 21);
    if (m_pressed) {
        fill = QColor(30, 30, 30);
    }
    painter.fillPath(path, fill);

    QPen glow(QColor(0, 229, 255, int(255 * m_glowOpacity)), 2);
    painter.setPen(glow);
    painter.drawPath(path);

    if (m_glowOpacity > 0.5) {
        QPen outer(QColor(255, 0, 204, int(120 * m_glowOpacity)), 4);
        painter.setPen(outer);
        painter.drawEllipse(circleRect.adjusted(-2, -2, 2, 2));
    }

    if (!icon().isNull()) {
        const QPixmap pm = icon().pixmap(iconSize());
        painter.drawPixmap((width() - pm.width()) / 2, (height() - pm.height()) / 2, pm);
    } else if (!text().isEmpty()) {
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(rect(), Qt::AlignCenter, text());
    }
}

void NeonButton::animateGlow(qreal target) {
    m_glowAnimation->stop();
    m_glowAnimation->setStartValue(m_glowOpacity);
    m_glowAnimation->setEndValue(target);
    m_glowAnimation->start();
}
