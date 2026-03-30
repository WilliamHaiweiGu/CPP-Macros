#include <windows.h>
#include <thread>
#include <chrono>
#include <unordered_map>
#include "util.cpp"

using namespace std;

const unordered_map<char, char> shift{
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

const unordered_map<char, char> special_key{
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

class Macro {
public:
    const double scale;
    const chrono::nanoseconds delay;
    bool is_shift;

    /**
     *
     * @param screen_scale Magnification scale in display settings
     * @param delay_s Small delay in seconds after each key/mouse action to avoid being skipped. Must be ≤ 2.147483647
     */
    Macro(const double screen_scale, const double delay_s): scale(screen_scale),
                                                            delay(round_int(delay_s * 1000000000)),
                                                            is_shift(false),
                                                            input{} {
    }

    void move_mouse_to(const double x, const double y) const {
        SetCursorPos(round_int(x / scale), round_int(y / scale));
        this_thread::sleep_for(delay);
    }

    pair<int, int> query_mouse_pos() const {
        POINT p;
        GetCursorPos(&p);
        return {round_int(p.x * scale), round_int(p.y * scale)};
    }

    void left_click(const double x, const double y) const {
        move_mouse_to(x, y);
        mouse_event(2, 0, 0, 0, 0); // left down
        mouse_event(4, 0, 0, 0, 0); // left up
    }

    void right_click(const double x, const double y) const {
        move_mouse_to(x, y);
        mouse_event(8, 0, 0, 0, 0); // left down
        mouse_event(16, 0, 0, 0, 0); // left up
    }

    void press_key(const char key) {
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<unsigned char>(key);
        input.ki.dwFlags = 0;
        SendInput(1, &input, sizeof(INPUT));
        this_thread::sleep_for(delay);
    }

    void release_key(const char key) {
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<unsigned char>(key);
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
        this_thread::sleep_for(delay);
    }

    /* Typing */

    void type_key(const char key) {
        press_key(key);
        release_key(key);
    }

    void non_shift_type(const char key) {
        if (is_shift) {
            release_key(VK_SHIFT);
            is_shift = false;
        }
        type_key(key);
    }

    void shift_type(const char key) {
        if (!is_shift) {
            press_key(VK_SHIFT);
            is_shift = true;
        }
        type_key(key);
    }

    void type_char(const char c) {
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

    void type_string(const string &s) {
        for (const char &c: s) {
            type_char(c);
        }
        if (is_shift) {
            release_key(VK_SHIFT);
            is_shift = false;
        }
    }

private:
    INPUT input;
};

int main() {
    Macro macro(2, 1.0 / 120);
    macro.type_string(
        " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\n\t");
}