#pragma once

#include "Graph.h"

#include <QMainWindow>
#include <QString>
#include <cstdint>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class QWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    bool loadDatasetFile(const QString& filename);

private:
    Graph graph;
    bool datasetLoaded = false;
    int selectedNodeId = -1;

    QTreeWidget* nodeTree;
    QComboBox* sourceSelector;
    QComboBox* targetSelector;
    QLineEdit* searchInput;
    QLineEdit* budgetInput;
    QLabel* datasetLabel;
    QLabel* summaryLabel;
    QLabel* graphMessage;
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
    bool readBudget(std::int64_t& budget) const;
};
