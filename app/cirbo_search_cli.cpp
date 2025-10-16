#include <CLI/CLI.hpp>
#include <exception>
#include <iostream>

#include "core/logic.hpp"
#include "core/operators.hpp"

int main(int const argc, char** argv)
{
    try
    {
        CLI::App app{"Cirbo-Search CLI"};

        bool verbose = false;
        app.add_flag("-v,--verbose", verbose, "Verbose output");
        CLI11_PARSE(app, argc, argv);

        std::cout << "Cirbo-Search CLI running" << (verbose ? " (verbose)" : "") << "\n";

        // Just for includes testing.
        std::cout << static_cast<int>(cirbo::GateType::AND) << std::endl;
        std::cout << static_cast<int>(cirbo::op::AND(cirbo::GateState::FALSE, cirbo::GateState::TRUE)) << std::endl;
    }
    catch (std::exception const& exc)
    {
        std::cerr << "An exception occurred:\n";
        std::cerr << exc.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown exception occurred." << std::endl;
        return 1;
    }

    return 0;
}
