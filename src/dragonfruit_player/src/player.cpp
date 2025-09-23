#include "player.hpp"

#include <algorithm>
#include <dragonfruit_engine/core/io/file_data_source.hpp>
#include <random>

Player::Player(const std::vector<std::filesystem::path>& song_files) : m_engine(100000), m_song_paths(song_files) {}

Player::~Player() {}

void Player::Pause(bool pause) { m_engine.Pause(pause); }

void Player::Play(int idx) {
    int clamped_idx = std::clamp(idx, 0, static_cast<int>(m_song_paths.size()) - 1);
    m_cur_song_idx = clamped_idx;

    // Load in the new song
    auto data = std::make_unique<dragonfruit::FileDataSource>(m_song_paths[m_cur_song_idx]);
    m_engine.PlayAsync(std::move(data));
}

void Player::PlayRelative(int delta) {
    int total_songs = m_song_paths.size();
    int wrapped_idx = ((m_cur_song_idx + delta) % total_songs + total_songs) % total_songs;
    Play(wrapped_idx);
}

double Player::GetCurrentSongTime() { return m_engine.GetCurrentSongTime(); }

double Player::GetTotalSongTime() { return m_engine.GetTotalSongTime(); }

void Player::Seek(double seconds) { m_engine.Seek(seconds); }

void Player::Shuffle() {
    std::random_device rd;
    std::mt19937 generator(rd());

    std::shuffle(m_song_paths.begin(), m_song_paths.end(), generator);

    Play(0);
}

void Player::SetVolume(double volume) {
    double volume_clamped = std::clamp(volume, 0.0, 1.0);
    m_engine.SetVolume(volume_clamped);
    m_cur_volume = volume_clamped;
}
