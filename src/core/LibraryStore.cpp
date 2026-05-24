#include "LibraryStore.h"
#include "ListeningStats.h"
#include "Playlist.h"
#include "PlaylistManager.h"
#include "Track.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace {

QJsonObject trackToJson(const Track *track) {
    QJsonObject obj;
    obj.insert(QStringLiteral("id"), track->id());
    obj.insert(QStringLiteral("filePath"), track->filePath());
    obj.insert(QStringLiteral("title"), track->title());
    obj.insert(QStringLiteral("artist"), track->artist());
    obj.insert(QStringLiteral("album"), track->album());
    obj.insert(QStringLiteral("genre"), track->genre());
    obj.insert(QStringLiteral("year"), track->year());
    obj.insert(QStringLiteral("description"), track->descriptionText());
    obj.insert(QStringLiteral("coverPath"), track->coverPath());
    obj.insert(QStringLiteral("duration"), static_cast<double>(track->duration()));
    obj.insert(QStringLiteral("durationOverride"), static_cast<double>(track->durationOverride()));
    obj.insert(QStringLiteral("metadataLocked"), track->isMetadataLocked());
    return obj;
}

void applyJsonToTrack(const QJsonObject &obj, Track *track) {
    if (obj.contains(QStringLiteral("id"))) {
        track->setId(obj.value(QStringLiteral("id")).toString());
    }
    if (obj.contains(QStringLiteral("filePath"))) {
        track->setFilePath(obj.value(QStringLiteral("filePath")).toString());
    }
    track->setTitle(obj.value(QStringLiteral("title")).toString(track->title()));
    track->setArtist(obj.value(QStringLiteral("artist")).toString(track->artist()));
    track->setAlbum(obj.value(QStringLiteral("album")).toString());
    track->setGenre(obj.value(QStringLiteral("genre")).toString());
    track->setYear(obj.value(QStringLiteral("year")).toInt());
    track->setDescriptionText(obj.value(QStringLiteral("description")).toString());
    track->setCoverPath(obj.value(QStringLiteral("coverPath")).toString());
    track->setDuration(static_cast<qint64>(obj.value(QStringLiteral("duration")).toDouble()));
    track->setDurationOverride(static_cast<qint64>(obj.value(QStringLiteral("durationOverride")).toDouble()));
    track->setMetadataLocked(obj.value(QStringLiteral("metadataLocked")).toBool(false));
}

QJsonObject statsToJson(const ListeningStats *stats) {
    QJsonObject root;
    root.insert(QStringLiteral("totalListenMs"), static_cast<double>(stats->totalListenMs()));

    QJsonObject byTrack;
    for (auto it = stats->trackListenMs().constBegin(); it != stats->trackListenMs().constEnd(); ++it) {
        byTrack.insert(it.key(), static_cast<double>(it.value()));
    }
    root.insert(QStringLiteral("byTrack"), byTrack);

    QJsonObject byPlaylist;
    for (auto it = stats->playlistListenMs().constBegin(); it != stats->playlistListenMs().constEnd(); ++it) {
        byPlaylist.insert(it.key(), static_cast<double>(it.value()));
    }
    root.insert(QStringLiteral("byPlaylist"), byPlaylist);
    return root;
}

void loadStatsFromJson(const QJsonObject &root, ListeningStats *stats) {
    stats->setTotalListenMs(static_cast<qint64>(root.value(QStringLiteral("totalListenMs")).toDouble()));

    QHash<QString, qint64> byTrack;
    const QJsonObject trackObj = root.value(QStringLiteral("byTrack")).toObject();
    for (auto it = trackObj.constBegin(); it != trackObj.constEnd(); ++it) {
        byTrack.insert(it.key(), static_cast<qint64>(it.value().toDouble()));
    }
    stats->setTrackListenMs(byTrack);

    QHash<QString, qint64> byPlaylist;
    const QJsonObject playlistObj = root.value(QStringLiteral("byPlaylist")).toObject();
    for (auto it = playlistObj.constBegin(); it != playlistObj.constEnd(); ++it) {
        byPlaylist.insert(it.key(), static_cast<qint64>(it.value().toDouble()));
    }
    stats->setPlaylistListenMs(byPlaylist);
}

} // namespace

QString LibraryStore::libraryFilePath() {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/library.json");
}

bool LibraryStore::load(PlaylistManager *manager, ListeningStats *stats) {
    if (!manager) {
        return false;
    }

    QFile file(libraryFilePath());
    if (!file.exists()) {
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        return false;
    }

    const QJsonObject root = doc.object();
    manager->clearAllForLoad();

    QHash<QString, Track *> tracksById;
    const QJsonArray tracksArray = root.value(QStringLiteral("tracks")).toArray();
    for (const QJsonValue &value : tracksArray) {
        const QJsonObject obj = value.toObject();
        const QString path = obj.value(QStringLiteral("filePath")).toString();
        Track *track = manager->createTrackFromStorage(path, obj.value(QStringLiteral("id")).toString());
        applyJsonToTrack(obj, track);
        tracksById.insert(track->id(), track);
    }

    const QJsonArray playlistsArray = root.value(QStringLiteral("playlists")).toArray();
    int currentIndex = root.value(QStringLiteral("currentPlaylistIndex")).toInt(0);
    for (const QJsonValue &value : playlistsArray) {
        const QJsonObject obj = value.toObject();
        const QString name = obj.value(QStringLiteral("name")).toString();
        const bool favorites = obj.value(QStringLiteral("isFavorites")).toBool(false);
        Playlist *playlist = manager->createPlaylistFromStorage(name, favorites);

        const QJsonArray ids = obj.value(QStringLiteral("trackIds")).toArray();
        for (const QJsonValue &idValue : ids) {
            Track *track = tracksById.value(idValue.toString());
            if (track) {
                playlist->addTrack(track);
            }
        }
    }

    manager->finalizeAfterLoad(currentIndex);

    if (stats && root.contains(QStringLiteral("stats"))) {
        loadStatsFromJson(root.value(QStringLiteral("stats")).toObject(), stats);
    }

    return true;
}

bool LibraryStore::save(PlaylistManager *manager, ListeningStats *stats) {
    if (!manager) {
        return false;
    }

    QJsonObject root;
    QJsonArray tracksArray;
    QHash<QString, bool> exported;

    for (Playlist *playlist : manager->playlists()) {
        for (Track *track : playlist->getTracks()) {
            if (exported.contains(track->id())) {
                continue;
            }
            exported.insert(track->id(), true);
            tracksArray.append(trackToJson(track));
        }
    }

    QJsonArray playlistsArray;
    int index = 0;
    int currentIndex = manager->currentIndex();
    for (Playlist *playlist : manager->playlists()) {
        QJsonObject obj;
        obj.insert(QStringLiteral("name"), playlist->name());
        obj.insert(QStringLiteral("isFavorites"), playlist->isFavorites());
        QJsonArray ids;
        for (Track *track : playlist->getTracks()) {
            ids.append(track->id());
        }
        obj.insert(QStringLiteral("trackIds"), ids);
        playlistsArray.append(obj);
        Q_UNUSED(index);
        ++index;
    }

    root.insert(QStringLiteral("tracks"), tracksArray);
    root.insert(QStringLiteral("playlists"), playlistsArray);
    root.insert(QStringLiteral("currentPlaylistIndex"), currentIndex);
    if (stats) {
        root.insert(QStringLiteral("stats"), statsToJson(stats));
    }

    QFile file(libraryFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}
