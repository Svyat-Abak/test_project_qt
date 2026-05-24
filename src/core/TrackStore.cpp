#include "TrackStore.h"
#include "Track.h"

#include <QSettings>

namespace {
QString settingsGroup(const QString &filePath) {
    return QStringLiteral("tracks/") + filePath;
}
} // namespace

void TrackStore::loadInto(Track *track) {
    if (!track || track->filePath().isEmpty()) {
        return;
    }
    QSettings settings;
    settings.beginGroup(settingsGroup(track->filePath()));

    const QString title = settings.value(QStringLiteral("title")).toString();
    const QString artist = settings.value(QStringLiteral("artist")).toString();
    const QString description = settings.value(QStringLiteral("description")).toString();
    const QString coverPath = settings.value(QStringLiteral("coverPath")).toString();
    const bool locked = settings.value(QStringLiteral("metadataLocked"), false).toBool();

    if (!title.isEmpty()) {
        track->setTitle(title);
    }
    if (!artist.isEmpty()) {
        track->setArtist(artist);
    }
    if (!description.isEmpty()) {
        track->setDescriptionText(description);
    }
    if (!coverPath.isEmpty()) {
        track->setCoverPath(coverPath);
    }
    track->setMetadataLocked(locked);

    settings.endGroup();
}

void TrackStore::save(const Track *track) {
    if (!track || track->filePath().isEmpty()) {
        return;
    }
    QSettings settings;
    settings.beginGroup(settingsGroup(track->filePath()));
    settings.setValue(QStringLiteral("title"), track->title());
    settings.setValue(QStringLiteral("artist"), track->artist());
    settings.setValue(QStringLiteral("description"), track->descriptionText());
    settings.setValue(QStringLiteral("coverPath"), track->coverPath());
    settings.setValue(QStringLiteral("metadataLocked"), track->isMetadataLocked());
    settings.endGroup();
    settings.sync();
}
