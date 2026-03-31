#pragma once

#include <string>
#include <utility>
#include <windows.h>

class Macro {
public:
    const double delay_s;
    bool is_shift;

    /**
     * @param delay_s Small delay in seconds after each key/mouse action to avoid being skipped.
     */
    Macro(double delay_s);

    void move_mouse_to(double x, double y) const;

    std::pair<int, int> query_mouse_pos() const;

    void left_click(double x, double y) const;

    void right_click(double x, double y) const;

    void press_key(char key);

    void release_key(char key);

    void type_key(char key);

    void non_shift_type(char key);

    void shift_type(char key);

    void type_char(char c);

    void type_string(const std::string &s);

private:
    INPUT input;
};
