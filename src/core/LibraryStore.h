#ifndef LIBRARYSTORE_H
#define LIBRARYSTORE_H

#include <QString>

class ListeningStats;
class PlaylistManager;

class LibraryStore {
public:
    static QString libraryFilePath();
    static bool load(PlaylistManager *manager, ListeningStats *stats);
    static bool save(PlaylistManager *manager, ListeningStats *stats);
};

#endif // LIBRARYSTORE_H
