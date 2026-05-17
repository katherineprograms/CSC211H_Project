#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QHeaderView>
#include <QSet>
#include <QPainter>
#include <algorithm>

static const QString TERRA       = "#c4785a";
static const QString TERRA_DARK  = "#9c5a3e";
static const QString TERRA_LIGHT = "#fae8e0";
static const QString SAFE_COL    = "#4CAF50";
static const QString MOD_COL     = "#FF9800";
static const QString DANGER_COL  = "#F44336";
static const QString BG_COL      = "#faf7f4";
static const QString CARD_COL    = "#ffffff";

// ─────────────────────────────────────────
// btnStyle helper
// ─────────────────────────────────────────
QString MainWindow::btnStyle(QString bg, QString hover) {
    return QString(
               "QPushButton {"
               "  background-color: %1; color: white;"
               "  padding: 7px 14px; border-radius: 6px;"
               "  font-weight: bold; font-size: 12px; border: none; }"
               "QPushButton:hover { background-color: %2; }"
               "QPushButton:pressed { background-color: %2; }"
               ).arg(bg, hover);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Cosmetic Ingredient Analyzer CSC 211H");
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
    centralWidget->setStyleSheet("background-color: " + BG_COL + ";");

    QVBoxLayout *master = new QVBoxLayout(centralWidget);
    master->setSpacing(10);
    master->setContentsMargins(14, 14, 14, 10);

    // ── Header ──
    QFrame *headerFrame = new QFrame(this);
    headerFrame->setStyleSheet(
        "QFrame { background: qlineargradient("
        "x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #c4785a, stop:1 #9c5a3e);"
        "border-radius: 10px; }");
    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setContentsMargins(16, 10, 16, 10);
    headerLayout->setSpacing(2);

    headerLabel = new QLabel("SkinFirst: Cosmetic Ingredient Analyzer", this);
    headerLabel->setAlignment(Qt::AlignCenter);
    headerLabel->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: white;"
        "background: transparent;");

    subtitleLabel = new QLabel(
        "Powered by the California Safe Cosmetics Program Database", this);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(
        "font-size: 11px; color: rgba(255,255,255,0.8);"
        "background: transparent;");

    headerLayout->addWidget(headerLabel);
    headerLayout->addWidget(subtitleLabel);
    master->addWidget(headerFrame);

    // ── Top row ──
    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(10);

    // Skin Profile card
    QGroupBox *profileBox = new QGroupBox("  Skin Profile", this);
    profileBox->setStyleSheet(
        "QGroupBox { font-weight: bold; font-size: 12px;"
        "border: 1.5px solid #c4785a; border-radius: 8px;"
        "margin-top: 8px; padding: 10px;"
        "background-color: #ffffff; }"
        "QGroupBox::title { color: #c4785a; subcontrol-origin: margin;"
        "left: 10px; padding: 0 4px; }");
    QVBoxLayout *profileLayout = new QVBoxLayout(profileBox);
    profileLayout->setSpacing(8);

    // Skin type dropdown
    QHBoxLayout *skinRow = new QHBoxLayout();
    QLabel *skinLbl = new QLabel("Skin Type:", this);
    skinLbl->setStyleSheet("font-weight: bold; font-size: 11px;");
    skinTypeDropdown = new QComboBox(this);
    skinTypeDropdown->addItems({"All","Oily","Dry","Sensitive","Combination"});
    skinTypeDropdown->setFixedWidth(130);
    skinTypeDropdown->setStyleSheet(
        "QComboBox { border: 1px solid #c4785a; border-radius: 4px;"
        "padding: 4px 8px; font-size: 11px; background: white; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox:hover { border-color: #9c5a3e; }");
    skinRow->addWidget(skinLbl);
    skinRow->addWidget(skinTypeDropdown);
    skinRow->addStretch();
    profileLayout->addLayout(skinRow);

    // Slider helper
    auto makeSlider = [&](const QString& lbl, QSlider*& slider, QLabel*& valLbl) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *l = new QLabel(lbl, this);
        l->setFixedWidth(72);
        l->setStyleSheet("font-size: 11px;");
        slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, 10);
        slider->setValue(5);
        slider->setFixedWidth(150);
        slider->setStyleSheet(
            "QSlider::groove:horizontal { height: 6px;"
            "background: #e0d0c8; border-radius: 3px; }"
            "QSlider::handle:horizontal { width: 16px; height: 16px;"
            "background: #c4785a; border-radius: 8px; margin: -5px 0; }"
            "QSlider::sub-page:horizontal { background: #c4785a;"
            "border-radius: 3px; }");
        valLbl = new QLabel("5", this);
        valLbl->setFixedWidth(20);
        valLbl->setStyleSheet(
            "font-weight: bold; font-size: 11px; color: #c4785a;");
        row->addWidget(l);
        row->addWidget(slider);
        row->addWidget(valLbl);
        profileLayout->addLayout(row);
    };

    makeSlider("Acne:",     acneSlider,     acneLabel);
    makeSlider("Dryness:",  drynessSlider,  drynessLabel);
    makeSlider("Hyperpig:", hyperpigSlider, hyperpigLabel);
    topRow->addWidget(profileBox, 45);

    // Search card
    QGroupBox *searchGroup = new QGroupBox("  Search & Actions", this);
    searchGroup->setStyleSheet(
        "QGroupBox { font-weight: bold; font-size: 12px;"
        "border: 1.5px solid #c4785a; border-radius: 8px;"
        "margin-top: 8px; padding: 10px;"
        "background-color: #ffffff; }"
        "QGroupBox::title { color: #c4785a; subcontrol-origin: margin;"
        "left: 10px; padding: 0 4px; }");
    QVBoxLayout *searchLayout = new QVBoxLayout(searchGroup);
    searchLayout->setSpacing(8);

    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("🔍  Search by ingredient, product, or brand...");
    searchBox->setStyleSheet(
        "QLineEdit { border: 1.5px solid #c4785a; border-radius: 6px;"
        "padding: 7px 12px; font-size: 12px; background: white; }"
        "QLineEdit:focus { border-color: #9c5a3e; }");
    searchLayout->addWidget(searchBox);

    // Primary buttons
    QHBoxLayout *btnRow1 = new QHBoxLayout();
    btnRow1->setSpacing(8);
    searchButton  = new QPushButton("🔍  Search",  this);
    analyzeButton = new QPushButton("⚗️  Analyze", this);
    searchButton->setStyleSheet(btnStyle("#9c6644","#7a4e33"));
    analyzeButton->setStyleSheet(btnStyle("#c4785a","#9c5a3e"));
    btnRow1->addWidget(searchButton);
    btnRow1->addWidget(analyzeButton);
    searchLayout->addLayout(btnRow1);

    // Secondary buttons
    QHBoxLayout *btnRow2 = new QHBoxLayout();
    btnRow2->setSpacing(8);
    resetButton  = new QPushButton("↺  Reset",  this);
    exportButton = new QPushButton("💾  Export", this);
    aboutButton  = new QPushButton("ℹ  About",  this);
    resetButton->setStyleSheet(btnStyle("#7a7a7a","#555555"));
    exportButton->setStyleSheet(btnStyle("#3b6d11","#2a5008"));
    aboutButton->setStyleSheet(btnStyle("#185fa5","#0d4580"));
    scoringInfoButton = new QPushButton("📊  How Scoring Works", this);
    scoringInfoButton->setStyleSheet(btnStyle("#7b4ea6","#5c3780"));

    btnRow2->addWidget(resetButton);
    btnRow2->addWidget(exportButton);
    btnRow2->addWidget(aboutButton);
    btnRow2->addWidget(scoringInfoButton);
    searchLayout->addLayout(btnRow2);
    searchLayout->addStretch();
    topRow->addWidget(searchGroup, 55);
    master->addLayout(topRow);

    // ── Category filter bar ──
    QFrame *filterFrame = new QFrame(this);
    filterFrame->setStyleSheet(
        "QFrame { background-color: white; border-radius: 8px;"
        "border: 1px solid #e0d0c8; }");
    QHBoxLayout *filterRow = new QHBoxLayout(filterFrame);
    filterRow->setContentsMargins(10, 6, 10, 6);
    filterRow->setSpacing(6);

    QLabel *filterLbl = new QLabel("Filter:", this);
    filterLbl->setStyleSheet(
        "font-weight: bold; font-size: 11px; color: #666;");
    filterRow->addWidget(filterLbl);

    auto makeFilter = [&](const QString& lbl, QPushButton*& btn) {
        btn = new QPushButton(lbl, this);
        btn->setCheckable(true);
        btn->setStyleSheet(
            "QPushButton { background: #f5ede8; color: #9c5a3e;"
            "border: 1px solid #c4785a; border-radius: 12px;"
            "padding: 4px 12px; font-size: 11px; font-weight: bold; }"
            "QPushButton:checked { background: #c4785a; color: white; }"
            "QPushButton:hover { background: #e8d0c4; }");
        filterRow->addWidget(btn);
    };

    makeFilter("All",  filterAll);
    makeFilter("Lip",  filterLip);
    makeFilter("Hair", filterHair);
    makeFilter("Nail", filterNail);
    makeFilter("Eye",  filterEye);
    makeFilter("Skin", filterSkin);
    filterAll->setChecked(true);
    filterRow->addStretch();

    resultCountLabel = new QLabel("", this);
    resultCountLabel->setStyleSheet(
        "color: #888; font-size: 11px; font-style: italic;");
    filterRow->addWidget(resultCountLabel);
    master->addWidget(filterFrame);

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
    resultsTable->setShowGrid(false);
    resultsTable->verticalHeader()->setDefaultSectionSize(32);
    resultsTable->setStyleSheet(
        "QTableWidget { border: 1px solid #e0d0c8; border-radius: 8px;"
        "background: white; font-size: 12px; }"
        "QTableWidget::item { padding: 6px 8px; }"
        "QTableWidget::item:selected { background-color: #fae8e0; color: #3b1a08; }"
        "QHeaderView::section { background-color: #c4785a;"
        "color: white; font-weight: bold; padding: 8px;"
        "border: none; font-size: 12px; }");
    master->addWidget(resultsTable);

    // ── Bar chart ──
    // ── View Chart button (replaces inline chart) ──
    chartView = new QChartView(this);
    chartView->hide(); // hidden — only shown in popup

    QPushButton *viewChartBtn = new QPushButton(
        "⚠️  View Ingredients to Avoid", this);
    viewChartBtn->setStyleSheet(
        "QPushButton { background-color: #534ab7; color: white;"
        "padding: 10px; border-radius: 6px; font-weight: bold;"
        "font-size: 13px; border: none; }"
        "QPushButton:hover { background-color: #3d368a; }"
        "QPushButton:disabled { background-color: #aaaaaa; }");
    viewChartBtn->setEnabled(false); // enabled after Analyze runs
    viewChartBtn->setObjectName("viewChartBtn");
    master->addWidget(viewChartBtn);

    connect(viewChartBtn, &QPushButton::clicked,
            this, &MainWindow::onViewChartClicked);

    // ── Suggestions ──
    QGroupBox *suggestBox = new QGroupBox("  Formulation Suggestions", this);
    suggestBox->setStyleSheet(
        "QGroupBox { font-weight: bold; font-size: 12px;"
        "border: 1.5px solid #c4785a; border-radius: 8px;"
        "margin-top: 8px; padding: 8px; background-color: #ffffff; }"
        "QGroupBox::title { color: #c4785a; subcontrol-origin: margin;"
        "left: 10px; padding: 0 4px; }");
    QHBoxLayout *suggestLayout = new QHBoxLayout(suggestBox);
    suggestLayout->setSpacing(10);

    auto makeSuggCard = [&](QLabel*& lbl, QString borderCol) {
        QFrame *card = new QFrame(this);
        card->setStyleSheet(QString(
                                "QFrame { background: white; border-radius: 6px;"
                                "border-left: 4px solid %1; }").arg(borderCol));
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(8, 6, 8, 6);
        lbl = new QLabel("Run '⚗️ Analyze' to see suggestions.", this);
        lbl->setWordWrap(true);
        lbl->setStyleSheet(
            "font-size: 11px; border: none; background: transparent;");
        cl->addWidget(lbl);
        suggestLayout->addWidget(card, 1);
    };

    makeSuggCard(suggestion1, "#4CAF50");
    makeSuggCard(suggestion2, "#FF9800");
    makeSuggCard(suggestion3, "#F44336");
    master->addWidget(suggestBox);

    // ── Status bar ──
    QFrame *statusFrame = new QFrame(this);
    statusFrame->setStyleSheet(
        "QFrame { background: white; border-radius: 6px;"
        "border: 1px solid #e0d0c8; }");
    QHBoxLayout *statusRow = new QHBoxLayout(statusFrame);
    statusRow->setContentsMargins(10, 4, 10, 4);
    statusLabel = new QLabel("Loading...", this);
    statusLabel->setStyleSheet(
        "color: #666; font-size: 11px; background: transparent; border: none;");
    statusRow->addWidget(statusLabel);
    master->addWidget(statusFrame);

    // ── Connect signals ──
    connect(searchButton,  &QPushButton::clicked,
            this, &MainWindow::onSearchClicked);
    connect(searchBox, &QLineEdit::returnPressed,
            this, &MainWindow::onSearchClicked);
    connect(analyzeButton, &QPushButton::clicked,
            this, &MainWindow::onAnalyzeClicked);
    connect(resetButton,   &QPushButton::clicked,
            this, &MainWindow::onResetClicked);
    connect(exportButton,  &QPushButton::clicked,
            this, &MainWindow::onExportClicked);
    connect(aboutButton,   &QPushButton::clicked,
            this, &MainWindow::onAboutClicked);
    connect(skinTypeDropdown, &QComboBox::currentTextChanged,
            this, &MainWindow::onSkinTypeChanged);
    connect(resultsTable, &QTableWidget::cellClicked,
            this, &MainWindow::onRowClicked);

    connect(filterAll,  &QPushButton::clicked,
            [this](){ onCategoryFilter("All");  setActiveFilter(filterAll);  });
    connect(filterLip,  &QPushButton::clicked,
            [this](){ onCategoryFilter("Lip");  setActiveFilter(filterLip);  });
    connect(filterHair, &QPushButton::clicked,
            [this](){ onCategoryFilter("Hair"); setActiveFilter(filterHair); });
    connect(filterNail, &QPushButton::clicked,
            [this](){ onCategoryFilter("Nail"); setActiveFilter(filterNail); });
    connect(filterEye,  &QPushButton::clicked,
            [this](){ onCategoryFilter("Eye");  setActiveFilter(filterEye);  });
    connect(filterSkin, &QPushButton::clicked,
            [this](){ onCategoryFilter("Skin"); setActiveFilter(filterSkin); });

    connect(acneSlider, &QSlider::valueChanged,
            [this](int v){ acneLabel->setText(QString::number(v)); });
    connect(drynessSlider, &QSlider::valueChanged,
            [this](int v){ drynessLabel->setText(QString::number(v)); });
    connect(hyperpigSlider, &QSlider::valueChanged,
            [this](int v){ hyperpigLabel->setText(QString::number(v)); });
    connect(scoringInfoButton, &QPushButton::clicked,
            this, &MainWindow::onScoringInfoClicked);
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
    analysisRun = true;

    populateScoredTable(lastScored);
    updateSuggestions(lastScored);
    updateChart(lastScored);

    // Enable the chart button now that we have data
    if (auto* btn = findChild<QPushButton*>("viewChartBtn"))
        btn->setEnabled(true);
    statusLabel->setText("✅ Analysis complete  " +
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
                         "   click ⚗️ Analyze to score ingredients.");
}
// ─────────────────────────────────────────
// updateChart ~ top 10 ingredients bar chart
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
// ─────────────────────────────────────────
// onResetClicked
// ─────────────────────────────────────────
void MainWindow::onResetClicked() {
    searchBox->clear();
    skinTypeDropdown->setCurrentIndex(0);
    acneSlider->setValue(5);
    drynessSlider->setValue(5);
    hyperpigSlider->setValue(5);
    setActiveFilter(filterAll);
    analysisRun = false;
    populateTable(parser.getRecords().mid(0, 100));
    suggestion1->setText("Run '⚗️ Analyze' to see suggestions.");
    suggestion2->setText("Run '⚗️ Analyze' to see suggestions.");
    suggestion3->setText("Run '⚗️ Analyze' to see suggestions.");
    resultCountLabel->setText("Showing 100 of " +
                              QString::number(parser.getRecordCount()) + " records");
    statusLabel->setText("↺  Reset ~ showing first 100 records.");
}

// ─────────────────────────────────────────
// onExportClicked
// ─────────────────────────────────────────
void MainWindow::onExportClicked() {
    if (lastScored.isEmpty()) {
        QMessageBox::information(this, "Export",
                                 "Run Analyze first to generate results to export.");
        return;
    }
    QString path = QFileDialog::getSaveFileName(
        this, "Export Results", "cosmetic_analysis.csv",
        "CSV Files (*.csv)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file);
        out << "Product,Brand,Chemical,Category,SafetyScore,OverallScore,Recommendation\n";
        for (const auto& s : lastScored) {
            out << "\"" << s.record.productName  << "\","
                << "\"" << s.record.brandName    << "\","
                << "\"" << s.record.chemicalName << "\","
                << "\"" << s.record.subCategory  << "\","
                << s.safetyScore.getValue()       << ","
                << s.overallScore.getValue()      << ","
                << "\"" << s.recommendation      << "\"\n";
        }
        file.close();
        statusLabel->setText("💾  Results exported to: " + path);
    }
}

// ─────────────────────────────────────────
// onAboutClicked
// ─────────────────────────────────────────
void MainWindow::onAboutClicked() {
    QMessageBox about(this);
    about.setWindowTitle("About — SkinFirst");
    about.setTextFormat(Qt::RichText);
    about.setText(
        "<h2 style='color:#c4785a; margin-bottom:4px;'>"
        "  SkinFirst: Cosmetic Ingredient Analyzer</h2>"

        "<p style='color:#888; font-size:11px; margin-top:0;'>"
        "CSC 211H Honors Project &nbsp;·&nbsp; "
         "Katherine Sanchez</p>"

        "<hr style='border:0.5px solid #e0d0c8; margin:8px 0;'/>"

        "<p style='font-size:12px; line-height:1.6;'>"
        "Every day, millions of people apply cosmetic products "
        "without knowing what is actually in them. Ingredient lists "
        "are long, technical, and nearly impossible to understand. "
        "The communities most affected are often the ones with "
        "the least access to safety information."
        "</p>"

        "<p style='font-size:12px; line-height:1.6;'>"
        "This app was built for <b>them</b>. For the person reading "
        "the back of a lipstick at the dollar store. For the family "
        "buying affordable hair products without knowing they contain "
        "flagged carcinogens. For anyone who deserves to know what "
        "they are putting on their body."
        "</p>"

        "<hr style='border:0.5px solid #e0d0c8; margin:8px 0;'/>"

        "<p style='font-size:12px; line-height:1.6;'>"
        "<b>What this app does:</b><br>"
        "Loads <b>114,635 real product records</b> from the "
        "California Safe Cosmetics Program, the only US government "
        "registry that requires companies to report harmful cosmetic "
        "chemicals. Every ingredient is scored for safety and "
        "skin-type effectiveness so you can make informed decisions "
        "about the products you use every day."
        "</p>"

        "<p style='font-size:12px; line-height:1.6;'>"
        "<b style='color:#4CAF50;'>✅ Safe ingredients</b> are "
        "highlighted in green. "
        "<b style='color:#FF9800;'>⚠️ Moderate risk</b> ingredients "
        "are shown in orange. "
        "<b style='color:#F44336;'>❌ High risk</b> chemicals   "
        "(like formaldehyde, lead, and coal tar) are flagged in red "
        "so you know exactly what to avoid."
        "</p>"

        "<hr style='border:0.5px solid #e0d0c8; margin:8px 0;'/>"

        "<p style='font-size:11px; color:#888; line-height:1.6;'>"
        "Built with <b>C++ / Qt 6</b> &nbsp;·&nbsp; CMake "
        "&nbsp;·&nbsp; Qt Charts<br>"
        "Data: California Department of Public Health<br>"
        "data.ca.gov/dataset/chemicals-in-cosmetics"
        "</p>"
        );
    about.setStyleSheet(
        "QMessageBox { background: white; min-width: 460px; }"
        "QPushButton { background: #c4785a; color: white;"
        "padding: 8px 24px; border-radius: 6px;"
        "font-weight: bold; font-size: 12px; border: none; }"
        "QPushButton:hover { background: #9c5a3e; }");
    about.exec();
}

// ─────────────────────────────────────────
// onRowClicked — ingredient detail popup
// ─────────────────────────────────────────
void MainWindow::onRowClicked(int row, int col) {
    Q_UNUSED(col);
    if (!analysisRun || row >= lastScored.size()) return;

    const ScoredIngredient& s = lastScored[row];
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Ingredient Detail");
    dlg->setMinimumWidth(380);
    dlg->setStyleSheet("QDialog { background: white; }");

    QVBoxLayout *layout = new QVBoxLayout(dlg);
    layout->setSpacing(8);

    QLabel *title = new QLabel(s.record.chemicalName, dlg);
    title->setStyleSheet(
        "font-size: 15px; font-weight: bold; color: white;"
        "background: #c4785a; padding: 10px; border-radius: 6px;");
    title->setWordWrap(true);
    layout->addWidget(title);

    auto addRow = [&](QString lbl, QString val) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *l = new QLabel("<b>" + lbl + "</b>", dlg);
        l->setFixedWidth(140);
        l->setStyleSheet("font-size: 11px;");
        QLabel *v = new QLabel(val, dlg);
        v->setWordWrap(true);
        v->setStyleSheet("font-size: 11px; color: #333;");
        row->addWidget(l);
        row->addWidget(v);
        layout->addLayout(row);
    };

    addRow("Product:",       s.record.productName);
    addRow("Brand:",         s.record.brandName);
    addRow("Category:",      s.record.subCategory);
    addRow("CAS Number:",    s.record.casNumber);
    addRow("Safety Score:",  QString::number(s.safetyScore.getValue()) + " / 10");
    addRow("Overall Score:", QString::number(s.overallScore.getValue()) + " / 10");
    addRow("Discontinued:",  s.record.isDiscontinued ? "❌ Yes" : "✅ No");
    addRow("Recommendation:",s.recommendation);

    QPushButton *closeBtn = new QPushButton("Close", dlg);
    closeBtn->setStyleSheet(btnStyle("#c4785a","#9c5a3e"));
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);
    layout->addWidget(closeBtn);

    dlg->exec();
}

// ─────────────────────────────────────────
// onCategoryFilter
// ─────────────────────────────────────────
void MainWindow::onCategoryFilter(const QString& category) {
    if (category == "All") {
        if (analysisRun) populateScoredTable(lastScored);
        else populateTable(parser.getRecords().mid(0, 100));
        return;
    }
    if (analysisRun) {
        QVector<ScoredIngredient> filtered;
        for (const auto& s : lastScored)
            if (s.record.subCategory.contains(category, Qt::CaseInsensitive) ||
                s.record.primaryCategory.contains(category, Qt::CaseInsensitive))
                filtered.append(s);
        populateScoredTable(filtered);
        resultCountLabel->setText(
            QString::number(filtered.size()) + " results");
    } else {
        QVector<IngredientRecord> filtered;
        for (const auto& r : parser.getRecords())
            if (r.subCategory.contains(category, Qt::CaseInsensitive) ||
                r.primaryCategory.contains(category, Qt::CaseInsensitive))
                filtered.append(r);
        populateTable(filtered);
        resultCountLabel->setText(
            QString::number(filtered.size()) + " results");
    }
}

// ─────────────────────────────────────────
// setActiveFilter
// ─────────────────────────────────────────
void MainWindow::setActiveFilter(QPushButton* active) {
    for (auto* btn : {filterAll, filterLip, filterHair,
                      filterNail, filterEye, filterSkin})
        btn->setChecked(btn == active);
}
// ─────────────────────────────────────────
// onViewChartClicked — opens chart in popup
// ─────────────────────────────────────────
void MainWindow::onViewChartClicked() {
    if (lastScored.isEmpty()) {
        statusLabel->setText("Run Analyze first to generate chart.");
        return;
    }

    // ── Take the 10 MOST DANGEROUS ingredients (lowest scores) ──
    QVector<ScoredIngredient> dangerous;

    // Sort a copy by score ascending (worst first)
    QVector<ScoredIngredient> sorted = lastScored;
    std::sort(sorted.begin(), sorted.end(),
              [](const ScoredIngredient& a, const ScoredIngredient& b) {
                  return a.overallScore.getValue() < b.overallScore.getValue();
              });

    // Take up to 10 lowest scoring
    int count = qMin(10, (int)sorted.size());
    for (int i = 0; i < count; i++)
        dangerous.append(sorted[i]);

    // ── Build chart ──
    QBarSet *dangerSet  = new QBarSet("High Risk (1-3)");
    QBarSet *modSet     = new QBarSet("Moderate Risk (4-6)");

    dangerSet->setColor(QColor("#F44336"));
    dangerSet->setBorderColor(Qt::transparent);
    modSet->setColor(QColor("#FF9800"));
    modSet->setBorderColor(Qt::transparent);

    QStringList categories;

    for (int i = 0; i < count; i++) {
        const ScoredIngredient& s = dangerous[i];
        int val = s.overallScore.getValue();
        QString name = s.record.chemicalName;
        if (name.length() > 18) name = name.left(18) + "...";
        categories << name;

        if (val <= 3) {
            *dangerSet << val;
            *modSet    << 0;
        } else {
            *dangerSet << 0;
            *modSet    << val;
        }
    }

    QBarSeries *series = new QBarSeries();
    series->append(dangerSet);
    series->append(modSet);
    series->setBarWidth(0.6);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Ingredients to Avoid — " +
                    skinTypeDropdown->currentText() + " Skin");
    chart->setTitleFont(QFont("Arial", 14, QFont::Bold));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundBrush(QBrush(Qt::white));
    chart->setMargins(QMargins(20, 20, 20, 20));

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsAngle(-35);
    axisX->setLabelsFont(QFont("Arial", 10));
    axisX->setTitleText("Ingredient");
    axisX->setTitleFont(QFont("Arial", 11, QFont::Bold));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 11);
    axisY->setTickCount(6);
    axisY->setTitleText("Risk Score (lower = more dangerous)");
    axisY->setTitleFont(QFont("Arial", 11, QFont::Bold));
    axisY->setLabelsFont(QFont("Arial", 10));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setFont(QFont("Arial", 11));

    // Count danger vs moderate
    int danger = 0, mod = 0, safe = 0;
    for (const auto& s : lastScored) {
        int v = s.overallScore.getValue();
        if      (v <= 3) danger++;
        else if (v <= 6) mod++;
        else             safe++;
    }

    // ── Popup ──
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Ingredients to Avoid");
    dlg->resize(920, 640);
    dlg->setStyleSheet("QDialog { background: white; }");

    QVBoxLayout *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(16, 16, 16, 12);
    layout->setSpacing(10);

    // Warning header
    QLabel *titleLbl = new QLabel(
        "⚠️  Ingredients to Avoid  " +
            skinTypeDropdown->currentText() + " Skin", dlg);
    titleLbl->setAlignment(Qt::AlignCenter);
    titleLbl->setStyleSheet(
        "font-size: 15px; font-weight: bold; color: white;"
        "background: #b41e1e; padding: 12px; border-radius: 8px;");
    layout->addWidget(titleLbl);

    // Subtitle
    QLabel *subLbl = new QLabel(
        QString("Based on CSCP data  |  "
                "❌ High Risk: %1 ingredients  "
                "⚠️ Moderate: %2 ingredients  "
                "✅ Safer: %3 ingredients")
            .arg(danger).arg(mod).arg(safe), dlg);
    subLbl->setAlignment(Qt::AlignCenter);
    subLbl->setStyleSheet("font-size: 11px; color: #555;");
    layout->addWidget(subLbl);

    // Warning note
    QLabel *noteLbl = new QLabel(
        "🔬  These are the most harmful ingredients found in cosmetic "
        "products reported to the California Safe Cosmetics Program. "
        "Lower bars = higher danger. Look for these on product labels "
        "and consider avoiding them.", dlg);
    noteLbl->setWordWrap(true);
    noteLbl->setStyleSheet(
        "font-size: 11px; color: #444; background: #fff8e1;"
        "border-left: 4px solid #FF9800; border-radius: 4px;"
        "padding: 8px 12px;");
    layout->addWidget(noteLbl);

    // Chart
    QChartView *popupChart = new QChartView(chart, dlg);
    popupChart->setRenderHint(QPainter::Antialiasing);
    popupChart->setMinimumHeight(380);
    popupChart->setStyleSheet(
        "border: 1px solid #e0d0c8; border-radius: 8px;");
    layout->addWidget(popupChart);

    // Close button
    QPushButton *closeBtn = new QPushButton("Close", dlg);
    closeBtn->setStyleSheet(
        "QPushButton { background: #b41e1e; color: white;"
        "padding: 8px 24px; border-radius: 6px;"
        "font-weight: bold; font-size: 12px; border: none; }"
        "QPushButton:hover { background: #8b0000; }");
    closeBtn->setFixedWidth(120);
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);
    dlg->exec();
}
// ─────────────────────────────────────────
// onScoringInfoClicked — scoring explanation
// ─────────────────────────────────────────
void MainWindow::onScoringInfoClicked() {
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("How Scoring Works");
    dlg->setMinimumWidth(520);
    dlg->setStyleSheet("QDialog { background: white; }");

    QVBoxLayout *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(20, 20, 20, 16);
    layout->setSpacing(12);

    // Title
    QLabel *title = new QLabel("📊  How Ingredient Scoring Works", dlg);
    title->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: white;"
        "background: #c4785a; padding: 12px; border-radius: 8px;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // Intro
    QLabel *intro = new QLabel(
        "Every ingredient is given a score from 1 to 10 based on "
        "two things: how safe it is, and how well it suits your skin type.", dlg);
    intro->setWordWrap(true);
    intro->setStyleSheet("font-size: 12px; color: #444; padding: 4px 0;");
    layout->addWidget(intro);

    // Formula box
    QFrame *formulaBox = new QFrame(dlg);
    formulaBox->setStyleSheet(
        "QFrame { background: #fae8e0; border-radius: 8px;"
        "border-left: 4px solid #c4785a; padding: 4px; }");
    QVBoxLayout *fl = new QVBoxLayout(formulaBox);
    fl->setContentsMargins(12, 10, 12, 10);
    QLabel *formulaTitle = new QLabel("The Formula", dlg);
    formulaTitle->setStyleSheet(
        "font-weight: bold; font-size: 12px; color: #c4785a;"
        "background: transparent; border: none;");
    QLabel *formula = new QLabel(
        "Overall Score  =  (Safety Score × 60%)  +  (Effectiveness Score × 40%)", dlg);
    formula->setStyleSheet(
        "font-size: 12px; font-weight: bold; color: #333;"
        "background: transparent; border: none;");
    formula->setWordWrap(true);
    fl->addWidget(formulaTitle);
    fl->addWidget(formula);
    layout->addWidget(formulaBox);

    // Safety section
    auto makeSection = [&](QString emoji, QString heading,
                           QString color, QString border,
                           QStringList points) {
        QFrame *box = new QFrame(dlg);
        box->setStyleSheet(QString(
                               "QFrame { background: %1; border-radius: 8px;"
                               "border-left: 4px solid %2; }").arg(color, border));
        QVBoxLayout *bl = new QVBoxLayout(box);
        bl->setContentsMargins(12, 10, 12, 10);
        bl->setSpacing(4);
        QLabel *h = new QLabel(emoji + "  " + heading, dlg);
        h->setStyleSheet(QString(
                             "font-weight: bold; font-size: 12px; color: %1;"
                             "background: transparent; border: none;").arg(border));
        bl->addWidget(h);
        for (const QString& pt : points) {
            QLabel *p = new QLabel("• " + pt, dlg);
            p->setWordWrap(true);
            p->setStyleSheet(
                "font-size: 11px; color: #444;"
                "background: transparent; border: none;");
            bl->addWidget(p);
        }
        layout->addWidget(box);
    };

    makeSection("🛡️", "Safety Score (60% of total)",
                "#fae8e0", "#c4785a", {
                    "Starts at 7 out of 10 ~ most ingredients assumed safe",
                    "Severe chemicals (formaldehyde, lead, mercury, coal tar) → score drops to 1",
                    "CSCP flagged allergens → lose 3 points",
                    "Mild irritants (parabens, sulfates, fragrance) → lose 1 point",
                    "Known safe ingredients (glycerin, ceramide, niacinamide) → gain 2 points",
                    "Discontinued products → lose 2 points"
                });

    makeSection("✨", "Effectiveness Score (40% of total)",
                "#eaf3de", "#3b6d11", {
                    "Starts at 5 out of 10 ~ neutral for all skin types",
                    "Ingredients known to benefit your skin type → gain 3 points",
                    "Ingredients known to irritate your skin type → lose 3 points",
                    "Slider boosts: high Acne level boosts salicylic acid and niacinamide",
                    "High Dryness level boosts hyaluronic acid and ceramide",
                    "High Hyperpigmentation level boosts niacinamide and vitamin C"
                });

    makeSection("🎨", "Score Colors in the Table",
                "#eeedfe", "#534ab7", {
                    "🟢 GREEN  (7-10) Safe and recommended for your profile",
                    "🟠 ORANGE (4-6)  Moderate risk, patch test recommended",
                    "🔴 RED    (1-3)  High risk, avoid if possible"
                });

    // Close button
    QPushButton *closeBtn = new QPushButton("Got it!", dlg);
    closeBtn->setStyleSheet(
        "QPushButton { background: #c4785a; color: white;"
        "padding: 8px 28px; border-radius: 6px;"
        "font-weight: bold; font-size: 12px; border: none; }"
        "QPushButton:hover { background: #9c5a3e; }");
    closeBtn->setFixedWidth(120);
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);
    dlg->exec();
}