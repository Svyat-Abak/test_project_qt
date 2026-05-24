#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QString>
#include <QVector>

class Track;

class Playlist {
public:
    explicit Playlist(const QString &name = QStringLiteral("New Playlist"));

    QString name() const;
    void setName(const QString &name);

    void addTrack(Track *track);
    void removeTrack(Track *track);
    void clear();
    QVector<Track *> search(const QString &query) const;
    QVector<Track *> getTracks() const;
    int indexOf(Track *track) const;

private:
    QString m_name;
    QVector<Track *> m_tracks;
};

#endif // PLAYLIST_H
