#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>

using namespace std;

/* 
 * =========================================================
 * КЛАС: Entity
 * Базовий клас для всіх істот у грі (гравець, вороги).
 * Зберігає спільні характеристики: ім'я, здоров'я та силу.
 * =========================================================
 */
class Entity {
public:
    string name;
    int hp, maxHp, atk, def;
    Entity(string n, int h, int a, int d) : name(n), hp(h), maxHp(h), atk(a), def(d) {}
    
    // Перевірка, чи істота ще жива
    bool isAlive() const { return hp > 0; }
};

/* 
 * =========================================================
 * КЛАС: Player
 * Розширює Entity. Додає систему рівнів (XP), золота та
 * координати гравця на мапі.
 * =========================================================
 */
class Player : public Entity {
public:
    int x, y, level, xp, gold, luck;
    Player() : Entity("Герой", 100, 20, 5), x(1), y(1), level(1), xp(0), gold(0), luck(5) {}

    /* 
     * Метод addXP:
     * Додає досвід та автоматично підвищує характеристики, 
     * якщо ліміт у 100 XP перевищено.
     */
    void addXP(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++;
            xp -= 100;
            maxHp += 20; 
            hp = maxHp; // Повне лікування при отриманні рівня
            atk += 5; 
            def += 2;
        }
    }
};

/* 
 * =========================================================
 * КЛАС: Enemy
 * Містить статичний метод для генерації ворогів залежно
 * від складності (поточного рівня підземелля).
 * =========================================================
 */
class Enemy : public Entity {
public:
    Enemy(string n, int h, int a, int d) : Entity(n, h, a, d) {}
    
    static Enemy spawn(int level) {
        if (level == 1) return Enemy("Скелет", 45, 12, 2);
        if (level == 2) return Enemy("Орк", 80, 18, 5);
        return Enemy("Темний Лицар", 150, 28, 10);
    }
};

/* 
 * =========================================================
 * СЕКЦІЯ: ІНТЕРФЕЙС (UI)
 * Функції для малювання гарних рамок та смужок здоров'я.
 * Використовуються спецсимволи для візуального стилю.
 * =========================================================
 */

void clearScreen() {
    // ANSI-код для повного очищення термінала
    cout << "\033[2J\033[1;1H";
}

void drawBox(string title, vector<string> lines) {
    int width = 50;
    // Малюємо верхню межу
    cout << "  ╔"; for(int i=0; i<width; i++) cout << "═"; cout << "╗\n";
    // Заголовок
    cout << "  ║ " << title;
    for (int i = 0; i < width - (int)title.length() - 1; i++) cout << " ";
    cout << "║\n  ╠"; for(int i=0; i<width; i++) cout << "═"; cout << "╣\n";
    // Вміст рядків
    for (const auto& line : lines) {
        cout << "  ║ " << line;
        for (int i = 0; i < width - (int)line.length() - 1; i++) cout << " ";
        cout << "║\n";
    }
    // Нижня межа
    cout << "  ╚"; for(int i=0; i<width; i++) cout << "═"; cout << "╝\n";
}

void drawHPBar(string label, int current, int maxVal, char symbol = '#') {
    int barWidth = 20;
    float ratio = (float)current / maxVal;
    int filled = (int)(barWidth * ratio);
    
    cout << "  " << label << " [";
    for (int i = 0; i < barWidth; ++i) {
        if (i < filled) cout << symbol;
        else cout << "-";
    }
    cout << "] " << current << "/" << maxVal << "\n";
}

/* 
 * =========================================================
 * КЛАС: GameEngine
 * Головний мозок гри. Керує мапою, боєм та переміщенням.
 * =========================================================
 */
class GameEngine {
private:
    vector<string> map;
    vector<string> fog; // Мапа "туману війни" (що бачить гравець)
    Player hero;
    int currentLevel;
    bool running;

    /* 
     * generateLevel:
     * Створює нову мапу, розставляє стіни, гравця, вихід (X)
     * та випадкові об'єкти (Вороги, Скрині, Пастки).
     */
    void generateLevel() {
        int h = 10, w = 20;
        map.assign(h, string(w, '#'));
        fog.assign(h, string(w, '?'));

        for (int y = 1; y < h - 1; y++)
            for (int x = 1; x < w - 1; x++) map[y][x] = '.';

        map[h - 2][w - 2] = 'X'; // Вихід на наступний рівень

        char items[] = { 'E', 'E', 'E', '$', '$', '~', '^' };
        for (char item : items) {
            int rx, ry;
            do { 
                rx = rand() % (w - 2) + 1; 
                ry = rand() % (h - 2) + 1; 
            } while (map[ry][rx] != '.');
            map[ry][rx] = item;
        }
        hero.x = 1; hero.y = 1;
        map[hero.y][hero.x] = '.'; // Початкова позиція гравця
    }

    /* 
     * combat:
     * Покроковий бій між гравцем та ворогом.
     * Розраховує шкоду на основі атаки та захисту.
     * Дозволяє гравцеві атакувати або спробувати втекти.
     */
    void combat(Enemy e) {
        while (hero.isAlive() && e.isAlive()) {
            clearScreen();
            drawBox("БІЙ: " + hero.name + " VS " + e.name, {
                "Ворог лютує! Оберіть свою наступну дію.",
                "Пам'ятайте: втеча не завжди вдається."
            });
            drawHPBar("ГЕРОЙ ", hero.hp, hero.maxHp, 'H');
            drawHPBar("ВОРОГ ", e.hp, e.maxHp, 'E');
            cout << "\n  [1] Атакувати  [2] Втекти\n  Ваш вибір >> ";
            
            char choice; cin >> choice;

            if (choice == '1') {
                // Розрахунок шкоди гравця
                int pDmg = max(1, hero.atk - e.def + (rand() % 5));
                e.hp -= pDmg;
                
                // Якщо ворог вижив, він атакує у відповідь
                if (e.isAlive()) {
                    int eDmg = max(1, e.atk - hero.def + (rand() % 3));
                    hero.hp -= eDmg;
                }
            } 
            else if (choice == '2') {
                // Перевірка на успіх втечі через удачу
                if (rand() % 10 < hero.luck) return;
                else {
                    // Провальна втеча - ворог атакує безкоштовно
                    int eDmg = max(1, e.atk - hero.def);
                    hero.hp -= eDmg;
                }
            }
        }

        if (hero.isAlive()) {
            hero.addXP(40); 
            hero.gold += 20;
            drawBox("ПЕРЕМОГА!", {"Ви здолали ворога!", "+40 XP отримано", "+20 золота знайдено"});
            cout << "  (Натисніть будь-яку клавішу + Enter, щоб продовжити)";
            char p; cin >> p;
        }
    }

public:
    GameEngine() : currentLevel(1), running(true) {
        srand(static_cast<unsigned int>(time(0)));
        generateLevel();
    }

    /* 
     * draw:
     * Відображає інтерфейс статусу та саму мапу.
     * Оновлює туман війни, щоб показувати лише сусідні клітинки.
     */
    void draw() {
        clearScreen();
        drawBox("СТАТУС ГЕРОЯ", {
            "Рівень: " + to_string(hero.level) + " | Досвід: " + to_string(hero.xp) + "/100",
            "Золото: " + to_string(hero.gold) + " | Глибина підземелля: " + to_string(currentLevel)
        });
        drawHPBar("ЗДОРОВ'Я", hero.hp, hero.maxHp, '#');
        cout << endl;

        // Оновлення туману війни навколо гравця (радіус 1)
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int py = hero.y + i, px = hero.x + j;
                if (py >= 0 && py < (int)map.size() && px >= 0 && px < (int)map[0].size())
                    fog[py][px] = map[py][px];
            }
        }

        // Малюємо мапу з відступом
        for (int i = 0; i < (int)map.size(); i++) {
            cout << "    ";
            for (int j = 0; j < (int)map[i].size(); j++) {
                if (i == hero.y && j == hero.x) cout << "@ "; // Гравець
                else cout << fog[i][j] << " ";
            }
            cout << endl;
        }
        cout << "\n  [W,A,S,D] Рух  [Q] Вихід з гри\n  Ваш хід >> ";
    }

    /* 
     * update:
     * Обробляє рух гравця та взаємодію з об'єктами на мапі.
     * Реагує на ворогів, скрині, джерела та пастки.
     */
    void update() {
        char move; cin >> move;
        if (move == 'q' || move == 'Q') { running = false; return; }

        int nx = hero.x, ny = hero.y;
        if (move == 'w' || move == 'W') ny--;
        else if (move == 's' || move == 'S') ny++;
        else if (move == 'a' || move == 'A') nx--;
        else if (move == 'd' || move == 'D') nx++;

        // Перевірка зіткнення зі стіною
        if (map[ny][nx] != '#') {
            hero.x = nx; hero.y = ny;
            char& cell = map[hero.y][hero.x];

            // Обробка типів клітинок
            if (cell == 'E') { 
                combat(Enemy::spawn(currentLevel)); 
                cell = '.'; 
            }
            else if (cell == '$') { 
                hero.gold += 50; 
                drawBox("СКАРБ!", {"Ви знайшли важку скриню!", "Всередині було 50 золотих монет."});
                char p; cin >> p; cell = '.'; 
            }
            else if (cell == '~') { 
                hero.hp = hero.maxHp; 
                drawBox("МАГІЧНЕ ДЖЕРЕЛО", {"Ви випили чистої води.", "Ваше здоров'я повністю відновлено!"});
                char p; cin >> p; cell = '.'; 
            }
            else if (cell == '^') { 
                hero.hp -= 20; 
                drawBox("ПАСТКА!", {"Ви наступили на приховані шипи!", "Це коштувало вам 20 одиниць здоров'я."});
                char p; cin >> p; cell = '.'; 
            }
            else if (cell == 'X') { 
                currentLevel++; 
                generateLevel(); 
            }
        }
    }

    // Перевірка стану гри
    bool isRunning() { return running && hero.isAlive(); }

    /* 
     * showGameOver:
     * Відображає фінальне вікно після смерті гравця.
     */
    void showGameOver() {
        clearScreen();
        drawBox("КІНЕЦЬ ГРИ", {
            "Ваша подорож обірвалася в темряві.",
            "Фінальний рівень: " + to_string(hero.level),
            "Зібране золото: " + to_string(hero.gold),
            "Спробуйте ще раз!"
        });
    }
};

// =========================================================
// ГОЛОВНА ФУНКЦІЯ: main
// Точка входу в програму.
// =========================================================
int main() {
    // Встановлення кодування UTF-8 для коректного відображення в Windows
    system("chcp 65001 > nul"); 
    
    clearScreen();
    drawBox("ПІДЗЕМЕЛЛЯ ДОЛІ", {
        "Вітаємо, Герою!",
        "Твоя мета - пробратися крізь небезпеку.",
        "Керування: Натисніть клавішу, потім ENTER."
    });
    cout << "  Введіть будь-який символ, щоб почати пригоду: ";
    char s; cin >> s;

    GameEngine game;
    while (game.isRunning()) {
        game.draw();
        game.update();
    }

    game.showGameOver();
    return 0;
}
