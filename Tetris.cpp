
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <iostream>

class Tetris {
    static const std::uint32_t lines{ 20 };
    static const std::uint32_t cols{ 10 }; 
    static const std::uint32_t squares{ 4 }; 
    static const std::uint32_t shapes{ 7 }; 

    std::vector<std::vector<std::uint32_t>> area; // Grid representation
    std::vector<std::vector<std::uint32_t>> forms; // Tetromino shapes
    // Coordinates for tetromino blocks
    struct Coords {
        std::uint32_t x, y;
    } z[squares], k[squares];

    // SFML objects for rendering
    std::shared_ptr<sf::RenderWindow> window;
    sf::Texture tiles, bg;
    std::shared_ptr<sf::Sprite> sprite, background;
    sf::Clock clock;
    sf::Font font;
    sf::Text txtScore, txtGameOver;

    // SFML audio objects
    sf::Music backgroundMusic;
    sf::SoundBuffer gameOverBuffer;
    sf::Sound gameOverSound;

    int dirx, color, score;
    bool rotate, gameover;
    float timercount, delay;

protected:
    void events();
    void draw();
    void moveToDown();
    void setRotate();
    void resetValues();
    void changePosition();
    bool maxLimit();
    void setScore();

public:
    Tetris();
    void run();
};

Tetris::Tetris() {
    area.resize(lines); // Initialize grid rows
    for (std::size_t i{}; i < area.size(); ++i) {
        area[i].resize(cols); // Initialize grid columns
    }
    // Tetromino shapes defined by coordinates
    forms = {
      {1,3,5,7}, // I
      {2,4,5,7}, // Z
      {3,5,4,6}, // S
      {3,5,4,7}, // T
      {2,3,5,7}, // L
      {3,5,7,6}, // J
      {2,3,4,5}, // O
    };

    // Create game window
    window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode(360, 720),
        "Tetris",
        sf::Style::Titlebar | sf::Style::Close
    );
    window->setPosition(sf::Vector2i(100, 100));

    // Load tile textures
    tiles.loadFromFile("./squares.png");
    sprite = std::make_shared<sf::Sprite>();
    sprite->setTexture(tiles);
    sprite->setTextureRect(sf::IntRect(0, 0, 36, 36));

    // Load background texture
    bg.loadFromFile("./background.png");
    background = std::make_shared<sf::Sprite>();
    background->setTexture(bg);

    // Initialize game state variables
    dirx = score = { 0 };
    rotate = gameover = { false };
    timercount = { 0.f };
    delay = { 0.3f };
    color = { 1 };

    // Generate the first tetromino
    std::uint32_t number = std::rand() % shapes;
    for (std::size_t i{}; i < squares; ++i) {
        z[i].x = forms[number][i] % 2 + (cols / 2 - 1);
        z[i].y = forms[number][i] / 2;
    }

    // Load font and initialize score text
    font.loadFromFile("./font.ttf");
    txtScore.setFont(font);
    txtScore.setPosition(100.f, 10.f);
    txtScore.setString("SCORE: " + std::to_string(score));
    txtScore.setCharacterSize(30);
    txtScore.setOutlineThickness(3);

    // Initialize game over text
    txtGameOver.setFont(font);
    txtGameOver.setPosition(30.f, 300.f);
    txtGameOver.setString("GAME OVER");
    txtGameOver.setCharacterSize(50);
    txtGameOver.setOutlineThickness(3);

    // Load and play background music
    if (!backgroundMusic.openFromFile("./background_music.ogg")) {
        throw std::runtime_error("Failed to load background music.");
    }
    backgroundMusic.setLoop(true);
    backgroundMusic.play();

    // Load game over sound
    if (!gameOverBuffer.loadFromFile("./game_over.ogg")) {
        throw std::runtime_error("Failed to load game over sound.");
    }
    gameOverSound.setBuffer(gameOverBuffer);
}

void Tetris::events() {
    float time = clock.getElapsedTime().asSeconds();
    clock.restart();
    timercount += time;

    auto e = std::make_shared<sf::Event>();
    while (window->pollEvent(*e)) {
        if (e->type == sf::Event::Closed) {
            window->close();
        }
        if (e->type == sf::Event::KeyPressed) {
            if (e->key.code == sf::Keyboard::Up) {
                rotate = true;
            }
            else if (e->key.code == sf::Keyboard::Right) {
                ++dirx;
            }
            else if (e->key.code == sf::Keyboard::Left) {
                --dirx;
            }
            else if (e->key.code == sf::Keyboard::Enter && gameover) {
                // Reset game
                gameover = false;
                score = 0;
                txtScore.setString("SCORE: " + std::to_string(score));
                for (std::size_t i{}; i < lines; ++i) {
                    for (std::size_t j{}; j < cols; ++j) {
                        area[i][j] = 0;
                    }
                }
                color = std::rand() % shapes + 1;
                std::uint32_t number = std::rand() % shapes;
                for (std::size_t i{}; i < squares; ++i) {
                    z[i].x = forms[number][i] % 2 + (cols / 2 - 1);
                    z[i].y = forms[number][i] / 2;
                }

                // Restart background music
                backgroundMusic.play();
            }
            else if (e->key.code == sf::Keyboard::Space) {
                // Move block down
                while (!maxLimit()) {
                    for (std::size_t i{}; i < squares; ++i) {
                        k[i] = z[i];
                        ++z[i].y;
                    }
                }
                for (std::size_t i{}; i < squares; ++i) {
                    area[k[i].y][k[i].x] = color;
                }
                color = std::rand() % shapes + 1;
                std::uint32_t number = std::rand() % shapes;
                for (std::size_t i{}; i < squares; ++i) {
                    z[i].x = forms[number][i] % 2 + (cols / 2 - 1);
                    z[i].y = forms[number][i] / 2;
                }
            }
            else if (e->key.code == sf::Keyboard::Escape) {
                window->close();
            }
        }
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        delay = 0.05f;
    }
}

void Tetris::draw() {
    window->clear(sf::Color::Black);
    window->draw(*background);
    // Draw grid blocks
    for (std::size_t i{}; i < lines; ++i) {
        for (std::size_t j{}; j < cols; ++j) {
            if (area[i][j] != 0) {
                sprite->setTextureRect(sf::IntRect(area[i][j] * 36, 0, 36, 36));
                sprite->setPosition(j * 36, i * 36);
                window->draw(*sprite);
            }
        }
    }
    // Draw current tetromino
    for (std::size_t i{}; i < squares; ++i) {
        sprite->setTextureRect(sf::IntRect(color * 36, 0, 36, 36));
        sprite->setPosition(z[i].x * 36, z[i].y * 36);
        window->draw(*sprite);
    }
    window->draw(txtScore);
    if (gameover) {
        window->draw(txtGameOver);
    }
    window->display();
}

void Tetris::run() {
    while (window->isOpen()) {
        events();
        if (!gameover) {
            changePosition();
            setRotate();
            moveToDown();
            setScore();
            resetValues();
        }
        else {
            // Stop background music and play game over sound
            if (backgroundMusic.getStatus() == sf::Music::Playing) {
                backgroundMusic.stop();
                gameOverSound.play();
            }
        }
        draw();
    }
}

// Handles moving the tetromino down
void Tetris::moveToDown() {
    if (timercount > delay) {
        for (std::size_t i{}; i < squares; ++i) {
            k[i] = z[i];
            ++z[i].y;
        }
        if (maxLimit()) {
            for (std::size_t i{}; i < squares; ++i) {
                area[k[i].y][k[i].x] = color;
            }
            color = std::rand() % shapes + 1;
            std::uint32_t number = std::rand() % shapes;
            for (std::size_t i{}; i < squares; ++i) {
                z[i].x = forms[number][i] % 2 + (cols / 2 - 1);
                z[i].y = forms[number][i] / 2;
            }
        }
        timercount = 0;
    }
}

// Rotates the tetromino
void Tetris::setRotate() {
    if (rotate) {
        Coords coords = z[1];
        for (std::size_t i{}; i < squares; ++i) {
            int x = z[i].y - coords.y;
            int y = z[i].x - coords.x;

            z[i].x = coords.x - x;
            z[i].y = coords.y + y;
        }
        if (maxLimit()) {
            for (std::size_t i{}; i < squares; ++i) {
                z[i] = k[i];
            }
        }
    }
}

// Resets flags and delays
void Tetris::resetValues() {
    dirx = 0;
    rotate = false;
    delay = 0.3f;
}

// Handles horizontal movement
void Tetris::changePosition() {
    for (std::size_t i{}; i < squares; ++i) {
        k[i] = z[i];
        z[i].x += dirx;
    }

    if (maxLimit()) {
        for (std::size_t i{}; i < squares; ++i) {
            z[i] = k[i];
        }
    }
}

// Checks if the tetromino is out of bounds or colliding
bool Tetris::maxLimit() {
    for (std::size_t i{}; i < squares; ++i) {
        if (z[i].x < 0 ||
            z[i].x >= cols ||
            z[i].y >= lines ||
            area[z[i].y][z[i].x]) {
            return true;
        }
    }
    return false;
}

// Updates the score and clears filled lines
void Tetris::setScore() {
    std::uint32_t match = lines - 1;
    for (std::size_t i = match; i >= 1; --i) {
        std::uint32_t sum{};
        for (std::size_t j{}; j < cols; ++j) {
            if (area[i][j]) {
                if (i == 1) {
                    gameover = true;
                }
                ++sum;
            }
            area[match][j] = area[i][j];
        }
        if (sum < cols) {
            --match;
        }
        else {
            txtScore.setString("SCORE: " + std::to_string(++score));
        }
    }
}
// Entry point
int main(int argc, char** argv) {
    std::srand(std::time(0));
    try {
        auto tetris = std::make_shared<Tetris>();
        tetris->run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
