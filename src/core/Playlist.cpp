#include "Playlist.h"
#include "Track.h"

Playlist::Playlist(const QString &name)
    : m_name(name) {}

QString Playlist::name() const { return m_name; }
void Playlist::setName(const QString &name) { m_name = name; }

bool Playlist::isFavorites() const { return m_isFavorites; }
void Playlist::setFavorites(bool favorites) { m_isFavorites = favorites; }

void Playlist::addTrack(Track *track) {
    if (!track || m_tracks.contains(track)) {
        return;
    }
    m_tracks.append(track);
}

void Playlist::removeTrack(Track *track) {
    m_tracks.removeAll(track);
}

void Playlist::clear() {
    m_tracks.clear();
}

QVector<Track *> Playlist::search(const QString &query) const {
    if (query.trimmed().isEmpty()) {
        return m_tracks;
    }
    const QString needle = query.trimmed().toLower();
    QVector<Track *> result;
    for (Track *track : m_tracks) {
        if (track->title().toLower().contains(needle)
            || track->artist().toLower().contains(needle)
            || track->album().toLower().contains(needle)) {
            result.append(track);
        }
    }
    return result;
}

QVector<Track *> Playlist::getTracks() const { return m_tracks; }

int Playlist::indexOf(Track *track) const { return m_tracks.indexOf(track); }
