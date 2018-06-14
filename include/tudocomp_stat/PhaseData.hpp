#pragma once

#include <string>
#include <memory>
#include <sstream>

#include <tudocomp_stat/json.hpp>

/// \cond INTERNAL

namespace tdc {
    using json = nlohmann::json;

class PhaseData {
private:
    static constexpr size_t STR_BUFFER_SIZE = 64;

    struct keyval {
        std::unique_ptr<keyval> next;
        char key[STR_BUFFER_SIZE];
        char val[STR_BUFFER_SIZE];

        inline keyval() {
        }
    };

    char m_title[STR_BUFFER_SIZE];

public:
    double time_start;
    double time_end;
    ssize_t mem_off;
    ssize_t mem_current;
    ssize_t mem_peak;

    std::unique_ptr<keyval> first_stat;

    PhaseData* first_child;
    PhaseData* next_sibling;

    inline PhaseData()
        : first_child(nullptr),
          next_sibling(nullptr) {
    }

    inline ~PhaseData() {
        if(first_child) delete first_child;
        if(next_sibling) delete next_sibling;
    }

    inline const char* title() const {
        return m_title;
    }

    inline void title(const char* title) {
        strncpy(m_title, title, STR_BUFFER_SIZE);
    }

    template<typename T>
    inline void log_stat(const char* key, const T& value) {
        std::unique_ptr<keyval> kv = std::make_unique<keyval>();

        std::stringstream ss;
        ss << value;

        strncpy(kv->key, key, STR_BUFFER_SIZE);
        strncpy(kv->val, ss.str().c_str(), STR_BUFFER_SIZE);

        if(first_stat) {
            keyval* last = first_stat.get();
            while(last->next) {
                last = last->next.get();
            }
            last->next = std::move(kv);
        } else {
            first_stat = std::move(kv);
        }
    }

    inline json to_json() const {
        json obj;
        obj["title"] = std::string(m_title);
        obj["timeStart"] = time_start;
        obj["timeEnd"] = time_end;
        obj["memOff"] = mem_off;
        obj["memPeak"] = mem_peak;
        obj["memFinal"] = mem_current;

        json stats = json::array();
        keyval* kv = first_stat.get();
        while(kv) {
            json pair;
            pair["key"] = std::string(kv->key);
            pair["value"] = std::string(kv->val);
            stats.push_back(pair);
            kv = kv->next.get();
        }
        obj["stats"] = stats;

        json sub = json::array();
        PhaseData* child = first_child;
        while(child) {
            sub.push_back(child->to_json());
            child = child->next_sibling;
        }
        obj["sub"] = sub;

        return obj;
    }
};

}

/// \endcond
