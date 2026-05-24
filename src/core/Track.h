#ifndef TRACK_H
#define TRACK_H

#include <QString>
#include <QtGlobal>

class Track {
public:
    explicit Track(const QString &filePath = QString());

    QString id() const;
    void setId(const QString &id);

    QString filePath() const;
    void setFilePath(const QString &path);

    QString title() const;
    void setTitle(const QString &title);

    QString artist() const;
    void setArtist(const QString &artist);

    QString album() const;
    void setAlbum(const QString &album);

    QString genre() const;
    void setGenre(const QString &genre);

    int year() const;
    void setYear(int year);

    QString descriptionText() const;
    void setDescriptionText(const QString &text);

    QString coverPath() const;
    void setCoverPath(const QString &path);

    qint64 duration() const;
    void setDuration(qint64 ms);

    qint64 durationOverride() const;
    void setDurationOverride(qint64 ms);

    qint64 effectiveDuration() const;

    QString formattedDuration() const;
    static QString formatMs(qint64 ms);

    bool isMetadataLocked() const;
    void setMetadataLocked(bool locked);

    bool fileExists() const;

private:
    QString m_id;
    QString m_filePath;
    QString m_title;
    QString m_artist;
    QString m_album;
    QString m_genre;
    int m_year = 0;
    QString m_descriptionText;
    QString m_coverPath;
    qint64 m_duration = 0;
    qint64 m_durationOverride = 0;
    bool m_metadataLocked = false;
};

#endif // TRACK_H
