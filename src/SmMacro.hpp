#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>
#include <cstdint>

namespace sm {

enum class EventType : uint8_t {
    Press = 0,
    Release = 1
};

struct InputEvent {
    uint64_t frame = 0;
    int button = 0;
    EventType type = EventType::Press;
};

struct Route {
    std::vector<InputEvent> events;
    uint64_t frames = 0;
};

enum class ExportFormat {
    SM,
    GDR,
    GDR2,
    ECHO
};

class RouteExporter {
public:
    static bool exportRoute(Route const& route, ExportFormat format, std::string const& path);
};

class Pathfinder {
public:
    void start();
    void stop();
    void retry();
    void verify();
    bool running() const;
    uint64_t attempts() const;
    uint32_t searchDepth() const;
    void setSearchDepth(uint32_t depth);

private:
    bool m_running = false;
    uint64_t m_attempts = 0;
    uint32_t m_searchDepth = 120;
};

void openSMMenu();

} // namespace sm
