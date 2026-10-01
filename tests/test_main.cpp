#include "LoadDataset.h"
#include "Simulation.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>

namespace {
int passed = 0, failed = 0;
#define CHECK(condition) do { if (!(condition)) throw std::runtime_error(std::string(#condition) + " at line " + std::to_string(__LINE__)); } while (false)
void test(const std::string& name, const std::function<void()>& body) {
    try { body(); ++passed; std::cout << "PASS | " << name << '\n'; }
    catch (const std::exception& e) { ++failed; std::cout << "FAIL | " << name << " | " << e.what() << '\n'; }
}
void invalid(const std::function<void()>& body) {
    bool caught = false;
    try { body(); } catch (const std::invalid_argument&) { caught = true; }
    CHECK(caught);
}
Graph nodes(std::initializer_list<int> ids) {
    Graph g;
    for (int id : ids) g.addNode(Node(id, "N" + std::to_string(id), NodeType::ENDPOINT, 0));
    return g;
}
void edge(Graph& g, int id, int u, int v, int weight = 1,
          bool blocked = false, std::optional<int> capacity = std::nullopt) {
    g.addEdge(Edge(u, v, weight, "AccessTo", blocked, id, capacity));
}
Graph demo() { return loadDataset("tests/fixtures/p2_demo.json"); }
Graph parse(const std::string& text) { std::istringstream in(text); return loadDataset(in); }
std::string dataset(const std::string& edges) {
    return R"({"nodes":[{"id":10,"name":"S","type":"ENTRY","assets":0},{"id":90,"name":"T","type":"TARGET","assets":7}],"edges":)" + edges + "}";
}
std::string row(const std::string& weight = "0", const std::string& extra = "") {
    return R"({"id":7,"from":10,"to":90,"weight":)" + weight + R"(,"relation":"AccessTo")" + extra + "}";
}
void graphTests() {
    test("G01 duplicate/negative nodes rejected; graph unchanged", [] {
        auto g = nodes({10,90}); invalid([&]{ g.addNode(Node(10,"x",NodeType::ENTRY,0)); });
        invalid([&]{ g.addNode(Node(-1,"x",NodeType::ENTRY,0)); }); CHECK(g.getNodes().size()==2);
    });
    test("G02 invalid fields and dangling edges rejected", [] {
        auto g = nodes({10,90}); invalid([&]{ edge(g,1,10,999); });
        invalid([&]{ edge(g,1,10,90,-1); }); invalid([&]{ edge(g,1,10,90,0,false,-1); });
        invalid([&]{ g.addEdge(Edge(10,90,1,"TYPO",false,1)); });
        invalid([&]{ g.addNode(Node(5,"  ",NodeType::ENTRY,0)); });
        invalid([&]{ g.addNode(Node(5,"x",static_cast<NodeType>(99),0)); });
        invalid([&]{ g.addNode(Node(5,"x",NodeType::ENTRY,-1)); }); CHECK(g.getEdges().empty());
    });
    test("G03 explicit ID, legacy auto ID and duplicate ID", [] {
        auto g=nodes({10,90}); edge(g,0,10,90); g.addEdge(Edge(10,90,0,"HasSession",false));
        CHECK(g.getEdge(1)); invalid([&]{ edge(g,0,90,10); }); CHECK(g.getEdges().size()==2);
    });
    test("G04 directed adjacency honors blocked and unblock", [] {
        auto g=nodes({10,90}); edge(g,7,10,90,0); CHECK(g.getNeighbor(90).empty());
        CHECK(g.getNeighbor(10)==std::vector<int>{90}); g.blockEdge(7);
        CHECK(g.getNeighbor(10).empty()); CHECK(g.getOutGoingEdge(10).empty());
        g.unblockEdge(7); CHECK(g.getOutGoingEdge(10).size()==1); CHECK(g.getEdges().size()==1);
    });
    test("G05 missing node/edge query contracts", [] {
        auto g=nodes({10}); CHECK(!g.getNode(9)); CHECK(!g.getEdge(9));
        invalid([&]{g.getNeighbor(9);}); invalid([&]{g.getOutGoingEdge(9);});
        invalid([&]{g.isBlocked(9);}); invalid([&]{g.blockEdge(9);}); invalid([&]{g.unblockEdge(9);});
        invalid([&]{g.bfs(9);}); invalid([&]{g.dfs(9);});
    });
    test("G06 batch patch validates all IDs before mutating; idempotent", [] {
        auto g=demo(); invalid([&]{g.blockEdges({0,999});}); CHECK(!g.isBlocked(0));
        g.blockEdges({0,0,2}); CHECK(g.isBlocked(0)&&g.isBlocked(2)); CHECK(g.getEdges().size()==4);
        g.blockEdges({}); g.blockEdge(0); CHECK(g.statistics().blockedEdges==2);
    });
    test("G07 BFS/DFS handle cycles, parallel edges, isolated node", [] {
        auto g=nodes({10,20,30,40}); edge(g,1,10,20);edge(g,2,20,10);edge(g,3,20,30);
        edge(g,4,10,20);edge(g,5,10,40,1,true);
        CHECK(g.bfs(10)==std::vector<int>({10,20,30})); CHECK(g.dfs(10)==std::vector<int>({10,20,30}));
        CHECK(g.bfs(40)==std::vector<int>({40})); CHECK(g.getNeighbor(10).size()==1);
        CHECK(g.getOutGoingEdge(10).size()==2);
    });
    test("G08 statistics count active edge degrees including parallel/self loop", [] {
        auto g=nodes({10,20});edge(g,1,10,20);edge(g,2,10,20);edge(g,3,20,20);edge(g,4,20,10,1,true);
        auto s=g.statistics();CHECK(s.nodes==2&&s.edges==4&&s.activeEdges==3&&s.blockedEdges==1);
        CHECK(s.outDegree.at(10)==2&&s.inDegree.at(20)==3&&s.outDegree.at(20)==1);
        CHECK(s.relations.at("AccessTo")==4);
    });
    test("G09 exact edge-ID path validation; blocked/wrong order/missing rejected", [] {
        auto g=demo();auto p=g.validatePath(0,3,{0,1,2});CHECK(p.valid&&p.cost==13);
        CHECK(!g.validatePath(0,3,{1,0,2}).valid);CHECK(!g.validatePath(0,3,{0}).valid);
        CHECK(!g.validatePath(0,3,{999}).valid);CHECK(!g.validatePath(999,3,{}).valid);
        CHECK(g.validatePath(0,0,{}).valid);CHECK(!g.validatePath(0,3,{}).valid);
        g.blockEdge(2);CHECK(!g.validatePath(0,3,{0,1,2}).valid);
    });
    test("G10 canMove handles zero/equal/insufficient/missing/blocked tokens", [] {
        auto g=nodes({1,2});edge(g,4,1,2,0);edge(g,5,1,2,4);
        CHECK(g.canMove(4,0));CHECK(!g.canMove(4,-1));CHECK(g.canMove(5,4));
        CHECK(!g.canMove(5,3));CHECK(!g.canMove(99,100));g.blockEdge(4);CHECK(!g.canMove(4,100));
    });
    test("G11 Graph copy independent; IDs valid after vector growth", [] {
        auto g=demo();auto copy=g;copy.blockEdge(0);CHECK(!g.isBlocked(0));
        for(int i=4;i<100;++i){g.addNode(Node(i,"x",NodeType::ENDPOINT,0));edge(g,i,0,i);}
        CHECK(g.getNode(0)->getName()=="Entry");CHECK(g.getEdge(0)->getTo()==1);
        CHECK(g.validatePath(0,3,{0,1,2}).valid);
    });
    test("G12 validation warnings report empty/missing capacity/unreachable targets", [] {
        Graph empty;CHECK(!empty.validationWarnings().empty());auto g=demo();
        CHECK(g.validationWarnings().size()==1);g.blockEdge(0);auto warnings=g.validationWarnings();
        CHECK(std::any_of(warnings.begin(),warnings.end(),[](const auto& s){return s.find("Target khong reachable")!=std::string::npos;}));
    });
}
void loaderTests() {
    test("L01 sparse node/edge IDs, weight zero, independent capacity preserved", [] {
        auto g=parse(dataset("["+row("0",",\"capacity\":9")+"]"));
        CHECK(g.getNode(90)->getAssets()==7);CHECK(g.getEdge(7)->getWeight()==0);
        CHECK(g.getEdge(7)->getCapacity()==9);CHECK(!g.isBlocked(7));
    });
    test("L02 missing/null capacity is unknown, explicit capacity zero is valid", [] {
        auto g=parse(dataset("["+row("3",",\"capacity\":null")+"]"));CHECK(!g.getEdge(7)->getCapacity());
        auto z=parse(dataset("["+row("3",",\"capacity\":0")+"]"));CHECK(z.getEdge(7)->getCapacity()==0);
        CHECK(z.getEdge(7)->getWeight()==3);
    });
    test("L03 blocked true loaded and respected", [] {
        auto g=parse(dataset("["+row("0",",\"blocked\":true")+"]"));CHECK(g.isBlocked(7));CHECK(g.getNeighbor(10).empty());
    });
    test("L04 float/string/negative/overflow weights rejected", [] {
        for(const auto& value:{"1.5","\"1\"","-1","2147483648","18446744073709551615"})
            invalid([&]{parse(dataset("["+row(value)+"]"));});
    });
    test("L05 duplicate/dangling edge IDs and duplicate node IDs rejected", [] {
        invalid([]{parse(dataset("["+row()+","+row()+"]"));});
        auto dangling=row();dangling.replace(dangling.find("90"),2,"91");invalid([&]{parse(dataset("["+dangling+"]"));});
        invalid([]{parse(R"({"nodes":[{"id":1,"name":"a","type":"ENTRY","assets":0},{"id":1,"name":"b","type":"TARGET","assets":0}],"edges":[]})");});
    });
    test("L06 malformed JSON, missing fields, wrong arrays rejected", [] {
        for(const auto& text:{"{", "{}", "[]", "{\"nodes\":{},\"edges\":[]}","{\"nodes\":[{}],\"edges\":[]}"})
            invalid([&]{parse(text);});
        invalid([]{parse(dataset("[{}]"));});
    });
    test("L07 invalid type/relation/boolean/unknown field rejected", [] {
        auto badType=dataset("[]");badType.replace(badType.find("ENTRY"),5,"BAD");invalid([&]{parse(badType);});
        auto badRelation=row();badRelation.replace(badRelation.find("AccessTo"),8,"BAD");invalid([&]{parse(dataset("["+badRelation+"]"));});
        invalid([]{parse(dataset("["+row("1",",\"blocked\":1")+"]"));});
        invalid([]{parse(dataset("["+row("1",",\"weigth\":1")+"]"));});
    });
    test("L08 duplicate JSON key and comments rejected", [] {
        invalid([]{parse(dataset("["+row("1",",\"weight\":2")+"]"));});
        invalid([]{parse("{/* comment */\"nodes\":[],\"edges\":[]}");});
    });
    test("L09 loader failure leaves caller's graph unchanged", [] {
        auto g=demo();invalid([&]{g=parse(dataset("["+row("-1")+"]"));});
        CHECK(g.getNodes().size()==4&&g.getEdges().size()==4);CHECK(dijkstra(g,0,3).totalCost==13);
    });
    test("L10 missing file reports explicit error", [] {
        bool caught=false;try{loadDataset("tests/fixtures/file_not_present.json");}catch(const std::runtime_error& e){caught=std::string(e.what()).find("Khong mo duoc")!=std::string::npos;}CHECK(caught);
    });
}
void attackTests() {
    test("A01 P2 original path 0-1-2-3 cost 13", [] {
        auto g=demo();auto p=dijkstra(g,0,3);CHECK(p.reachable&&p.totalCost==13);
        CHECK(p.nodes==std::vector<int>({0,1,2,3}));CHECK(p.edgeIds==std::vector<int>({0,1,2}));
    });
    test("A02 budget 12/13/15 gives OVER_BUDGET/SUCCESS/SUCCESS", [] {
        auto g=demo();auto low=hackerSimulation(g,0,3,12);CHECK(low.status==AttackStatus::OVER_BUDGET&&!low.remainingToken);
        auto exact=hackerSimulation(g,0,3,13);CHECK(exact.status==AttackStatus::SUCCESS&&exact.remainingToken==0);
        auto high=hackerSimulation(g,0,3,15);CHECK(high.status==AttackStatus::SUCCESS&&high.remainingToken==2);
    });
    test("A03 directed reverse has NO_PATH even with huge budget", [] {
        auto g=demo();auto a=hackerSimulation(g,3,0,9999999999LL);
        CHECK(a.status==AttackStatus::NO_PATH&&!a.path.reachable&&a.path.nodes.empty()&&!a.remainingToken);
    });
    test("A04 zero weight cycle and zero budget", [] {
        auto g=nodes({10,20,30});edge(g,1,10,20,0);edge(g,2,20,10,0);edge(g,3,20,30,0);
        auto a=hackerSimulation(g,10,30,0);CHECK(a.status==AttackStatus::SUCCESS&&a.path.totalCost==0);
        CHECK(a.path.nodes==std::vector<int>({10,20,30}));
    });
    test("A05 parallel edges choose exact cheaper ID; block changes path", [] {
        auto g=nodes({10,90});edge(g,100,10,90,9);edge(g,101,10,90,2);
        CHECK(dijkstra(g,10,90).edgeIds==std::vector<int>{101});g.blockEdge(101);
        CHECK(dijkstra(g,10,90).totalCost==9);CHECK(g.validatePath(10,90,{100}).cost==9);
    });
    test("A06 missing IDs/negative budget rejected; S==T costs zero", [] {
        auto g=demo();invalid([&]{dijkstra(g,99,3);});invalid([&]{dijkstra(g,0,99);});
        invalid([&]{hackerSimulation(g,0,3,-1);});auto a=hackerSimulation(g,2,2,0);
        CHECK(a.status==AttackStatus::SUCCESS&&a.path.totalCost==0&&a.path.edgeIds.empty());
    });
    test("A07 total cost above old INF and int32 range preserved", [] {
        auto g=nodes({1,2,3});edge(g,1,1,2,2000000000);edge(g,2,2,3,2000000000);
        auto a=hackerSimulation(g,1,3,4000000000LL);CHECK(a.path.totalCost==4000000000LL&&a.remainingToken==0);
    });
    test("A08 more than 40 nodes with sparse IDs", [] {
        Graph g;for(int i=0;i<60;++i)g.addNode(Node(i*10,"n",NodeType::ENDPOINT,0));
        for(int i=0;i<59;++i)edge(g,i,i*10,(i+1)*10,1);
        auto p=dijkstra(g,0,590);CHECK(p.reachable&&p.totalCost==59&&p.nodes.size()==60);
    });
}
void defenderTests() {
    test("D01 directed BFS reachable and unreachable", [] {
        auto g=demo();Defender d(g);CHECK(d.isReachable(0,3));CHECK(!d.isReachable(3,0));CHECK(d.isReachable(1,1));
        invalid([&]{d.isReachable(99,3);});invalid([&]{d.suggestCutEdge(0,99);});
    });
    test("D02 single-cut proposal is ID 0 and does not mutate graph", [] {
        auto g=demo();Defender d(g);CHECK(d.suggestCutEdge(0,3)==0);CHECK(g.statistics().blockedEdges==0);
        d.blockEdge(0);CHECK(!d.isReachable(0,3));CHECK(g.isBlocked(0));
    });
    test("D03 already disconnected returns -1 without cutting unrelated edge", [] {
        auto g=nodes({0,1,2});edge(g,1,0,1);Defender d(g);
        CHECK(d.suggestCutEdge(0,2)==-1);CHECK(!g.isBlocked(1));
    });
    test("D04 two edge-disjoint branches: single-cut returns -1", [] {
        auto g=nodes({0,1,2,3});edge(g,1,0,1);edge(g,2,1,3);edge(g,3,0,2);edge(g,4,2,3);
        Defender d(g);CHECK(d.suggestCutEdge(0,3)==-1);CHECK(d.isReachable(0,3));CHECK(g.statistics().blockedEdges==0);
    });
    test("D05 blocked input state preserved during proposal", [] {
        auto g=demo();g.blockEdge(2);Defender d(g);CHECK(d.suggestCutEdge(0,3)==0);
        CHECK(g.isBlocked(2)&&!g.isBlocked(0)&&g.statistics().blockedEdges==1);
    });
    test("D06 single-cut is explicitly not capacity-optimal", [] {
        auto g=nodes({0,1,2});edge(g,10,0,1,1,false,100);edge(g,20,1,2,1,false,1);
        Defender d(g);CHECK(d.suggestCutEdge(0,2)==10); // Real weighted min-cut would choose 20.
        CHECK(g.getEdge(10)->getCapacity()==100);
    });
    test("D07 S==T no cut; parallel edges require blocking both", [] {
        auto g=nodes({0,1});edge(g,1,0,1);edge(g,2,0,1);Defender d(g);
        CHECK(d.suggestCutEdge(0,0)==-1&&d.suggestCutEdge(0,1)==-1);
        g.blockEdge(1);CHECK(d.isReachable(0,1));CHECK(d.suggestCutEdge(0,1)==2);
    });
    test("D08 original P3 10-node topology and IDs 201..211", [] {
        Graph g;for(int i=0;i<10;++i)g.addNode(Node(i,"fixture",NodeType::ENDPOINT,0));
        std::vector<std::pair<int,int>> pairs{{0,1},{1,2},{1,3},{2,4},{3,4},{4,5},{4,6},{5,7},{6,7},{7,8},{8,9}};
        for(std::size_t i=0;i<pairs.size();++i)edge(g,201+static_cast<int>(i),pairs[i].first,pairs[i].second,0);
        // P3 did not provide weights; fixture uses zero solely for connectivity, no attack assertion.
        Defender d(g);CHECK(d.isReachable(0,9));CHECK(d.suggestCutEdge(0,9)==201);
        d.blockEdge(201);CHECK(!d.isReachable(0,9));CHECK(g.getEdges().size()==11);
    });
}
void integrationTests() {
    test("I01 JSON -> Attack -> real P3 single-cut -> Patch -> NO_PATH", [] {
        auto g=demo();auto r=runSimulation(g,0,3,15);
        CHECK(r.before.status==AttackStatus::SUCCESS&&r.before.path.totalCost==13);
        CHECK(r.patchedEdgeIds==std::vector<int>{0});CHECK(r.after.status==AttackStatus::NO_PATH);
        CHECK(r.reachableBefore&&!r.reachableAfter&&g.isBlocked(0));CHECK(g.getEdges().size()==4);
        CHECK(r.before.initialBudget==15&&r.after.initialBudget==15);
    });
    test("I02 manual patch edge 2: new cost 19 reachable but OVER_BUDGET", [] {
        auto g=demo();auto r=runSimulation(g,0,3,15,std::vector<int>{2});
        CHECK(r.after.path.nodes==std::vector<int>({0,1,3}));CHECK(r.after.path.totalCost==19);
        CHECK(r.after.status==AttackStatus::OVER_BUDGET&&r.reachableAfter);CHECK(g.isBlocked(2));
    });
    test("I03 reset budget 20: before remaining 7, after remaining 1", [] {
        auto g=demo();auto r=runSimulation(g,0,3,20,std::vector<int>{2});
        CHECK(r.before.remainingToken==7&&r.after.remainingToken==1);CHECK(r.after.status==AttackStatus::SUCCESS);
    });
    test("I04 no single cut: do not pretend Min-Cut, graph unchanged", [] {
        auto g=nodes({0,1,2,3});edge(g,1,0,1);edge(g,2,1,3);edge(g,3,0,2);edge(g,4,2,3);
        auto r=runSimulation(g,0,3,2);CHECK(r.patchedEdgeIds.empty());CHECK(r.reachableAfter);
        CHECK(r.defenseMessage.find("Can Max-Flow/Min-Cut")!=std::string::npos);
    });
    test("I05 invalid batch patch leaves graph unchanged", [] {
        auto g=demo();invalid([&]{runSimulation(g,0,3,15,std::vector<int>{0,999});});
        CHECK(g.statistics().blockedEdges==0&&dijkstra(g,0,3).totalCost==13);
    });
    test("I06 independent scenarios reload initial graph and repeated run uses current state", [] {
        auto g=demo();auto first=runSimulation(g,0,3,15);auto second=runSimulation(g,0,3,15);
        CHECK(first.before.status==AttackStatus::SUCCESS&&second.before.status==AttackStatus::NO_PATH);
        CHECK(second.patchedEdgeIds.empty());auto fresh=demo();CHECK(hackerSimulation(fresh,0,3,15).status==AttackStatus::SUCCESS);
    });
    test("I07 CLI wording distinguishes failure reasons", [] {
        auto g=demo();g.blockEdge(2);std::ostringstream over;printAttack(hackerSimulation(g,0,3,15),over);
        CHECK(over.str().find("OVER_BUDGET")!=std::string::npos);g.blockEdge(0);
        std::ostringstream no;printAttack(hackerSimulation(g,0,3,15),no);CHECK(no.str().find("NO_PATH")!=std::string::npos);
    });
}
void regressionTests() {
    test("R01 Dijkstra vs independent Bellman-Ford on 80 seeded small graphs, before/after patch", [] {
        std::mt19937 rng(20261001);
        for(int scenario=0;scenario<80;++scenario) {
            auto g=nodes({0,1,2,3,4,5});int id=0;
            for(int u=0;u<6;++u)for(int v=0;v<6;++v)
                if(rng()%4==0)edge(g,id++,u,v,static_cast<int>(rng()%8),rng()%5==0);
            for(int stage=0;stage<2;++stage) {
                for(int s=0;s<6;++s) {
                    const std::int64_t inf=1000000000000LL;
                    std::vector<std::int64_t> dist(6,inf);dist[s]=0;
                    for(int pass=0;pass<5;++pass)for(const auto& e:g.getEdges())
                        if(!e.isBlocked()&&dist[e.getFrom()]!=inf)
                            dist[e.getTo()]=std::min(dist[e.getTo()],dist[e.getFrom()]+e.getWeight());
                    for(int t=0;t<6;++t) {
                        auto p=dijkstra(g,s,t);CHECK(p.reachable==(dist[t]!=inf));
                        if(p.reachable){CHECK(p.totalCost==dist[t]);auto valid=g.validatePath(s,t,p.edgeIds);CHECK(valid.valid&&valid.cost==p.totalCost);}
                    }
                }
                if(id>0)g.blockEdge(static_cast<int>(rng()%static_cast<unsigned>(id)));
            }
        }
    });
    test("R02 empty graph does not fabricate a path", [] {
        Graph g;invalid([&]{dijkstra(g,0,0);});Defender d(g);invalid([&]{d.isReachable(0,0);});
        CHECK(g.statistics().nodes==0&&g.statistics().edges==0);
    });
}
}
int main(int argc,char* argv[]) {
    std::string group=argc>1?argv[1]:"all";
    const std::vector<std::pair<std::string,std::function<void()>>> suites{{"graph",graphTests},{"loader",loaderTests},{"attack",attackTests},{"defender",defenderTests},{"integration",integrationTests},{"regression",regressionTests}};
    bool found=false;
    for(const auto& suite:suites)if(group=="all"||group==suite.first){found=true;suite.second();}
    if(!found){std::cerr<<"Unknown group\n";return 2;}
    std::cout<<"RESULT: "<<passed<<" passed, "<<failed<<" failed\n";
    return failed?1:0;
}
