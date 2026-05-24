#include "CoverLabel.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>

CoverLabel::CoverLabel(QWidget *parent)
    : QLabel(parent) {
    setMinimumSize(100, 100);
    setAlignment(Qt::AlignCenter);
    setObjectName(QStringLiteral("coverLabel"));
    m_placeholder = QStringLiteral("No Cover");
}

void CoverLabel::setCoverPixmap(const QPixmap &pixmap) {
    m_sourcePixmap = pixmap;
    update();
}

void CoverLabel::loadCoverFromPath(const QString &path) {
    if (path.isEmpty()) {
        clearCover();
        return;
    }
    QPixmap pixmap(path);
    if (pixmap.isNull()) {
        clearCover();
        return;
    }
    m_sourcePixmap = pixmap;
    update();
}

void CoverLabel::setPlaceholderText(const QString &text) {
    m_placeholder = text;
    update();
}

void CoverLabel::clearCover() {
    m_sourcePixmap = QPixmap();
    update();
}

void CoverLabel::setFavoriteBadge(bool enabled) {
    m_favoriteBadge = enabled;
    update();
}

void CoverLabel::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRect rect = this->rect().adjusted(8, 8, -8, -8);
    QPainterPath clip;
    clip.addRoundedRect(rect, 18, 18);
    painter.setClipPath(clip);

    painter.fillRect(rect, QColor(21, 21, 21));

    if (!m_sourcePixmap.isNull()) {
        const QPixmap scaled = scaledPixmap();
        const QPoint topLeft(rect.x() + (rect.width() - scaled.width()) / 2,
                             rect.y() + (rect.height() - scaled.height()) / 2);
        painter.drawPixmap(topLeft, scaled);
    } else {
        painter.setPen(QColor(0, 229, 255, 180));
        painter.setFont(font());
        painter.drawText(rect, Qt::AlignCenter, m_placeholder);
    }

    painter.setClipping(false);
    QPen glowPen(QColor(0, 229, 255, 120), 2);
    painter.setPen(glowPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect, 18, 18);

    QPen glowOuter(QColor(255, 0, 204, 60), 4);
    painter.setPen(glowOuter);
    painter.drawRoundedRect(rect.adjusted(-2, -2, 2, 2), 20, 20);

    if (m_favoriteBadge) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 0, 102, 220));
        painter.drawEllipse(rect.topRight() + QPoint(-28, 8), 12, 12);
        painter.setPen(Qt::white);
        QFont f = painter.font();
        f.setPointSize(9);
        painter.setFont(f);
        painter.drawText(QRect(rect.right() - 34, rect.top() + 2, 24, 24), Qt::AlignCenter, QStringLiteral("♥"));
    }
}

void CoverLabel::mousePressEvent(QMouseEvent *event) {
    emit clicked();
    QLabel::mousePressEvent(event);
}

void CoverLabel::resizeEvent(QResizeEvent *event) {
    QLabel::resizeEvent(event);
    update();
}

QPixmap CoverLabel::scaledPixmap() const {
    const QSize target = size() - QSize(32, 32);
    return m_sourcePixmap.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
