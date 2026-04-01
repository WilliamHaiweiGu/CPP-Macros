#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_map>
#include "util.h"

constexpr int N_LETTER = 5;

using word_t = std::array<char, N_LETTER>;

// Wordle data

const std::array<char, N_LETTER> from_str(const std::string &str) {
    if (str.size() < N_LETTER) {
        std::cerr << "String too short (must >=5): " << str << std::endl;
        exit(-1);
    }
    word_t word;
    copy_n(str.begin(), N_LETTER, word.begin());
    return word;
}

const std::vector<word_t> load_file(const std::string &file_name) {
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

    static std::string match(const int ans_idx, const word_t &guess) {
        const word_t &target = ANSWERS[ans_idx];
        std::unordered_map<char, uint8_t> target_freqs;
        std::string ans = "BBBBB";
        for (int i = 0; i < N_LETTER; i++) {
            const char target_c = target[i];
            if (guess[i] == target_c) {
                ans[i] = 'G';
            }
            else {
                target_freqs[target_c]++;
            }
        }
        for (int i = 0; i < N_LETTER; i++) {
            if (ans[i] == 'G') {
                continue;
            }
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

constexpr int N_WORDLE = 4;

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
            std::vector<int> &possible_answer_idx = possible_answer_idxs[w];
            const int n_possible = static_cast<int>(possible_answer_idx.size());
            int n_match = 0;
            for (int i = 0; i < n_possible; i++) {
                const int answer_idx = possible_answer_idx[i];
                if (colors[w] == WordleGame::match(answer_idx, guess)) { // keep result
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
                std::cerr << "Err: No more word possible for game " << w << std::endl;
                exit(-1);
            }
            if (n_possible == 1) {
                solved[w] = true;
                return ANSWERS[possible_answer_idx[0]];
            }
        }
        // Get total entropy for every allowed word
        std::vector<double> entropy(N_ALLOWED);
        for (int w = 0; w < N_WORDLE; w++) {
            if (solved[w]) {
                continue;
            }
            std::vector<int> &possible_answer_idx = possible_answer_idxs[w];
            const int n_possible = static_cast<int>(possible_answer_idx.size());
            for (int i = 0; i < N_ALLOWED; i++) {
                std::unordered_map<std::string, int> color_row_freqs;
                const word_t word = ALLOWED[i];
                for (const int answer_idx: possible_answer_idx) {
                    color_row_freqs[WordleGame::match(answer_idx, word)]++;
                }
                double word_entropy = 0;
                for (const auto &[colors, count]: color_row_freqs) {
                    const double prob = static_cast<double>(count) / n_possible;
                    word_entropy -= prob * log2(prob);
                }
                entropy[i] += word_entropy;
            }
        }
        const auto max_it = std::max_element(entropy.begin(), entropy.end());
        const int argmax = static_cast<int>(std::distance(entropy.begin(), max_it));
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
        std::cout << "Guess: " << guess_str << "\nColor: " << std::flush;
        bool stop = true;
        for (auto &color: colors) {
            std::string in;
            std::cin >> in;
            if (color != "GGGGG") {
                color = in;
            }
            stop = stop && color == "GGGGG";
        }
        if (stop) {
            break;
        }
        player.process_feedback(guess, colors);
    }
}

int main() {
    shell();
}
