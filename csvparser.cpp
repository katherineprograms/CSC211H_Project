#include "csvparser.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

CSVParser::CSVParser() {}

bool CSVParser :: loadFile(const QString& filePath) {

    //Clear any previously loaded data
    records.clear();
    lastError.clear();

    QFile file(filePath);

    //Check if file exists and can be opened
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        lastError = "Could not open file: " + filePath +
                    "\nMake sure the CSV is in your data folder.";
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8); //handles the BOM that excel adds

    // Skip the header row (first line)
    if (!in.atEnd()) {
        in.readLine();
    }

    int lineNumber = 1;
    int skipped = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
        lineNumber++;

        // Skip empty lines
        if (line.trimmed().isEmpty()) {
            continue;
        }

        QStringList columns = splitCSVLine(line);

        // Your CSV has 23 columns, skip malformed rows
        if (columns.size() < 19) {
            skipped++;
            qDebug() << "Skipping malformed row at line" << lineNumber
                     << "— only" << columns.size() << "columns found";
            continue;
        }

        try {
            records.append(parseRow(columns));
        } catch (const std::exception& e) {
            lastError = QString("Parse error at line %1: %2")
            .arg(lineNumber)
                .arg(e.what());
            qDebug() << lastError;
        }
    }

    file.close();

    qDebug() << "=== CSV Load Complete ===";
    qDebug() << "Records loaded:" << records.size();
    qDebug() << "Rows skipped:  " << skipped;

    if (records.isEmpty()) {
        lastError = "File opened but no valid records were found.";
        return false;
    }

    return true;
}

// ------------------------------------------------------------
// parseRow — maps each column index to your struct fields
// Column order from your actual CSV:
//  0:_id       1:CDPHId      2:ProductName   3:CSFId
//  4:CSF       5:CompanyId   6:CompanyName   7:BrandName
//  8:PrimaryCategoryId       9:PrimaryCategory
//  10:SubCategoryId          11:SubCategory
//  12:CasId    13:CasNumber  14:ChemicalId   15:ChemicalName
//  16:InitialDateReported    17:MostRecentDateReported
//  18:DiscontinuedDate       19:ChemicalCreatedAt
//  20:ChemicalUpdatedAt      21:ChemicalDateRemoved
//  22:ChemicalCount
// ------------------------------------------------------------
IngredientRecord CSVParser::parseRow(const QStringList& col) {
    IngredientRecord r;

    r.id              = col[0].toInt();
    r.productName     = col[2];
    r.companyName     = col[6];
    r.brandName       = col[7];
    r.primaryCategory = col[9];
    r.subCategory     = col[11];
    r.casNumber       = col[13];  // always QString never convert to int
    r.chemicalName    = col[15];
    r.dateReported    = col[16];
    r.isDiscontinued  = !col[18].trimmed().isEmpty();

    return r;
}

// ------------------------------------------------------------
// splitCSVLine handles quoted fields that contain commas
// e.g. "Lip Color, Lipstick" should be one field, not two
// ------------------------------------------------------------
QStringList CSVParser::splitCSVLine(const QString& line) {
    QStringList columns;
    QString field;
    bool inQuotes = false;

    for (int i = 0; i < line.length(); i++) {
        QChar ch = line[i];

        if (ch == '"') {
            // Handle double quotes inside quoted fields ""
            if (inQuotes && i + 1 < line.length() && line[i+1] == '"') {
                field += '"';
                i++;  // skip the second quote
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == ',' && !inQuotes) {
            columns.append(field.trimmed());
            field.clear();
        } else {
            field += ch;
        }
    }

    // Don't forget the last field
    columns.append(field.trimmed());

    return columns;
}

// ------------------------------------------------------------
// Search functions
// ------------------------------------------------------------

QVector<IngredientRecord> CSVParser::getRecords() const {
    return records;
}

QVector<IngredientRecord> CSVParser::searchByChemical(const QString& name) const {
    QVector<IngredientRecord> results;
    for (const auto& r : records) {
        if (r.chemicalName.contains(name, Qt::CaseInsensitive)) {
            results.append(r);
        }
    }
    return results;
}

QVector<IngredientRecord> CSVParser::searchByProduct(const QString& name) const {
    QVector<IngredientRecord> results;
    for (const auto& r : records) {
        if (r.productName.contains(name, Qt::CaseInsensitive)) {
            results.append(r);
        }
    }
    return results;
}

QVector<IngredientRecord> CSVParser::searchByBrand(const QString& name) const {
    QVector<IngredientRecord> results;
    for (const auto& r : records) {
        if (r.brandName.contains(name, Qt::CaseInsensitive)) {
            results.append(r);
        }
    }
    return results;
}

QVector<IngredientRecord> CSVParser::getDiscontinued() const {
    QVector<IngredientRecord> results;
    for (const auto& r : records) {
        if (r.isDiscontinued) {
            results.append(r);
        }
    }
    return results;
}

QString CSVParser::getLastError() const {
    return lastError;
}

int CSVParser::getRecordCount() const {
    return records.size();
}

