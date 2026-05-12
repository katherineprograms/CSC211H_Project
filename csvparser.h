#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QString>
#include <QVector>
#include <QStringList>

// One row from the CSCP database
struct IngredientRecord {
    int     id;
    QString productName;
    QString companyName;
    QString brandName;
    QString primaryCategory;
    QString subCategory;
    QString casNumber;
    QString chemicalName;
    QString dateReported;
    bool    isDiscontinued;
};

class CSVParser {
public:
    CSVParser();

    bool loadFile(const QString& filePath);

    QVector<IngredientRecord> getRecords() const;
    QVector<IngredientRecord> searchByChemical(const QString& name) const;
    QVector<IngredientRecord> searchByProduct(const QString& name) const;
    QVector<IngredientRecord> searchByBrand(const QString& name) const;
    QVector<IngredientRecord> getDiscontinued() const;

    QString getLastError() const;
    int getRecordCount() const;

private:
    QVector<IngredientRecord> records;
    QString lastError;

    IngredientRecord parseRow(const QStringList& columns);
    QStringList splitCSVLine(const QString& line);
};

#endif // CSVPARSER_H