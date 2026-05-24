#ifndef COVERLABEL_H
#define COVERLABEL_H

#include <QLabel>

class CoverLabel : public QLabel {
    Q_OBJECT
public:

public:
    explicit CoverLabel(QWidget *parent = nullptr);

    void setCoverPixmap(const QPixmap &pixmap);
    void loadCoverFromPath(const QString &path);
    void setPlaceholderText(const QString &text);
    void clearCover();
    void setFavoriteBadge(bool enabled);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPixmap scaledPixmap() const;

    QPixmap m_sourcePixmap;
    QString m_placeholder;
    bool m_favoriteBadge = false;
};

#endif // COVERLABEL_H
