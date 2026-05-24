#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QHash>
#include <QVector>

class QLabel;

class BarChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit BarChartWidget(QWidget *parent = nullptr);

    void setData(const QVector<QPair<QString, qint64>> &data, const QString &unitLabel);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<QPair<QString, qint64>> m_data;
    QString m_unitLabel;
    qint64 m_maxValue = 1;
};

class ProfileDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProfileDialog(QWidget *parent = nullptr);

    void setStats(qint64 totalMs,
                  const QVector<QPair<QString, qint64>> &tracks,
                  const QVector<QPair<QString, qint64>> &playlists);
};

#endif // PROFILEDIALOG_H
