cmake_minimum_required(VERSION 3.16)
project(untitled VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Multimedia
    MultimediaWidgets
)

set(PROJECT_SOURCES
    src/main.cpp
    src/core/Track.cpp
    src/core/Playlist.cpp
    src/core/PlaylistManager.cpp
    src/core/PlaybackQueue.cpp
    src/core/AudioController.cpp
    src/core/LibraryStore.cpp
    src/core/ListeningStats.cpp
    src/ui/MainWindow.cpp
    src/ui/widgets/CoverLabel.cpp
    src/ui/widgets/SpectrumVisualizer.cpp
    src/ui/widgets/NeonButton.cpp
    src/ui/widgets/ParticleBackground.cpp
    src/ui/widgets/NowPlayingPanel.cpp
    src/ui/dialogs/TrackMetadataDialog.cpp
    src/ui/dialogs/ProfileDialog.cpp
    resources/resources.qrc
)

add_executable(untitled ${PROJECT_SOURCES})

target_include_directories(untitled PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(untitled PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)
