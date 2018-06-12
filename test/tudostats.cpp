#include <gtest/gtest.h>

#include <tudocomp_stat/StatPhase.hpp>
#include <tudocomp_stat/StatPhaseDummy.hpp>

#include <memory>

using namespace tdc;

TEST(Tudostats, empty_phase) {
    tdc::StatPhase root("Root");
    {
    }
    auto j = root.to_json();
    std::cout << j.dump(4) << std::endl;
}

TEST(Tudostats, phase_root_100) {
    tdc::StatPhase root("Root");
    {
        std::make_unique<char[]>(100);
    }
    auto j = root.to_json();
    std::cout << j.dump(4) << std::endl;
}

TEST(Tudostats, subphase_1) {
    tdc::StatPhase root("Root");
    {
        auto x1 = std::make_unique<char[]>(300);
        {
            tdc::StatPhase sub1("sub1");
            {
                std::make_unique<char[]>(100);
            }
            sub1.split("sub2");
            {
                std::make_unique<char[]>(200);
            }
        }
        auto x2 = std::make_unique<char[]>(100);
    }
    auto j = root.to_json();
    std::cout << j.dump(4) << std::endl;
}
TEST(Tudostats, subphase_2) {
    tdc::StatPhase root("Root");
    {
        auto x1 = std::make_unique<char[]>(300);
        {
            tdc::StatPhase sub1("sub1");
            auto cross_phase = std::make_unique<char[]>(100);
            sub1.split("sub2");
        }
        auto x2 = std::make_unique<char[]>(100);
    }
    auto j = root.to_json();
    std::cout << j.dump(4) << std::endl;
}
