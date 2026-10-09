#include "Simulation.h"
#include "LoadDataset.h"
#include <functional>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <stdexcept>

namespace {
void check(bool ok,const char* message) {if(!ok) throw std::runtime_error(message);}
Graph sample() {
    Graph g;
    g.addNode(Node(10,"entry",NodeType::ENTRY,0));
    g.addNode(Node(40,"pc",NodeType::ENDPOINT,10));
    g.addNode(Node(90,"target",NodeType::TARGET,100));
    g.addEdge(Edge(10,40,4,"HasSession",false,1,2));
    g.addEdge(Edge(40,90,5,"AccessTo",false,2,3));
    g.addEdge(Edge(10,90,12,"AccessTo",false,3,4));
    return g;
}
void onlyPatchChanged(const Graph& initial,const Graph& after,const std::vector<int>& patch) {
    const std::set<int> ids(patch.begin(),patch.end());
    check(initial.getNodes().size()==after.getNodes().size()&&initial.getEdges().size()==after.getEdges().size(),"Graph size changed");
    for(std::size_t i=0;i<initial.getNodes().size();++i) {
        const auto& a=initial.getNodes()[i];const auto& b=after.getNodes()[i];
        check(a.getID()==b.getID()&&a.getName()==b.getName()&&a.getType()==b.getType()&&a.getAssets()==b.getAssets(),"Node changed");
    }
    for(std::size_t i=0;i<initial.getEdges().size();++i) {
        const auto& a=initial.getEdges()[i];const auto& b=after.getEdges()[i];
        check(a.getID()==b.getID()&&a.getFrom()==b.getFrom()&&a.getTo()==b.getTo()&&a.getWeight()==b.getWeight()&&a.getCapacity()==b.getCapacity()&&a.getRelation()==b.getRelation(),"Edge data changed");
        check(b.isBlocked()==(a.isBlocked()||ids.count(a.getID())!=0),"Unexpected blocked state");
    }
}
void autoMultiple() {
    const auto initial=loadDataset(std::string(DATASET_PATH));auto g=initial;
    auto r=runSimulation(g,36,3,30);
    check(r.before.path.nodes==std::vector<int>({36,32,20,7,3}),"Default attack path");
    check(r.before.path.totalCost==28&&r.before.status==AttackStatus::SUCCESS,"Default attack result");
    check(r.minCut&&r.minCut->maxFlow==6&&r.minCut->minCutCapacity==6,"Default cut metrics");
    check(r.patchedEdgeIds==r.minCut->edgeIds&&r.newlyBlockedEdges==3,"Entire mincut patched");
    check(r.reachableBefore&&!r.reachableAfter&&r.after.status==AttackStatus::NO_PATH,"Auto cut disconnects");
    check(g.statistics().blockedEdges==3,"Default blocked count");
    onlyPatchChanged(initial,g,r.patchedEdgeIds);
    auto low=initial;auto lowResult=runSimulation(low,36,3,27);
    check(lowResult.before.status==AttackStatus::OVER_BUDGET&&lowResult.after.status==AttackStatus::NO_PATH,"Defense for over-budget attack");
}
void autoSingle() {
    auto g=sample();g.blockEdge(3);const Graph initial=g;
    auto r=runSimulation(g,10,90,20);
    check(r.minCut&&r.minCut->minCutCapacity==2&&r.newlyBlockedEdges==1,"Single edge auto patch");
    check(r.patchedEdgeIds==std::vector<int>{1}&&!r.reachableAfter,"Single cut ID");
    onlyPatchChanged(initial,g,r.patchedEdgeIds);
}
void disconnected() {
    auto g=sample();g.blockEdges({1,3});const Graph initial=g;
    for(bool manual:{false,true}) {
        auto r=runSimulation(g,10,90,20,manual?std::optional<std::vector<int>>{{2}}:std::nullopt);
        check(!r.minCut&&r.patchedEdgeIds.empty()&&r.newlyBlockedEdges==0,"Disconnected graph patched");
        check(!r.reachableBefore&&!r.reachableAfter&&r.before.status==AttackStatus::NO_PATH&&r.after.status==AttackStatus::NO_PATH,"Disconnected results");
        onlyPatchChanged(initial,g,{});
    }
}
void sameNode() {
    auto g=sample();const Graph initial=g;
    for(bool manual:{false,true}) {
        auto r=runSimulation(g,10,10,0,manual?std::optional<std::vector<int>>{{1}}:std::nullopt);
        check(!r.minCut&&r.patchedEdgeIds.empty()&&r.newlyBlockedEdges==0,"Equal endpoints patched");
        check(r.before.path.totalCost==0&&r.after.status==AttackStatus::SUCCESS&&r.reachableAfter,"Equal endpoints attack");
        check(r.defenseMessage.find("cung mot node")!=std::string::npos,"Equal endpoints message");
        onlyPatchChanged(initial,g,{});
    }
}
void manual() {
    auto g=sample();auto r=runSimulation(g,10,90,20,std::vector<int>{1});
    check(!r.minCut&&r.patchedEdgeIds==std::vector<int>{1},"Manual mode called MinCut");
    check(r.reachableAfter&&r.after.path.totalCost==12,"Manual alternate path");
    auto full=sample();r=runSimulation(full,10,90,20,std::vector<int>{1,3});
    check(!r.reachableAfter&&r.newlyBlockedEdges==2,"Manual full patch");
    auto duplicate=sample();duplicate.blockEdge(3);
    r=runSimulation(duplicate,10,90,20,std::vector<int>{3,1,1});
    check(r.patchedEdgeIds==std::vector<int>{1}&&r.newlyBlockedEdges==1,"Duplicate/preblocked count");
    auto empty=sample();r=runSimulation(empty,10,90,20,std::vector<int>{});
    check(!r.minCut&&r.reachableAfter&&r.newlyBlockedEdges==0,"Empty manual is not auto");
}
void budget() {
    auto g=sample();auto r=runSimulation(g,10,90,20,std::vector<int>{1});
    check(r.before.initialBudget==20&&r.after.initialBudget==20,"Initial budget not reset");
    check(r.before.remainingToken==11&&r.after.remainingToken==8&&r.after.status==AttackStatus::SUCCESS,"Remaining budget leaked");
    auto low=sample();r=runSimulation(low,10,90,10,std::vector<int>{1});
    check(r.before.status==AttackStatus::SUCCESS&&r.after.status==AttackStatus::OVER_BUDGET&&r.after.initialBudget==10,"Manual over budget");
}
void atomic() {
    auto g=sample();const Graph initial=g;bool threw=false;
    try {runSimulation(g,10,90,20,std::vector<int>{1,999,3});}
    catch(const std::invalid_argument&){threw=true;}
    check(threw,"Invalid manual ID accepted");onlyPatchChanged(initial,g,{});
    for(auto endpoints:{std::pair<int,int>{999,90},{10,999}}) {
        threw=false;try {runSimulation(g,endpoints.first,endpoints.second,20,std::vector<int>{1});}
        catch(const std::invalid_argument&){threw=true;}
        check(threw,"Invalid node accepted");onlyPatchChanged(initial,g,{});
    }
    threw=false;try {runSimulation(g,10,90,-1,std::vector<int>{1});}
    catch(const std::invalid_argument&){threw=true;}
    check(threw,"Negative budget accepted");onlyPatchChanged(initial,g,{});
}
std::string fileContent() {
    std::ifstream in(DATASET_PATH);check(in.good(),"Dataset missing");
    return {std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};
}
void independent() {
    const auto bytes=fileContent();const auto initial=loadDataset(std::string(DATASET_PATH));
    auto first=initial;auto r1=runSimulation(first,36,3,30);
    auto second=initial;auto r2=runSimulation(second,36,3,30);
    check(r1.before.path.nodes==r2.before.path.nodes&&r1.patchedEdgeIds==r2.patchedEdgeIds&&r2.reachableBefore,"Scenarios share blocked state");
    onlyPatchChanged(initial,loadDataset(std::string(DATASET_PATH)),{});
    check(fileContent()==bytes,"JSON changed");
    auto repeat=runSimulation(first,36,3,30);
    check(!repeat.reachableBefore&&repeat.patchedEdgeIds.empty(),"Same graph should retain patch");
}
void unchanged() {
    auto g=sample();const Graph initial=g;
    auto r=runSimulation(g,10,90,20,std::vector<int>{2});onlyPatchChanged(initial,g,r.patchedEdgeIds);
}
void zero() {
    Graph g;g.addNode(Node(10,"entry",NodeType::ENTRY,0));g.addNode(Node(90,"target",NodeType::TARGET,100));
    g.addEdge(Edge(10,90,5,"AccessTo",false,7,0));
    auto r=runSimulation(g,10,90,10);
    check(r.minCut&&r.minCut->maxFlow==0&&r.minCut->minCutCapacity==0,"Zero cut cost");
    check(r.newlyBlockedEdges==1&&!r.reachableAfter,"Zero capacity must still patch");
}
}
int main(int argc,char** argv) {
    try {
        const std::string group=argc>1?argv[1]:"all";bool ran=false;
        for(const auto& test:std::vector<std::pair<std::string,std::function<void()>>>{
            {"auto_multiple",autoMultiple},{"auto_single",autoSingle},{"disconnected",disconnected},
            {"same_node",sameNode},{"manual",manual},{"budget",budget},{"atomic",atomic},
            {"independent",independent},{"unchanged",unchanged},{"zero",zero}}) {
            if(group=="all"||group==test.first){test.second();ran=true;std::cout<<test.first<<" PASS\n";}
        }
        check(ran,"Unknown group");return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
