// olv: replay a MOEX OrderLog CSV and print a per-instrument quality summary.
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "olv/order_book.hpp"
#include "olv/orderlog_csv.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: olv <orderlog.csv> [price_step=0.01]\n";
        return EXIT_FAILURE;
    }
    const std::string path = argv[1];
    const std::string step = argc > 2 ? argv[2] : "0.01";

    std::ifstream in(path);
    if (!in) {
        std::cerr << "cannot open " << path << '\n';
        return EXIT_FAILURE;
    }

    const auto parsed = olv::read_orderlog(in, step);
    std::cout << "parsed events: " << parsed.events.size()
              << ", unparsable lines: " << parsed.rejected_lines.size() << "\n\n";

    std::cout << std::left << std::setw(10) << "SECCODE" << std::right << std::setw(10) << "events"
              << std::setw(8) << "pass" << std::setw(8) << "repair" << std::setw(12) << "quarantine"
              << std::setw(8) << "reject" << std::setw(9) << "crossed" << std::setw(11) << "overfills"
              << '\n';
    for (const auto& [code, s] : olv::replay(parsed.events)) {
        std::cout << std::left << std::setw(10) << code << std::right << std::setw(10) << s.events
                  << std::setw(8) << s.passed << std::setw(8) << s.repaired << std::setw(12)
                  << s.quarantined << std::setw(8) << s.rejected << std::setw(9) << s.crossed
                  << std::setw(11) << s.overfills << '\n';
    }
    return EXIT_SUCCESS;
}
