#include <CLI/CLI.hpp>
#include <chrono>
#include <exception>
#include <iostream>

#include "core/algo.hpp"
#include "core/operators.hpp"
#include "core/types.hpp"
#include "io/parsers/bench_to_circuit.hpp"
#include "io/parsers/ibench_parser.hpp"
#include "io/writers/write_utils.hpp"
#include "logger.hpp"
#include "minimization/nest.hpp"
#include "minimization/strategy.hpp"

namespace
{

using namespace cirbo;

/**
 * Helper for file stream opening.
 */
std::ifstream openFileStream(std::string const& file_path)
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        log::error("Can't open file, path is incorrect.");
        std::abort();
    }
    log::debug("File opened.");
    return file;
}

/**
 * Writes resulting circuit either to an output file, or to the stdout if first is not given.
 */
void writeResult(
    DAG const& simplified_circuit,
    utils::NameEncoder const& encoder,
    std::string const& output_path)
{
    std::ofstream file_out(output_path);
    io::writers::writeBenchFile(simplified_circuit, encoder, file_out);
}

/**
 * Helper to run specific simplification strategies on the circuit.
 */
std::tuple<std::unique_ptr<DAG>, std::unique_ptr<utils::NameEncoder> > applySimplification(
    std::unique_ptr<DAG>& csat_instance,
    utils::NameEncoder& encoder)
{
    return minimization::Composition<
            DAG,
            minimization::DuplicateGatesCleaner<DAG>,
            minimization::DuplicateOperandsCleaner<DAG>>().apply(*csat_instance, encoder);
}

/**
 * Performs minimization of the given circuit located at the `input_path`.
 *
 * @param input_file path to the input circuit.
 */
void minimize(
    std::string const& input_file,
    std::string const& output_file)
{

    log::debug("Opening circuit file at ", input_file, ".");
    auto fstream = openFileStream(input_file);

    log::debug("Parsing a circuit file ", input_file, ".");
    io::parsers::BenchToCircuit<DAG> parser{};
    parser.parseStream(fstream);

    auto encoder       = parser.getEncoder();
    auto csat_instance = parser.instantiate();

    // Start simplification step.
    log::debug(input_file, ": minimization start.");
    auto [simplified_instance, simplified_encoder] = applySimplification(csat_instance, encoder);
    log::debug(input_file, ": simplification end.");

    auto timeEnd           = std::chrono::steady_clock::now();

    writeResult(*simplified_instance, *simplified_encoder, output_file);
}

} // namespace

int main(int const argc, char** argv)
{
    try
    {
        CLI::App app{"Cirbo-Search CLI"};

        std::string input_file;
        std::string output_file;
        app.add_option("-i,--input-path", input_file,"directory with input .BENCH files (or a single .BENCH file)");
        app.add_option("-o,--output", output_file, "path to resulting directory or to a resulting single .BENCH file");

        CLI11_PARSE(app, argc, argv);

        minimize(input_file, output_file);
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
