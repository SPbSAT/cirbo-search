# Cirbo-Search library
Library for boolean circuit synthesis and minimization.

## (DEV) Setup environment

1. Install dev dependencies: `clang-tidy, clang-formatter`
1. Update submodules `git submodule update --init --recursive`.
2. Open project in IDE (e.g. `CLion`).
3. CMake profiles set in `CMakeProfiles.json` (for `CLion` one needs to enable them in profile setup section of settings)

For `DEBUG` profile:
  ```
  -DCMAKE_BUILD_TYPE=DEBUG -DCIRBO_SEARCH_BUILD_TESTS=1 -DCIRBO_SEARCH_APP_DEBUG=1 -DCIRBO_SEARCH_APP_SANITIZE=1
  ```

## Project Structure

Sources:
- `app/` contains source files for executables, which use main library.
- `` contains source files organized as header-only library.
- `third_party/` contains source files for third-party libraries.

Data:
- `database/` contains serialized database with small circuits of optimal size.
- `instances/` contains plenty of boolean circuit instances.

Auxiliary:
- `test/` contains source files for tests, that cover main library functionalities.
- `benchmarking/` contains source files for performance and quality benchmarking of the library functionalities.

### Library Structure

Directory `` contains main source files of library and is organized as follows:
- `core/` defines core types and structures:
  - `algorithms/` defines general algorithms for graphs and boolean circuits specifically.
  - `structure/` defines core (data) structs used in the library.
  - `logic.hpp` defines logical types and enums (e.g. `gate type`, `literal`, `arity`).
  - `operators.hpp` defines implementation of logical operators (e.g. `AND`, `NOT`, `XOR`).
- `io/` defines capabilities to read (parse) inputs and write results:
  - `parsers/` defines parsers of common data formats.
  - `writers/` defines functions for writing in common data formats.
- `circuits_db/` defines IO operations for database of small circuits.
- `minimization/` defines algorithms for circuit size minimization:
  - `low_effort/` defines simple and fast algorithms, which usually apply some common logic rules.
  - `local_search/` defines local search procedures for circuit minimization (e.g. subcircuit minimization).
- `synthesis/` defines algorithms for circuit synthesis:
  - `meta_heuristics/` defines meta-heuristical algorithms (e.g. genetic) for boolean circuit synthesis.
  - `sat/` defines SAT-based search algorithm for boolean circuit synthesis.
