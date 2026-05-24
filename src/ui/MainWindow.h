#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMainWindow>

class AudioController;
class CoverLabel;
class ListeningStats;
class NeonButton;
class NowPlayingPanel;
class ParticleBackground;
class PlaybackQueue;
class Playlist;
class PlaylistManager;
class SpectrumVisualizer;
class Track;
class TrackMetadataDialog;

class QLabel;
class QLineEdit;
class QPushButton;
class QListWidget;
class QSlider;
class QSplitter;
class QTextEdit;
class QTimer;
class QWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

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
    void onOpenMetadataEditor();
    void onToggleFavorite();
    void onToggleNowPlaying();
    void onProfile();
    void onDescriptionChanged();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onPlaybackStateChanged(bool playing);
    void onTrackMetadataUpdated(Track *track);
    void onAudioFinished();
    void onStatsTick();
    void saveLibrary();

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
    void updateHeartButton();
    void selectTrackInList(Track *track);
    void importAudioFiles(const QStringList &paths);
    void openMetadataDialog(Track *track);
    Track *selectedTrack() const;
    Track *activeTrack() const;
    Track *resolveTrack(bool forward);
    QString formatTime(qint64 ms) const;
    qint64 trackDurationForSlider(Track *track) const;
    void updatePlayPauseButton(bool playing);
    void setUiEnabled(bool hasTracks);
    void syncNowPlayingUi(Track *track);
    void syncOverlayGeometry();

    PlaylistManager *m_playlistManager = nullptr;
    PlaybackQueue *m_queue = nullptr;
    AudioController *m_audio = nullptr;
    ListeningStats *m_stats = nullptr;
    QTimer *m_statsTimer = nullptr;

    QWidget *m_centralContainer = nullptr;
    ParticleBackground *m_particles = nullptr;
    QWidget *m_contentWidget = nullptr;
    QSplitter *m_mainSplitter = nullptr;
    NowPlayingPanel *m_nowPlayingPanel = nullptr;
    bool m_nowPlayingVisible = false;

    QListWidget *m_playlistList = nullptr;
    QListWidget *m_trackList = nullptr;
    QListWidget *m_queueList = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    CoverLabel *m_cover = nullptr;
    CoverLabel *m_nowPlayingCover = nullptr;
    QLineEdit *m_titleEdit = nullptr;
    QLineEdit *m_artistEdit = nullptr;
    QPushButton *m_saveMetadataBtn = nullptr;
    QPushButton *m_heartBtn = nullptr;
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
