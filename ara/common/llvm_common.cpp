#include "common/llvm_common.h"
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace ara {

	std::string print_type(Type* argument) {
		std::string type_str;
		raw_string_ostream rso(type_str);
		argument->print(rso);
		return rso.str() + "\"\n";
	}

	bool isCallToLLVMIntrinsic(const Instruction* inst) {
		if (const CallBase* call = dyn_cast<CallBase>(inst)) {
			const Function* func = call->getCalledFunction();
			if (func && func->isIntrinsic()) {
				return true;
			}
		}
		return false;
	}

	bool isInlineAsm(const Instruction* inst) {
		if (const CallBase* call = dyn_cast<CallBase>(inst)) {
			return call->isInlineAsm();
		}
		return false;
	}

} // namespace ara
