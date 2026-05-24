#ifndef TRACKMETADATADIALOG_H
#define TRACKMETADATADIALOG_H

#include <QDialog>

class Track;

class QCheckBox;
class QLineEdit;
class QPushButton;
class QTextEdit;

class TrackMetadataDialog : public QDialog {
    Q_OBJECT

public:
    explicit TrackMetadataDialog(QWidget *parent = nullptr);

    void setTrack(Track *track);
    Track *track() const;

signals:
    void saved(Track *track);

private slots:
    void onBrowseCover();
    void onSave();
    void onCancel();

private:
    void applyToTrack();

    Track *m_track = nullptr;

    QLineEdit *m_title = nullptr;
    QLineEdit *m_artist = nullptr;
    QLineEdit *m_album = nullptr;
    QLineEdit *m_genre = nullptr;
    QLineEdit *m_year = nullptr;
    QLineEdit *m_duration = nullptr;
    QLineEdit *m_coverPath = nullptr;
    QTextEdit *m_description = nullptr;
    QCheckBox *m_useCustomDuration = nullptr;
};

#endif // TRACKMETADATADIALOG_H
