#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "core/structures/mutable_circuit.hpp"
#include "crossover.hpp"
#include "fitness.hpp"
#include "mutation.hpp"
#include "utils/log_utils.hpp"
#include "utils/random.hpp"

namespace cirbo::minimization::genetic
{
struct GeneticParams
{
    size_t generations     = 100;
    size_t population_size = 10;

    double mutation_rate = 0.0;
    // double mutation_rate  = 0.8;
    double elite_rate     = 0.1;
    double crossover_rate = 1.0;

    uint64_t seed = cirbo::utils::DefaultGlobalSeed;
};

struct GeneticConstants
{
    static constexpr size_t big_stats_period   = 50;
    static constexpr size_t small_stats_period = 10;
};

template<class CircuitT>
class GeneticAlgorithm
{
public:
    /**
     * @brief Runs the genetic algorithm for circuit minimization.
     * @param initial_circuit Circuit used as the starting point for the population.
     * @param parameters Configuration parameters controlling the algorithm.
     * @return Unique pointer to the best circuit found during evolution.
     *
     * The algorithm initializes a population from the initial circuit and
     * iteratively applies evaluation, selection, elitism, and mutation
     * to search for a smaller correct circuit.
     */
    std::unique_ptr<CircuitT> run(CircuitT const& initial_circuit, GeneticParams const& parameters = GeneticParams{})
    {
        rng.seed(static_cast<std::mt19937::result_type>(parameters.seed));

#ifdef CIRBO_ENABLE_GENETIC_LOGS
        std::ofstream mutation_log("genetic_mutations.txt");
#endif

        std::cerr << "Initial circuit size: " << initial_circuit.getActualNumberOfGatesWithoutNot() << " gates\n";

        std::vector<std::unique_ptr<CircuitT>> population;
        std::vector<std::unique_ptr<CircuitT>> new_population;
        std::vector<std::vector<std::string>> population_histories;
        std::vector<std::vector<std::string>> new_population_histories;

        population.reserve(parameters.population_size);
        new_population.reserve(parameters.population_size);
        population_histories.reserve(parameters.population_size);
        new_population_histories.reserve(parameters.population_size);

        for (size_t i = 0; i < parameters.population_size; ++i)
        {
            population.push_back(std::make_unique<CircuitT>(initial_circuit));
            population_histories.emplace_back();
        }

        FitnessFunction<CircuitT> fitness(initial_circuit);

        std::unique_ptr<CircuitT> best_circuit_ever = std::make_unique<CircuitT>(initial_circuit);
        size_t best_size_ever                       = initial_circuit.getActualNumberOfGatesWithoutNot();
        std::vector<std::string> best_history_ever;

        size_t correct_count = 0;

        size_t last_best_size = initial_circuit.getActualNumberOfGatesWithoutNot();

        std::vector<double> fitness_results;
        fitness_results.reserve(population.size());
        for (size_t generation = 0; generation < parameters.generations; ++generation)
        {
            fitness_results.clear();
            new_population.clear();
            new_population_histories.clear();

            correct_count             = 0;
            size_t best_this_gen_size = std::numeric_limits<size_t>::max();
            size_t best_this_gen_idx  = 0;

            for (size_t i = 0; i < population.size(); ++i)
            {
                double fitness_result = fitness.evaluateFitness(*population[i]);
                fitness_results.push_back(fitness_result);

                if (fitness.isCorrect(fitness_result))
                {
                    correct_count++;
                    size_t circuit_size = population[i]->getActualNumberOfGatesWithoutNot();

                    if (circuit_size < best_this_gen_size)
                    {
                        best_this_gen_size = circuit_size;
                        best_this_gen_idx  = i;
                    }
                }
            }

            if (best_this_gen_size < best_size_ever)
            {
                best_size_ever    = best_this_gen_size;
                best_circuit_ever = std::make_unique<CircuitT>(*population[best_this_gen_idx]);
                best_history_ever = population_histories[best_this_gen_idx];

                std::cerr << "\n*** NEW BEST CORRECT CIRCUIT at gen " << generation << ": " << best_size_ever
                          << " gates ***\n";
                std::cerr << "Improved from " << last_best_size << " gates\n";
                std::cerr << "Change history:\n";
#ifdef CIRBO_ENABLE_GENETIC_LOGS

                if (mutation_log.is_open())
                {
                    mutation_log << "\n*** NEW BEST CORRECT CIRCUIT at gen " << generation << ": " << best_size_ever
                                 << " gates ***\n";
                    mutation_log << "Improved from " << last_best_size << " gates\n";
                    mutation_log << "Change history:\n";
                }
                if (best_history_ever.empty())
                {
                    std::cerr << "  [0] initial circuit\n";
                    if (mutation_log.is_open())
                    {
                        mutation_log << "  [0] initial circuit\n";
                    }
                }
                else
                {
                    for (size_t step = 0; step < best_history_ever.size(); ++step)
                    {
                        std::cerr << "  [" << step + 1 << "] " << best_history_ever[step] << "\n";
                        if (mutation_log.is_open())
                        {
                            mutation_log << "  [" << step + 1 << "] " << best_history_ever[step] << "\n";
                        }
                    }
                }
#endif
                last_best_size = best_size_ever;
            }

            auto best_it                = std::max_element(fitness_results.begin(), fitness_results.end());
            size_t best_idx             = std::distance(fitness_results.begin(), best_it);
            double current_best_fitness = fitness_results[best_idx];
            size_t current_best_size    = population[best_idx]->getActualNumberOfGatesWithoutNot();

            if (generation % GeneticConstants::big_stats_period == 0 || generation == 0)
            {
                printGateStats(population, fitness_results, fitness, generation);
#ifdef CIRBO_ENABLE_SCATTER_STATS
                collectScatterStats(population, fitness, generation);
#endif
            }
            else if (generation % GeneticConstants::small_stats_period == 0)
            {
                size_t total_gates = 0;
                size_t min_gates   = std::numeric_limits<size_t>::max();
                size_t max_gates   = 0;

                for (size_t i = 0; i < population.size(); ++i)
                {
                    size_t gates = population[i]->getActualNumberOfGatesWithoutNot();
                    total_gates += gates;
                    min_gates = std::min(min_gates, gates);
                    max_gates = std::max(max_gates, gates);
                }

                double avg_gates = static_cast<double>(total_gates) / population.size();

                std::cerr << "Gen " << generation << " | sizes: min=" << min_gates << " avg=" << avg_gates
                          << " max=" << max_gates << " | correct=" << correct_count << "/" << population.size()
                          << " | best_ever=" << best_size_ever << " | best_fit=" << current_best_fitness << "\n";
            }

            size_t elite_count =
                std::max(static_cast<size_t>(parameters.population_size * parameters.elite_rate), size_t(1));

            std::vector<size_t> indices(population.size());
            for (size_t i = 0; i < population.size(); ++i) { indices[i] = i; }

            std::sort(
                indices.begin(),
                indices.end(),
                [&](size_t a, size_t b)
                {
                    bool a_correct = fitness.isCorrect(fitness_results[a]);
                    bool b_correct = fitness.isCorrect(fitness_results[b]);
                    size_t a_size  = population[a]->getActualNumberOfGatesWithoutNot();
                    size_t b_size  = population[b]->getActualNumberOfGatesWithoutNot();

                    if (a_correct && b_correct)
                    {
                        if (a_size != b_size)
                        {
                            return a_size < b_size;
                        }
                        return fitness_results[a] > fitness_results[b];
                    }
                    else if (a_correct && !b_correct)
                    {
                        return true;
                    }
                    else if (!a_correct && b_correct)
                    {
                        return false;
                    }
                    else
                    {
                        return fitness_results[a] > fitness_results[b];
                    }
                });

            for (size_t i = 0; i < elite_count && i < indices.size(); ++i)
            {
                new_population.push_back(std::make_unique<CircuitT>(*population[indices[i]]));
                new_population_histories.push_back(population_histories[indices[i]]);
            }
            
            while (new_population.size() < parameters.population_size)
            {
                size_t parent1_idx = tournamentSelection(fitness_results, population, fitness);

                std::unique_ptr<CircuitT> offspring;
                std::vector<std::string> offspring_history = population_histories[parent1_idx];

                bool do_crossover = randomDouble() < parameters.crossover_rate;

                if (do_crossover)
                {
                    size_t parent2_idx = tournamentSelection(fitness_results, population, fitness);

                    offspring_history.push_back(
                        "crossover selection: parent1_idx=" + std::to_string(parent1_idx)
                        + ", parent2_idx=" + std::to_string(parent2_idx));
                    // std::cerr << "[GA] crossover selected\n";
                    size_t const crossover_history_start = offspring_history.size();
                    offspring = layerCrossoverV3(
                        *population[parent1_idx],
                        *population[parent2_idx],
                        rng,
                        &offspring_history);

                    double const child_fitness = fitness.evaluateFitness(*offspring);
                    bool const child_correct   = fitness.isCorrect(child_fitness);
                    std::string const correctness_info =
                        ", child_fitness=" + std::to_string(child_fitness)
                        + ", child_correct=" + (child_correct ? std::string("true") : std::string("false"));

                    std::string crossover_log_entry = "crossover_v3: status=unknown";
                    if (offspring_history.size() > crossover_history_start)
                    {
                        crossover_log_entry = offspring_history.back();
                        offspring_history.back() += correctness_info;
                    }

                    std::ofstream crossover_log("crossover_v3_logs.txt", std::ios::app);
                    if (crossover_log.is_open())
                    {
                        crossover_log << crossover_log_entry << correctness_info << "\n";
                    }
                }
                else
                {
                    offspring = std::make_unique<CircuitT>(*population[parent1_idx]);
                }

                if (randomDouble() < parameters.mutation_rate)
                {
                    applyRandomMutation(offspring.get(), rng, &offspring_history);
                }

                new_population.push_back(std::move(offspring));
                new_population_histories.push_back(std::move(offspring_history));
            }

            population.swap(new_population);
            population_histories.swap(new_population_histories);
        }

        std::cerr << "\n=== FINAL RESULT ===\n";
        std::cerr << "Initial gates: " << initial_circuit.getActualNumberOfGatesWithoutNot() << "\n";
        std::cerr << "Final gates: " << best_size_ever << "\n";
        if (best_size_ever < initial_circuit.getActualNumberOfGatesWithoutNot())
        {
            std::cerr << "IMPROVEMENT: saved " << initial_circuit.getActualNumberOfGatesWithoutNot() - best_size_ever
                      << " gates!\n";
        }
        else
        {
            std::cerr << "No improvement found.\n";
        }

        double final_fitness = fitness.evaluateFitness(*best_circuit_ever);
        std::cerr << "Final circuit fitness: " << final_fitness << "\n";
        if (fitness.isCorrect(final_fitness))
        {
            std::cerr << "Final circuit is CORRECT\n";
        }
        else
        {
            std::cerr << "Final circuit is INCORRECT\n";
        }

        return std::move(best_circuit_ever);
    }

private:
    std::mt19937 rng;

#ifdef CIRBO_ENABLE_SCATTER_STATS
    void collectScatterStats(
        std::vector<std::unique_ptr<CircuitT>> const& population,
        FitnessFunction<CircuitT> const& fitness,
        size_t generation)
    {
        std::ofstream out("scatter_gen_" + std::to_string(generation) + ".csv");

        out << "size,ln_matches\n";

        for (auto const& circuit : population)
        {
            size_t size = circuit->getActualNumberOfGatesWithoutNot();

            size_t matches = fitness.getCorrectMatches(*circuit);

            double ln_matches = std::log((double)matches + 1.0);

            out << size << "," << ln_matches << "\n";
        }

        out.close();
    }
#endif

    /**
     * @brief Selects an individual using tournament selection.
     * @param fitness_results Vector containing fitness values of the population.
     * @param population Current population of circuits.
     * @param tournament_size Number of individuals participating in the tournament.
     * @return Index of the selected individual in the population.
     *
     * The best individual is chosen based on correctness, circuit size,
     * and fitness value depending on their validity.
     */
    size_t tournamentSelection(
        std::vector<double> const& fitness_results,
        std::vector<std::unique_ptr<CircuitT>> const& population,
        FitnessFunction<CircuitT> const& fitness,
        size_t tournament_size = 5)
    {
        size_t best_idx     = randomIndex(fitness_results.size());
        double best_fitness = fitness_results[best_idx];
        bool best_correct   = fitness.isCorrect(best_fitness);
        size_t best_size    = population[best_idx]->getActualNumberOfGatesWithoutNot();

        for (size_t i = 1; i < tournament_size; ++i)
        {
            size_t idx         = randomIndex(fitness_results.size());
            double fitness_idx = fitness_results[idx];
            size_t size        = population[idx]->getActualNumberOfGatesWithoutNot();
            bool b_correct     = fitness.isCorrect(fitness_idx);

            if (best_correct && b_correct)
            {
                if (size < best_size)
                {
                    best_fitness = fitness_idx;
                    best_idx     = idx;
                    best_size    = size;
                }
            }
            else if (b_correct && !best_correct)
            {
                best_fitness = fitness_idx;
                best_idx     = idx;
                best_size    = size;
                best_correct = true;
            }
            else if (!b_correct && !best_correct)
            {
                if (fitness_idx > best_fitness)
                {
                    best_fitness = fitness_idx;
                    best_idx     = idx;
                    best_size    = size;
                }
            }
        }

        return best_idx;
    }

    /**
     * @brief Generates a random index in the range [0, size).
     */
    size_t randomIndex(size_t size)
    {
        std::uniform_int_distribution<size_t> dist(0, size - 1);
        return dist(rng);
    }

    /**
     * @brief Generates a random index in the range [0, size).
     */
    double randomDouble()
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(rng);
    }

    /**
     * @brief Prints statistics about the current population.
     * @param population Vector of circuits in the current generation.
     * @param fitness_results Corresponding fitness values for each circuit.
     * @param generation Current generation number.
     *
     * Displays information such as gate count distribution, number of
     * correct circuits, and the smallest correct circuit found.
     */
    void printGateStats(
        std::vector<std::unique_ptr<CircuitT>> const& population,
        std::vector<double> const& fitness_results,
        FitnessFunction<CircuitT> const& fitness,
        size_t generation)
    {
        std::map<size_t, size_t> size_distribution;
        std::map<size_t, size_t> correct_size_distribution;

        size_t total_gates   = 0;
        size_t min_gates     = std::numeric_limits<size_t>::max();
        size_t max_gates     = 0;
        size_t correct_count = 0;

        for (size_t i = 0; i < population.size(); ++i)
        {
            size_t gates = population[i]->getActualNumberOfGatesWithoutNot();
            total_gates += gates;
            min_gates = std::min(min_gates, gates);
            max_gates = std::max(max_gates, gates);

            size_distribution[gates]++;

            if (fitness.isCorrect(fitness_results[i]))
            {
                correct_count++;
                correct_size_distribution[gates]++;
            }
        }

        std::cerr << "\n--- Generation " << generation << " ---\n";
        std::cerr << "Population size: " << population.size() << "\n";
        std::cerr << "Gate count: min=" << min_gates << ", max=" << max_gates << "\n";
        std::cerr << "Correct circuits: " << correct_count << "/" << population.size() << "\n";

        std::cerr << "\nSize distribution (all circuits):\n";
        for (auto const& [size, count] : size_distribution)
        {
            double percentage = 100.0 * count / population.size();
            std::cerr << "  " << size << " gates: " << count << " (" << percentage << "%)";

            if (correct_size_distribution.count(size))
            {
                double correct_percentage = 100.0 * correct_size_distribution[size] / count;
                std::cerr << " - correct: " << correct_size_distribution[size] << " (" << correct_percentage << "%)";
            }
            std::cerr << "\n";
        }

        size_t best_correct_size = std::numeric_limits<size_t>::max();
        for (auto const& [size, count] : correct_size_distribution)
        {
            if (size < best_correct_size && count > 0)
            {
                best_correct_size = size;
            }
        }

        if (best_correct_size < std::numeric_limits<size_t>::max())
        {
            std::cerr << "\nBest correct circuit size: " << best_correct_size << " gates\n";
        }

        std::cerr << "================================\n";
    }
};
}  // namespace cirbo::minimization::genetic