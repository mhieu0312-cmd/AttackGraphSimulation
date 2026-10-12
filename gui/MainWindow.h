#pragma once

#include "Graph.h"
#include "Dijkstra.h"
#include "MinCut.h"

#include <QMainWindow>
#include <QString>
#include <cstdint>
#include <optional>
#include <vector>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class QWidget;
class GraphView;
class QListWidget;
class QTableWidget;
class QTabWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    bool loadDatasetFile(const QString& filename);

private:
    enum class Stage { Ready, Attacked, CutProposed, Patched, Completed };
    Graph graph;
    Graph initialGraph;
    Stage stage = Stage::Ready;
    AttackResult beforeAttack;
    AttackResult afterAttack;
    bool hasBefore = false;
    bool hasAfter = false;
    bool reachableBefore = false;
    bool reachableAfter = false;
    std::optional<MinCutResult> cutResult;
    std::vector<int> patchedIds;
    int scenarioSource = -1;
    int scenarioTarget = -1;
    std::int64_t scenarioBudget = 0;
    bool datasetLoaded = false;
    int selectedNodeId = -1;

    QTreeWidget* nodeTree;
    QComboBox* sourceSelector;
    QComboBox* targetSelector;
    QLineEdit* searchInput;
    QLineEdit* budgetInput;
    QLabel* datasetLabel;
    QLabel* summaryLabel;
    GraphView* graphView;
    QComboBox* graphMode;
    QLabel* remainingLabel;
    QLabel* attackStatusLabel;
    QLabel* costLabel;
    QLabel* pathLabel;
    QLabel* flowLabel;
    QLabel* cutCapacityLabel;
    QLabel* patchedLabel;
    QLabel* reachabilityLabel;
    QLabel* resultMessage;
    QListWidget* cutList;
    QTableWidget* comparison;
    QTabWidget* analysisTabs;
    QPushButton* attackButton;
    QPushButton* defendButton;
    QPushButton* reAttackButton;
    QPushButton* resetButton;
    QPushButton* patchButton;
    QLabel* validationLabel;
    QLabel* warningsLabel;
    QLabel* sourceLabel;
    QLabel* targetLabel;
    QLabel* budgetLabel;
    QLabel* nodeDetails;
    QPushButton* useAsSourceButton;
    QPushButton* useAsTargetButton;

    QWidget* createExplorer();
    QWidget* createGraphPanel();
    QWidget* createAnalysis();
    QWidget* createControls();
    void applyTheme();
    void importDataset();
    void populateNodes();
    void filterNodes(const QString& text);
    void selectNode(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void useSelectedAsSource();
    void useSelectedAsTarget();
    void updateScenario();
    void attack();
    void defend();
    void applyPatch();
    void reAttack();
    void resetSimulation();
    void clearResults();
    void updateAnalysis();
    void updateButtons();
    void selectGraphNode(int id);
    void refreshGraph();
    void showError(const QString& message);
    bool readBudget(std::int64_t& budget) const;
};
