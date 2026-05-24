#ifndef TRACKSTORE_H
#define TRACKSTORE_H

class Track;

class TrackStore {
public:
    static void loadInto(Track *track);
    static void save(const Track *track);
};

#endif // TRACKSTORE_H
