#pragma once

#include <dragonfruit_engine/audio_engine.hpp>
#include <filesystem>

/**
 * @brief Defines the main interface for interacting with the underlying dragonfruit audio engine. Frontends should use
 * this to play music and keep track of its current state.
 *
 */
class Player {
   public:
    /**
     * @brief Construct a new Player object with a predefined queue of songs. This will also initialize the underlying
     * audio engine.
     *
     * @param song_filenames A list of filepaths to valid song files to initialize the internal song queue.
     */
    Player(const std::vector<std::filesystem::path>& song_filenames);
    ~Player();

    /**
     * @brief Pauses the current song.
     *
     * @param pause If true, will pause the song. If false, will resume the song.
     */
    void Pause(bool pause);

    /**
     * @brief Check whether the current song is paused or not.
     *
     * @return true if the song is paused.
     * @return false otherwise.
     */
    inline bool IsPaused() { return m_paused; };

    /**
     * @brief Start playing the song at a given index into the song queue. In the case of an overflow (the index being
     * too large), the last song in the queue will be selected. This is a non-blocking call.
     *
     * @param idx The index in the queue of the song to play.
     */
    void Play(int idx);

    /**
     * @brief Start playing a song from the relative position in the song queue. For example, if the current queue index
     * is 2 and delta is 1, song 3 will be played. This will safely wrap around the queue in case of an
     * underflow/overflow.
     *
     * @param delta The index to move relative to the currently playing song's index in the queue.
     */
    void PlayRelative(int delta);

    /**
     * @brief Seek by a given delta in seconds relative to the current song's current position. This will safely clamp
     * to either the beginning of the song (in case of an underflow) or the end of the song (in case of an overflow).
     *
     * @param seconds A delta in seconds to seek by.
     */
    void Seek(double seconds);

    /**
     * @brief Get the total song time in seconds for the currently playing song.
     *
     * @return The total song time in seconds of the currently playing song.
     */
    double GetTotalSongTime();

    /**
     * @brief Get the number of seconds that have elapsed in the current song.
     *
     * @return The number of seconds that have elapsed in the current song.
     */
    double GetCurrentSongTime();

    /**
     * @brief Check if the currently playing song has finished playing.
     *
     * @return true if the currently playing song has finished playing.
     * @return false if the currently playing song has not finished playing.
     */
    inline bool IsFinished() { return m_engine.IsFinished(); }

    /**
     * @brief Get a list of the songs. This queue contains the filepaths of each song which is in this player's queue.
     *
     * @return A list filepaths for queued songs.
     */
    inline std::vector<std::filesystem::path>& GetSongQueue() { return m_song_paths; }

    /**
     * @brief Get the index of the currently playing song in the queue.
     *
     * @return The index of the currently playing song in the queue.
     */
    inline int GetCurrentSongIdx() { return m_cur_song_idx; }

    /**
     * @brief Get the currently playing song. This pointer will be empty if no song has been played yet.
     *
     * @return A shared pointer to the currently playing song.
     */
    inline std::shared_ptr<dragonfruit::Sound> GetCurrentSong() { return m_cur_sound; }

    /**
     * @brief Shuffles the queue and restarts playback at the first song.
     *
     */
    void Shuffle();

    /**
     * @brief Set the volume of the player.
     *
     * @param volume The volume from 0.0 to 1.0.
     */
    void SetVolume(double volume);

   private:
    dragonfruit::AudioEngine m_engine;
    std::vector<std::filesystem::path> m_song_paths;

    int m_cur_song_idx = 0;
    std::shared_ptr<dragonfruit::Sound> m_cur_sound;

    bool m_paused = false;
    double m_cur_volume = 1.0;
};