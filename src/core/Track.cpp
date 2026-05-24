#include "Track.h"

#include <QFileInfo>

Track::Track(const QString &filePath)
    : m_filePath(filePath) {
    const QFileInfo info(filePath);
    m_title = info.completeBaseName();
    m_artist = QStringLiteral("Unknown Artist");
}

QString Track::filePath() const { return m_filePath; }
void Track::setFilePath(const QString &path) { m_filePath = path; }

QString Track::title() const { return m_title; }
void Track::setTitle(const QString &title) { m_title = title; }

QString Track::artist() const { return m_artist; }
void Track::setArtist(const QString &artist) { m_artist = artist; }

QString Track::descriptionText() const { return m_descriptionText; }
void Track::setDescriptionText(const QString &text) { m_descriptionText = text; }

QString Track::coverPath() const { return m_coverPath; }
void Track::setCoverPath(const QString &path) { m_coverPath = path; }

qint64 Track::duration() const { return m_duration; }
void Track::setDuration(qint64 ms) { m_duration = ms; }

bool Track::isMetadataLocked() const { return m_metadataLocked; }
void Track::setMetadataLocked(bool locked) { m_metadataLocked = locked; }

QString Track::formattedDuration() const {
    if (m_duration <= 0) {
        return QStringLiteral("00:00");
    }
    const qint64 totalSeconds = m_duration / 1000;
    const qint64 minutes = totalSeconds / 60;
    const qint64 seconds = totalSeconds % 60;
    return QStringLiteral("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}
