#ifndef NEONBUTTON_H
#define NEONBUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>

class NeonButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(qreal glowOpacity READ glowOpacity WRITE setGlowOpacity)

public:
    explicit NeonButton(const QString &text = QString(), QWidget *parent = nullptr);
    explicit NeonButton(const QIcon &icon, QWidget *parent = nullptr);

    qreal glowOpacity() const;
    void setGlowOpacity(qreal value);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void animateGlow(qreal target);

    QPropertyAnimation *m_glowAnimation = nullptr;
    qreal m_glowOpacity = 0.35;
    bool m_pressed = false;
};

#endif // NEONBUTTON_H
