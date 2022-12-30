#include <array>
#include <iostream>

template <std::size_t Size>
using chars = std::array<char, Size>;

constexpr auto copy(const char* start, const char* end, char* dst) {
    while (start < end) {
        *dst++ = *start++;
    }
}

template <std::size_t N1, std::size_t N2>
constexpr auto concat(const chars<N1>& arr1, const chars<N2>& arr2) {
    chars<N1 + N2> res{};
    copy(arr1.begin(), arr1.end(), res.begin());
    copy(arr2.begin(), arr2.end(), res.begin() + N1);
    return res;
}

template <unsigned N>
constexpr auto to_chars() {
    constexpr char last_digit = '0' + N % 10;
    if constexpr (N >= 10) {
        return concat(to_chars<N / 10>(), chars<1>{last_digit});
    } else {
        return chars<1>{last_digit};
    }
}

template <unsigned N>
constexpr auto nFizzBuzz() {
    constexpr chars<4> FIZZ{'f', 'i', 'z', 'z'};
    constexpr chars<4> BUZZ{'b', 'u', 'z', 'z'};
    if constexpr (N % 15 == 0) {
        return concat(FIZZ, BUZZ);
    } else if constexpr (N % 3 == 0) {
        return FIZZ;
    } else if constexpr (N % 5 == 0) {
        return BUZZ;
    } else {
        return to_chars<N>();
    }
}

template <unsigned N>
constexpr auto fizzBuzz() {
    static_assert(N > 0);
    constexpr chars<2> SEPERATOR{',', ' '};
    if constexpr (N > 1) {
        return concat(fizzBuzz<N - 1>(), concat(SEPERATOR, nFizzBuzz<N>()));
    } else {
        return nFizzBuzz<N>();
    }
}

int main() {
    constexpr auto res = fizzBuzz<56>();
    for (const auto& c : res) {
        std::cout << c;
    }
    std::cout << std::endl;
    return 0;
}
