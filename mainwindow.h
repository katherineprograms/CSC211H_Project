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
#include <QDialog>
#include <QScrollArea>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QFrame>
#include "csvparser.h"
#include "ingredient.h"
#include "skinscorer.h"

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
    void onAnalyzeClicked();
    void onResetClicked();
    void onExportClicked();
    void onAboutClicked();
    void onScoringInfoClicked();
    void onRowClicked(int row, int col);
    void onCategoryFilter(const QString& category);
    void onViewChartClicked();

private:
    Ui::MainWindow *ui;

    // ── Data ──
    CSVParser  parser;
    SkinScorer scorer;
    QVector<Ingredient*>      ingredients;
    QVector<ScoredIngredient> lastScored;
    bool analysisRun = false;

    // ── Profile UI ──
    QWidget     *centralWidget;
    QLabel      *headerLabel;
    QLabel      *subtitleLabel;
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
    QPushButton *resetButton;
    QPushButton *exportButton;
    QPushButton *aboutButton;
    QPushButton *scoringInfoButton;

    // ── Filter buttons ──
    QPushButton *filterAll;
    QPushButton *filterLip;
    QPushButton *filterHair;
    QPushButton *filterNail;
    QPushButton *filterEye;
    QPushButton *filterSkin;

    // ── Results UI ──
    QTableWidget *resultsTable;
    QLabel       *statusLabel;
    QLabel       *resultCountLabel;
    QLabel       *suggestion1;
    QLabel       *suggestion2;
    QLabel       *suggestion3;
    QChartView   *chartView;

    // ── Helpers ──
    void setupUI();
    void populateTable(const QVector<IngredientRecord>& records);
    void populateScoredTable(const QVector<ScoredIngredient>& scored);
    void updateSuggestions(const QVector<ScoredIngredient>& scored);
    void updateChart(const QVector<ScoredIngredient>& scored);
    void setActiveFilter(QPushButton* active);
    SkinProfile getCurrentProfile();

    QString btnStyle(QString bg, QString hover);
};

#endif // MAINWINDOW_H