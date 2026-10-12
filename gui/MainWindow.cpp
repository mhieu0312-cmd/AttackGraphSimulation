#include "MainWindow.h"
#include "LoadDataset.h"

#include <QAbstractItemView>
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
    summaryLabel = makeValue("No dataset", "summaryLabel");
    layout->addWidget(summaryLabel);
    layout->addStretch();
    QLabel* symbol = new QLabel("GRAPH");
    symbol->setObjectName("graphSymbol");
    symbol->setAlignment(Qt::AlignCenter);
    layout->addWidget(symbol);
    graphMessage = makeValue("Import JSON to explore nodes and choose a scenario.", "graphMessage");
    graphMessage->setAlignment(Qt::AlignCenter);
    layout->addWidget(graphMessage);
    QLabel* stageLabel = makeValue("Graph visualization will be connected in the next step.", "stageLabel");
    stageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(stageLabel);
    layout->addStretch();
    warningsLabel = makeValue("", "warningsLabel");
    warningsLabel->setMaximumHeight(140);
    layout->addWidget(warningsLabel);
    return panel;
}

QWidget* MainWindow::createAnalysis() {
    QFrame* panel = new QFrame;
    panel->setObjectName("panel");
    panel->setMinimumWidth(260);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setSpacing(10);
    layout->addWidget(makeTitle("Analysis"));
    layout->addWidget(new QLabel("Source"));
    sourceLabel = makeValue("--", "sourceLabel");
    layout->addWidget(sourceLabel);
    layout->addWidget(new QLabel("Target"));
    targetLabel = makeValue("--", "targetLabel");
    layout->addWidget(targetLabel);
    layout->addWidget(new QLabel("Initial Budget"));
    budgetLabel = makeValue("--", "budgetLabel");
    layout->addWidget(budgetLabel);

    layout->addWidget(makeTitle("Selected Node"));
    nodeDetails = makeValue("Select a node in Explorer.", "nodeDetails");
    layout->addWidget(nodeDetails);
    layout->addWidget(makeTitle("Before / After"));

    QTableWidget* comparison = new QTableWidget(4, 3);
    comparison->setObjectName("comparisonTable");
    comparison->setHorizontalHeaderLabels({"Metric", "Before", "After"});
    comparison->verticalHeader()->hide();
    comparison->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    comparison->setEditTriggers(QAbstractItemView::NoEditTriggers);
    comparison->setSelectionMode(QAbstractItemView::NoSelection);
    comparison->setFocusPolicy(Qt::NoFocus);
    comparison->setMinimumHeight(165);
    const QStringList metrics{"Status", "Cost", "Remaining", "Reachable"};
    for (int row = 0; row < metrics.size(); ++row) {
        comparison->setItem(row, 0, new QTableWidgetItem(metrics.at(row)));
        comparison->setItem(row, 1, new QTableWidgetItem("--"));
        comparison->setItem(row, 2, new QTableWidgetItem("--"));
    }
    layout->addWidget(comparison);
    QLabel* resultMessage = makeValue("No simulation results yet.", "resultMessage");
    layout->addWidget(resultMessage);
    layout->addStretch();
    return panel;
}

QWidget* MainWindow::createControls() {
    QFrame* bar = new QFrame;
    bar->setObjectName("controlBar");
    QHBoxLayout* layout = new QHBoxLayout(bar);
    const QStringList names{"Attack", "Defend", "Re-Attack", "Reset"};
    for (const QString& name : names) {
        QPushButton* button = new QPushButton(name);
        button->setEnabled(false);
        button->setToolTip("Simulation controls will be connected in a later step.");
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
        QLabel#graphSymbol { color: #40536d; font-size: 44px; font-weight: 600; }
        QLabel#datasetLabel, QLabel#summaryLabel, QLabel#stageLabel, QLabel#resultMessage { color: #94a2b5; }
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
        QTreeWidget, QTableWidget { background: #111720; border: none; gridline-color: #273346; }
        QTreeWidget::item { padding: 6px 2px; }
        QTreeWidget::item:selected { background: #25446b; color: #ffffff; }
        QHeaderView::section { background: #192230; color: #adbdd0; border: none; padding: 7px; }
        QSplitter::handle { background: #0c1016; width: 10px; }
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

        graph = importedGraph;
        datasetLoaded = true;
        datasetLabel->setText(QFileInfo(filename).fileName());
        datasetLabel->setToolTip(QFileInfo(filename).absoluteFilePath());
        const GraphStatistics statistics = graph.statistics();
        summaryLabel->setText(QString("%1 nodes  /  %2 edges  /  %3 blocked")
            .arg(statistics.nodes).arg(statistics.edges).arg(statistics.blockedEdges));
        graphMessage->setText("Dataset loaded. Explore nodes and choose Source / Target.");
        QStringList warningTexts;
        for (const std::string& warning : warnings) {
            warningTexts.append(QString::fromStdString(warning));
        }
        warningsLabel->setText(warningTexts.join('\n'));
        warningsLabel->setToolTip(warningTexts.join('\n'));
        populateNodes();
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
