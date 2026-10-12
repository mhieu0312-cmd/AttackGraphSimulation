#include "GraphView.h"
#include <algorithm>
#include <cmath>

namespace {
bool includes(const std::vector<int>& ids, int id) {
    for (int value : ids) if (value == id) return true;
    return false;
}
Color nodeColor(NodeType type) {
    switch (type) {
        case NodeType::ENTRY: return {67,135,204,255};
        case NodeType::ENDPOINT: return {115,134,158,255};
        case NodeType::IDENTITY: return {136,116,178,255};
        case NodeType::CRITICAL_SYSTEM: return {52,140,154,255};
        case NodeType::TARGET: return {54,157,115,255};
    }
    return GRAY;
}
Vector2 bezier(Vector2 start, Vector2 control1, Vector2 control2, Vector2 end, float t) {
    float u = 1 - t;
    return {u*u*u*start.x + 3*u*u*t*control1.x + 3*u*t*t*control2.x + t*t*t*end.x,
            u*u*u*start.y + 3*u*u*t*control1.y + 3*u*t*t*control2.y + t*t*t*end.y};
}
void centered(Font font, const std::string& text, Vector2 point, float size, Color color) {
    Vector2 extent = MeasureTextEx(font, text.c_str(), size, 1);
    DrawTextEx(font, text.c_str(), {point.x - extent.x / 2, point.y}, size, 1, color);
}
}

void GraphView::setGraph(const Graph& graph) {
    nodes.clear();
    edges = graph.getEdges();
    attackIds.clear();
    cutIds.clear();
    const NodeType types[]{NodeType::ENTRY, NodeType::ENDPOINT, NodeType::IDENTITY,
                           NodeType::CRITICAL_SYSTEM, NodeType::TARGET};
    for (int layer = 0; layer < 5; ++layer) {
        std::vector<const Node*> group;
        for (const Node& node : graph.getNodes()) {
            if (node.getType() == types[layer]) group.push_back(&node);
        }
        for (size_t index = 0; index < group.size(); ++index) {
            float y = (static_cast<float>(index) - static_cast<float>(group.size()-1)/2) * 105;
            nodes.push_back({group[index]->getID(), types[layer], group[index]->getName(), {layer*230.0f,y}});
        }
    }
    fit();
}

void GraphView::updateGraph(const Graph& graph, const std::vector<int>& attack, const std::vector<int>& cut) {
    attackIds = attack;
    cutIds = cut;
    for (Edge& edge : edges) {
        const Edge* current = graph.getEdge(edge.getID());
        if (current != nullptr) edge.setBlocked(current->isBlocked());
    }
}

Vector2 GraphView::positionOf(int id) const {
    for (const NodePosition& node : nodes) if (node.id == id) return node.position;
    return {0,0};
}

void GraphView::setBounds(Rectangle area) {
    if (bounds.width > 0 && bounds.height > 0) {
        camera.zoom *= std::min(area.width / bounds.width, area.height / bounds.height);
    }
    bounds = area;
    camera.offset = {area.x + area.width/2, area.y + area.height/2};
}

void GraphView::fit() {
    float height = 120;
    for (const NodePosition& node : nodes) height = std::max(height, std::abs(node.position.y)*2 + 150);
    camera.target = {460,0};
    camera.zoom = std::min(bounds.width/1150, bounds.height/height);
    camera.zoom = std::max(0.01f, camera.zoom);
}

int GraphView::nodeAt(Vector2 mouse) const {
    if (!CheckCollisionPointRec(mouse, bounds)) return -1;
    Vector2 world = GetScreenToWorld2D(mouse, camera);
    for (const NodePosition& node : nodes) {
        if (CheckCollisionPointCircle(world, node.position, 24)) return node.id;
    }
    return -1;
}

void GraphView::handleInput() {
    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, bounds)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector2 before = GetScreenToWorld2D(mouse, camera);
            camera.zoom = std::max(0.02f, std::min(4.0f, camera.zoom * (wheel > 0 ? 1.15f : 1/1.15f)));
            Vector2 after = GetScreenToWorld2D(mouse, camera);
            camera.target.x += before.x - after.x;
            camera.target.y += before.y - after.y;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) ||
            (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && nodeAt(mouse) == -1)) panning = true;
    }
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) panning = false;
    if (panning) {
        Vector2 movement = GetMouseDelta();
        camera.target.x -= movement.x/camera.zoom;
        camera.target.y -= movement.y/camera.zoom;
    }
}

void GraphView::draw(Font font, bool edgeLabels, bool dimOthers, int selectedNode) const {
    BeginScissorMode(static_cast<int>(bounds.x), static_cast<int>(bounds.y),
                     static_cast<int>(bounds.width), static_cast<int>(bounds.height));
    BeginMode2D(camera);
    const char* layers[]{"ENTRY","ENDPOINT","IDENTITY","CRITICAL SYSTEM","TARGET"};
    float top = -50;
    for (const NodePosition& node : nodes) top = std::min(top, node.position.y-65);
    for (int layer = 0; layer < 5; ++layer) centered(font, layers[layer], {layer*230.0f,top}, 17, {160,176,197,255});
    for (const Edge& edge : edges) {
        Vector2 from = positionOf(edge.getFrom()), to = positionOf(edge.getTo());
        int total = 0, ordinal = 0;
        for (const Edge& other : edges) {
            if ((other.getFrom()==edge.getFrom() && other.getTo()==edge.getTo()) ||
                (other.getFrom()==edge.getTo() && other.getTo()==edge.getFrom())) {
                if (other.getID()==edge.getID()) ordinal=total;
                ++total;
            }
        }
        float offset = (ordinal - (total-1)/2.0f)*35;
        if (edge.getFrom()>edge.getTo()) offset=-offset;
        Vector2 start, end, control1, control2;
        if (edge.getFrom()==edge.getTo()) {
            start={from.x+17,from.y-17};end={from.x-17,from.y-17};
            control1={from.x+95+ordinal*25,from.y-110};control2={from.x-65,from.y-110-ordinal*25};
        } else if (std::abs(from.x-to.x)<1) {
            start={from.x+24,from.y};end={to.x+24,to.y};
            control1={from.x+100+ordinal*30,from.y};control2={control1.x,to.y};
        } else {
            float direction=to.x>from.x?1.0f:-1.0f;
            start={from.x+direction*24,from.y};end={to.x-direction*24,to.y};
            float bend=std::abs(end.x-start.x)/2;
            if (std::abs(to.x-from.x)>250) offset-=75;
            control1={start.x+direction*bend,start.y+offset};control2={end.x-direction*bend,end.y+offset};
        }
        bool attack=includes(attackIds,edge.getID()), cut=includes(cutIds,edge.getID());
        Color color{105,123,146,255};float thickness=2;
        if (attack) { color={245,176,77,255};thickness=4; }
        if (cut) { color={206,122,245,255};thickness=4; }
        if (edge.isBlocked()) { color={240,120,128,255};thickness=3; }
        if (dimOthers && (!attackIds.empty() || !cutIds.empty()) && !attack && !cut && !edge.isBlocked()) color.a=45;
        Vector2 previous=start;
        for (int segment=1;segment<=32;++segment) {
            Vector2 point=bezier(start,control1,control2,end,segment/32.0f);
            if (!edge.isBlocked() || (segment/3)%2==0) DrawLineEx(previous,point,thickness,color);
            previous=point;
        }
        Vector2 before=bezier(start,control1,control2,end,0.98f);
        float angle=std::atan2(end.y-before.y,end.x-before.x);
        Vector2 left{end.x-12*std::cos(angle)+5*std::sin(angle),end.y-12*std::sin(angle)-5*std::cos(angle)};
        Vector2 right{end.x-12*std::cos(angle)-5*std::sin(angle),end.y-12*std::sin(angle)+5*std::cos(angle)};
        DrawTriangle(end,left,right,color);
        if (edgeLabels) {
            std::string text=edge.getRelation()+" w"+std::to_string(edge.getWeight())+"/c"+std::to_string(edge.getCapacity());
            Vector2 middle=bezier(start,control1,control2,end,0.5f);middle.y-=20;
            centered(font,text,middle,15,color);
        }
    }
    for (const NodePosition& node : nodes) {
        bool onPath=false;
        for (const Edge& edge : edges) {
            if (includes(attackIds,edge.getID()) && (edge.getFrom()==node.id || edge.getTo()==node.id)) onPath=true;
        }
        DrawCircleV(node.position,24,onPath?Color{204,139,52,255}:nodeColor(node.type));
        DrawCircleLinesV(node.position,24,{224,233,245,255});
        if (node.id==selectedNode) DrawCircleLinesV(node.position,29,{245,176,77,255});
        const std::string id=std::to_string(node.id);
        float size=id.size()>5?11.0f:19.0f;
        centered(font,id,{node.position.x,node.position.y-size/2},size,WHITE);
        std::string name=node.name;
        while (MeasureTextEx(font,name.c_str(),16,1).x>190 && !name.empty()) {
            size_t end=name.size()-1;
            while (end>0 && (static_cast<unsigned char>(name[end])&0xC0)==0x80) --end;
            name.erase(end);
        }
        if (name!=node.name) name+="...";
        centered(font,name,{node.position.x,node.position.y+32},16,{217,226,237,255});
    }
    EndMode2D();
    EndScissorMode();
}
