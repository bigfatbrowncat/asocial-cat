template<typename StringFunction>
static void splitString(const std::string &str, char delimiter, StringFunction f) {
    std::size_t from = 0;
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (str[i] == delimiter) {
            f(str, from, i);
            from = i + 1;
        }
    }
    if (from <= str.size())
        f(str, from, str.size());
}

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}
