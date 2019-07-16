// vim: set noet ts=4 sw=4:

#include "llvm_map.h"

#include "llvm_common.h"

#include <llvm/IR/BasicBlock.h>

using namespace llvm;
using namespace std;

namespace step {
	std::string LLVMMap::get_description() {
		return "Map llvm::Basicblock and ara::cfg::ABB"
		       "\n"
		       "Maps in a one to one mapping.";
	}

	void LLVMMap::run(graph::Graph& graph) {
		ara::graph::Graph& new_graph = graph.new_graph;
		Module& mod = new_graph.get_module();
		ara::cfg::ABBGraph& abbs = new_graph.abbs();

		map<const BasicBlock*, std::shared_ptr<ara::cfg::ABB>> bb_map;

		unsigned name_counter = 0;

		for (Function& func : mod) {
			bool first_run = true;
			for (BasicBlock& bb : func) {
				std::stringstream ss;
				ss << "ABB" << name_counter++;
				ara::cfg::ABBType ty =
				    (FakeCallBase::isa(bb.front())) ? ara::cfg::ABBType::call : ara::cfg::ABBType::computation;
				auto vertex = abbs.add_vertex(ss.str(), ty, &bb, &bb);

				// connect already mapped successors and predecessors
				for (const BasicBlock* succ_b : successors(&bb)) {
					if (abbs.contain(succ_b)) {
						abbs.add_edge(vertex, abbs.back_map(succ_b));
					}
				}
				for (const BasicBlock* pred_b : predecessors(&bb)) {
					if (abbs.contain(pred_b)) {
						abbs.add_edge(abbs.back_map(pred_b), vertex);
					}
				}
			}
		}
	}
} // namespace step
