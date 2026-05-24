#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QHash>
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
    Playlist *favoritesPlaylist() const;
    int currentIndex() const;

    Playlist *createPlaylist(const QString &name);
    void removePlaylist(Playlist *playlist);
    void renamePlaylist(Playlist *playlist, const QString &name);
    void setCurrentPlaylist(Playlist *playlist);
    void setCurrentIndex(int index);

    Track *getOrCreateTrack(const QString &filePath);
    Track *addTrackToCurrent(const QString &filePath);
    Track *addTrackToPlaylist(Playlist *playlist, const QString &filePath);
    void removeTrack(Track *track);

    bool isFavorite(Track *track) const;
    void setFavorite(Track *track, bool favorite);
    void toggleFavorite(Track *track);
    Track *findTrackById(const QString &id) const;

    void clearAllForLoad();
    Track *createTrackFromStorage(const QString &filePath, const QString &id);
    Playlist *createPlaylistFromStorage(const QString &name, bool favorites);
    void finalizeAfterLoad(int currentIndex);

signals:
    void playlistsChanged();
    void currentPlaylistChanged(Playlist *playlist);
    void trackAdded(Track *track);
    void trackRemoved(Track *track);
    void favoritesChanged(Track *track, bool favorite);
    void libraryChanged();

private:
    void ensureDefaultPlaylists();
    QString normalizePath(const QString &path) const;

    QVector<Playlist *> m_playlists;
    Playlist *m_favorites = nullptr;
    int m_currentIndex = 0;
    QHash<QString, Track *> m_tracksByPath;
};

#endif // PLAYLISTMANAGER_H
