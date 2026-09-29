#include "color.h"
#include <cmath>
#include <algorithm>

HSV RgbToHsv(RGB in) {
    float r = in.r / 255.0f;
    float g = in.g / 255.0f;
    float b = in.b / 255.0f;

    float mx = (std::max)({r, g, b});
    float mn = (std::min)({r, g, b});
    float df = mx - mn;

    HSV out{0.f, 0.f, mx};
    if (df > 1e-6f && mx > 1e-6f) {
        out.s = df / mx;
        if      (mx == r) out.h = 60.f * fmodf((g - b) / df, 6.f);
        else if (mx == g) out.h = 60.f * ((b - r) / df + 2.f);
        else              out.h = 60.f * ((r - g) / df + 4.f);
    } else {
        out.s = 0.f;
    }
    if (out.h < 0.f) out.h += 360.f;
    return out;
}

bool IsTargetColor(const uint8_t* bgra, int tolH, int tolS, int tolV) {
    if (bgra[3] == 0 && (bgra[0] | bgra[1] | bgra[2]) != 0)
        return false;

    // ---- COR EXATA DO OUTLINE DO VALORANT ----
    // HEX #D32BFA  ->  RGB 211, 43, 250
    // HSV ~ (290, 0.83, 0.98)
    const float target_h = 290.0f;
    const float target_s = 0.83f;
    const float target_v = 0.98f;

    RGB px{ bgra[2], bgra[1], bgra[0] };
    HSV hsv = RgbToHsv(px);

    // filtros duros: outline é SATURADO e CLARO
    if (hsv.s < 0.60f) return false;   // corta pele, cinza, roupa apagada
    if (hsv.v < 0.55f) return false;   // corta sombra

    float dh = std::fabs(hsv.h - target_h);
    if (dh > 180.f) dh = 360.f - dh;

    bool hOk = dh <= tolH;
    bool sOk = std::fabs(hsv.s - target_s) <= (tolS / 100.0f);
    bool vOk = std::fabs(hsv.v - target_v) <= (tolV / 100.0f);

    return hOk && sOk && vOk;
}