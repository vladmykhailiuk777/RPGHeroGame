#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <thread>
#include <chrono>
using namespace std;

// =========================================================
// БАЗОВИЙ КЛАС: Entity
// Описує будь-яку живу істоту в грі — гравця чи ворога.
// Зберігає загальні характеристики: ім'я, HP, атаку, захист.
// =========================================================

class Entity {
public:
    string name;
    int hp, maxHp, atk, def;

    // Конструктор одразу задає всі базові параметри об'єкта.
    // maxHp і hp рівні на старті — персонаж починає з повним здоров'ям.
    Entity(string n, int h, int a, int d)
        : name(n), hp(h), maxHp(h), atk(a), def(d) {}

    // Проста перевірка: живий персонаж — той, у кого HP більше нуля.
    bool isAlive() const { return hp > 0; }
};

// =========================================================
// КЛАС: Item (Предмет)
// Описує будь-який предмет, який гравець може підібрати.
// Тип предмета визначає, що він робить при використанні.
// =========================================================

class Item {
public:
    string name;
    string type;    // "heal" — лікує, "atk" — підсилює атаку, "def" — підсилює захист
    int value;      // числове значення ефекту (скільки HP відновити, скільки атаки додати тощо)
    string symbol;  // символ для відображення в інвентарі

    Item() : name(""), type(""), value(0), symbol("?") {}
    Item(string n, string t, int v, string s)
        : name(n), type(t), value(v), symbol(s) {}
};

// =========================================================
// КЛАС: Player (Гравець)
// Наслідує Entity — отримує всі базові характеристики.
// Додає унікальні поля: позицію на карті, рівень, досвід,
// золото, удачу та інвентар предметів.
// =========================================================

class Player : public Entity {
public:
    int x, y;         // координати на карті
    int level, xp;    // рівень і поточний досвід
    int gold, luck;   // золото і шанс втечі від ворогів
    vector<Item> inventory;  // інвентар — список підібраних предметів

    Player()
        : Entity("Герой", 100, 20, 5),
          x(1), y(1), level(1), xp(0), gold(0), luck(5) {}

    // Додає досвід і перевіряє, чи досягнуто нового рівня.
    // При підвищенні рівня ростуть усі характеристики, HP повністю відновлюється.
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

    // Додає знайдений предмет до інвентаря, якщо там є місце.
    // Максимум 6 предметів — щоб інвентар не ставав нескінченним.
    bool pickItem(Item item) {
        if ((int)inventory.size() >= 6) return false;
        inventory.push_back(item);
        return true;
    }

    // Використовує предмет за індексом зі списку.
    // Залежно від типу — лікує, підсилює атаку або захист.
    // Після використання предмет видаляється з інвентаря.
    string useItem(int index) {
        if (index < 0 || index >= (int)inventory.size()) return "Невірний вибір.";
        Item& it = inventory[index];
        string result = "";
        if (it.type == "heal") {
            int healed = min(it.value, maxHp - hp); // не лікуємо вище максимуму
            hp += healed;
            result = "Відновлено " + to_string(healed) + " HP!";
        } else if (it.type == "atk") {
            atk += it.value;
            result = "Атака +" + to_string(it.value) + "!";
        } else if (it.type == "def") {
            def += it.value;
            result = "Захист +" + to_string(it.value) + "!";
        }
        inventory.erase(inventory.begin() + index); // видаляємо використаний предмет
        return result;
    }
};

// =========================================================
// КЛАС: Enemy (Ворог)
// Звичайний Entity з фабричним методом spawn().
// spawn() генерує ворога відповідно до поточного рівня гравця —
// чим далі гравець пройшов, тим сильніший противник.
// =========================================================

class Enemy : public Entity {
public:
    Enemy(string n, int h, int a, int d) : Entity(n, h, a, d) {}

    static Enemy spawn(int level) {
        if (level == 1) return Enemy("Скелет",       45,  12,  2);
        if (level == 2) return Enemy("Орк",           80,  18,  5);
        return             Enemy("Темний Лицар",     150,  28, 10);
    }
};

// =========================================================
// ФУНКЦІЇ ВІДОБРАЖЕННЯ
// Відповідають за весь візуальний інтерфейс гри:
// очищення екрану, рамки з текстом, шкала HP.
// =========================================================

// Очищає консоль за допомогою ANSI-коду — працює в більшості терміналів.
void clearScreen() {
    cout << "\033[2J\033[1;1H";
}

// Малює красиву рамку з заголовком і довільним списком рядків всередині.
// Довжина рамки фіксована — 55 символів. Кожен рядок доповнюється пробілами.
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

// Відображає горизонтальну шкалу HP символами # та -.
// Заповненість шкали пропорційна відношенню current/maxVal.
void drawHPBar(string label, int current, int maxVal) {
    int barWidth = 20;
    int filled = (int)(barWidth * (float)current / maxVal);
    cout << " " << label << " [";
    for (int i = 0; i < barWidth; i++) cout << (i < filled ? "#" : "-");
    cout << "] " << current << "/" << maxVal << "\n";
}

// =========================================================
// КЛАС: GameEngine (Ігровий рушій)
// Містить усю логіку гри: карту, тумани, бій, оновлення стану.
// Це головний клас, який зв'язує всі інші разом.
// =========================================================

class GameEngine {
private:
    vector<string> map;  // реальна карта з усіма об'єктами
    vector<string> fog;  // туман війни — те, що гравець вже бачив
    Player hero;
    int currentLevel;
    bool running;

    // Таблиця предметів, які можна знайти на карті.
    // Кожен Item описується назвою, типом ефекту, силою і символом на карті.
    vector<Item> itemTable = {
        Item("Зілля лікування",  "heal", 40, "P"),
        Item("Отрута ельфів",    "heal", 70, "E"),
        Item("Камінь сили",      "atk",   8, "S"),
        Item("Щит воїна",        "def",   6, "D"),
    };

    // Генерує новий рівень підземелля.
    // Спочатку вся карта заповнена стінами (#), потім вирізається відкритий простір.
    // Розміщуються вихід (X), вороги (E), золото ($), пастки (^) та предмети (I).
    void generateLevel() {
        int h = 8, w = 18;
        map.assign(h, string(w, '#'));
        fog.assign(h, string(w, '?'));

        // Вирізаємо прохідну зону всередині рамки зі стін
        for (int y = 1; y < h - 1; y++)
            for (int x = 1; x < w - 1; x++) map[y][x] = '.';

        map[h - 2][w - 2] = 'X'; // вихід на наступний рівень — завжди в правому нижньому куті

        // Розкидаємо об'єкти в довільних вільних клітинках
        char objects[] = { 'E', 'E', '$', '^', 'I', 'I' };
        for (char obj : objects) {
            int rx, ry;
            // шукаємо вільну клітинку, повторюємо поки не знайдемо порожню
            do { rx = rand() % (w - 2) + 1; ry = rand() % (h - 2) + 1; }
            while (map[ry][rx] != '.');
            map[ry][rx] = obj;
        }

        hero.x = 1; hero.y = 1; // гравець завжди стартує у лівому верхньому куті
    }

    // Показує вміст інвентаря і дозволяє використати будь-який предмет.
    // Якщо інвентар порожній — повідомляємо гравця і виходимо.
    void showInventory() {
        clearScreen();
        if (hero.inventory.empty()) {
            drawBox("ІНВЕНТАР", { "Інвентар порожній.", "", "Натисніть Enter..." });
            cin.ignore(); cin.get(); return;
        }

        // Формуємо список рядків для відображення в рамці
        vector<string> lines;
        lines.push_back("Оберіть предмет для використання:");
        lines.push_back("");
        for (int i = 0; i < (int)hero.inventory.size(); i++) {
            Item& it = hero.inventory[i];
            string line = "[" + to_string(i + 1) + "] " + it.name + " (+" + to_string(it.value) + ")";
            lines.push_back(line);
        }
        lines.push_back("");
        lines.push_back("[0] Повернутись");
        drawBox("ІНВЕНТАР", lines);

        cout << " Вибір: ";
        int choice; cin >> choice;
        if (choice == 0) return;

        // Використовуємо предмет і показуємо результат
        string result = hero.useItem(choice - 1);
        clearScreen();
        drawBox("ПРЕДМЕТ ВИКОРИСТАНО", { result, "", "Натисніть Enter..." });
        cin.ignore(); cin.get();
    }

    // Бойова система: покроковий бій між гравцем і ворогом.
    // Гравець обирає — атакувати або тікати. Ворог б'є у відповідь автоматично.
    // Бій триває поки хтось не загине або гравець не втече.
    void combat(Enemy e) {
        while (hero.isAlive() && e.isAlive()) {
            clearScreen();
            drawBox("БІЙ З " + e.name, {
                "Ворог стоїть перед вами!",
                "Атака: " + to_string(e.atk) + " | Захист: " + to_string(e.def)
            });
            drawHPBar("ВАШ HP   ", hero.hp, hero.maxHp);
            drawHPBar("ВОРОГ HP ", e.hp,    e.hp > 0 ? e.maxHp : 1);
            cout << "\n [1] Атака  [2] Інвентар  [3] Втеча\n Вибір: ";
            char choice; cin >> choice;

            if (choice == '1') {
                // Розрахунок шкоди: атака мінус захист + випадкова складова
                int pDmg = max(1, hero.atk - e.def + rand() % 5);
                e.hp -= pDmg;
                cout << ">>> Ви завдали " << pDmg << " шкоди!\n";

                // Якщо ворог ще живий — він б'є у відповідь
                if (e.isAlive()) {
                    int eDmg = max(1, e.atk - hero.def + rand() % 3);
                    hero.hp -= eDmg;
                    cout << ">>> Ворог вдарив вас на " << eDmg << "!\n";
                }
                this_thread::sleep_for(chrono::milliseconds(1000));

            } else if (choice == '2') {
                // Інвентар доступний прямо під час бою — можна випити зілля
                showInventory();

            } else if (choice == '3') {
                // Шанс втечі залежить від удачі гравця (0-9: luck < 5 = 50% шанс)
                if (rand() % 10 < hero.luck) {
                    cout << "Ви успішно втекли!\n";
                    this_thread::sleep_for(chrono::seconds(1));
                    return;
                }
                cout << "Втеча не вдалася! Ворог б'є у відповідь!\n";
                hero.hp -= e.atk / 2;
                this_thread::sleep_for(chrono::seconds(1));
            }
        }

        // Нагорода за перемогу в бою: досвід і золото
        if (hero.isAlive()) {
            cout << "\nВорог подоланий! +40 XP, +20 золота\n";
            hero.addXP(40);
            hero.gold += 20;
            this_thread::sleep_for(chrono::seconds(1));
        }
    }

    // Обробляє підбір предмета: вибирає випадковий зі списку itemTable
    // і намагається додати до інвентаря гравця.
    void pickupItem() {
        Item found = itemTable[rand() % itemTable.size()];
        if (hero.pickItem(found)) {
            drawBox("ЗНАХІДКА!", {
                "Ви знайшли: " + found.name,
                "Тип ефекту: +" + to_string(found.value),
                "",
                "Предмет додано до інвентаря."
            });
        } else {
            drawBox("ЗНАХІДКА!", {
                "Ви знайшли: " + found.name,
                "Але ваш інвентар повний (макс. 6)!",
                "Предмет залишено на місці."
            });
        }
        this_thread::sleep_for(chrono::milliseconds(1200));
    }

public:
    GameEngine() : currentLevel(1), running(true) {
        srand(time(0)); // ініціалізуємо генератор випадкових чисел поточним часом
        generateLevel();
    }

    // Малює поточний стан карти з туманом війни.
    // Гравець бачить лише клітинки поруч із собою (радіус 1).
    // Символ @ — це гравець, # — стіна, ? — ще не досліджена зона.
    void draw() {
        clearScreen();
        cout << " === РІВЕНЬ " << currentLevel
             << " | HP: " << hero.hp << "/" << hero.maxHp
             << " | ЗОЛОТО: " << hero.gold
             << " | РВЧ: " << hero.level
             << " | СУМКА: " << hero.inventory.size() << "/6 ===\n\n";

        // Відкриваємо туман навколо гравця у радіусі 1 клітинки
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++) {
                int ny = hero.y + i, nx = hero.x + j;
                if (ny >= 0 && ny < (int)map.size() && nx >= 0 && nx < (int)map[0].size())
                    fog[ny][nx] = map[ny][nx];
            }

        // Друкуємо карту: де стоїть герой — ставимо @, інакше — туман або відкрита клітинка
        for (int i = 0; i < (int)map.size(); i++) {
            cout << "  ";
            for (int j = 0; j < (int)map[i].size(); j++) {
                if (i == hero.y && j == hero.x) cout << "@ ";
                else cout << fog[i][j] << " ";
            }
            cout << "\n";
        }

        // Легенда символів карти
        cout << "\n Легенда: E=ворог  $=золото  ^=пастка  I=предмет  X=вихід\n";
        cout << " [w/a/s/d] Рух  [i] Інвентар  [q] Вихід\n > ";
    }

    // Читає команду гравця і оновлює стан гри.
    // Перевіряє клітинку, куди хоче зайти гравець, і викликає потрібну дію.
    void update() {
        char move; cin >> move;
        if (move == 'q') { running = false; return; }

        // Відкриваємо інвентар без зміни позиції
        if (move == 'i') { showInventory(); return; }

        // Розраховуємо нову позицію залежно від натиснутої клавіші
        int nx = hero.x, ny = hero.y;
        if (move == 'w') ny--;
        if (move == 's') ny++;
        if (move == 'a') nx--;
        if (move == 'd') nx++;

        // Перевіряємо: якщо нова клітинка не стіна — дозволяємо рух
        if (map[ny][nx] != '#') {
            hero.x = nx; hero.y = ny;
            char& cell = map[hero.y][hero.x]; // посилання на клітинку під гравцем

            if      (cell == 'E') { combat(Enemy::spawn(currentLevel)); cell = '.'; }
            else if (cell == '$') {
                hero.gold += 50;
                drawBox("ЗОЛОТО!", { "Ви знайшли 50 золота!", "Усього золота: " + to_string(hero.gold) });
                this_thread::sleep_for(chrono::milliseconds(800));
                cell = '.';
            }
            else if (cell == '~') {
                hero.hp = hero.maxHp;
                drawBox("ВІДНОВЛЕННЯ", { "HP повністю відновлено!" });
                this_thread::sleep_for(chrono::milliseconds(800));
                cell = '.';
            }
            else if (cell == '^') {
                hero.hp -= 20;
                drawBox("ПАСТКА!", { "Ви потрапили в пастку! -20 HP" });
                this_thread::sleep_for(chrono::milliseconds(800));
                cell = '.';
            }
            else if (cell == 'I') {
                // Підбір предмета — окрема функція з вибором випадкового Item
                clearScreen();
                pickupItem();
                cell = '.';
            }
            else if (cell == 'X') {
                // Вихід з рівня — генеруємо наступний
                currentLevel++;
                generateLevel();
            }
        }
    }

    bool isRunning() { return running && hero.isAlive(); }

    // Екран завершення гри — показує підсумкову статистику гравця.
    void showGameOver() {
        clearScreen();
        drawBox("ГРА ЗАВЕРШЕНА", {
            "Герой пав у підземеллі...",
            "",
            "Рівень: "  + to_string(hero.level),
            "Золото: "  + to_string(hero.gold),
            "XP:     "  + to_string(hero.xp),
        });
    }
};

// =========================================================
// ГОЛОВНА ФУНКЦІЯ
// main() залишається мінімальним: лише запуск і головний цикл.
// =========================================================

int main() {
    system("chcp 65001 > nul"); // підтримка кирилиці в Windows-терміналі

    clearScreen();
    drawBox("ПІДЗЕМЕЛЛЯ ДОЛІ", {
        "Вітаємо у світі пригод!",
        "",
        "WASD - рух        I - інвентар",
        "Q    - вийти      X - наступний рівень",
        "",
        "Введіть будь-який символ для старту:"
    });
    char start; cin >> start;

    GameEngine game;
    while (game.isRunning()) {
        game.draw();
        game.update();
    }

    game.showGameOver();
    return 0;
}
