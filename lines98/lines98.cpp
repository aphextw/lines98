#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <array>
#include <queue>

using namespace std;
using namespace sf;

const int COLORS = 5;
const int SIZE = 9;
const int CELL_SIZE = 60;  // Размер клетки
const float BALL_RADIUS = 25.f;  // Новый радиус шара
const float OUTLINE_THICKNESS = 3.f;  // Толщина обводки

struct Ball {
    int color;
    bool exists = false;  // Шар существует по умолчанию
};

struct Point {
    int x, y;
};

using Board = std::array<std::array<Ball, SIZE>, SIZE>;

Board board;
Point selectedBall = { -1, -1 };  // Изначально нет выбранного шара

bool selectionCleared = false;  // Флаг для проверки, было ли выделение сброшено
bool mouseClicked = false;      // Флаг для отслеживания первого клика

// Функция для инициализации игрового поля
void initializeBoard() {
    for (auto& row : board) {
        for (auto& ball : row) {
            ball.exists = false;
        }
    }
}

// Функция для добавления случайных шаров
void addRandomBalls(int count) {
    int emptyCells = 0;
    for (const auto& row : board) {
        for (const auto& ball : row) {
            if (!ball.exists) emptyCells++;
        }
    }
    if (emptyCells < count) return;
    int added = 0;
    while (added < count) {
        int x = rand() % SIZE;
        int y = rand() % SIZE;
        if (!board[x][y].exists) {
            board[x][y] = { rand() % COLORS + 1, true };
            added++;
        }
    }
}

// Проверка доступности пути с помощью BFS
bool isPathAvailable(Point start, Point end) {
    if (!board[start.x][start.y].exists || board[end.x][end.y].exists) return false;

    array<array<bool, SIZE>, SIZE> visited = {};
    queue<Point> q;
    q.push(start);
    visited[start.x][start.y] = true;

    int dx[] = { 1, -1, 0, 0 };
    int dy[] = { 0, 0, 1, -1 };

    while (!q.empty()) {
        Point p = q.front();
        q.pop();

        if (p.x == end.x && p.y == end.y) return true;

        for (int i = 0; i < 4; ++i) {
            int nx = p.x + dx[i], ny = p.y + dy[i];
            if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && !visited[nx][ny] && !board[nx][ny].exists) {
                visited[nx][ny] = true;
                q.push({ nx, ny });
            }
        }
    }
    return false;
}

// Перемещение шара
bool moveBall(Point start, Point end) {
    if (start.x == end.x && start.y == end.y) {
        return false;  // Если выбранная позиция такая же, как начальная
    }

    // Проверка, что начальная клетка содержит шар, а целевая пуста
    if (!board[start.x][start.y].exists || board[end.x][end.y].exists) {
        return false;
    }

    // Проверяем, есть ли путь от начальной клетки к целевой
    if (!isPathAvailable(start, end)) {
        return false;  // Путь отсутствует
    }

    // Если путь есть, перемещаем шар
    board[end.x][end.y] = board[start.x][start.y];
    board[start.x][start.y] = { 0, false };  // Очищаем старую позицию
    return true;
}

// Проверка линий и удаление совпадений
bool checkLines() {
    bool foundLine = false;

    // Проверка горизонтальных и вертикальных линий
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j <= SIZE - 5; ++j) {
            // Горизонтальная линия
            if (board[i][j].exists &&
                board[i][j].color == board[i][j + 1].color &&
                board[i][j].color == board[i][j + 2].color &&
                board[i][j].color == board[i][j + 3].color &&
                board[i][j].color == board[i][j + 4].color) {
                for (int k = 0; k < 5; ++k) board[i][j + k] = { 0, false };
                foundLine = true;
            }

            // Вертикальная линия
            if (i <= SIZE - 5 && board[i][j].exists &&
                board[i][j].color == board[i + 1][j].color &&
                board[i][j].color == board[i + 2][j].color &&
                board[i][j].color == board[i + 3][j].color &&
                board[i][j].color == board[i + 4][j].color) {
                for (int k = 0; k < 5; ++k) board[i + k][j] = { 0, false };
                foundLine = true;
            }
        }
    }
    return foundLine;
}

// Функция для отрисовки игрового поля с использованием SFML
void drawBoardSFML(RenderWindow& window) {
    window.clear(Color(240, 240, 240));  // Легкий серо-бежевый фон для игры

    // Рисуем сетку с мягкими границами
    for (int i = 0; i <= SIZE; ++i) {
        // Горизонтальные линии
        Vertex line1[] = {
            Vertex(Vector2f(0, i * CELL_SIZE), Color(180, 180, 180)),
            Vertex(Vector2f(SIZE * CELL_SIZE, i * CELL_SIZE), Color(180, 180, 180))
        };
        window.draw(line1, 2, Lines);

        // Вертикальные линии
        Vertex line2[] = {
            Vertex(Vector2f(i * CELL_SIZE, 0), Color(180, 180, 180)),
            Vertex(Vector2f(i * CELL_SIZE, SIZE * CELL_SIZE), Color(180, 180, 180))
        };
        window.draw(line2, 2, Lines);
    }

    // Рисуем шары с красивыми градиентами
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j].exists) {
                CircleShape ball(BALL_RADIUS);
                ball.setPosition(j * CELL_SIZE + (CELL_SIZE - BALL_RADIUS * 2) / 2, i * CELL_SIZE + (CELL_SIZE - BALL_RADIUS * 2) / 2);  // Центрируем шар в клетке
                Color ballColor;
                switch (board[i][j].color) {
                case 1: ballColor = Color(255, 0, 0); break;    // Red
                case 2: ballColor = Color(0, 255, 0); break;    // Green
                case 3: ballColor = Color(0, 0, 255); break;    // Blue
                case 4: ballColor = Color(255, 255, 0); break;  // Yellow
                case 5: ballColor = Color(255, 0, 255); break;  // Magenta
                }

                ball.setFillColor(ballColor);
                ball.setOutlineColor(Color(0, 0, 0));  // Черная обводка
                ball.setOutlineThickness(OUTLINE_THICKNESS);
                ball.setPointCount(50); // Увеличиваем количество точек для плавности круга

                window.draw(ball);
            }
        }
    }

    // Если выбран шар, показываем его выделение с плавной анимацией
    if (selectedBall.x != -1 && selectedBall.y != -1) {
        RectangleShape selection(Vector2f(CELL_SIZE, CELL_SIZE));
        selection.setPosition(selectedBall.y * CELL_SIZE, selectedBall.x * CELL_SIZE);
        selection.setOutlineColor(Color::Cyan);
        selection.setOutlineThickness(5);
        selection.setFillColor(Color::Transparent);
        window.draw(selection);
    }

    window.display();
}

int main() {
    setlocale(LC_ALL, "Russian");
    srand(time(0));
    initializeBoard();
    addRandomBalls(5);

    RenderWindow window(VideoMode(SIZE * CELL_SIZE, SIZE * CELL_SIZE), "Lines98");

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }

        if (Mouse::isButtonPressed(Mouse::Left) && !mouseClicked) {
            Vector2i mousePos = Mouse::getPosition(window);
            int x = mousePos.y / CELL_SIZE; // Вертикальная координата (строка)
            int y = mousePos.x / CELL_SIZE; // Горизонтальная координата (столбец)

            // Проверяем, что клик внутри игрового поля
            if (x >= 0 && x < SIZE && y >= 0 && y < SIZE) {
                if (selectedBall.x == -1) {
                    // Если шар ещё не выбран
                    if (board[x][y].exists) {
                        selectedBall = { x, y }; // Выбираем шар
                        cout << "Выбран шар на позиции: (" << x << ", " << y << ")\n";
                    }
                }
                else {
                    // Если шар уже выбран, пробуем переместить
                    if (selectedBall.x == x && selectedBall.y == y && !selectionCleared) {
                        // Если снова кликнули на тот же шар, сбрасываем выделение
                        selectedBall = { -1, -1 };
                        selectionCleared = false;  // Сбрасываем флаг выделения
                        cout << "Выделение сброшено.\n";
                    }
                    else if (selectedBall.x != x || selectedBall.y != y) {
                        if (moveBall(selectedBall, { x, y })) {
                            cout << "Шар перемещён с (" << selectedBall.x << ", " << selectedBall.y
                                << ") на (" << x << ", " << y << ")\n";

                            // Проверка на линии
                            if (checkLines()) {
                                cout << "Линия удалена!\n";
                            }

                            addRandomBalls(2); // Добавляем новые шары

                            // Сбрасываем выделение после перемещения
                            selectedBall = { -1, -1 };
                            selectionCleared = false;  // Сбрасываем флаг выделения
                        }
                        else {
                            cout << "Невозможно переместить шар.\n";
                        }
                    }
                }
            }
            mouseClicked = true;  // Помечаем, что клик был обработан
        }

        // Если кнопка мыши отпущена, сбрасываем флаг
        if (!Mouse::isButtonPressed(Mouse::Left)) {
            mouseClicked = false;
        }

        // Отрисовка игрового поля
        drawBoardSFML(window);
    }

    return 0;
}
