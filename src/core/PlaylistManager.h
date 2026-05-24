#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QVector>

class Playlist;
class Track;

class PlaylistManager : public QObject {
    Q_OBJECT

public:
    explicit PlaylistManager(QObject *parent = nullptr);
    ~PlaylistManager() override;

    QVector<Playlist *> playlists() const;
    Playlist *currentPlaylist() const;
    int currentIndex() const;

    Playlist *createPlaylist(const QString &name);
    void removePlaylist(Playlist *playlist);
    void renamePlaylist(Playlist *playlist, const QString &name);
    void setCurrentPlaylist(Playlist *playlist);
    void setCurrentIndex(int index);

    Track *addTrackToCurrent(const QString &filePath);
    void removeTrack(Track *track);

signals:
    void playlistsChanged();
    void currentPlaylistChanged(Playlist *playlist);
    void trackAdded(Track *track);
    void trackRemoved(Track *track);

private:
    void ensureDefaultPlaylist();

    QVector<Playlist *> m_playlists;
    int m_currentIndex = 0;
};

#endif // PLAYLISTMANAGER_H
