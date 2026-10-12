#include "MainWindow.h"
#include "LoadDataset.h"
#include "GraphView.h"
#include "Defender.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QListWidget>
#include <QScrollArea>
#include <QTimer>
#include <QTabWidget>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace {
QLabel* makeTitle(const QString& text) {
    QLabel* label = new QLabel(text);
    label->setObjectName("panelTitle");
    return label;
}

QLabel* makeValue(const QString& text, const QString& name) {
    QLabel* label = new QLabel(text);
    label->setObjectName(name);
    label->setWordWrap(true);
    label->setTextFormat(Qt::PlainText);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

QString nodeText(const Node& node) {
    return QString::number(node.getID()) + " - " + QString::fromStdString(node.getName());
}
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("AttackGraph Simulation");
    resize(1360, 860);
    setMinimumSize(1040, 760);

    QWidget* central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->setContentsMargins(20, 16, 20, 12);
    layout->setSpacing(16);
    layout->addWidget(makeTitle("AttackGraph Simulation"));

    QSplitter* panels = new QSplitter(Qt::Horizontal);
    panels->addWidget(createExplorer());
    panels->addWidget(createGraphPanel());
    panels->addWidget(createAnalysis());
    panels->setChildrenCollapsible(false);
    panels->setStretchFactor(0, 0);
    panels->setStretchFactor(1, 1);
    panels->setStretchFactor(2, 0);
    panels->setSizes({280, 740, 300});
    layout->addWidget(panels, 1);
    layout->addWidget(createControls());
    setCentralWidget(central);
    applyTheme();
    updateButtons();
    statusBar()->showMessage("Import a JSON dataset to choose your scenario.");
}

QWidget* MainWindow::createExplorer() {
    QFrame* panel = new QFrame;
    panel->setObjectName("panel");
    panel->setMinimumWidth(250);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setSpacing(10);
    layout->addWidget(makeTitle("Explorer"));

    QPushButton* importButton = new QPushButton("Import JSON");
    importButton->setObjectName("importButton");
    connect(importButton, &QPushButton::clicked, this, &MainWindow::importDataset);
    layout->addWidget(importButton);

    datasetLabel = makeValue("No dataset loaded", "datasetLabel");
    layout->addWidget(datasetLabel);
    searchInput = new QLineEdit;
    searchInput->setObjectName("searchInput");
    searchInput->setPlaceholderText("Search by ID or name...");
    searchInput->setClearButtonEnabled(true);
    searchInput->setEnabled(false);
    connect(searchInput, &QLineEdit::textChanged, this, &MainWindow::filterNodes);
    layout->addWidget(searchInput);

    nodeTree = new QTreeWidget;
    nodeTree->setObjectName("nodeTree");
    nodeTree->setHeaderHidden(true);
    nodeTree->setMinimumHeight(180);
    nodeTree->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(nodeTree, &QTreeWidget::currentItemChanged, this, &MainWindow::selectNode);
    layout->addWidget(nodeTree, 1);

    QHBoxLayout* selectionButtons = new QHBoxLayout;
    useAsSourceButton = new QPushButton("Use as Source");
    useAsSourceButton->setObjectName("useAsSourceButton");
    useAsTargetButton = new QPushButton("Use as Target");
    useAsTargetButton->setObjectName("useAsTargetButton");
    useAsSourceButton->setEnabled(false);
    useAsTargetButton->setEnabled(false);
    connect(useAsSourceButton, &QPushButton::clicked, this, &MainWindow::useSelectedAsSource);
    connect(useAsTargetButton, &QPushButton::clicked, this, &MainWindow::useSelectedAsTarget);
    selectionButtons->addWidget(useAsSourceButton);
    selectionButtons->addWidget(useAsTargetButton);
    layout->addLayout(selectionButtons);

    layout->addWidget(new QLabel("Source"));
    sourceSelector = new QComboBox;
    sourceSelector->setObjectName("sourceSelector");
    sourceSelector->setEnabled(false);
    layout->addWidget(sourceSelector);
    layout->addWidget(new QLabel("Target"));
    targetSelector = new QComboBox;
    targetSelector->setObjectName("targetSelector");
    targetSelector->setEnabled(false);
    layout->addWidget(targetSelector);
    connect(sourceSelector, &QComboBox::currentIndexChanged, this, &MainWindow::updateScenario);
    connect(targetSelector, &QComboBox::currentIndexChanged, this, &MainWindow::updateScenario);

    layout->addWidget(new QLabel("Token Budget"));
    budgetInput = new QLineEdit("0");
    budgetInput->setObjectName("budgetInput");
    budgetInput->setPlaceholderText("Nonnegative integer");
    budgetInput->setEnabled(false);
    connect(budgetInput, &QLineEdit::textChanged, this, &MainWindow::updateScenario);
    layout->addWidget(budgetInput);
    validationLabel = makeValue("", "validationLabel");
    layout->addWidget(validationLabel);
    return panel;
}

QWidget* MainWindow::createGraphPanel() {
    QFrame* panel = new QFrame;
    panel->setObjectName("graphPanel");
    panel->setMinimumWidth(350);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->addWidget(makeTitle("Network Graph"));
    summaryLabel = makeValue("Import a dataset to display the graph.", "summaryLabel");
    layout->addWidget(summaryLabel);
    QHBoxLayout* toolbar = new QHBoxLayout;
    QPushButton* zoomIn = new QPushButton("+");
    QPushButton* zoomOut = new QPushButton("-");
    QPushButton* fit = new QPushButton("Fit");
    QCheckBox* edgeLabels = new QCheckBox("Edge labels");
    QCheckBox* dimEdges = new QCheckBox("Dim unrelated");
    edgeLabels->setObjectName("edgeLabels");
    dimEdges->setObjectName("dimEdges");
    toolbar->addWidget(zoomIn);
    toolbar->addWidget(zoomOut);
    toolbar->addWidget(fit);
    toolbar->addWidget(edgeLabels);
    toolbar->addWidget(dimEdges);
    graphMode = new QComboBox;
    graphMode->setObjectName("graphMode");
    graphMode->addItems({"Current graph", "Before defense"});
    graphMode->setEnabled(false);
    layout->addWidget(graphMode);
    toolbar->addStretch();
    layout->addLayout(toolbar);
    graphView = new GraphView;
    connect(zoomIn, &QPushButton::clicked, graphView, &GraphView::zoomIn);
    connect(zoomOut, &QPushButton::clicked, graphView, &GraphView::zoomOut);
    connect(fit, &QPushButton::clicked, graphView, &GraphView::fitGraph);
    connect(edgeLabels, &QCheckBox::toggled, graphView, &GraphView::setEdgeLabelsVisible);
    connect(dimEdges, &QCheckBox::toggled, graphView, &GraphView::setDimUnrelated);
    connect(graphMode, &QComboBox::currentIndexChanged, this, &MainWindow::refreshGraph);
    connect(graphView, &GraphView::nodeSelected, this, &MainWindow::selectGraphNode);
    layout->addWidget(graphView, 1);
    QLabel* legend = new QLabel("Orange: attack   |   Purple: cut   |   Red dashed: blocked");
    legend->setWordWrap(true);
    layout->addWidget(legend);
    warningsLabel = makeValue("", "warningsLabel");
    warningsLabel->setMaximumHeight(85);
    layout->addWidget(warningsLabel);
    return panel;
}

QWidget* MainWindow::createAnalysis() {
    QFrame* panel = new QFrame;
    panel->setObjectName("panel");
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setSpacing(8);
    layout->addWidget(makeTitle("Analysis"));
    layout->addWidget(new QLabel("Source"));
    sourceLabel = makeValue("--", "sourceLabel");
    layout->addWidget(sourceLabel);
    layout->addWidget(new QLabel("Target"));
    targetLabel = makeValue("--", "targetLabel");
    layout->addWidget(targetLabel);
    layout->addWidget(new QLabel("Initial Budget / Remaining Token"));
    budgetLabel = makeValue("--", "budgetLabel");
    remainingLabel = makeValue("--", "remainingLabel");
    layout->addWidget(budgetLabel);
    layout->addWidget(remainingLabel);
    attackStatusLabel = makeValue("--", "attackStatusLabel");
    costLabel = makeValue("--", "costLabel");
    pathLabel = makeValue("--", "pathLabel");
    layout->addWidget(new QLabel("Attack Status / Cost / Path"));
    layout->addWidget(attackStatusLabel);
    layout->addWidget(costLabel);
    layout->addWidget(pathLabel);
    flowLabel = makeValue("--", "flowLabel");
    cutCapacityLabel = makeValue("--", "cutCapacityLabel");
    layout->addWidget(new QLabel("Defense before patch: Max-Flow / Min-Cut Capacity"));
    layout->addWidget(flowLabel);
    layout->addWidget(cutCapacityLabel);
    layout->addWidget(new QLabel("Proposed Cut: choose edges to patch"));
    cutList = new QListWidget;
    cutList->setObjectName("cutList");
    cutList->setMaximumHeight(125);
    cutList->setMinimumHeight(65);
    layout->addWidget(cutList);
    patchButton = new QPushButton("Apply Selected Patch");
    patchButton->setObjectName("patchButton");
    connect(patchButton, &QPushButton::clicked, this, &MainWindow::applyPatch);
    layout->addWidget(patchButton);
    layout->addWidget(new QLabel("Patched Edge IDs"));
    patchedLabel = makeValue("--", "patchedLabel");
    layout->addWidget(patchedLabel);
    reachabilityLabel = makeValue("--", "reachabilityLabel");
    layout->addWidget(new QLabel("Reachability Before / After"));
    layout->addWidget(reachabilityLabel);
    comparison = new QTableWidget(4, 3);
    comparison->setObjectName("comparisonTable");
    comparison->setHorizontalHeaderLabels({"Metric", "Before", "After"});
    comparison->verticalHeader()->hide();
    comparison->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    comparison->setEditTriggers(QAbstractItemView::NoEditTriggers);
    comparison->setSelectionMode(QAbstractItemView::NoSelection);
    comparison->setMinimumHeight(165);
    comparison->setMaximumHeight(175);
    const QStringList metrics{"Status", "Cost", "Remaining", "Reachable"};
    for (int row = 0; row < metrics.size(); ++row) {
        comparison->setItem(row, 0, new QTableWidgetItem(metrics.at(row)));
        comparison->setItem(row, 1, new QTableWidgetItem("--"));
        comparison->setItem(row, 2, new QTableWidgetItem("--"));
    }
    resultMessage = makeValue("Import a dataset, then click Attack.", "resultMessage");
    layout->addWidget(resultMessage);
    layout->addWidget(makeTitle("Selected Node"));
    nodeDetails = makeValue("Select a node in Explorer or on the graph.", "nodeDetails");
    layout->addWidget(nodeDetails);
    layout->addStretch();
    QScrollArea* area = new QScrollArea;
    area->setWidgetResizable(true);
    area->setFrameShape(QFrame::NoFrame);
    area->setMinimumWidth(290);
    area->setWidget(panel);
    QWidget* comparePage = new QWidget;
    QVBoxLayout* compareLayout = new QVBoxLayout(comparePage);
    compareLayout->addWidget(makeTitle("Before / After"));
    compareLayout->addWidget(comparison);
    QLabel* budgetNote = new QLabel("Re-Attack receives the same initial budget as Before.");
    budgetNote->setWordWrap(true);
    compareLayout->addWidget(budgetNote);
    compareLayout->addStretch();
    analysisTabs = new QTabWidget;
    analysisTabs->setObjectName("analysisTabs");
    analysisTabs->setMinimumWidth(290);
    analysisTabs->addTab(area, "Analysis");
    analysisTabs->addTab(comparePage, "Comparison");
    return analysisTabs;
}

QWidget* MainWindow::createControls() {
    QFrame* bar = new QFrame;
    bar->setObjectName("controlBar");
    QHBoxLayout* layout = new QHBoxLayout(bar);
    attackButton = new QPushButton("Attack");
    defendButton = new QPushButton("Defend");
    reAttackButton = new QPushButton("Re-Attack");
    resetButton = new QPushButton("Reset");
    attackButton->setObjectName("attackButton");
    defendButton->setObjectName("defendButton");
    reAttackButton->setObjectName("reAttackButton");
    resetButton->setObjectName("resetButton");
    connect(attackButton, &QPushButton::clicked, this, &MainWindow::attack);
    connect(defendButton, &QPushButton::clicked, this, &MainWindow::defend);
    connect(reAttackButton, &QPushButton::clicked, this, &MainWindow::reAttack);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetSimulation);
    for (QPushButton* button : {attackButton, defendButton, reAttackButton, resetButton}) {
        button->setMinimumHeight(42);
        layout->addWidget(button);
    }
    return bar;
}

void MainWindow::applyTheme() {
    setStyleSheet(QStringLiteral(R"(
        QMainWindow, QWidget { background: #0c1016; color: #e5eaf0; font-size: 13px; }
        QFrame#panel { background: #111720; border: 1px solid #222c3a; border-radius: 8px; }
        QFrame#graphPanel { background: #0a0e14; border: 1px solid #222c3a; border-radius: 8px; }
        QFrame#controlBar { border-top: 1px solid #222c3a; }
        QLabel { background: transparent; }
        QLabel#panelTitle { font-size: 17px; font-weight: 600; padding: 4px 0; }
        QLabel#datasetLabel, QLabel#summaryLabel, QLabel#resultMessage { color: #94a2b5; }
        QLabel#sourceLabel, QLabel#targetLabel, QLabel#budgetLabel { font-size: 15px; font-weight: 600; }
        QLabel#validationLabel { color: #f18a8a; }
        QLabel#warningsLabel { color: #e7b665; }
        QLineEdit, QComboBox { background: #192230; border: 1px solid #2b394d; border-radius: 5px; padding: 7px; }
        QLineEdit:focus, QComboBox:focus { border: 1px solid #5d9ce6; }
        QComboBox QAbstractItemView { background: #192230; selection-background-color: #25446b; }
        QPushButton { background: #1b2737; border: 1px solid #31425a; border-radius: 5px; padding: 8px; }
        QPushButton:hover { background: #26374d; border-color: #5d9ce6; }
        QPushButton#importButton { background: #245a95; border-color: #3977ba; font-weight: 600; }
        QPushButton:disabled, QLineEdit:disabled, QComboBox:disabled { color: #637084; background: #141b26; border-color: #232e3e; }
        QTreeWidget, QTableWidget, QListWidget { background: #111720; border: none; gridline-color: #273346; }
        QTreeWidget::item { padding: 6px 2px; }
        QTreeWidget::item:selected { background: #25446b; color: #ffffff; }
        QTabWidget::pane { border: 1px solid #222c3a; }
        QTabBar::tab { background: #141b26; color: #9db0c7; padding: 9px 12px; }
        QTabBar::tab:selected { background: #25446b; color: #ffffff; }
        QHeaderView::section { background: #192230; color: #adbdd0; border: none; padding: 7px; }
        QSplitter::handle { background: #0c1016; width: 10px; }
        QScrollBar:horizontal { background: #111720; height: 9px; margin: 0; }
        QScrollBar::handle:horizontal { background: #35445a; min-width: 24px; border-radius: 4px; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }
        QScrollBar:vertical { background: #111720; width: 9px; margin: 0; }
        QScrollBar::handle:vertical { background: #35445a; min-height: 24px; border-radius: 4px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
        QStatusBar { color: #94a2b5; background: #0c1016; }
    )"));
}

void MainWindow::importDataset() {
    const QString filename = QFileDialog::getOpenFileName(
        this, "Import Dataset", QString(), "JSON datasets (*.json);;All files (*)");
    if (!filename.isEmpty()) {
        loadDatasetFile(filename);
    }
}

bool MainWindow::loadDatasetFile(const QString& filename) {
    try {
        // QFile ho tro duong dan Unicode tren Windows; Core van doc JSON tu stream.
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            throw std::runtime_error(file.errorString().toStdString());
        }
        const QByteArray contents = file.readAll();
        if (file.error() != QFileDevice::NoError) {
            throw std::runtime_error(file.errorString().toStdString());
        }
        std::istringstream input(contents.toStdString());
        Graph importedGraph = loadDataset(input);
        const std::vector<std::string> warnings = importedGraph.validationWarnings();

        initialGraph = importedGraph;
        graph = importedGraph;
        datasetLoaded = true;
        clearResults();
        datasetLabel->setText(QFileInfo(filename).fileName());
        datasetLabel->setToolTip(QFileInfo(filename).absoluteFilePath());
        const GraphStatistics statistics = graph.statistics();
        summaryLabel->setText(QString("%1 nodes  /  %2 edges  /  %3 blocked")
            .arg(statistics.nodes).arg(statistics.edges).arg(statistics.blockedEdges));

        QStringList warningTexts;
        for (const std::string& warning : warnings) {
            warningTexts.append(QString::fromStdString(warning));
        }
        warningsLabel->setText(warningTexts.join('\n'));
        warningsLabel->setToolTip(warningTexts.join('\n'));
        graphView->setGraph(graph);
        populateNodes();
        QTimer::singleShot(0, graphView, &GraphView::fitGraph);
        statusBar()->showMessage("Loaded: " + QFileInfo(filename).absoluteFilePath());
        return true;
    } catch (const std::exception& error) {
        const QString message = QString::fromUtf8(error.what());
        statusBar()->showMessage("Import failed. Previous dataset kept.");
        QMessageBox messageBox(QMessageBox::Critical, "Cannot import dataset", message,
                              QMessageBox::Ok, this);
        messageBox.setTextFormat(Qt::PlainText);
        messageBox.exec();
        return false;
    }
}

void MainWindow::populateNodes() {
    const QSignalBlocker treeSignals(nodeTree);
    const QSignalBlocker sourceSignals(sourceSelector);
    const QSignalBlocker targetSignals(targetSelector);
    const QSignalBlocker searchSignals(searchInput);
    const QSignalBlocker budgetSignals(budgetInput);
    nodeTree->clear();
    sourceSelector->clear();
    targetSelector->clear();
    searchInput->clear();
    selectedNodeId = -1;
    nodeDetails->setText("Select a node in Explorer.");
    useAsSourceButton->setEnabled(false);
    useAsTargetButton->setEnabled(false);

    const NodeType types[]{NodeType::ENTRY, NodeType::ENDPOINT, NodeType::IDENTITY,
                           NodeType::CRITICAL_SYSTEM, NodeType::TARGET};
    for (NodeType type : types) {
        QTreeWidgetItem* group = new QTreeWidgetItem(nodeTree);
        const QString typeName = QString::fromStdString(nodeTypeToString(type));
        int count = 0;
        for (const Node& node : graph.getNodes()) {
            if (node.getType() == type) {
                QTreeWidgetItem* item = new QTreeWidgetItem(group);
                item->setText(0, nodeText(node));
                item->setData(0, Qt::UserRole, node.getID());
                item->setToolTip(0, nodeText(node) + "\n" + typeName);
                ++count;
            }
        }
        group->setText(0, typeName + " (" + QString::number(count) + ")");
        group->setExpanded(true);
    }

    int firstEntry = -1;
    int firstTarget = -1;
    for (const Node& node : graph.getNodes()) {
        const int index = sourceSelector->count();
        sourceSelector->addItem(nodeText(node), node.getID());
        targetSelector->addItem(nodeText(node), node.getID());
        if (firstEntry == -1 && node.getType() == NodeType::ENTRY) {
            firstEntry = index;
        }
        if (firstTarget == -1 && node.getType() == NodeType::TARGET) {
            firstTarget = index;
        }
    }
    if (firstEntry != -1) {
        sourceSelector->setCurrentIndex(firstEntry);
    }
    if (firstTarget != -1) {
        targetSelector->setCurrentIndex(firstTarget);
    }
    const bool hasNodes = !graph.getNodes().empty();
    searchInput->setEnabled(hasNodes);
    sourceSelector->setEnabled(hasNodes);
    targetSelector->setEnabled(hasNodes);
    budgetInput->setEnabled(hasNodes);
    budgetInput->setText("0");
    updateScenario();
}

void MainWindow::filterNodes(const QString& text) {
    const QString query = text.trimmed();
    for (int index = 0; index < nodeTree->topLevelItemCount(); ++index) {
        QTreeWidgetItem* group = nodeTree->topLevelItem(index);
        bool hasVisibleNode = false;
        for (int childIndex = 0; childIndex < group->childCount(); ++childIndex) {
            QTreeWidgetItem* item = group->child(childIndex);
            const bool matches = item->text(0).contains(query, Qt::CaseInsensitive);
            item->setHidden(!matches);
            if (matches) {
                hasVisibleNode = true;
            }
        }
        group->setHidden(!query.isEmpty() && !hasVisibleNode);
    }
}

void MainWindow::selectNode(QTreeWidgetItem* current, QTreeWidgetItem*) {
    selectedNodeId = -1;
    if (current != nullptr && current->parent() != nullptr) {
        selectedNodeId = current->data(0, Qt::UserRole).toInt();
    }
    const Node* node = graph.getNode(selectedNodeId);
    useAsSourceButton->setEnabled(node != nullptr);
    useAsTargetButton->setEnabled(node != nullptr);
    if (node == nullptr) {
        nodeDetails->setText("Select a node in Explorer.");
        return;
    }
    analysisTabs->setCurrentIndex(0);
    graphView->selectNode(selectedNodeId);
    nodeDetails->setText(nodeText(*node) + "\n" +
        QString::fromStdString(nodeTypeToString(node->getType())));
}

void MainWindow::useSelectedAsSource() {
    const int index = sourceSelector->findData(selectedNodeId);
    if (index != -1) {
        sourceSelector->setCurrentIndex(index);
    }
}

void MainWindow::useSelectedAsTarget() {
    const int index = targetSelector->findData(selectedNodeId);
    if (index != -1) {
        targetSelector->setCurrentIndex(index);
    }
}

bool MainWindow::readBudget(std::int64_t& budget) const {
    const QString text = budgetInput->text().trimmed();
    if (text.isEmpty()) {
        return false;
    }
    for (const QChar character : text) {
        if (character < QChar('0') || character > QChar('9')) {
            return false;
        }
    }
    bool converted = false;
    const qulonglong value = text.toULongLong(&converted);
    if (!converted || value > static_cast<qulonglong>(std::numeric_limits<std::int64_t>::max())) {
        return false;
    }
    budget = static_cast<std::int64_t>(value);
    return true;
}

void MainWindow::updateScenario() {
    if (hasBefore) {
        graph = initialGraph;
        clearResults();
        graphView->updateGraph(graph, {}, {});
        statusBar()->showMessage("Parameters changed: new scenario from the original dataset.");
    }
    updateButtons();
    if (!datasetLoaded || graph.getNodes().empty()) {
        sourceLabel->setText("--");
        targetLabel->setText("--");
        budgetLabel->setText("--");
        validationLabel->setText(datasetLoaded ? "Dataset has no nodes to select." : "");
        return;
    }
    sourceLabel->setText(sourceSelector->currentText());
    targetLabel->setText(targetSelector->currentText());
    std::int64_t budget = 0;
    if (!readBudget(budget)) {
        budgetLabel->setText("Invalid");
        validationLabel->setText("Budget must be an integer from 0 to 9223372036854775807.");
        return;
    }
    budgetLabel->setText(QString::number(static_cast<qlonglong>(budget)));
    validationLabel->setText("");
}

namespace {
QString attackStatusText(AttackStatus status) {
    if (status == AttackStatus::SUCCESS) return "SUCCESS";
    if (status == AttackStatus::OVER_BUDGET) return "OVER_BUDGET";
    return "NO_PATH";
}

QString idListText(const std::vector<int>& ids) {
    QStringList texts;
    for (int id : ids) texts.append(QString::number(id));
    return texts.isEmpty() ? "none" : texts.join(", ");
}

QString pathText(const Graph& graph, const PathResult& path) {
    if (!path.reachable) return "No path";
    QStringList texts;
    for (int id : path.nodes) {
        const Node* node = graph.getNode(id);
        texts.append(node == nullptr ? QString::number(id) : nodeText(*node));
    }
    return texts.join(" -> ");
}
}

void MainWindow::showError(const QString& message) {
    QMessageBox box(QMessageBox::Critical, "Simulation error", message, QMessageBox::Ok, this);
    box.setTextFormat(Qt::PlainText);
    box.exec();
}

void MainWindow::clearResults() {
    const QSignalBlocker modeSignals(graphMode);
    graphMode->setCurrentIndex(0);
    graphMode->setEnabled(false);
    analysisTabs->setCurrentIndex(0);
    stage = Stage::Ready;
    hasBefore = false;
    hasAfter = false;
    reachableBefore = false;
    reachableAfter = false;
    beforeAttack = AttackResult{};
    afterAttack = AttackResult{};
    cutResult.reset();
    patchedIds.clear();
    cutList->clear();
    cutList->setEnabled(false);
    updateAnalysis();
}

void MainWindow::updateButtons() {
    std::int64_t budget = 0;
    const bool valid = datasetLoaded && sourceSelector->currentIndex() >= 0 &&
        targetSelector->currentIndex() >= 0 && readBudget(budget);
    attackButton->setEnabled(valid && (stage == Stage::Ready || stage == Stage::Attacked));
    defendButton->setEnabled(valid && stage == Stage::Attacked);
    patchButton->setEnabled(valid && stage == Stage::CutProposed && cutList->count() > 0);
    reAttackButton->setEnabled(valid && stage == Stage::Patched);
    resetButton->setEnabled(datasetLoaded);
    graphMode->setEnabled(hasBefore);
}

void MainWindow::attack() {
    try {
        if (!attackButton->isEnabled()) return;
        if (!readBudget(scenarioBudget)) return;
        scenarioSource = sourceSelector->currentData().toInt();
        scenarioTarget = targetSelector->currentData().toInt();
        const AttackResult result = hackerSimulation(graph, scenarioSource, scenarioTarget, scenarioBudget);
        const bool reachable = Defender(graph).isReachable(scenarioSource, scenarioTarget);
        beforeAttack = result;
        hasBefore = true;
        hasAfter = false;
        reachableBefore = reachable;
        cutResult.reset();
        patchedIds.clear();
        cutList->clear();
        stage = Stage::Attacked;
        analysisTabs->setCurrentIndex(0);
        updateAnalysis();
        refreshGraph();
        resultMessage->setText(beforeAttack.path.reachable ?
            "Attack recorded. Click Defend to calculate a cut." :
            "No active path to the target. No patch is needed.");
        if (scenarioSource == scenarioTarget) resultMessage->setText(
            "Source equals Target: cost is 0. Min-Cut is not defined for this case.");
        statusBar()->showMessage("Attack: " + attackStatusText(beforeAttack.status));
        updateButtons();
    } catch (const std::exception& error) {
        showError(QString::fromUtf8(error.what()));
    }
}

void MainWindow::defend() {
    try {
        if (stage != Stage::Attacked) return;
        if (scenarioSource == scenarioTarget || !reachableBefore) {
            stage = Stage::Patched;
            reachableAfter = reachableBefore;
            updateAnalysis();
            resultMessage->setText("No patch applied. Re-Attack can confirm the unchanged graph.");
            updateButtons();
            return;
        }
        MinCutResult result = minimumSTCut(graph, scenarioSource, scenarioTarget);
        cutResult = result;
        cutList->clear();
        for (int id : result.edgeIds) {
            const Edge* edge = graph.getEdge(id);
            auto* item = new QListWidgetItem(QString("#%1: %2 -> %3  (capacity %4)")
                .arg(id).arg(edge->getFrom()).arg(edge->getTo()).arg(edge->getCapacity()), cutList);
            item->setData(Qt::UserRole, id);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
        }
        cutList->setEnabled(true);
        stage = Stage::CutProposed;
        updateAnalysis();
        refreshGraph();
        resultMessage->setText("Cut highlighted in purple. Select edges, then Apply Selected Patch.");
        statusBar()->showMessage("Min-Cut proposed; the graph is not patched yet.");
        updateButtons();
    } catch (const std::exception& error) {
        showError(QString::fromUtf8(error.what()));
    }
}

void MainWindow::applyPatch() {
    try {
        if (stage != Stage::CutProposed) return;
        std::vector<int> selected;
        std::int64_t capacity = 0;
        for (int index = 0; index < cutList->count(); ++index) {
            QListWidgetItem* item = cutList->item(index);
            if (item->checkState() == Qt::Checked) {
                int id = item->data(Qt::UserRole).toInt();
                selected.push_back(id);
                capacity += graph.getEdge(id)->getCapacity();
            }
        }
        if (selected.empty()) {
            resultMessage->setText("Select at least one cut edge to patch.");
            return;
        }
        Graph patchedGraph = graph;
        patchedGraph.blockEdges(selected);
        const bool reachable = Defender(patchedGraph).isReachable(scenarioSource, scenarioTarget);
        graph = patchedGraph;
        patchedIds = selected;
        reachableAfter = reachable;
        stage = Stage::Patched;
        cutList->setEnabled(false);
        updateAnalysis();
        refreshGraph();
        resultMessage->setText(QString("Applied patch capacity: %1. %2 Click Re-Attack.")
            .arg(static_cast<qlonglong>(capacity))
            .arg(reachable ? "A path still exists (partial cut)." : "Source and Target are disconnected."));
        statusBar()->showMessage("Patch applied in memory; JSON is unchanged.");
        updateButtons();
    } catch (const std::exception& error) {
        showError(QString::fromUtf8(error.what()));
    }
}

void MainWindow::reAttack() {
    try {
        if (stage != Stage::Patched) return;
        afterAttack = hackerSimulation(graph, scenarioSource, scenarioTarget, scenarioBudget);
        reachableAfter = Defender(graph).isReachable(scenarioSource, scenarioTarget);
        hasAfter = true;
        stage = Stage::Completed;
        analysisTabs->setCurrentIndex(1);
        updateAnalysis();
        refreshGraph();
        resultMessage->setText("Before / After recorded with the same initial budget. Reset for a new run.");
        statusBar()->showMessage("Re-Attack: " + attackStatusText(afterAttack.status));
        updateButtons();
    } catch (const std::exception& error) {
        showError(QString::fromUtf8(error.what()));
    }
}

void MainWindow::resetSimulation() {
    if (!datasetLoaded) return;
    graph = initialGraph;
    clearResults();
    graphView->updateGraph(graph, {}, {});
    updateScenario();
    statusBar()->showMessage("Reset: original dataset restored, including its initial blocked edges.");
}

void MainWindow::updateAnalysis() {
    const GraphStatistics statistics = graph.statistics();
    if (datasetLoaded) summaryLabel->setText(QString("%1 nodes  /  %2 edges  /  %3 blocked")
        .arg(statistics.nodes).arg(statistics.edges).arg(statistics.blockedEdges));
    flowLabel->setText(cutResult ? QString::number(static_cast<qlonglong>(cutResult->maxFlow)) : "--");
    cutCapacityLabel->setText(cutResult ? QString::number(static_cast<qlonglong>(cutResult->minCutCapacity)) : "--");
    patchedLabel->setText(hasBefore ? idListText(patchedIds) : "--");
    const bool hasPatchState = stage == Stage::Patched || stage == Stage::Completed;
    reachabilityLabel->setText(hasBefore ?
        QString(reachableBefore ? "Yes" : "No") + " -> " +
        (hasPatchState ? (reachableAfter ? "Yes" : "No") : "--") : "--");
    for (int column = 1; column <= 2; ++column) {
        const bool available = column == 1 ? hasBefore : hasAfter;
        const AttackResult& result = column == 1 ? beforeAttack : afterAttack;
        comparison->item(0, column)->setText(available ? attackStatusText(result.status) : "--");
        comparison->item(1, column)->setText(available && result.path.reachable ?
            QString::number(static_cast<qlonglong>(result.path.totalCost)) : "--");
        comparison->item(2, column)->setText(available && result.remainingToken ?
            QString::number(static_cast<qlonglong>(*result.remainingToken)) : "--");
        const bool reachKnown = column == 1 ? hasBefore : hasPatchState;
        const bool reachable = column == 1 ? reachableBefore : reachableAfter;
        comparison->item(3, column)->setText(reachKnown ? (reachable ? "Yes" : "No") : "--");
    }
    for (int row = 0; row < comparison->rowCount(); ++row) {
        for (int column = 0; column < comparison->columnCount(); ++column) {
            comparison->item(row, column)->setToolTip(comparison->item(row, column)->text());
        }
    }
    const AttackResult& current = hasAfter ? afterAttack : beforeAttack;
    attackStatusLabel->setText(hasBefore ? attackStatusText(current.status) : "--");
    costLabel->setText(hasBefore && current.path.reachable ?
        QString::number(static_cast<qlonglong>(current.path.totalCost)) : "--");
    pathLabel->setText(hasBefore ? pathText(graph, current.path) : "--");
    remainingLabel->setText(hasBefore && current.remainingToken ?
        QString::number(static_cast<qlonglong>(*current.remainingToken)) : "--");
    QString statusColor = "#94a2b5";
    if (hasBefore) {
        if (current.status == AttackStatus::SUCCESS) statusColor = "#59c997";
        else if (current.status == AttackStatus::OVER_BUDGET) statusColor = "#f5b04d";
        else statusColor = "#f07880";
    }
    attackStatusLabel->setStyleSheet("color: " + statusColor + "; font-weight: bold;");
    if (!hasBefore) resultMessage->setText("Choose Source / Target / Budget, then click Attack.");
}

void MainWindow::selectGraphNode(int id) {
    searchInput->clear();
    for (int index = 0; index < nodeTree->topLevelItemCount(); ++index) {
        QTreeWidgetItem* group = nodeTree->topLevelItem(index);
        for (int child = 0; child < group->childCount(); ++child) {
            QTreeWidgetItem* item = group->child(child);
            if (item->data(0, Qt::UserRole).toInt() == id) {
                group->setExpanded(true);
                nodeTree->setCurrentItem(item);
                nodeTree->scrollToItem(item);
                return;
            }
        }
    }
}

void MainWindow::refreshGraph() {
    const bool showBefore = graphMode->currentIndex() == 1 && hasBefore;
    const GraphStatistics displayed = (showBefore ? initialGraph : graph).statistics();
    summaryLabel->setText(QString("%1 nodes  /  %2 edges  /  %3 blocked%4")
        .arg(displayed.nodes).arg(displayed.edges).arg(displayed.blockedEdges)
        .arg(showBefore ? " (Before defense)" : ""));
    if (graphMode->currentIndex() == 1 && hasBefore) {
        graphView->updateGraph(initialGraph, beforeAttack.path.edgeIds, {});
    } else {
        std::vector<int> attackIds;
        std::vector<int> cutIds;
        if (hasBefore) attackIds = hasAfter ? afterAttack.path.edgeIds : beforeAttack.path.edgeIds;
        if (cutResult) cutIds = cutResult->edgeIds;
        graphView->updateGraph(graph, attackIds, cutIds);
    }
}
