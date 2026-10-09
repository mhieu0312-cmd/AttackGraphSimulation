#include "MinCut.h"
#include "Defender.h"
#include "LoadDataset.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>

namespace {
void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
Graph nodes(std::initializer_list<int> ids) {
    Graph g;
    for (int id : ids) g.addNode(Node(id,"node"+std::to_string(id),NodeType::ENDPOINT,123));
    return g;
}
void edge(Graph& g,int id,int from,int to,int capacity,bool blocked=false) {
    g.addEdge(Edge(from,to,999,"AccessTo",blocked,id,capacity));
}
void unchanged(const Graph& a,const Graph& b) {
    check(a.getNodes().size()==b.getNodes().size()&&a.getEdges().size()==b.getEdges().size(),"Graph size changed");
    for(std::size_t i=0;i<a.getNodes().size();++i) {
        const auto& x=a.getNodes()[i];const auto& y=b.getNodes()[i];
        check(x.getID()==y.getID()&&x.getName()==y.getName()&&x.getType()==y.getType()&&x.getAssets()==y.getAssets(),"Node changed");
    }
    for(std::size_t i=0;i<a.getEdges().size();++i) {
        const auto& x=a.getEdges()[i];const auto& y=b.getEdges()[i];
        check(x.getID()==y.getID()&&x.getFrom()==y.getFrom()&&x.getTo()==y.getTo()&&x.getWeight()==y.getWeight()&&x.getCapacity()==y.getCapacity()&&x.getRelation()==y.getRelation()&&x.isBlocked()==y.isBlocked(),"Edge changed");
    }
}
MinCutResult verify(const Graph& g,int s,int t,std::int64_t expected) {
    const Graph snapshot=g;
    const auto result=minimumSTCut(g,s,t);
    unchanged(g,snapshot);
    check(result.maxFlow==expected&&result.minCutCapacity==expected,"Flow/cut mismatch");
    std::set<int> unique;
    std::int64_t sum=0;
    for(int id:result.edgeIds) {
        const auto* e=g.getEdge(id);
        check(e&&!e->isBlocked()&&unique.insert(id).second,"Invalid/blocked/duplicate cut edge");
        sum+=e->getCapacity();
    }
    check(sum==expected,"Cut sum mismatch");
    Graph patched=g;patched.blockEdges(result.edgeIds);
    check(!Defender(patched).isReachable(s,t),"Cut does not disconnect topology");
    check(minimumSTCut(g,s,t).edgeIds==result.edgeIds,"Repeated calculation changed result");
    return result;
}
void single() {
    auto g=nodes({10,40,900});edge(g,100,10,40,7);edge(g,200,40,900,3);
    check(verify(g,10,900,3).edgeIds==std::vector<int>{200},"Single path cut");
}
void multiple() {
    auto g=nodes({10,20,30,90});edge(g,1,10,20,2);edge(g,2,20,90,4);
    edge(g,3,10,30,3);edge(g,4,30,90,5);
    check(verify(g,10,90,5).edgeIds.size()==2,"Multi-edge cut");
}
void parallel() {
    auto g=nodes({10,90});edge(g,101,10,90,2);edge(g,202,10,90,3);edge(g,303,90,10,8);
    check(verify(g,10,90,5).edgeIds==std::vector<int>({101,202}),"Parallel/opposite cut IDs");
}
void cycles() {
    auto g=nodes({10,20,90});edge(g,1,10,10,100);edge(g,2,10,20,5);
    edge(g,3,20,10,2);edge(g,4,20,20,9);edge(g,5,20,90,3);edge(g,6,90,20,8);
    verify(g,10,90,3);
}
void zero() {
    auto g=nodes({10,20,90});edge(g,1,10,20,0);edge(g,2,20,90,4);
    check(verify(g,10,90,0).edgeIds==std::vector<int>{1},"Zero-capacity topological cut");
    edge(g,3,10,90,2);verify(g,10,90,2);
    auto allZero=nodes({1,2,3});edge(allZero,1,1,2,0);edge(allZero,2,2,3,0);edge(allZero,3,1,3,0);
    check(verify(allZero,1,3,0).edgeIds.size()==2,"All-zero cut");
}
void blocked() {
    auto g=nodes({10,20,90});edge(g,1,10,90,100,true);edge(g,2,10,20,2);edge(g,3,20,90,4);
    verify(g,10,90,2);g.blockEdge(2);
    check(verify(g,10,90,0).edgeIds.empty(),"Disconnected blocked graph needs no cut");
}
void invalid() {
    const auto g=nodes({10,90});
    for(auto endpoints: {std::pair<int,int>{999,90},{10,999},{10,10}}) {
        bool threw=false;
        try {minimumSTCut(g,endpoints.first,endpoints.second);}
        catch(const std::invalid_argument& e) {threw=std::string(e.what()).find("MinCut:")==0;}
        check(threw,"Invalid endpoint not rejected clearly");
    }
    bool threw=false;try {minimumSTCut(Graph{},1,2);}catch(const std::invalid_argument&){threw=true;}
    check(threw,"Empty graph input");
}
void disconnected() {
    auto g=nodes({10,20,90});edge(g,1,90,10,5);edge(g,2,10,20,0);
    check(verify(g,10,90,0).edgeIds.empty(),"Already disconnected returns empty cut");
}
void large() {
    auto g=nodes({10,90});const int max=std::numeric_limits<int>::max();
    edge(g,1,10,90,max);edge(g,2,10,90,max);edge(g,3,90,10,max);
    verify(g,10,90,2LL*max);
}
void residualCancellation() {
    // The second augmentation must cancel a->b via its residual reverse arc.
    auto g=nodes({0,1,2,3,4,5});
    edge(g,1,0,1,1);edge(g,2,0,2,1);edge(g,3,1,3,1);edge(g,4,1,4,1);
    edge(g,5,2,3,1);edge(g,6,3,5,1);edge(g,7,4,5,1);
    verify(g,0,5,2);
}
void dataset() {
    const auto g=loadDataset(std::string(DATASET_PATH));
    check(g.statistics().nodes==18&&g.statistics().edges==24,"Dataset counts");
    for(auto target: {std::pair<int,int>{1,3},{2,4},{3,6},{13,3}}) {
        const auto r=verify(g,36,target.first,target.second);
        std::cout<<"36 -> "<<target.first<<": flow="<<r.maxFlow<<", cut="<<r.minCutCapacity<<", IDs:";
        for(int id:r.edgeIds) std::cout<<' '<<id;
        std::cout<<'\n';
    }
}
void exhaustive() {
    // Independent oracle: enumerate every S/T partition in 200 small graphs.
    std::mt19937 random(20261010);
    for(int trial=0;trial<200;++trial) {
        auto g=nodes({10,30,70,90,200});const std::vector<int> ids{10,30,70,90,200};
        for(int id=0;id<20;++id) {
            int u=random()%5,v=random()%5,cap=random()%6;
            edge(g,id,ids[u],ids[v],cap,random()%5==0);
        }
        std::int64_t best=std::numeric_limits<std::int64_t>::max();
        for(unsigned mask=0;mask<32;++mask) {
            if(!(mask&1)||(mask&(1U<<4))) continue;
            std::int64_t sum=0;
            for(const auto& e:g.getEdges()) {
                auto u=std::find(ids.begin(),ids.end(),e.getFrom())-ids.begin();
                auto v=std::find(ids.begin(),ids.end(),e.getTo())-ids.begin();
                if(!e.isBlocked()&&(mask&(1U<<u))&&!(mask&(1U<<v))) sum+=e.getCapacity();
            }
            best=std::min(best,sum);
        }
        verify(g,10,200,best);
    }
}
}
int main(int argc,char** argv) {
    try {
        const std::string group=argc>1?argv[1]:"all";bool ran=false;
        for(const auto& test:std::vector<std::pair<std::string,std::function<void()>>>{
            {"single",single},{"multiple",multiple},{"parallel",parallel},{"cycles",cycles},
            {"zero",zero},{"blocked",blocked},{"invalid",invalid},{"disconnected",disconnected},
            {"large",large},{"reverse",residualCancellation},{"dataset",dataset},{"exhaustive",exhaustive}}) {
            if(group=="all"||group==test.first) {test.second();ran=true;std::cout<<test.first<<" PASS\n";}
        }
        check(ran,"Unknown test group");return 0;
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
