#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H

#include <QMediaPlayer>
#include <QObject>

#include <functional>

class QAudioOutput;
class QMediaPlayer;
class Track;

class AudioController : public QObject {
    Q_OBJECT

public:
    explicit AudioController(QObject *parent = nullptr);
    ~AudioController() override;

    void loadTrack(Track *track);
    void play();
    void pause();
    void stop();
    void setVolume(float volume);
    void setPosition(qint64 positionMs);
    void next();
    void previous();

    Track *currentTrack() const;
    qint64 position() const;
    qint64 duration() const;
    bool isPlaying() const;

    void setTrackResolver(std::function<Track *(bool forward)> resolver);

signals:
    void trackChanged(Track *track);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void playbackStateChanged(bool playing);
    void trackMetadataUpdated(Track *track);
    void finished();

public slots:
    void onNextRequested();
    void onPreviousRequested();

private slots:
    void onDurationChanged(qint64 duration);
    void onMetaDataChanged();
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);

private:
    void applyMetadataToTrack();
    void resolveAndLoad(bool forward);

    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    Track *m_currentTrack = nullptr;
    std::function<Track *(bool forward)> m_trackResolver;
};

#endif // AUDIOCONTROLLER_H
