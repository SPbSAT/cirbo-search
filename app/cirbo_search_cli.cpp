#include "core/logic.hpp"
#include "core/operators.hpp"

#include <CLI/CLI.hpp>

#include <iostream>
#include <cstdint>

int main(int argc, char** argv) {
    CLI::App app{"Cirbo-Search CLI"};

    bool verbose = false;
    app.add_flag("-v,--verbose", verbose, "Verbose output");
    CLI11_PARSE(app, argc, argv);

    std::cout << "Cirbo-Search CLI running" << (verbose ? " (verbose)" : "") << "\n";

    // Just for includes testing.
    std::cout << static_cast<int>(cirbo::GateType::AND) << std::endl;

    return 0;
}
