#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QHeaderView>
#include <QSet>
#include <QPainter>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Cosmetic Ingredient Analyzer — CSC 211H");
    resize(1100, 800);

    setupUI();

    if (parser.loadFile("/Users/katherinesanchez/Documents/CSC211H/"
                        "Honors Project CSC 211H/CSC211H_Project/"
                        "data/chemicals_in_cosmetics.csv")) {

        statusLabel->setText("✅ Loaded " +
                             QString::number(parser.getRecordCount()) +
                             " records. Select skin type and click Analyze.");

        for (const auto& record : parser.getRecords())
            ingredients.append(Ingredient::fromRecord(record));

        qDebug() << "Records loaded:"      << parser.getRecordCount();
        qDebug() << "Ingredients converted:" << ingredients.size();

        populateTable(parser.getRecords().mid(0, 100));

    } else {
        statusLabel->setText("❌ Error: " + parser.getLastError());
        qDebug() << "CSV Error:" << parser.getLastError();
    }
}

// ─────────────────────────────────────────
// setupUI
// ─────────────────────────────────────────
void MainWindow::setupUI() {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *master = new QVBoxLayout(centralWidget);
    master->setSpacing(8);
    master->setContentsMargins(12, 12, 12, 12);

    // ── Header ──
    headerLabel = new QLabel("🧴 Cosmetic Ingredient Analyzer", this);
    headerLabel->setAlignment(Qt::AlignCenter);
    headerLabel->setStyleSheet(
        "font-size: 22px; font-weight: bold; padding: 12px;"
        "background-color: #c4785a; color: white; border-radius: 6px;");
    master->addWidget(headerLabel);

    // ── Top row: profile box + search box ──
    QHBoxLayout *topRow = new QHBoxLayout();

    // Skin profile group
    QGroupBox *profileBox = new QGroupBox("Skin Profile", this);
    profileBox->setStyleSheet(
        "QGroupBox { font-weight: bold; border: 2px solid #c4785a;"
        "border-radius: 6px; margin-top: 6px; padding: 8px; }"
        "QGroupBox::title { color: #c4785a; }");
    QVBoxLayout *profileLayout = new QVBoxLayout(profileBox);

    // Skin type row
    QHBoxLayout *skinRow = new QHBoxLayout();
    skinRow->addWidget(new QLabel("Skin Type:"));
    skinTypeDropdown = new QComboBox(this);
    skinTypeDropdown->addItems({"All","Oily","Dry","Sensitive","Combination"});
    skinTypeDropdown->setFixedWidth(120);
    skinRow->addWidget(skinTypeDropdown);
    skinRow->addStretch();
    profileLayout->addLayout(skinRow);

    // Acne slider
    QHBoxLayout *acneRow = new QHBoxLayout();
    acneRow->addWidget(new QLabel("Acne:      "));
    acneSlider = new QSlider(Qt::Horizontal, this);
    acneSlider->setRange(0, 10);
    acneSlider->setValue(5);
    acneSlider->setFixedWidth(140);
    acneLabel  = new QLabel("5", this);
    acneRow->addWidget(acneSlider);
    acneRow->addWidget(acneLabel);
    profileLayout->addLayout(acneRow);

    // Dryness slider
    QHBoxLayout *dryRow = new QHBoxLayout();
    dryRow->addWidget(new QLabel("Dryness:  "));
    drynessSlider = new QSlider(Qt::Horizontal, this);
    drynessSlider->setRange(0, 10);
    drynessSlider->setValue(5);
    drynessSlider->setFixedWidth(140);
    drynessLabel  = new QLabel("5", this);
    dryRow->addWidget(drynessSlider);
    dryRow->addWidget(drynessLabel);
    profileLayout->addLayout(dryRow);

    // Hyperpigmentation slider
    QHBoxLayout *hypRow = new QHBoxLayout();
    hypRow->addWidget(new QLabel("Hyperpig: "));
    hyperpigSlider = new QSlider(Qt::Horizontal, this);
    hyperpigSlider->setRange(0, 10);
    hyperpigSlider->setValue(5);
    hyperpigSlider->setFixedWidth(140);
    hyperpigLabel  = new QLabel("5", this);
    hypRow->addWidget(hyperpigSlider);
    hypRow->addWidget(hyperpigLabel);
    profileLayout->addLayout(hypRow);

    topRow->addWidget(profileBox);

    // Search group
    QGroupBox *searchGroup = new QGroupBox("Search", this);
    searchGroup->setStyleSheet(
        "QGroupBox { font-weight: bold; border: 2px solid #c4785a;"
        "border-radius: 6px; margin-top: 6px; padding: 8px; }"
        "QGroupBox::title { color: #c4785a; }");
    QVBoxLayout *searchLayout = new QVBoxLayout(searchGroup);

    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Search ingredient or product...");
    searchLayout->addWidget(searchBox);

    QHBoxLayout *btnRow = new QHBoxLayout();
    searchButton = new QPushButton("🔍 Search", this);
    searchButton->setStyleSheet(
        "background-color: #9c6644; color: white;"
        "padding: 6px; border-radius: 4px; font-weight: bold;");
    analyzeButton = new QPushButton("⚗️ Analyze", this);
    analyzeButton->setStyleSheet(
        "background-color: #c4785a; color: white;"
        "padding: 6px; border-radius: 4px; font-weight: bold;");
    btnRow->addWidget(searchButton);
    btnRow->addWidget(analyzeButton);
    searchLayout->addLayout(btnRow);
    searchLayout->addStretch();
    topRow->addWidget(searchGroup);
    master->addLayout(topRow);

    // ── Results table ──
    resultsTable = new QTableWidget(this);
    resultsTable->setColumnCount(6);
    resultsTable->setHorizontalHeaderLabels({
        "Product","Brand","Chemical","Category","Score","Status"
    });
    resultsTable->horizontalHeader()->setStretchLastSection(true);
    resultsTable->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultsTable->setAlternatingRowColors(true);
    resultsTable->setStyleSheet(
        "QHeaderView::section { background-color: #c4785a;"
        "color: white; font-weight: bold; padding: 6px; }");
    master->addWidget(resultsTable);

    // ── Bar chart ──
    QChartView *chartView = new QChartView(this);
    chartView->setMinimumHeight(200);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("border: 2px solid #c4785a; border-radius: 6px;");
    master->addWidget(chartView);
    this->chartView = chartView;

    // ── Suggestions box ──
    QGroupBox *suggestBox = new QGroupBox("Formulation Suggestions",this);
    suggestBox->setStyleSheet(
        "QGroupBox { font-weight: bold; border: 2px solid #c4785a;"
        "border-radius: 6px; margin-top: 6px; padding: 6px; }"
        "QGroupBox::title { color: #c4785a; }");
    QVBoxLayout *suggestLayout = new QVBoxLayout(suggestBox);
    suggestion1 = new QLabel("Run ⚗️ Analyze to see suggestions.", this);
    suggestion2 = new QLabel("", this);
    suggestion3 = new QLabel("", this);
    suggestion1->setWordWrap(true);
    suggestion2->setWordWrap(true);
    suggestion3->setWordWrap(true);
    suggestLayout->addWidget(suggestion1);
    suggestLayout->addWidget(suggestion2);
    suggestLayout->addWidget(suggestion3);
    master->addWidget(suggestBox);

    // ── Status label ──
    statusLabel = new QLabel("Loading...", this);
    statusLabel->setStyleSheet("color: gray; font-size: 11px;");
    master->addWidget(statusLabel);

    // ── Connect signals ──
    connect(searchButton,  &QPushButton::clicked,
            this, &MainWindow::onSearchClicked);
    connect(analyzeButton, &QPushButton::clicked,
            this, &MainWindow::onAnalyzeClicked);
    connect(skinTypeDropdown, &QComboBox::currentTextChanged,
            this, &MainWindow::onSkinTypeChanged);

    // Slider live labels
    connect(acneSlider, &QSlider::valueChanged,
            [this](int v){ acneLabel->setText(QString::number(v)); });
    connect(drynessSlider, &QSlider::valueChanged,
            [this](int v){ drynessLabel->setText(QString::number(v)); });
    connect(hyperpigSlider, &QSlider::valueChanged,
            [this](int v){ hyperpigLabel->setText(QString::number(v)); });
}

// ─────────────────────────────────────────
// getCurrentProfile
// ─────────────────────────────────────────
SkinProfile MainWindow::getCurrentProfile() {
    SkinProfile p;
    p.skinType      = skinTypeDropdown->currentText();
    p.acneLevel     = acneSlider->value();
    p.drynessLevel  = drynessSlider->value();
    p.hyperpigLevel = hyperpigSlider->value();
    return p;
}

// ─────────────────────────────────────────
// onAnalyzeClicked
// ─────────────────────────────────────────
void MainWindow::onAnalyzeClicked() {
    SkinProfile profile = getCurrentProfile();
    statusLabel->setText("⚗️ Analyzing for " +
                         profile.skinType + " skin...");

    // Score first 500 records for performance
    QVector<IngredientRecord> subset =
        parser.getRecords().mid(0, 500);
    lastScored = scorer.scoreAll(subset, profile);

    populateScoredTable(lastScored);
    updateSuggestions(lastScored);
    updateChart(lastScored);

    statusLabel->setText("✅ Analysis complete — " +
                         QString::number(lastScored.size()) +
                         " ingredients scored for " + profile.skinType + " skin.");
}

// ─────────────────────────────────────────
// populateTable — raw records, no scores
// ─────────────────────────────────────────
void MainWindow::populateTable(
    const QVector<IngredientRecord>& records)
{
    resultsTable->setRowCount(0);

    for (const auto& r : records) {
        int row = resultsTable->rowCount();
        resultsTable->insertRow(row);
        resultsTable->setItem(row,0,new QTableWidgetItem(r.productName));
        resultsTable->setItem(row,1,new QTableWidgetItem(r.brandName));
        resultsTable->setItem(row,2,new QTableWidgetItem(r.chemicalName));
        resultsTable->setItem(row,3,new QTableWidgetItem(r.subCategory));
        resultsTable->setItem(row,4,new QTableWidgetItem("—"));
        QString status = r.isDiscontinued ? "❌ Discontinued":"✅ Active";
        resultsTable->setItem(row,5,new QTableWidgetItem(status));
    }
    statusLabel->setText("Showing " +
                         QString::number(records.size()) + " results.");
}

// ─────────────────────────────────────────
// populateScoredTable — with color scores
// ─────────────────────────────────────────
void MainWindow::populateScoredTable(
    const QVector<ScoredIngredient>& scored)
{
    resultsTable->setRowCount(0);

    for (const auto& s : scored) {
        int row = resultsTable->rowCount();
        resultsTable->insertRow(row);
        resultsTable->setItem(row,0,
                              new QTableWidgetItem(s.record.productName));
        resultsTable->setItem(row,1,
                              new QTableWidgetItem(s.record.brandName));
        resultsTable->setItem(row,2,
                              new QTableWidgetItem(s.record.chemicalName));
        resultsTable->setItem(row,3,
                              new QTableWidgetItem(s.record.subCategory));

        // Colored score cell
        QString scoreText =
            QString::number(s.overallScore.getValue()) + "/10";
        QTableWidgetItem* scoreItem =
            new QTableWidgetItem(scoreText);
        scoreItem->setBackground(QColor(s.overallScore.getColor()));
        scoreItem->setTextAlignment(Qt::AlignCenter);
        resultsTable->setItem(row, 4, scoreItem);

        resultsTable->setItem(row, 5,
                              new QTableWidgetItem(s.recommendation));
    }
}

// ─────────────────────────────────────────
// updateSuggestions
// ─────────────────────────────────────────
void MainWindow::updateSuggestions(
    const QVector<ScoredIngredient>& scored)
{
    QVector<QString> tips =
        scorer.getFormulationSuggestions(scored);

    if (tips.size() >= 3) {
        suggestion1->setText(tips[0]);
        suggestion2->setText(tips[1]);
        suggestion3->setText(tips[2]);
        suggestion1->setStyleSheet(
            "color: #2e7d32; font-weight: bold;");
        suggestion2->setStyleSheet(
            "color: #e65100; font-weight: bold;");
        suggestion3->setStyleSheet(
            "color: #c62828; font-weight: bold;");
    }
}

// ─────────────────────────────────────────
// onSearchClicked
// ─────────────────────────────────────────
void MainWindow::onSearchClicked() {
    QString query = searchBox->text().trimmed();

    if (query.isEmpty()) {
        populateTable(parser.getRecords().mid(0, 100));
        return;
    }

    QVector<IngredientRecord> results =
        parser.searchByChemical(query);
    QVector<IngredientRecord> byProduct =
        parser.searchByProduct(query);

    QSet<int> seenIds;
    for (const auto& r : results)    seenIds.insert(r.id);
    for (const auto& r : byProduct) {
        if (!seenIds.contains(r.id)) {
            results.append(r);
            seenIds.insert(r.id);
        }
    }
    populateTable(results);
}

// ─────────────────────────────────────────
// onSkinTypeChanged
// ─────────────────────────────────────────
void MainWindow::onSkinTypeChanged() {
    statusLabel->setText("Skin type: " +
                         skinTypeDropdown->currentText() +
                         " — click ⚗️ Analyze to score ingredients.");
}
// ─────────────────────────────────────────
// updateChart — top 10 ingredients bar chart
// ─────────────────────────────────────────
void MainWindow::updateChart(const QVector<ScoredIngredient>& scored) {
    if (scored.isEmpty()) return;

    // Take top 10 results (already sorted highest first)
    int count = qMin(10, (int)scored.size());

    // Three bar sets by safety category
    QBarSet *safeSet     = new QBarSet("Safe (7-10)");
    QBarSet *moderateSet = new QBarSet("Moderate (4-6)");
    QBarSet *dangerSet   = new QBarSet("Danger (1-3)");

    safeSet->setColor(QColor("#4CAF50"));
    moderateSet->setColor(QColor("#FF9800"));
    dangerSet->setColor(QColor("#F44336"));

    QStringList categories;

    for (int i = 0; i < count; i++) {
        const ScoredIngredient& s = scored[i];
        int val = s.overallScore.getValue();

        // Shorten chemical name for axis label
        QString name = s.record.chemicalName;
        if (name.length() > 12)
            name = name.left(12) + "...";
        categories << name;

        // Add score to correct set, 0 to others
        if      (val >= 7) {
            *safeSet     << val;
            *moderateSet << 0;
            *dangerSet   << 0;
        }
        else if (val >= 4) {
            *safeSet     << 0;
            *moderateSet << val;
            *dangerSet   << 0;
        }
        else {
            *safeSet     << 0;
            *moderateSet << 0;
            *dangerSet   << val;
        }
    }

    // Build series
    QBarSeries *series = new QBarSeries();
    series->append(safeSet);
    series->append(moderateSet);
    series->append(dangerSet);

    // Build chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Top 10 Ingredient Safety Scores");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundBrush(QBrush(QColor("#FBF8F4")));

    // X axis — ingredient names
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsAngle(-30);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Y axis — score 0-10
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 10);
    axisY->setTitleText("Score");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    chartView->setChart(chart);
}
// ─────────────────────────────────────────
// Destructor
// ─────────────────────────────────────────
MainWindow::~MainWindow() {
    for (Ingredient* i : ingredients) delete i;
    ingredients.clear();
    delete ui;
}