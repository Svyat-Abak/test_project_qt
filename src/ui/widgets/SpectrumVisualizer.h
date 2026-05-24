#ifndef SPECTRUMVISUALIZER_H
#define SPECTRUMVISUALIZER_H

#include <QWidget>

class QHBoxLayout;
class QProgressBar;
class QTimer;

class SpectrumVisualizer : public QWidget {
    Q_OBJECT

public:
    explicit SpectrumVisualizer(int barCount = 32, QWidget *parent = nullptr);

    void setActive(bool active);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void tick();

private:
    void buildBars(int count);

    QHBoxLayout *m_layout = nullptr;
    QVector<QProgressBar *> m_bars;
    QTimer *m_timer = nullptr;
    bool m_active = false;
};

#endif // SPECTRUMVISUALIZER_H
