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
void drawSegment(Vector2 start, Vector2 end, float thickness, Color color, bool blocked) {
    if (!blocked) {
        DrawLineEx(start, end, thickness, color);
        return;
    }
    float horizontal = end.x - start.x;
    float vertical = end.y - start.y;
    float length = std::sqrt(horizontal * horizontal + vertical * vertical);
    if (length < 0.01f) return;
    for (float distance = 0; distance < length; distance += 14) {
        float finish = std::min(distance + 8, length);
        Vector2 first{start.x + horizontal * distance / length, start.y + vertical * distance / length};
        Vector2 second{start.x + horizontal * finish / length, start.y + vertical * finish / length};
        DrawLineEx(first, second, thickness, color);
    }
}
void centered(Font font, const std::string& text, Vector2 point, float size, Color color) {
    Vector2 extent = MeasureTextEx(font, text.c_str(), size, 1);
    DrawTextEx(font, text.c_str(), {point.x - extent.x / 2, point.y}, size, 1, color);
}
}

void GraphView::setGraph(const Graph& graph) {
    nodes.clear();
    draggedNode = -1;
    panning = false;
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
    float left = -115, right = 1035;
    float top = -60, bottom = 60;
    for (const NodePosition& node : nodes) {
        left = std::min(left, node.position.x - 115);
        right = std::max(right, node.position.x + 115);
        top = std::min(top, node.position.y - 75);
        bottom = std::max(bottom, node.position.y + 75);
    }
    camera.target = {(left + right) / 2, (top + bottom) / 2};
    camera.zoom = std::min(bounds.width / (right - left), bounds.height / (bottom - top));
    camera.zoom = std::max(0.01f, camera.zoom);
}

int GraphView::nodeAt(Vector2 mouse) const {
    if (!CheckCollisionPointRec(mouse, bounds)) return -1;
    Vector2 world = GetScreenToWorld2D(mouse, camera);
    for (int index = static_cast<int>(nodes.size()) - 1; index >= 0; --index) {
        if (CheckCollisionPointCircle(world, nodes[index].position, 24)) return nodes[index].id;
    }
    return -1;
}

void GraphView::handleInput() {
    Vector2 mouse = GetMousePosition();
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) draggedNode = -1;
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) panning = false;
    if (CheckCollisionPointRec(mouse, bounds)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector2 before = GetScreenToWorld2D(mouse, camera);
            camera.zoom = std::max(0.02f, std::min(4.0f, camera.zoom * (wheel > 0 ? 1.15f : 1/1.15f)));
            Vector2 after = GetScreenToWorld2D(mouse, camera);
            camera.target.x += before.x - after.x;
            camera.target.y += before.y - after.y;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
            panning = true;
            draggedNode = -1;
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            draggedNode = nodeAt(mouse);
            panning = draggedNode == -1;
            if (draggedNode != -1) {
                Vector2 position = positionOf(draggedNode);
                Vector2 world = GetScreenToWorld2D(mouse, camera);
                dragOffset = {position.x - world.x, position.y - world.y};
            }
        }
    }
    if (panning) {
        Vector2 movement = GetMouseDelta();
        camera.target.x -= movement.x/camera.zoom;
        camera.target.y -= movement.y/camera.zoom;
    } else if (draggedNode != -1) {
        Vector2 world = GetScreenToWorld2D(mouse, camera);
        for (NodePosition& node : nodes) {
            if (node.id == draggedNode) {
                node.position = {world.x + dragOffset.x, world.y + dragOffset.y};
                break;
            }
        }
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
        Vector2 difference{to.x - from.x, to.y - from.y};
        float length = std::sqrt(difference.x * difference.x + difference.y * difference.y);
        std::vector<Vector2> points;
        if (edge.getFrom() == edge.getTo() || length < 48) {
            float extent = 65 + ordinal * 15.0f;
            points = {{from.x + 17, from.y - 17}, {from.x + extent, from.y - extent},
                      {to.x - extent, to.y - extent}, {to.x - 17, to.y - 17}};
        } else {
            Vector2 direction{difference.x / length, difference.y / length};
            Vector2 normal{-direction.y, direction.x};
            if (edge.getFrom() > edge.getTo()) normal = {-normal.x, -normal.y};
            float offset = 0;
            if (total > 1) offset = -18 + ordinal * 36.0f / (total - 1);
            float border = std::sqrt(24 * 24 - offset * offset);
            Vector2 start{from.x + normal.x * offset + direction.x * border,
                          from.y + normal.y * offset + direction.y * border};
            Vector2 end{to.x + normal.x * offset - direction.x * border,
                        to.y + normal.y * offset - direction.y * border};
            points = {start, end};
        }
        bool attack=includes(attackIds,edge.getID()), cut=includes(cutIds,edge.getID());
        Color color{105,123,146,255};float thickness=2;
        if (attack) { color={245,176,77,255};thickness=4; }
        if (cut) { color={206,122,245,255};thickness=4; }
        if (edge.isBlocked()) { color={240,120,128,255};thickness=3; }
        if (dimOthers && (!attackIds.empty() || !cutIds.empty()) && !attack && !cut && !edge.isBlocked()) color.a=45;
        for (size_t index = 1; index < points.size(); ++index) {
            drawSegment(points[index - 1], points[index], thickness, color, edge.isBlocked());
        }
        Vector2 end = points.back();
        Vector2 previous = points[points.size() - 2];
        float angle = std::atan2(end.y - previous.y, end.x - previous.x);
        Vector2 left{end.x-12*std::cos(angle)+5*std::sin(angle),end.y-12*std::sin(angle)-5*std::cos(angle)};
        Vector2 right{end.x-12*std::cos(angle)-5*std::sin(angle),end.y-12*std::sin(angle)+5*std::cos(angle)};
        DrawTriangle(end,left,right,color);
        if (edgeLabels) {
            std::string text=edge.getRelation()+" w"+std::to_string(edge.getWeight())+"/c"+std::to_string(edge.getCapacity());
            Vector2 middle;
            if (points.size() == 2) middle = {(points[0].x + points[1].x) / 2, (points[0].y + points[1].y) / 2};
            else middle = {(points[1].x + points[2].x) / 2, (points[1].y + points[2].y) / 2};
            middle.y -= 20;
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
