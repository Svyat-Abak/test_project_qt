#ifndef NOWPLAYINGPANEL_H
#define NOWPLAYINGPANEL_H

#include <QWidget>

class CoverLabel;
class Track;

class QLabel;
class QPushButton;
class QTextEdit;

class NowPlayingPanel : public QWidget {
    Q_OBJECT

public:
    explicit NowPlayingPanel(QWidget *parent = nullptr);

    void setTrack(Track *track);
    void clear();

signals:
    void editMetadataRequested(Track *track);
    void closeRequested();

private:
    CoverLabel *m_cover = nullptr;
    QLabel *m_title = nullptr;
    QLabel *m_artist = nullptr;
    QLabel *m_album = nullptr;
    QLabel *m_genre = nullptr;
    QLabel *m_year = nullptr;
    QLabel *m_duration = nullptr;
    QLabel *m_path = nullptr;
    QTextEdit *m_description = nullptr;
    QPushButton *m_editBtn = nullptr;
    Track *m_track = nullptr;
};

#endif // NOWPLAYINGPANEL_H
