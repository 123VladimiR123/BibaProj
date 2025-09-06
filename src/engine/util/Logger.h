//
// Created by Владимир on 09.08.2025.
//

#ifndef BIBAPROJ_LOGGER_H
#define BIBAPROJ_LOGGER_H

struct NullBuffer final : std::streambuf {
    int overflow(const int c) override { return c; }
};

class NullStream final : public std::ostream {
public:
    NullStream() : std::ostream(&nullBuffer) {}
private:
    NullBuffer nullBuffer;
};

class Logger {
    static NullStream nullStream;
    static std::ostream *stream;
public:
    static std::ostream &get() {
        return *stream;
    }
};

#endif //BIBAPROJ_LOGGER_H
