#pragma once

#include "core/math/Vec2.h"
#include "core/gfx/Canvas.h"

#include <array>

namespace spaceinvaders {

class Game;

struct Inputs {
    union {
        struct {
            uint8_t left : 1;
            uint8_t right : 1;
            uint8_t shoot : 1;
        };
        uint8_t keys = 0;
    };
};

struct Outputs {
    union {
        struct {
            uint8_t shoot : 1;
            uint8_t explosion: 1;
            uint8_t hit: 1;
        };
        uint8_t gates = 0;
    };
};

//-----------------------------------------------------------------------------
// Pool
//-----------------------------------------------------------------------------

template<typename T, size_t N>
class Pool {
public:
    size_t capacity() const {
        return N;
    }

    size_t size() const {
        size_t size = 0;
        for (const auto &item : _items) {
            if (item.used) {
                ++size;
            }
        }
        return size;
    }

    void reset() {
        for (auto &item : _items) {
            item.used = false;
        }
    }

    T *allocate() {
        for (auto &item : _items) {
            if (!item.used) {
                item.used = true;
                return &item.object;
            }
        }
        return nullptr;
    }

    template<typename Func>
    void forEach(Func func) {
        for (auto &item : _items) {
            if (item.used) {
                func(item.object);
            }
        }
    }

    template<typename Func>
    void forEachRemove(Func func) {
        for (auto &item : _items) {
            if (item.used) {
                if (func(item.object)) {
                    item.used = false;
                }
            }
        }
    }

private:
    struct Item {
        bool used = false;
        T object;
    };

    std::array<Item, N> _items;
};

//-----------------------------------------------------------------------------
// Player
//-----------------------------------------------------------------------------

class Player {
public:
    Player(Game &game);

    void reset();

    float x() const { return _x; }
    void setX(float x) { _x = x; }

    bool killed() const { return _killed; }
    void kill() { _killed = true; }

    int score() const { return _score; }
    void setScore(int score) { _score = score; }
    void addScore(int score) { _score += score; }

    int lives() const { return _lives; }
    void setLives(int lives) { _lives = lives; }

    bool shooting() const { return _shootTime > ShootInterval - ShootGate; }

    void update(float dt, Inputs &inputs);
    void draw(Canvas &canvas);

private:
    const float Speed = 80.f;
    const float ShootInterval = 0.4f;
    const float ShootGate = 0.05f;
    const float ShootVelocity = 120.f;

    Game &_game;
    float _x;
    bool _killed;
    int _score;
    int _lives;
    float _shootTime;
};

//-----------------------------------------------------------------------------
// Invader
//-----------------------------------------------------------------------------

class Invader {
public:
    enum Type {
        Type1 = 0,  // 10 points
        Type2 = 1,  // 20 points
        Type3 = 2,  // 30 points
    };

    float x() const { return _x; }
    float y() const { return _y; }
    Type type() const { return _type; }
    bool animFrame() const { return _animFrame; }

    void spawn(float x, float y, Type type);
    void setPosition(float x, float y) { _x = x; _y = y; }
    void setAnimFrame(bool frame) { _animFrame = frame; }

    void draw(Canvas &canvas);

private:
    float _x;
    float _y;
    Type _type;
    bool _animFrame;
};

//-----------------------------------------------------------------------------
// Projectile
//-----------------------------------------------------------------------------

class Projectile {
public:
    enum Type {
        Player,
        Invader,
    };

    float x() const { return _x; }
    float y() const { return _y; }
    Type type() const { return _type; }

    void kill() { _killed = true; }
    bool killed() const { return _killed; }

    void spawn(float x, float y, float velocity, Type type);

    void update(float dt);
    void draw(Canvas &canvas);

private:
    float _x;
    float _y;
    float _velocity;
    Type _type;
    bool _killed;
};

//-----------------------------------------------------------------------------
// Particle
//-----------------------------------------------------------------------------

class Particle {
public:
    void spawn(float x, float y, float vx, float vy, float lifeTime);

    bool killed() const { return _killed; }

    void update(float dt);
    void draw(Canvas &canvas);

private:
    float _x;
    float _y;
    float _vx;
    float _vy;
    float _time;
    float _lifeTime;
    bool _killed;
};

//-----------------------------------------------------------------------------
// Bunker
//-----------------------------------------------------------------------------

class Bunker {
public:
    void init(float x);
    bool checkHit(float x, float y);
    void draw(Canvas &canvas);

private:
    static constexpr int Width = 16;
    static constexpr int Height = 12;

    float _x;
    uint8_t _damage[Width * Height / 8];  // Bit field for damaged pixels
};

//-----------------------------------------------------------------------------
// Game
//-----------------------------------------------------------------------------

class Game {
public:
    enum State {
        Intro,
        Start,
        Play,
        Win,
        Lose,
    };

    Game();

    void init();
    void reset();
    void levelStart(int level);
    bool levelFinished();

    void setState(State state);

    void update(float dt, Inputs &inputs, Outputs &outputs);
    void draw(Canvas &canvas);

    void spawnPlayerProjectile(float x, float y);
    void spawnInvaderProjectile(float x, float y);
    void spawnExplosion(float x, float y);

private:
    void drawTexts(Canvas &canvas, const char *title, const char *msg);
    void drawHUD(Canvas &canvas);
    void drawShadowText(Canvas &canvas, int x, int y, Color color, const char *str);

    State _state;
    int _level;
    uint8_t _lastKeys = 0;
    float _time;

    Player _player;
    Pool<Invader, 55> _invaders;  // 11 columns x 5 rows
    Pool<Projectile, 16> _projectiles;
    Pool<Particle, 32> _particles;
    std::array<Bunker, 4> _bunkers;

    float _invaderDirection;
    float _invaderSpeed;
    float _invaderMoveTime;
    float _invaderShootTime;
    float _explosionTime;
    bool _invaderAnimFrame;
};

} // namespace spaceinvaders
