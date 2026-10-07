#include "Pathfinder.hpp"

namespace sm::pathfinder {

SearchEngine::SearchEngine(SearchConfig config) : m_config(config) {}

void SearchEngine::reset() {
    m_running = false;
    m_attempts = 0;
    m_expandedStates = 0;
}

void SearchEngine::begin() {
    m_running = true;
}

void SearchEngine::cancel() {
    m_running = false;
}

bool SearchEngine::running() const {
    return m_running;
}

uint64_t SearchEngine::attempts() const {
    return m_attempts;
}

uint64_t SearchEngine::expandedStates() const {
    return m_expandedStates;
}

SearchConfig const& SearchEngine::config() const {
    return m_config;
}

} // namespace sm::pathfinder
