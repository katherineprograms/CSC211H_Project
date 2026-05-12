#ifndef INGREDIENT_H
#define INGREDIENT_H

#include <QString>
#include "csvparser.h"

class Ingredient {
protected:
    QString name;
    QString function;
    QString casNumber;   // casNumber added for ID
    int safetyScore;

public:
    Ingredient(QString name, QString function,
               QString casNumber, int score);  // unique ID codes assigned to every known chemical in the world.
    virtual ~Ingredient() = default;

    virtual QString getType()  const { return "Base"; }
    virtual int     getScore() const { return safetyScore; }

    QString getName()      const { return name; }
    QString getFunction()  const { return function; }
    QString getCasNumber() const { return casNumber; }

    // Converts a CSV row into the correct Ingredient type
    static Ingredient* fromRecord(const IngredientRecord& record);
};

class ActiveIngredient : public Ingredient {
    QString targetConcern;

public:
    ActiveIngredient(QString name, QString function,
                     QString casNumber, int score,  //casNumber
                     QString concern);

    QString getType()    const override { return "Active"; }
    QString getConcern() const          { return targetConcern; }
};

class AllergenIngredient : public Ingredient {
    QString allergenType;

public:
    AllergenIngredient(QString name, QString function,
                       QString casNumber, int score,  // casNumber
                       QString allergenType);

    QString getType()        const override { return "Allergen"; }
    QString getAllergenType() const          { return allergenType; }
    int     getScore()       const override;
};

#endif // INGREDIENT_H
