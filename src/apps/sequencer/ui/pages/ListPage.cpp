#include "ListPage.h"

#include "ui/LedPainter.h"
#include "ui/painters/WindowPainter.h"

#include "core/math/Math.h"

ListPage::ListPage(PageManager &manager, PageContext &context, ListModel &listModel) :
    BasePage(manager, context)
{
    setListModel(listModel);
}

void ListPage::setListModel(ListModel &listModel) {
    _listModel = &listModel;
    setSelectedRow(0);
    _edit = false;
}

void ListPage::show() {
    BasePage::show();
}

void ListPage::enter() {
    scrollTo(_selectedRow);
}

void ListPage::exit() {
}

void ListPage::draw(Canvas &canvas) {
    int displayRow = _displayRow;
    if (displayRow + LineCount > _listModel->rows()) {
        displayRow = std::max(0, _listModel->rows() - LineCount);
    }

    // DYNAMIC COLUMN WIDTH CALCULATION
    // Measure actual content width for column 0 across all visible rows
    canvas.setFont(Font::Small);
    int maxCol0Width = 0;
    for (int i = 0; i < LineCount; ++i) {
        int row = displayRow + i;
        if (row < _listModel->rows()) {
            FixedStringBuilder<32> str;
            _listModel->cell(row, 0, str);
            int textWidth = canvas.textWidth(str);
            if (textWidth > maxCol0Width) {
                maxCol0Width = textWidth;
            }
        }
    }

    // Calculate column positions with dynamic spacing
    const int leftMargin = 8;
    const int minGap = 8;           // Minimum gap between columns
    const int scrollbarWidth = 12;  // Space for scrollbar on right
    const int availableWidth = Width - leftMargin - scrollbarWidth;

    // Column 0 gets exactly what it needs, column 1 gets the rest
    int col0Width = maxCol0Width + minGap;
    int col1X = leftMargin + col0Width;
    int col1Width = availableWidth - col0Width;

    // Ensure column 1 has minimum space (if content too wide, allow overlap with scrollbar)
    if (col1Width < 40) {
        col1Width = 40;
        col1X = Width - scrollbarWidth - col1Width;
        col0Width = col1X - leftMargin - minGap;
    }

    // Draw all cells with dynamic positioning
    for (int i = 0; i < LineCount; ++i) {
        int row = displayRow + i;
        if (row < _listModel->rows()) {
            drawCell(canvas, row, 0, leftMargin, 18 + i * LineHeight, col0Width, LineHeight);
            drawCell(canvas, row, 1, col1X, 18 + i * LineHeight, col1Width, LineHeight);
        }
    }

    WindowPainter::drawScrollbar(canvas, Width - 8, 13, 4, LineCount * LineHeight, _listModel->rows(), LineCount, displayRow);
}

void ListPage::updateLeds(Leds &leds) {
}

void ListPage::keyPress(KeyPressEvent &event) {
    const auto &key = event.key();

    if (key.pageModifier()) {
        return;
    }

    if (key.isLeft()) {
        if (_edit) {
            _listModel->edit(_selectedRow, 1, -1, globalKeyState()[Key::Shift]);
        } else {
            setSelectedRow(selectedRow() - 1);
        }
        event.consume();
    }
    else if (key.isRight()) {
        if (_edit) {
            _listModel->edit(_selectedRow, 1, 1, globalKeyState()[Key::Shift]);
        } else {
            setSelectedRow(selectedRow() + 1);
        }
        event.consume();
    } else if (key.isEncoder()) {
        _edit = !_edit;
        event.consume();
    }
}

void ListPage::encoder(EncoderEvent &event) {
    if (_edit) {
        _listModel->edit(_selectedRow, 1, event.value(), event.pressed() | globalKeyState()[Key::Shift]);
    } else {
        setSelectedRow(selectedRow() + event.value());
    }
    event.consume();
}

void ListPage::setSelectedRow(int selectedRow) {
    if (selectedRow != _selectedRow) {
        _selectedRow = clamp(selectedRow, 0, _listModel->rows() - 1);;
        scrollTo(_selectedRow);
    }
}

void ListPage::setTopRow(int row) {
    _displayRow = std::min(row, std::max(0, _listModel->rows() - LineCount));
}

void ListPage::drawCell(Canvas &canvas, int row, int column, int x, int y, int w, int h) {
    FixedStringBuilder<32> str;
    _listModel->cell(row, column, str);
    canvas.setFont(Font::Small);
    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(column == int(_edit) && row == _selectedRow ? Color::Bright : Color::Medium);
    canvas.drawText(x, y + 2, str);
}

void ListPage::scrollTo(int row) {
    if (row < _displayRow) {
        _displayRow = row;
    } else if (row >= _displayRow + LineCount) {
        _displayRow = row - (LineCount - 1);
    }
}
