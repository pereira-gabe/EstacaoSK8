#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <cmath>
#include <memory>
#include <fstream>
#include <algorithm>
#include <random>
#include <functional> 

using namespace sf;
using namespace std;

constexpr double PI_CONST = 3.14159265358979323846;

// Heap binário simples (agora configurado como Min-Heap para Dijkstra)
template<typename T, typename Compare = less<T>>
class BinaryHeap {
    vector<T> data;
    Compare comp;
public:
    void push(const T& v) { data.push_back(v); siftUp(data.size() - 1); }
    void pop() { if (data.empty()) return; data[0] = data.back(); data.pop_back(); if (!data.empty()) siftDown(0); }
    const T& top() const { return data.front(); }
    bool empty() const { return data.empty(); }
    size_t size() const { return data.size(); }
private:
    void siftUp(size_t i) {
        while (i > 0) {
            size_t p = (i - 1) / 2;
            if (!comp(data[i], data[p])) break;
            swap(data[p], data[i]); i = p;
        }
    }
    void siftDown(size_t i) {
        for (;;) {
            size_t l = 2 * i + 1, r = 2 * i + 2, smallest = i;
            if (l < data.size() && comp(data[l], data[smallest])) smallest = l;
            if (r < data.size() && comp(data[r], data[smallest])) smallest = r;
            if (smallest == i) break;
            swap(data[i], data[smallest]); i = smallest;
        }
    }
};

struct CellScore {
    int x, y, score;
    bool operator>(const CellScore& o) const { return score > o.score; }
    bool operator<(const CellScore& o) const { return score < o.score; }
};

struct Cell {
    bool walls[4];
    bool visited;
    bool treasure;
    bool treasureLocked;
    bool treasureUnlocked;
    bool isExit;
    bool trap;
    bool bonus;
    int minigameType;
    bool hadMinigame;
    Cell() {
        walls[0] = walls[1] = walls[2] = walls[3] = true;
        visited = false;
        treasure = false;
        treasureLocked = false;
        treasureUnlocked = false;
        isExit = false;
        trap = false;
        bonus = false;
        minigameType = 0;
        hadMinigame = false;
    }
};

class Maze {
public:
    vector<vector<Cell>> grid;
    int w = 0, h = 0;
    void generate(int W, int H, int extraPaths = 0) {
        w = W; h = H; grid.assign(w, vector<Cell>(h));
        vector<pair<int, int>> st;
        int sx = 0, sy = 0; grid[sx][sy].visited = true; st.emplace_back(sx, sy);
        auto neighbors = [&](int x, int y) { vector<pair<int, int>> n; if (y > 0 && !grid[x][y - 1].visited) n.emplace_back(x, y - 1); if (x + 1 < w && !grid[x + 1][y].visited) n.emplace_back(x + 1, y); if (y + 1 < h && !grid[x][y + 1].visited) n.emplace_back(x, y + 1); if (x > 0 && !grid[x - 1][y].visited) n.emplace_back(x - 1, y); return n; };
        while (!st.empty()) {
            auto [x, y] = st.back(); auto n = neighbors(x, y);
            if (n.empty()) { st.pop_back(); continue; }
            auto chosen = n[rand() % n.size()]; int nx = chosen.first, ny = chosen.second;
            if (nx == x && ny == y - 1) { grid[x][y].walls[0] = false; grid[nx][ny].walls[2] = false; }
            else if (nx == x + 1 && ny == y) { grid[x][y].walls[1] = false; grid[nx][ny].walls[3] = false; }
            else if (nx == x && ny == y + 1) { grid[x][y].walls[2] = false; grid[nx][ny].walls[0] = false; }
            else if (nx == x - 1 && ny == y) { grid[x][y].walls[3] = false; grid[nx][ny].walls[1] = false; }
            grid[nx][ny].visited = true; st.emplace_back(nx, ny);
        }
        if (extraPaths > 0) {
            int base = max(1, (w * h) / 30);
            int extraCount = extraPaths * base;
            std::mt19937 rng((unsigned)time(nullptr));
            std::uniform_int_distribution<int> distX(0, w - 1);
            std::uniform_int_distribution<int> distY(0, h - 1);
            std::uniform_int_distribution<int> distDir(0, 3);
            int attempts = 0;
            int created = 0;
            while (created < extraCount && attempts < extraCount * 10) {
                ++attempts;
                int x = distX(rng); int y = distY(rng); int dir = distDir(rng);
                int nx = x, ny = y;
                if (dir == 0 && y > 0) ny = y - 1; else if (dir == 1 && x + 1 < w) nx = x + 1; else if (dir == 2 && y + 1 < h) ny = y + 1; else if (dir == 3 && x > 0) nx = x - 1; else continue;
                bool wallBetween = false;
                if (nx == x && ny == y - 1) wallBetween = grid[x][y].walls[0] && grid[nx][ny].walls[2];
                else if (nx == x + 1 && ny == y) wallBetween = grid[x][y].walls[1] && grid[nx][ny].walls[3];
                else if (nx == x && ny == y + 1) wallBetween = grid[x][y].walls[2] && grid[nx][ny].walls[0];
                else if (nx == x - 1 && ny == y) wallBetween = grid[x][y].walls[3] && grid[nx][ny].walls[1];
                if (!wallBetween) continue;
                if (nx == 0 && ny == 0) continue;
                if (nx == x && ny == y - 1) { grid[x][y].walls[0] = false; grid[nx][ny].walls[2] = false; }
                else if (nx == x + 1 && ny == y) { grid[x][y].walls[1] = false; grid[nx][ny].walls[3] = false; }
                else if (nx == x && ny == y + 1) { grid[x][y].walls[2] = false; grid[nx][ny].walls[0] = false; }
                else if (nx == x - 1 && ny == y) { grid[x][y].walls[3] = false; grid[nx][ny].walls[1] = false; }
                ++created;
            }
        }
        for (int i = 0;i < w;++i) for (int j = 0;j < h;++j) { grid[i][j].visited = false; grid[i][j].treasure = false; grid[i][j].treasureLocked = false; grid[i][j].treasureUnlocked = false; grid[i][j].isExit = false; grid[i][j].trap = false; grid[i][j].bonus = false; grid[i][j].minigameType = 0; grid[i][j].hadMinigame = false; }
    }

    void placeTreasuresAndTraps(int phase) {
        std::mt19937 rng((unsigned)time(nullptr));
        vector<pair<int, int>> candidates;
        for (int i = 0;i < w;++i) for (int j = 0;j < h;++j) if (!(i == 0 && j == 0)) candidates.emplace_back(i, j);

        int ex = -1, ey = -1;
        if (!candidates.empty()) {
            std::uniform_int_distribution<int> dist(0, (int)candidates.size() - 1);
            auto epos = candidates[dist(rng)]; ex = epos.first; ey = epos.second;
            grid[ex][ey].isExit = true;
        }

        int treasures = 4 + phase * 2;
        int maxMinigameMaze = 11 + 3 * 6;
        int effectiveMaze = min(min(w, h), maxMinigameMaze);
        int minSpread = max(2, (effectiveMaze / 6));
        vector<pair<int, int>> placedMinigames;
        int attempts = 0;
        for (int t = 0; t < treasures && attempts < treasures * 100; ++t) {
            ++attempts;
            if (candidates.empty()) break;
            std::uniform_int_distribution<int> d(0, (int)candidates.size() - 1);
            auto pos = candidates[d(rng)];
            int x = pos.first, y = pos.second;
            if ((x == 0 && y == 0) || (x == ex && y == ey) || grid[x][y].treasure || grid[x][y].bonus) { --t; continue; }
            bool ok = true;
            for (auto& p : placedMinigames) if (abs(p.first - x) + abs(p.second - y) < minSpread) { ok = false; break; }
            if (!ok) { --t; continue; }
            grid[x][y].treasure = true; grid[x][y].treasureLocked = true; grid[x][y].treasureUnlocked = false; grid[x][y].minigameType = rng() % 3;
            placedMinigames.emplace_back(x, y);
        }

        int bonusRooms = max(3, treasures / 2 + phase);
        int bplaced = 0; attempts = 0;
        while (bplaced < bonusRooms && attempts < bonusRooms * 120) {
            ++attempts;
            if (candidates.empty()) break;
            int idx = rng() % (int)candidates.size(); auto pos = candidates[idx]; int x = pos.first, y = pos.second;
            if ((x == 0 && y == 0) || (x == ex && y == ey) || grid[x][y].treasure || grid[x][y].bonus) continue;
            bool ok = true;
            for (auto& p : placedMinigames) if (abs(p.first - x) + abs(p.second - y) < minSpread) { ok = false; break; }
            if (!ok) continue;
            if (rng() % 100 < 60 + phase * 5) { grid[x][y].bonus = true; grid[x][y].minigameType = rng() % 3; placedMinigames.emplace_back(x, y); ++bplaced; }
        }

        int traps = max(3, (w * h) / 12 + phase * 3);
        int placed = 0; attempts = 0;
        while (placed < traps && attempts < traps * 50) {
            ++attempts;
            int x = rng() % w; int y = rng() % h;
            if ((x == 0 && y == 0) || (x == ex && y == ey) || grid[x][y].treasure || grid[x][y].trap || grid[x][y].bonus) continue;
            grid[x][y].trap = true; ++placed;
        }
    }
};

// Gerenciador de áudio - gera sons
class AudioManager {
public:
    SoundBuffer bufCollect, bufTrap, bufWin, bufDecoy, bufStart;
    unique_ptr<Sound> sCollect, sTrap, sWin, sDecoy, sStart;
    AudioManager() { generateAll(); }
    vector<int16_t> generateSine(double freq, size_t samples, double amplitude = 8000.0, unsigned sampleRate = 22050) {
        vector<int16_t> data(samples);
        double twoPiF = 2.0 * PI_CONST * freq / sampleRate;
        for (size_t i = 0;i < samples;++i) data[i] = static_cast<int16_t>(amplitude * sin(i * twoPiF));
        return data;
    }
    void generateAll() {
        unsigned sr = 22050;
        auto c1 = generateSine(880.0, sr / 12, 8000.0, sr);
        auto c2 = generateSine(440.0, sr / 12, 6000.0, sr);
        auto c3 = generateSine(1760.0, sr / 8, 9000.0, sr);
        auto c4 = generateSine(330.0, sr / 12, 6000.0, sr);
        auto c5 = generateSine(660.0, sr / 6, 7000.0, sr);
        vector<SoundChannel> mono{ SoundChannel::Mono };
        bufCollect.loadFromSamples(c1.data(), c1.size(), 1u, sr, mono);
        bufTrap.loadFromSamples(c2.data(), c2.size(), 1u, sr, mono);
        bufWin.loadFromSamples(c3.data(), c3.size(), 1u, sr, mono);
        bufDecoy.loadFromSamples(c4.data(), c4.size(), 1u, sr, mono);
        bufStart.loadFromSamples(c5.data(), c5.size(), 1u, sr, mono);
        sCollect = make_unique<Sound>(bufCollect);
        sTrap = make_unique<Sound>(bufTrap);
        sWin = make_unique<Sound>(bufWin);
        sDecoy = make_unique<Sound>(bufDecoy);
        sStart = make_unique<Sound>(bufStart);
    }
};

// Classe principal do jogo
class Game {
    RenderWindow window;
    Font font;
    Maze maze;
    AudioManager audio;

    int phase{ 1 };
    int mazeSize{ 5 };
    int px{ 0 }, py{ 0 };
    int ex{ 0 }, ey{ 0 };
    int score{ 0 };
    int totalScore{ 0 };
    int scoreAtPhaseStart{ 0 };
    int bestScore{ 0 };
    int timeLeft{ 0 };
    Clock clockGame; Time totalTime;
    bool gameOver{ false };
    bool playerAlive{ true };
    bool won{ false };

    vector<int> phaseScores;

    struct Pickup { Vector2f pos; int type; bool clicked; };
    vector<Pickup> currentPickups;
    bool pickupsActive{ false };

    bool minigameActive{ false };
    float minigameTimeLeft{ 0.f };
    const float minigameTotal{ 12.f };
    int lastMinigameType{ -1 };

    float artifactAngle{ 0.f };

    enum class State { Menu, Playing, Paused, End }; State state{ State::Menu };
    int menuSelection{ 0 };

    float overlayAlpha{ 0.f };
    float fadeSpeed{ 1.8f };

    bool fontLoaded{ false };

    float exitMoveTimer{ 0.f };
    float exitMoveInterval{ 9999.f };

    float enemyTimer{ 0.f };

public:
    Game() {
        window.create(VideoMode(Vector2u(1280u, 820u)), "Estacao SK-8 - Projeto AEDS");
        window.setFramerateLimit(60);
        if (font.openFromFile("C:\\Users\\perei\\AppData\\Local\\Microsoft\\Windows\\Fonts\\PressStart2P-Regular.ttf")) fontLoaded = true;
        else if (font.openFromFile("C:/Windows/Fonts/arial.ttf")) fontLoaded = true;
        else { fontLoaded = false; cerr << "Fonte nao encontrada\n"; }
        srand((unsigned)time(nullptr));
        loadBestScore();
        state = State::Menu;
        if (audio.sStart) audio.sStart->play();
    }

    void run() {
        Clock deltaClock;
        while (window.isOpen()) {
            float dt = deltaClock.restart().asSeconds();
            processEvents();
            if (state == State::Playing) update(dt); else updateFade(dt);
            render();
        }
    }

private:
    void loadBestScore() { ifstream in("highscore.txt"); if (in) { in >> bestScore; in.close(); } else bestScore = 0; }
    void saveBestScore() { if (totalScore > bestScore) { ofstream out("highscore.txt"); if (out) { out << totalScore; out.close(); bestScore = totalScore; } } }

    void startNewRun() { totalScore = 0; phaseScores.clear(); phase = 1; score = 0; scoreAtPhaseStart = 0; startPhase(1); }

    vector<pair<int, int>> findPath(int sx, int sy, int tx, int ty) {
        vector<pair<int, int>> empty;
        if (sx == tx && sy == ty) return { {sx,sy} };
        int W = maze.w, H = maze.h;
        const int INF = 1e9;
        vector<vector<int>> dist(W, vector<int>(H, INF));
        vector<vector<pair<int, int>>> parent(W, vector<pair<int, int>>(H, { -1,-1 }));

        BinaryHeap<CellScore, less<CellScore>> pq;

        dist[sx][sy] = 0;
        pq.push({ sx, sy, 0 });

        while (!pq.empty()) {
            CellScore current = pq.top();
            pq.pop();
            int x = current.x;
            int y = current.y;
            int d = current.score;

            if (d > dist[x][y]) continue;
            if (x == tx && y == ty) break;

            int dx[] = { 0, 1, 0, -1 };
            int dy[] = { -1, 0, 1, 0 };
            int wallIndex[] = { 0, 1, 2, 3 };

            for (int i = 0; i < 4; ++i) {
                if (!maze.grid[x][y].walls[wallIndex[i]]) {
                    int nx = x + dx[i];
                    int ny = y + dy[i];

                    if (nx >= 0 && nx < W && ny >= 0 && ny < H) {
                        int newDist = d + 1;
                        if (newDist < dist[nx][ny]) {
                            dist[nx][ny] = newDist;
                            parent[nx][ny] = { x, y };
                            pq.push({ nx, ny, newDist });
                        }
                    }
                }
            }
        }

        if (dist[tx][ty] == INF) return empty;

        vector<pair<int, int>> path;
        pair<int, int> cur = { tx,ty };
        while (!(cur.first == -1 && cur.second == -1)) {
            path.push_back(cur);
            if (cur.first == sx && cur.second == sy) break;
            cur = parent[cur.first][cur.second];
        }
        reverse(path.begin(), path.end());
        return path;
    }

    void triggerTrapRoom() {
        for (int i = 0;i < maze.w;++i) for (int j = 0;j < maze.h;++j) if (maze.grid[i][j].isExit) { ex = i; ey = j; }
        int extraPaths = min(phase, 5);
        maze.generate(maze.w, maze.h, extraPaths);
        maze.placeTreasuresAndTraps(phase);
        if (ex != -1 && ey != -1) {
            for (int i = 0;i < maze.w;++i) for (int j = 0;j < maze.h;++j) maze.grid[i][j].isExit = false;
            ex = min(max(ex, 0), maze.w - 1); ey = min(max(ey, 0), maze.h - 1);
            maze.grid[ex][ey].isExit = true;
        }
        if (px >= 0 && px < maze.w && py >= 0 && py < maze.h) maze.grid[px][py].visited = true;
        if (audio.sTrap) audio.sTrap->play();
    }

    void startPhase(int p) {
        phase = p;
        int effectivePhaseForSize = min(phase - 1, 4);
        mazeSize = 11 + effectivePhaseForSize * 6;
        int extraPaths = min(phase, 5);
        maze.generate(mazeSize, mazeSize, extraPaths);
        maze.placeTreasuresAndTraps(phase);
        px = 0; py = 0;  playerAlive = true; gameOver = false; won = false; artifactAngle = 0.f; pickupsActive = false; currentPickups.clear();
        exitMoveInterval = 9999.f;
        exitMoveTimer = 0.f;
        totalTime = seconds(60.f + (phase - 1) * 30.f);
        clockGame.restart(); timeLeft = static_cast<int>(totalTime.asSeconds());
        overlayAlpha = 200.f;
        exitMoveInterval = max(0.25f, 1.5f - 0.15f * (phase - 1));
        scoreAtPhaseStart = score;
        for (int i = 0;i < maze.w;++i) for (int j = 0;j < maze.h;++j) if (maze.grid[i][j].isExit) { ex = i; ey = j; }
    }

    void processEvents() {
        while (auto ev = window.pollEvent()) {
            if (ev->is<Event::Closed>()) { saveBestScore(); window.close(); }
            if (ev->is<Event::KeyPressed>()) {
                auto kp = ev->getIf<Event::KeyPressed>(); if (!kp) continue;
                if (state == State::Menu) handleMenuInput(kp->code);
                else if (state == State::Playing) {
                    if (kp->code == Keyboard::Key::Escape) { state = State::Paused; overlayAlpha = 0.f; }
                    else if (kp->code == Keyboard::Key::R) { // restart diretamente a corrida
                        startNewRun(); state = State::Playing; overlayAlpha = 200.f; if (audio.sStart) audio.sStart->play();
                    }
                    else handleMovement(kp->code);
                }
                else if (state == State::Paused) {
                    if (kp->code == Keyboard::Key::Escape) { state = State::Playing; overlayAlpha = 0.f; }
                    else handlePauseInput(kp->code);
                }
                else if (state == State::End) { if (kp->code == Keyboard::Key::Enter || kp->code == Keyboard::Key::Space) { state = State::Menu; overlayAlpha = 0.f; } }
            }
            if (ev->is<Event::MouseButtonPressed>()) {
                auto mp = ev->getIf<Event::MouseButtonPressed>(); if (!mp) continue;
                if (state == State::Playing && mp->button == Mouse::Button::Left && playerAlive) handleClick(mp->position.x, mp->position.y);
                if (state == State::Menu && mp->button == Mouse::Button::Left) handleMenuMouse(mp->position.x, mp->position.y);
                if (state == State::End && mp->button == Mouse::Button::Left) { state = State::Menu; overlayAlpha = 0.f; }
            }
        }
    }

    void handleMenuInput(Keyboard::Key code) {
        if (code == Keyboard::Key::Enter || code == Keyboard::Key::Space) { startNewRun(); state = State::Playing; overlayAlpha = 200.f; if (audio.sStart) audio.sStart->play(); }
    }

    void handlePauseInput(Keyboard::Key code) { if (code == Keyboard::Key::Up) menuSelection = max(0, menuSelection - 1); else if (code == Keyboard::Key::Down) menuSelection = min(1, menuSelection + 1); else if (code == Keyboard::Key::Enter) applyMenuSelection(); }

    void handleMenuMouse(int mx, int my) {
        float boxW = 360.f, boxH = 200.f;
        float boxX = ((float)window.getSize().x - boxW) / 2.f;
        float boxY = ((float)window.getSize().y - boxH) / 2.f;
        if (mx >= (int)boxX && mx <= (int)(boxX + boxW) && my >= (int)boxY && my <= (int)(boxY + boxH)) {
            int relY = (int)(my - boxY);
            int item = relY / 80;
            if (item == 0) { startNewRun(); state = State::Playing; overlayAlpha = 200.f; }
            else if (item == 1) { saveBestScore(); window.close(); }
        }
    }

    void applyMenuSelection() { if (menuSelection == 0) { startNewRun(); state = State::Playing; overlayAlpha = 200.f; } else if (menuSelection == 1) { saveBestScore(); window.close(); } }

    void generatePickupsForCurrentRoom() {
        currentPickups.clear();
        int type = maze.grid[px][py].minigameType;
        if (type == lastMinigameType) type = 0;
        lastMinigameType = type;
        int blueCount = 8 + rand() % 6;
        int redCount = 1 + rand() % 3;
        if (type == 0) { blueCount = 8 + rand() % 6; redCount = 1 + rand() % 2; }
        else if (type == 1) { blueCount = 6 + rand() % 4; redCount = 2 + rand() % 3; }
        else if (type == 2) { blueCount = 4 + rand() % 4; redCount = 0 + rand() % 2; }
        float side = getViewSide(); float spacing = getViewSpacing(); float totalWidth = side * 2 + spacing;
        float left = ((float)window.getSize().x - totalWidth) / 2.f;
        float top = ((float)window.getSize().y - side) / 2.f;
        int pad = max(12, (int)(side * 0.08f));
        int areaW = max(20, (int)(side)-pad * 2);
        int areaH = max(20, (int)(side)-pad * 2);
        auto pushRandom = [&](int t) { int rx = (rand() % areaW) + (int)(left + pad); int ry = (rand() % areaH) + (int)(top + pad); currentPickups.push_back({ Vector2f((float)rx,(float)ry), t, false }); };
        for (int i = 0;i < blueCount;++i) pushRandom(0);
        for (int i = 0;i < redCount;++i) pushRandom(2);
        std::mt19937 rng((unsigned)rand());
        std::shuffle(currentPickups.begin(), currentPickups.end(), rng);
        pickupsActive = true;
        minigameActive = true; minigameTimeLeft = minigameTotal;
    }

    void handleClick(int mx, int my) {
        if (pickupsActive) {
            for (auto& p : currentPickups) {
                float dx = mx - p.pos.x; float dy = my - p.pos.y; if (dx * dx + dy * dy <= 18.f * 18.f && !p.clicked) {
                    p.clicked = true;
                    if (p.type == 2) { score = max(0, score - 5); if (audio.sDecoy) audio.sDecoy->play(); }
                    else { if (audio.sCollect) audio.sCollect->play(); }
                }
            }
            bool anyBlueRemaining = false; for (auto& p : currentPickups) { if (p.type == 0 && !p.clicked) { anyBlueRemaining = true; break; } }
            if (!anyBlueRemaining) { pickupsActive = false; currentPickups.clear(); maze.grid[px][py].treasureLocked = false; maze.grid[px][py].treasureUnlocked = true; minigameActive = false; maze.grid[px][py].hadMinigame = true; }
            return;
        }
        float side = getViewSide(); float spacing = getViewSpacing(); float totalWidth = side * 2 + spacing;
        float left = ((float)window.getSize().x - totalWidth) / 2.f;
        float top = ((float)window.getSize().y - side) / 2.f;
        IntRect pickupArea(Vector2i(static_cast<int>(left), static_cast<int>(top)), Vector2i(static_cast<int>(side), static_cast<int>(side)));
        if (pickupArea.contains(Vector2i(mx, my))) {
            if (maze.grid[px][py].treasureUnlocked) { score += 10; maze.grid[px][py].treasureUnlocked = false; maze.grid[px][py].treasure = false; if (audio.sCollect) audio.sCollect->play(); }
            if (maze.grid[px][py].treasureUnlocked) { score += 10; maze.grid[px][py].treasureUnlocked = false; maze.grid[px][py].treasure = false; maze.grid[px][py].hadMinigame = true; if (audio.sCollect) audio.sCollect->play(); }
        }
    }

    void update(float dt) {
        if (!gameOver) {
            Time el = clockGame.getElapsedTime(); timeLeft = max(0, (int)(totalTime.asSeconds() - el.asSeconds()));
            if (timeLeft <= 0) { gameOver = true; playerAlive = false; if (audio.sTrap) audio.sTrap->play(); startPhase(1); }
            if (playerAlive && !minigameActive) {
                enemyTimer += dt;
                if (enemyTimer >= 0.35f) {
                    enemyTimer = 0.f;
                    vector<pair<int, int>> path = findPath(ex, ey, px, py);
                    if (path.size() > 1) { ex = path[1].first; ey = path[1].second; }
                    if (ex == px && ey == py) {
                        gameOver = true; playerAlive = false;
                        if (audio.sTrap) audio.sTrap->play();
                        startPhase(1);
                    }
                }
            }
        }
        if (minigameActive) { minigameTimeLeft -= dt; if (minigameTimeLeft <= 0.f) { maze.grid[px][py].treasure = false; maze.grid[px][py].treasureLocked = false; maze.grid[px][py].treasureUnlocked = false; maze.grid[px][py].bonus = false; pickupsActive = false; currentPickups.clear(); minigameActive = false; } }
        if (minigameActive) {
            minigameTimeLeft -= dt;
            if (minigameTimeLeft <= 0.f) {
                maze.grid[px][py].treasure = false; maze.grid[px][py].treasureLocked = false; maze.grid[px][py].treasureUnlocked = false; maze.grid[px][py].bonus = false; pickupsActive = false; currentPickups.clear(); minigameActive = false; maze.grid[px][py].hadMinigame = true;
            }
        }
        artifactAngle += 120.f * dt;
        updateFade(dt);
    }

    void updateFade(float dt) { float target = (state == State::Playing) ? 0.f : 200.f; if (overlayAlpha < target) overlayAlpha = min(target, overlayAlpha + fadeSpeed * 255.f * dt); else if (overlayAlpha > target) overlayAlpha = max(target, overlayAlpha - fadeSpeed * 255.f * dt); }

    void render() { window.clear(Color(6, 8, 12)); drawBackground(); if (state == State::Menu) drawMainMenu(); else if (state == State::End) drawEndScreen(); else { drawFirstPersonAndMap(); if (state == State::Paused) drawPauseOverlay(); } window.display(); }

    void drawBackground() {
        // gradiente de fundo
        RectangleShape bg(Vector2f((float)window.getSize().x, (float)window.getSize().y)); bg.setFillColor(Color(10, 12, 18)); window.draw(bg);
        // linhas de scan
        for (int y = 0;y < (int)window.getSize().y; y += 2) { RectangleShape line(Vector2f((float)window.getSize().x, 1.f)); line.setPosition(Vector2f(0.f, (float)y)); line.setFillColor(Color(0, 0, 0, 20)); window.draw(line); }
        // barra superior
        RectangleShape topBar(Vector2f((float)window.getSize().x, 60.f)); topBar.setFillColor(Color(20, 18, 28)); window.draw(topBar);
        if (fontLoaded) {
            Text title(font, "ESTACAO SK-8 - RETRO ARCADE", 20); title.setFillColor(Color(180, 210, 240)); title.setPosition(Vector2f(14.f, 14.f)); window.draw(title);
            // área HUD abaixo do cabeçalho
            Text s1(font, string("Fase: ") + to_string(phase), 16); s1.setFillColor(Color::White); s1.setPosition(Vector2f(14.f, 66.f)); window.draw(s1);
            Text s2(font, string("Pontos: ") + to_string(score), 16); s2.setFillColor(Color::White); s2.setPosition(Vector2f(140.f, 66.f)); window.draw(s2);
            Text s3(font, string(" Tempo: ") + to_string(timeLeft) + "s", 16); s3.setFillColor(Color::White); s3.setPosition(Vector2f(300.f, 66.f)); window.draw(s3);
        }
        if (fontLoaded) { Text instr(font, "WASD/Setas: mover | Clique: pegar artefato | R: reiniciar", 14); instr.setFillColor(Color(200, 200, 220)); instr.setPosition(Vector2f(14.f, (float)window.getSize().y - 36.f)); window.draw(instr); }
    }

    void drawMainMenu() {
        RectangleShape overlay(Vector2f((float)window.getSize().x, (float)window.getSize().y)); overlay.setFillColor(Color(0, 0, 0, static_cast<uint8_t>(overlayAlpha))); window.draw(overlay);
        float boxW = 360.f, boxH = 200.f; float boxX = ((float)window.getSize().x - boxW) / 2.f; float boxY = ((float)window.getSize().y - boxH) / 2.f;
        RectangleShape box(Vector2f(boxW, boxH)); box.setFillColor(Color(12, 14, 20)); box.setOutlineColor(Color(160, 160, 200)); box.setOutlineThickness(2.f); box.setPosition(Vector2f(boxX, boxY)); window.draw(box);
        if (fontLoaded) {
            Text h(font, "Jogar", 28); h.setPosition(Vector2f(boxX + boxW / 2.f - 40.f, boxY + 20.f)); h.setFillColor(Color::Cyan); window.draw(h);
            Text quit(font, "Sair", 20); quit.setPosition(Vector2f(boxX + boxW / 2.f - 20.f, boxY + 110.f)); quit.setFillColor(Color::White); window.draw(quit);
        }
    }

    void drawEndScreen() {
        RectangleShape overlay(Vector2f((float)window.getSize().x, (float)window.getSize().y)); overlay.setFillColor(Color(0, 0, 0, static_cast<uint8_t>(overlayAlpha))); window.draw(overlay);
        float boxW = 640.f, boxH = 480.f; float boxX = ((float)window.getSize().x - boxW) / 2.f; float boxY = ((float)window.getSize().y - boxH) / 2.f;
        RectangleShape box(Vector2f(boxW, boxH)); box.setFillColor(Color(8, 10, 14)); box.setOutlineColor(Color(180, 180, 220)); box.setOutlineThickness(2.f); box.setPosition(Vector2f(boxX, boxY)); window.draw(box);
        if (fontLoaded) {
            Text t(font, "FIM DE JOGO - RESUMO", 26); t.setPosition(Vector2f(boxX + 40.f, boxY + 24.f)); t.setFillColor(Color::Cyan); window.draw(t);
            int y = (int)(boxY + 80.f);
            for (int i = 0;i < (int)phaseScores.size();++i) { stringstream ss; ss << "Fase " << (i + 1) << ": " << phaseScores[i] << " pontos"; Text p(font, ss.str(), 18); p.setPosition(Vector2f(boxX + 40.f, (float)y)); p.setFillColor(Color::White); window.draw(p); y += 30; }
            stringstream total; total << "Total: " << totalScore; Text tot(font, total.str(), 20); tot.setPosition(Vector2f(boxX + 40.f, (float)y + 10)); tot.setFillColor(Color::Yellow); window.draw(tot);
            Text hint(font, "Pressione Enter para voltar ao menu", 16); hint.setPosition(Vector2f(boxX + 40.f, (float)y + 60)); hint.setFillColor(Color::White); window.draw(hint);
        }
    }

    void drawPauseOverlay() {
        RectangleShape overlay(Vector2f((float)window.getSize().x, (float)window.getSize().y)); overlay.setFillColor(Color(0, 0, 0, static_cast<uint8_t>(overlayAlpha))); window.draw(overlay);
        if (fontLoaded) { Text p(font, "PAUSADO", 36); p.setPosition(Vector2f(((float)window.getSize().x) / 2.f - 60.f, 260.f)); p.setFillColor(Color::White); window.draw(p); Text instr(font, "Pressione ESC para voltar", 16); instr.setPosition(Vector2f(((float)window.getSize().x) / 2.f - 110.f, 320.f)); instr.setFillColor(Color::White); window.draw(instr); }
    }

    void drawFirstPersonAndMap() {
        // área de visão e minimapa (responsive sizes)
        float side = getViewSide(); float spacing = getViewSpacing(); float totalWidth = side * 2 + spacing; float left = ((float)window.getSize().x - totalWidth) / 2.f;
        Vector2f viewPos(left, ((float)window.getSize().y - side) / 2.f);
        RectangleShape roomBox(Vector2f(side, side)); roomBox.setPosition(viewPos); roomBox.setFillColor(Color(36, 40, 56)); roomBox.setOutlineColor(Color::White); roomBox.setOutlineThickness(2.f); window.draw(roomBox);
        // mostrar somente tempo do minigame (sem titulo)
        if (minigameActive && fontLoaded) { stringstream ss; ss << (int)ceil(minigameTimeLeft) << "s"; Text mm(font, ss.str(), 20); mm.setFillColor(Color::Cyan); mm.setPosition(Vector2f(viewPos.x + side / 2.f - 18.f, viewPos.y - 28.f)); window.draw(mm); }
        // efeitos visuais e portas
        for (int i = 0;i < 6;++i) { float s = side - i * 40.f; RectangleShape r(Vector2f(s, s)); r.setOrigin(Vector2f(s / 2.f, s / 2.f)); r.setPosition(Vector2f(viewPos.x + side / 2.f, viewPos.y + side / 2.f)); r.setFillColor(Color(0, 0, 0, 0)); r.setOutlineThickness(1.f); r.setOutlineColor(Color(60 + i * 8, 60 + i * 5, 90 + i * 6)); window.draw(r); }
        if (!maze.grid[px][py].walls[0]) if (fontLoaded) { Text up(font, "^", 28); up.setPosition(Vector2f(viewPos.x + side / 2.f - 12, viewPos.y + 6.f)); up.setFillColor(Color::White); window.draw(up); }
        if (!maze.grid[px][py].walls[1]) if (fontLoaded) { Text rgt(font, ">", 28); rgt.setPosition(Vector2f(viewPos.x + side - 28, viewPos.y + side / 2.f - 12)); rgt.setFillColor(Color::White); window.draw(rgt); }
        if (!maze.grid[px][py].walls[2]) if (fontLoaded) { Text dn(font, "v", 28); dn.setPosition(Vector2f(viewPos.x + side / 2.f - 12, viewPos.y + side - 36)); dn.setFillColor(Color::White); window.draw(dn); }
        if (!maze.grid[px][py].walls[3]) if (fontLoaded) { Text lft(font, "<", 28); lft.setPosition(Vector2f(viewPos.x + 6.f, viewPos.y + side / 2.f - 12)); lft.setFillColor(Color::White); window.draw(lft); }
        if (pickupsActive) for (auto& p : currentPickups) {
            float pickupSize = max(28.f, side * 0.09f); // larger pickup squares
            RectangleShape pix(Vector2f(pickupSize, pickupSize)); pix.setOrigin(Vector2f(pickupSize / 2.f, pickupSize / 2.f)); pix.setPosition(p.pos);
            if (p.clicked) pix.setFillColor(Color(80, 80, 120));
            else { if (p.type == 0) pix.setFillColor(Color(100, 160, 255)); else pix.setFillColor(Color(200, 60, 60)); }
            pix.setOutlineThickness(2.f); pix.setOutlineColor(Color(20, 20, 30)); window.draw(pix);
        }
        if (maze.grid[px][py].treasureUnlocked) { float bob = sin(artifactAngle * 0.05f) * 8.f; RectangleShape trec(Vector2f(28.f, 28.f)); trec.setOrigin(Vector2f(14.f, 14.f)); trec.setPosition(Vector2f(viewPos.x + side / 2.f, viewPos.y + side / 2.f + bob)); trec.setFillColor(Color::Yellow); trec.setOutlineColor(Color(200, 180, 60)); trec.setOutlineThickness(2.f); window.draw(trec); }

        Vector2f mapPos(viewPos.x + side + spacing, viewPos.y);
        RectangleShape mapBox(Vector2f(side, side)); mapBox.setPosition(mapPos); mapBox.setFillColor(Color(18, 18, 28)); mapBox.setOutlineColor(Color::White); mapBox.setOutlineThickness(2.f); window.draw(mapBox);
        int w = maze.w, h = maze.h; float cell = side / float(max(1, w));
        for (int i = 0;i < w;++i) for (int j = 0;j < h;++j) { float cx = mapPos.x + i * cell; float cy = mapPos.y + j * cell; RectangleShape c(Vector2f(cell - 1, cell - 1)); c.setPosition(Vector2f(cx, cy)); if (i == px && j == py) c.setFillColor(Color::Cyan); else if (maze.grid[i][j].isExit) c.setFillColor(Color::Green); else c.setFillColor(maze.grid[i][j].visited ? Color(70, 70, 90) : Color(30, 30, 40)); window.draw(c); }

        // draw trap-room markers as small triangles (more frequent now)
        for (int i = 0;i < w;++i) for (int j = 0;j < h;++j) if (maze.grid[i][j].trap) {
            float cx = mapPos.x + i * cell; float cy = mapPos.y + j * cell;
            ConvexShape tri; tri.setPointCount(3);
            tri.setPoint(0, Vector2f(cx + cell * 0.5f, cy + cell * 0.15f));
            tri.setPoint(1, Vector2f(cx + cell * 0.15f, cy + cell * 0.85f));
            tri.setPoint(2, Vector2f(cx + cell * 0.85f, cy + cell * 0.85f));
            tri.setFillColor(Color(220, 80, 30, 220)); window.draw(tri);
        }

        // draw circles for cells that have or had minigames (always visible)
        for (int i = 0;i < w;++i) for (int j = 0;j < h;++j) {
            if (maze.grid[i][j].treasure || maze.grid[i][j].bonus || maze.grid[i][j].hadMinigame) {
                float cx = mapPos.x + i * cell + cell * 0.5f;
                float cy = mapPos.y + j * cell + cell * 0.5f;
                float r = max(1.f, cell * 0.22f);
                CircleShape circ(r);
                circ.setOrigin(Vector2f(r, r));
                circ.setPosition(Vector2f(cx, cy));
                if (maze.grid[i][j].treasure || maze.grid[i][j].bonus) circ.setFillColor(Color(100, 200, 255, 220)); // active minigame
                else circ.setFillColor(Color(180, 100, 200, 200)); // cleared
                window.draw(circ);
            }
        }

        // draw enemy
        float ecx = mapPos.x + ex * cell + cell * 0.5f;
        float ecy = mapPos.y + ey * cell + cell * 0.5f;
        float er = max(1.f, cell * 0.3f);
        CircleShape enemyShape(er);
        enemyShape.setOrigin(Vector2f(er, er));
        enemyShape.setPosition(Vector2f(ecx, ecy));
        enemyShape.setFillColor(Color::Red);
        window.draw(enemyShape);

        for (int i = 0;i < w;++i) for (int j = 0;j < h;++j) { float x0 = mapPos.x + i * cell, y0 = mapPos.y + j * cell; if (maze.grid[i][j].walls[0]) { RectangleShape line(Vector2f(cell, max(1.f, cell * 0.08f))); line.setPosition(Vector2f(x0, y0)); line.setFillColor(Color::White); window.draw(line); } if (maze.grid[i][j].walls[1]) { RectangleShape line(Vector2f(max(1.f, cell * 0.08f), cell)); line.setPosition(Vector2f(x0 + cell - max(1.f, cell * 0.08f), y0)); line.setFillColor(Color::White); window.draw(line); } if (maze.grid[i][j].walls[2]) { RectangleShape line(Vector2f(cell, max(1.f, cell * 0.08f))); line.setPosition(Vector2f(x0, y0 + cell - max(1.f, cell * 0.08f))); line.setFillColor(Color::White); window.draw(line); } if (maze.grid[i][j].walls[3]) { RectangleShape line(Vector2f(max(1.f, cell * 0.08f), cell)); line.setPosition(Vector2f(x0, y0)); line.setFillColor(Color::White); window.draw(line); } }
    }

    // responsive sizing for first-person view and minimap
    float getViewSide() const {
        float W = (float)window.getSize().x;
        float H = (float)window.getSize().y;
        // slightly smaller than before
        return min(H * 0.66f, W * 0.50f);
    }
    float getViewSpacing() const { return max(16.f, (float)window.getSize().x * 0.02f); }

    void handleMovement(Keyboard::Key code) {
        // bloquear movimento enquanto minigame ativo
        if (minigameActive) return;
        int nx = px, ny = py;
        if (code == Keyboard::Key::Up || code == Keyboard::Key::W) { if (!maze.grid[px][py].walls[0]) ny -= 1; }
        else if (code == Keyboard::Key::Down || code == Keyboard::Key::S) { if (!maze.grid[px][py].walls[2]) ny += 1; }
        else if (code == Keyboard::Key::Left || code == Keyboard::Key::A) { if (!maze.grid[px][py].walls[3]) nx -= 1; }
        else if (code == Keyboard::Key::Right || code == Keyboard::Key::D) { if (!maze.grid[px][py].walls[1]) nx += 1; }
        if (nx != px || ny != py) {
            px = nx; py = ny; maze.grid[px][py].visited = true;
            // se sala tem tesouro travado ou bonus, iniciar minigame
            if ((maze.grid[px][py].treasure && maze.grid[px][py].treasureLocked) || maze.grid[px][py].bonus) { generatePickupsForCurrentRoom(); pickupsActive = true; }
            if (maze.grid[px][py].trap) {
                triggerTrapRoom();
                return;
            }
            if (maze.grid[px][py].isExit) {
                int phasePoints = score - scoreAtPhaseStart;
                phaseScores.push_back(phasePoints);
                totalScore = score; // cumulativo
                if (phase < 6) startPhase(phase + 1);
                else {
                    won = true; gameOver = true; playerAlive = false;
                    if (audio.sWin) audio.sWin->play();
                    saveBestScore(); state = State::End; overlayAlpha = 0.f;
                }
            }
        }
    }
};

int main() { Game game; game.run(); return 0; }
