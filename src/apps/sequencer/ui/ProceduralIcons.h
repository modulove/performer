#pragma once

#include "../../../../core/gfx/Canvas.h"

/**
 * POW|FORMER Procedural Icon Library
 *
 * Memory-efficient icons drawn algorithmically instead of using bitmaps.
 * Each icon is 8x8, 12x12, or 16x16 pixels and uses ~20-100 bytes of code
 * instead of 8-32 bytes of bitmap data per icon.
 *
 * Benefits:
 * - Scalable to any size
 * - No bitmap storage required
 * - Can be animated/modified easily
 * - Consistent visual style
 */

namespace Icons {

// ============================================
// PLAYBACK CONTROLS (8x8)
// ============================================

static void play(Canvas &canvas, int x, int y, int size = 8) {
    // Right-pointing triangle
    float scale = size / 8.0f;
    int x1 = x + 2*scale;
    int y1 = y + 1*scale;
    int x2 = x + 6*scale;
    int y2 = y + 4*scale;
    int y3 = y + 7*scale;

    // Draw filled triangle
    for (int dy = 0; dy < 6*scale; dy++) {
        int width = (dy * 4*scale) / (6*scale);
        canvas.hline(x1, y1 + dy, width);
    }
}

static void pause(Canvas &canvas, int x, int y, int size = 8) {
    // Two vertical bars
    float scale = size / 8.0f;
    canvas.fillRect(x + 2*scale, y + 2*scale, 2*scale, 4*scale);
    canvas.fillRect(x + 5*scale, y + 2*scale, 2*scale, 4*scale);
}

static void stop(Canvas &canvas, int x, int y, int size = 8) {
    // Square
    float scale = size / 8.0f;
    canvas.fillRect(x + 2*scale, y + 2*scale, 4*scale, 4*scale);
}

static void record(Canvas &canvas, int x, int y, int size = 8) {
    // Filled circle
    float scale = size / 8.0f;
    int centerX = x + 4*scale;
    int centerY = y + 4*scale;
    int radius = 2*scale;

    for (int dy = -radius; dy <= radius; dy++) {
        int dx = radius - abs(dy);
        canvas.hline(centerX - dx, centerY + dy, 2*dx);
    }
}

static void rewind(Canvas &canvas, int x, int y, int size = 8) {
    // Two left-pointing triangles
    float scale = size / 8.0f;

    // First triangle
    for (int dy = 0; dy < 6*scale; dy++) {
        int width = ((6*scale - dy) * 3*scale) / (6*scale);
        canvas.hline(x + 1*scale, y + 1*scale + dy, width);
    }

    // Second triangle
    for (int dy = 0; dy < 6*scale; dy++) {
        int width = ((6*scale - dy) * 3*scale) / (6*scale);
        canvas.hline(x + 4*scale, y + 1*scale + dy, width);
    }
}

static void fastForward(Canvas &canvas, int x, int y, int size = 8) {
    // Two right-pointing triangles
    float scale = size / 8.0f;

    // First triangle
    for (int dy = 0; dy < 6*scale; dy++) {
        int width = (dy * 3*scale) / (6*scale);
        canvas.hline(x + 1*scale, y + 1*scale + dy, width);
    }

    // Second triangle
    for (int dy = 0; dy < 6*scale; dy++) {
        int width = (dy * 3*scale) / (6*scale);
        canvas.hline(x + 4*scale, y + 1*scale + dy, width);
    }
}

// ============================================
// AUDIO CONTROLS (8x8)
// ============================================

static void mute(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Speaker
    canvas.fillRect(x + 1*scale, y + 3*scale, 2*scale, 2*scale);
    canvas.fillRect(x + 3*scale, y + 2*scale, 1*scale, 4*scale);
    // X symbol
    canvas.line(x + 5*scale, y + 2*scale, x + 7*scale, y + 6*scale);
    canvas.line(x + 7*scale, y + 2*scale, x + 5*scale, y + 6*scale);
}

static void volumeUp(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Speaker
    canvas.fillRect(x + 1*scale, y + 3*scale, 2*scale, 2*scale);
    canvas.fillRect(x + 3*scale, y + 2*scale, 1*scale, 4*scale);
    // Sound waves
    canvas.line(x + 5*scale, y + 2*scale, x + 5*scale, y + 6*scale);
    canvas.line(x + 6*scale, y + 1*scale, x + 6*scale, y + 7*scale);
}

static void volumeDown(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Speaker
    canvas.fillRect(x + 1*scale, y + 3*scale, 2*scale, 2*scale);
    canvas.fillRect(x + 3*scale, y + 2*scale, 1*scale, 4*scale);
    // Single wave
    canvas.line(x + 5*scale, y + 2*scale, x + 5*scale, y + 6*scale);
}

// ============================================
// SEQUENCER FUNCTIONS (12x12)
// ============================================

static void pattern(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Grid representing a pattern
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 4; col++) {
            canvas.fillRect(x + (1 + col*3)*scale, y + (1 + row*3)*scale, 2*scale, 2*scale);
        }
    }
}

static void sequence(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Steps in sequence
    int heights[] = {4, 6, 3, 7, 5, 8, 4, 6};
    for (int i = 0; i < 8; i++) {
        int h = heights[i] * scale;
        canvas.fillRect(x + (1 + i)*scale, y + (10 - h)*scale, 1*scale, h);
    }
}

static void keyboard(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Simplified piano keys - 5 vertical white keys
    for (int i = 0; i < 5; i++) {
        canvas.fillRect(x + (1 + i*2.2)*scale, y + 1*scale, 1.8*scale, 10*scale);
    }
}

static void keyboardDetailed(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Detailed piano keys with black keys
    // White keys
    for (int i = 0; i < 7; i++) {
        canvas.drawRect(x + i*1.7*scale, y + 2*scale, 1.5*scale, 8*scale);
    }
    // Black keys
    for (int i = 0; i < 6; i++) {
        if (i != 2) { // Skip E-F gap
            canvas.fillRect(x + (i*1.7 + 1.2)*scale, y + 2*scale, 0.8*scale, 5*scale);
        }
    }
}

static void clock(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    int centerX = x + 6*scale;
    int centerY = y + 6*scale;
    int radius = 5*scale;

    // Clock circle
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = radius - abs(dy);
        if (dx > 0) {
            canvas.point(centerX - dx, centerY + dy);
            canvas.point(centerX + dx, centerY + dy);
        }
    }

    // Clock hands
    canvas.line(centerX, centerY, centerX, centerY - 3*scale); // Hour
    canvas.line(centerX, centerY, centerX + 2*scale, centerY); // Minute
}

static void sync(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Circular arrows
    int centerX = x + 6*scale;
    int centerY = y + 6*scale;

    // Top arc
    for (int i = 0; i < 4; i++) {
        canvas.point(centerX - 3*scale + i, centerY - 3*scale);
        canvas.point(centerX + 3*scale - i, centerY - 3*scale);
    }
    // Bottom arc
    for (int i = 0; i < 4; i++) {
        canvas.point(centerX - 3*scale + i, centerY + 3*scale);
        canvas.point(centerX + 3*scale - i, centerY + 3*scale);
    }
    // Arrows
    canvas.line(centerX - 3*scale, centerY - 3*scale, centerX - 2*scale, centerY - 2*scale);
    canvas.line(centerX + 3*scale, centerY + 3*scale, centerX + 2*scale, centerY + 2*scale);
}

static void quantize(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Grid lines (representing quantization grid)
    for (int i = 0; i < 4; i++) {
        canvas.vline(x + (2 + i*3)*scale, y + 2*scale, 8*scale);
    }
    // Notes snapping to grid
    canvas.fillRect(x + 2*scale, y + 4*scale, 2*scale, 2*scale);
    canvas.fillRect(x + 5*scale, y + 6*scale, 2*scale, 2*scale);
    canvas.fillRect(x + 8*scale, y + 3*scale, 2*scale, 2*scale);
}

// ============================================
// MODULATION & EFFECTS (12x12)
// ============================================

static void lfo(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Sine wave
    int lastY = y + 6*scale;
    for (int i = 0; i < 10; i++) {
        float angle = (i * 3.14159f) / 5.0f;
        int newY = y + 6*scale - 4*scale * sin(angle);
        canvas.line(x + 1*scale + i*scale, lastY, x + 1*scale + (i+1)*scale, newY);
        lastY = newY;
    }
}

static void envelope(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // ADSR envelope shape
    canvas.line(x + 1*scale, y + 10*scale, x + 3*scale, y + 2*scale);  // Attack
    canvas.line(x + 3*scale, y + 2*scale, x + 5*scale, y + 5*scale);   // Decay
    canvas.hline(x + 5*scale, y + 5*scale, 3*scale);                   // Sustain
    canvas.line(x + 8*scale, y + 5*scale, x + 10*scale, y + 10*scale); // Release
}

static void filter(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Low-pass filter curve
    canvas.hline(x + 1*scale, y + 3*scale, 4*scale);
    canvas.line(x + 5*scale, y + 3*scale, x + 10*scale, y + 9*scale);
}

static void modulation(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Wavy lines representing modulation
    for (int row = 0; row < 3; row++) {
        int baseY = y + (3 + row*3)*scale;
        for (int i = 0; i < 10; i++) {
            float wave = sin((i * 3.14159f) / 2.5f) * scale;
            canvas.point(x + 1*scale + i*scale, baseY + wave);
        }
    }
}

// ============================================
// FILE & SYSTEM (8x8)
// ============================================

static void save(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Floppy disk
    canvas.drawRect(x + 1*scale, y + 1*scale, 6*scale, 6*scale);
    canvas.fillRect(x + 2*scale, y + 5*scale, 4*scale, 2*scale); // Label area
    canvas.fillRect(x + 5*scale, y + 2*scale, 1*scale, 2*scale); // Metal shutter
}

static void load(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Folder
    canvas.drawRect(x + 1*scale, y + 2*scale, 6*scale, 5*scale);
    canvas.fillRect(x + 1*scale, y + 2*scale, 3*scale, 1*scale); // Tab
}

static void settings(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Gear
    canvas.fillRect(x + 3*scale, y + 3*scale, 2*scale, 2*scale); // Center
    // Teeth
    canvas.fillRect(x + 3*scale, y + 1*scale, 2*scale, 1*scale);
    canvas.fillRect(x + 3*scale, y + 6*scale, 2*scale, 1*scale);
    canvas.fillRect(x + 1*scale, y + 3*scale, 1*scale, 2*scale);
    canvas.fillRect(x + 6*scale, y + 3*scale, 1*scale, 2*scale);
}

static void info(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Circle with 'i'
    int centerX = x + 4*scale;
    int centerY = y + 4*scale;
    int radius = 3*scale;

    // Circle outline
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = radius - abs(dy);
        if (dx > 0) {
            canvas.point(centerX - dx, centerY + dy);
            canvas.point(centerX + dx, centerY + dy);
        }
    }

    // 'i' dot
    canvas.point(centerX, centerY - 1*scale);
    // 'i' line
    canvas.vline(centerX, centerY, 2*scale);
}

static void warning(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Triangle with !
    canvas.line(x + 4*scale, y + 1*scale, x + 1*scale, y + 7*scale);
    canvas.line(x + 4*scale, y + 1*scale, x + 7*scale, y + 7*scale);
    canvas.hline(x + 1*scale, y + 7*scale, 6*scale);

    // Exclamation mark
    canvas.vline(x + 4*scale, y + 3*scale, 2*scale);
    canvas.point(x + 4*scale, y + 6*scale);
}

// ============================================
// NAVIGATION (8x8)
// ============================================

static void arrowUp(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Up arrow
    for (int i = 0; i < 4*scale; i++) {
        canvas.hline(x + (4-i)*scale, y + i, 1 + i*2);
    }
    canvas.fillRect(x + 3*scale, y + 4*scale, 2*scale, 3*scale);
}

static void arrowDown(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Down arrow
    canvas.fillRect(x + 3*scale, y + 1*scale, 2*scale, 3*scale);
    for (int i = 0; i < 4*scale; i++) {
        canvas.hline(x + (4-i)*scale, y + 4*scale + i, 1 + i*2);
    }
}

static void arrowLeft(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Left arrow
    for (int i = 0; i < 4*scale; i++) {
        canvas.vline(x + i, y + (4-i)*scale, 1 + i*2);
    }
    canvas.fillRect(x + 4*scale, y + 3*scale, 3*scale, 2*scale);
}

static void arrowRight(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Right arrow
    canvas.fillRect(x + 1*scale, y + 3*scale, 3*scale, 2*scale);
    for (int i = 0; i < 4*scale; i++) {
        canvas.vline(x + 4*scale + i, y + (4-i)*scale, 1 + i*2);
    }
}

static void checkmark(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // Checkmark
    canvas.line(x + 1*scale, y + 4*scale, x + 3*scale, y + 6*scale);
    canvas.line(x + 3*scale, y + 6*scale, x + 7*scale, y + 2*scale);
}

static void cross(Canvas &canvas, int x, int y, int size = 8) {
    float scale = size / 8.0f;
    // X mark
    canvas.line(x + 2*scale, y + 2*scale, x + 6*scale, y + 6*scale);
    canvas.line(x + 6*scale, y + 2*scale, x + 2*scale, y + 6*scale);
}

// ============================================
// MIDI & CV (12x12)
// ============================================

static void midiIn(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Circle with arrow pointing in
    canvas.drawRect(x + 2*scale, y + 2*scale, 8*scale, 8*scale);
    // Arrow
    canvas.line(x + 11*scale, y + 6*scale, x + 7*scale, y + 6*scale);
    canvas.line(x + 7*scale, y + 6*scale, x + 9*scale, y + 4*scale);
    canvas.line(x + 7*scale, y + 6*scale, x + 9*scale, y + 8*scale);
}

static void midiOut(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Circle with arrow pointing out
    canvas.drawRect(x + 2*scale, y + 2*scale, 8*scale, 8*scale);
    // Arrow
    canvas.line(x + 1*scale, y + 6*scale, x + 5*scale, y + 6*scale);
    canvas.line(x + 5*scale, y + 6*scale, x + 3*scale, y + 4*scale);
    canvas.line(x + 5*scale, y + 6*scale, x + 3*scale, y + 8*scale);
}

static void cvGate(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Gate pulse waveform
    canvas.hline(x + 1*scale, y + 8*scale, 2*scale);
    canvas.vline(x + 3*scale, y + 3*scale, 5*scale);
    canvas.hline(x + 3*scale, y + 3*scale, 4*scale);
    canvas.vline(x + 7*scale, y + 3*scale, 5*scale);
    canvas.hline(x + 7*scale, y + 8*scale, 3*scale);
}

static void trigger(Canvas &canvas, int x, int y, int size = 12) {
    float scale = size / 12.0f;
    // Lightning bolt
    canvas.line(x + 6*scale, y + 1*scale, x + 4*scale, y + 5*scale);
    canvas.line(x + 4*scale, y + 5*scale, x + 7*scale, y + 5*scale);
    canvas.line(x + 7*scale, y + 5*scale, x + 5*scale, y + 10*scale);
}

} // namespace Icons
