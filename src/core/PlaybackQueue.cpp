#include "PlaybackQueue.h"
#include "Track.h"

PlaybackQueue::PlaybackQueue(QObject *parent)
    : QObject(parent) {}

void PlaybackQueue::addTrack(Track *track) {
    if (!track || m_tracks.contains(track)) {
        return;
    }
    m_tracks.append(track);
    if (m_currentIndex < 0) {
        m_currentIndex = 0;
        emit currentChanged(currentTrack());
    }
    emit queueChanged();
}

void PlaybackQueue::removeTrack(Track *track) {
    const int idx = m_tracks.indexOf(track);
    if (idx < 0) {
        return;
    }
    m_tracks.removeAt(idx);
    if (m_tracks.isEmpty()) {
        m_currentIndex = -1;
    } else if (idx < m_currentIndex) {
        m_currentIndex--;
    } else if (idx == m_currentIndex && m_currentIndex >= m_tracks.size()) {
        m_currentIndex = m_tracks.size() - 1;
    }
    emit queueChanged();
    emit currentChanged(currentTrack());
}

void PlaybackQueue::consumeTrack(Track *track) {
    const int idx = m_tracks.indexOf(track);
    if (idx < 0) {
        return;
    }
    m_tracks.removeAt(idx);
    if (m_tracks.isEmpty()) {
        m_currentIndex = -1;
    } else if (idx < m_currentIndex) {
        m_currentIndex--;
    } else if (idx == m_currentIndex && m_currentIndex >= m_tracks.size()) {
        m_currentIndex = m_tracks.size() - 1;
    }
    emit queueChanged();
    emit currentChanged(currentTrack());
}

void PlaybackQueue::clear() {
    m_tracks.clear();
    m_currentIndex = -1;
    emit queueChanged();
    emit currentChanged(nullptr);
}

QVector<Track *> PlaybackQueue::tracks() const { return m_tracks; }
bool PlaybackQueue::isEmpty() const { return m_tracks.isEmpty(); }

bool PlaybackQueue::contains(Track *track) const {
    return track && m_tracks.contains(track);
}

Track *PlaybackQueue::currentTrack() const {
    if (m_currentIndex < 0 || m_currentIndex >= m_tracks.size()) {
        return nullptr;
    }
    return m_tracks.at(m_currentIndex);
}

int PlaybackQueue::currentIndex() const { return m_currentIndex; }

void PlaybackQueue::setCurrentIndex(int index) {
    if (m_tracks.isEmpty() || index < 0 || index >= m_tracks.size()) {
        return;
    }
    m_currentIndex = index;
    emit currentChanged(currentTrack());
}

Track *PlaybackQueue::advanceNext() {
    if (m_tracks.isEmpty()) {
        return nullptr;
    }
    if (m_currentIndex < m_tracks.size() - 1) {
        m_currentIndex++;
        emit currentChanged(currentTrack());
        return currentTrack();
    }
    return nullptr;
}

Track *PlaybackQueue::advancePrevious() {
    if (m_tracks.isEmpty()) {
        return nullptr;
    }
    if (m_currentIndex > 0) {
        m_currentIndex--;
        emit currentChanged(currentTrack());
        return currentTrack();
    }
    return nullptr;
}
