#include "Track.h"

#include <QFileInfo>
#include <QUuid>

Track::Track(const QString &filePath)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_filePath(filePath) {
    const QFileInfo info(filePath);
    m_title = info.completeBaseName();
    m_artist = QStringLiteral("Unknown Artist");
}

QString Track::id() const { return m_id; }
void Track::setId(const QString &id) { m_id = id; }

QString Track::filePath() const { return m_filePath; }
void Track::setFilePath(const QString &path) { m_filePath = path; }

QString Track::title() const { return m_title; }
void Track::setTitle(const QString &title) { m_title = title; }

QString Track::artist() const { return m_artist; }
void Track::setArtist(const QString &artist) { m_artist = artist; }

QString Track::album() const { return m_album; }
void Track::setAlbum(const QString &album) { m_album = album; }

QString Track::genre() const { return m_genre; }
void Track::setGenre(const QString &genre) { m_genre = genre; }

int Track::year() const { return m_year; }
void Track::setYear(int year) { m_year = year; }

QString Track::descriptionText() const { return m_descriptionText; }
void Track::setDescriptionText(const QString &text) { m_descriptionText = text; }

QString Track::coverPath() const { return m_coverPath; }
void Track::setCoverPath(const QString &path) { m_coverPath = path; }

qint64 Track::duration() const { return m_duration; }
void Track::setDuration(qint64 ms) { m_duration = ms; }

qint64 Track::durationOverride() const { return m_durationOverride; }
void Track::setDurationOverride(qint64 ms) { m_durationOverride = ms; }

qint64 Track::effectiveDuration() const {
    if (m_durationOverride > 0) {
        return m_durationOverride;
    }
    return m_duration;
}

QString Track::formatMs(qint64 ms) {
    if (ms <= 0) {
        return QStringLiteral("00:00");
    }
    const qint64 totalSeconds = ms / 1000;
    return QStringLiteral("%1:%2")
        .arg(totalSeconds / 60, 2, 10, QChar('0'))
        .arg(totalSeconds % 60, 2, 10, QChar('0'));
}

QString Track::formattedDuration() const {
    return formatMs(effectiveDuration());
}

bool Track::isMetadataLocked() const { return m_metadataLocked; }
void Track::setMetadataLocked(bool locked) { m_metadataLocked = locked; }

bool Track::fileExists() const {
    return !m_filePath.isEmpty() && QFileInfo(m_filePath).isFile();
}
