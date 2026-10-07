#include <SFML/Graphics.hpp>
#include <windows.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <functional>
#include "Header.h"
using namespace std;
using Col = sf::Color;
using Vec = sf::Vector2f;
const float PI = 3.14159265f;
const float W = 1200.f, H = 800.f;
const unsigned FPS = 60;
const float MAX_DT = 0.08f;
const char* const FONT_FILE = "font.ttf";
const char* const FISH_FILE = "fish_data.txt";
const wchar_t* const APP_TITLE = L"Рыбоводческое хозяйство";

struct Rect {
    float x, y, w, h;
    bool has(float px, float py) const { return px >= x && px <= x + w && py >= y && py <= y + h; }
};
struct Grad { Col a, b; };
struct TS { unsigned size; Col color; TS with(Col c) const { return { size, c }; } };
struct BoxStyle { float radius; Col top, bottom, border; float borderW; };
struct BtnStyle { Col top, bottom, text; };

namespace Clr {
    const Col text(238, 244, 240), gold(255, 215, 132), muted(170, 195, 190);
    const Col good(130, 222, 140), bad(255, 120, 110), title(255, 244, 214);
    const Col hint(240, 246, 250), sceneHint(30, 50, 70);
    const Col windowBg(18, 35, 40), sidebarBg(14, 26, 34), graphBg(14, 26, 44);
    const Col dim(0, 0, 0, 150), finishDim(6, 10, 20, 170), selection(255, 219, 107);
    const Col barWeek(84, 190, 206), barContract(255, 215, 132), barFeed(214, 140, 80);
    const Col inputText(20, 30, 40);
}
namespace FS { const unsigned cap = 12, note = 13, label = 14, body = 15, sub = 17, large = 19, head = 22, input = 24, title = 28, huge = 32; }
namespace T {
    const TS title{ FS::title, Clr::title }, huge{ FS::huge, Clr::text }, hint{ FS::body, Clr::hint };
    const TS head{ FS::head, Clr::gold }, body{ FS::body, Clr::text }, bodyGold{ FS::body, Clr::gold };
    const TS label{ FS::label, Clr::text }, labelGold{ FS::label, Clr::gold }, labelMuted{ FS::label, Clr::muted };
    const TS note{ FS::note, Clr::text }, noteMuted{ FS::note, Clr::muted };
    const TS cap{ FS::cap, Clr::text }, capMuted{ FS::cap, Clr::muted };
    const TS sceneHint{ FS::label, Clr::sceneHint }, largeGold{ FS::large, Clr::gold };
    const TS input{ FS::input, Clr::inputText };
}
namespace Box {
    const BoxStyle panel{ 12, Col(26, 46, 56, 235), Col(14, 28, 36, 240), Col(124, 176, 196, 120), 1.4f };
    const BoxStyle field{ 8, Col(10, 20, 30), Col(6, 14, 22), Col::Transparent, 0 };
    const BoxStyle input{ 9, Col(240, 240, 236), Col(214, 218, 220), Col::Transparent, 0 };
    const BoxStyle tooltip{ 8, Col(10, 20, 30, 240), Col(10, 20, 30, 240), Col::Transparent, 0 };
    const BoxStyle tag{ 8, Col(18, 32, 44, 215), Col(10, 20, 30, 225), Col::Transparent, 0 };
    const BoxStyle track{ 4, Col(8, 16, 24), Col(20, 32, 44), Col::Transparent, 0 };
}
enum BtnKind { B_NORMAL, B_PRIMARY, B_DANGER };
const BtnStyle BTN_STYLES[] = {
    { Col(46, 82, 90),   Col(28, 52, 62),  Clr::text },
    { Col(236, 190, 96), Col(190, 134, 50), Col(50, 34, 10) },
    { Col(176, 84, 76),  Col(128, 52, 50), Clr::text },
};
const BtnStyle BTN_DISABLED{ Col(58, 62, 66), Col(44, 48, 52), Col(135, 140, 145) };
namespace Btn {
    const float radius = 8, borderW = 1.2f, textLift = 3;
    const Col border(142, 185, 176, 130);
}
namespace Bar { const float h = 8, minFill = 8; }
namespace Lay {
    const float fieldW = 868.f, panelX = 862.f, panelW = W - 868.f - 8.f;
    const Rect sidebar{ 852.f, 0, W - 852.f, H };
    const Rect stats{ panelX, 4, panelW, 192 }, contract{ panelX, 198, panelW, 130 };
    const Rect pond{ panelX, 340, panelW, 262 }, logBox{ panelX, 698, panelW, 98 };
    const float padX = 14, padTop = 6, lineGap = 6, barGap = 14, gridGap = 4;
    const float mainBtnH = 38, row2H = 32, row2Gap = 6;
    const float actBtnH = 30, spBtnH = 31, spGap = 5, actGap = 4, spTop = 2;
    const int logLines = 5; const size_t logChars = 46;
    const float logPadY = 4, logLineH = 18;
    const float arrowW = 11, arrowH = 9, arrowGap = 6, priceColW = 130.f;
    const float hintX = 20, hintY = 12, controlsY = 616;
}
struct Block { float x, y, w, rowH; };
namespace Menu {
    const float titleX = 40, titleY = 20, hintY = 62;
    const Block w1{ 50, 250, 520, 46 }, w2{ 640, 250, 520, 46 };
    const Block b1{ 50, 224, 520, 42 }, b2{ 640, 224, 520, 42 };
    const int wSplit = 8, bSplit = 8;
    const float padX = 14, padTop = 12, padBottom = 6;
    const Rect priceBox{ 626, 520, 548, 128 };
    const float priceTextX = 14, priceHeadY = 6, priceRowsY = 30, priceRowH = 23, priceColW = 270;
    const int pricePerCol = 4, priceMax = 8;
    const Rect btnPlay{ 50, 700, 250, 50 }, btnBot{ 320, 700, 250, 50 };
    const Rect btnBack{ 50, 726, 180, 48 }, btnCmp{ 420, 726, 200, 48 }, btnRun{ 700, 724, 300, 52 };
    const Rect cmpBox{ 36, 580, 1128, 128 };
    const float cmpTextX = 14, cmpHeadY = 8, cmpEmptyY = 40, cmpRowsY = 32, cmpRowH = 22;
    const float cmpCols[5] = { 14, 264, 524, 644, 864 };
}
namespace SetRow {
    const float stepW = 34, btnH = 28, valueW = 112, gap = 4;
    const float controlsW = 4 * stepW + valueW + 4 * gap, textPadY = 5;
}
namespace Dlg {
    const float w = 480, h = 290, pad = 30;
    const float titlePadX = 24, titlePadY = 18;
    const float inputY = 64, inputH = 44, inputPadX = 14, inputPadY = 7;
    const float chipsY = 122, chipH = 32, chipPadX = 14, chipGap = 8;
    const float buttonsY = 210, btnW = 150, btnH = 50, okX = 60, cancelX = 270;
    const size_t maxDigits = 9; const float cursorPeriod = 1.f;
}
namespace Chart {
    const Rect panel{ 60, 114, 1080, 580 }, plot{ 140, 160, 960, 470 };
    const float titleY = 16, tabsY = 62, tabsX = 150, tabW = 210, tabStep = 230, tabH = 40;
    const Rect btnBack{ 500, 720, 200, 50 };
    const int gridLines = 5;
    const float lineW = 3, pointR = 3.5f;
    const Col gridMain(255, 255, 255, 90), gridMinor(255, 255, 255, 26), gridVert(255, 255, 255, 14);
    const Col cursorLine(255, 255, 255, 90);
    const float axisGap = 8, axisLift = 8, weekGap = 6, weekShiftX = 5;
    const int maxWeekTicks = 12;
    const float tipW = 240, tipPad = 10, tipOffset = 14, tipRowH = 20, tipHeadH = 28;
    const float legendY = 28, swatch = 14, legendGap = 6, legendItemGap = 32;
    const float emptyX = 380, emptyY = 380;
}
namespace Fin {
    const Rect box{ 300, 90, 600, 620 };
    const float titleX = 40, titleY = 24;
    const float rowsY = 170, rowStep = 38, rowPadX = 50, rowLineY = 28;
    const Col rowLine(255, 255, 255, 24);
    const float buttonsY = 80, btnW = 160, btnH = 50, btnPad = 40, btnStep = 180;
}
namespace Scene {
    const float horizon = 215;
    const Col skyTop(70, 140, 220), skyBottom(190, 225, 248);
    const Grad sunGlow{ Col(255, 238, 170, 130), Col(255, 238, 170, 0) };
    const Col sunCore(255, 241, 190);
    const Vec sun{ 720, 80 };
    const float glowR = 100, coreR = 30; const int sunSeg = 32;
    struct Peak { float x, h; };
    struct Ridge {
        vector<Peak> peaks; float halfW; Col color;
        Col snow; float snowDepth, snowHalfW;
    };
    const vector<Ridge> ridges = {
        { { {60, 80}, {220, 110}, {390, 70}, {570, 115}, {750, 75}, {940, 105}, {1120, 80} },
        170, Col(112, 132, 170), Col(240, 246, 252), 0.28f, 48 },
        { { {0, 45}, {200, 60}, {420, 40}, {640, 62}, {860, 45}, {1060, 58} },
        190, Col(74, 112, 104), Col::White, 0, 0 },
    };
    const Col grassTop(92, 148, 84), grassBottom(40, 86, 54);
}
namespace Ponds {
    const float left = 28, top = 205, right = 835, bottom = H - 10;
    const float rxOfCell = 0.38f, ryOfCell = 0.28f, maxAspect = 0.62f;
    const float jitX = 12, jitY = 10, shiftY = 22;
    const int outlinePts = 52;
    struct Wob { int n; float amp; };
    const Wob wob[] = { {2, 0.085f}, {3, 0.052f}, {5, 0.035f} };
    const float clickR = 1.2f;
    const float labelW = 150, labelH = 36, labelRise = 1.22f, labelGap = 40;
    const float labelPadX = 10, line1 = 3, line2 = 19;
    int columns(int n) { return n <= 3 ? n : (n == 4 ? 2 : (n <= 6 ? 3 : 4)); }
}
namespace Art {
    const float bankS = 1.16f, edgeS = 1.04f, glowS = 0.89f, ringS = 0.7f, selS = 1.28f;
    const Vec shadowOff{ 3, 8 }, glowOff{ -0.06f, -0.08f };
    const Grad shadow{ Col(48, 61, 43), Col(94, 112, 66) };
    const Grad edge{ Col(125, 113, 76), Col(185, 168, 116) };
    const Grad dry{ Col(92, 70, 46), Col(130, 100, 66) };
    const Grad cleanW{ Col(18, 89, 103), Col(45, 145, 157) }, cleanG{ Col(20, 115, 132), Col(50, 165, 175) };
    const Grad dirtyW{ Col(86, 74, 48), Col(132, 112, 70) }, dirtyG{ Col(96, 84, 56), Col(120, 102, 66) };
    const Col bankLine(65, 82, 48), edgeLine(216, 199, 143), shoreLine(133, 220, 216, 180);
    const Col ring(230, 247, 250, 170), crack(60, 44, 28, 170);
    const float bankLW = 4, edgeLW = 3, shoreLW = 2, ringLW = 2, selLW = 3, crackLW = 1.4f;
    const int nCracks = 6;
    const Col ripple(204, 246, 237, 60);
    const int nRipples = 7, rippleSeg = 12;
    const int nStones = 4;
    const Col stone(90, 95, 88), stoneHi(170, 177, 161);
    const int nLilies = 3;
    const Grad lily{ Col(48, 111, 56), Col(96, 157, 69) };
    const float lilyRx = 8, lilyRy = 4; const int lilySeg = 12;
}
namespace Fish {
    const float youngLen = 7, oldLen = 12, youngH = 3.5f, oldH = 6.5f;
    const float bodyHL = 0.64f, backHL = 0.38f, backHH = 0.58f;
    const Vec backShift{ 0.1f, 0.25f };
    const int bodySeg = 18, backSeg = 14;
    const array<Vec, 3> tail{ Vec{ -0.35f, 0 }, Vec{ -0.92f, -0.65f }, Vec{ -0.92f, 0.65f } };
    const array<array<Vec, 3>, 2> fins = { {
        { Vec{ -0.08f, -0.20f }, Vec{ 0.18f, -1.05f }, Vec{ 0.28f,  0.05f } },
        { Vec{ -0.08f,  0.20f }, Vec{ 0.16f,  0.95f }, Vec{ 0.30f, -0.02f } } } };
    const Vec eye{ 0.4f, -0.12f };
    const float eyeSize = 1.25f, eyeMin = 1.1f, pupilSize = 0.65f, pupilMin = 0.6f, pupilShift = 0.7f;
    const int eyeSeg = 8, dotSeg = 8;
    const Col eyeWhite(248, 246, 220), pupil(20, 28, 32);
    const int spots = 5; const float spotFirst = -0.17f, spotStep = 0.17f, spotR = 0.16f;
    const int stripes = 3; const float stripeFirst = -0.12f, stripeStep = 0.22f;
    const float stripeFrom = -0.55f, stripeTo = 0.5f, stripeW = 1.4f;
    const int dots = 4; const float dotFirst = -0.08f, dotStep = -0.13f, dotR = 0.08f;
    const Vec whiskerRoot{ 0.45f, 0.1f }, whiskerA{ 0.85f, 0.6f }, whiskerB{ 0.85f, -0.4f };
    const float whiskerW = 1.f;
}
namespace Sim {
    const int minFish = 2, maxFish = 34;
    const float perSqrtKg = 2.4f, spawnR = 0.68f;
    const float youngSpd = 0.35f, oldSpd = 0.23f, spdSpread = 0.18f;
    const float sizeLo = 0.78f, sizeHi = 1.2f;
    const float firstTurn = 1.5f, turnLo = 0.6f, turnHi = 2.2f, turnPow = 1.7f;
    const float driftAmpX = 0.3f, driftAmpY = 0.28f, driftFreqX = 0.13f, driftFreqY = 0.11f;
    const float pullMax = 0.6f, pullPerDist = 0.45f;
    const float wallR2 = 0.6f, wallPull = 2.5f, hardLimit = 0.84f;
    const float drawScaleX = 0.78f, drawScaleY = 0.72f;
}
namespace Rules {
    const int periodWeeks = 3, priceSeed = 4242;
    const double minFishPrice = 10, minFeedPrice = 5, youngFeedShare = 0.5;
    const size_t logLimit = 200;
    const int maxWeeksGuard = 100;
}
namespace BotRules {
    const double maxSellShare = 0.6, cashBufferK = 1.6, feedMargin = 1e-4, keepMoney = 1.0;
    const int quotaRiskWeeks = 3, restockStopWeeks = 2, diverseTop = 3;
    const int speedStep = 2, speedMin = 1, speedMax = 30;
}
enum LogKind { LOG_NORMAL, LOG_INFO, LOG_BAD, LOG_GOOD };
const Col LOG_COLORS[] = { Clr::text, Clr::gold, Clr::bad, Clr::good };
enum Pattern { PAT_NONE, PAT_SPOTS, PAT_STRIPES, PAT_DOTS, PAT_WHISKERS };
struct Species {
    Col back, belly, fin, mark;
    float len, h;
    Pattern pattern;
};
const Species DEFAULT_SPECIES{ Col(150,150,150), Col(220,220,220), Col(180,180,180), Col::White, 1.00f, 0.80f, PAT_NONE };
const map<string, Species> SPECIES = {
    {  "trout",       { Col(112,128,104), Col(240,176,152), Col(198, 92, 78), Col(235,112, 92), 1.00f, 0.75f, PAT_SPOTS } },
    {  "carp",        { Col(196,142, 52), Col(244,214,128), Col(224,162, 64), Col::White,       1.05f, 0.95f, PAT_NONE } },
    {  "pike",        { Col(66,108, 58),  Col(204,214,152), Col(92,130, 65),  Col(185,206,128), 1.38f, 0.52f, PAT_DOTS } },
    {  "catfish",     { Col(72, 72, 82),  Col(156,146,134), Col(110,102, 98), Col(40, 40, 44),  1.18f, 0.72f, PAT_WHISKERS } },
    {  "perch",       { Col(84,124, 62),  Col(230,218,142), Col(194,160, 66), Col(35, 70, 38, 190), 0.92f, 0.86f, PAT_STRIPES } },
    {  "crucian",     { Col(176,154, 74), Col(228,218,164), Col(210,174, 86), Col::White,       0.86f, 1.02f, PAT_NONE } },
    {  "silver_carp", { Col(150,172,194), Col(238,242,248), Col(178,192,206), Col::White,       1.10f, 0.76f, PAT_NONE } },
};
const Species& speciesOf(const string& id) {
    auto it = SPECIES.find(id);
    return it == SPECIES.end() ? DEFAULT_SPECIES : it->second;
}
struct WorldParams {
    int ponds = 4, money = 560000, weekFeed = 5000, weeks = 12, needFish = 300, penalty = 1000;
    int fishPrice = 450, fishStep = 20, feedPrice = 60, feedStep = 5, volatility = 0, lossMin = 5, lossMax = 10;
};
enum Strategy { STRATEGY_BEST, STRATEGY_FIXED, STRATEGY_DIVERSE, STRATEGY_ROUND_ROBIN, STRATEGY_COUNT };
struct BotSettings {
    int strategy = STRATEGY_BEST, fixedFish = 2, startYoung = 100, startOld = 0, pondsInUse = 7;
    int feedReserveWeeks = 1, cashBuffer = 1, minCash = 0, prioritizeContract = 1, sellStartPct = 50;
    int harvestPct = 0, cleanEvery = 8, cleanGuard = 2, restock = 0, restockAmount = 50, speedTenths = 10;
};
struct LogEntry { wstring text; LogKind kind; };
struct Rec { int week = 0, total = 0, young = 0, old = 0, sold = 0, need = 0; double money = 0, capital = 0, spent = 0, feed = 0; };
struct Result { double cash = 0, fishValue = 0, capital = 0, capital0 = 0, penalty = 0; int fish = 0, sold = 0, weeks = 0; bool broke = false; };

vector<Back::ContractPeriod> makePeriods(const WorldParams& P) {
    vector<Back::ContractPeriod> out;
    mt19937 rng(Rules::priceSeed);
    uniform_real_distribution<double> noise(-1.0, 1.0);
    for (int from = 1, k = 0; from <= P.weeks; from += Rules::periodWeeks, ++k) {
        Back::ContractPeriod cp;
        cp.from_week = from;
        cp.to_week = min(from + Rules::periodWeeks - 1, P.weeks);
        double fp = P.fishPrice + k * P.fishStep, kp = P.feedPrice + k * P.feedStep;
        if (P.volatility > 0) {
            fp *= 1.0 + noise(rng) * P.volatility / 100.0;
            kp *= 1.0 + noise(rng) * P.volatility / 100.0;
        }
        cp.fish_price = max(Rules::minFishPrice, floor(fp));
        cp.feed_price = max(Rules::minFeedPrice, floor(kp));
        out.push_back(cp);
    }
    return out;
}

wstring wide(const string& s) {
    int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    vector<wchar_t> buf(len);
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, buf.data(), len);
    return wstring(buf.data(), len - 1);
}

float frand() { return rand() / (float)RAND_MAX; }
float hash01(int n) {
    unsigned x = (unsigned)n * 2654435761u;
    x ^= x >> 15; x *= 2246822519u; x ^= x >> 13;
    return float(x & 0xFFFFFFu) / 16777216.f;
}
float prand(int pond, int salt) { return hash01(pond * 131 + salt * 17 + 7); }

Col mix(Col a, Col b, float t) {
    t = clamp(t, 0.f, 1.f);
    auto ch = [&](int x, int y) { return (uint8_t)(x + (y - x) * t); };
    return Col(ch(a.r, b.r), ch(a.g, b.g), ch(a.b, b.b), ch(a.a, b.a));
}
float angleDiff(float a, float b) {
    float d = fmodf(a - b + PI, 2.f * PI);
    if (d < 0) d += 2.f * PI;
    return d - PI;
}
wstring fmtMoney(double v) {
    long long x = llround(v);
    wstring d = to_wstring(x < 0 ? -x : x), out;
    for (size_t i = 0; i < d.size(); ++i) {
        if (i > 0 && (d.size() - i) % 3 == 0) out += L' ';
        out += d[i];
    }
    return x < 0 ? L"-" + out : out;
}
wstring fmtNum(double v, int dec) {
    wstring s = to_wstring(v);
    size_t dot = s.find(L'.');
    if (dot == wstring::npos) return s;
    return s.substr(0, dec <= 0 ? dot : min(s.size(), dot + 1 + (size_t)dec));
}
wstring pondTitle(int id) { return L"Пруд " + to_wstring(id + 1); }
double niceMax(double v) {
    if (v <= 0) return 1;
    double e = pow(10.0, floor(log10(v))), m = v / e;
    return (m <= 1 ? 1 : m <= 2 ? 2 : m <= 5 ? 5 : 10) * e;
}
int fishOf(const Back::Prud& p) { return p.fish.young + p.fish.old; }
double feedUnits(const Back::Prud& p) { return p.fish.young * Rules::youngFeedShare + p.fish.old; }

struct Farm {
    Back back;
    WorldParams params;
    vector<Back::information> types;
    vector<Back::ContractPeriod> periods;
    vector<int> cleaningFish, weekGrowth;
    vector<Rec> history;
    deque<LogEntry> log;
    double startCapital = 0, spentThisWeek = 0;
    bool over = false, broke = false;

    void say(const wstring& s, LogKind kind = LOG_NORMAL) {
        log.push_back({ s, kind });
        if (log.size() > Rules::logLimit) log.pop_front();
    }

    void init(const WorldParams& wp) {
        params = wp;
        back = Back();
        back.load_fish_from_file(FISH_FILE);
        types = back.get_fish_type();
        if (types.empty()) throw runtime_error(string("Не найден или пуст файл ") + FISH_FILE);
        back.start_all(params.ponds, params.money);
        back.set_loss_range(params.lossMin / 100.0, params.lossMax / 100.0);
        periods = makePeriods(params);
        back.createContract(params.weeks, params.weekFeed, params.needFish, params.penalty, periods);
        cleaningFish.assign(count(), 0);
        weekGrowth.assign(count(), 0);
        log.clear();
        over = broke = false;
        say(L"Контракт: " + to_wstring(params.weeks) + L" нед., поставить " + to_wstring(params.needFish)
            + L" кг, штраф " + fmtMoney(params.penalty) + L" руб/кг", LOG_INFO);
        rebase();
    }

    void rebase() {
        history.clear();
        spentThisWeek = 0;
        record();
        startCapital = history[0].capital;
    }

    void delete_prud(int id) {
        back.delete_prud(id);
    }

    int count() const { return (int)back.get_pruds().size(); }
    int week() const { return min(back.get_contract().curr + 1, params.weeks); }

    const Back::information* type(const string& id) const {
        for (const auto& t : types) if (t.type_id == id) return &t;
        return nullptr;
    }

    wstring fishName(const string& id) const {
        const Back::information* t = type(id);
        return t ? wide(t->name) : wstring(L"?");
    }

    double feedNeed(const Back::Prud& p) const {
        const Back::information* t = type(p.type_id);
        return t ? t->feed_norm * feedUnits(p) : 0.0;
    }

    const Back::ContractPeriod& periodAt(int w) const {
        for (const auto& p : periods) if (w >= p.from_week && w <= p.to_week) return p;
        return periods.back();
    }

    double fishPriceAt(int w) const { return periodAt(w).fish_price; }
    double feedPriceAt(int w) const { return periodAt(w).feed_price; }

    double fishKgAll() const {
        double s = back.get_totalFish();
        for (int kg : cleaningFish) s += kg;
        return s;
    }

    double capital() const { return back.get_money() + fishKgAll() * fishPriceAt(week()); }

    double speciesScore(const Back::information& t) const {
        double trace = 2.0 - t.b - t.d;
        double det = (1.0 - t.b) * (1.0 - t.d) - t.a * t.b;
        double lambda = (trace + sqrt(trace * trace - 4.0 * det)) / 2.0;
        double weeklyGrowth = lambda * (1.0 - (params.lossMin + params.lossMax) / 200.0);
        double youngPerOld = t.a / max(1e-9, lambda - 1.0 + t.b);
        double feedWeight = (youngPerOld * Rules::youngFeedShare + 1.0) / (youngPerOld + 1.0);
        return (weeklyGrowth - 1.0) * fishPriceAt(1) - t.feed_norm * feedWeight * feedPriceAt(1);
    }

    Back::Prud pond(int id) const {
        vector<Back::Prud> v = back.get_pruds();
        return (id < 0 || id >= (int)v.size()) ? Back::Prud() : v[id];
    }

    bool newPond(int id, const string& typeId) {
        Back::Prud p = pond(id);
        if (id < 0 || id >= count() || p.active || p.cleaning || !type(typeId)) return false;
        back.new_prud(id, typeId);
        say(pondTitle(id) + L": заселён видом " + fishName(typeId));
        return true;
    }

    void release(int id) {
        if (!pond(id).active) return;
        back.delete_prud(id);
        cleaningFish[id] = 0;
        say(pondTitle(id) + L" освобождён");
    }

    void addFish(int id, int young, int old, bool quiet = false) {
        Back::Prud p = pond(id);
        if (!p.active || p.cleaning || (young <= 0 && old <= 0)) return;
        back.add_fish(id, young, old);
        if (!quiet) say(pondTitle(id) + L": добавлено " + to_wstring(young) + L"+" + to_wstring(old) + L" кг рыбы");
    }

    int sell(int id, int kg, bool quiet = false) {
        Back::Prud p = pond(id);
        if (!p.active || p.cleaning || kg <= 0 || kg > p.fish.old) return 0;
        double before = back.get_money();
        back.sell_fish(id, kg);
        if (!quiet) say(pondTitle(id) + L": продано " + to_wstring(kg) + L" кг, +" + fmtMoney(back.get_money() - before) + L" руб.", LOG_GOOD);
        return kg;
    }

    double buyFeed(int id, double kg, bool quiet = false) {
        Back::Prud p = pond(id);
        if (!p.active || p.cleaning || kg <= 0) return 0;
        double cost = kg * feedPriceAt(week());
        back.buy_korm(id, kg);
        spentThisWeek += cost;
        if (!quiet) say(pondTitle(id) + L": куплено " + fmtNum(kg, 1) + L" кг корма за " + fmtMoney(cost) + L" руб.");
        return cost;
    }

    void clean(int id) {
        Back::Prud p = pond(id);
        if (!p.active || p.cleaning) return;
        cleaningFish[id] = fishOf(p);
        back.start_cleaning(id);
        say(pondTitle(id) + L" поставлен на очистку");
    }

    void record() {
        Rec r;
        r.week = back.get_week();
        r.money = back.get_money();
        r.total = back.get_totalFish();
        r.capital = capital();
        r.spent = spentThisWeek;
        spentThisWeek = 0;
        for (const auto& p : back.get_pruds()) {
            r.young += p.fish.young;
            r.old += p.fish.old;
            r.feed += p.current_feed;
        }
        Back::contract c = back.get_contract();
        r.sold = c.sold_fish;
        r.need = c.need_fish;
        history.push_back(r);
    }

    void checkBankruptcy() {
        if (broke || !back.get_no_money()) return;
        over = broke = true;
        say(L"БАНКРОТСТВО: денег не хватает для выполнения обязательств", LOG_BAD);
    }

    void nextWeek() {
        if (over) return;
        vector<Back::Prud> before = back.get_pruds();
        for (const auto& p : before) {
            if (!p.active || p.cleaning) continue;
            double need = feedNeed(p);
            if (need > 0 && p.current_feed + 1e-6 < need)
                say(pondTitle(p.id) + L": не хватило корма, рыба голодает!", LOG_BAD);
        }
        back.nextWeek();
        vector<Back::Prud> after = back.get_pruds();
        for (size_t i = 0; i < after.size(); ++i) {
            cleaningFish[i] = 0;
            weekGrowth[i] = (before[i].active && !before[i].cleaning) ? fishOf(after[i]) - fishOf(before[i]) : 0;
        }
        record();
        say(L"Итог недели " + to_wstring(back.get_week()) + L": рыба " + to_wstring(back.get_totalFish())
            + L" кг, наличные " + fmtMoney(back.get_money()));
        Back::contract c = back.get_contract();
        if (!c.active) {
            over = true;
            int missing = max(0, params.needFish - c.sold_fish);
            say(L"Контракт завершён: продано " + to_wstring(c.sold_fish) + L" из " + to_wstring(params.needFish) + L" кг", LOG_INFO);
            if (missing > 0) say(L"Неустойка: " + to_wstring(missing) + L" кг x " + fmtMoney(params.penalty) + L" руб.", LOG_BAD);
        }
        checkBankruptcy();
    }

    Result result() const {
        Result r;
        Back::contract c = back.get_contract();
        r.cash = back.get_money();
        r.fish = back.get_totalFish();
        r.fishValue = r.fish * periods.back().fish_price;
        r.capital = r.cash + r.fishValue;
        r.capital0 = startCapital;
        r.sold = c.sold_fish;
        r.weeks = back.get_week();
        r.broke = broke;
        bool ended = !c.active && c.all_weeks > 0 && c.curr >= c.all_weeks;
        r.penalty = ended ? max(0, params.needFish - c.sold_fish) * (double)params.penalty : 0.0;
        return r;
    }
};

struct Bot {
    BotSettings cfg;

    string chooseType(const Farm& f, int pondId) const {
        vector<pair<double, string>> rank;
        for (const auto& t : f.types) rank.push_back({ f.speciesScore(t), t.type_id });
        sort(rank.begin(), rank.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
        switch (cfg.strategy) {
        case STRATEGY_FIXED:
            return f.types[clamp(cfg.fixedFish, 1, (int)f.types.size()) - 1].type_id;
        case STRATEGY_DIVERSE: {
            int profitable = 0;
            for (const auto& r : rank) if (r.first > 0) profitable++;
            return rank[pondId % max(1, min(BotRules::diverseTop, profitable))].second;
        }
        case STRATEGY_ROUND_ROBIN:
            return f.types[pondId % (int)f.types.size()].type_id;
        default:
            return rank[0].second;
        }
    }

    void fillEmpty(Farm& f) {
        for (int id = 0; id < min(f.count(), cfg.pondsInUse); ++id)
            if (f.newPond(id, chooseType(f, id))) f.addFish(id, cfg.startYoung, cfg.startOld, true);
    }

    void plan(Farm& f) {
        Back::contract c = f.back.get_contract();
        if (!c.active || f.over) return;
        const int w = c.curr + 1, total = c.all_weeks, left = total - c.curr;
        const double fishPrice = f.fishPriceAt(w), feedPrice = f.feedPriceAt(w);
        f.say(L"== Неделя " + to_wstring(w) + L"/" + to_wstring(total) + L": решения бота ==", LOG_INFO);
        fillEmpty(f);
        ps = f.back.get_pruds();
        restockYoung(f, w, total);
        bool quotaRisk = c.need_fish > c.sold_fish && left <= BotRules::quotaRiskWeeks;
        cleanPonds(f, w, total, quotaRisk);
        ps = f.back.get_pruds();
        sellShare = left <= 1 ? 1.0 : BotRules::maxSellShare;
        oldAtStart.assign(ps.size(), 0);
        soldFrom.assign(ps.size(), 0);
        for (size_t i = 0; i < ps.size(); ++i) oldAtStart[i] = ps[i].fish.old;
        if (cfg.harvestPct > 0 && c.curr >= 1) sellKg(f, sellableTotal() * cfg.harvestPct / 100, L"плановая продажа");
        c = f.back.get_contract();
        int needLeft = max(0, c.need_fish - c.sold_fish), quota = 0;
        if (needLeft > 0) {
            if (left <= 1) quota = needLeft;
            else if (cfg.prioritizeContract && c.curr >= total * cfg.sellStartPct / 100) quota = (needLeft + left - 1) / left;
        }
        sellKg(f, min(quota, sellableTotal()), L"квота контракта");
        buyFeedForWeek(f, left, fishPrice, feedPrice);
    }

private:
    vector<Back::Prud> ps;
    vector<int> oldAtStart, soldFrom;
    double sellShare = 1.0;

    void restockYoung(Farm& f, int w, int total) {
        if (!cfg.restock || w > total - BotRules::restockStopWeeks) return;
        for (const auto& p : ps)
            if (p.active && !p.cleaning && fishOf(p) < cfg.restockAmount)
                f.addFish(p.id, cfg.restockAmount, 0);
    }

    void cleanPonds(Farm& f, int w, int total, bool quotaRisk) {
        if (cfg.cleanEvery <= 1 || w <= 1 || w > total - cfg.cleanGuard || quotaRisk) return;
        for (const auto& p : ps)
            if (p.active && !p.cleaning && fishOf(p) > 0 && (w + p.id) % cfg.cleanEvery == 0)
                f.clean(p.id);
    }

    int roomToSell(int i) const {
        if (!ps[i].active || ps[i].cleaning) return 0;
        int limit = (int)ceil(oldAtStart[i] * sellShare) - soldFrom[i];
        return max(0, min(ps[i].fish.old, limit));
    }

    int sellableTotal() const {
        int sum = 0;
        for (size_t i = 0; i < ps.size(); ++i) sum += roomToSell((int)i);
        return sum;
    }

    int sellKg(Farm& f, int kg, const wstring& reason) {
        int sold = 0;
        double before = f.back.get_money();
        while (kg > 0) {
            int best = -1;
            for (int i = 0; i < (int)ps.size(); ++i)
                if (roomToSell(i) > (best < 0 ? 0 : roomToSell(best))) best = i;
            if (best < 0) break;
            int did = f.sell(best, min(kg, roomToSell(best)), true);
            if (did <= 0) break;
            ps = f.back.get_pruds();
            soldFrom[best] += did;
            kg -= did;
            sold += did;
        }
        if (sold > 0)
            f.say(L"Продано " + to_wstring(sold) + L" кг (" + reason + L"): +" + fmtMoney(f.back.get_money() - before) + L" руб.", LOG_GOOD);
        return sold;
    }

    double planFeed(const Farm& f, int reserveWeeks, double feedPrice, vector<double>& buy) const {
        buy.assign(ps.size(), 0.0);
        double cost = 0;
        for (size_t i = 0; i < ps.size(); ++i) {
            if (!ps[i].active || ps[i].cleaning) continue;
            double need = f.feedNeed(ps[i]);
            double deficit = need * reserveWeeks - ps[i].current_feed;
            if (need > 0 && deficit > 0) {
                buy[i] = deficit + BotRules::feedMargin;
                cost += buy[i] * feedPrice;
            }
        }
        return cost;
    }

    void buyFeedForWeek(Farm& f, int left, double fishPrice, double feedPrice) {
        const int reserve = max(1, min(cfg.feedReserveWeeks, left));
        vector<double> buy;
        double cost = planFeed(f, reserve, feedPrice, buy);
        double gap = max(cost, (double)f.params.weekFeed) + cost * BotRules::cashBufferK * cfg.cashBuffer + cfg.minCash - f.back.get_money();
        if (gap > 0 && fishPrice > 0 && sellKg(f, min((int)ceil(gap / fishPrice), sellableTotal()), L"деньги на корм") > 0)
            cost = planFeed(f, reserve, feedPrice, buy);
        double limit = max(0.0, f.back.get_money() - BotRules::keepMoney);
        double factor = cost > limit ? (cost > 0 ? limit / cost : 0.0) : 1.0;
        double spent = 0, kg = 0;
        for (size_t i = 0; i < ps.size() && factor > 0; ++i)
            if (buy[i] > 0) { spent += f.buyFeed((int)i, buy[i] * factor, true); kg += buy[i] * factor; }
        if (spent < f.params.weekFeed && feedPrice > 0) {
            double extra = min(f.params.weekFeed - spent, f.back.get_money() - BotRules::keepMoney);
            int best = -1;
            double bestUnits = -1;
            for (int i = 0; i < (int)ps.size(); ++i) {
                double units = feedUnits(ps[i]);
                if (ps[i].active && !ps[i].cleaning && units > bestUnits) { bestUnits = units; best = i; }
            }
            if (best >= 0 && extra > BotRules::keepMoney) {
                spent += f.buyFeed(best, extra / feedPrice + 1e-6, true);
                kg += extra / feedPrice;
            }
        }
        f.say(L"Куплено корма: " + fmtNum(kg, 1) + L" кг на " + fmtMoney(spent) + L" руб.");
        if (factor < 1.0) f.say(L"НЕ ХВАТАЕТ ДЕНЕГ: корма куплено " + fmtNum(factor * 100, 0) + L"% от нужного", LOG_BAD);
    }
};

Result runHeadless(const WorldParams& wp, const BotSettings& bs) {
    Farm f;
    Bot bot;
    bot.cfg = bs;
    f.init(wp);
    bot.fillEmpty(f);
    f.rebase();
    for (int guard = 0; !f.over && guard < Rules::maxWeeksGuard; ++guard) {
        bot.plan(f);
        f.checkBankruptcy();
        f.nextWeek();
    }
    return f.result();
}

Farm farm;
Bot bot;
WorldParams worldCfg;
BotSettings botCfg;
Result cmpResults[STRATEGY_COUNT];
bool haveCompare = false;

struct Batch {
    vector<sf::Vertex> v;

    void tri(Vec a, Vec b, Vec c, Col ca, Col cb, Col cc) {
        v.push_back({ a, ca }); v.push_back({ b, cb }); v.push_back({ c, cc });
    }
    void tri(Vec a, Vec b, Vec c, Col col) { tri(a, b, c, col, col, col); }
    void quad(Vec a, Vec b, Vec c, Vec d, Col ca, Col cb, Col cc, Col cd) {
        tri(a, b, c, ca, cb, cc);
        tri(a, c, d, ca, cc, cd);
    }
    void quad(Vec a, Vec b, Vec c, Vec d, Col col) { quad(a, b, c, d, col, col, col, col); }
    void rect(Rect r, Col col) { rect(r, col, col); }
    void rect(Rect r, Col top, Col bottom) {
        quad({ r.x, r.y }, { r.x + r.w, r.y }, { r.x + r.w, r.y + r.h }, { r.x, r.y + r.h }, top, top, bottom, bottom);
    }
    void ellipse(float cx, float cy, float rx, float ry, Grad g, int seg = 24, float rot = 0.f) {
        float cr = cosf(rot), sr = sinf(rot);
        auto at = [&](float a) {
            float x = cosf(a) * rx, y = sinf(a) * ry;
            return Vec{ cx + x * cr - y * sr, cy + x * sr + y * cr };
            };
        Vec prev = at(0.f);
        for (int i = 1; i <= seg; ++i) {
            Vec cur = at(2.f * PI * i / seg);
            tri({ cx, cy }, prev, cur, g.a, g.b, g.b);
            prev = cur;
        }
    }
    void ellipse(float cx, float cy, float rx, float ry, Col col, int seg = 24, float rot = 0.f) {
        ellipse(cx, cy, rx, ry, Grad{ col, col }, seg, rot);
    }
    void circle(Vec c, float r, Col col, int seg = 24) { ellipse(c.x, c.y, r, r, col, seg); }
    void fan(Vec center, const vector<Vec>& pts, Grad g) {
        for (size_t i = 0; i < pts.size(); ++i) tri(center, pts[i], pts[(i + 1) % pts.size()], g.a, g.b, g.b);
    }
    void fan(Vec center, const vector<Vec>& pts, Col col) { fan(center, pts, Grad{ col, col }); }
    void line(Vec a, Vec b, float thickness, Col col) {
        float dx = b.x - a.x, dy = b.y - a.y, len = sqrtf(dx * dx + dy * dy);
        if (len <= 0.f) return;
        float nx = -dy / len * thickness / 2, ny = dx / len * thickness / 2;
        quad({ a.x + nx, a.y + ny }, { b.x + nx, b.y + ny }, { b.x - nx, b.y - ny }, { a.x - nx, a.y - ny }, col);
    }
    void polyline(const vector<Vec>& p, float thickness, Col col, bool closed = false) {
        for (size_t i = 0; i + 1 < p.size(); ++i) line(p[i], p[i + 1], thickness, col);
        if (closed && p.size() > 2) line(p.back(), p[0], thickness, col);
    }
    static vector<Vec> roundedPoints(Rect r, float radius) {
        radius = min(radius, min(r.w, r.h) / 2.f);
        const float cx[4] = { r.x + radius, r.x + r.w - radius, r.x + r.w - radius, r.x + radius };
        const float cy[4] = { r.y + radius, r.y + radius, r.y + r.h - radius, r.y + r.h - radius };
        const float a0[4] = { PI, 1.5f * PI, 0.f, 0.5f * PI };
        vector<Vec> p;
        for (int c = 0; c < 4; ++c)
            for (int k = 0; k <= 5; ++k) {
                float a = a0[c] + 0.5f * PI * k / 5;
                p.push_back({ cx[c] + cosf(a) * radius, cy[c] + sinf(a) * radius });
            }
        return p;
    }
    void roundRect(Rect r, float radius, Col top, Col bottom) {
        vector<Vec> p = roundedPoints(r, radius);
        Vec mid{ r.x + r.w / 2, r.y + r.h / 2 };
        for (size_t i = 0; i < p.size(); ++i) {
            const Vec& a = p[i];
            const Vec& b = p[(i + 1) % p.size()];
            tri(mid, a, b, mix(top, bottom, 0.5f), mix(top, bottom, (a.y - r.y) / r.h), mix(top, bottom, (b.y - r.y) / r.h));
        }
    }
    void outline(Rect r, float radius, float thickness, Col col) { polyline(roundedPoints(r, radius), thickness, col, true); }
    void flush(sf::RenderWindow& win) {
        if (v.empty()) return;
        win.draw(v.data(), v.size(), sf::PrimitiveType::Triangles);
        v.clear();
    }
};

struct TextCmd { float x, y; wstring s; TS style; };
sf::RenderWindow window;
sf::Font font;
Batch batch;
vector<TextCmd> texts;

void drawText(float x, float y, const wstring& s, TS style) { texts.push_back({ x, y, s, style }); }
float textWidth(const wstring& s, unsigned size) {
    sf::Text t(font, sf::String(s), size);
    return t.getLocalBounds().size.x;
}
void flushFrame() {
    batch.flush(window);
    for (const auto& t : texts) {
        sf::Text q(font, sf::String(t.s), t.style.size);
        q.setPosition({ t.x, t.y });
        q.setFillColor(t.style.color);
        window.draw(q);
    }
    texts.clear();
}
void drawBox(Rect r, const BoxStyle& st) {
    batch.roundRect(r, st.radius, st.top, st.bottom);
    if (st.borderW > 0) batch.outline(r, st.radius, st.borderW, st.border);
}
void panel(Rect r) { drawBox(r, Box::panel); }
void bar(float x, float y, float w, float fraction, Col color) {
    Rect track{ x, y, w, Bar::h };
    drawBox(track, Box::track);
    fraction = clamp(fraction, 0.f, 1.f);
    if (fraction > 0.001f) {
        Rect fill{ x, y, max(Bar::minFill, w * fraction), Bar::h };
        batch.roundRect(fill, Box::track.radius, mix(color, Col::White, 0.25f), color);
    }
}
void backdrop() {
    batch.rect({ 0, 0, W, Scene::horizon }, Scene::skyTop, Scene::skyBottom);
    batch.ellipse(Scene::sun.x, Scene::sun.y, Scene::glowR, Scene::glowR, Scene::sunGlow, Scene::sunSeg);
    batch.ellipse(Scene::sun.x, Scene::sun.y, Scene::coreR, Scene::coreR, Scene::sunCore, Scene::sunSeg);
    for (const auto& ridge : Scene::ridges)
        for (const auto& p : ridge.peaks) {
            float topY = Scene::horizon - p.h;
            batch.tri({ p.x - ridge.halfW, Scene::horizon }, { p.x + ridge.halfW, Scene::horizon }, { p.x, topY }, ridge.color);
            if (ridge.snowHalfW > 0)
                batch.tri({ p.x - ridge.snowHalfW, topY + ridge.snowDepth * p.h },
                    { p.x + ridge.snowHalfW, topY + ridge.snowDepth * p.h }, { p.x, topY }, ridge.snow);
        }
    batch.rect({ 0, Scene::horizon - 10, W, H - Scene::horizon + 10 }, Scene::grassTop, Scene::grassBottom);
}
struct AnimFish {
    float x = 0, y = 0, heading = 0, speed = 0, turn = 0, timer = 0, size = 1;
    bool young = false;
};
struct PondView {
    Vec c;
    float rx = 0, ry = 0;
    vector<Vec> unit, water, edge, bank, glow, ring, sel;
    vector<AnimFish> fish;
};
vector<PondView> ponds;
vector<Vec> scaled(const PondView& p, const vector<Vec>& unit, float s) {
    vector<Vec> out;
    out.reserve(unit.size());
    for (const auto& u : unit) out.push_back({ p.c.x + u.x * p.rx * s, p.c.y + u.y * p.ry * s });
    return out;
}
void buildPonds(const Farm& farm) {
    int k = farm.count();
    ponds.clear();
    int cols = Ponds::columns(k), rows = (k + cols - 1) / cols;
    float cellW = (Ponds::right - Ponds::left) / cols, cellH = (Ponds::bottom - Ponds::top) / rows;
    for (int i = 0; i < k; ++i) {
        int row = i / cols, col = i % cols, inRow = min(cols, k - row * cols);
        PondView p;
        p.rx = cellW * Ponds::rxOfCell;
        p.ry = min(cellH * Ponds::ryOfCell, p.rx * Ponds::maxAspect);
        p.c.x = Ponds::left + (col + 0.5f) * cellW + (cols - inRow) * cellW / 2 + (frand() - 0.5f) * Ponds::jitX;
        p.c.y = Ponds::top + (row + 0.5f) * cellH + Ponds::shiftY + (frand() - 0.5f) * Ponds::jitY;
        float phase[3];
        for (int j = 0; j < 3; ++j) phase[j] = prand(i, j) * 2 * PI;
        for (int j = 0; j < Ponds::outlinePts; ++j) {
            float a = 2.f * PI * j / Ponds::outlinePts, r = 1.f;
            for (int w = 0; w < 3; ++w) r += Ponds::wob[w].amp * sinf(Ponds::wob[w].n * a + phase[w]);
            p.unit.push_back({ cosf(a) * r, sinf(a) * r });
        }
        p.water = scaled(p, p.unit, 1.f);
        p.edge = scaled(p, p.unit, Art::edgeS);
        p.bank = scaled(p, p.unit, Art::bankS);
        p.glow = scaled(p, p.unit, Art::glowS);
        p.ring = scaled(p, p.unit, Art::ringS);
        p.sel = scaled(p, p.unit, Art::selS);
        ponds.push_back(p);
    }
}
void drawFish(Vec pos, float ang, float size, const Species& sp, bool young) {
    float len = (young ? Fish::youngLen : Fish::oldLen) * size * sp.len;
    float h = (young ? Fish::youngH : Fish::oldH) * size * sp.h;
    float cs = cosf(ang), sn = sinf(ang);
    auto P = [&](float u, float v) {
        float a = u * len, b = v * h;
        return Vec{ pos.x + a * cs - b * sn, pos.y + a * sn + b * cs };
        };
    batch.tri(P(Fish::tail[0].x, Fish::tail[0].y), P(Fish::tail[1].x, Fish::tail[1].y), P(Fish::tail[2].x, Fish::tail[2].y), sp.fin);
    for (const auto& f : Fish::fins)
        batch.tri(P(f[0].x, f[0].y), P(f[1].x, f[1].y), P(f[2].x, f[2].y), sp.fin);
    batch.ellipse(pos.x, pos.y, len * Fish::bodyHL, h, Grad{ sp.belly, sp.back }, Fish::bodySeg, ang);
    Vec b = P(Fish::backShift.x, Fish::backShift.y);
    batch.ellipse(b.x, b.y, len * Fish::backHL, h * Fish::backHH, sp.back, Fish::backSeg, ang);
    switch (sp.pattern) {
    case PAT_SPOTS:
        for (int i = 0; i < Fish::spots; ++i)
            batch.circle(P(Fish::spotFirst + i * Fish::spotStep, 0), h * Fish::spotR, sp.mark, Fish::dotSeg);
        break;
    case PAT_STRIPES:
        for (int i = 0; i < Fish::stripes; ++i) {
            float u = Fish::stripeFirst + i * Fish::stripeStep;
            batch.line(P(u, Fish::stripeFrom), P(u, Fish::stripeTo), Fish::stripeW, sp.mark);
        }
        break;
    case PAT_DOTS:
        for (int i = 0; i < Fish::dots; ++i)
            batch.circle(P(Fish::dotFirst + i * Fish::dotStep, 0), h * Fish::dotR, sp.mark, Fish::dotSeg);
        break;
    case PAT_WHISKERS:
        batch.line(P(Fish::whiskerRoot.x, Fish::whiskerRoot.y), P(Fish::whiskerA.x, Fish::whiskerA.y), Fish::whiskerW, sp.mark);
        batch.line(P(Fish::whiskerRoot.x, Fish::whiskerRoot.y), P(Fish::whiskerB.x, Fish::whiskerB.y), Fish::whiskerW, sp.mark);
        break;
    default:
        break;
    }
    Vec e = P(Fish::eye.x, Fish::eye.y);
    batch.circle(e, max(Fish::eyeMin, size * Fish::eyeSize), Fish::eyeWhite, Fish::eyeSeg);
    batch.circle({ e.x + cs * Fish::pupilShift, e.y + sn * Fish::pupilShift }, max(Fish::pupilMin, size * Fish::pupilSize), Fish::pupil, Fish::eyeSeg);
}
void drawPond(int i, const Back::Prud& p, bool selected) {
    const PondView& v = ponds[i];
    batch.fan({ v.c.x + Art::shadowOff.x, v.c.y + Art::shadowOff.y }, v.bank, Art::shadow);
    batch.polyline(v.bank, Art::bankLW, Art::bankLine, true);
    batch.fan(v.c, v.edge, Art::edge);
    batch.polyline(v.edge, Art::edgeLW, Art::edgeLine, true);
    if (!p.active) {
        batch.fan(v.c, v.water, Art::dry);
        for (int k = 0; k < Art::nCracks; ++k) {
            float a = prand(i, 40 + k) * 2 * PI, tw = 0.1f;
            batch.line({ v.c.x + cosf(a) * v.rx * 0.1f, v.c.y + sinf(a) * v.ry * 0.1f },
                { v.c.x + cosf(a + tw) * v.rx * 0.8f, v.c.y + sinf(a + tw) * v.ry * 0.8f }, Art::crackLW, Art::crack);
        }
        return;
    }

    Grad wg, gg;

    wg.a = mix(Art::cleanW.a, Art::dirtyW.a, p.pollution);
    wg.b = mix(Art::cleanW.b, Art::dirtyW.b, p.pollution);

    gg.a = mix(Art::cleanG.a, Art::dirtyG.a, p.pollution);
    gg.b = mix(Art::cleanG.b, Art::dirtyG.b, p.pollution);

    batch.fan(v.c, v.water, wg);
    batch.fan({ v.c.x + v.rx * Art::glowOff.x, v.c.y + v.ry * Art::glowOff.y }, v.glow, gg);
    batch.polyline(v.water, Art::shoreLW, Art::shoreLine, true);
    for (int k = 0; k < Art::nRipples; ++k) {
        float a = prand(i, 60 + k) * 2 * PI;
        float x = v.c.x + cosf(a) * v.rx * (0.18f + 0.58f * prand(i, 80 + k));
        float y = v.c.y + sinf(a) * v.ry * (0.12f + 0.55f * prand(i, 100 + k));
        batch.ellipse(x, y, 12.f + 17.f * prand(i, 120 + k), 1.4f + 1.6f * prand(i, 140 + k), Art::ripple, Art::rippleSeg, a * 0.2f);
    }
    for (int k = 0; k < Art::nStones; ++k) {
        Vec q = v.bank[(size_t)(i * 13 + k * 11) % v.bank.size()];
        float s = 5.f + 5.f * prand(i, 160 + k);
        batch.ellipse(q.x, q.y + 2, s, 3.5f, Art::stone, 10);
        batch.ellipse(q.x - 1, q.y, 2.4f, 1.6f, Art::stoneHi, 8);
    }
    if (!p.cleaning) {
        for (int k = 0; k < Art::nLilies; ++k) {
            float a = prand(i, 200 + k) * 2 * PI;
            batch.ellipse(v.c.x + cosf(a) * v.rx * 0.8f, v.c.y + sinf(a) * v.ry * 0.8f,
                Art::lilyRx, Art::lilyRy, Art::lily, Art::lilySeg, a);
        }
        for (const auto& f : v.fish)
            drawFish({ v.c.x + f.x * v.rx * Sim::drawScaleX, v.c.y + f.y * v.ry * Sim::drawScaleY },
                f.heading, f.size, speciesOf(p.type_id), f.young);
    }
    else {
        batch.polyline(v.ring, Art::ringLW, Art::ring, true);
        drawText(v.c.x - 32, v.c.y - 8, L"Очистка", T::label.with(Col::White));
    }
    if (selected) batch.polyline(v.sel, Art::selLW, Clr::selection, true);
}
void drawPondLabel(int i, const Back::Prud& p) {
    const PondView& v = ponds[i];
    Rect r{ v.c.x - Ponds::labelW / 2, v.c.y - v.ry * Ponds::labelRise - Ponds::labelGap, Ponds::labelW, Ponds::labelH };
    drawBox(r, Box::tag);
    drawText(r.x + Ponds::labelPadX, r.y + Ponds::line1, to_wstring(i + 1) + L"   " + (p.active ? farm.fishName(p.type_id) : wstring(L"свободен")), T::label);
    wstring state = p.cleaning ? L"очистка"
        : p.active ? L"М  " + to_wstring(p.fish.young) + L"   В  " + to_wstring(p.fish.old)
        : L"пруд пуст";
    drawText(r.x + Ponds::labelPadX, r.y + Ponds::line2, state, T::capMuted);
}
void fitFish(PondView& p, bool young, int target) {
    int have = 0;
    for (const auto& f : p.fish) if (f.young == young) have++;
    for (; have < target; ++have) {
        AnimFish f;
        float a = frand() * 2 * PI, r = sqrtf(frand()) * Sim::spawnR;
        f.x = cosf(a) * r;
        f.y = sinf(a) * r;
        f.heading = frand() * 2 * PI;
        f.speed = (young ? Sim::youngSpd : Sim::oldSpd) + frand() * Sim::spdSpread;
        f.timer = frand() * Sim::firstTurn;
        f.size = Sim::sizeLo + (Sim::sizeHi - Sim::sizeLo) * frand();
        f.young = young;
        p.fish.push_back(f);
    }
    for (; have > target; --have)
        for (int j = (int)p.fish.size() - 1; j >= 0; --j)
            if (p.fish[j].young == young) { p.fish.erase(p.fish.begin() + j); break; }
}
void updateFish(const Farm& farm, float dt, float time) {
    vector<Back::Prud> ps = farm.back.get_pruds();
    if (ponds.size() != ps.size()) buildPonds(farm);
    for (size_t i = 0; i < ps.size(); ++i) {
        PondView& v = ponds[i];
        int total = ps[i].fish.young + ps[i].fish.old, shown = 0, shownOld = 0;
        if (ps[i].active && !ps[i].cleaning && total > 0) {
            shown = min(Sim::maxFish, Sim::minFish + (int)(Sim::perSqrtKg * sqrtf((float)total)));
            shownOld = (int)lroundf(shown * (float)ps[i].fish.old / total);
        }
        fitFish(v, false, shownOld);
        fitFish(v, true, shown - shownOld);
        float sx = Sim::driftAmpX * sinf(time * Sim::driftFreqX + i * 1.7f);
        float sy = Sim::driftAmpY * cosf(time * Sim::driftFreqY + i * 2.3f);
        for (auto& f : v.fish) {
            f.timer -= dt;
            if (f.timer <= 0) {
                f.turn = (frand() - 0.5f) * Sim::turnPow;
                f.timer = Sim::turnLo + (Sim::turnHi - Sim::turnLo) * frand();
            }
            float dx = sx - f.x, dy = sy - f.y, dist = sqrtf(dx * dx + dy * dy);
            f.heading += (f.turn + angleDiff(atan2f(dy, dx), f.heading) * min(Sim::pullMax, dist * Sim::pullPerDist)) * dt;
            if (f.x * f.x + f.y * f.y > Sim::wallR2) f.heading += angleDiff(atan2f(-f.y, -f.x), f.heading) * Sim::wallPull * dt;
            f.x += cosf(f.heading) * f.speed * dt;
            f.y += sinf(f.heading) * f.speed * dt;
            float r = sqrtf(f.x * f.x + f.y * f.y);
            if (r > Sim::hardLimit) { f.x *= Sim::hardLimit / r; f.y *= Sim::hardLimit / r; }
        }
    }
}
enum Screen { SCREEN_SETUP, SCREEN_BOT_SETUP, SCREEN_GAME, SCREEN_GRAPH, SCREEN_FINISH };
enum DialogAction { DLG_NONE, DLG_FEED, DLG_YOUNG, DLG_OLD, DLG_SELL };
const wchar_t* const DLG_TITLES[] = {
    L"", L"Сколько кг корма купить?", L"Сколько кг молоди добавить?",
    L"Сколько кг взрослой рыбы добавить?", L"Сколько кг взрослой рыбы продать?"
};
const vector<int> DLG_CHIPS = { 50, 100, 250 };
struct Mouse { float x = 0, y = 0; bool clicked = false; } mouse;
struct Dialog {
    bool open = false, drawing = false;
    DialogAction action = DLG_NONE;
    wstring input;
    vector<pair<wstring, int>> chips;
} dialog;
struct UiState {
    Screen screen = SCREEN_SETUP;
    int tab = 0, selectedPond = 0;
    bool botMode = false, botPaused = false;
    float botTimer = 0, time = 0;
} ui;

Rect contentOf(Rect p) { return { p.x + Lay::padX, p.y + Lay::padTop, p.w - 2 * Lay::padX, p.h - Lay::padTop }; }
Rect gridCell(Rect area, int col, int row, int cols, float cellH, float rowGap) {
    float cellW = (area.w - Lay::gridGap * (cols - 1)) / cols;
    return { area.x + col * (cellW + Lay::gridGap), area.y + row * (cellH + rowGap), cellW, cellH };
}
struct Column {
    float x, y, w;
    void line(const wstring& s, TS st) { drawText(x, y, s, st); y += st.size + Lay::lineGap; }
    void skip(float dy) { y += dy; }
};
bool inputActive() { return !dialog.open || dialog.drawing; }
bool button(Rect r, const wstring& label, BtnKind kind = B_NORMAL, bool enabled = true, unsigned fontSize = FS::body) {
    bool hover = enabled && r.has(mouse.x, mouse.y) && inputActive();
    BtnStyle st = enabled ? BTN_STYLES[kind] : BTN_DISABLED;
    Col fill = st.top;
    if (hover) fill = mix(fill, Col::White, 0.15f);
    batch.rect(r, fill);
    drawText(r.x + (r.w - textWidth(label, fontSize)) / 2, r.y + (r.h - fontSize) / 2 - 2.f, label, TS{ fontSize, st.text });
    if (hover && mouse.clicked) { mouse.clicked = false; return true; }
    return false;
}
void trendArrow(float x, float y, bool up, bool isGood) {
    Col c = isGood ? Clr::good : Clr::bad;
    float w = Lay::arrowW, h = Lay::arrowH;
    if (up) batch.tri({ x, y + h }, { x + w, y + h }, { x + w / 2, y }, c);
    else    batch.tri({ x, y }, { x + w, y }, { x + w / 2, y + h }, c);
}
struct Setting {
    wstring name;
    int* value;
    int lo, hi, step, bigStep;
    vector<wstring> labels;
    wstring suffix;
    int divisor;
};
Setting number(const wchar_t* name, int* value, int lo, int hi, int step, const wchar_t* suffix = L"", int divisor = 1) {
    return { name, value, lo, hi, step, step * 10, {}, suffix, divisor };
}
Setting choice(const wchar_t* name, int* value, int first, vector<wstring> labels) {
    return { name, value, first, first + (int)labels.size() - 1, 1, 1, labels, L"", 1 };
}
vector<wstring> fishLabels() {
    vector<wstring> v;
    for (const auto& t : farm.types) v.push_back(wide(t.name));
    return v;
}
const vector<wstring> YES_NO = { L"Нет", L"Да" };
const vector<wstring> STRAT_SHORT = { L"Лучший вид", L"Выбранный", L"Диверсиф.", L"По кругу" };
const wchar_t* const STRAT_FULL[STRATEGY_COUNT] = { L"Лучший вид", L"Выбранный вид", L"Диверсификация", L"Все виды по кругу" };
vector<Setting> worldSettings, botSettings;

void buildSettings() {
    worldSettings = {
        number(L"Количество прудов",           &worldCfg.ponds,      2, 7, 1),
        number(L"Стартовый капитал",           &worldCfg.money,      50000, 20000000, 10000, L" руб."),
        number(L"Закупка корма в неделю",      &worldCfg.weekFeed,   0, 500000, 500, L" руб."),
        number(L"Срок контракта",              &worldCfg.weeks,      6, 24, 1, L" нед."),
        number(L"Поставка рыбы по контракту",  &worldCfg.needFish,   0, 100000, 50, L" кг"),
        number(L"Штраф за недопоставку",       &worldCfg.penalty,    0, 20000, 100, L" руб/кг"),
        number(L"Потери рыбы: минимум",        &worldCfg.lossMin,    0, 50, 1, L" %"),
        number(L"Потери рыбы: максимум",       &worldCfg.lossMax,    0, 50, 1, L" %"),
        number(L"Цена рыбы (1-й период)",      &worldCfg.fishPrice,  50, 5000, 10, L" руб/кг"),
        number(L"Сдвиг цены рыбы за период",   &worldCfg.fishStep,   -300, 500, 5, L" руб."),
        number(L"Цена корма (1-й период)",     &worldCfg.feedPrice,  10, 1000, 5, L" руб/кг"),
        number(L"Сдвиг цены корма за период",  &worldCfg.feedStep,   -100, 200, 1, L" руб."),
        number(L"Разброс цен (волатильность)", &worldCfg.volatility, 0, 40, 1, L" %"),
    };
    botSettings = {
        choice(L"Стратегия выбора рыбы",        &botCfg.strategy,     STRATEGY_BEST, STRAT_SHORT),
        choice(L"Вид для \"Выбранный\"",        &botCfg.fixedFish,    1, fishLabels()),
        number(L"Молодь на пруд при запуске",   &botCfg.startYoung,   0, 2000, 25, L" кг"),
        number(L"Взрослая рыба на пруд",        &botCfg.startOld,     0, 2000, 25, L" кг"),
        number(L"Прудов в работе",              &botCfg.pondsInUse,   1, 7, 1),
        number(L"Запас корма вперёд",           &botCfg.feedReserveWeeks, 1, 4, 1, L" нед."),
        choice(L"Докупать мальков (бесплатно)", &botCfg.restock,     0, YES_NO),
        number(L"Порог докупки малька",         &botCfg.restockAmount, 10, 500, 10, L" кг"),
        choice(L"Приоритет контракта",          &botCfg.prioritizeContract, 0, YES_NO),
        number(L"Начать квоту с",               &botCfg.sellStartPct, 0, 90, 5, L" %"),
        number(L"Плановая продажа взрослой",    &botCfg.harvestPct,   0, 50, 5, L" %"),
        number(L"Запас денег (недель роста)",   &botCfg.cashBuffer,   0, 4, 1),
        number(L"Минимальный остаток денег",    &botCfg.minCash,      0, 500000, 5000, L" руб."),
        number(L"Очистка прудов (0 = нет)",     &botCfg.cleanEvery,   0, 12, 1, L" нед."),
        number(L"Не чистить в последние",       &botCfg.cleanGuard,   0, 6, 1, L" нед."),
        number(L"Пауза между неделями",         &botCfg.speedTenths,  BotRules::speedMin, BotRules::speedMax, 1, L" с", 10),
    };
}

wstring settingValueText(const Setting& s) {
    if (!s.labels.empty()) return s.labels[clamp(*s.value - s.lo, 0, (int)s.labels.size() - 1)];
    return (s.divisor > 1 ? fmtNum((double)*s.value / s.divisor, 1) : fmtMoney(*s.value)) + s.suffix;
}

void settingRow(Setting& s, float x, float y, float w) {
    drawText(x, y + SetRow::textPadY, s.name, T::body);
    float bx = x + w - SetRow::controlsW;
    auto next = [&](float width) { Rect r{ bx, y, width, SetRow::btnH }; bx += width + SetRow::gap; return r; };
    if (button(next(SetRow::stepW), L"--", B_NORMAL, true, FS::note)) *s.value = max(s.lo, *s.value - s.bigStep);
    if (button(next(SetRow::stepW), L"-"))                             *s.value = max(s.lo, *s.value - s.step);
    Rect valueBox = next(SetRow::valueW);
    drawBox(valueBox, Box::field);
    wstring text = settingValueText(s);
    drawText(valueBox.x + (SetRow::valueW - textWidth(text, FS::label)) / 2, y + SetRow::textPadY, text, T::labelGold);
    if (button(next(SetRow::stepW), L"+"))                             *s.value = min(s.hi, *s.value + s.step);
    if (button(next(SetRow::stepW), L"++", B_NORMAL, true, FS::note))  *s.value = min(s.hi, *s.value + s.bigStep);
}

void settingsBlock(vector<Setting>& list, int from, int to, Block b) {
    panel({ b.x - Menu::padX, b.y - Menu::padTop, b.w + 2 * Menu::padX, (to - from) * b.rowH + Menu::padTop + Menu::padBottom });
    for (int i = from; i < to; ++i) settingRow(list[i], b.x, b.y + (i - from) * b.rowH, b.w);
}

void startGame(bool automatic) {
    ui.botMode = automatic;
    ui.botPaused = false;
    ui.botTimer = 0;
    bot.cfg = botCfg;
    farm.init(worldCfg);
    if (ui.botMode) {
        bot.fillEmpty(farm);
        farm.say(L"Бот: зарыбление выполнено", LOG_INFO);
        farm.rebase();
    }
    buildPonds(farm);
    ui.selectedPond = 0;
    ui.screen = SCREEN_GAME;
}

void checkGameOver() {
    farm.checkBankruptcy();
    if (farm.over) ui.screen = SCREEN_FINISH;
}

void stepWeek() {
    if (farm.over) return;
    if (ui.botMode) bot.plan(farm);
    farm.checkBankruptcy();
    farm.nextWeek();
    checkGameOver();
}

void openDialog(DialogAction action) {
    Back::Prud p = farm.pond(ui.selectedPond);
    double stock = p.current_feed;
    dialog.action = action;
    dialog.input.clear();
    dialog.open = true;
    mouse.clicked = false;
    dialog.chips.clear();
    if (action == DLG_FEED) {
        double need = farm.feedNeed(p);
        dialog.chips = { { L"На неделю",   max(0, (int)ceil(need - stock)) },
                        { L"На 2 недели", max(0, (int)ceil(2 * need - stock)) } };
    }
    else if (action == DLG_SELL) dialog.chips = { { L"Половину", p.fish.old / 2 }, { L"Всё", p.fish.old } };
    else for (int v : DLG_CHIPS) dialog.chips.push_back({ to_wstring(v), v });
}

void applyDialog() {
    int value = dialog.input.empty() ? 0 : stoi(dialog.input);
    int id = ui.selectedPond;
    switch (dialog.action) {
    case DLG_FEED:  farm.buyFeed(id, value); break;
    case DLG_YOUNG: farm.addFish(id, value, 0); break;
    case DLG_OLD:   farm.addFish(id, 0, value); break;
    case DLG_SELL:  farm.sell(id, value); break;
    default: break;
    }
    dialog.open = false;
}

void drawDialog() {
    if (!dialog.open) return;
    flushFrame();
    dialog.drawing = true;
    batch.rect({ 0, 0, W, H }, Clr::dim);
    Rect box{ (W - Dlg::w) / 2, (H - Dlg::h) / 2, Dlg::w, Dlg::h };
    panel(box);
    drawText(box.x + Dlg::titlePadX, box.y + Dlg::titlePadY, DLG_TITLES[dialog.action], T::largeGold);
    Rect input{ box.x + Dlg::pad, box.y + Dlg::inputY, box.w - 2 * Dlg::pad, Dlg::inputH };
    drawBox(input, Box::input);
    bool cursorOn = fmodf(ui.time, Dlg::cursorPeriod) < Dlg::cursorPeriod / 2;
    drawText(input.x + Dlg::inputPadX, input.y + Dlg::inputPadY,
        (dialog.input.empty() ? wstring(L"0") : dialog.input) + (cursorOn ? L"|" : L""), T::input);
    float cx = box.x + Dlg::pad;
    for (const auto& chip : dialog.chips) {
        float cw = textWidth(chip.first, FS::label) + 2 * Dlg::chipPadX;
        if (button({ cx, box.y + Dlg::chipsY, cw, Dlg::chipH }, chip.first, B_NORMAL, true, FS::label))
            dialog.input = to_wstring(chip.second);
        cx += cw + Dlg::chipGap;
    }
    if (button({ box.x + Dlg::okX,     box.y + Dlg::buttonsY, Dlg::btnW, Dlg::btnH }, L"ОК", B_PRIMARY, true, FS::large)) applyDialog();
    if (button({ box.x + Dlg::cancelX, box.y + Dlg::buttonsY, Dlg::btnW, Dlg::btnH }, L"Отмена", B_DANGER, true, FS::large)) dialog.open = false;
    flushFrame();
    dialog.drawing = false;
}

void drawSetupScreen() {
    backdrop();
    drawText(Menu::titleX, Menu::titleY, L"РЫБОВОДЧЕСКОЕ ХОЗЯЙСТВО", T::title);
    drawText(Menu::titleX, Menu::hintY, L"Настройте модель: -/+ меняют на шаг, --/++ на десять шагов", T::hint);
    if (worldCfg.lossMin > worldCfg.lossMax) worldCfg.lossMax = worldCfg.lossMin;
    int n = (int)worldSettings.size();
    settingsBlock(worldSettings, 0, Menu::wSplit, Menu::w1);
    settingsBlock(worldSettings, Menu::wSplit, n, Menu::w2);
    panel(Menu::priceBox);
    drawText(Menu::priceBox.x + Menu::priceTextX, Menu::priceBox.y + Menu::priceHeadY, L"Расписание цен контракта", T::labelGold);
    vector<Back::ContractPeriod> periods = makePeriods(worldCfg);
    for (int i = 0; i < (int)periods.size() && i < Menu::priceMax; ++i) {
        const auto& p = periods[i];
        float x = Menu::priceBox.x + Menu::priceTextX + (i / Menu::pricePerCol) * Menu::priceColW;
        float y = Menu::priceBox.y + Menu::priceRowsY + (i % Menu::pricePerCol) * Menu::priceRowH;
        drawText(x, y, to_wstring(p.from_week) + L"-" + to_wstring(p.to_week) + L" нед.  рыба " + fmtMoney(p.fish_price) + L"  корм " + fmtMoney(p.feed_price), T::note);
    }
    if (button(Menu::btnPlay, L"ИГРАТЬ ВРУЧНУЮ", B_PRIMARY, true, FS::large)) startGame(false);
    if (button(Menu::btnBot, L"НАСТРОИТЬ БОТА", B_NORMAL, true, FS::large)) ui.screen = SCREEN_BOT_SETUP;
}

void drawCompareTable() {
    panel(Menu::cmpBox);
    drawText(Menu::cmpBox.x + Menu::cmpTextX, Menu::cmpBox.y + Menu::cmpHeadY, L"Сравнение стратегий на текущих параметрах (прогон без анимации)", T::labelGold);
    if (!haveCompare) {
        drawText(Menu::cmpBox.x + Menu::cmpTextX, Menu::cmpBox.y + Menu::cmpEmptyY,
            L"Нажмите \"СРАВНИТЬ\": бот проведёт хозяйство четыре раза, по одной стратегии.", T::labelMuted);
        return;
    }
    int best = 0;
    for (int s = 1; s < STRATEGY_COUNT; ++s)
        if (!cmpResults[s].broke && (cmpResults[best].broke || cmpResults[s].capital > cmpResults[best].capital)) best = s;
    for (int s = 0; s < STRATEGY_COUNT; ++s) {
        const Result& r = cmpResults[s];
        TS st = T::label.with(r.broke ? Clr::bad : (s == best ? Clr::good : Clr::text));
        float y = Menu::cmpBox.y + Menu::cmpRowsY + s * Menu::cmpRowH;
        auto cell = [&](int col, const wstring& text) { drawText(Menu::cmpBox.x + Menu::cmpCols[col], y, text, st); };
        cell(0, STRAT_FULL[s]);
        cell(1, fmtMoney(r.capital) + L" руб.");
        cell(2, L"x" + fmtNum(r.capital0 > 0 ? r.capital / r.capital0 : 0, 2));
        cell(3, L"неустойка " + fmtMoney(r.penalty));
        cell(4, r.broke ? L"банкротство" : (s == best ? L"лучшая" : L"успех"));
    }
}

void drawBotSetupScreen() {
    backdrop();
    drawText(Menu::titleX, Menu::titleY, L"НАСТРОЙКИ БОТА-УПРАВЛЯЮЩЕГО", T::title);
    int n = (int)botSettings.size();
    settingsBlock(botSettings, 0, Menu::bSplit, Menu::b1);
    settingsBlock(botSettings, Menu::bSplit, n, Menu::b2);
    drawCompareTable();
    if (button(Menu::btnBack, L"НАЗАД")) ui.screen = SCREEN_SETUP;
    if (button(Menu::btnCmp, L"СРАВНИТЬ", B_NORMAL, true, FS::sub)) {
        for (int s = 0; s < STRATEGY_COUNT; ++s) {
            BotSettings b = botCfg;
            b.strategy = s;
            cmpResults[s] = runHeadless(worldCfg, b);
        }
        haveCompare = true;
    }
    if (button(Menu::btnRun, L"ЗАПУСТИТЬ БОТА", B_PRIMARY, true, FS::large)) startGame(true);
}

void drawStatsPanel(const vector<Back::Prud>& ps, bool broke) {
    panel(Lay::stats);
    Rect c = contentOf(Lay::stats);
    Column col{ c.x, c.y, c.w };
    int young = 0, old = 0;
    double feedAll = 0;
    for (const auto& p : ps) { young += p.fish.young; old += p.fish.old; feedAll += p.current_feed; }
    int week = farm.back.get_week(), weeks = farm.params.weeks;
    col.line(L"Неделя " + to_wstring(week) + L" / " + to_wstring(weeks), T::head);
    bar(col.x, col.y, col.w, week / (float)weeks, Clr::barWeek);
    col.skip(Lay::barGap);
    col.line(fmtMoney(farm.back.get_money()) + L" руб.", T::head.with(broke ? Clr::bad : Clr::gold));
    col.line(L"Капитал: " + fmtMoney(farm.capital()) + L" руб.", T::body);
    col.line(L"Рыба: " + to_wstring(young + old) + L" кг (молодь " + to_wstring(young) + L", взр. " + to_wstring(old) + L")", T::label);
    col.line(L"Корма в запасе: " + fmtNum(feedAll, 1) + L" кг", T::labelMuted);
}

void drawContractPanel() {
    panel(Lay::contract);
    Rect r = contentOf(Lay::contract);
    Column col{ r.x, r.y, r.w };
    Back::contract c = farm.back.get_contract();
    const Back::ContractPeriod& per = farm.periodAt(farm.week());
    bool hasPrev = per.from_week > 1;
    double prevFish = hasPrev ? farm.fishPriceAt(per.from_week - 1) : per.fish_price;
    double prevFeed = hasPrev ? farm.feedPriceAt(per.from_week - 1) : per.feed_price;
    col.line(c.active ? L"КОНТРАКТ (недели " + to_wstring(per.from_week) + L"-" + to_wstring(per.to_week) + L")"
        : wstring(L"КОНТРАКТ ЗАВЕРШЁН"), T::labelGold);
    wstring fishText = L"Рыба " + fmtMoney(per.fish_price), feedText = L"Корм " + fmtMoney(per.feed_price);
    drawText(col.x, col.y, fishText, T::label);
    if (per.fish_price != prevFish)
        trendArrow(col.x + textWidth(fishText, FS::label) + Lay::arrowGap, col.y + 1, per.fish_price > prevFish, per.fish_price > prevFish);
    float feedX = col.x + Lay::priceColW;
    drawText(feedX, col.y, feedText, T::label);
    if (per.feed_price != prevFeed)
        trendArrow(feedX + textWidth(feedText, FS::label) + Lay::arrowGap, col.y + 1, per.feed_price > prevFeed, per.feed_price < prevFeed);
    col.skip(FS::label + Lay::lineGap + 2);
    col.line(L"Поставка: " + to_wstring(c.sold_fish) + L" / " + to_wstring(c.need_fish) + L" кг", T::noteMuted);
    bar(col.x, col.y - 2, col.w, c.need_fish > 0 ? (float)c.sold_fish / c.need_fish : 1.f, Clr::barContract);
    col.skip(Lay::barGap - 2);
    col.line(L"Корм за неделю: " + fmtMoney(farm.spentThisWeek) + L" / " + fmtMoney(c.week_korm) + L" руб.", T::noteMuted);
    bar(col.x, col.y - 2, col.w, c.week_korm > 0 ? (float)(farm.spentThisWeek / c.week_korm) : 1.f, Clr::barFeed);
}

void drawPondPanel(const Back::Prud& p, bool broke) {
    panel(Lay::pond);
    Rect r = contentOf(Lay::pond);
    Column col{ r.x, r.y, r.w };
    int id = ui.selectedPond;
    if (!p.active) {
        col.line(pondTitle(id) + L" - свободен", T::bodyGold);
        col.line(L"Выберите вид рыбы:", T::capMuted);
        col.skip(Lay::spTop);
        for (int i = 0; i < (int)farm.types.size(); ++i) {
            const Back::information& info = farm.types[i];
            Rect cell = gridCell({ col.x, col.y, col.w, 0 }, i % 2, i / 2, 2, Lay::spBtnH, Lay::spGap);
            if (button(cell, wide(info.name), B_NORMAL, !broke, FS::note)) farm.newPond(id, info.type_id);
        }
        return;
    }
    double need = farm.feedNeed(p), stock = p.current_feed;
    int growth = farm.weekGrowth[id];
    col.line(L"ПРУД " + to_wstring(id + 1) + L" - " + farm.fishName(p.type_id), T::bodyGold);
    col.line(p.cleaning ? L"Состояние: очистка" : L"Состояние: откорм", T::note.with(p.cleaning ? Clr::muted : Clr::good));
    col.line(L"Корма в запасе: " + fmtNum(stock, 1) + L" кг", T::note);
    col.line(L"Нужно на неделю: " + fmtNum(need, 1) + L" кг", T::note.with(stock + 1e-6 < need ? Clr::bad : Clr::muted));
    col.line(L"Прирост за неделю: " + wstring(growth >= 0 ? L"+" : L"") + to_wstring(growth) + L" кг", T::note.with(growth >= 0 ? Clr::good : Clr::bad));
    col.line(L"Загрязнение: " + fmtNum(p.pollution * 100, 0) + L" %", T::note.with(p.pollution >= 0.5 ? Clr::bad : Clr::muted));
    bool can = !p.cleaning && !broke;
    Rect area{ col.x, col.y + Lay::actGap, col.w, 0 };
    auto cell = [&](int cc, int row) { return gridCell(area, cc, row, 2, Lay::actBtnH, Lay::gridGap); };
    if (button(cell(0, 0), L"Купить корм", B_NORMAL, can, FS::label)) openDialog(DLG_FEED);
    if (button(cell(1, 0), L"+ Молодь", B_NORMAL, can, FS::label)) openDialog(DLG_YOUNG);
    if (button(cell(0, 1), L"Продать", B_PRIMARY, can, FS::label)) openDialog(DLG_SELL);
    if (button(cell(1, 1), L"Очистить", B_NORMAL, can, FS::label)) { farm.clean(id); farm.delete_prud(id); }
}

void drawControls(bool broke) {
    Rect c = contentOf(Lay::pond);
    float x = c.x, w = c.w, y = Lay::controlsY;
    if (!ui.botMode) {
        if (button({ x, y, w, Lay::mainBtnH }, L"СЛЕДУЮЩАЯ НЕДЕЛЯ  [Space]", B_PRIMARY, !broke && !farm.over, FS::sub)) stepWeek();
    }
    else {
        float bx = x;
        auto next = [&](float width) { Rect r{ bx, y, width, Lay::mainBtnH }; bx += width + Lay::gridGap; return r; };
        if (button(next(96), ui.botPaused ? L"Старт" : L"Пауза", B_PRIMARY, !farm.over)) ui.botPaused = !ui.botPaused;
        if (button(next(66), L"Шаг", B_NORMAL, !farm.over)) { ui.botPaused = true; stepWeek(); }
        if (button(next(40), L"-", B_NORMAL, true, FS::large))
            botCfg.speedTenths = min(BotRules::speedMax, botCfg.speedTenths + BotRules::speedStep);
        if (button(next(40), L"+", B_NORMAL, true, FS::large))
            botCfg.speedTenths = max(BotRules::speedMin, botCfg.speedTenths - BotRules::speedStep);
        drawText(bx + 6, y + (Lay::mainBtnH - FS::note) / 2 - Btn::textLift, fmtNum(botCfg.speedTenths / 10.0, 1) + L" с", T::noteMuted);
    }
    Rect row{ x, y + Lay::mainBtnH + Lay::row2Gap, w, 0 };
    auto cell = [&](int cc) { return gridCell(row, cc, 0, 3, Lay::row2H, 0); };
    if (button(cell(0), ui.botMode ? L"Бот: вкл" : L"Бот: выкл", ui.botMode ? B_PRIMARY : B_NORMAL, !farm.over, FS::note)) {
        ui.botMode = !ui.botMode;
        bot.cfg = botCfg;
        ui.botPaused = false;
    }
    if (button(cell(1), L"Графики", B_NORMAL, true, FS::note)) { ui.tab = 0; ui.screen = SCREEN_GRAPH; }
    if (button(cell(2), L"Меню", B_DANGER, true, FS::note)) ui.screen = SCREEN_SETUP;
}

void drawLog() {
    panel(Lay::logBox);
    Rect c = contentOf(Lay::logBox);
    int n = (int)farm.log.size(), first = max(0, n - Lay::logLines);
    for (int i = first; i < n; ++i) {
        const LogEntry& e = farm.log[i];
        wstring s = e.text.size() > Lay::logChars ? e.text.substr(0, Lay::logChars) : e.text;
        drawText(c.x, Lay::logBox.y + Lay::logPadY + (i - first) * Lay::logLineH, s, T::cap.with(LOG_COLORS[e.kind]));
    }
}

void drawHud() {
    vector<Back::Prud> ps = farm.back.get_pruds();
    bool broke = farm.back.get_no_money();
    if (ui.selectedPond >= (int)ps.size()) ui.selectedPond = 0;
    drawStatsPanel(ps, broke);
    drawContractPanel();
    drawPondPanel(ps[ui.selectedPond], broke);
    drawControls(broke);
    drawLog();
}

void drawGameScreen() {
    backdrop();
    batch.rect(Lay::sidebar, Clr::sidebarBg);
    vector<Back::Prud> ps = farm.back.get_pruds();
    int shown = (int)min(ps.size(), ponds.size());
    for (int i = 0; i < shown; ++i) drawPond(i, ps[i], i == ui.selectedPond);
    for (int i = 0; i < shown; ++i) drawPondLabel(i, ps[i]);
    if (mouse.clicked && mouse.x < Lay::fieldW - 20 && !dialog.open)
        for (int i = 0; i < (int)ponds.size(); ++i) {
            float dx = (mouse.x - ponds[i].c.x) / (ponds[i].rx * Ponds::clickR);
            float dy = (mouse.y - ponds[i].c.y) / (ponds[i].ry * Ponds::clickR);
            if (dx * dx + dy * dy < 1.f) { ui.selectedPond = i; break; }
        }
    drawText(Lay::hintX, Lay::hintY, L"Щёлкните по пруду, чтобы выбрать его", T::sceneHint);
    drawHud();
}

struct SeriesDef { wstring name; Col color; function<double(const Rec&)> value; };
struct ChartTab { wstring title; vector<SeriesDef> series; };
vector<ChartTab> makeChartTabs() {
    return {
        { L"Популяция", {
            { L"Всего",    Col(90, 220, 240),  [](const Rec& r) { return (double)r.total; } },
            { L"Молодь",   Col(130, 214, 120), [](const Rec& r) { return (double)r.young; } },
            { L"Взрослая", Col(244, 170, 70),  [](const Rec& r) { return (double)r.old; } } } },
        { L"Финансы", {
            { L"Наличные",       Col(120, 224, 130), [](const Rec& r) { return r.money; } },
            { L"Капитал",        Col(244, 200, 106), [](const Rec& r) { return r.capital; } },
            { L"Расход на корм", Col(255, 140, 90),  [](const Rec& r) { return r.spent; } } } },
        { L"Контракт", {
            { L"Продано",          Col(244, 214, 90),  [](const Rec& r) { return (double)r.sold; } },
            { L"План контракта",   Col(240, 100, 96),  [](const Rec& r) { return (double)r.need; } },
            { L"Равномерный темп", Col(190, 210, 224), [](const Rec& r) { return (double)r.need * r.week / max(1, farm.params.weeks); } } } },
        { L"Корм", {
            { L"Запас корма",       Col(214, 164, 100), [](const Rec& r) { return r.feed; } },
            { L"Закупка за неделю", Col(255, 150, 80),  [](const Rec& r) { return r.spent; } } } },
    };
}

void drawChart(const ChartTab& tab) {
    const vector<Rec>& hist = farm.history;
    const int weeks = max(1, farm.params.weeks);
    if (hist.size() < 2) {
        drawText(Chart::emptyX, Chart::emptyY, L"Недостаточно данных: проведите хотя бы одну неделю", T::largeGold.with(Clr::muted));
        return;
    }
    vector<vector<double>> values(tab.series.size());
    double hi = 1, lo = 0;
    for (size_t s = 0; s < tab.series.size(); ++s)
        for (const Rec& r : hist) {
            double v = tab.series[s].value(r);
            values[s].push_back(v);
            hi = max(hi, v);
            lo = min(lo, v);
        }
    hi = niceMax(hi);
    if (lo < 0) lo = -niceMax(-lo);
    double span = hi - lo;
    auto px = [&](int wk) { return Chart::plot.x + Chart::plot.w * wk / (float)weeks; };
    auto py = [&](double v) { return Chart::plot.y + Chart::plot.h - (float)((v - lo) / span) * Chart::plot.h; };
    for (int i = 0; i <= Chart::gridLines; ++i) {
        double v = lo + span * i / Chart::gridLines;
        batch.rect({ Chart::plot.x, py(v) - 0.5f, Chart::plot.w, 1.f }, i == 0 ? Chart::gridMain : Chart::gridMinor);
        wstring label = fmtMoney(v);
        drawText(Chart::plot.x - Chart::axisGap - textWidth(label, FS::cap), py(v) - Chart::axisLift, label, T::capMuted);
    }
    for (int wk = 0; wk <= weeks; wk += max(1, weeks / Chart::maxWeekTicks)) {
        batch.rect({ px(wk) - 0.5f, Chart::plot.y, 1.f, Chart::plot.h }, Chart::gridVert);
        drawText(px(wk) - Chart::weekShiftX, Chart::plot.y + Chart::plot.h + Chart::weekGap, to_wstring(wk), T::capMuted);
    }
    for (size_t s = 0; s < tab.series.size(); ++s) {
        vector<Vec> line;
        for (size_t i = 0; i < hist.size(); ++i) line.push_back({ px(hist[i].week), py(values[s][i]) });
        batch.polyline(line, Chart::lineW, tab.series[s].color);
        for (const Vec& q : line) batch.circle(q, Chart::pointR, tab.series[s].color, 10);
    }
    if (Chart::plot.has(mouse.x, mouse.y)) {
        int wk = clamp((int)lroundf((mouse.x - Chart::plot.x) / Chart::plot.w * weeks), 0, weeks), idx = -1;
        for (size_t i = 0; i < hist.size(); ++i) if (hist[i].week == wk) idx = (int)i;
        if (idx >= 0) {
            batch.rect({ px(wk) - 0.5f, Chart::plot.y, 1.5f, Chart::plot.h }, Chart::cursorLine);
            Rect tip{ min(px(wk) + Chart::tipOffset, Chart::plot.x + Chart::plot.w - Chart::tipW - Chart::tipPad), Chart::plot.y + Chart::tipPad,
                      Chart::tipW, Chart::tipHeadH + tab.series.size() * Chart::tipRowH };
            drawBox(tip, Box::tooltip);
            drawText(tip.x + Chart::tipPad, tip.y + 5, L"Неделя " + to_wstring(wk), T::labelGold);
            for (size_t s = 0; s < tab.series.size(); ++s)
                drawText(tip.x + Chart::tipPad, tip.y + Chart::tipHeadH + s * Chart::tipRowH, tab.series[s].name + L": " + fmtMoney(values[s][idx]), T::cap);
        }
    }
    float lx = Chart::plot.x + Chart::tipPad;
    for (const auto& s : tab.series) {
        batch.rect({ lx, Chart::plot.y - Chart::legendY, Chart::swatch, Chart::swatch }, s.color);
        drawText(lx + Chart::swatch + Chart::legendGap, Chart::plot.y - Chart::legendY - 4, s.name, T::note);
        lx += Chart::swatch + Chart::legendGap + textWidth(s.name, FS::note) + Chart::legendItemGap;
    }
}

void drawGraphScreen() {
    static const vector<ChartTab> tabs = makeChartTabs();
    batch.rect({ 0, 0, W, H }, Clr::graphBg);
    const wstring title = L"АНАЛИТИКА И ГРАФИКИ";
    drawText((W - textWidth(title, FS::title)) / 2, Chart::titleY, title, T::title.with(Clr::gold));
    for (int i = 0; i < (int)tabs.size(); ++i)
        if (button({ Chart::tabsX + i * Chart::tabStep, Chart::tabsY, Chart::tabW, Chart::tabH }, tabs[i].title, B_NORMAL, ui.tab != i, FS::sub)) ui.tab = i;
    panel(Chart::panel);
    drawChart(tabs[ui.tab]);
    if (button(Chart::btnBack, L"НАЗАД")) ui.screen = farm.over ? SCREEN_FINISH : SCREEN_GAME;
}

void drawFinishScreen() {
    drawGameScreen();
    flushFrame();
    batch.rect({ 0, 0, W, H }, Clr::finishDim);
    Result r = farm.result();
    panel(Fin::box);
    drawText(Fin::box.x + Fin::titleX, Fin::box.y + Fin::titleY, r.broke ? L"БАНКРОТСТВО" : L"КОНТРАКТ ЗАВЕРШЁН", T::huge.with(r.broke ? Clr::bad : Clr::good));
    double ratio = r.capital0 > 0 ? r.capital / r.capital0 : 0;
    float y = Fin::box.y + Fin::rowsY;
    auto row = [&](const wstring& key, const wstring& value, Col color) {
        drawText(Fin::box.x + Fin::rowPadX, y, key, T::labelMuted);
        drawText(Fin::box.x + Fin::box.w - Fin::rowPadX - textWidth(value, FS::sub), y, value, TS{ FS::sub, color });
        batch.rect({ Fin::box.x + Fin::rowPadX, y + Fin::rowLineY, Fin::box.w - 2 * Fin::rowPadX, 1 }, Fin::rowLine);
        y += Fin::rowStep;
        };
    row(L"Недель прошло", to_wstring(r.weeks) + L" из " + to_wstring(farm.params.weeks), Clr::text);
    row(L"Наличные", fmtMoney(r.cash) + L" руб.", Clr::text);
    row(L"Рыба в прудах", to_wstring(r.fish) + L" кг = " + fmtMoney(r.fishValue) + L" руб.", Clr::text);
    row(L"Итоговый капитал", fmtMoney(r.capital) + L" руб.", Clr::gold);
    row(L"Стартовый капитал", fmtMoney(r.capital0) + L" руб.", Clr::text);
    row(L"Рост капитала", L"x" + fmtNum(ratio, 2), ratio >= 1 ? Clr::good : Clr::bad);
    row(L"Поставка по контракту", to_wstring(r.sold) + L" из " + to_wstring(farm.params.needFish) + L" кг", r.sold >= farm.params.needFish ? Clr::good : Clr::bad);
    row(L"Неустойка", fmtMoney(r.penalty) + L" руб.", r.penalty > 0 ? Clr::bad : Clr::good);
    float by = Fin::box.y + Fin::box.h - Fin::buttonsY;
    auto cell = [&](int i) { return Rect{ Fin::box.x + Fin::btnPad + i * Fin::btnStep, by, Fin::btnW, Fin::btnH }; };
    if (button(cell(0), L"ГРАФИКИ", B_NORMAL, true, FS::sub)) { ui.tab = 1; ui.screen = SCREEN_GRAPH; }
    if (button(cell(1), L"ЕЩЁ РАЗ", B_PRIMARY, true, FS::sub)) startGame(ui.botMode);
    if (button(cell(2), L"В МЕНЮ", B_DANGER, true, FS::sub)) ui.screen = SCREEN_SETUP;
}

void initApp() {
    window.create(sf::VideoMode(sf::Vector2u{ (unsigned)W, (unsigned)H }), APP_TITLE);
    window.setFramerateLimit(FPS);
    if (!font.openFromFile(FONT_FILE))
        throw runtime_error("Не найден шрифт font.ttf");
    farm.init(worldCfg);
    buildSettings();
    buildPonds(farm);
}

void handleEvents() {
    while (auto e = window.pollEvent()) {
        if (e->is<sf::Event::Closed>()) window.close();
        if (auto* m = e->getIf<sf::Event::MouseButtonPressed>())
            if (m->button == sf::Mouse::Button::Left) mouse.clicked = true;
        if (dialog.open)
            if (auto* t = e->getIf<sf::Event::TextEntered>()) {
                if (t->unicode == U'\r') applyDialog();
                else if (t->unicode == U'\b') { if (!dialog.input.empty()) dialog.input.pop_back(); }
                else if (t->unicode >= U'0' && t->unicode <= U'9' && dialog.input.size() < Dlg::maxDigits) dialog.input += (wchar_t)t->unicode;
            }
        if (auto* k = e->getIf<sf::Event::KeyPressed>()) {
            using Key = sf::Keyboard::Scancode;
            if (k->scancode == Key::Escape) { if (dialog.open) dialog.open = false; else window.close(); }
            if (ui.screen == SCREEN_GAME && !dialog.open) {
                if (k->scancode == Key::Space) { if (ui.botMode) ui.botPaused = !ui.botPaused; else stepWeek(); }
                if (k->scancode == Key::Left)  ui.selectedPond = max(0, ui.selectedPond - 1);
                if (k->scancode == Key::Right) ui.selectedPond = min((int)ponds.size() - 1, ui.selectedPond + 1);
            }
        }
    }
}

void update(float dt) {
    if (ui.screen == SCREEN_GAME) {
        updateFish(farm, dt, ui.time);
        checkGameOver();
        if (ui.botMode && !ui.botPaused && !dialog.open && ui.screen == SCREEN_GAME) {
            ui.botTimer += dt;
            if (ui.botTimer >= botCfg.speedTenths / 10.f) { ui.botTimer = 0; stepWeek(); }
        }
    }
    if (ui.screen == SCREEN_FINISH) updateFish(farm, dt, ui.time);
}

void render() {
    window.clear(Clr::windowBg);
    switch (ui.screen) {
    case SCREEN_SETUP:     drawSetupScreen(); break;
    case SCREEN_BOT_SETUP: drawBotSetupScreen(); break;
    case SCREEN_GAME:      drawGameScreen(); break;
    case SCREEN_GRAPH:     drawGraphScreen(); break;
    case SCREEN_FINISH:    drawFinishScreen(); break;
    }
    flushFrame();
    drawDialog();
    window.display();
}

void mainLoop() {
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = min(clock.restart().asSeconds(), MAX_DT);
        ui.time += dt;
        mouse.clicked = false;
        sf::Vector2f pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        mouse.x = pos.x;
        mouse.y = pos.y;
        handleEvents();
        update(dt);
        render();
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    try {
        initApp();
        mainLoop();
    }
    catch (const exception& e) {
        cerr << "Critical Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}