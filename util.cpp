#include <cmath>
#include <iostream>
#include <cmath>
#include <vector>
#include <unordered_set>
#include <unordered_map>
using namespace std;

template<typename T0, typename T1>
ostream &operator<<(ostream &os, const pair<T0, T1> &p) {
    os << '(' << p.first << ", " << p.second << ')';
    return os;
}

template<typename T>
ostream &operator<<(ostream &os, const vector<T> &vec) {
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
ostream &operator<<(ostream &os, const unordered_set<T> &set) {
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
ostream &operator<<(ostream &os, const unordered_map<K, V> &map) {
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

int round_int(const double n) {
    return static_cast<int>(round(n));
}