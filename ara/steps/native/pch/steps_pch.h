#ifndef ARA_STEP_PCH_H
#define ARA_STEP_PCH_H

// extracted with:
// rg "#include <" -N -I -g '!pch' | sort | uniq

#include <Python.h>
#include <any>
#include <assert.h>
#include <boost/graph/filtered_graph.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/python.hpp>
#include <boost/type_traits.hpp>
#include <cassert>
#include <common/llvm_common.h>
#include <functional>
#include <graph.h>
#include <iostream>
#include <list>
#include <llvm/ADT/GraphTraits.h>
#include <llvm/ADT/SCCIterator.h>
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/CFGPrinter.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/MemorySSA.h>
#include <llvm/Analysis/OrderedBasicBlock.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Value.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/JSON.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/UnifyFunctionExitNodes.h>
#include <map>
#include <optional>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#endif // ARA_STEP_PCH_H
