#!/usr/bin/env python3

"""Automated Realtime System Analysis"""

import graph
#import pass1
import argparse
import sys
import passagemanager

from passages import OilPassage
from passages import LLVMPassage


def main():
    parser = argparse.ArgumentParser(prog=sys.argv[0],
                                     description=sys.modules[__name__].__doc__)
    parser.add_argument('--verbose', '-v', help="be verbose",
                        action="store_true", default=False)
    parser.add_argument('--os', '-O', help="specify the operation system",
                        choices=['freertos', 'osek'], default='osek')
    parser.add_argument('input_files', help="all LLVM-IR input files",
                        nargs='+')
    args = parser.parse_args()

    print(args.os)

    g = graph.PyGraph()

    p_manager = passagemanager.PassageManager(g)

    p_manager.register(OilPassage(args))
    p_manager.register(LLVMPassage(args))

    #p = pass1.PyPass()
    #a = [x.encode('utf-8') for x in args.input_files]

    #p.run(g, a)

    p_manager.execute(['LLVMPassage'])


if __name__ == '__main__':
    main()
