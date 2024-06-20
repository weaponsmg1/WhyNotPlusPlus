#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <Windows.h>

using namespace std;
using namespace chrono;

class Game {
public:
    string name;
    string path;
    bool isRunning;
    steady_clock::time_point startTime;
    duration<int> totalTime;

    Game(string n, string p) : name(n), path(p), isRunning(false), totalTime(0) {}

    void launch() {
        cout << "Launching game: " << name << endl;
        wstring wpath(path.begin(), path.end());
        ShellExecute(NULL, L"runas", wpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        isRunning = true;
        startTime = steady_clock::now();
    }

    void stop() {
        if (isRunning) {
            auto endTime = steady_clock::now();
            totalTime += duration_cast<seconds>(endTime - startTime);
            isRunning = false;
        }
    }

    string formatTime(duration<int> time) const {
        int hours = time.count() / 3600;
        int minutes = (time.count() % 3600) / 60;
        int seconds = time.count() % 60;
        return to_string(hours) + "h " + to_string(minutes) + "m " + to_string(seconds) + "s";
    }

    string getTotalTime() const {
        return formatTime(totalTime);
    }
};

class UserProfile {
public:
    string username;
    vector<Game> library;

    UserProfile(string name) : username(name) {}

    void addGame(const string& name, const string& path) {
        Game newGame(name, path);
        library.push_back(newGame);
    }

    void removeGame(int index) {
        library.erase(library.begin() + index);
    }

    void clearLibrary() {
        library.clear();
    }
};

vector<UserProfile> profiles;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printHeader() {
    cout << "WhyNot++\n\n";
}

void saveProfilesToFile() {
    ofstream file("whynot++.cfg");
    if (!file.is_open()) {
        cerr << "Unable to open file for saving.\n";
        return;
    }

    for (const auto& profile : profiles) {
        file << "[Profile: " << profile.username << "]\n";
        for (const auto& game : profile.library) {
            file << game.name << "=" << game.path << "\n";
        }
        file << "\n";
    }

    file.close();
    cout << "Profiles saved to whynot++.cfg.\n";
}

void loadProfilesFromFile() {
    ifstream file("whynot++.cfg");
    if (!file.is_open()) {
        cerr << "Unable to open file for loading.\n";
        return;
    }

    profiles.clear(); 

    string line;
    string currentProfile;
    while (getline(file, line)) {
        if (line.empty()) continue;

        if (line.find("[Profile: ") != string::npos) {
            currentProfile = line.substr(10, line.size() - 11); 
            UserProfile newUser(currentProfile);
            profiles.push_back(newUser);
        }
        else {
            size_t pos = line.find("=");
            if (pos != string::npos) {
                string gameName = line.substr(0, pos);
                string gamePath = line.substr(pos + 1);
                profiles.back().addGame(gameName, gamePath); 
            }
        }
    }

    file.close();
    cout << "Profiles loaded from whynot++.cfg.\n";
}

void createProfile() {
    clearScreen();
    printHeader();
    string username;
    cout << "Enter username for the new profile: ";
    getline(cin, username);

    UserProfile newUser(username);
    profiles.push_back(newUser);

    cout << "Profile '" << username << "' created successfully!\n";
    this_thread::sleep_for(seconds(2));
    clearScreen();
    saveProfilesToFile();
}

int selectProfile() {
    clearScreen();
    printHeader();
    cout << "List of profiles:\n";
    for (size_t i = 0; i < profiles.size(); ++i) {
        cout << i + 1 << ". " << profiles[i].username << endl;
    }

    cout << "Choose profile number (or 0 to exit): ";
    int choice;
    cin >> choice;
    cin.ignore();

    clearScreen();

    if (choice > 0 && choice <= static_cast<int>(profiles.size())) {
        return choice - 1;
    }
    else if (choice == 0) {
        return -1;
    }
    else {
        cout << "Invalid choice.\n";
        return selectProfile();
    }
}

void addGameToProfile(UserProfile& profile) {
    clearScreen();
    printHeader();
    string name, path;
    cout << "Enter game name: ";
    getline(cin, name);
    cout << "Enter full path to the game executable (.exe): ";
    getline(cin, path);

    profile.addGame(name, path);

    cout << "Game successfully added to the library!\n";
    this_thread::sleep_for(seconds(2));
    clearScreen();
    saveProfilesToFile();
}

void removeGameFromProfile(UserProfile& profile) {
    clearScreen();
    printHeader();
    if (profile.library.empty()) {
        cout << "The game library is empty.\n";
        this_thread::sleep_for(seconds(2));
        clearScreen();
        return;
    }

    cout << "List of games in the library:\n";
    for (size_t i = 0; i < profile.library.size(); ++i) {
        cout << i + 1 << ". " << profile.library[i].name << endl;
    }

    cout << "Choose the game number to remove (or 0 to cancel): ";
    int choice;
    cin >> choice;
    cin.ignore();

    if (choice > 0 && choice <= static_cast<int>(profile.library.size())) {
        profile.removeGame(choice - 1);
        cout << "Game removed from the library!\n";
    }
    else if (choice != 0) {
        cout << "Invalid choice.\n";
    }

    this_thread::sleep_for(seconds(2));
    clearScreen();
    saveProfilesToFile();
}

void launchGameFromProfile(UserProfile& profile) {
    clearScreen();
    printHeader();
    if (profile.library.empty()) {
        cout << "The game library is empty. Add a game using 'Add Game'.\n";
        this_thread::sleep_for(seconds(2));
        clearScreen();
        return;
    }

    cout << "List of available games:\n";
    for (size_t i = 0; i < profile.library.size(); ++i) {
        cout << i + 1 << ". " << profile.library[i].name;
        if (profile.library[i].isRunning) {
            cout << " (Running)";
        }
        else {
            cout << " (Total time: " << profile.library[i].getTotalTime() << ")";
        }
        cout << endl;
    }

    cout << "Choose the game number to launch (or 0 to exit): ";
    int choice;
    cin >> choice;
    cin.ignore();

    clearScreen();

    if (choice > 0 && choice <= static_cast<int>(profile.library.size())) {
        for (auto& game : profile.library) {
            if (game.isRunning) {
                game.stop();
            }
        }

        profile.library[choice - 1].launch();

        this_thread::sleep_for(seconds(5));
        profile.library[choice - 1].stop();

        launchGameFromProfile(profile);
    }
    else if (choice != 0) {
        cout << "Invalid choice.\n";
        this_thread::sleep_for(seconds(2));
        clearScreen();
    }
}

void requestAdminRights() {
    wchar_t szPath[MAX_PATH];
    if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath))) {
        SHELLEXECUTEINFO sei = { sizeof(sei) };
        sei.lpVerb = L"runas";
        sei.lpFile = szPath;
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.lpParameters = L"--elevated";

        if (!ShellExecuteEx(&sei)) {
            DWORD dwError = GetLastError();
            if (dwError == ERROR_CANCELLED) {
                cout << "Administrator privileges are required. Exiting.\n";
                exit(EXIT_FAILURE);
            }
        }
        else {
            WaitForSingleObject(sei.hProcess, INFINITE);
            CloseHandle(sei.hProcess);
            exit(0); 
        }
    }
}

int main(int argc, char* argv[]) {
    
    if (argc < 2 || strcmp(argv[1], "--elevated") != 0) {
        requestAdminRights();
    }

    loadProfilesFromFile(); 

    while (true) {
        clearScreen();
        printHeader();
        cout << "\nMenu:\n";
        cout << "1. Create Profile\n";
        cout << "2. Select Profile\n";
        cout << "3. Exit\n";
        cout << "Choose an option: ";

        int option;
        cin >> option;
        cin.ignore();

        switch (option) {
        case 1: {
            createProfile();
            break;
        }
        case 2: {
            int profileIndex = selectProfile();
            if (profileIndex != -1) {
                UserProfile& selectedProfile = profiles[profileIndex];

                while (true) {
                    clearScreen();
                    printHeader();
                    cout << "\nProfile: " << selectedProfile.username << endl;
                    cout << "1. Add Game\n";
                    cout << "2. Remove Game\n";
                    cout << "3. Library\n";
                    cout << "4. Back to Profiles\n";
                    cout << "Choose an option: ";

                    int profileOption;
                    cin >> profileOption;
                    cin.ignore();

                    switch (profileOption) {
                    case 1:
                        addGameToProfile(selectedProfile);
                        break;
                    case 2:
                        removeGameFromProfile(selectedProfile);
                        break;
                    case 3:
                        launchGameFromProfile(selectedProfile);
                        break;
                    case 4:
                        cout << "Returning to Profiles menu.\n";
                        this_thread::sleep_for(seconds(2));
                        clearScreen();
                        break;
                    default:
                        cout << "Unknown option. Please try again.\n";
                        this_thread::sleep_for(seconds(2));
                        clearScreen();
                        break;
                    }

                    if (profileOption == 4) {
                        break;
                    }
                }
            }
            break;
        }
        case 3:
            saveProfilesToFile(); 
            cout << "Thank you for using the program!\n";
            return 0;
        default:
            cout << "Unknown option. Please try again.\n";
            this_thread::sleep_for(seconds(2));
            clearScreen();
            break;
        }
    }

    return 0;
}