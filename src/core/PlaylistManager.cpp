#include "PlaylistManager.h"
#include "Playlist.h"
#include "Track.h"
#include "TrackStore.h"

PlaylistManager::PlaylistManager(QObject *parent)
    : QObject(parent) {
    ensureDefaultPlaylist();
}

PlaylistManager::~PlaylistManager() {
    for (Playlist *playlist : m_playlists) {
        qDeleteAll(playlist->getTracks());
        delete playlist;
    }
}

void PlaylistManager::ensureDefaultPlaylist() {
    if (m_playlists.isEmpty()) {
        m_playlists.append(new Playlist(QStringLiteral("Library")));
        m_currentIndex = 0;
    }
}

QVector<Playlist *> PlaylistManager::playlists() const { return m_playlists; }

Playlist *PlaylistManager::currentPlaylist() const {
    if (m_currentIndex < 0 || m_currentIndex >= m_playlists.size()) {
        return nullptr;
    }
    return m_playlists.at(m_currentIndex);
}

int PlaylistManager::currentIndex() const { return m_currentIndex; }

Playlist *PlaylistManager::createPlaylist(const QString &name) {
    auto *playlist = new Playlist(name);
    m_playlists.append(playlist);
    emit playlistsChanged();
    return playlist;
}

void PlaylistManager::removePlaylist(Playlist *playlist) {
    const int idx = m_playlists.indexOf(playlist);
    if (idx < 0 || m_playlists.size() <= 1) {
        return;
    }
    qDeleteAll(playlist->getTracks());
    delete playlist;
    m_playlists.removeAt(idx);
    if (m_currentIndex >= m_playlists.size()) {
        m_currentIndex = m_playlists.size() - 1;
    }
    emit playlistsChanged();
    emit currentPlaylistChanged(currentPlaylist());
}

void PlaylistManager::renamePlaylist(Playlist *playlist, const QString &name) {
    if (!playlist || name.trimmed().isEmpty()) {
        return;
    }
    playlist->setName(name.trimmed());
    emit playlistsChanged();
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

Track *PlaylistManager::addTrackToCurrent(const QString &filePath) {
    Playlist *playlist = currentPlaylist();
    if (!playlist) {
        return nullptr;
    }
    auto *track = new Track(filePath);
    TrackStore::loadInto(track);
    playlist->addTrack(track);
    emit trackAdded(track);
    return track;
}

void PlaylistManager::removeTrack(Track *track) {
    for (Playlist *playlist : m_playlists) {
        playlist->removeTrack(track);
    }
    emit trackRemoved(track);
    delete track;
}
