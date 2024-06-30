import json
import sys

if len(sys.argv) != 2:
    print('Usage: python3 compare.py <workflow_run>.json')
    sys.exit(1)

with open(sys.argv[1], 'r') as f:
    workflow_json = json.load(f)

workflow_map = {}
for benchmark in workflow_json['benchmarks']:
    workflow_map[benchmark['name']] = benchmark['Pushes']

with open('baseline.json', 'r') as f:
    baseline_json = json.load(f)

baseline_map = {}
for benchmark in baseline_json['benchmarks']:
    baseline_map[benchmark['name']] = benchmark['Pushes']

deteriorated_benchmarks = list()
for name, pushes in baseline_map.items():
    if name in workflow_map:
        if pushes > workflow_map[name] and (pushes - workflow_map[name]) / pushes > 0.05:
            deteriorated_benchmarks.append((name, (pushes - workflow_map[name]) / pushes))
            result = False

if deteriorated_benchmarks:
    print(*deteriorated_benchmarks, sep='\n')
    exit(-1)
exit(0)
