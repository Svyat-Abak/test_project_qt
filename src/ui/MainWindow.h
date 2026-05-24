#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class AudioController;
class CoverLabel;
class NeonButton;
class PlaybackQueue;
class Playlist;
class PlaylistManager;
class SpectrumVisualizer;
class Track;

class QLabel;
class QLineEdit;
class QPushButton;
class QListWidget;
class QSlider;
class QTextEdit;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAddTracks();
    void onAddPlaylist();
    void onRemovePlaylist();
    void onRenamePlaylist();
    void onPlaylistSelectionChanged();
    void onTrackSelectionChanged();
    void onTrackDoubleClicked();
    void onSearchTextChanged(const QString &text);
    void onAddToQueue();
    void onRemoveFromQueue();
    void onClearQueue();
    void onQueueSelectionChanged();
    void onPlayPause();
    void onStop();
    void onNext();
    void onPrevious();
    void onSeek(int position);
    void onVolumeChanged(int value);
    void onSetCover();
    void onFixMetadata();
    void onDescriptionChanged();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onPlaybackStateChanged(bool playing);
    void onTrackMetadataUpdated(Track *track);
    void onAudioFinished();

private:
    void setupUi();
    void loadStyleSheet();
    void connectSignals();
    void refreshPlaylistList();
    void refreshTrackList(const QString &filter = QString());
    void refreshQueueList();
    void displayTrack(Track *track);
    void applyEditsFromUi(Track *track);
    void updateMetadataEditState(Track *track);
    void selectTrackInList(Track *track);
    Track *selectedTrack() const;
    Track *activeTrack() const;
    Track *resolveTrack(bool forward);
    QString formatTime(qint64 ms) const;
    void updatePlayPauseButton(bool playing);
    void setUiEnabled(bool hasTracks);

    PlaylistManager *m_playlistManager = nullptr;
    PlaybackQueue *m_queue = nullptr;
    AudioController *m_audio = nullptr;

    QListWidget *m_playlistList = nullptr;
    QListWidget *m_trackList = nullptr;
    QListWidget *m_queueList = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    CoverLabel *m_cover = nullptr;
    QLineEdit *m_titleEdit = nullptr;
    QLineEdit *m_artistEdit = nullptr;
    QPushButton *m_saveMetadataBtn = nullptr;
    QLabel *m_placeholderLabel = nullptr;
    QTextEdit *m_descriptionEdit = nullptr;
    SpectrumVisualizer *m_visualizer = nullptr;
    QLabel *m_currentTimeLabel = nullptr;
    QLabel *m_totalTimeLabel = nullptr;
    QSlider *m_progressSlider = nullptr;
    QSlider *m_volumeSlider = nullptr;
    NeonButton *m_playPauseButton = nullptr;
    Track *m_editingTrack = nullptr;
    bool m_seeking = false;
    bool m_updatingMetadata = false;
};

#endif // MAINWINDOW_H
