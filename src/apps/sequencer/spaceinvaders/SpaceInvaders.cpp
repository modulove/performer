#include "SpaceInvaders.h"

#include "core/utils/Random.h"
#include "core/utils/StringBuilder.h"
#include "core/math/Math.h"

#include <cstring>
#include <cmath>

namespace spaceinvaders {

static Random rng(12345);  // Random number generator

static const float ScreenWidth = 256.f;
static const float ScreenHeight = 64.f;
static const float PlayerY = 56.f;
static const float PlayerWidth = 11.f;
static const float PlayerHeight = 7.f;
static const float InvaderWidth = 8.f;
static const float InvaderHeight = 8.f;
static const float InvaderStartY = 12.f;  // Start just below HUD
static const float InvaderSpacingX = 12.f;
static const float InvaderSpacingY = 5.f;  // Tighter vertical spacing for more play room

//-----------------------------------------------------------------------------
// Player
//-----------------------------------------------------------------------------

Player::Player(Game &game) :
    _game(game)
{
}

void Player::reset() {
    _x = ScreenWidth / 2.f;
    _killed = false;
    _score = 0;
    _lives = 3;
    _shootTime = 0.f;
}

void Player::update(float dt, Inputs &inputs) {
    if (_killed) {
        return;
    }

    // Movement
    if (inputs.left) {
        _x -= Speed * dt;
    }
    if (inputs.right) {
        _x += Speed * dt;
    }

    _x = clamp(_x, PlayerWidth / 2.f, ScreenWidth - PlayerWidth / 2.f);

    // Shooting
    _shootTime += dt;
    if (inputs.shoot && _shootTime >= ShootInterval) {
        _shootTime = 0.f;
        _game.spawnPlayerProjectile(_x, PlayerY - PlayerHeight / 2.f);
    }
}

void Player::draw(Canvas &canvas) {
    if (_killed) {
        return;
    }

    int cx = int(_x - PlayerWidth / 2.f);
    int cy = int(PlayerY - PlayerHeight / 2.f);

    canvas.setColor(Color::Bright);

    // Player ship (simple tank-like shape)
    // Turret
    canvas.fillRect(cx + 5, cy, 1, 3);
    // Body
    canvas.fillRect(cx + 1, cy + 3, 9, 2);
    // Base
    canvas.fillRect(cx, cy + 5, 11, 2);
}

//-----------------------------------------------------------------------------
// Invader
//-----------------------------------------------------------------------------

void Invader::spawn(float x, float y, Type type) {
    _x = x;
    _y = y;
    _type = type;
    _animFrame = false;
}

void Invader::draw(Canvas &canvas) {
    int cx = int(_x);
    int cy = int(_y);

    canvas.setColor(Color::Bright);

    // Draw different invader types using the sprites from startup screen
    static const uint8_t invader0[][2] = {
        {0b00011000, 0b00111100},
        {0b00111100, 0b01111110},
        {0b01111110, 0b11011011},
        {0b11011011, 0b11111111},
        {0b11111111, 0b01111110},
        {0b00100100, 0b00100100},
        {0b01011010, 0b01000010},
        {0b10100101, 0b10000001}
    };

    static const uint8_t invader1[][2] = {
        {0b00100100, 0b00100100},
        {0b00100100, 0b01011010},
        {0b01111110, 0b11111111},
        {0b11011011, 0b10111101},
        {0b11111111, 0b10111101},
        {0b11111111, 0b11111111},
        {0b10100101, 0b10100101},
        {0b00100100, 0b01011010}
    };

    static const uint8_t invader2[][2] = {
        {0b00111100, 0b00111100},
        {0b01111110, 0b01111110},
        {0b11011011, 0b11011011},
        {0b11111111, 0b11111111},
        {0b01111110, 0b01111110},
        {0b00100100, 0b00100100},
        {0b01000010, 0b10000001},
        {0b10000001, 0b01000010}
    };

    const uint8_t (*pattern)[2];
    switch (_type) {
        case Type1: pattern = invader0; break;
        case Type2: pattern = invader1; break;
        case Type3: pattern = invader2; break;
        default: pattern = invader0; break;
    }

    int frame = _animFrame ? 1 : 0;

    // Draw 8x8 sprite
    for (int row = 0; row < 8; ++row) {
        uint8_t rowData = pattern[row][frame];
        for (int col = 0; col < 8; ++col) {
            if (rowData & (1 << (7 - col))) {
                canvas.fillRect(cx + col, cy + row, 1, 1);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Projectile
//-----------------------------------------------------------------------------

void Projectile::spawn(float x, float y, float velocity, Type type) {
    _x = x;
    _y = y;
    _velocity = velocity;
    _type = type;
    _killed = false;
}

void Projectile::update(float dt) {
    _y += _velocity * dt;

    // Kill if off screen
    if (_y < 0.f || _y > ScreenHeight) {
        _killed = true;
    }
}

void Projectile::draw(Canvas &canvas) {
    canvas.setColor(Color::Bright);
    canvas.fillRect(int(_x), int(_y), 1, 3);
}

//-----------------------------------------------------------------------------
// Particle
//-----------------------------------------------------------------------------

void Particle::spawn(float x, float y, float vx, float vy, float lifeTime) {
    _x = x;
    _y = y;
    _vx = vx;
    _vy = vy;
    _time = 0.f;
    _lifeTime = lifeTime;
    _killed = false;
}

void Particle::update(float dt) {
    _time += dt;
    if (_time >= _lifeTime) {
        _killed = true;
        return;
    }

    _x += _vx * dt;
    _y += _vy * dt;
}

void Particle::draw(Canvas &canvas) {
    float alpha = 1.f - (_time / _lifeTime);
    Color color = alpha > 0.5f ? Color::Bright : Color::Medium;
    canvas.setColor(color);
    canvas.fillRect(int(_x), int(_y), 1, 1);
}

//-----------------------------------------------------------------------------
// Bunker
//-----------------------------------------------------------------------------

void Bunker::init(float x) {
    _x = x;
    std::memset(_damage, 0, sizeof(_damage));
}

bool Bunker::checkHit(float x, float y) {
    int bx = int(x - _x);
    int by = int(y - 44.f);  // Bunker Y position

    if (bx < 0 || bx >= Width || by < 0 || by >= Height) {
        return false;
    }

    int bitIndex = by * Width + bx;
    int byteIndex = bitIndex / 8;
    int bitOffset = bitIndex % 8;

    // Check if already damaged
    if (_damage[byteIndex] & (1 << bitOffset)) {
        return false;
    }

    // Mark as damaged
    _damage[byteIndex] |= (1 << bitOffset);

    // Damage surrounding pixels
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = bx + dx;
            int ny = by + dy;
            if (nx >= 0 && nx < Width && ny >= 0 && ny < Height) {
                int nBitIndex = ny * Width + nx;
                int nByteIndex = nBitIndex / 8;
                int nBitOffset = nBitIndex % 8;
                _damage[nByteIndex] |= (1 << nBitOffset);
            }
        }
    }

    return true;
}

void Bunker::draw(Canvas &canvas) {
    canvas.setColor(Color::Bright);

    // Draw bunker shape (dome)
    for (int y = 0; y < Height; ++y) {
        for (int x = 0; x < Width; ++x) {
            // Dome shape
            int cx = x - Width / 2;
            int cy = y;
            if (cx * cx + cy * cy * 2 < Width * Width / 4 && cy < Height - 2) {
                // Check if damaged
                int bitIndex = y * Width + x;
                int byteIndex = bitIndex / 8;
                int bitOffset = bitIndex % 8;

                if (!(_damage[byteIndex] & (1 << bitOffset))) {
                    canvas.fillRect(int(_x + x), int(44.f + y), 1, 1);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Game
//-----------------------------------------------------------------------------

Game::Game() :
    _player(*this)
{
}

void Game::init() {
    _state = State::Intro;
    _level = 1;
    reset();
}

void Game::reset() {
    _time = 0.f;
    _player.reset();
    _invaders.reset();
    _projectiles.reset();
    _particles.reset();

    _invaderDirection = 1.f;
    _invaderSpeed = 10.f;
    _invaderMoveTime = 0.f;
    _invaderShootTime = 0.f;
    _explosionTime = 0.f;
    _invaderAnimFrame = false;

    // Initialize bunkers
    for (int i = 0; i < 4; ++i) {
        _bunkers[i].init(40.f + i * 56.f);
    }
}

void Game::levelStart(int level) {
    _level = level;
    _invaders.reset();
    _projectiles.reset();
    _particles.reset();

    _invaderDirection = 1.f;
    _invaderSpeed = 10.f + level * 2.f;
    _invaderMoveTime = 0.f;
    _invaderShootTime = 0.f;
    _invaderAnimFrame = false;

    // Spawn invaders in formation (11 columns x 5 rows)
    float startX = (ScreenWidth - (11 * InvaderSpacingX)) / 2.f;

    for (int row = 0; row < 5; ++row) {
        Invader::Type type;
        if (row == 0) type = Invader::Type3;  // Top row - 30 pts
        else if (row <= 2) type = Invader::Type2;  // Middle rows - 20 pts
        else type = Invader::Type1;  // Bottom rows - 10 pts

        for (int col = 0; col < 11; ++col) {
            auto invader = _invaders.allocate();
            if (invader) {
                float x = startX + col * InvaderSpacingX;
                float y = InvaderStartY + row * InvaderSpacingY;
                invader->spawn(x, y, type);
            }
        }
    }
}

bool Game::levelFinished() {
    return _invaders.size() == 0;
}

void Game::setState(State state) {
    _state = state;
    _time = 0.f;
}

void Game::update(float dt, Inputs &inputs, Outputs &outputs) {
    _time += dt;

    outputs.gates = 0;

    // Handle state transitions
    switch (_state) {
        case State::Intro:
            if (_time > 2.f || (inputs.keys && !_lastKeys)) {
                setState(State::Start);
            }
            break;

        case State::Start:
            if (_time > 2.f || (inputs.keys && !_lastKeys)) {
                reset();
                levelStart(_level);
                setState(State::Play);
            }
            break;

        case State::Play:
            // Update player
            _player.update(dt, inputs);
            if (_player.shooting()) {
                outputs.shoot = 1;
            }

            // Update invaders
            _invaderMoveTime += dt;
            if (_invaderMoveTime >= 1.f / _invaderSpeed) {
                _invaderMoveTime = 0.f;
                _invaderAnimFrame = !_invaderAnimFrame;

                bool moveDown = false;
                float minX = ScreenWidth;
                float maxX = 0.f;

                // Find extents
                _invaders.forEach([&](Invader &invader) {
                    minX = std::min(minX, invader.x());
                    maxX = std::max(maxX, invader.x());
                });

                // Check if need to reverse direction
                if ((maxX >= ScreenWidth - InvaderWidth && _invaderDirection > 0.f) ||
                    (minX <= InvaderWidth && _invaderDirection < 0.f)) {
                    _invaderDirection = -_invaderDirection;
                    moveDown = true;
                }

                // Move all invaders
                _invaders.forEach([&](Invader &invader) {
                    float newX = invader.x() + _invaderDirection * 2.f;
                    float newY = moveDown ? invader.y() + InvaderHeight : invader.y();
                    invader.setPosition(newX, newY);
                    invader.setAnimFrame(_invaderAnimFrame);

                    // Check if invaders reached player
                    if (newY >= PlayerY - PlayerHeight) {
                        _player.kill();
                    }
                });
            }

            // Invaders shoot randomly
            _invaderShootTime += dt;
            if (_invaderShootTime >= 1.f && _invaders.size() > 0) {
                _invaderShootTime = 0.f;

                // Pick random invader from bottom row
                int invaderCount = 0;
                _invaders.forEach([&](Invader &) { invaderCount++; });

                if (invaderCount > 0) {
                    int target = rng.nextRange(invaderCount);
                    int current = 0;
                    _invaders.forEach([&](Invader &invader) {
                        if (current++ == target) {
                            spawnInvaderProjectile(invader.x(), invader.y());
                        }
                    });
                }
            }

            // Update projectiles
            _projectiles.forEachRemove([&](Projectile &projectile) {
                projectile.update(dt);

                if (projectile.killed()) {
                    return true;
                }

                // Check collisions
                if (projectile.type() == Projectile::Player) {
                    // Check invader hits
                    bool hit = false;
                    _invaders.forEachRemove([&](Invader &invader) {
                        float dx = projectile.x() - invader.x();
                        float dy = projectile.y() - invader.y();
                        if (std::abs(dx) < InvaderWidth / 2.f && std::abs(dy) < InvaderHeight / 2.f) {
                            // Hit!
                            int points = (invader.type() == Invader::Type3) ? 30 :
                                        (invader.type() == Invader::Type2) ? 20 : 10;
                            _player.addScore(points);
                            spawnExplosion(invader.x(), invader.y());
                            projectile.kill();
                            outputs.explosion = 1;
                            hit = true;
                            return true;  // Remove invader
                        }
                        return false;
                    });

                    if (hit) {
                        return true;
                    }

                    // Check bunker hits
                    for (auto &bunker : _bunkers) {
                        if (bunker.checkHit(projectile.x(), projectile.y())) {
                            projectile.kill();
                            outputs.hit = 1;
                            return true;
                        }
                    }
                } else {
                    // Invader projectile
                    // Check player hit
                    float dx = projectile.x() - _player.x();
                    float dy = projectile.y() - PlayerY;
                    if (std::abs(dx) < PlayerWidth / 2.f && std::abs(dy) < PlayerHeight / 2.f && !_player.killed()) {
                        _player.kill();
                        spawnExplosion(_player.x(), PlayerY);
                        projectile.kill();
                        outputs.explosion = 1;
                        return true;
                    }

                    // Check bunker hits
                    for (auto &bunker : _bunkers) {
                        if (bunker.checkHit(projectile.x(), projectile.y())) {
                            projectile.kill();
                            return true;
                        }
                    }
                }

                return projectile.killed();
            });

            // Update particles
            _particles.forEachRemove([](Particle &particle) {
                particle.update(0.016f);
                return particle.killed();
            });

            // Update explosion time
            if (_explosionTime > 0.f) {
                _explosionTime -= dt;
                outputs.explosion = 1;
            }

            // Check win/lose conditions
            if (levelFinished()) {
                _level++;
                setState(State::Win);
            } else if (_player.killed()) {
                int lives = _player.lives() - 1;
                _player.setLives(lives);
                if (lives <= 0) {
                    setState(State::Lose);
                } else {
                    // Respawn player
                    _player.reset();
                    _player.setScore(_player.score());  // Keep score
                    _player.setLives(lives);
                }
            }
            break;

        case State::Win:
            if (_time > 2.f) {
                levelStart(_level);
                setState(State::Play);
            }
            break;

        case State::Lose:
            if (_time > 3.f || (inputs.keys && !_lastKeys)) {
                setState(State::Start);
            }
            break;
    }

    _lastKeys = inputs.keys;
}

void Game::draw(Canvas &canvas) {
    canvas.setColor(Color::None);
    canvas.fill();

    switch (_state) {
        case State::Intro:
            drawTexts(canvas, "SPACE INVADERS", "PRESS ANY KEY");
            break;

        case State::Start:
            drawTexts(canvas, "GET READY!", nullptr);
            break;

        case State::Play:
            // Draw bunkers
            for (auto &bunker : _bunkers) {
                bunker.draw(canvas);
            }

            // Draw player
            _player.draw(canvas);

            // Draw invaders
            _invaders.forEach([&](Invader &invader) {
                invader.draw(canvas);
            });

            // Draw projectiles
            _projectiles.forEach([&](Projectile &projectile) {
                projectile.draw(canvas);
            });

            // Draw particles
            _particles.forEach([&](Particle &particle) {
                particle.draw(canvas);
            });

            // Draw HUD
            drawHUD(canvas);
            break;

        case State::Win:
            drawTexts(canvas, "LEVEL COMPLETE!", nullptr);
            drawHUD(canvas);
            break;

        case State::Lose:
            drawTexts(canvas, "GAME OVER", nullptr);
            drawHUD(canvas);
            break;
    }
}

void Game::spawnPlayerProjectile(float x, float y) {
    auto projectile = _projectiles.allocate();
    if (projectile) {
        projectile->spawn(x, y, -120.f, Projectile::Player);
    }
}

void Game::spawnInvaderProjectile(float x, float y) {
    auto projectile = _projectiles.allocate();
    if (projectile) {
        projectile->spawn(x, y, 60.f, Projectile::Invader);
    }
}

void Game::spawnExplosion(float x, float y) {
    _explosionTime = 0.2f;

    // Spawn explosion particles
    for (int i = 0; i < 8; ++i) {
        auto particle = _particles.allocate();
        if (particle) {
            float angle = (i / 8.f) * 2.f * M_PI;
            float speed = 20.f + rng.nextFloat() * 20.f;  // Range 20-40
            float vx = std::cos(angle) * speed;
            float vy = std::sin(angle) * speed;
            particle->spawn(x, y, vx, vy, 0.5f);
        }
    }
}

void Game::drawTexts(Canvas &canvas, const char *title, const char *msg) {
    if (title) {
        canvas.setFont(Font::Small);
        drawShadowText(canvas, (ScreenWidth - canvas.textWidth(title)) / 2, 20, Color::Bright, title);
    }
    if (msg) {
        canvas.setFont(Font::Tiny);
        drawShadowText(canvas, (ScreenWidth - canvas.textWidth(msg)) / 2, 40, Color::Medium, msg);
    }
}

void Game::drawHUD(Canvas &canvas) {
    canvas.setFont(Font::Tiny);

    // Score (left side)
    FixedStringBuilder<16> scoreStr("%d", _player.score());
    drawShadowText(canvas, 2, 8, Color::Bright, scoreStr);

    // Level (right side)
    FixedStringBuilder<16> levelStr("L%d", _level);
    drawShadowText(canvas, ScreenWidth - canvas.textWidth(levelStr) - 2, 8, Color::Bright, levelStr);

    // Lives (bottom right)
    canvas.setColor(Color::Medium);
    for (int i = 0; i < _player.lives(); ++i) {
        canvas.fillRect(int(ScreenWidth - 12 - i * 8), 56, 6, 4);
    }
}

void Game::drawShadowText(Canvas &canvas, int x, int y, Color color, const char *str) {
    canvas.setColor(Color::Low);
    canvas.drawText(x + 1, y + 1, str);
    canvas.setColor(color);
    canvas.drawText(x, y, str);
}

} // namespace spaceinvaders
