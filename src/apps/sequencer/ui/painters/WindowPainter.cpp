#include "WindowPainter.h"

#include "Config.h"

#include "core/utils/StringBuilder.h"
#include "ScreenPainter.h"

static void drawInvertedText(ScreenPainter &painter, int x, int baselineY, const char *text, bool inverted = true) {
    auto &canvas = painter.canvas();
    const auto metrics = canvas.fontMetrics();

    int drawX = x + painter.offsetX();
    int baseline = baselineY + painter.offsetY();
    int top = baseline - metrics.ascent;
    int height = metrics.ascent + metrics.descent;

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);

    if (inverted) {
        canvas.fillRect(drawX - 1, top, canvas.textWidth(text) + 1, height);
        canvas.setBlendMode(BlendMode::Sub);
    }

    canvas.drawText(drawX, baseline, text);
    canvas.setBlendMode(BlendMode::Set);
}

void WindowPainter::clear(Canvas &canvas) {
    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::None);
    canvas.fill();
}

void WindowPainter::drawFrame(Canvas &canvas, int x, int y, int w, int h) {
    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::None);
    canvas.fillRect(x, y, w, h);
    canvas.setColor(Color::Bright);
    canvas.drawRect(x, y, w, h);
}

void WindowPainter::drawFunctionKeys(Canvas &canvas, const char *names[], const KeyState &keyState, int highlight) {
    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Medium);
    canvas.hline(0, PageHeight - FooterHeight - 1, PageWidth);

    for (int i = 0; i < FunctionKeyCount; ++i) {
        if (names[i] || (i + 1 < FunctionKeyCount && names[i + 1])) {
            int x = (PageWidth * (i + 1)) / FunctionKeyCount;
            canvas.vline(x, PageHeight - FooterHeight, FooterHeight);
        }
    }

    ScreenPainter painter(canvas);
    painter.setFont(Font::Tiny);

    for (int i = 0; i < FunctionKeyCount; ++i) {
        canvas.setColor(Color::Medium);
        if (names[i]) {
            bool pressed = keyState[Key::F0 + i];

            if (highlight >= 0) {
                pressed = i == highlight;
            }

            int x0 = (PageWidth * i) / FunctionKeyCount;
            int x1 = (PageWidth * (i + 1)) / FunctionKeyCount;
            int w = x1 - x0 + 1;

            if (pressed) {
                canvas.setColor(Color::Bright);
            }

            int labelX = x0 + (w - painter.textWidth(names[i])) / 2;
            painter.drawTextBaseline(labelX, PageHeight - 3, names[i]);
        }
    }
}

// Overloaded version with availability flags for dimming unavailable options
void WindowPainter::drawFunctionKeys(Canvas &canvas, const char *names[], const KeyState &keyState, int highlight, const bool available[]) {
    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Medium);
    canvas.hline(0, PageHeight - FooterHeight - 1, PageWidth);

    for (int i = 0; i < FunctionKeyCount; ++i) {
        if (names[i] || (i + 1 < FunctionKeyCount && names[i + 1])) {
            int x = (PageWidth * (i + 1)) / FunctionKeyCount;
            canvas.vline(x, PageHeight - FooterHeight, FooterHeight);
        }
    }

    ScreenPainter painter(canvas);
    painter.setFont(Font::Tiny);

    for (int i = 0; i < FunctionKeyCount; ++i) {
        if (names[i]) {
            bool pressed = keyState[Key::F0 + i];

            if (highlight >= 0) {
                pressed = i == highlight;
            }

            int x0 = (PageWidth * i) / FunctionKeyCount;
            int x1 = (PageWidth * (i + 1)) / FunctionKeyCount;
            int w = x1 - x0 + 1;

            // Set color based on availability and pressed state
            if (!available[i]) {
                canvas.setColor(Color::Low);  // Dim unavailable options
            } else if (pressed) {
                canvas.setColor(Color::Bright);  // Bright for selected
            } else {
                canvas.setColor(Color::Medium);  // Medium for available
            }

            int labelX = x0 + (w - painter.textWidth(names[i])) / 2;
            painter.drawTextBaseline(labelX, PageHeight - 3, names[i]);
        }
    }
}

void WindowPainter::drawClock(Canvas &canvas, const Engine &engine) {
    static const char *clockModeName[] = { "A", "M", "S" };
    const char *name = engine.recording() ? "R" : clockModeName[int(engine.clock().activeMode())];

    ScreenPainter painter(canvas);
    painter.setFont(Font::Tiny);
    painter.setOffset(0, -2);

    drawInvertedText(painter, 2, 8, name);

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);
    painter.drawTextBaseline(10, 8, FixedStringBuilder<8>("%.1f", engine.tempo()));
}

void WindowPainter::drawActiveState(Canvas &canvas, int track, int playPattern, int editPattern, bool snapshotActive, bool songActive) {
    ScreenPainter painter(canvas);
    painter.setFont(Font::Tiny);
    painter.setOffset(0, -2);

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);

    // draw selected track (shifted right for better spacing from BPM)
    painter.drawTextBaseline(45, 8, FixedStringBuilder<8>("T%d", track + 1));

    if (snapshotActive) {
        drawInvertedText(painter, 68, 8, "SNAP", true);
    } else {
        // draw active pattern (shifted right to accommodate two-digit track numbers)
        drawInvertedText(painter, 68, 8, FixedStringBuilder<8>("P%d", playPattern + 1), songActive);

        // draw edit pattern (shifted right to maintain spacing)
        drawInvertedText(painter, 87, 8, FixedStringBuilder<8>("E%d", editPattern + 1), playPattern == editPattern);
    }
}

void WindowPainter::drawActiveMode(Canvas &canvas, const char *mode) {
    ScreenPainter painter(canvas);
    painter.setFont(Font::Tiny);
    painter.setOffset(0, -2);

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);
    int x = PageWidth - painter.textWidth(mode) - 2;
    painter.drawTextBaseline(x, 8, mode);
}

void WindowPainter::drawActiveFunction(Canvas &canvas, const char *function) {
    ScreenPainter painter(canvas);
    painter.setFont(Font::Tiny);
    painter.setOffset(0, -2);

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);
    painter.drawTextBaseline(100, 8, function);
}

void WindowPainter::drawHeader(Canvas &canvas, const Model &model, const Engine &engine, const char *mode) {
    const auto &project = model.project();
    int track = project.selectedTrackIndex();
    int playPattern = project.playState().trackState(track).pattern();
    int editPattern = project.selectedPatternIndex();
    bool snapshotActive = project.playState().snapshotActive();
    bool songActive = project.playState().songState().playing();

    drawClock(canvas, engine);
    drawActiveState(canvas, track, playPattern, editPattern, snapshotActive, songActive);
    drawActiveMode(canvas, mode);

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Medium);
    canvas.hline(0, HeaderHeight + 1, PageWidth);
}

void WindowPainter::drawFooter(Canvas &canvas) {
    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Medium);
    canvas.hline(0, PageHeight - FooterHeight - 1, PageWidth);
}

void WindowPainter::drawFooter(Canvas &canvas, const char *names[], const KeyState &keyState, int highlight) {
    drawFunctionKeys(canvas, names, keyState, highlight);
}

void WindowPainter::drawFooter(Canvas &canvas, const char *names[], const KeyState &keyState, int highlight, const bool available[]) {
    drawFunctionKeys(canvas, names, keyState, highlight, available);
}

void WindowPainter::drawPagination(Canvas &canvas, int currentPage, int totalPages) {
    if (totalPages <= 1) {
        return;  // No pagination needed for single page
    }

    canvas.setBlendMode(BlendMode::Set);

    // Footer is 9 pixels high (55-63), center arrows vertically within it
    const int footerY = PageHeight - FooterHeight + 2;  // Start at y=57
    const int arrowSize = 3;  // Smaller size to fit within footer

    // Left arrow (previous page) - at left edge of footer
    bool hasPrevPage = (currentPage > 0);
    canvas.setColor(hasPrevPage ? Color::Bright : Color::Low);

    // Draw left triangle arrow (◀︎) - points left
    int leftX = 3;
    for (int i = 0; i < arrowSize; ++i) {
        int height = (i * 2) + 1;
        int yOffset = arrowSize - i - 1;
        canvas.vline(leftX + i, footerY + yOffset, height);
    }

    // Right arrow (next page) - at right edge of footer
    bool hasNextPage = (currentPage < totalPages - 1);
    canvas.setColor(hasNextPage ? Color::Bright : Color::Low);

    // Draw right triangle arrow (▶︎) - points right
    int rightX = PageWidth - 3 - arrowSize;
    for (int i = 0; i < arrowSize; ++i) {
        int height = (arrowSize - i) * 2 - 1;
        canvas.vline(rightX + i, footerY + i, height);
    }
}

void WindowPainter::drawScrollbar(Canvas &canvas, int x, int y, int w, int h, int totalRows, int visibleRows, int displayRow) {
    if (visibleRows >= totalRows) {
        return;
    }

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Medium);
    canvas.drawRect(x, y, w, h);

    int bh = (visibleRows * h) / totalRows;
    int by = (displayRow * h) / totalRows;
    canvas.setColor(Color::Bright);
    canvas.fillRect(x, y + by, w, bh);
}
