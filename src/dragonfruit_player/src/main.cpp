#include "frontends/default_frontend.hpp"
#include "player.hpp"

bool IsWavFile(const std::filesystem::path& path) { return path.extension() == ".wav"; }

void CollectFiles(const std::filesystem::path& path, std::vector<std::filesystem::path>& files) {
    if (std::filesystem::is_regular_file(path) && IsWavFile(path)) {
        files.push_back(path);
    } else if (std::filesystem::is_directory(path)) {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file() && IsWavFile(entry.path())) files.push_back(entry.path());
        }
    }
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        fprintf(stderr, "Usage: %s <song/directory> [song/directory, ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    std::vector<std::filesystem::path> song_paths;

    for (int i = 1; i < argc; i++) { CollectFiles(argv[i], song_paths); }

    if (song_paths.empty()) {
        fprintf(stderr, "No song files provided, quitting.\n");
        exit(EXIT_FAILURE);
    }

    Player player(song_paths);
    Frontend* frontend = new DefaultFrontend(player);

    frontend->Start();

    delete frontend;
    return 0;
}