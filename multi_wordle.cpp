/** Must compile with --std=c++20*/
#include <algorithm>
#include <array>
#include <format>
#include <fstream>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

constexpr int N_LETTER = 5;

using word_t = std::array<char, N_LETTER>;

// Wordle data

const std::array<char, N_LETTER> from_str(const std::string &str) {
    if (str.size() < N_LETTER) {
        throw std::runtime_error(std::format("Input string too short (must >=5): {}", str));
    }
    word_t word;
    copy_n(str.begin(), N_LETTER, word.begin());
    return word;
}

const std::vector<word_t> load_file(const std::string &file_name) {
    std::ifstream file(file_name);
    std::vector<word_t> words;
    if (!file.is_open()) {
        throw std::runtime_error(std::format("Failed to open file {} ", file_name));
    }
    std::string line;
    while (getline(file, line))
        words.push_back(from_str(line));
    file.close();
    return words;
}

const std::vector<word_t> ALLOWED = load_file("Quordle/allowed.txt");
const int N_ALLOWED = static_cast<int>(ALLOWED.size());
const std::vector<word_t> ANSWERS = load_file("Quordle/answers.txt");
const int N_ANSWERS = static_cast<int>(ANSWERS.size());

std::default_random_engine gen((std::random_device())());
std::uniform_int_distribution<int> ans_dist(0, N_ANSWERS - 1);

class WordleGame {
public:
    int target_idx;

    WordleGame() : target_idx(ans_dist(gen)) {
    }

    std::string test(const word_t &guess) {
        return match(target_idx, guess);
    }

    static std::string match(const int ans_idx, const word_t &guess) {
        const word_t &answer = ANSWERS[ans_idx];
        uint8_t yellow_freq[26]{};
        std::string res(N_LETTER, 'B');
        for (int i = 0; i < N_LETTER; i++) {
            const char answer_i = answer[i];
            if (guess[i] == answer_i) {
                res[i] = 'G';
            } else {
                yellow_freq[answer_i - 'A']++;
            }
        }
        for (int i = 0; i < N_LETTER; i++) {
            if (res[i] == 'G') {
                continue;
            }
            uint8_t &yellow_remain_cnt = yellow_freq[guess[i] - 'A'];
            if (yellow_remain_cnt > 0) {
                res[i] = 'Y';
                yellow_remain_cnt--;
            }
        }
        return res;
    }
};

constexpr int N_WORDLE = 256;
const std::string SOLVED_COLOR(N_LETTER, 'G');

class NWordleSolver {
public:
    std::vector<int> possible_answer_idxs[N_WORDLE];
    bool solved[N_WORDLE];
    bool default_guess;

    NWordleSolver() : possible_answer_idxs{}, solved{false}, default_guess(true) {
        for (std::vector<int> &possible_answer_idx: possible_answer_idxs) {
            possible_answer_idx.reserve(N_ANSWERS);
            for (int i = 0; i < N_ANSWERS; i++) {
                possible_answer_idx.push_back(i);
            }
        }
    }

    void process_feedback(const word_t &guess, const std::string (&colors)[N_WORDLE]) {
        for (int w = 0; w < N_WORDLE; w++) {
            const std::string &color = colors[w];
            if (color == SOLVED_COLOR) {
                solved[w] = true;
                continue;
            }
            std::vector<int> &possible_answer_idx = possible_answer_idxs[w];
            const int n_possible = static_cast<int>(possible_answer_idx.size());
            int n_match = 0;
            for (int i = 0; i < n_possible; i++) {
                const int answer_idx = possible_answer_idx[i];
                if (color == WordleGame::match(answer_idx, guess)) { // keep result
                    possible_answer_idx[n_match++] = answer_idx;
                }
            }
            // remove unsatisfactory idxs
            possible_answer_idx.resize(n_match);
        }
    }

    word_t make_guess() {
        if (default_guess) {
            default_guess = false;
            return from_str("SOARE");
        }
        // If any wordle has a definite answer, guess it first
        for (int w = 0; w < N_WORDLE; w++) {
            if (solved[w]) {
                continue;
            }
            std::vector<int> &possible_answer_idx = possible_answer_idxs[w];
            const int n_possible = static_cast<int>(possible_answer_idx.size());
            if (n_possible <= 0) {
                throw std::runtime_error(std::format("Err: No more word possible for game {}", w));
            }
            if (n_possible == 1) {
                solved[w] = true;
                return ANSWERS[possible_answer_idx[0]];
            }
        }
        // Get total entropy for every allowed word
        double max_entropy = 0;
        int argmax = 0;
        for (int i = 0; i < N_ALLOWED; i++) {
            const word_t word = ALLOWED[i];
            double word_info_gain = 0;
            for (int w = 0; w < N_WORDLE; w++) {
                if (solved[w]) {
                    continue;
                }
                std::vector<int> &possible_answer_idx = possible_answer_idxs[w];
                std::unordered_map<std::string, int> color_row_freqs;
                for (const int answer_idx: possible_answer_idx) {
                    color_row_freqs[WordleGame::match(answer_idx, word)]++;
                }
                const double n_possible = static_cast<double>(possible_answer_idx.size());
                for (const auto &[colors, count]: color_row_freqs) {
                    const double prob = count / n_possible;
                    word_info_gain -= prob * log2(prob);
                }
            }
            if (word_info_gain > max_entropy) {
                max_entropy = word_info_gain;
                argmax = i;
            }
        }
        return ALLOWED[argmax];
    }
};

void shell() {
    NWordleSolver player;
    std::cout << "Enter color in format:";
    for (int i = 0; i < N_WORDLE; i++) {
        std::cout << ' ';
        for (int j = 0; j < N_LETTER; j++) {
            std::cout << 'C';
        }
    }
    std::cout << '\n';
    std::string colors[N_WORDLE];
    while (true) {
        const word_t guess = player.make_guess();
        const std::string guess_str(guess.data(), N_LETTER);
        std::cout << guess_str << " | ";
        bool stop = true;
        for (auto &color: colors) {
            std::string in;
            std::cin >> in;
            if (color != SOLVED_COLOR) {
                color = in;
            }
            stop = stop && color == SOLVED_COLOR;
        }
        if (stop) {
            break;
        }
        player.process_feedback(guess, colors);
    }
}

void auto_game() {
    WordleGame game[N_WORDLE]{};
    std::string colors[N_WORDLE];
    NWordleSolver player;
    for (int i = 1;; i++) {
        const word_t guess = player.make_guess();
        const std::string guess_str(guess.data(), N_LETTER);
        std::cout << guess_str << " |";
        bool stop = true;
        for (int w = 0; w < N_WORDLE; w++) {
            std::string &color = colors[w];
            if (i <= 0 || color != SOLVED_COLOR) {
                color = game[w].test(guess);
            }
            std::cout << ' ' << color;
            stop = stop && color == SOLVED_COLOR;
        }
        std::cout << std::endl;
        if (stop) {
            std::cout << "Solved in " << i << " rounds." << std::endl;
            return;
        }
        player.process_feedback(guess, colors);
    }
}

int main() {
    auto_game();
}
