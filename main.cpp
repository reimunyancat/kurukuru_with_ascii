#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <algorithm>
#include <csignal>

using namespace std;
namespace fs = std::filesystem;

bool running = true;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        running = false;
    }
}

void playAsciiFrames(const string& frameDir, int fps) {
    vector<string> frameFiles;
    for (const auto& entry : fs::directory_iterator(frameDir)) {
        if (entry.path().extension() == ".txt") {
            frameFiles.push_back(entry.path());
        }
    }
    sort(frameFiles.begin(), frameFiles.end());

    int frameTimeMs = 1000 / fps;
    while (running) {
        for (const auto& frameFile : frameFiles) {
            if (!running) break;
            auto start = chrono::steady_clock::now();
            ifstream file(frameFile);
            if (file.is_open()) {
                string line;
                cout << "\033[H";
                while (getline(file, line)) {
                    cout << line << "\n";
                }
                file.close();
            }
            auto elapsed = chrono::steady_clock::now() - start;
            auto sleepTime = chrono::milliseconds(frameTimeMs) -
                             chrono::duration_cast<chrono::milliseconds>(elapsed);
            if (sleepTime > chrono::milliseconds(0)) {
                this_thread::sleep_for(sleepTime);
            }
        }
    }
}

void playAudio(const string& audioFile) {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        cerr << "SDL_mixer Error: " << Mix_GetError() << endl;
        return;
    }
    Mix_Music* music = Mix_LoadMUS(audioFile.c_str());
    if (!music) {
        cerr << "Failed to load audio: " << Mix_GetError() << endl;
        return;
    }
    Mix_PlayMusic(music, -1);
    while (running && Mix_PlayingMusic()) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    Mix_HaltMusic();
    Mix_FreeMusic(music);
    Mix_CloseAudio();
}

int main() {
    const string basePath = "/home/reimunyancat/Archive/kururin/";
    const string frameDir = basePath + "frames";
    const string audioFile = basePath + "kuru_kuru.mp3";
    const int fps = 30;

    signal(SIGINT, signalHandler);

    try {
        thread frameThread(playAsciiFrames, frameDir, fps);
        playAudio(audioFile);
        frameThread.join();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    } catch (...) {
        cout << "\nkuru kuru" << endl;
    }

    return 0;
}

