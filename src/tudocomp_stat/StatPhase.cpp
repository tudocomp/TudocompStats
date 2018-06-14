#include <tudocomp_stat/malloc.hpp>
#include <tudocomp_stat/StatPhase.hpp>

#ifndef STATS_DISABLED

using tdc::StatPhase;

StatPhase* StatPhase::s_current = nullptr;
uint16_t StatPhase::s_pause_guard_state = 0;
bool StatPhase::s_track_memory = false;

void malloc_callback::on_alloc(size_t bytes) {
    StatPhase::track_alloc(bytes);
}

void malloc_callback::on_free(size_t bytes) {
    StatPhase::track_free(bytes);
}

#endif
