#include <Geode/Geode.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <fstream>
#include <set>
using namespace geode::prelude;

struct Click { int f; bool down; int btn; bool p1; };
static std::vector<Click> g_macro;
static int g_frame = 0;
static size_t g_idx = 0;
static bool g_inject = false;
static CCLabelBMFont* g_label = nullptr;

static auto S() { return Mod::get(); }
static std::string mode() { return S()->getSettingValue<std::string>("macro-mode"); }
static auto macroPath() { return S()->getSaveDir() / "macro.bin"; }

// ---------- Speedhack + music sync ----------
class $modify(CCScheduler) {
    void update(float dt) {
        float sp = (float)S()->getSettingValue<double>("speed");
        CCScheduler::update(dt * sp);
        if (auto fm = FMODAudioEngine::sharedEngine())
            if (fm->m_backgroundMusicChannel)
                fm->m_backgroundMusicChannel->setPitch(
                    S()->getSettingValue<bool>("sync-music") ? sp : 1.f);
    }
};

// ---------- Macro + frame counter ----------
class $modify(GJBaseGameLayer) {
    void processCommands(float dt) {
        if (PlayLayer::get()) {
            if (mode() == "play")
                while (g_idx < g_macro.size() && g_macro[g_idx].f <= g_frame) {
                    auto& c = g_macro[g_idx++];
                    g_inject = true;
                    this->handleButton(c.down, c.btn, c.p1);
                    g_inject = false;
                }
            g_frame++;
            if (g_label && S()->getSettingValue<bool>("show-frames"))
                g_label->setString(fmt::format("Frame: {}", g_frame).c_str());
        }
        GJBaseGameLayer::processCommands(dt);
    }
    void handleButton(bool down, int btn, bool p1) {
        GJBaseGameLayer::handleButton(down, btn, p1);
        if (!PlayLayer::get()) return;
        if (!g_inject && mode() == "record") g_macro.push_back({g_frame, down, btn, p1});
        auto snd = S()->getSettingValue<std::string>("sound-file");
        if (down && !snd.empty() && S()->getSettingValue<bool>("click-sound"))
            FMODAudioEngine::sharedEngine()->playEffect(snd);
    }
};

class $modify(PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        g_frame = 0; g_idx = 0;
        if (mode() == "play") {
            g_macro.clear();
            std::ifstream f(macroPath(), std::ios::binary);
            Click c;
            while (f.read((char*)&c, sizeof c)) g_macro.push_back(c);
        }
        g_label = CCLabelBMFont::create("Frame: 0", "bigFont.fnt");
        g_label->setScale(0.4f);
        g_label->setAnchorPoint({0, 1});
        g_label->setPosition(5, CCDirector::get()->getWinSize().height - 5);
        g_label->setZOrder(100);
        this->addChild(g_label);
    }
    void resetLevel() {
        PlayLayer::resetLevel();
        g_frame = 0; g_idx = 0;
        if (mode() == "record") g_macro.clear();
    }
    void onQuit() {
        if (mode() == "record") {
            std::ofstream f(macroPath(), std::ios::binary);
            for (auto& c : g_macro) f.write((const char*)&c, sizeof c);
        }
        g_label = nullptr;
        PlayLayer::onQuit();
    }
};

// ---------- AI build popup (Geode v5) ----------
class AIPopup : public Popup {
    LevelEditorLayer* m_ed = nullptr;
    TextInput* m_in = nullptr;
    async::TaskHolder<web::WebResponse> m_task;
public:
    static AIPopup* create(LevelEditorLayer* e) {
        auto r = new AIPopup();
        if (r->init(e)) { r->autorelease(); return r; }
        delete r; return nullptr;
    }
protected:
    bool init(LevelEditorLayer* e) {
        if (!Popup::init(360.f, 160.f)) return false;
        m_ed = e;
        this->setTitle("AI Build");
        m_in = TextInput::create(320, "Theme & details (e.g. neon cyber city)");
        m_mainLayer->addChildAtPosition(m_in, Anchor::Center, {0, 5});
        auto b = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Build"), this, menu_selector(AIPopup::onGo));
        m_buttonMenu->addChildAtPosition(b, Anchor::Bottom, {0, 28});
        return true;
    }
    void onGo(CCObject*) {
        float minX = 1e9, maxX = -1e9, maxY = 0;
        CCObject* o;
        CCARRAY_FOREACH(m_ed->m_objects, o) {
            auto p = static_cast<GameObject*>(o)->getPosition();
            minX = std::min(minX, p.x); maxX = std::max(maxX, p.x); maxY = std::max(maxY, p.y);
        }
        if (minX > maxX) { minX = 0; maxX = 3000; maxY = 300; }
        auto prompt = fmt::format(
            "Geometry Dash decoration generator. Theme: {}. Return ONLY a JSON array of up to 150 "
            "objects like [{{\"id\":211,\"x\":300,\"y\":90}}]. Use non-solid decoration object IDs, "
            "x between {} and {}, y between 0 and {}, all multiples of 15.",
            m_in->getString(), (int)minX, (int)maxX, (int)maxY + 150);
        auto body = matjson::makeObject({
            {"model", S()->getSettingValue<std::string>("model")},
            {"max_tokens", 4000},
            {"messages", std::vector<matjson::Value>{
                matjson::makeObject({{"role", "user"}, {"content", prompt}})}}});
        web::WebRequest req;
        req.header("x-api-key", S()->getSettingValue<std::string>("api-key"));
        req.header("anthropic-version", "2023-06-01");
        req.bodyJSON(body);
        m_task.spawn(req.post("https://api.anthropic.com/v1/messages"),
            [this](web::WebResponse res) { this->onRes(res); });
        Notification::create("AI is building...", NotificationIcon::Loading)->show();
    }
    void onRes(web::WebResponse res) {
        if (!res.ok()) return (void)Notification::create("Request failed", NotificationIcon::Error)->show();
        auto j = res.json().unwrapOr(matjson::Value());
        auto t = j["content"][0]["text"].asString().unwrapOr("");
        auto a = t.find('['), z = t.rfind(']');
        if (a == std::string::npos || z == std::string::npos) return;
        auto arr = matjson::parse(t.substr(a, z - a + 1)).unwrapOr(matjson::Value());
        int n = 0;
        for (auto& o : arr.asArray().unwrapOr({})) {
            int id = o["id"].asInt().unwrapOr(0);
            if (id <= 0) continue;
            m_ed->createObject(id, {(float)o["x"].asDouble().unwrapOr(0), (float)o["y"].asDouble().unwrapOr(0)}, false);
            n++;
        }
        Notification::create(fmt::format("Placed {} objects", n), NotificationIcon::Success)->show();
    }
};

// ---------- Editor buttons + auto deco ----------
class $modify(MyEditorPause, EditorPauseLayer) {
    void customSetup() {
        EditorPauseLayer::customSetup();
        auto m = CCMenu::create();
        m->setPosition(75, 90);
        auto b1 = CCMenuItemSpriteExtra::create(ButtonSprite::create("Auto Deco"), this, menu_selector(MyEditorPause::onDeco));
        auto b2 = CCMenuItemSpriteExtra::create(ButtonSprite::create("AI Build"), this, menu_selector(MyEditorPause::onAI));
        b2->setPosition(0, -38);
        m->addChild(b1); m->addChild(b2);
        this->addChild(m);
    }
    void onDeco(CCObject*) {
        std::set<std::pair<int, int>> cells;
        std::vector<CCPoint> tops;
        CCObject* o;
        CCARRAY_FOREACH(m_editorLayer->m_objects, o) {
            auto g = static_cast<GameObject*>(o);
            if (g->m_objectType != GameObjectType::Solid) continue;
            auto p = g->getPosition();
            cells.insert({(int)p.x / 30, (int)p.y / 30});
            tops.push_back(p);
        }
        int id = (int)S()->getSettingValue<int64_t>("deco-id"), n = 0;
        for (auto& p : tops)
            if (!cells.count({(int)p.x / 30, (int)p.y / 30 + 1})) {
                m_editorLayer->createObject(id, {p.x, p.y + 30}, false);
                n++;
            }
        Notification::create(fmt::format("Added {} deco", n), NotificationIcon::Success)->show();
    }
    void onAI(CCObject*) { AIPopup::create(m_editorLayer)->show(); }
};
