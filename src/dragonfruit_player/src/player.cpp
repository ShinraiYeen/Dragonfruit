#include "player.hpp"

#include <algorithm>
#include <iostream>
#include <random>

Player::Player(const std::vector<std::filesystem::path>& song_files) : song_paths_(song_files) {}

Player::~Player() {}

void Player::Pause(bool pause) {
    paused_ = pause;
    engine_.Pause(pause);
}

void Player::Play(int idx) {
    paused_ = false;
    int clamped_idx = std::clamp(idx, 0, static_cast<int>(song_paths_.size()) - 1);
    cur_song_idx_ = clamped_idx;

    // Load in the new song
    std::shared_ptr<dragonfruit::Sound> tmp_song(new dragonfruit::Sound(song_paths_[cur_song_idx_]));
    engine_.PlayAsync(tmp_song);

    // Once the old sound has finished, we swap it out with the temp one. This ensures proper freeing of the sound.
    std::swap(tmp_song, cur_sound_);
}

void Player::PlayRelative(int delta) {
    paused_ = false;
    int total_songs = song_paths_.size();
    int wrapped_idx = ((cur_song_idx_ + delta) % total_songs + total_songs) % total_songs;
    Play(wrapped_idx);
}

double Player::GetCurrentSongTime() { return engine_.GetCurrentSongTime(); }

double Player::GetTotalSongTime() { return engine_.GetTotalSongTime(); }

void Player::Seek(double seconds) { engine_.Seek(seconds); }

void Player::Shuffle() {
    std::random_device rd;
    std::mt19937 generator(rd());

    std::shuffle(song_paths_.begin(), song_paths_.end(), generator);

    Play(0);
}

void Player::SetVolume(double volume) {
    double volume_clamped = std::clamp(volume, 0.0, 1.0);
    engine_.SetVolume(volume_clamped);
    cur_volume_ = volume_clamped;
}