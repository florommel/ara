// vim: set noet ts=4 sw=4:

#pragma once

#include "option.h"
#include "step.h"

#include <graph.h>

namespace step {
	class ICFG : public Step {
	  public:
		virtual std::string get_name() const override { return "ICFG"; }
		virtual std::string get_description() const override;
		virtual std::vector<std::string> get_dependencies() override { return {"LLVMMap"}; }

		virtual void run(ara::graph::Graph& graph) override;
	};
} // namespace step
