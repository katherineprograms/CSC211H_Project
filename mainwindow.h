#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include "csvparser.h"
#include "ingredient.h"
#include "SkinScorer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onSearchClicked();
    void onSkinTypeChanged();
    void onAnalyzeClicked();    // runs full scoring pipeline

private:
    Ui::MainWindow *ui;

    // ── Data ──
    CSVParser  parser;
    SkinScorer scorer;
    QVector<Ingredient*>      ingredients;
    QVector<ScoredIngredient> lastScored;

    // ── Profile UI ──
    QWidget     *centralWidget;
    QLabel      *headerLabel;
    QComboBox   *skinTypeDropdown;
    QSlider     *acneSlider;
    QSlider     *drynessSlider;
    QSlider     *hyperpigSlider;
    QLabel      *acneLabel;
    QLabel      *drynessLabel;
    QLabel      *hyperpigLabel;

    // ── Search UI ──
    QLineEdit   *searchBox;
    QPushButton *searchButton;
    QPushButton *analyzeButton;

    // ── Results UI ──
    QTableWidget *resultsTable;
    QLabel       *statusLabel;
    QLabel       *suggestion1;
    QLabel       *suggestion2;
    QLabel       *suggestion3;
    QChartView *chartView;

    // ── Helpers ──
    void setupUI();
    void populateTable(const QVector<IngredientRecord>& records);
    void populateScoredTable(const QVector<ScoredIngredient>& scored);
    void updateSuggestions(const QVector<ScoredIngredient>& scored);
    void updateChart(const QVector<ScoredIngredient>& scored);
    SkinProfile getCurrentProfile();
};

#endif // MAINWINDOW_H