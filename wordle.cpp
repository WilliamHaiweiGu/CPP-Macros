#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>
#include "macro.h"
#include "screenshot.h"
#include "util.h"

constexpr int N_LETTER = 5;
constexpr int N_ROW = 6;

using word_t = std::array<char, N_LETTER>;


// Wordle data

std::array<char, N_LETTER> from_str(const std::string &str) {
    if (str.size() < N_LETTER) {
        std::cerr << "String too short (must >=5): " << str << std::endl;
        exit(-1);
    }
    word_t word;
    copy_n(str.begin(), N_LETTER, word.begin());
    return word;
}

std::vector<word_t> load_file(const std::string &file_name) {
    std::ifstream file(file_name);
    std::vector<word_t> words;
    if (!file.is_open()) {
        std::cerr << "Error opening file " << file_name << std::endl;
        exit(-1);
    }
    std::string line;
    while (getline(file, line))
        words.push_back(from_str(line));
    file.close();
    return words;
}

const std::vector<word_t> allowed = load_file("Wordle Unlimited/words.txt");
const std::vector<word_t> answers = load_file("Wordle Unlimited/solution.txt");

std::default_random_engine gen((std::random_device())());
std::uniform_int_distribution<> ans_dist(0, static_cast<int>(answers.size()) - 1);

class WordleGame {
public:
    int target_idx;

    WordleGame() : target_idx(ans_dist(gen)) {
    }

    static std::string match(const int ans_idx, const word_t &guess) {
        const word_t &target = answers[ans_idx];
        std::unordered_map<char, uint8_t> target_freqs;
        std::string ans = "BBBBB";
        for (int i = 0; i < N_LETTER; i++) {
            const char target_c = target[i];
            if (guess[i] == target_c)
                ans[i] = 'G';
            else
                target_freqs[target_c]++;
        }
        for (int i = 0; i < N_LETTER; i++) {
            if (ans[i] == 'G')
                continue;
            const char guess_c = guess[i];
            auto c_target_freq = target_freqs.find(guess_c);
            if (c_target_freq != target_freqs.end() && c_target_freq->second > 0) {
                ans[i] = 'Y';
                c_target_freq->second--;
            }
        }
        return ans;
    }
};

class WordleSolver {
public:
    std::vector<int> possible_answer_idx;
    bool default_guess;

    WordleSolver() : default_guess(true) {
        const int n_answer = static_cast<int>(answers.size());
        possible_answer_idx.reserve(n_answer);
        for (int i = 0; i < n_answer; i++)
            possible_answer_idx.push_back(i);
    }

    void process_feedback(const word_t &guess, const std::string &colors) {
        const int n_possible = static_cast<int>(possible_answer_idx.size());
        int i = 0;
        for (int j = 0; j < n_possible; j++) {
            const int answer_idx = possible_answer_idx[j];
            std::string result = WordleGame::match(answer_idx, guess);
            if (colors == result) // keep result
            {
                possible_answer_idx[i] = possible_answer_idx[j];
                i++;
            }
        }
        // remove unsatisfactory idxs
        for (; i < n_possible; i++)
            possible_answer_idx.pop_back();
    }

    word_t make_guess() {
        if (default_guess) {
            default_guess = false;
            return from_str("SOARE");
        }
        const int n_possible = static_cast<int>(possible_answer_idx.size());
        if (n_possible <= 0) {
            std::cerr << "Err: No more word possible" << std::endl;
            exit(-1);
        }
        if (n_possible == 1)
            return answers[possible_answer_idx[0]];

        const int n_allowed = static_cast<int>(allowed.size());
        int entropy_argmax = -1;
        double entropy_max = 0;
        for (int i = 0; i < n_allowed; i++) {
            const word_t word = allowed[i];
            std::unordered_map<std::string, int> color_row_freqs;
            for (const int answer_idx: possible_answer_idx)
                color_row_freqs[WordleGame::match(answer_idx, word)]++;
            double entropy = 0;
            for (const auto &[colors, count]: color_row_freqs) {
                const double prob = (double) count / n_possible;
                entropy -= prob * log2(prob);
            }
            if (entropy_argmax < 0 || entropy > entropy_max) {
                entropy_max = entropy;
                entropy_argmax = i;
            }
        }
        return allowed[entropy_argmax];
    }
};

std::string get_row_color(const int i) {
    constexpr std::pair<const int, const int> top_left_tile = {996, 310};
    constexpr std::pair<const int, const int> bottom_right_tile = {1532, 984};
    constexpr int offset_pix = 32;

    constexpr double horz_step = static_cast<double>(bottom_right_tile.first - top_left_tile.first) / (N_LETTER - 1);
    constexpr double vert_step = static_cast<double>(bottom_right_tile.second - top_left_tile.second) / (N_ROW - 1);

    const int y = round_int(top_left_tile.second + i * vert_step);

    Screenshot screenshot;

    std::string ans;
    ans.reserve(N_LETTER);
    for (int j = 0; j < N_LETTER; j++) {
        const int x = round_int(top_left_tile.first + j * horz_step);
        //single tile
        const std::array<unsigned char, 3> left_color = screenshot.get_rgb(x - offset_pix, y);
        const std::array<unsigned char, 3> right_color = screenshot.get_rgb(x + offset_pix, y);
        const int red = left_color[0] + right_color[0];
        const int green = left_color[1] + right_color[1];
        const char color = red > 225 ? 'Y' : green > 225 ? 'G' : 'B';
        ans.push_back(color);
    }
    return ans;
}

int main() {
    Macro macro(1.0 / 120);
    constexpr std::pair<const int, const int> play_again = {1035, 1111};

    std::cout << "Open https://wordleunlimited.org and use full screen mode. After the pass, switch to that window ASAP"
            << std::endl;
    system("pause");
    std::cout << "Macro starting in 3 seconds..." << std::endl;
    sleep(3);
    while (true) {
        WordleSolver solver;
        for (int i = 0; i < N_ROW; i++) {
            const word_t guess = solver.make_guess();
            const std::string guess_str(guess.data(), N_LETTER);
            std::cout << guess_str << std::flush;
            macro.type_ascii_string(guess_str);
            macro.type_ascii_string("\n");
            sleep(2);
            const std::string row_color = get_row_color(i);
            std::cout << ' ' << row_color << std::endl;
            if (row_color == "GGGGG") {
                break;
            }
            if (i == N_ROW - 1) {
                throw std::runtime_error("Could not solve in 6 guesses :(");
            }
            solver.process_feedback(guess, row_color);
        }
        std::cout << std::endl;
        sleep(2.3);
        macro.left_click(play_again.first, play_again.second);
        sleep(0.7);
    }
}
