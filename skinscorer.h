#ifndef SKINSCORER_H
#define SKINSCORER_H

#include <QString>
#include <QVector>
#include "Score.h"
#include "csvparser.h"

// User's skin profile from the UI sliders
struct SkinProfile {
    QString skinType;       // "Oily", "Dry", "Sensitive", "Combination"
    int     acneLevel;      // 0-10
    int     drynessLevel;   // 0-10
    int     hyperpigLevel;  // 0-10
};

// One ingredient after scoring
struct ScoredIngredient {
    IngredientRecord record;
    Score<int>       safetyScore;
    Score<int>       effectivenessScore;
    Score<int>       overallScore;
    QString          recommendation;
};

class SkinScorer {
public:
    SkinScorer();

    // Score one ingredient against a skin profile
    ScoredIngredient score(const IngredientRecord& record,
                           const SkinProfile& profile);

    // Score all records, returns sorted results
    QVector<ScoredIngredient> scoreAll(
        const QVector<IngredientRecord>& records,
        const SkinProfile& profile);

    // Returns 3 formulation suggestions
    QVector<QString> getFormulationSuggestions(
        const QVector<ScoredIngredient>& scored);

private:
    int calcSafetyScore(const IngredientRecord& record);
    int calcEffectivenessScore(const IngredientRecord& record,
                               const SkinProfile& profile);

    // Harmful chemicals from CSCP
    QVector<QString> allergenKeywords = {
        "coal tar", "formaldehyde", "lead", "mercury",
        "asbestos", "estragole", "carbon black",
        "styrene", "talc", "chromium"
    };

    // Beneficial per skin type
    QVector<QString> oilyBeneficial     = {
        "salicylic", "niacinamide", "glycolic", "zinc", "kaolin"
    };
    QVector<QString> dryBeneficial      = {
        "hyaluronic", "ceramide", "glycerin", "shea", "squalane"
    };
    QVector<QString> sensitiveBeneficial = {
        "zinc oxide", "aloe", "chamomile", "oat", "allantoin"
    };
    QVector<QString> comboBeneficial    = {
        "niacinamide", "hyaluronic", "glycolic", "peptide"
    };

    // Ingredients to avoid per skin type
    QVector<QString> oilyAvoid      = {
        "mineral oil", "coconut oil", "lanolin", "petrolatum"
    };
    QVector<QString> dryAvoid       = {
        "alcohol denat", "sulfate", "benzoyl"
    };
    QVector<QString> sensitiveAvoid = {
        "fragrance", "parfum", "paraben", "sulfate", "alcohol denat"
    };
    QVector<QString> comboAvoid     = {
        "petrolatum", "isopropyl myristate"
    };
};

#endif // SKINSCORER_H