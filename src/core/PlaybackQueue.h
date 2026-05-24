#ifndef PLAYBACKQUEUE_H
#define PLAYBACKQUEUE_H

#include <QObject>
#include <QVector>

class Track;

class PlaybackQueue : public QObject {
    Q_OBJECT

public:
    explicit PlaybackQueue(QObject *parent = nullptr);

    void addTrack(Track *track);
    void removeTrack(Track *track);
    void clear();
    QVector<Track *> tracks() const;
    bool isEmpty() const;
    bool contains(Track *track) const;

    Track *currentTrack() const;
    int currentIndex() const;
    void setCurrentIndex(int index);

    Track *advanceNext();
    Track *advancePrevious();
    void consumeTrack(Track *track);

signals:
    void queueChanged();
    void currentChanged(Track *track);

private:
    QVector<Track *> m_tracks;
    int m_currentIndex = -1;
};

#endif // PLAYBACKQUEUE_H
