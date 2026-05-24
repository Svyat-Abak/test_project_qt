#ifndef PARTICLEBACKGROUND_H
#define PARTICLEBACKGROUND_H

#include <QColor>
#include <QVector>
#include <QWidget>

class QTimer;

class ParticleBackground : public QWidget {
    Q_OBJECT

public:
    explicit ParticleBackground(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct Particle {
        QPointF pos;
        QPointF velocity;
        float radius = 8.f;
        QColor color;
    };

    void initParticles();
    void step();

    QVector<Particle> m_particles;
    QTimer *m_timer = nullptr;
};

#endif // PARTICLEBACKGROUND_H
