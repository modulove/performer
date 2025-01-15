#pragma once

#include "ui/model/ContextMenuModel.h"

#include <functional>

class ContextMenu : public ContextMenuModel {
public:
    using ActionCallback = std::function<void(int)>;
    using ItemEnabledCallback = std::function<bool(int)>;

    ContextMenu() = default;

    ContextMenu(
        const Item items[],
        int itemCount,
        ActionCallback actionCallback,
        ItemEnabledCallback itemEnabledCallback = [] (int) { return true; }
    ) :
        _items(items),
        _itemCount(itemCount),
        _actionCallback(actionCallback),
        _itemEnabledCallback(itemEnabledCallback)
    {
    }

    const ActionCallback &actionCallback() const { return _actionCallback; }

private:
    virtual int itemCount() const override {
        return _itemCount;
    }

    virtual const ContextMenuModel::Item &item(int index) const override {
        return _items[index];
    }

    virtual bool itemEnabled(int index) const override {
        return _itemEnabledCallback(index);
    }

    const Item *_items;
    int _itemCount;
    ActionCallback _actionCallback;
    ItemEnabledCallback _itemEnabledCallback;
};
