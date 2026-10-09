#include "Graph.h"
#include "LoadDataset.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <functional>
using nlohmann::json;

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
void rejected(const std::function<void()>& action) {
    try { action(); } catch (const std::invalid_argument&) { return; }
    throw std::runtime_error("Expected invalid_argument");
}
bool warning(const Graph& g, const std::string& text) {
    for (const auto& s : g.validationWarnings())
        if (s.find(text) != std::string::npos) return true;
    return false;
}
Graph sample() {
    Graph g;
    g.addNode(Node(10,"Entry",NodeType::ENTRY,0));
    g.addNode(Node(40,"PC",NodeType::ENDPOINT,0));
    g.addNode(Node(90,"Target",NodeType::TARGET,0));
    g.addEdge(Edge(10,40,1,"HasSession",false,1,1));
    g.addEdge(Edge(40,90,1,"AccessTo",false,2,1));
    return g;
}
json document() {
    return json::parse(R"({"nodes":[{"id":10,"name":"Entry","type":"ENTRY","assets":0},{"id":90,"name":"Target","type":"TARGET","assets":0}],"edges":[{"id":1,"from":10,"to":90,"weight":1,"capacity":1,"relation":"AccessTo","blocked":false}]})");
}
Graph parse(const json& j) { std::istringstream in(j.dump()); return loadDataset(in); }
void dataset() {
    auto g = loadDataset(std::string(DATASET_PATH));
    check(g.statistics().nodes == 18 && g.statistics().edges == 24,"Real dataset counts");
    check(g.validationWarnings().empty(),"Real dataset warnings");
    auto order=g.dfs(36);
    check(std::find(order.begin(),order.end(),3)!=order.end(),"Real target reachable");
    check(parse(document()).validationWarnings().empty(),"Valid JSON");
}
void directErrors() {
    auto g=sample();
    rejected([&]{g.addNode(Node(10,"Duplicate",NodeType::ENTRY,0));});
    rejected([&]{g.addNode(Node(-1,"Bad",NodeType::ENTRY,0));});
    for (const auto& name : {"", " \t\r\n"})
        rejected([&]{g.addNode(Node(100,name,NodeType::ENTRY,0));});
    rejected([&]{g.addNode(Node(100,"Bad",static_cast<NodeType>(99),0));});
    for (auto edge : {Edge(10,40,1,"AccessTo",false,1,1),
                     Edge(10,40,1,"AccessTo",false,-1,1),
                     Edge(999,40,1,"AccessTo",false,3,1),
                     Edge(10,999,1,"AccessTo",false,3,1),
                     Edge(10,40,-1,"AccessTo",false,3,1),
                     Edge(10,40,1,"AccessTo",false,3,-1),
                     Edge(10,40,1,"bad",false,3,1)})
        rejected([&]{g.addEdge(edge);});
    check(g.statistics().nodes==3 && g.statistics().edges==2,"Rejected additions mutated graph");
    int id=3;
    for (auto relation : {"HasSession","AdminTo","MemberOf","AccessTo"})
        g.addEdge(Edge(10,40,0,relation,false,id++,0));
    int nodeId=100;
    for (auto type : {NodeType::ENTRY,NodeType::ENDPOINT,NodeType::IDENTITY,
                      NodeType::CRITICAL_SYSTEM,NodeType::TARGET})
        g.addNode(Node(nodeId++,"valid",type,0));
}
void jsonErrors() {
    auto base=document();
    auto j=base; j["nodes"].push_back(j["nodes"][0]); rejected([&]{parse(j);});
    j=base; j["edges"].push_back(j["edges"][0]); rejected([&]{parse(j);});
    for (auto field : {"id","from","to","weight","capacity"}) {
        for (auto bad : {json(-1),json(1.5),json(4294967296ULL),json("1"),json(true),json(nullptr)}) {
            j=base; j["edges"][0][field]=bad; rejected([&]{parse(j);});
        }
    }
    for (auto bad : {json(-1),json(1.5),json(4294967296ULL),json("1"),json(true)}) {
        j=base; j["nodes"][0]["id"]=bad; rejected([&]{parse(j);});
    }
    for (auto name : {"", " \t\n"}) {j=base;j["nodes"][0]["name"]=name;rejected([&]{parse(j);});}
    j=base;j["nodes"][0]["type"]="UNKNOWN";rejected([&]{parse(j);});
    j=base;j["edges"][0]["relation"]="accessTo";rejected([&]{parse(j);});
    j=base;j["edges"][0]["from"]=999;rejected([&]{parse(j);});
    j=base;j["edges"][0]["to"]=999;rejected([&]{parse(j);});
    j=base;j["edges"][0]["blocked"]=1;rejected([&]{parse(j);});
    j=base;j["edges"][0].erase("capacity");rejected([&]{parse(j);});
    j=base;j["nodes"]=json::object();rejected([&]{parse(j);});
    j=base;j["edges"]=json::object();rejected([&]{parse(j);});
    rejected([]{std::istringstream input("{broken");loadDataset(input);});
}
void warnings() {
    Graph empty; check(warning(empty,"Graph rong"),"Empty warning");
    Graph g;g.addNode(Node(7,"isolated",NodeType::ENDPOINT,0));
    check(warning(g,"ENTRY")&&warning(g,"TARGET")&&warning(g,"co lap ID 7"),"Missing type/isolation warnings");
    g=sample();g.blockEdge(2);
    check(warning(g,"TARGET khong reachable"),"Blocked target warning");
    check(!warning(g,"co lap"),"Blocked edges still count for topology isolation");
    g.addNode(Node(99,"second entry",NodeType::ENTRY,0));
    g.addEdge(Edge(99,90,1,"AccessTo",false,3,1));
    check(!warning(g,"TARGET khong reachable"),"Reachable from any entry");
    Graph reversed;
    reversed.addNode(Node(1,"entry",NodeType::ENTRY,0));
    reversed.addNode(Node(2,"target",NodeType::TARGET,0));
    reversed.addEdge(Edge(2,1,1,"AccessTo",false,1,1));
    check(warning(reversed,"TARGET khong reachable"),"Directed reachability warning");
}
void dfs() {
    auto g=sample();
    g.addNode(Node(200,"branch",NodeType::ENDPOINT,0));
    g.addNode(Node(999,"isolated",NodeType::ENDPOINT,0));
    g.addEdge(Edge(10,200,1,"AccessTo",false,3,1));
    g.addEdge(Edge(90,10,1,"AccessTo",false,4,1)); // cycle
    g.addEdge(Edge(40,40,1,"AccessTo",false,5,1)); // self loop
    g.addEdge(Edge(10,40,1,"AccessTo",false,6,1)); // parallel
    const Graph& view=g;
    check(view.dfs(10)==std::vector<int>({10,40,90,200}),"DFS order/cycle/parallel/self-loop");
    check(view.dfs(999)==std::vector<int>({999}),"Isolated DFS");
    rejected([&]{view.dfs(123);});
    g.blockEdge(1);g.blockEdge(6);
    const auto before=g.statistics();
    const auto nodes=g.getNodes();const auto edges=g.getEdges();
    check(view.dfs(10)==std::vector<int>({10,200}),"DFS blocked edges");
    check(view.dfs(200)==std::vector<int>({200}),"DFS direction");
    view.validationWarnings();
    const auto after=g.statistics();
    check(before.nodes==after.nodes&&before.edges==after.edges&&before.blockedEdges==after.blockedEdges,"DFS statistics changed");
    for(size_t i=0;i<edges.size();++i) {
        const auto& a=edges[i];const auto& b=g.getEdges()[i];
        check(a.getID()==b.getID()&&a.getFrom()==b.getFrom()&&a.getTo()==b.getTo()&&
              a.getWeight()==b.getWeight()&&a.getCapacity()==b.getCapacity()&&
              a.getRelation()==b.getRelation()&&a.isBlocked()==b.isBlocked(),"Edge mutated");
    }
    for(size_t i=0;i<nodes.size();++i) {
        const auto& a=nodes[i];const auto& b=g.getNodes()[i];
        check(a.getID()==b.getID()&&a.getName()==b.getName()&&a.getType()==b.getType()&&a.getAssets()==b.getAssets(),"Node mutated");
    }
    g.unblockEdge(6);
    check(view.dfs(10)==std::vector<int>({10,200,40,90}),"DFS unblock/order");
}
int main(int argc,char** argv) {
    try {
        const std::string group=argc>1?argv[1]:"all";
        bool ran=false;
        for (const auto& test : std::vector<std::pair<std::string,std::function<void()>>>{
                {"dataset",dataset},{"direct_errors",directErrors},{"json_errors",jsonErrors},
                {"warnings",warnings},{"dfs",dfs}}) {
            if(group=="all"||group==test.first) {test.second();ran=true;std::cout<<test.first<<" PASS\n";}
        }
        check(ran,"Unknown test group");return 0;
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
