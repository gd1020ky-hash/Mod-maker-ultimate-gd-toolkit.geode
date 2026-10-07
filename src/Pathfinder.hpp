#pragma once
#include "SmMacro.hpp"

namespace sm::pathfinder {

struct SearchConfig {
    uint32_t maxDepth = 120;
    uint32_t maxAttempts = 100000;
    uint32_t branchLimit = 8;
    bool useCheckpoints = true;
    bool verifyBeforeExport = true;
};

class SearchEngine {
public:
    explicit SearchEngine(SearchConfig config = {});
    void reset();
    void begin();
    void cancel();
    bool running() const;
    uint64_t attempts() const;
    uint64_t expandedStates() const;
    SearchConfig const& config() const;

private:
    SearchConfig m_config;
    bool m_running = false;
    uint64_t m_attempts = 0;
    uint64_t m_expandedStates = 0;
};

} // namespace sm::pathfinder
