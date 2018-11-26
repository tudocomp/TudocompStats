#pragma once

#include <cstring>
#include <ctime>
#include <string>
#include <memory>

#include <tudocomp_stat/json.hpp>

#ifndef STATS_DISABLED

#include <tudocomp_stat/PhaseData.hpp>

#include <time.h>
#include <sys/time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace tdc {

// Use clock_gettime in linux, clock_get_time in OS X.
inline void get_monotonic_time(struct timespec *ts){
#ifdef __MACH__
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
#else
  clock_gettime(CLOCK_MONOTONIC, ts);
#endif
}

/// \brief Provides access to runtime and memory measurement in statistics
///        phases.
///
/// Phases are used to track runtime and memory allocations over the course
/// of the application. The measured data can be printed as a JSON string for
/// use in the tudocomp charter for visualization or third party applications.
class StatPhase {
private:
    //////////////////////////////////////////
    // Memory tracking
    //////////////////////////////////////////

    static uint16_t s_suppress_memory_tracking_state;
    static bool s_user_disabled_memory_tracking;

    struct suppress_memory_tracking {
        inline suppress_memory_tracking(suppress_memory_tracking const&) = delete;
        inline suppress_memory_tracking() {
            s_suppress_memory_tracking_state++;
        }
        inline ~suppress_memory_tracking() {
            s_suppress_memory_tracking_state--;
        }
        inline static bool is_paused() {
            return s_suppress_memory_tracking_state != 0;
        }
    };

    inline void user_memory_pause() {
        s_user_disabled_memory_tracking = true;
    }

    inline void user_memory_resume() {
        s_user_disabled_memory_tracking = false;
    }

    inline bool currently_tracking_memory() {
        return !suppress_memory_tracking::is_paused()
            && !s_user_disabled_memory_tracking;
    }

    inline void track_alloc_internal(size_t bytes) {
        if(currently_tracking_memory()) {
            if (!m_data) abort(); // better not use DCHECK, in case it allocates

            m_data->mem_current += bytes;
            m_data->mem_peak = std::max(m_data->mem_peak, m_data->mem_current);
            if(m_parent) m_parent->track_alloc_internal(bytes);
        }
    }

    inline void track_free_internal(size_t bytes) {
        if(currently_tracking_memory()) {
            if (!m_data) abort(); // better not use DCHECK, in case it allocates

            m_data->mem_current -= bytes;
            if(m_parent) m_parent->track_free_internal(bytes);
        }
    }

    //////////////////////////////////////////
    // Other StatPhase state
    //////////////////////////////////////////

    static StatPhase* s_current;
    StatPhase* m_parent = nullptr;
    std::unique_ptr<PhaseData> m_data;

    bool m_disabled = false;

    inline static double current_time_millis() {
        timespec t;
        get_monotonic_time(&t);

        return double(t.tv_sec * 1000L) + double(t.tv_nsec) / double(1000000L);
    }

    inline void init(std::string&& title) {
        suppress_memory_tracking guard;

        m_parent = s_current;

        m_data = std::make_unique<PhaseData>();
        m_data->title(std::move(title));

        m_data->mem_off = m_parent ? m_parent->m_data->mem_current : 0;
        m_data->mem_current = 0;
        m_data->mem_peak = 0;

        m_data->time_end = 0;
        m_data->time_start = current_time_millis();

        s_current = this;
    }

    /// Finish the current Phase
    ///
    /// Returns a pointer to the PhaseData if it got added to a parent,
    /// or null if there is no parent.
    inline PhaseData* finish() {
        m_data->time_end = current_time_millis();

        suppress_memory_tracking guard;
        PhaseData* r = nullptr;

        if(m_parent) {
            // add data to parent's data
            r = m_data.get();
            m_parent->m_data->append_child(std::move(m_data));
        } else {
            // if this was the root, delete data
            m_data.reset();
        }

        // pop parent
        s_current = m_parent;

        return r;
    }

public:
    /// \brief Executes a lambda as a single statistics phase.
    ///
    /// The new phase is started as a sub phase of the current phase and will
    /// immediately become the current phase.
    ///
    /// In case the given lambda accepts a \c StatPhase reference parameter,
    /// the phase object will be passed to it for use during execution.
    ///
    /// \param title the phase title
    /// \param func  the lambda to execute
    /// \return the return value of the lambda
    template<typename F>
    inline static auto wrap(std::string&& title, F func) ->
        typename std::result_of<F(StatPhase&)>::type {

        StatPhase phase(std::move(title));
        return func(phase);
    }

    /// \brief Executes a lambda as a single statistics phase.
    ///
    /// The new phase is started as a sub phase of the current phase and will
    /// immediately become the current phase.
    ///
    /// In case the given lambda accepts a \c StatPhase reference parameter,
    /// the phase object will be passed to it for use during execution.
    ///
    /// \param title the phase title
    /// \param func  the lambda to execute
    /// \return the return value of the lambda
    template<typename F>
    inline static auto wrap(std::string&& title, F func) ->
        typename std::result_of<F()>::type {

        StatPhase phase(std::move(title));
        return func();
    }

    /// \brief Tracks a memory allocation of the given size for the current
    ///        phase.
    ///
    /// Use this only if memory is allocated with methods that do not result
    /// in calls of \c malloc but should still be tracked (e.g., when using
    /// direct kernel allocations like memory mappings).
    ///
    /// \param bytes the amount of allocated bytes to track for the current
    ///              phase
    inline static void track_alloc(size_t bytes) {
        if(s_current) s_current->track_alloc_internal(bytes);
    }

    /// \brief Tracks a memory deallocation of the given size for the current
    ///        phase.
    ///
    /// Use this only if memory is allocated with methods that do not result
    /// in calls of \c malloc but should still be tracked (e.g., when using
    /// direct kernel allocations like memory mappings).
    ///
    /// \param bytes the amount of freed bytes to track for the current phase
    inline static void track_free(size_t bytes) {
        if(s_current) s_current->track_free_internal(bytes);
    }

    /// \brief Pauses the tracking of memory allocations in the current phase.
    ///
    /// Memory tracking is paused until \ref pause_tracking is called or the
    /// phase object is destroyed.
    inline static void pause_tracking() {
        if(s_current) s_current->user_memory_pause();
    }

    /// \brief Resumes the tracking of memory allocations in the current phase.
    ///
    /// This only has an effect if tracking has previously been paused using
    /// \ref pause_tracking.
    inline static void resume_tracking() {
        if(s_current) s_current->user_memory_resume();
    }

    /// \brief Logs a user statistic for the current phase.
    ///
    /// User statistics will be stored in a special data block for a phase
    /// and is included in the JSON output.
    ///
    /// \param key the statistic key or name
    /// \param value the value to log (will be converted to a string)
    template<typename T>
    inline static void log(std::string&& key, const T& value) {
        if(s_current) s_current->log_stat(std::move(key), value);
    }

    /// \brief Creates a inert statistics phase without any effect.
    inline StatPhase() {
        m_disabled = true;
    }

    /// \brief Creates a new statistics phase.
    ///
    /// The new phase is started as a sub phase of the current phase and will
    /// immediately become the current phase.
    ///
    /// \param title the phase title
    inline StatPhase(std::string&& title) {
        init(std::move(title));
    }

    /// \brief Destroys and ends the phase.
    ///
    /// The phase's parent phase, if any, will become the current phase.
    inline ~StatPhase() {
        if (!m_disabled) {
            finish();
        }
    }

    /// \brief Starts a new phase as a sibling, reusing the same object.
    ///
    /// This function behaves exactly as if the current phase was ended and
    /// a new phases was started immediately after.
    ///
    /// \param new_title the new phase title
    inline void split(std::string&& new_title) {
        if (!m_disabled) {
            PhaseData* old_data = finish();

            init(std::move(new_title));
            if(old_data) {
                m_data->mem_off = old_data->mem_off + old_data->mem_current;
            }
        }
    }

    /// \brief Logs a user statistic for this phase.
    ///
    /// User statistics will be stored in a special data block for a phase
    /// and is included in the JSON output.
    ///
    /// \param key the statistic key or name
    /// \param value the value to log (will be converted to a string)
    template<typename T>
    inline void log_stat(std::string&& key, const T& value) {
        if (!m_disabled) {
            suppress_memory_tracking guard;
            m_data->log_stat(std::move(key), value);
        }
    }

    /// \brief Constructs the JSON representation of the measured data.
    ///
    /// It contains the subtree of phases beneath this phase.
    ///
    /// \return the \ref json::Object containing the JSON representation
    inline json to_json() {
        suppress_memory_tracking guard;
        if (!m_disabled) {
            m_data->time_end = current_time_millis();
            json obj = m_data->to_json();
            return obj;
        } else {
            return json();
        }
    }
};

}

#else

#include <tudocomp_stat/StatPhaseDummy.hpp>

namespace tdc {

    using StatPhase = StatPhaseDummy;

}

#endif
