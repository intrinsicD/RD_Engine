#pragma once

namespace RDE {
    enum MouseButton {
        BUTTON_UNKNOWN = 0,
        BUTTON_LEFT,
        BUTTON_RIGHT,
        BUTTON_MIDDLE,
        BUTTON_LAST
    };

    enum MouseButtonAction {
        BUTTON_PRESS = 0,
        BUTTON_RELEASE = 1,
        BUTTON_REPEAT = 2
    };
}
