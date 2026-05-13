#include "SkinScorer.h"
#include <algorithm>

SkinScorer::SkinScorer() {}

// ── Score one ingredient ──
ScoredIngredient SkinScorer::score(const IngredientRecord& record,
                                   const SkinProfile& profile)
{
    ScoredIngredient result;
    result.record = record;

    int safety        = calcSafetyScore(record);
    int effectiveness = calcEffectivenessScore(record, profile);

    // Formula: 60% safety + 40% effectiveness
    int overall = static_cast<int>(safety * 0.6 + effectiveness * 0.4);

    result.safetyScore        = Score<int>(safety,        10, "Safety");
    result.effectivenessScore = Score<int>(effectiveness, 10, "Effectiveness");
    result.overallScore       = Score<int>(overall,       10, "Overall");

    if (overall >= 7)
        result.recommendation = "✅ Recommended for " + profile.skinType;
    else if (overall >= 4)
        result.recommendation = "⚠️ Use with caution";
    else
        result.recommendation = "❌ Avoid for " + profile.skinType + " skin";

    return result;
}

// ── Score all records ──
QVector<ScoredIngredient> SkinScorer::scoreAll(
    const QVector<IngredientRecord>& records,
    const SkinProfile& profile)
{
    QVector<ScoredIngredient> results;
    results.reserve(records.size());

    for (const auto& r : records)
        results.append(score(r, profile));

    std::sort(results.begin(), results.end(),
              [](const ScoredIngredient& a, const ScoredIngredient& b) {
                  return a.overallScore.getValue() > b.overallScore.getValue();
              });

    return results;
}

// ── Safety score 1-10 ──
int SkinScorer::calcSafetyScore(const IngredientRecord& record) {
    QString chemLower = record.chemicalName.toLower();

    // Start at 7 — neutral assumption
    int score = 7;

    // Discontinued = automatic penalty
    if (record.isDiscontinued) score -= 2;

    // Tier 1 — most dangerous (CSCP carcinogens/reproductive toxins)
    QStringList tier1 = {
        "formaldehyde", "lead", "mercury", "asbestos",
        "arsenic", "chromium hexavalent", "cadmium",
        "coal tar", "estragole", "carbon black", "styrene"
    };
    for (const QString& kw : tier1)
        if (chemLower.contains(kw)) { score = 1; return score; }

    // Tier 2 — CSCP allergen list (moderate-severe)
    for (const QString& kw : allergenKeywords)
        if (chemLower.contains(kw)) { score -= 3; break; }

    // Tier 3 — common irritants (mild penalty)
    QStringList tier3 = {
        "paraben", "fragrance", "parfum", "sulfate",
        "phthalate", "triclosan", "oxybenzone",
        "butylated hydroxytoluene", "petroleum",
        "mineral oil", "talc", "nickel"
    };
    for (const QString& kw : tier3)
        if (chemLower.contains(kw)) { score -= 1; break; }

    // Tier 4 — well-known safe ingredients (boost)
    QStringList safe = {
        "water", "aqua", "glycerin", "aloe vera",
        "tocopherol", "panthenol", "allantoin",
        "niacinamide", "hyaluronic acid", "ceramide",
        "zinc oxide", "dimethicone", "stearic acid",
        "cetyl alcohol", "shea butter", "jojoba",
        "retinol", "ascorbic acid", "squalane"
    };
    for (const QString& kw : safe)
        if (chemLower.contains(kw)) { score += 2; break; }

    return std::max(1, std::min(10, score));
}

// ── Effectiveness score 1-10 ──
int SkinScorer::calcEffectivenessScore(const IngredientRecord& record,
                                       const SkinProfile& profile)
{
    QString chemLower = record.chemicalName.toLower();
    QString skin      = profile.skinType.toLower();
    int score         = 5;

    QVector<QString>* beneficial = nullptr;
    QVector<QString>* avoid      = nullptr;

    if      (skin == "oily")        { beneficial = &oilyBeneficial;
        avoid      = &oilyAvoid; }
    else if (skin == "dry")         { beneficial = &dryBeneficial;
        avoid      = &dryAvoid; }
    else if (skin == "sensitive")   { beneficial = &sensitiveBeneficial;
        avoid      = &sensitiveAvoid; }
    else if (skin == "combination") { beneficial = &comboBeneficial;
        avoid      = &comboAvoid; }
    else {
        static QVector<QString> allBeneficial = {
            "glycerin", "aloe", "panthenol", "niacinamide",
            "hyaluronic", "ceramide", "zinc oxide", "allantoin",
            "tocopherol", "dimethicone", "stearic"
        };
        static QVector<QString> allAvoid = {
            "formaldehyde", "paraben", "sulfate",
            "fragrance", "parfum", "lead", "mercury",
            "coal tar", "asbestos"
        };
        beneficial = &allBeneficial;
        avoid      = &allAvoid;
    }

    if (beneficial)
        for (const QString& kw : *beneficial)
            if (chemLower.contains(kw)) { score += 3; break; }

    if (avoid)
        for (const QString& kw : *avoid)
            if (chemLower.contains(kw)) { score -= 3; break; }

    // Slider boosts
    if (profile.acneLevel > 5)
        if (chemLower.contains("salicylic") ||
            chemLower.contains("niacinamide")) score += 2;

    if (profile.drynessLevel > 5)
        if (chemLower.contains("hyaluronic") ||
            chemLower.contains("ceramide"))    score += 2;

    if (profile.hyperpigLevel > 5)
        if (chemLower.contains("niacinamide") ||
            chemLower.contains("ascorbic") ||
            chemLower.contains("kojic"))       score += 2;

    return std::max(1, std::min(10, score));
}

// ── Three formulation suggestions ──
QVector<QString> SkinScorer::getFormulationSuggestions(
    const QVector<ScoredIngredient>& scored)
{
    QVector<QString> suggestions;
    int safe = 0, moderate = 0, danger = 0;

    for (const auto& s : scored) {
        int v = s.overallScore.getValue();
        if      (v >= 6) safe++;       // lowered threshold
        else if (v >= 3) moderate++;
        else             danger++;
    }

    int total = scored.size();

    suggestions.append(
        QString("✅ Safest Option: %1% of ingredients score 6+/10 "
                "and are suitable for your skin type.")
            .arg(total > 0 ? (safe * 100 / total) : 0));

    suggestions.append(
        QString("⚠️ Moderate Option: %1 ingredients carry moderate risk "
                "— patch test before full use.")
            .arg(moderate));

    suggestions.append(
        QString("❌ Avoid: %1 ingredients flagged as high risk by the "
                "California Safe Cosmetics database for your profile.")
            .arg(danger));

    return suggestions;
}