#pragma once
#include "GraphView.h"
#include "Simulation.h"
#include <string>

using namespace std;

class SimulationUI {
  public:
    enum class Stage { READY, ATTACKED, CUT_PROPOSED, PATCHED, COMPLETED };
    explicit SimulationUI(Font font);
    bool importDataset(const string& filename);
    void frame();
    void attack();
    void defend();
    void applyPatch();
    void reAttack();
    void reset();

  private:
    Font font;
    Graph originalGraph;
    Graph graph;
    GraphView view;
    Stage stage = Stage::READY;
    int source = -1;
    int target = -1;
    int selectedNode = -1;
    string datasetPath;
    string budgetText = "30";
    string searchText;
    string message = "Import a JSON dataset to begin.";
    int activeInput = 0;
    bool selectAll = false;
    int popup = 0;
    int scroll = 0;
    bool showBefore = false;
    bool showLabels = false;
    bool dimEdges = true;
    bool hasBefore = false;
    bool hasAfter = false;
    bool hasCut = false;
    bool reachableBefore = false;
    bool reachableAfter = false;
    int64_t initialBudget = 0;
    AttackResult before;
    AttackResult after;
    MinCutResult cut;
    vector<bool> patchSelection;
    vector<int> patchedIds;
    void clearResults();
    void refreshView();
    void chooseNode(int id, bool chooseSource);
    void drawPopup(bool acceptInput);
    void drawResults(float y);
    bool button(Rectangle area, const string& label, bool enabled = true);
    void textbox(Rectangle area, string& value, int id, bool enabled = true);
    void text(const string& value, float x, float y, float size = 18, Color color = WHITE) const;
    void clippedText(const string& value, Rectangle area, Color color = WHITE) const;
    string nodeLabel(int id) const;
    bool readBudget(int64_t& value) const;
};
