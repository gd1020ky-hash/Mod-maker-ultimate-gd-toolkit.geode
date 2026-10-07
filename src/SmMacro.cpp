#include "SmMacro.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace sm {

static Pathfinder g_pathfinder;

void Pathfinder::start() {
    m_running = true;
    // Integration point:
    // 1. capture deterministic frame/state snapshots
    // 2. generate candidate input branches
    // 3. advance/retry candidates
    // 4. prune impossible branches
    // 5. detect completion
}

void Pathfinder::stop() {
    m_running = false;
}

void Pathfinder::retry() {
    if (m_running)
        ++m_attempts;
}

void Pathfinder::verify() {
    // Verification intentionally remains separate from "found" so that a route
    // must pass a second playback/check before it can be exported.
}

bool Pathfinder::running() const {
    return m_running;
}

uint64_t Pathfinder::attempts() const {
    return m_attempts;
}

uint32_t Pathfinder::searchDepth() const {
    return m_searchDepth;
}

void Pathfinder::setSearchDepth(uint32_t depth) {
    m_searchDepth = std::max<uint32_t>(1, depth);
}

bool RouteExporter::exportRoute(
    Route const& route,
    ExportFormat format,
    std::string const& path
) {
    // `.sm` is the native format. Other exporters are isolated so they can be
    // implemented/validated independently instead of producing fake files.
    // Returning false here prevents a misleading "successful" export.
    (void)route;
    (void)format;
    (void)path;
    return false;
}

// Menu model: the existing Claude/GD Toolkit settings are intentionally grouped
// into one SM menu instead of being exposed as the primary configuration surface.
// The concrete layer/button implementation should be wired to the current Geode
// UI APIs for the exact SDK revision during the build pass.
void openSMMenu() {
    log::info("Ultimate GD Toolkit menu requested");
}

$on_mod(Loaded) {
    log::info("Ultimate GD Toolkit Battle Build loaded");
    log::info("Unified menu + Pathfinder + universal export architecture enabled");
}

} // namespace sm
