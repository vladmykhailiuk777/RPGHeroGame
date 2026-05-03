#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <thread>
#include <chrono>

using namespace std;

// =========================================================
// ЕТАП 1: Класи об'єктів
// =========================================================

class Entity {
public:
    string name;
    int hp, maxHp, atk, def;
    Entity(string n, int h, int a, int d) : name(n), hp(h), maxHp(h), atk(a), def(d) {}
    bool isAlive() const { return hp > 0; }
};

class Player : public Entity {
public:
    int x, y, level, xp, gold, luck;
    Player() : Entity("Герой", 100, 20, 5), x(1), y(1), level(1), xp(0), gold(0), luck(5) {}

    void addXP(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++;
            xp -= 100;
            maxHp += 20;
            hp = maxHp;
            atk += 5;
            def += 2;
            cout << "\n[!] РІВЕНЬ ПІДНЯТО! Тепер ви " << level << " рівня!\n";
        }
    }
};

class Enemy : public Entity {
public:
    Enemy(string n, int h, int a, int d) : Entity(n, h, a, d) {}
    static Enemy spawn(int level) {
        if (level == 1) return Enemy("Скелет", 45, 12, 2);
        if (level == 2) return Enemy("Орк", 80, 18, 5);
        return Enemy("Темний Лицар", 150, 28, 10);
    }
};

// =========================================================
// ЕТАП 2: Візуал та Інтерфейс
// =========================================================

void clearScreen() {
    // Універсальний спосіб очищення для CLion/Windows
    cout << "\033[2J\033[1;1H";
}

void drawBox(string title, vector<string> lines) {
    int width = 55;
    cout << " +-----------------------------------------------------+\n";
    cout << " | " << title;
    for (int i = 0; i < width - (int)title.length() - 3; i++) cout << " ";
    cout << "|\n +-----------------------------------------------------+\n";
    for (const auto& line : lines) {
        cout << " | " << line;
        for (int i = 0; i < width - (int)line.length() - 2; i++) cout << " ";
        cout << "|\n";
    }
    cout << " +-----------------------------------------------------+\n";
}

void drawHPBar(string label, int current, int maxVal) {
    int barWidth = 20;
    float ratio = (float)current / maxVal;
    int filled = (int)(barWidth * ratio);
    cout << " " << label << " [";
    for (int i = 0; i < barWidth; ++i) {
        if (i < filled) cout << "#";
        else cout << "-";
    }
    cout << "] " << current << "/" << maxVal << "\n";
}

// =========================================================
// ЕТАП 3: Логіка гри
// =========================================================

class GameEngine {
private:
    vector<string> map;
    vector<string> fog;
    Player hero;
    int currentLevel;
    bool running;

    void generateLevel() {
        int h = 8, w = 18;
        map.assign(h, string(w, '#'));
        fog.assign(h, string(w, '?'));

        for (int y = 1; y < h - 1; y++)
            for (int x = 1; x < w - 1; x++) map[y][x] = '.';

        map[h - 2][w - 2] = 'X'; // Вихід

        char items[] = { 'E', 'E', '$', '~', '^' };
        for (char item : items) {
            int rx, ry;
            do { rx = rand() % (w - 2) + 1; ry = rand() % (h - 2) + 1; }
            while (map[ry][rx] != '.');
            map[ry][rx] = item;
        }
        hero.x = 1; hero.y = 1;
        map[hero.y][hero.x] = '.';
    }

    void combat(Enemy e) {
        while (hero.isAlive() && e.isAlive()) {
            clearScreen();
            drawBox("БІЙ З " + e.name, { "Ворог готує атаку!", "Ваша черга діяти." });
            drawHPBar("ВАШЕ HP  ", hero.hp, hero.maxHp);
            drawHPBar("ВОРОГ HP ", e.hp, e.maxHp);
            cout << "\n [1] Атака | [2] Втеча\n Вибір: ";
            char choice; cin >> choice;

            if (choice == '1') {
                int pDmg = max(1, hero.atk - e.def + (rand() % 5));
                e.hp -= pDmg;
                cout << ">>> Ви завдали " << pDmg << " шкоди!\n";
                if (e.isAlive()) {
                    int eDmg = max(1, e.atk - hero.def + (rand() % 3));
                    hero.hp -= eDmg;
                    cout << ">>> Ворог вдарив вас на " << eDmg << "!\n";
                }
                this_thread::sleep_for(chrono::milliseconds(1000));
            } else {
                if (rand() % 10 < hero.luck) {
                    cout << "Ви успішно втекли!\n";
                    this_thread::sleep_for(chrono::seconds(1));
                    return;
                }
                cout << "Втеча не вдалася!\n";
                hero.hp -= e.atk / 2;
                this_thread::sleep_for(chrono::seconds(1));
            }
        }
        if (hero.isAlive()) {
            cout << "\nВорог подоланий! +40 XP\n";
            hero.addXP(40);
            hero.gold += 20;
            this_thread::sleep_for(chrono::seconds(1));
        }
    }

public:
    GameEngine() : currentLevel(1), running(true) {
        srand(time(0));
        generateLevel();
    }

    void draw() {
        clearScreen();
        cout << " === РІВЕНЬ " << currentLevel << " | ЗОЛОТО: " << hero.gold << " ===\n";
        drawHPBar("HP ГЕРОЯ", hero.hp, hero.maxHp);
        cout << endl;

        // Оновлення туману війни
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++)
                if (hero.y + i >= 0 && hero.y + i < map.size() && hero.x + j >= 0 && hero.x + j < map[0].size())
                    fog[hero.y + i][hero.x + j] = map[hero.y + i][hero.x + j];

        for (int i = 0; i < map.size(); i++) {
            cout << "  ";
            for (int j = 0; j < map[i].size(); j++) {
                if (i == hero.y && j == hero.x) cout << "@ ";
                else cout << fog[i][j] << " ";
            }
            cout << endl;
        }
        cout << "\n [w,a,s,d + Enter] - Рух | [q + Enter] - Вихід\n > ";
    }

    void update() {
        char move;
        if (!(cin >> move)) return;
        if (move == 'q') { running = false; return; }

        int nx = hero.x, ny = hero.y;
        if (move == 'w') ny--; if (move == 's') ny++;
        if (move == 'a') nx--; if (move == 'd') nx++;

        if (map[ny][nx] != '#') {
            hero.x = nx; hero.y = ny;
            char& cell = map[hero.y][hero.x];
            if (cell == 'E') { combat(Enemy::spawn(currentLevel)); cell = '.'; }
            else if (cell == '$') { hero.gold += 50; cout << "Знайдено золото!\n"; cell = '.'; this_thread::sleep_for(chrono::milliseconds(500)); }
            else if (cell == '~') { hero.hp = hero.maxHp; cout << "HP відновлено!\n"; cell = '.'; this_thread::sleep_for(chrono::milliseconds(500)); }
            else if (cell == '^') { hero.hp -= 20; cout << "Пастка! -20 HP\n"; cell = '.'; this_thread::sleep_for(chrono::milliseconds(500)); }
            else if (cell == 'X') { currentLevel++; generateLevel(); }
        }
    }

    bool isRunning() { return running && hero.isAlive(); }
    void showGameOver() {
        clearScreen();
        drawBox("ГРА ЗАВЕРШЕНА", { "Герой пав у підземеллі...", "Ваш рівень: " + to_string(hero.level), "Золото: " + to_string(hero.gold) });
    }
};

// =========================================================
// ГОЛОВНА ФУНКЦІЯ
// =========================================================

int main() {
    // Вмикаємо підтримку укр. мови в консолі Windows
    system("chcp 65001 > nul");

    clearScreen();
    drawBox("ПІДЗЕМЕЛЛЯ ДОЛІ", { "Вітаємо у світі пригод!", "Використовуйте WASD та ENTER для гри.", "", "Введіть будь-який символ для старту:" });
    char start; cin >> start;

    GameEngine game;
    while (game.isRunning()) {
        game.draw();
        game.update();
    }

    game.showGameOver();
    return 0;
}