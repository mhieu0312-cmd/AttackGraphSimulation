#include "SimulationUI.h"
#include "LoadDataset.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <limits>

namespace {
const Color background{15,21,31,255};
const Color panel{24,33,47,255};
const Color muted{163,178,198,255};
const Color accent{245,176,77,255};
std::string idsText(const std::vector<int>& ids, const std::string& separator = ", ") {
    std::string result;
    for (int id : ids) {
        if (!result.empty()) result += separator;
        result += std::to_string(id);
    }
    return result.empty() ? "None" : result;
}
std::string statusText(AttackStatus status) {
    if (status == AttackStatus::SUCCESS) return "SUCCESS";
    if (status == AttackStatus::OVER_BUDGET) return "OVER_BUDGET";
    return "NO_PATH";
}
std::string lowerText(std::string value) {
    for (char& character : value) character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    return value;
}
void eraseLastCharacter(std::string& value) {
    if (value.empty()) return;
    size_t index = value.size()-1;
    while (index > 0 && (static_cast<unsigned char>(value[index]) & 0xC0) == 0x80) --index;
    value.erase(index);
}
}

SimulationUI::SimulationUI(Font selectedFont) : font(selectedFont) {}

void SimulationUI::text(const std::string& value, float x, float y, float size, Color color) const {
    DrawTextEx(font, value.c_str(), {x,y}, size, 1, color);
}

void SimulationUI::clippedText(const std::string& value, Rectangle area, Color color) const {
    BeginScissorMode(static_cast<int>(area.x), static_cast<int>(area.y),
                     static_cast<int>(area.width), static_cast<int>(area.height));
    text(value,area.x,area.y,18,color);
    EndScissorMode();
}

std::string SimulationUI::nodeLabel(int id) const {
    const Node* node = graph.getNode(id);
    if (node == nullptr) return "Choose node";
    return std::to_string(id) + "  " + node->getName();
}

bool SimulationUI::readBudget(std::int64_t& value) const {
    if (budgetText.empty()) return false;
    value = 0;
    for (char character : budgetText) {
        if (character < '0' || character > '9') return false;
        int digit = character-'0';
        if (value > (std::numeric_limits<std::int64_t>::max()-digit)/10) return false;
        value = value*10+digit;
    }
    return true;
}

void SimulationUI::clearResults() {
    graph = originalGraph;
    stage = Stage::READY;
    hasBefore = hasAfter = hasCut = false;
    showBefore = false;
    before = AttackResult{};
    after = AttackResult{};
    cut = MinCutResult{};
    patchedIds.clear();
    patchSelection.clear();
    popup = 0;
    refreshView();
}

void SimulationUI::refreshView() {
    const Graph& displayed = showBefore ? originalGraph : graph;
    std::vector<int> path;
    if (showBefore && hasBefore) path = before.path.edgeIds;
    else if (hasAfter) path = after.path.edgeIds;
    else if (hasBefore && stage != Stage::PATCHED) path = before.path.edgeIds;
    view.updateGraph(displayed,path,hasCut && !showBefore ? cut.edgeIds : std::vector<int>{});
}

bool SimulationUI::importDataset(const std::string& filename) {
    try {
        std::string path = filename;
        if (path.size() >= 2 && path.front() == '"' && path.back() == '"') path = path.substr(1,path.size()-2);
        std::ifstream input(std::filesystem::u8path(path));
        if (!input) throw std::runtime_error("Cannot open dataset: " + path);
        Graph loaded = loadDataset(input);
        originalGraph = loaded;
        datasetPath = path;
        source = target = -1;
        for (const Node& node : loaded.getNodes()) {
            if (source == -1 && node.getType() == NodeType::ENTRY) source = node.getID();
            if (target == -1 && node.getType() == NodeType::TARGET) target = node.getID();
        }
        if (!loaded.getNodes().empty()) {
            if (source == -1) source = loaded.getNodes().front().getID();
            if (target == -1) target = loaded.getNodes().back().getID();
        }
        selectedNode = -1;
        clearResults();
        view.setGraph(graph);
        message = "Loaded " + std::to_string(graph.getNodes().size()) + " nodes / " + std::to_string(graph.getEdges().size()) + " edges.";
        std::vector<std::string> warnings = graph.validationWarnings();
        if (!warnings.empty()) message += " Warning: " + warnings.front();
        return true;
    } catch (const std::exception& error) {
        message = "Import error: " + std::string(error.what());
        return false;
    }
}

void SimulationUI::chooseNode(int id, bool chooseSource) {
    if (graph.getNode(id) == nullptr) return;
    int& selected = chooseSource ? source : target;
    if (selected != id) {
        selected = id;
        clearResults();
        message = "Scenario changed. Run Attack again.";
    }
    popup = 0;
    activeInput = 0;
}

void SimulationUI::attack() {
    if (stage != Stage::READY) return;
    std::int64_t budget;
    if (graph.getNode(source) == nullptr || graph.getNode(target) == nullptr || !readBudget(budget)) {
        message = "Choose valid nodes and a nonnegative integer budget (up to INT64_MAX).";
        return;
    }
    try {
        before = hackerSimulation(graph,source,target,budget);
        reachableBefore = Defender(graph).isReachable(source,target);
        initialBudget = budget;
        hasBefore = true;
        stage = Stage::ATTACKED;
        message = "Dijkstra: " + statusText(before.status) + ". Defend can now calculate the minimum cut.";
        refreshView();
    } catch (const std::exception& error) { message = error.what(); }
}

void SimulationUI::defend() {
    if (stage != Stage::ATTACKED) return;
    try {
        showBefore = false;
        if (source == target || !reachableBefore) {
            stage = Stage::PATCHED;
            message = source == target ? "Source equals Target: no S-T cut. Graph unchanged." : "Already unreachable: no patch needed.";
        } else {
            cut = minimumSTCut(graph,source,target);
            hasCut = true;
            patchSelection.assign(cut.edgeIds.size(),true);
            stage = Stage::CUT_PROPOSED;
            popup = 3;
            scroll = 0;
            activeInput = 0;
            message = "Review Min-Cut edges, then Apply Patch. Partial selection may leave an attack path.";
        }
        refreshView();
    } catch (const std::exception& error) { message = error.what(); }
}

void SimulationUI::applyPatch() {
    if (stage != Stage::CUT_PROPOSED) return;
    std::vector<int> selected;
    for (size_t index = 0; index < cut.edgeIds.size(); ++index) {
        if (patchSelection[index]) selected.push_back(cut.edgeIds[index]);
    }
    if (selected.empty()) { message = "Select at least one cut edge to patch."; return; }
    try {
        Graph patched = graph;
        patched.blockEdges(selected);
        reachableAfter = Defender(patched).isReachable(source,target);
        graph = patched;
        patchedIds = selected;
        stage = Stage::PATCHED;
        popup = 0;
        showBefore = false;
        message = "Patched edges: " + idsText(patchedIds) + ". Reachability: " + (reachableAfter ? "YES" : "NO") + ". Re-Attack uses the original budget.";
        refreshView();
    } catch (const std::exception& error) { message = error.what(); }
}

void SimulationUI::reAttack() {
    if (stage != Stage::PATCHED) return;
    try {
        after = hackerSimulation(graph,source,target,initialBudget);
        reachableAfter = Defender(graph).isReachable(source,target);
        hasAfter = true;
        stage = Stage::COMPLETED;
        showBefore = false;
        message = "Re-Attack: " + statusText(after.status) + ". Reset to start a new run.";
        refreshView();
    } catch (const std::exception& error) { message = error.what(); }
}

void SimulationUI::reset() {
    clearResults();
    message = "Reset to original dataset, including its initially blocked edges. JSON was not changed.";
}

bool SimulationUI::button(Rectangle area, const std::string& label, bool enabled) {
    bool hovered = CheckCollisionPointRec(GetMousePosition(),area);
    Color color = enabled ? (hovered ? Color{63,80,105,255} : Color{39,53,74,255}) : Color{28,36,48,255};
    DrawRectangleRounded(area,0.15f,6,color);
    Vector2 size = MeasureTextEx(font,label.c_str(),18,1);
    text(label,area.x+(area.width-size.x)/2,area.y+(area.height-size.y)/2,18,enabled ? WHITE : Color{91,105,123,255});
    return enabled && hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void SimulationUI::textbox(Rectangle area, std::string& value, int id, bool enabled) {
    bool hovered = CheckCollisionPointRec(GetMousePosition(),area);
    if (enabled && hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        activeInput = id;
        selectAll = false;
    }
    DrawRectangleRec(area,background);
    DrawRectangleLinesEx(area,1,activeInput == id ? accent : Color{63,80,105,255});
    if (enabled && activeInput == id) {
        bool control = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
        if (control && IsKeyPressed(KEY_A)) selectAll = true;
        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
            if (id == 2 && stage != Stage::READY) {
                clearResults();
                message = "Budget changed. Run Attack again.";
            }
            if (selectAll) value.clear(); else eraseLastCharacter(value);
            selectAll = false;
        }
        std::string added;
        if (control && IsKeyPressed(KEY_V)) {
            const char* clipboard = GetClipboardText();
            if (clipboard != nullptr) added = clipboard;
        }
        int codepoint = GetCharPressed();
        while (codepoint > 0) {
            if (!control && codepoint >= 32) {
                int length = 0;
                const char* encoded = CodepointToUTF8(codepoint,&length);
                added.append(encoded,static_cast<size_t>(length));
            }
            codepoint = GetCharPressed();
        }
        if (!added.empty()) {
            if (id == 2 && stage != Stage::READY) {
                clearResults();
                message = "Budget changed. Run Attack again.";
            }
            if (selectAll) value.clear();
            for (char character : added) {
                if (character != '\n' && character != '\r' && (id != 2 || (character >= '0' && character <= '9'))) value += character;
            }
            selectAll = false;
            if (id == 3) scroll = 0;
        }
    }
    BeginScissorMode(static_cast<int>(area.x+6),static_cast<int>(area.y),static_cast<int>(area.width-12),static_cast<int>(area.height));
    float width = MeasureTextEx(font,value.c_str(),18,1).x;
    float x = area.x+7;
    if (activeInput == id && width > area.width-18) x -= width-(area.width-18);
    if (selectAll && activeInput == id) DrawRectangleRec({x,area.y+5,width,23},{53,78,112,255});
    text(value,x,area.y+8,18,enabled ? WHITE : muted);
    if (activeInput == id && static_cast<int>(GetTime()*2)%2 == 0) text("|",x+width,area.y+8);
    EndScissorMode();
}

void SimulationUI::drawResults(float y) {
    float width = static_cast<float>(GetScreenWidth());
    DrawRectangleRounded({20,y,width-40,154},0.08f,6,panel);
    float beforeX = 190;
    float afterX = width/2+80;
    text("COMPARISON",34,y+12,17,muted);
    text("BEFORE",beforeX,y+12,17,accent);
    text("AFTER",afterX,y+12,17,accent);
    const char* labels[]{"Status / Cost","Tokens / Reach","Attack path"};
    for (int row=0;row<3;++row) text(labels[row],34,y+39+row*25,16,muted);
    const AttackResult* results[]{&before,&after};
    bool present[]{hasBefore,hasAfter};
    bool reachable[]{reachableBefore,reachableAfter};
    float columns[]{beforeX,afterX};
    for (int column=0;column<2;++column) {
        if (!present[column]) { text("--",columns[column],y+39,17,muted); continue; }
        const AttackResult& result = *results[column];
        std::string cost = result.path.reachable ? std::to_string(result.path.totalCost) : "N/A";
        clippedText(statusText(result.status)+" / "+cost,{columns[column],y+39,width/2-200,22},accent);
        std::string remaining = result.remainingToken ? std::to_string(*result.remainingToken) : "N/A";
        text(remaining+" / "+(reachable[column]?"YES":"NO"),columns[column],y+64,17);
        clippedText(idsText(result.path.nodes," > "),{columns[column],y+89,width/2-200,22});
    }
    std::string defense = "Flow / Cut: ";
    defense += hasCut ? std::to_string(cut.maxFlow)+" / "+std::to_string(cut.minCutCapacity) : "N/A";
    defense += "    Patched IDs: " + idsText(patchedIds);
    clippedText(defense,{34,y+124,width-68,23},muted);
}

void SimulationUI::drawPopup(bool acceptInput) {
    float width = std::min(720.0f,static_cast<float>(GetScreenWidth()-80));
    float height = std::min(540.0f,static_cast<float>(GetScreenHeight()-100));
    Rectangle box{(GetScreenWidth()-width)/2,(GetScreenHeight()-height)/2,width,height};
    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),{0,0,0,190});
    DrawRectangleRounded(box,0.03f,8,panel);
    text(popup == 3 ? "MINIMUM CUT - select edges to patch" : "EXPLORER - choose " + std::string(popup == 1 ? "Source" : "Target"),box.x+20,box.y+18,21,accent);
    if (button({box.x+width-85,box.y+12,65,32},"Close",acceptInput)) { popup=0; activeInput=0; return; }
    if (acceptInput && IsKeyPressed(KEY_ESCAPE)) { popup=0;activeInput=0;return; }
    float listY = box.y+96;
    if (popup != 3) textbox({box.x+20,box.y+58,width-40,32},searchText,3,acceptInput);
    else text("Capacity is defense cost. Unchecking edges may keep S-T reachable.",box.x+20,box.y+62,16,muted);
    std::vector<int> rows;
    if (popup == 3) {
        for (size_t index=0;index<cut.edgeIds.size();++index) rows.push_back(static_cast<int>(index));
    } else {
        const NodeType types[]{NodeType::ENTRY,NodeType::ENDPOINT,NodeType::IDENTITY,NodeType::CRITICAL_SYSTEM,NodeType::TARGET};
        std::string query = lowerText(searchText);
        for (NodeType type : types) {
            for (const Node& node : graph.getNodes()) {
                if (node.getType() == type && lowerText(nodeLabel(node.getID())).find(query) != std::string::npos) rows.push_back(node.getID());
            }
        }
    }
    int visible = static_cast<int>((height-175)/36);
    int maximum = std::max(0,static_cast<int>(rows.size())-visible);
    if (acceptInput) scroll -= static_cast<int>(GetMouseWheelMove()*3);
    scroll = std::max(0,std::min(scroll,maximum));
    for (int row=0;row<visible && row+scroll<static_cast<int>(rows.size());++row) {
        int value = rows[row+scroll];
        Rectangle area{box.x+20,listY+row*36,width-40,32};
        std::string label;
        if (popup == 3) {
            const Edge* edge = graph.getEdge(cut.edgeIds[value]);
            label = std::string(patchSelection[value]?"[x] ":"[ ] ") + "#"+std::to_string(edge->getID())+"  "+std::to_string(edge->getFrom())+" -> "+std::to_string(edge->getTo())+"  "+edge->getRelation()+"  cap="+std::to_string(edge->getCapacity());
        } else {
            const Node* node = graph.getNode(value);
            label = nodeTypeToString(node->getType())+"  |  "+nodeLabel(value);
        }
        bool clicked = acceptInput && CheckCollisionPointRec(GetMousePosition(),area) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        DrawRectangleRec(area,CheckCollisionPointRec(GetMousePosition(),area)?Color{48,65,89,255}:background);
        clippedText(label,{area.x+8,area.y+7,area.width-16,22});
        if (clicked) {
            if (popup == 3) patchSelection[value] = !patchSelection[value];
            else { chooseNode(value,popup == 1);return; }
        }
    }
    text(std::to_string(rows.size())+" items. Mouse wheel to scroll.",box.x+20,box.y+height-63,16,muted);
    if (popup == 3) {
        std::int64_t selectedCapacity = 0;
        int selectedCount = 0;
        for (size_t index=0;index<patchSelection.size();++index) {
            if (patchSelection[index]) {
                ++selectedCount;
                selectedCapacity += graph.getEdge(cut.edgeIds[index])->getCapacity();
            }
        }
        text("Selected: "+std::to_string(selectedCount)+" edges / capacity "+std::to_string(selectedCapacity),box.x+20,box.y+height-90,16,accent);
        if (button({box.x+width-170,box.y+height-51,150,34},"Apply Patch",acceptInput && selectedCount > 0)) applyPatch();
    }
}

void SimulationUI::frame() {
    float width = static_cast<float>(GetScreenWidth());
    float height = static_cast<float>(GetScreenHeight());
    bool modal = popup != 0;
    if (!modal && IsFileDropped()) {
        FilePathList files = LoadDroppedFiles();
        if (files.count > 0) importDataset(files.paths[0]);
        UnloadDroppedFiles(files);
    }
    view.setBounds({20,168,width-40,height-454});
    if (!modal) {
        view.handleInput();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(),view.getBounds())) {
            selectedNode = view.nodeAt(GetMousePosition());
            activeInput = 0;
        }
        if (activeInput == 0 && selectedNode != -1) {
            if (IsKeyPressed(KEY_S)) chooseNode(selectedNode,true);
            if (IsKeyPressed(KEY_T)) chooseNode(selectedNode,false);
        }
        if (IsKeyPressed(KEY_ESCAPE)) activeInput = 0;
    }
    BeginDrawing();
    ClearBackground(background);
    text("ATTACK GRAPH",20,14,25,accent);
    text("raylib / C++17   |   Wheel: zoom   Drag node: move   Empty/middle drag: pan   S/T: assign",240,20,16,muted);
    textbox({20,48,width-186,34},datasetPath,1,!modal);
    if (button({width-156,48,136,34},"Import JSON",!modal)) importDataset(datasetPath);
    float nodeWidth = (width-256)/2;
    text("SOURCE",20,91,14,muted);
    text("TARGET",30+nodeWidth,91,14,muted);
    text("TOKEN BUDGET",width-216,91,14,muted);
    Rectangle sourceBox{20,110,nodeWidth,34},targetBox{30+nodeWidth,110,nodeWidth,34};
    DrawRectangleRec(sourceBox,panel);DrawRectangleRec(targetBox,panel);
    clippedText(nodeLabel(source),{sourceBox.x+10,118,nodeWidth-20,22});
    clippedText(nodeLabel(target),{targetBox.x+10,118,nodeWidth-20,22});
    if (!modal && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(GetMousePosition(),sourceBox)) { popup=1;scroll=0;searchText.clear();activeInput=3; }
        if (CheckCollisionPointRec(GetMousePosition(),targetBox)) { popup=2;scroll=0;searchText.clear();activeInput=3; }
    }
    textbox({width-216,110,196,34},budgetText,2,!modal);
    DrawRectangleRounded(view.getBounds(),0.01f,6,panel);
    view.draw(font,showLabels,dimEdges,selectedNode);
    float toolbar = height-276;
    if (button({20,toolbar,128,30},showBefore?"View: Before":"View: Current",!modal && hasBefore)) { showBefore=!showBefore;refreshView(); }
    if (button({158,toolbar,112,30},showLabels?"Labels: On":"Labels: Off",!modal)) showLabels=!showLabels;
    if (button({280,toolbar,128,30},dimEdges?"Focus: On":"Focus: Off",!modal)) dimEdges=!dimEdges;
    if (button({418,toolbar,70,30},"Fit",!modal)) view.fit();
    clippedText(selectedNode == -1 ? "Orange: attack   Purple: cut   Red dashed: blocked" : "Selected: "+nodeLabel(selectedNode)+" | S/T keys assign",{505,toolbar+6,width-525,22},muted);
    drawResults(height-236);
    float actionWidth = (width-80)/5;
    float y = height-42;
    if (button({20,y,actionWidth,32},"Attack",!modal && stage == Stage::READY && source != -1 && target != -1)) attack();
    if (button({30+actionWidth,y,actionWidth,32},"Defend",!modal && stage == Stage::ATTACKED)) defend();
    if (button({40+actionWidth*2,y,actionWidth,32},"Review Cut",!modal && stage == Stage::CUT_PROPOSED)) { popup=3;scroll=0;activeInput=0; }
    if (button({50+actionWidth*3,y,actionWidth,32},"Re-Attack",!modal && stage == Stage::PATCHED)) reAttack();
    if (button({60+actionWidth*4,y,actionWidth,32},"Reset",!modal && !graph.getNodes().empty())) reset();
    // Long messages remain readable in a tooltip without covering the graph.
    clippedText(message,{20,height-74,width-40,24},muted);
    if (!modal && CheckCollisionPointRec(GetMousePosition(),{20,height-74,width-40,24})) {
        DrawRectangle(20,static_cast<int>(height-168),static_cast<int>(width-40),90,panel);
        std::string line;
        float lineY = height-162;
        for (char character : message) {
            line += character;
            if (MeasureTextEx(font,line.c_str(),17,1).x > width-90 && character == ' ') {
                text(line,30,lineY,17,muted);line.clear();lineY+=21;
            }
        }
        text(line,30,lineY,17,muted);
    }
    if (popup != 0) drawPopup(modal);
    EndDrawing();
}
