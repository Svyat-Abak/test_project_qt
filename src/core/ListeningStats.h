#ifndef LISTENINGSTATS_H
#define LISTENINGSTATS_H

#include <QHash>
#include <QObject>
#include <QString>

class Track;
class Playlist;

class ListeningStats : public QObject {
    Q_OBJECT

public:
    explicit ListeningStats(QObject *parent = nullptr);

    void startSession(Track *track, Playlist *playlist);
    void stopSession();
    void tick();

    qint64 totalListenMs() const;
    QHash<QString, qint64> trackListenMs() const;
    QHash<QString, qint64> playlistListenMs() const;

    void setTotalListenMs(qint64 ms);
    void setTrackListenMs(const QHash<QString, qint64> &data);
    void setPlaylistListenMs(const QHash<QString, qint64> &data);

signals:
    void statsChanged();

private:
    void addElapsed(qint64 deltaMs);

    qint64 m_totalListenMs = 0;
    QHash<QString, qint64> m_trackListenMs;
    QHash<QString, qint64> m_playlistListenMs;

    Track *m_activeTrack = nullptr;
    Playlist *m_activePlaylist = nullptr;
    bool m_running = false;
};

#endif // LISTENINGSTATS_H
