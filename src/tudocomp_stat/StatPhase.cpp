#include <tudocomp_stat/malloc.hpp>
#include <tudocomp_stat/StatPhase.hpp>

#ifndef STATS_DISABLED

using tdc::StatPhase;

StatPhase* StatPhase::s_current = nullptr;
uint16_t StatPhase::s_suppress_memory_tracking_state = 0;
bool StatPhase::s_user_disabled_memory_tracking = false;

void malloc_callback::on_alloc(size_t bytes) {
    StatPhase::track_alloc(bytes);
}

void malloc_callback::on_free(size_t bytes) {
    StatPhase::track_free(bytes);
}

#endif
