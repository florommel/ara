"""Common init function for tests."""
import sys
import json

import graph
import stepmanager


def fail_with(*arg):
    """Print an error message and exit."""
    print("ERROR:", *arg, file=sys.stderr)
    sys.exit(1)


def init_test():
    """CLI usage: your_program <os_name> <json_file> <ll_file>"""
    g = graph.PyGraph()
    os_name = sys.argv[1]
    json_file = sys.argv[2]
    i_file = sys.argv[3]
    print("Testing with", i_file, "and json:", json_file, "and os: ", os_name)
    with open(json_file) as f:
        data = json.load(f)

    config = {'os': os_name,
              'input_files': [i_file]}
    s_manager = stepmanager.StepManager(g, config)

    s_manager.execute(['ValidationStep'])

    return g, data, s_manager
