#include <windows.h>
#include <thread>
#include <chrono>
#include <unordered_map>
#include "util.h"
#include "macro.h"

const std::unordered_map<char, char> shift{
    {'~', VK_OEM_3},
    {'!', '1'},
    {'@', '2'},
    {'#', '3'},
    {'$', '4'},
    {'%', '5'},
    {'^', '6'},
    {'&', '7'},
    {'(', '9'},
    {')', '0'},
    {'_', VK_OEM_MINUS},
    {'{', VK_OEM_4},
    {'}', VK_OEM_6},
    {'|', VK_OEM_5},
    {':', VK_OEM_1},
    {'"', VK_OEM_7},
    {'<', VK_OEM_COMMA},
    {'>', VK_OEM_PERIOD},
    {'?', VK_OEM_2}
};

const std::unordered_map<char, char> special_key{
    {'`', VK_OEM_3},
    {'-', VK_OEM_MINUS},
    {'=', VK_OEM_PLUS},
    {'\t', VK_TAB},
    {'[', VK_OEM_4},
    {']', VK_OEM_6},
    {'\\', VK_OEM_5},
    {';', VK_OEM_1},
    {'\'', VK_OEM_7},
    {'\n', VK_RETURN},
    {',', VK_OEM_COMMA},
    {'.', VK_OEM_PERIOD},
    {'/', VK_OEM_2},
    {' ', VK_SPACE},
    {'*', VK_MULTIPLY},
    {'+', VK_ADD}
};

Macro::Macro(const double delay_s): delay(round_int(delay_s * 1000000000)),
                                    is_shift(false),
                                    input{} {
    SetProcessDPIAware();
}

void Macro::move_mouse_to(const double x, const double y) const {
    SetCursorPos(round_int(x), round_int(y));
    std::this_thread::sleep_for(delay);
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

void Macro::press_key(const char key) {
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = static_cast<unsigned char>(key);
    input.ki.dwFlags = 0;
    SendInput(1, &input, sizeof(INPUT));
    std::this_thread::sleep_for(delay);
}

void Macro::release_key(const char key) {
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = static_cast<unsigned char>(key);
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
    std::this_thread::sleep_for(delay);
}

/* Typing */

void Macro::type_key(const char key) {
    press_key(key);
    release_key(key);
}

void Macro::non_shift_type(const char key) {
    if (is_shift) {
        release_key(VK_SHIFT);
        is_shift = false;
    }
    type_key(key);
}

void Macro::shift_type(const char key) {
    if (!is_shift) {
        press_key(VK_SHIFT);
        is_shift = true;
    }
    type_key(key);
}

void Macro::type_char(const char c) {
    if ('0' <= c && c <= '9') {
        non_shift_type(c);
        return;
    }
    if ('a' <= c && c <= 'z') {
        non_shift_type(static_cast<char>(c + 'A' - 'a'));
        return;
    }
    auto key_ptr = special_key.find(c);
    if (key_ptr != special_key.end()) {
        non_shift_type(key_ptr->second);
        return;
    }
    if ('A' <= c && c <= 'Z') {
        shift_type(c);
        return;
    }
    key_ptr = shift.find(c);
    if (key_ptr != shift.end()) {
        shift_type(key_ptr->second);
    }
}

void Macro::type_string(const std::string &s) {
    for (const char &c: s) {
        type_char(c);
    }
    if (is_shift) {
        release_key(VK_SHIFT);
        is_shift = false;
    }
}