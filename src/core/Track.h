#ifndef TRACK_H
#define TRACK_H

#include <QString>
#include <QtGlobal>

class Track {
public:
    explicit Track(const QString &filePath = QString());

    QString filePath() const;
    void setFilePath(const QString &path);

    QString title() const;
    void setTitle(const QString &title);

    QString artist() const;
    void setArtist(const QString &artist);

    QString descriptionText() const;
    void setDescriptionText(const QString &text);

    QString coverPath() const;
    void setCoverPath(const QString &path);

    qint64 duration() const;
    void setDuration(qint64 ms);

    QString formattedDuration() const;

    bool isMetadataLocked() const;
    void setMetadataLocked(bool locked);

private:
    QString m_filePath;
    QString m_title;
    QString m_artist;
    QString m_descriptionText;
    QString m_coverPath;
    qint64 m_duration = 0;
    bool m_metadataLocked = false;
};

#endif // TRACK_H
