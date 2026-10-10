#include "RuntimeInput.h"
#include "LoadDataset.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace fs=std::filesystem;
void check(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
int main() {
    const auto temp=fs::temp_directory_path()/ ("attackgraph-input-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    try {
        fs::create_directories(temp/"project/data");fs::create_directories(temp/"package/data");
        auto write=[&](const fs::path& path,const char* text){std::ofstream out(path);out<<text;check(out.good(),"Write failed");};
        const auto exe=temp/"package/AttackGraph.exe";
        write(temp/"project/data/graph.json","project");write(temp/"package/data/graph.json","old copy");
        check(resolveDatasetPath({},exe,temp/"project")==temp/"project/data/graph.json","Project priority");
        check(resolveDatasetPath(temp/"missing.json",exe,temp/"project")==temp/"missing.json","Explicit path fallback");
        check(resolveDatasetPath({},exe,temp/"absent")==temp/"package/data/graph.json","Packaged data fallback");
        fs::remove(temp/"package/data/graph.json");write(temp/"package/graph.json","package");
        check(resolveDatasetPath({},exe,temp/"absent")==temp/"package/graph.json","Adjacent JSON fallback");
        Graph g;g.addNode(Node(700,"entry",NodeType::ENTRY,0));g.addNode(Node(920,"endpoint",NodeType::ENDPOINT,0));g.addNode(Node(1500,"target",NodeType::TARGET,0));
        std::istringstream in("999\n920\n1500\n-1\n17\n");std::ostringstream out;
        auto p=selectParameters(g,in,out);
        check(p.source==920&&p.target==1500&&p.budget==17,"Interactive retry/endpoint source");
        check(out.str().find("700 - entry")!=std::string::npos&&out.str().find("1500 - target")!=std::string::npos,"Dynamic listings");
        auto d=demoParameters(g);check(d.source==700&&d.target==1500&&d.budget==0,"Dynamic demo");
        std::istringstream eof;bool rejected=false;try{selectParameters(g,eof,out);}catch(const std::runtime_error&){rejected=true;}check(rejected,"EOF handling");
        for(auto text:{"-1","1.2","","18446744073709551616"}) {
            rejected=false;try{parseNonnegative(text);}catch(const std::invalid_argument&){rejected=true;}check(rejected,"Bad numeric input");
        }
        fs::remove_all(temp);std::cout<<"runtime_input PASS\n";return 0;
    }catch(const std::exception& e){fs::remove_all(temp);std::cerr<<e.what()<<'\n';return 1;}
}
