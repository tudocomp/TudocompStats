#include <gtest/gtest.h>

#include <tudocomp_stat/StatPhase.hpp>
#include <tudocomp_stat/StatPhaseDummy.hpp>

#include <memory>

using namespace tdc;

// NB: If you end up getting a infinite recursion segfault here, its because
// of some bad interaction between gtest and json during printing of an
// assertion failure.

TEST(Tudostats, empty_phase) {
    tdc::StatPhase root("Root");
    {
    }
    auto j = root.to_json();
    std::cout << j.dump(4) << std::endl;

    ASSERT_EQ(j["memFinal"], 0);
    ASSERT_EQ(j["memOff"], 0);
    ASSERT_EQ(j["memPeak"], 0);
}

TEST(Tudostats, phase_root100) {
    tdc::StatPhase root("Root");
    {
        std::make_unique<char[]>(100);
    }
    auto j = root.to_json();
    std::cout << j.dump(4) << std::endl;

    ASSERT_EQ(j["memFinal"], 0);
    ASSERT_EQ(j["memOff"], 0);
    ASSERT_EQ(j["memPeak"], 100);
}

TEST(Tudostats, phase_root400_sub2_200) {
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

    ASSERT_EQ(j["memFinal"], 0);
    ASSERT_EQ(j["memOff"], 0);
    ASSERT_EQ(j["memPeak"], 500);

    auto s1 = j["sub"][0];
    ASSERT_EQ(int(s1["memFinal"]), 0);
    ASSERT_EQ(int(s1["memOff"]), 300);
    ASSERT_EQ(int(s1["memPeak"]), 100);

    auto s2 = j["sub"][1];
    ASSERT_EQ(int(s2["memFinal"]), 0);
    ASSERT_EQ(int(s2["memOff"]), 300);
    ASSERT_EQ(int(s2["memPeak"]), 200);
}

TEST(Tudostats, phaseroot_400_sub1_100_cross_mem) {
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

    ASSERT_EQ(j["memFinal"], 0);
    ASSERT_EQ(j["memOff"], 0);
    ASSERT_EQ(j["memPeak"], 400);

    auto s1 = j["sub"][0];
    ASSERT_EQ(int(s1["memFinal"]), 100);
    ASSERT_EQ(int(s1["memOff"]), 300);
    ASSERT_EQ(int(s1["memPeak"]), 100);

    auto s2 = j["sub"][1];
    ASSERT_EQ(int(s2["memFinal"]), -100);
    ASSERT_EQ(int(s2["memOff"]), 400);
    ASSERT_EQ(int(s2["memPeak"]), 0);
}
