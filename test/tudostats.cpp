#include <gtest/gtest.h>

#include <StatPhase.hpp>

#include <memory>

using namespace tdc;

TEST(Tudostats, empty_phase) {
    tdc::StatPhase root("Root");
    {
        auto x = std::make_unique<char[]>(128);
        for (size_t i = 0; i < 128; i++) {
            x[i] = i;
            std::cout << int(x[i]) << ",";
        }
        std::cout << std::endl;
        auto y = malloc(128);
        *(char*)y = 128;
        free(y);
        std::vector<char> z { 1, 2, 3, 4 };
    }
    root.to_json().str(std::cout);
    std::cout << std::endl;
}

TEST(Tudostats, subphase_1) {
    tdc::StatPhase root("Root");
    {
        tdc::StatPhase sub1("sub1");
        {
            auto x = std::make_unique<char[]>(128);
            for (size_t i = 0; i < 128; i++) {
                x[i] = i;
                std::cout << int(x[i]) << ",";
            }
            std::cout << std::endl;
        }
        sub1.split("sub2");
        {
            auto x = std::make_unique<char[]>(128);
            for (size_t i = 0; i < 128; i++) {
                x[i] = i;
                std::cout << int(x[i]) << ",";
            }
            std::cout << std::endl;
        }
    }
    root.to_json().str(std::cout);
    std::cout << std::endl;
}
