#include "PlaylistManager.h"
#include "Playlist.h"
#include "Track.h"

#include <QFileInfo>

PlaylistManager::PlaylistManager(QObject *parent)
    : QObject(parent) {
    ensureDefaultPlaylists();
}

PlaylistManager::~PlaylistManager() {
    for (Playlist *playlist : m_playlists) {
        delete playlist;
    }
    qDeleteAll(m_tracksByPath);
}

void PlaylistManager::ensureDefaultPlaylists() {
    if (m_playlists.isEmpty()) {
        m_playlists.append(new Playlist(QStringLiteral("Library")));
        auto *favorites = new Playlist(QStringLiteral("Любимое"));
        favorites->setFavorites(true);
        m_favorites = favorites;
        m_playlists.append(favorites);
        m_currentIndex = 0;
    }
}

QString PlaylistManager::normalizePath(const QString &path) const {
    return QFileInfo(path).absoluteFilePath();
}

QVector<Playlist *> PlaylistManager::playlists() const { return m_playlists; }

Playlist *PlaylistManager::currentPlaylist() const {
    if (m_currentIndex < 0 || m_currentIndex >= m_playlists.size()) {
        return nullptr;
    }
    return m_playlists.at(m_currentIndex);
}

Playlist *PlaylistManager::favoritesPlaylist() const { return m_favorites; }

int PlaylistManager::currentIndex() const { return m_currentIndex; }

Playlist *PlaylistManager::createPlaylist(const QString &name) {
    auto *playlist = new Playlist(name);
    m_playlists.append(playlist);
    emit playlistsChanged();
    emit libraryChanged();
    return playlist;
}

void PlaylistManager::removePlaylist(Playlist *playlist) {
    if (!playlist || playlist->isFavorites() || m_playlists.size() <= 2) {
        return;
    }
    delete playlist;
    m_playlists.removeAll(playlist);
    if (m_currentIndex >= m_playlists.size()) {
        m_currentIndex = m_playlists.size() - 1;
    }
    emit playlistsChanged();
    emit currentPlaylistChanged(currentPlaylist());
    emit libraryChanged();
}

void PlaylistManager::renamePlaylist(Playlist *playlist, const QString &name) {
    if (!playlist || playlist->isFavorites() || name.trimmed().isEmpty()) {
        return;
    }
    playlist->setName(name.trimmed());
    emit playlistsChanged();
    emit libraryChanged();
}

void PlaylistManager::setCurrentPlaylist(Playlist *playlist) {
    const int idx = m_playlists.indexOf(playlist);
    if (idx < 0) {
        return;
    }
    m_currentIndex = idx;
    emit currentPlaylistChanged(currentPlaylist());
}

void PlaylistManager::setCurrentIndex(int index) {
    if (index < 0 || index >= m_playlists.size()) {
        return;
    }
    m_currentIndex = index;
    emit currentPlaylistChanged(currentPlaylist());
}

Track *PlaylistManager::getOrCreateTrack(const QString &filePath) {
    const QString key = normalizePath(filePath);
    if (key.isEmpty()) {
        return nullptr;
    }
    if (m_tracksByPath.contains(key)) {
        return m_tracksByPath.value(key);
    }
    auto *track = new Track(key);
    m_tracksByPath.insert(key, track);
    return track;
}

Track *PlaylistManager::addTrackToPlaylist(Playlist *playlist, const QString &filePath) {
    if (!playlist) {
        return nullptr;
    }
    Track *track = getOrCreateTrack(filePath);
    if (!track) {
        return nullptr;
    }
    if (!playlist->getTracks().contains(track)) {
        playlist->addTrack(track);
        emit trackAdded(track);
        emit libraryChanged();
    }
    return track;
}

Track *PlaylistManager::addTrackToCurrent(const QString &filePath) {
    return addTrackToPlaylist(currentPlaylist(), filePath);
}

void PlaylistManager::removeTrack(Track *track) {
    if (!track) {
        return;
    }
    for (Playlist *playlist : m_playlists) {
        playlist->removeTrack(track);
    }
    m_tracksByPath.remove(normalizePath(track->filePath()));
    emit trackRemoved(track);
    emit libraryChanged();
    delete track;
}

bool PlaylistManager::isFavorite(Track *track) const {
    return track && m_favorites && m_favorites->getTracks().contains(track);
}

void PlaylistManager::setFavorite(Track *track, bool favorite) {
    if (!track || !m_favorites) {
        return;
    }
    const bool was = isFavorite(track);
    if (favorite && !was) {
        m_favorites->addTrack(track);
        emit favoritesChanged(track, true);
        emit libraryChanged();
    } else if (!favorite && was) {
        m_favorites->removeTrack(track);
        emit favoritesChanged(track, false);
        emit libraryChanged();
    }
}

void PlaylistManager::toggleFavorite(Track *track) {
    setFavorite(track, !isFavorite(track));
}

Track *PlaylistManager::findTrackById(const QString &id) const {
    for (Track *track : m_tracksByPath) {
        if (track->id() == id) {
            return track;
        }
    }
    return nullptr;
}

void PlaylistManager::clearAllForLoad() {
    for (Playlist *playlist : m_playlists) {
        delete playlist;
    }
    m_playlists.clear();
    qDeleteAll(m_tracksByPath);
    m_tracksByPath.clear();
    m_favorites = nullptr;
    m_currentIndex = 0;
}

Track *PlaylistManager::createTrackFromStorage(const QString &filePath, const QString &id) {
    const QString key = normalizePath(filePath);
    auto *track = new Track(key);
    if (!id.isEmpty()) {
        track->setId(id);
    }
    m_tracksByPath.insert(key, track);
    return track;
}

Playlist *PlaylistManager::createPlaylistFromStorage(const QString &name, bool favorites) {
    auto *playlist = new Playlist(name);
    playlist->setFavorites(favorites);
    if (favorites) {
        m_favorites = playlist;
    }
    m_playlists.append(playlist);
    return playlist;
}

void PlaylistManager::finalizeAfterLoad(int currentIndex) {
    if (!m_favorites) {
        for (Playlist *playlist : m_playlists) {
            if (playlist->isFavorites()) {
                m_favorites = playlist;
                break;
            }
        }
        if (!m_favorites) {
            auto *favorites = new Playlist(QStringLiteral("Любимое"));
            favorites->setFavorites(true);
            m_favorites = favorites;
            m_playlists.append(favorites);
        }
    }
    m_currentIndex = qBound(0, currentIndex, m_playlists.size() - 1);
    emit playlistsChanged();
    emit currentPlaylistChanged(currentPlaylist());
}
