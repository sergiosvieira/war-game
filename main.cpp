#include <iostream>
#include <raylib.h>
#include <vector>
#include <ctime>
#include <cmath>
#include <string>
#include <unordered_map>
#include <memory>

using namespace std;

using Images = std::unordered_map<std::string, Texture2D>;
using Musics = std::unordered_map<std::string, Music>;

struct Layer {
    Texture2D& texture;
    Vector2 position = {};
    Vector2 velocity = {};
};

using Parallax = std::vector<Layer>;

// Deixo os recursos do jogo centralizados
struct Resources {
    Musics musics;
    Images textures;
    void loadMusics() {
        musics = {
            {"sovietMenuMusic", LoadMusicStream("sovietTheme8bitBegin.mp3")},
            {"sovietOverTheme", LoadMusicStream("sovietTheme8bitEnd.mp3")},
            {"levelTheme", LoadMusicStream("attackGame.mp3")},
            {"internacionalTheme", LoadMusicStream("internacional8bitEndGame.mp3")}
        };
    }
    void loadTextures2D() {
        textures = {
            {"menuFace", LoadTexture("stalinFace.png")},
            {"overSymbol", LoadTexture("sovietSymbol.png")},
            {"leninStatue", LoadTexture("backLeninArt.png")},
            {"black-run", LoadTexture("gfx/black-run.png")},
            {"black-death", LoadTexture("gfx/black-death.png")},
            {"background", LoadTexture("gfx/bg.png")},
            {"layer1", LoadTexture("gfx/bg1.png")},
            {"layer2", LoadTexture("gfx/bg2.png")},
            {"layer3", LoadTexture("gfx/bg3.png")},
            {"layer4", LoadTexture("gfx/road.png")},
        };
    }
    Resources() {
        loadMusics();
        loadTextures2D();
    }
    ~Resources() {
        for (auto& [key, music]: musics) {
            UnloadMusicStream(music);
        }
        for (auto& [key, texture]: textures) {
            UnloadTexture(texture);
        }
    }
};

// Máquina de Estados
enum class GameState {
    MainTitle,
    Playing,
    Winning,
    GameOver
};

struct Utils {
    static bool timer(float currentTime,
                      float& lastTime,
                      float interval);
    static float center(const float f1,
                          const float f2);
};

float Utils::center(const float f1,
                    const float f2) {
    return (f1 - f2)/2.f;
}

bool Utils::timer(float currentTime,
                 float& lastTime,
                 float interval) {
    if (currentTime - lastTime > interval) {
        lastTime = currentTime;
        return true;
    }
    return false;
}


struct Constants {
    static constexpr float blinkIterval = 0.2f;
    static const int windowX = 708;
    static const int windowY = 640;
};


// Elementos do Jogo
struct Frame {
    float width = 0;
    float height = 0;
    int firstIndex = 0;
    int lastIndex = 0;
    int currentIndex = 0;
    float interval = 0.0f;
    float start = GetTime();
    void next() {
        if (currentIndex <= lastIndex) {
            float now = GetTime();
            if (Utils::timer(now, start, interval)) {
                start = now;
                ++currentIndex;
            }
        } else {
            currentIndex = firstIndex;
        }
    }
    Rectangle rect() const {
        float x = static_cast<float>(currentIndex * width);
        float y = 0;
        return {x, y, width, height};
    }
};

struct Renderer {
    float scale = 1.0;
    virtual void render(const Texture2D& texture,
                        const Vector2& position = {}){
        float width = static_cast<float>(texture.width);
        float height = static_cast<float>(texture.height);
        DrawTexturePro(texture,
                       {0, 0, width, height},
                       {position.x, position.y, width * scale, height * scale},
                       {}, {}, WHITE);
    };
};

struct AnimationRenderer: Renderer {
    shared_ptr<Frame> frame = nullptr;
    void setFrame(shared_ptr<Frame> frame) {
        this->frame = frame;
    }
    void render(const Texture2D& texture,
                const Vector2& position) override {
        if (frame == nullptr) return;
        float width = frame->width;
        float height =frame->height;
        DrawTexturePro(texture,
                       {frame->currentIndex * width, 0, width, height},
                       {position.x, position.y, width * scale, height * scale},
                       {}, {}, WHITE);
    };
};


struct Sprite2D {
    Texture2D &texture;
    Sprite2D(Texture2D& texture):
        texture(texture) {
    }
    float getWidth() const {
        return static_cast<float>(this->texture.width);
    }
    float getHeight() const {
        return static_cast<float>(this->texture.height);
    }
    Rectangle getRectangle() const {
        return {0.0, 0.0, getWidth(), getHeight()};
    }
};

struct AnimatedSprite2D: Sprite2D {
    shared_ptr<Frame> frame = nullptr;
    AnimatedSprite2D(Texture2D& texture) : Sprite2D(texture) {}
};

template <typename R>
struct Node {
    R renderer;
    virtual void render() = 0;
    virtual void update() {
    }
};

struct Player : Node<AnimationRenderer> {
    Vector2 position = {};
    Vector2 velocity = {0.0f, 5.0f};
    unique_ptr<AnimatedSprite2D> running = nullptr;
    Player(Resources& resources) :
        running(make_unique<AnimatedSprite2D>(resources.textures["black-run"])){
        running->frame = make_shared<Frame>();
        running->frame->width = 48;
        running->frame->height = 48;
        running->frame->firstIndex = 0;
        running->frame->lastIndex = 5;
        running->frame->currentIndex = 0;
        running->frame->interval = 0.15;
        running->frame->start = 0;
        renderer.setFrame(running->frame);
        renderer.scale = 2.5f;
        position.x = running->frame->width;
        position.y = Utils::center(Constants::windowY * 1.5, running->frame->height);
    }
    void move() {
        if (IsKeyDown(KEY_UP)) {
            position = {position.x + velocity.x, position.y - velocity.y};
        } else if (IsKeyDown(KEY_DOWN)) {
            position = {position.x + velocity.x, position.y + velocity.y};
        }
    }
    void update() override {
        running->frame->next();
        move();
    }
    void render() override {
        renderer.render(running->texture, position);
    }
};


struct Background: Node<Renderer> {
    Texture2D& background;
    Parallax layers;
    Background(Texture2D& texture,
               const Parallax& layers):
        background(texture),
        layers(layers) {
        renderer.scale = 2.6;
    }
    void update() override {
        for (auto& layer: layers) {
            layer.position = {layer.position.x + layer.velocity.x, layer.position.y};
        }
    }
    void render() override {
        renderer.render(background);
        for (auto& layer: layers) {
            renderer.render(layer.texture, layer.position);
            Vector2 position {
                layer.position.x + layer.texture.width * renderer.scale,
                layer.position.y
            };
            renderer.render(layer.texture, position);
            if (layer.position.x + layer.texture.width * renderer.scale <=0) {
                layer.position = {0, layer.position.y};
            }
        }
    }
};

struct Game {
    Game(Resources& resources) {
        initPlayer(resources);
    }
    void initPlayer(Resources& resources);
    void playThemeMusic(const Resources& resources);
    void mainTitle(const Resources& resources,
                          Color startColor);
    void update();
    void render();
    unique_ptr<Player> player = nullptr;
};

void Game::initPlayer(Resources& resources) {
    player = make_unique<Player>(resources);
}

void Game::update() {
    player->update();
}

void Game::render() {
    player->render();
}

void Game::playThemeMusic(const Resources& resources) {
    PlayMusicStream(resources.musics.at("sovietMenuTheme"));
    UpdateMusicStream(resources.musics.at("sovietMenuTheme"));
}

void Game::mainTitle(const Resources& resources,
                     Color startColor) {
    DrawTexture(resources.textures.at("menuFace"), 20, -70, RED);
    DrawText("Stalin.io", 200, 140, 80, WHITE);
    DrawText("Pressione Enter para começar!", 95, 330, 35, startColor);
    DrawText("CCCP, 19XX", 284, 600, 25, WHITE);
}

int main() {
    InitWindow(Constants::windowX, Constants::windowY, "Stalin.io");
    SetTargetFPS(60);
    InitAudioDevice();
    Resources* resources = new Resources();
    SetMusicVolume(resources->musics["sovietMenuTheme"], 0.4);
    Game game(*resources);
    game.playThemeMusic(*resources);
    GameState currentState = GameState::MainTitle;
    float start = GetTime();
    int counter = 0;

    Background background(resources->textures["background"], {
        {resources->textures["layer1"], {0, 0}, {-0.24, 0}},
        {resources->textures["layer2"], {0, 27}, {-0.5, 0}},
        {resources->textures["layer3"], {0, 146}, {-1.0, 0}},
        {resources->textures["layer4"], {0, 417}, {-1.5, 0}}
    });

    while (!WindowShouldClose()) {
        game.update();
        background.update();
        BeginDrawing();
        ClearBackground(BLACK);
        background.render();
        if (currentState == GameState::MainTitle) {
            bool changeColor = Utils::timer(GetTime(), start, Constants::blinkIterval);
            if (changeColor) ++counter;
            game.mainTitle(*resources, counter % 2 == 0 ? WHITE : BLACK);
            if (IsKeyPressed(KEY_ENTER)) {
                currentState = GameState::Playing;
            }
        } else if (currentState == GameState::Playing) {
            game.render();
        }
        EndDrawing();
    }
    delete resources;
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
