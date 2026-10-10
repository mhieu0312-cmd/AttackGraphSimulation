"""End-to-end runtime tests: temporary data only; no user JSON is modified."""
import json
import pathlib
import subprocess
import sys
import tempfile

exe = pathlib.Path(sys.argv[1]).resolve()
fixtures = pathlib.Path(sys.argv[2])

def run(args, input=None, ok=True, cwd=None):
    p = subprocess.run([str(exe), *map(str, args)], input=input, text=True,
                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=15, cwd=cwd)
    assert (p.returncode == 0) == ok, p.stdout
    return p.stdout

def node(id, type):
    return dict(id=id, name=f"arbitrary_{id}", type=type, assets=0)

def edge(id, u, v, weight, capacity, blocked=False):
    return dict(id=id, **{"from":u}, to=v, weight=weight, capacity=capacity,
                relation="AccessTo", blocked=blocked)

with tempfile.TemporaryDirectory(prefix="attackgraph-dynamic-") as folder:
    root = pathlib.Path(folder)
    a = fixtures / "company_original.json"
    b = fixtures / "company_renumbered.json"
    for path, source in [(a,36),(b,18)]:
        output = run([path,source,3,30])
        assert "Cost: 28" in output and "Max-Flow: 6" in output and "Reachable: 1 -> 0" in output, output
        assert str(path.resolve()) in output, output
    c = dict(nodes=[node(700,"ENTRY"),node(800,"ENTRY"),node(920,"ENDPOINT"),
                    node(1500,"TARGET"),node(1600,"TARGET"),node(2000,"IDENTITY"),
                    node(2500,"CRITICAL_SYSTEM")],
             edges=[edge(501,700,920,2,4),edge(777,920,1500,3,2),edge(999,800,1500,1,3),
                    edge(1111,700,1600,1,8,True)])
    path = root / "graph.json"
    path.write_text(json.dumps(c))
    output = run([path,700,1500,10])
    assert "Nodes: 7" in output and "Edges: 4" in output and "Cost: 5" in output and "Min-Cut Capacity: 2" in output, output
    assert "Patched edges: 777" in output and "Reachable: 1 -> 0" in output, output
    output = run([path], "700\n1500\n10\n")
    assert "700 - arbitrary_700" in output and "1600 - arbitrary_1600" in output and "Cost: 5" in output, output
    output = run([path,920,1500,10])
    assert "Cost: 3" in output, output
    output = run([path,700,1600,10])
    assert "NO_PATH" in output and "Reachable: 0 -> 0" in output, output
    assert "Source ID khong ton tai" in run([path,123,1500,10],ok=False)
    assert "Target ID khong ton tai" in run([path,700,123,10],ok=False)
    assert "ERROR:" in run([path],input="",ok=False)
    output=run(["--demo",path]);assert "Source: 700" in output and "Target: 1500" in output and "Budget: 0" in output,output
    # Same executable, same path, changed JSON: no rebuild between subprocesses.
    c["edges"][1]["weight"] = 7
    c["edges"][1]["capacity"] = 1
    path.write_text(json.dumps(c))
    output=run([path,700,1500,10])
    assert "Cost: 9" in output and "Min-Cut Capacity: 1" in output,output
    # Missing role types warn but interactive mode can choose any existing nodes.
    missing=dict(nodes=[node(42,"ENDPOINT"),node(99,"IDENTITY")],edges=[edge(8,42,99,4,1)])
    path.write_text(json.dumps(missing))
    output=run([path],"42\n99\n8\n")
    assert "Khong co node ENTRY" in output and "Khong co node TARGET" in output and "Cost: 4" in output,output
    assert "Demo can node ENTRY" in run(["--demo",path],ok=False)
    path.write_text('{"nodes":[],"edges":"invalid"}')
    assert "Dataset khong hop le" in run([path,42,99,10],ok=False)
    path.write_text('{"nodes":[],"edges":[]}')
    assert "Graph rong" in run([path],ok=False)
    # Explicit invalid/missing files must not silently fall back to valid project JSON.
    assert str((root/"missing.json").resolve()) in run([root/"missing.json",1,2,10],ok=False)
    assert "Usage:" in run(["--help"])
print("dynamic datasets A/B/C, interactive/CLI/demo, reload and error handling PASS")
