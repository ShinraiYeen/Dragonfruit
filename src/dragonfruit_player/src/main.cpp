#include <stdio.h>

#include "frontends/default_frontend.hpp"
#include "player.hpp"
#include "version.hpp"

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

void DisplayUsageMessage(char** argv) {
    printf("Usage:\n");
    printf("  %s [OPTIONS] <path> [<path> ...]\n", argv[0]);
}

void DisplayHelpMessage(char** argv) {
    DisplayUsageMessage(argv);
    printf("\n");
    printf("Arguments:\n");
    printf("  <path>:           One or more files/directories containing music to play.\n");
    printf("                    Playing a directory will collect all valid song files in\n");
    printf("                    that directory and add them to the song queue.\n\n");
    printf("Options:\n");
    printf("  -h, --help:       Displays this help message and exits.\n");
    printf("  -v, --version:    Displays the version number and exits.\n\n");
    printf("Usage Examples:\n");
    printf("  Playing a single song:\n    %s song.wav\n", argv[0]);
    printf("  Playing songs from a directory:\n    %s dir\n", argv[0]);
    printf("  Playing multiple songs/directories:\n    %s song.wav dir\n", argv[0]);
}

void DisplayVersion() {
    printf("Dragonfruit v%d.%d.%d\n", DRAGONFRUIT_VERSION_MAJOR, DRAGONFRUIT_VERSION_MINOR, DRAGONFRUIT_VERSION_PATCH);
}

int main(int argc, char** argv) {
    std::vector<std::filesystem::path> song_paths;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "-h" || arg == "--help") {
            DisplayHelpMessage(argv);
            return EXIT_SUCCESS;
        } else if (arg == "-v" || arg == "--version") {
            DisplayVersion();
            return EXIT_SUCCESS;
        } else if (arg.empty() || arg[0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", arg.c_str());
            DisplayUsageMessage(argv);
            return EXIT_FAILURE;
        } else {
            CollectFiles(argv[i], song_paths);
        }
    }

    if (song_paths.empty()) {
        fprintf(stderr, "No valid song files found, quitting.\n");
        DisplayUsageMessage(argv);
        return EXIT_FAILURE;
    }

    Player player(song_paths);
    std::unique_ptr<Frontend> frontend(new DefaultFrontend(player));
    frontend->Start();

    return 0;
}