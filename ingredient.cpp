#include "ingredient.h"

// Base class constructor
Ingredient::Ingredient(QString name, QString function,
                       QString casNumber, int score)
    : name(name), function(function),
    casNumber(casNumber), safetyScore(score) {}

// ActiveIngredient constructor
ActiveIngredient::ActiveIngredient(QString name, QString function,
                                   QString casNumber, int score,
                                   QString concern)
    : Ingredient(name, function, casNumber, score),
    targetConcern(concern) {}

// AllergenIngredient constructor
AllergenIngredient::AllergenIngredient(QString name, QString function,
                                       QString casNumber, int score,
                                       QString allergenType)
    : Ingredient(name, function, casNumber, score),
    allergenType(allergenType) {}

// Allergens get their score cut in half as a penalty
int AllergenIngredient::getScore() const {
    return safetyScore / 2;
}

// Factory method
Ingredient* Ingredient::fromRecord(const IngredientRecord& record) {

    QStringList allergenKeywords = {
        "coal tar", "formaldehyde", "lead", "mercury",
        "asbestos", "estragole", "titanium dioxide",
        "carbon black", "styrene", "talc"
    };

    QStringList activeKeywords = {
        "niacinamide", "retinol", "vitamin c", "hyaluronic",
        "salicylic", "glycolic", "peptide", "ceramide",
        "zinc oxide", "ascorbic"
    };

    QString chemLower = record.chemicalName.toLower();
    int score = record.isDiscontinued ? 3 : 7;

    for (const QString& keyword : allergenKeywords) {
        if (chemLower.contains(keyword)) {
            return new AllergenIngredient(
                record.chemicalName,
                record.subCategory,
                record.casNumber,
                score,
                "CSCP Flagged"
                );
        }
    }

    for (const QString& keyword : activeKeywords) {
        if (chemLower.contains(keyword)) {
            return new ActiveIngredient(
                record.chemicalName,
                record.subCategory,
                record.casNumber,
                score,
                "Beneficial"
                );
        }
    }

    return new Ingredient(
        record.chemicalName,
        record.subCategory,
        record.casNumber,
        score
        );
}