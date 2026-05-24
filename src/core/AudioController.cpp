#include "AudioController.h"
#include "Track.h"

#include <QAudioOutput>
#include <QFileInfo>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QUrl>

AudioController::AudioController(QObject *parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this)) {
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.75f);

    connect(m_player, &QMediaPlayer::positionChanged, this, &AudioController::positionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &AudioController::onDurationChanged);
    connect(m_player, &QMediaPlayer::metaDataChanged, this, &AudioController::onMetaDataChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &AudioController::onMediaStatusChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &AudioController::onPlaybackStateChanged);
}

AudioController::~AudioController() = default;

void AudioController::loadTrack(Track *track) {
    if (!track || track->filePath().isEmpty()) {
        return;
    }
    m_currentTrack = track;
    m_player->setSource(QUrl::fromLocalFile(track->filePath()));
    emit trackChanged(track);
}

void AudioController::play() {
    m_player->play();
}

void AudioController::pause() {
    m_player->pause();
}

void AudioController::stop() {
    m_player->stop();
}

void AudioController::setVolume(float volume) {
    m_audioOutput->setVolume(qBound(0.0f, volume, 1.0f));
}

void AudioController::setPosition(qint64 positionMs) {
    m_player->setPosition(positionMs);
}

void AudioController::next() {
    onNextRequested();
}

void AudioController::previous() {
    onPreviousRequested();
}

Track *AudioController::currentTrack() const { return m_currentTrack; }

qint64 AudioController::position() const { return m_player->position(); }

qint64 AudioController::duration() const {
    return m_player->duration() > 0 ? m_player->duration() : (m_currentTrack ? m_currentTrack->duration() : 0);
}

bool AudioController::isPlaying() const {
    return m_player->playbackState() == QMediaPlayer::PlayingState;
}

void AudioController::setTrackResolver(std::function<Track *(bool forward)> resolver) {
    m_trackResolver = std::move(resolver);
}

void AudioController::onNextRequested() {
    resolveAndLoad(true);
}

void AudioController::onPreviousRequested() {
    resolveAndLoad(false);
}

void AudioController::onDurationChanged(qint64 duration) {
    if (m_currentTrack && duration > 0) {
        m_currentTrack->setDuration(duration);
        if (!m_currentTrack->isMetadataLocked()) {
            emit trackMetadataUpdated(m_currentTrack);
        }
    }
    emit durationChanged(duration);
}

void AudioController::onMetaDataChanged() {
    applyMetadataToTrack();
}

void AudioController::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {
        emit finished();
    }
}

void AudioController::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
    emit playbackStateChanged(state == QMediaPlayer::PlayingState);
}

void AudioController::applyMetadataToTrack() {
    if (!m_currentTrack || m_currentTrack->isMetadataLocked()) {
        return;
    }

    const auto title = m_player->metaData().value(QMediaMetaData::Title).toString();
    const auto artist = m_player->metaData().value(QMediaMetaData::ContributingArtist).toString();
    const auto albumArtist = m_player->metaData().value(QMediaMetaData::AlbumArtist).toString();

    if (!title.isEmpty()) {
        m_currentTrack->setTitle(title);
    }
    if (!artist.isEmpty()) {
        m_currentTrack->setArtist(artist);
    } else if (!albumArtist.isEmpty()) {
        m_currentTrack->setArtist(albumArtist);
    }

    if (m_player->duration() > 0) {
        m_currentTrack->setDuration(m_player->duration());
    }

    emit trackMetadataUpdated(m_currentTrack);
}

void AudioController::resolveAndLoad(bool forward) {
    if (!m_trackResolver) {
        return;
    }
    Track *nextTrack = m_trackResolver(forward);
    if (!nextTrack) {
        return;
    }
    loadTrack(nextTrack);
    play();
}
