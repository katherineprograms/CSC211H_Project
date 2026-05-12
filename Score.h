#ifndef SCORE_H
#define SCORE_H

#include <QString>
// ── Generic Score<T> template ──
// Satisfies the honors C++ template requirement
// Works for int, double, or float scores
template <typename T>
class Score {
public:
    Score() : value(0), maxValue(10), label("Unscored") {}

    Score(T val, T maxVal, QString lbl)
        : value(val), maxValue(maxVal), label(lbl) {}

    T       getValue()    const { return value; }
    T       getMaxValue() const { return maxValue; }
    QString getLabel()    const { return label; }

    // Returns 0.0 to 1.0
    double getPercentage() const {
        if (maxValue == 0) return 0.0;
        return static_cast<double>(value) /
               static_cast<double>(maxValue);
    }

    // Color for UI display
    QString getColor() const {
        double pct = getPercentage();
        if (pct >= 0.7) return "#4CAF50";  // green  — safe
        if (pct >= 0.4) return "#FF9800";  // orange — moderate
        return "#F44336";                   // red    — danger
    }

    // Text label for UI
    QString getSafetyLabel() const {
        double pct = getPercentage();
        if (pct >= 0.7) return "✅ Safe";
        if (pct >= 0.4) return "⚠️ Moderate";
        return "❌ Avoid";
    }

    void setValue(T val)        { value = val; }
    void setLabel(QString lbl)  { label = lbl; }

private:
    T       value;
    T       maxValue;
    QString label;
};

#endif // SCORE_H
