#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

constexpr size_t kTapeSize = 30000;

// mapping every "[" with its "]"
struct Program {
    std::string code;
    std::unordered_map<size_t, size_t> jump;
};

Program compile(const std::string& source) {
    Program prog;
    prog.code.reserve(source.size());
    for (char c : source) {
        if (c == '>' || c == '<' || c == '+' || c == '-' ||
            c == '.' || c == ',' || c == '[' || c == ']') {
            prog.code.push_back(c);
        }
    }

    std::vector<size_t> stack;
    for (size_t i = 0; i < prog.code.size(); ++i) {
        if (prog.code[i] == '[') {
            stack.push_back(i);
        } else if (prog.code[i] == ']') {
            if (stack.empty()) {
                throw std::runtime_error("unmatched ']' at instruction " +
                                          std::to_string(i));
            }
            size_t open = stack.back();
            stack.pop_back();
            prog.jump[open] = i;
            prog.jump[i] = open;
        }
    }
    if (!stack.empty()) {
        throw std::runtime_error("unmatched '[' at instruction " +
                                  std::to_string(stack.back()));
    }
    return prog;
}

void run(const Program& prog) {
    std::vector<uint8_t> tape(kTapeSize, 0);
    size_t ptr = 0;
    size_t ip = 0;

    while (ip < prog.code.size()) {
        switch (prog.code[ip]) {
            case '>':
                ptr = (ptr + 1) % kTapeSize;
                break;
            case '<':
                ptr = (ptr == 0) ? kTapeSize - 1 : ptr - 1;
                break;
            case '+':
                ++tape[ptr];
                break;
            case '-':
                --tape[ptr];
                break;
            case '.':
                std::cout.put(static_cast<char>(tape[ptr]));
                break;
            case ',': {
                int in = std::cin.get();
                tape[ptr] = (in == EOF) ? 0 : static_cast<uint8_t>(in);
                break;
            }
            case '[':
                if (tape[ptr] == 0) ip = prog.jump.at(ip);
                break;
            case ']':
                if (tape[ptr] != 0) ip = prog.jump.at(ip);
                break;
            default:
                break;
        }
        ++ip;
    }
    std::cout.flush();
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("could not open file: " + path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <program.bf>\n";
        return 1;
    }

    try {
        std::string source = readFile(argv[1]);
        Program prog = compile(source);
        run(prog);
    } catch (const std::exception& e) {
        std::cerr << "bfvm: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
