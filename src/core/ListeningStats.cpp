#include "ListeningStats.h"
#include "Playlist.h"
#include "Track.h"

ListeningStats::ListeningStats(QObject *parent)
    : QObject(parent) {}

void ListeningStats::startSession(Track *track, Playlist *playlist) {
    m_activeTrack = track;
    m_activePlaylist = playlist;
    m_running = track != nullptr;
}

void ListeningStats::stopSession() {
    m_running = false;
    m_activeTrack = nullptr;
    m_activePlaylist = nullptr;
}

void ListeningStats::tick() {
    if (m_running) {
        addElapsed(1000);
    }
}

qint64 ListeningStats::totalListenMs() const { return m_totalListenMs; }

QHash<QString, qint64> ListeningStats::trackListenMs() const { return m_trackListenMs; }

QHash<QString, qint64> ListeningStats::playlistListenMs() const { return m_playlistListenMs; }

void ListeningStats::setTotalListenMs(qint64 ms) {
    m_totalListenMs = ms;
}

void ListeningStats::setTrackListenMs(const QHash<QString, qint64> &data) {
    m_trackListenMs = data;
}

void ListeningStats::setPlaylistListenMs(const QHash<QString, qint64> &data) {
    m_playlistListenMs = data;
}

void ListeningStats::addElapsed(qint64 deltaMs) {
    m_totalListenMs += deltaMs;
    if (m_activeTrack) {
        m_trackListenMs[m_activeTrack->id()] += deltaMs;
    }
    if (m_activePlaylist) {
        m_playlistListenMs[m_activePlaylist->name()] += deltaMs;
    }
    emit statsChanged();
}
