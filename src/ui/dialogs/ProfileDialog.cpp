#include "ProfileDialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
QString formatListenTime(qint64 ms) {
    const qint64 totalMinutes = ms / 60000;
    const qint64 hours = totalMinutes / 60;
    const qint64 minutes = totalMinutes % 60;
    if (hours > 0) {
        return QStringLiteral("%1 ч %2 мин").arg(hours).arg(minutes);
    }
    return QStringLiteral("%1 мин").arg(qMax<qint64>(1, minutes));
}
} // namespace

BarChartWidget::BarChartWidget(QWidget *parent)
    : QWidget(parent) {
    setMinimumHeight(180);
}

void BarChartWidget::setData(const QVector<QPair<QString, qint64>> &data, const QString &unitLabel) {
    m_data = data;
    m_unitLabel = unitLabel;
    m_maxValue = 1;
    for (const auto &entry : m_data) {
        m_maxValue = qMax(m_maxValue, entry.second);
    }
    update();
}

void BarChartWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.fillRect(rect(), QColor(15, 15, 15));

    if (m_data.isEmpty()) {
        painter.setPen(QColor(200, 200, 200));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("Нет данных"));
        return;
    }

    const int left = 8;
    const int top = 8;
    const int bottom = 24;
    const int barGap = 6;
    const int count = m_data.size();
    const int chartW = width() - left * 2;
    const int chartH = height() - top - bottom;
    const int barW = qMax(12, (chartW - barGap * (count - 1)) / count);

    for (int i = 0; i < count; ++i) {
        const auto &entry = m_data.at(i);
        const float ratio = static_cast<float>(entry.second) / static_cast<float>(m_maxValue);
        const int barH = qMax(4, static_cast<int>(chartH * ratio));
        const int x = left + i * (barW + barGap);
        const int y = top + chartH - barH;

        QLinearGradient grad(x, y, x, y + barH);
        grad.setColorAt(0.0, QColor(0, 229, 255));
        grad.setColorAt(1.0, QColor(255, 0, 204));
        painter.fillRect(QRect(x, y, barW, barH), grad);

        painter.setPen(QColor(180, 180, 180));
        const QString label = entry.first.left(8);
        painter.drawText(QRect(x - 4, top + chartH + 2, barW + 8, bottom), Qt::AlignHCenter | Qt::AlignTop, label);
    }

    painter.setPen(QColor(0, 229, 255));
    painter.drawText(QRect(left, 0, chartW, top + 4), Qt::AlignLeft | Qt::AlignVCenter, m_unitLabel);
}

ProfileDialog::ProfileDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(QStringLiteral("Профиль — статистика"));
    resize(640, 520);
    setObjectName(QStringLiteral("profileDialog"));
}

void ProfileDialog::setStats(qint64 totalMs,
                             const QVector<QPair<QString, qint64>> &tracks,
                             const QVector<QPair<QString, qint64>> &playlists) {
    auto *layout = new QVBoxLayout(this);

    auto *totalLabel = new QLabel(
        QStringLiteral("Всего прослушано: %1").arg(formatListenTime(totalMs)), this);
    totalLabel->setObjectName(QStringLiteral("profileTotal"));
    layout->addWidget(totalLabel);

    auto *tracksHeader = new QLabel(QStringLiteral("Топ треков"), this);
    tracksHeader->setObjectName(QStringLiteral("sectionHeader"));
    auto *tracksChart = new BarChartWidget(this);
    tracksChart->setData(tracks, QStringLiteral("мин"));
    layout->addWidget(tracksHeader);
    layout->addWidget(tracksChart, 1);

    auto *playlistsHeader = new QLabel(QStringLiteral("Топ плейлистов"), this);
    playlistsHeader->setObjectName(QStringLiteral("sectionHeader"));
    auto *playlistsChart = new BarChartWidget(this);
    playlistsChart->setData(playlists, QStringLiteral("мин"));
    layout->addWidget(playlistsHeader);
    layout->addWidget(playlistsChart, 1);
}
