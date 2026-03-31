#pragma once

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template<typename T0, typename T1>
std::ostream &operator<<(std::ostream &os, const std::pair<T0, T1> &p) {
    os << '(' << p.first << ", " << p.second << ')';
    return os;
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec) {
    os << '[';
    bool first = true;
    for (T elem: vec) {
        if (first) {
            first = false;
        } else {
            os << ", ";
        }
        os << elem;
    }
    os << ']';
    return os;
}

template<typename T, size_t N>
std::ostream &operator<<(std::ostream &os, const std::array<T, N> &vec) {
    os << '[';
    bool first = true;
    for (T elem: vec) {
        if (first) {
            first = false;
        } else {
            os << ", ";
        }
        os << elem;
    }
    os << ']';
    return os;
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const std::unordered_set<T> &set) {
    os << '{';
    bool first = true;
    for (T elem: set) {
        if (first) {
            first = false;
        } else {
            os << ", ";
        }
        os << elem;
    }
    os << '}';
    return os;
}

template<typename K, typename V>
std::ostream &operator<<(std::ostream &os, const std::unordered_map<K, V> &map) {
    os << '{';
    bool first = true;
    for (auto &[k,v]: map) {
        if (first) {
            first = false;
        } else {
            os << ", ";
        }
        os << k << ": " << v;
    }
    os << '}';
    return os;
}

int round_int(double n);

/**
 * @param duration_s Must be ≤ 4.294967295
 */
void sleep(double duration_s);
