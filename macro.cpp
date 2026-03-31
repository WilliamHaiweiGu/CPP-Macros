#include <array>
#include <windows.h>
#include "macro.h"
#include "util.h"

constexpr int ASCII_SIZE = CHAR_MAX + 1;

constexpr std::array<unsigned char, ASCII_SIZE> make_shift_table() {
    std::array<unsigned char, ASCII_SIZE> t{};
    t['~'] = VK_OEM_3;
    t['!'] = '1';
    t['@'] = '2';
    t['#'] = '3';
    t['$'] = '4';
    t['%'] = '5';
    t['^'] = '6';
    t['&'] = '7';
    t['('] = '9';
    t[')'] = '0';
    t['_'] = VK_OEM_MINUS;
    t['{'] = VK_OEM_4;
    t['}'] = VK_OEM_6;
    t['|'] = VK_OEM_5;
    t[':'] = VK_OEM_1;
    t['"'] = VK_OEM_7;
    t['<'] = VK_OEM_COMMA;
    t['>'] = VK_OEM_PERIOD;
    t['?'] = VK_OEM_2;
    return t;
}

constexpr std::array<unsigned char, ASCII_SIZE> SHIFT_TB = make_shift_table();

constexpr std::array<unsigned char, ASCII_SIZE> make_special_key_table() {
    std::array<unsigned char, ASCII_SIZE> t{};
    t['`'] = VK_OEM_3;
    t['-'] = VK_OEM_MINUS;
    t['='] = VK_OEM_PLUS;
    t['\t'] = VK_TAB;
    t['['] = VK_OEM_4;
    t[']'] = VK_OEM_6;
    t['\\'] = VK_OEM_5;
    t[';'] = VK_OEM_1;
    t['\''] = VK_OEM_7;
    t['\n'] = VK_RETURN;
    t[','] = VK_OEM_COMMA;
    t['.'] = VK_OEM_PERIOD;
    t['/'] = VK_OEM_2;
    t[' '] = VK_SPACE;
    t['*'] = VK_MULTIPLY;
    t['+'] = VK_ADD;
    return t;
}

constexpr std::array<unsigned char, ASCII_SIZE> SPECIAL_KEY = make_special_key_table();

Macro::Macro(const double delay_s): delay_s(delay_s),
                                    is_shift(false),
                                    input{} {
    SetProcessDPIAware();
}

void Macro::move_mouse_to(const double x, const double y) const {
    SetCursorPos(round_int(x), round_int(y));
    sleep(delay_s);
}

std::pair<int, int> Macro::query_mouse_pos() const {
    POINT p;
    GetCursorPos(&p);
    return {p.x, p.y};
}

void Macro::left_click(const double x, const double y) const {
    move_mouse_to(x, y);
    mouse_event(2, 0, 0, 0, 0); // left down
    mouse_event(4, 0, 0, 0, 0); // left up
}

void Macro::right_click(const double x, const double y) const {
    move_mouse_to(x, y);
    mouse_event(8, 0, 0, 0, 0); // left down
    mouse_event(16, 0, 0, 0, 0); // left up
}

void Macro::press_key(const unsigned char key) {
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    input.ki.dwFlags = 0;
    SendInput(1, &input, sizeof(INPUT));
    sleep(delay_s);
}

void Macro::release_key(const unsigned char key) {
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
    sleep(delay_s);
}

/* Typing */

void Macro::type_key(const unsigned char key) {
    press_key(key);
    release_key(key);
}

void Macro::non_shift_type(const unsigned char key) {
    if (is_shift) {
        release_key(VK_SHIFT);
        is_shift = false;
    }
    type_key(key);
}

void Macro::shift_type(const unsigned char key) {
    if (!is_shift) {
        press_key(VK_SHIFT);
        is_shift = true;
    }
    type_key(key);
}

void Macro::type_ascii(const unsigned char c) {
    if (c >= ASCII_SIZE) {
        return;
    }
    if ('0' <= c && c <= '9') {
        non_shift_type(c);
        return;
    }
    if ('a' <= c && c <= 'z') {
        non_shift_type(static_cast<char>(c + 'A' - 'a'));
        return;
    }
    const unsigned char special_key_ = SPECIAL_KEY[c];
    if (special_key_ != 0) {
        non_shift_type(special_key_);
        return;
    }
    if ('A' <= c && c <= 'Z') {
        shift_type(c);
        return;
    }
    const unsigned char shift_key = SHIFT_TB[c];
    if (shift_key != 0) {
        shift_type(shift_key);
    }
}

void Macro::type_ascii_string(const std::string &s) {
    for (const char &c: s) {
        type_ascii(c);
    }
    if (is_shift) {
        release_key(VK_SHIFT);
        is_shift = false;
    }
}
