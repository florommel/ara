// vim: set noet ts=4 sw=4:

#include "llvm/Analysis/AssumptionCache.h"
#include "FreeRTOSinstances.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/CFG.h"
#include "llvm/Pass.h"
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <cassert>
#include <stdexcept>
#include <functional>
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/TypeFinder.h"
#include "CFG.h"
#include "llvm/ADT/PostOrderIterator.h" 
#include "llvm/ADT/SCCIterator.h"

using namespace llvm;



start_scheduler_relation before_scheduler_instructions(graph::Graph& graph,OS::shared_function function,std::vector<std::size_t> *already_visited);
//print the argument
std::string print_tmp(llvm::Value* val){
    std::string type_str;
	llvm::raw_string_ostream rso(type_str);
	val->print(rso);
	return rso.str() ;
}

void test_debug_argument(argument_data argument){
	
	
	const std::size_t  tmp_int = typeid(int).hash_code();
	const std::size_t  tmp_double = typeid(double).hash_code();
	const std::size_t  tmp_string = typeid(std::string).hash_code();
	const std::size_t tmp_long 	= typeid(long).hash_code();
	std::cerr << "Argument: ";
	/*
	std::string type_str;
	llvm::raw_string_ostream rso(type_str);
	type->print(rso);
	std::cout<< rso.str() << std::endl ;
	*/
	//std::cout << "reference: " << tmp_int << " " << tmp_double << " " << tmp_string << " " << tmp_long << std::endl;
	
	for(auto element : argument.any_list){
        std::size_t const tmp = element.type().hash_code();
        if(tmp_int == tmp){
            std::cerr << std::any_cast<int>(element)   ;
        }else if(tmp_double == tmp){ 
            std::cerr << std::any_cast<double>(element) ;
        }else if(tmp_string == tmp){
            std::cerr << std::any_cast<std::string>(element)  ;  
        }else if(tmp_long == tmp){
            std::cerr << std::any_cast<long>(element) ;  
        }else{
            std::cerr << "[warning: cast not possible] type: " <<element.type().name() ;  
        }
        std::cerr << ", ";
    }
    std::cerr << std::endl;
    for(auto element : argument.value_list){
        if(element == nullptr)continue;
        std::string type_str;
        llvm::raw_string_ostream rso(type_str);
        element->print(rso);
        std::cerr << rso.str() << std::endl << "," ;
    }
    std::cerr << std::endl;
}

//check if element is in list
bool list_contains_element(std::list<std::size_t>* list, size_t target){
	for(auto element : *list){
		if(element == target)return true;
	}
	return false;
}

//check if instruction a is before instruction b 
 bool check_instruction_order( Instruction *InstA,  Instruction *InstB,DominatorTree *DT) {
	DenseMap< BasicBlock *, std::unique_ptr<OrderedBasicBlock>> OBBMap;
	if (InstA->getParent() == InstB->getParent()){
		BasicBlock *IBB = InstA->getParent();
		auto OBB = OBBMap.find(IBB);
		if (OBB == OBBMap.end())OBB = OBBMap.insert({IBB, make_unique<OrderedBasicBlock>(IBB)}).first;
		return OBB->second->dominates(InstA, InstB);
	}
	DomTreeNode *DA = DT->getNode(InstA->getParent());
	DomTreeNode *DB = DT->getNode(InstB->getParent());
	return DA->getDFSNumIn() < DB->getDFSNumIn();
 }

std::vector<llvm::Instruction*> get_start_scheduler_instruction(OS::shared_function function){
	std::vector<llvm::Instruction*> instruction_vector;
	for(auto & abb : function->get_atomic_basic_blocks()){
		if(abb->get_call_type() != no_call){
			//std::cout << abb->get_call_name() << std::endl; 
			for(auto& call_name : abb->get_call_names() ){
				if(call_name == "vTaskStartScheduler")instruction_vector.push_back(abb->get_syscall_instruction_reference());
			}
		}
	}
	return instruction_vector;
}

//return true if instruction is reachable from a instruction from the list
bool validate_instructions_reachability(std::vector<llvm::Instruction*> *instruction_list, llvm::Instruction* instruction,llvm::DominatorTree *dominator_tree){
	bool success = false;
	for(auto * tmp_instruction:*instruction_list){
        if(instruction == nullptr)std::cerr << "ERROR";
		if(llvm::isPotentiallyReachable(tmp_instruction,instruction,dominator_tree,nullptr)){
			success = true;
			break;
		}
	}

	return success;
}

//return true if instruction is dominated by all instructions from the list
bool validate_instructions_dominance(std::vector<llvm::Instruction*> *instruction_list, llvm::Instruction* instruction,llvm::DominatorTree *dominator_tree){
	bool success = true;
	for(auto * tmp_instruction:*instruction_list){
		if(!dominator_tree->dominates(tmp_instruction,instruction)){
			success = false;
			break;
		}
	}

	return success;
}


int bfs_level(std::vector<std::tuple<int, llvm::BasicBlock*>>*bfs_reference , llvm::Instruction*  instruction ) {
	for(auto& tuple :*bfs_reference){
		llvm::BasicBlock* bb = std::get<llvm::BasicBlock*>(tuple);
		if(bb->getName() == instruction->getParent()->getName()){
			return std::get<int>(tuple);;
		}
	}
	return -1;
}



//function to create all abbs in the graph
//TODO adapt branches backwards if they merge with a branch with lower level
std::vector<std::tuple<int, llvm::BasicBlock*>> generate_bfs(llvm::Function*  function ) {
	
	std::hash<std::string> hash_fn;
	

	std::vector<std::tuple<int,llvm::BasicBlock*>> bfs_reference;
    //get first basic block of the function

    //create ABB
	llvm::BasicBlock& entry_bb = function->getEntryBlock();
	
    //store coresponding basic block in ABB
    //queue for new created ABBs
    std::queue<std::tuple<int, llvm::BasicBlock*>> queue; 
	
	auto tmp = std::make_tuple (0,&entry_bb);
	
	
	bfs_reference.push_back(tmp);
    queue.push(tmp);

    //queue with information, which abbs were already analyzed
    std::vector<size_t> visited_bbs;

    //iterate about the ABB queue
	

    while(!queue.empty()) {

		//get first element of the queue
		auto tuple = queue.front();
		queue.pop();
		auto old_bb = std::get<llvm::BasicBlock*>(tuple);
		auto old_bb_level = std::get<int>(tuple);
        queue.front();

		//iterate about the successors of the abb
		for (auto it = succ_begin(old_bb); it != succ_end(old_bb); ++it){

			//get sucessor basicblock reference
			llvm::BasicBlock *succ = *it;
			
			bool visited =false;
			
			size_t succ_seed =  hash_fn(succ->getName().str());
			
			for(auto seed : visited_bbs){
				if(seed ==  succ_seed)visited = true;
				
			}
			
			//check if the successor abb is already stored in the list				
			if(!visited) {
				auto tmp = std::make_tuple (old_bb_level+1,succ);
				//update the lists
				queue.push(tmp);
				bfs_reference.push_back(tmp);
				visited_bbs.push_back(succ_seed);
            }
        }
    }
    return bfs_reference;
}


bool is_reachable(std::vector<llvm::Instruction*> *start_scheduler_func_calls, llvm::BasicBlock *bb, std::vector<size_t> *already_visited){
	
	std::hash<std::string> hash_fn;
	size_t hash_value = hash_fn(bb->getName().str());
	for(auto hash : *already_visited){
		if(hash_value == hash){
			//basic block already visited
			return false;
		}
	}
	//insert bb in alread visited
	already_visited->emplace_back(hash_value);
	
	for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
		for(llvm::Instruction* start_scheduler : *start_scheduler_func_calls){
			llvm::Instruction* instr = &*i;
			if(instr == start_scheduler)return true;
		}
	}

	for (auto it = pred_begin(bb), et = pred_end(bb); it != et; ++it){
		if(is_reachable(start_scheduler_func_calls, *it, already_visited))return true;
	}
	return false;
}


bool verify_instruction_order(llvm::Instruction* expected_front, llvm::Instruction* expected_back,std::vector<std::tuple<int, llvm::BasicBlock*>>*bfs_reference){
	
	if(bfs_level( bfs_reference ,expected_front) <  bfs_level( bfs_reference ,expected_back) )return true;
	else return false;
}




void update_called_functions(graph::Graph& graph,OS::shared_function function,std::vector<std::size_t> *already_visited ,start_scheduler_relation state){
    
	//check if valid function pointer was committed
	if(function == nullptr) return;
		
	std::hash<std::string> hash_fn;

	size_t hash_value = hash_fn(function->get_name());
	
	//search hash value in list of already visited basic blocks
	for(auto hash : *already_visited){
		if(hash_value == hash){
			//function already visited
			return;
		}
	}
	
	for(auto & abb : function->get_atomic_basic_blocks()){
		abb->set_start_scheduler_relation(state);
	}
	
	
	//set function as already visited
	already_visited->emplace_back(hash_value);
    
    auto called_functions =  function->get_called_functions();
    //check if function has no calling functions
    
    //push the calling function on the stack
    for (auto called_function: called_functions){
       update_called_functions(graph,called_function,already_visited , state);
    }
}


void recursive_before_scheduler_instructions(graph::Graph& graph,llvm::Instruction* instr,start_scheduler_relation& state,std::vector<std::size_t> *already_visited,std::vector<llvm::Instruction*>* start_scheduler_func_calls,std::vector<llvm::Instruction*>* uncertain_start_scheduler_func_calls){
    
    std::hash<std::string> hash_fn;
    
    llvm::Function* called_function = nullptr;
    if(llvm::CallInst* call_instruction = dyn_cast<CallInst>(instr)){
        called_function = call_instruction->getCalledFunction();
    }else if(llvm::InvokeInst* call_instruction = dyn_cast<InvokeInst>(instr)){
        called_function = call_instruction->getCalledFunction();
    }
   
    start_scheduler_relation result;
   
    if(called_function != nullptr){
        graph::shared_vertex target_vertex = graph.get_vertex( hash_fn(called_function->getName().str() +  typeid(OS::Function).name())); 
        auto target_function = std::dynamic_pointer_cast<OS::Function>(target_vertex);
        result = before_scheduler_instructions(graph,target_function  ,already_visited);
        
        
        if(result == after){
            //in the called function a start scheduler instruction is inevitably executed
            start_scheduler_func_calls->emplace_back(instr);
            if (state == before)state = uncertain; 
        }
        else{
            if (result == uncertain){
                //in the called function a start scheduler instruction is executed, but not inevitably
                uncertain_start_scheduler_func_calls->emplace_back(instr);
                //if the current state of the function is before and the result of the called function is uncertain, then set state to uncertain 
                if (state == before)state = uncertain; 
            }
        }
    }
}

//TODO case: where in func call uncertain start scheduler exists
start_scheduler_relation before_scheduler_instructions(graph::Graph& graph,OS::shared_function function,std::vector<std::size_t> *already_visited){
    
	//default start scheduler relation state of the function 
	start_scheduler_relation state = uncertain;
	
	//check if valid function pointer was committed
	if(function == nullptr) return not_defined;
	
    
	std::vector<llvm::Instruction*> start_scheduler_func_calls;
	std::vector<llvm::Instruction*> uncertain_start_scheduler_func_calls;
	std::vector<llvm::Instruction*> return_instructions;
		
	std::hash<std::string> hash_fn;
    
    
    //generate hash value of the functionname
	size_t hash_value = hash_fn(function->get_name());
	
	//search hash value in list of already visited basic blocks
	for(auto hash : *already_visited){
		if(hash_value == hash){
			//function already visited
			return not_defined;
		}
	}
	//set function as already visited 
	already_visited->emplace_back(hash_value);
	
	//get all start scheduler instructions of the current function (not the subfunctions)
	start_scheduler_func_calls = get_start_scheduler_instruction(function);
	
	//generate dominator tree
	llvm::DominatorTree* dominator_tree = function->get_dominator_tree();
	
	//check if a start scheduler call was found in current function
	if(start_scheduler_func_calls.size() == 0){
        //set scheduler relation state of function to before
		state = before;
	}else{
        
        //get all return instructions of the function and store them
		for(auto &abb : function->get_atomic_basic_blocks()){
			for(llvm::BasicBlock* bb : *abb->get_BasicBlocks()){
				for(auto &instr:*bb){
					if(isa<ReturnInst>(instr)){
						return_instructions.emplace_back(&instr);
					}
				}
			}
		}
	}
	
	//iterate about abbs which contain a func call and detect function calls and execute the recursive analysis for the called function
	for(auto &abb : function->get_atomic_basic_blocks()){
		for(llvm::BasicBlock* bb : *abb->get_BasicBlocks()){
            for(auto &instruction:*bb){
                for(auto *instr :abb->get_call_instruction_references()){
                    //start the recursion for the called function
                    recursive_before_scheduler_instructions( graph,instr, state,already_visited,&start_scheduler_func_calls,&uncertain_start_scheduler_func_calls);
                }
			}
		}
	}
	
	//check if certain und uncertain start scheduler calls exists
	if(state ==uncertain){
		for(auto &abb : function->get_atomic_basic_blocks()){
            bool before_flag = false;
            bool uncertain_flag = false;
			for(llvm::BasicBlock* bb : *abb->get_BasicBlocks()){
                for(auto &instruction:*bb){
                    //check if the instruction is not reachable from all certain and uncertain start scheduler instructions
                    if(!validate_instructions_reachability(&start_scheduler_func_calls,&instruction, dominator_tree ) && !validate_instructions_reachability(&uncertain_start_scheduler_func_calls,&instruction, dominator_tree ) ){
                         //instruction is not reachable from a uncertain or certain start scheduler instruction 
                        before_flag = true;
                        
                    }else{
                        before_flag = false;
                        //check if the instruction is reachable from a uncertain start scheduler instruction
                        if(validate_instructions_reachability(&uncertain_start_scheduler_func_calls,&instruction,dominator_tree)){
                            //instruction is reachable from a uncertain start scheduler instruction 
                            uncertain_flag = true;
                        }
                    }
                }
			}
			if(before_flag)abb->set_start_scheduler_relation(before);
            else if(uncertain_flag)abb->set_start_scheduler_relation(uncertain);
		}
		
		bool flag = true;
		for(auto instr : return_instructions){
            //check if return instruction is dominated by all certain start scheduler instructions
			if(!validate_instructions_dominance(&start_scheduler_func_calls,instr, dominator_tree ))flag = false;
		}
		
		//state is after if all retrun instructions were dominated by a certain start scheduler instruction
		if(flag)state = after;
	}
	
    std::vector<std::size_t> tmp_already_visited;
    //iterate about the abbs of the function
    for(auto &abb : function->get_atomic_basic_blocks()){
        
        if(abb->get_start_scheduler_relation() == after){
            //iterate about the called functions of the abb
            for (auto called_function : abb->get_called_functions()){
                
                //set all abbs, which are reachable from a after start scheduler abb from the main function, to after start scheduler 
                update_called_functions(graph, called_function  ,&tmp_already_visited,after);
            }
        }
    }
	
	return state;
	
}



//function to extract the handler name of the instance
std::string get_handler_name(llvm::Instruction * instruction, unsigned int argument_index){
	
	if(instruction == nullptr)std::cerr << "ERROR" << std::endl;
	
	std::string handler_name = "";	
	//check if call instruction has one user
	if(instruction->hasOneUse()){
		//get the user of the call instruction
		llvm::User* user = instruction->user_back();
		//check if user is store instruction
		if(isa<StoreInst>(user)){
			//get name of specific operand (-> handler name)
			Value * operand = user->getOperand(argument_index);
			handler_name = operand->getName().str();
		}
		else if(isa<BitCastInst>(user)){
			instruction = cast<Instruction>(user);
			handler_name = get_handler_name(instruction, argument_index);
		}
	}
	
	if(handler_name == "")std::cerr << "ERROR no handler name" << std::endl;
	return handler_name;
}

bool validate_loop(llvm::BasicBlock *bb, std::vector<std::size_t>* already_visited){
    
	//generate hash code of basic block name
	std::hash<std::string> hash_fn;
	size_t hash_value = hash_fn(bb->getName().str());

    
	//search hash value in list of already visited basic blocks
	for(auto hash : *already_visited){
		if(hash_value == hash){
			//basic block already visited
			return true;
			break;
		}
	}
	//set basic block hash value in already visited list
	already_visited->push_back(hash_value);
	bool success = true;
	//search loop of function
	llvm::DominatorTree DT = llvm::DominatorTree();
	DT.recalculate(*bb->getParent());
	DT.updateDFSNumbers();
	llvm::LoopInfoBase<llvm::BasicBlock, llvm::Loop>LIB;
	LIB.analyze(DT);
	llvm::Loop * L = LIB.getLoopFor(bb);

	//check if basic block is in loop
	//TODO get loop count
	if(L != nullptr){
        
        /*
		AssumptionCache AC = AssumptionCache(*bb->getParent());
		Triple ModuleTriple(llvm::sys::getDefaultTargetTriple());
		TargetLibraryInfoImpl TLII(ModuleTriple);
		//TODO check behavoiur
		TLII.disableAllFunctions();
		TargetLibraryInfoWrapperPass TLI = TargetLibraryInfoWrapperPass(TLII);
		//TODO getExitBlock - If getExitBlocks would return exactly one block, return that block.
		LoopInfo LI = LoopInfo(DT);
		LI.analyze (DT);
		ScalarEvolution SE = ScalarEvolution(*bb->getParent(), TLI.getTLI(),AC, DT, LI);
		//SE.verify();
		SmallVector<BasicBlock *,10> blocks;
		if(L->getExitingBlock()==nullptr){
			//std::cout << "loop has more or no exiting blocks" << std::endl;
		}
		else{
			//std::cout << "loop has one exiting block" << std::endl;
			//const SCEVConstant *ExitCount = dyn_cast<SCEVConstant>(getExitCount(L, ExitingBlock));
			//std::cout << "finaler Test" << SE.getSmallConstantTripCount (L, L->getExitingBlock()) << std::endl;
		}
		//auto  blocks;
		//llvm::SmallVectorImpl< llvm::BasicBlock *> blocks = llvm::SmallVectorImpl< llvm::BasicBlock *> ();
		//L->getUniqueExitBlocks(blocks);
		*/
        
		success = false;
        
	}else{
		//check if function of basic block is called in a loop of other function
		for(auto user : bb->getParent()->users()){  // U is of type User*
			if(CallInst* instruction = dyn_cast<CallInst>(user)){
				//analyse basic block of call instruction 
				success = validate_loop(instruction->getParent() ,already_visited);
				if(success == false)break;
			}
		}
	}
	return success;
}

call_data get_syscall_relative_arguments(std::vector<argument_data>* arguments,std::vector<llvm::Instruction*>*call_references,llvm::Instruction* call_instruction_reference){
    call_data specific_call_data;
    
    specific_call_data.sys_call = true;
    
    specific_call_data.call_instruction = call_instruction_reference;
    
    for(auto argument : *arguments){
        argument_data tmp_argument;
        std::any any_value;
        llvm::Value* llvm_reference;
        argument_data specific_argument;
        get_call_relative_argument(any_value,llvm_reference, argument,call_references);
                
        
        tmp_argument.any_list.emplace_back(any_value);
        tmp_argument.value_list.emplace_back(llvm_reference);
        specific_argument.multiple = false;
        specific_call_data.arguments.emplace_back(tmp_argument);
        
    }
    return specific_call_data;
}


void get_call_relative_argument(std::any &any_value,llvm::Value* &llvm_value,argument_data argument,std::vector<llvm::Instruction*>*call_references){
   
    if( argument.any_list.size() == 0)return;
    
    //check if multiple argument values are possible
    if(argument.multiple ==false){
        any_value =  argument.any_list.front();
        llvm_value =  argument.value_list.front();
        return;
    }
    else{
        std::vector<std::tuple<std::any,llvm::Value*,std::vector<char>>> valid_candidates;
        char index = 0;
        for(auto argument_calles :argument.argument_calles_list){

            auto tmp_argument_calles = argument_calles;
            //erase first call, not necassary in evaluation
            tmp_argument_calles.erase(tmp_argument_calles.begin());
            std::vector<char> missmatch_list;
            char missmatches = 0;
            for(auto call_reference : *call_references){
               
               
                
                if(call_reference == tmp_argument_calles.front()){
                    tmp_argument_calles.erase(tmp_argument_calles.begin());
                    missmatch_list.emplace_back(missmatches);
                    missmatches = 0;
                }
                else ++missmatches;
            }
            if(tmp_argument_calles.empty())valid_candidates.emplace_back( std::make_tuple(argument.any_list.at(index),argument.value_list.at(index),missmatch_list));
            ++index;
        }
        if(valid_candidates.size() == 1){
            
            any_value = std::get<std::any>(valid_candidates.front());
            llvm_value = std::get<llvm::Value*>(valid_candidates.front());;
            return;
        }else{
            if(valid_candidates.size() == 0){
                 std::cerr << "no argument values are possible"<< std::endl;
            }else{
                //test_debug_argument(argument);
                llvm::Value* old_value = std::get<llvm::Value*>(valid_candidates.front());
                bool success = true;
                for(auto data : valid_candidates){
                    if(old_value !=std::get<llvm::Value*>(data)){
                        success = false;
                        break;
                    }
                }
                if(success){
                    any_value = std::get<std::any>(valid_candidates.front());
                    llvm_value = std::get<llvm::Value*>(valid_candidates.front());
                    //llvm_value = argument.value_list.front();;
                    //any_value = argument.any_list.front();
                    return;
                    
                }else{
                    //std::cerr << "multiple argument values are possible" <<  valid_candidates.size() << std::endl;
                    //TODO select the best 
                }
            }
        }
    }
    any_value = (std::string) "multiple argument values are possible";
    llvm_value = nullptr;
}



//xTaskCreate
graph::shared_vertex create_task(graph::Graph& graph,OS::shared_abb abb, bool before_scheduler_start,std::vector<llvm::Instruction*>*call_references ){
	
	
	llvm::Instruction* instruction = abb->get_syscall_instruction_reference();
	
	
	//create reference list for all arguments types of the task creation syscall
	std::vector<argument_data>argument_list;
    
    
	for(auto & argument : abb->get_syscall_arguments()){
		argument_list.emplace_back(argument);
	}
    std::any specific_argument;
    llvm::Value* argument_reference;
	//load the arguments
	argument_data argument= argument_list.at(0);
	
	get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	std::string function_reference_name =  std::any_cast<std::string>(specific_argument);
	
	argument = argument_list.at(1);
	get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	std::string task_name =  std::any_cast<std::string>(specific_argument);
	
	argument = argument_list.at(2);
	get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	unsigned long stacksize =  std::any_cast<long>(specific_argument);
	
	argument = argument_list.at(3);
	get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	std::string task_argument =  std::any_cast<std::string>(specific_argument);
	
	argument = argument_list.at(4);
	get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	unsigned long priority =  std::any_cast<long>(specific_argument);
	
	argument = argument_list.at(5);
	get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	std::string handler_name =  std::any_cast<std::string>(specific_argument);
	
    //if no handler name was transmitted
    if(handler_name == "&$%NULL&$%"){
        handler_name =function_reference_name;
    }
	
	//create task and set properties
	auto task = std::make_shared<OS::Task>(&graph,task_name);
	task->set_handler_name( handler_name);
	task->set_stacksize( stacksize);
	task->set_priority( priority);
	task->set_start_scheduler_creation_flag(before_scheduler_start);
	graph.set_vertex(task);
	
    
    if(!task->set_definition_function(function_reference_name)){
        std::cerr << "ERROR setting defintion function!" << std::endl;
        abort();
    }
    
	std::hash<std::string> hash_fn;
	
	graph::shared_vertex vertex = nullptr;
	vertex =  graph.get_vertex(hash_fn(function_reference_name +  typeid(OS::Function).name()));

	if(vertex != nullptr){
		auto function_reference = std::dynamic_pointer_cast<OS::Function> (vertex);
		function_reference->set_definition_vertex(task);
	}else{
		std::cerr << "ERROR task definition function does not exist " << function_reference_name << std::endl;
		abort();
	}
	
	std::cout << "task successfully created"<< std::endl;
   
    
	return task;
}





//..Semaphore...Create
//{ binary, counting, mutex, recursive_mutex }semaphore_type;
graph::shared_vertex create_semaphore(graph::Graph& graph,OS::shared_abb abb,semaphore_type type , bool before_scheduler_start,std::vector<llvm::Instruction*>* call_references ){
	
    bool success = false;
    
	llvm::Instruction* instruction = abb->get_syscall_instruction_reference();
		
	//create reference list for all arguments types of the task creation syscall
	std::vector<argument_data>argument_list;
	
	for(auto & argument : abb->get_syscall_arguments()){
		argument_list.emplace_back(argument);
	}
	
	std::string handler_name = get_handler_name(instruction, 1);
	
	auto semaphore = std::make_shared<OS::Semaphore>(&graph,handler_name);
	semaphore->set_semaphore_type(type);
	semaphore->set_handler_name(handler_name);
	semaphore->set_start_scheduler_creation_flag(before_scheduler_start);
	
	
	//std::cout << "semaphore handler name: " <<  handler_name << std::endl;
	switch(type){
		
		case binary:{
            
            success = true;
            
			std::cout << "binary semaphore successfully created"<< std::endl;
			break;
		}
		
		case counting:{
            
			success = true;
            std::any specific_argument;
            llvm::Value* argument_reference;
	
			//load the arguments
			argument_data argument  = argument_list.at(1);
            get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
			unsigned long initial_count =  std::any_cast<long>(specific_argument);
			
			argument  = argument_list.at(0);
			get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
			unsigned long max_count =  std::any_cast<long>(specific_argument);
			

			semaphore->set_initial_count(initial_count);
			semaphore->set_max_count(max_count);

			std::cout << "counting semaphore successfully created"<< std::endl;
			
			break;
		}
		
		
		case mutex:{
			
			success = true;
            std::any specific_argument;
            llvm::Value* argument_reference;
			//load the arguments
			argument_data argument  = argument_list.at(0);
			get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
			unsigned long mutex_type =  std::any_cast<long>(specific_argument);
			
			//set the mutex type (mutex, recursive mutex)
			type = (semaphore_type) mutex_type;
			
			semaphore->set_semaphore_type(type);
			if(type == mutex)std::cout << "mutex successfully created"<< std::endl;
			if(type == recursive_mutex)std::cout << "recursive mutex successfully created"<< std::endl;

			break;
		
		}
		default:{
			std::cout << "wrong semaphore type" << std::endl;
			break;
		}
	}
	if(success){
        graph.set_vertex(semaphore);
        return semaphore;
    }else{
        return nullptr;
    }
}

//xQueueCreate
graph::shared_vertex create_queue(graph::Graph& graph, OS::shared_abb abb ,bool before_scheduler_start,std::vector<llvm::Instruction*>* call_references){
	
	llvm::Instruction* instruction = abb->get_syscall_instruction_reference();
	
	
	//create reference list for all arguments types of the task creation syscall
	std::vector<argument_data>argument_list;
	
	for(auto & argument : abb->get_syscall_arguments()){
		argument_list.emplace_back(argument);
	}
	
    std::any specific_argument;
    llvm::Value* argument_reference;
	
	//load the arguments
	argument_data argument   = argument_list.at(0);
    
    abb->print_information();
    get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	long queue_length =  std::any_cast<long>(specific_argument);
	
	argument  = argument_list.at(1);
    get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
    long item_size =  std::any_cast<long>(specific_argument);

	argument  = argument_list.at(2);
    get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	long queue_type =  std::any_cast<long>(specific_argument);
	
	semaphore_type type = (semaphore_type) queue_type;
	
    graph::shared_vertex vertex = nullptr;
    
	if(type != binary){
		
		std::string handler_name = get_handler_name(instruction, 1);
		
		
		//create queue and set properties
		auto queue = std::make_shared<OS::Queue>(&graph,handler_name);
		
		queue->set_handler_name(handler_name);
		queue->set_length(queue_length);
		queue->set_item_size(item_size);
		queue->set_start_scheduler_creation_flag(before_scheduler_start);
		graph.set_vertex(queue);
       
        vertex = queue;
		
		std::cout << "queue successfully created"<< std::endl;
	}else{
		vertex = create_semaphore(graph,abb,binary, before_scheduler_start,call_references);
	}

	return vertex;
}


//xQueueCreate
graph::shared_vertex create_event_group(graph::Graph& graph,OS::shared_abb abb, bool before_scheduler_start,std::vector<llvm::Instruction*>* call_references){
	
	
	llvm::Instruction* instruction = abb->get_syscall_instruction_reference();
	
	
	//create reference list for all arguments types of the task creation syscall
	std::vector<argument_data>argument_list;
	
	for(auto & argument : abb->get_syscall_arguments()){
		argument_list.emplace_back(argument);
	}
	//create queue and set properties 
	std::string handler_name = get_handler_name(instruction, 1);
	auto event_group = std::make_shared<OS::EventGroup>(&graph,handler_name);
		
	//std::cerr <<  "EventGroupHandlerName" << handler_name << std::endl;
	event_group->set_handler_name(handler_name);
	event_group->set_start_scheduler_creation_flag(before_scheduler_start);
	graph.set_vertex(event_group);
	std::cout << "event group successfully created" <<  std::endl;
		
	
	return event_group;
}




//xQueueSet
graph::shared_vertex create_queue_set(graph::Graph& graph, OS::shared_abb abb,  bool before_scheduler_start,std::vector<llvm::Instruction*>* call_references){
	

	llvm::Instruction* instruction = abb->get_syscall_instruction_reference();
	
	
	//create reference list for all arguments types of the task creation syscall
	std::vector<argument_data>argument_list;
	
	for(auto & argument : abb->get_syscall_arguments()){
		argument_list.emplace_back(argument);
	}
	
    std::any specific_argument;
    llvm::Value* argument_reference;
	
	//load the arguments
    argument_data argument   = argument_list.at(0);
    get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	unsigned long queue_set_size =  std::any_cast<long>(specific_argument);
	

	//create queue set and set properties 
	std::string handler_name = get_handler_name(instruction, 1);
	auto queue_set = std::make_shared<OS::QueueSet>(&graph,handler_name);
	
	queue_set->set_handler_name(handler_name);
	queue_set->set_length(queue_set_size);
	
	std::cout << "queue set successfully created"<< std::endl;
	//set timer to graph
	queue_set->set_start_scheduler_creation_flag(before_scheduler_start);
	graph.set_vertex(queue_set);
	
	return queue_set;
}

//xTimerCreate
graph::shared_vertex create_timer(graph::Graph& graph,OS::shared_abb abb, bool before_scheduler_start,std::vector<llvm::Instruction*>* call_references){
	
	
    abb->print_information();
	std::vector<argument_data>argument_list;
	
	for(auto & argument : abb->get_syscall_arguments()){
		argument_list.emplace_back(argument);
	}
	
    std::any specific_argument;
    llvm::Value* argument_reference;
	
	//load the arguments
	argument_data argument   = argument_list.at(0);
    get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	std::string timer_name =  std::any_cast<std::string>(specific_argument);
	
	argument  = argument_list.at(1);
    get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	long timer_periode =  std::any_cast<long>(specific_argument);

	argument  = argument_list.at(2);
    get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	long timer_autoreload =  std::any_cast<long>(specific_argument);
	
	argument  = argument_list.at(3);
    get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	std::string timer_id =  std::any_cast<std::string>(specific_argument);
	
	argument  = argument_list.at(4);
    get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	std::string timer_definition_function =  std::any_cast<std::string>(specific_argument);
	
	//create timer and set properties 
	auto timer = std::make_shared<OS::Timer>(&graph,timer_name);
	
	timer->set_periode(timer_periode);
	//TODO extract timer id
	//std::cout << timer_id << std::endl;
	//timer->set_timer_id(timer_id);
    
	if(timer_autoreload == 0) timer->set_timer_type(oneshot);
	else timer->set_timer_type(autoreload);
	std::cout << "timer successfully created"<< std::endl;
	//set timer to graph
	timer->set_start_scheduler_creation_flag(before_scheduler_start);
	graph.set_vertex(timer);
    
    std::cerr << "timer callback function " <<timer_definition_function << std::endl;
	timer->set_definition_function(timer_definition_function);
    
	return timer;
}


//xTimerCreate
graph::shared_vertex create_buffer(graph::Graph& graph,OS::shared_abb abb, bool before_scheduler_start,std::vector<llvm::Instruction*>* call_references){
	
	
	llvm::Instruction* instruction = abb->get_syscall_instruction_reference();
	//create reference list for all arguments types of the task creation syscall
	

	//get the typeid hashcode of the expected arguments
		std::vector<argument_data>argument_list;
	
	for(auto & argument : abb->get_syscall_arguments()){
		argument_list.emplace_back(argument);
	}
	
	std::any specific_argument;
    llvm::Value* argument_reference;
			
	//load the arguments
	argument_data argument   = argument_list.at(2);
    get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	buffer_type type = (buffer_type) std::any_cast<long>(specific_argument);
	
	//std::cout << "buffer type: "<< std::any_cast<long>(argument)<< std::endl;
	
	argument  = argument_list.at(1);
    get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	long trigger_level =  std::any_cast<long>(specific_argument);

	argument  = argument_list.at(0);
    get_call_relative_argument(specific_argument, argument_reference,argument,call_references);
	long buffer_size =  std::any_cast<long>(specific_argument);
	
	//create timer and set properties 
	//create queue set and set properties 
	std::string handler_name = get_handler_name(instruction, 1);
	auto buffer = std::make_shared<OS::Buffer>(&graph,handler_name);
	
	
	buffer->set_buffer_size(buffer_size);
	buffer->set_trigger_level(trigger_level);
	buffer->set_handler_name(handler_name);
	
	buffer->set_buffer_type(type);
	
	std::cout << "buffer successfully created"<< std::endl;
	//set timer to graph
	buffer->set_start_scheduler_creation_flag(before_scheduler_start);
	graph.set_vertex(buffer);
	
	return buffer;
}



// bool verify_isr_prefix(llvm::Function *function){
// 	bool success = false;
// 	std::string type_str;
// 	llvm::raw_string_ostream rso(type_str);
// 	function->print(rso);
// 	std::stringstream ss(rso.str());
// 	std::string line;
// 	int line_ctr = 0;
// 	while(std::getline(ss,line,'\n') && line_ctr < 3){
// 		if(line.find("x86_intrcc")!=std::string::npos)success =true;
// 		line_ctr++;
// 	}
// 	return success;
// }
	
//detect isrs
bool detect_isrs(graph::Graph& graph){
	
    //iterate about the abbs
    std::vector<size_t> visited_functions;
    
    std::list<graph::shared_vertex> vertex_list =  graph.get_type_vertices(typeid(OS::ABB).hash_code());

    for (auto &vertex : vertex_list) {
        
        //cast vertex to abb 
        auto abb = std::dynamic_pointer_cast<OS::ABB> (vertex);
        //check if syscall is isr specific 
        if(abb->get_syscall_name().find("FromISR") != std::string::npos){
            
            //std::cerr << abb->get_syscall_name() << std::endl;
            
           
            bool success = false;
               //queue for functions
            std::stack<OS::shared_function> stack; 
            
            
            stack.push(abb->get_parent_function());
            //iterate about the stack 
            while(stack.size()!=0) {
                
                //get first element of the queue
                OS::shared_function function = stack.top();
                stack.pop();
                //check if the function was already visited by DFS
                size_t seed =  function->get_seed();
                for(auto tmp_seed : visited_functions){
                    if(seed == tmp_seed)continue;
                }
                //set the function to the already visited functions
                visited_functions.emplace_back(seed);

                //get the calling functions of the function
                auto calling_functions =  function->get_calling_functions();
                //check if function has no calling functions
                if(calling_functions.size() == 0){
                    
                    std::string isr_name = function->get_name();
                    auto isr = std::make_shared<OS::ISR>(&graph,isr_name);
                    graph.set_vertex(isr);
                    isr->set_definition_function(function->get_name());
                    isr->set_handler_name(function->get_name());
                    
                    success = true;
                    std::cerr << "isr successfully created" << std::endl;
                    
                }else{
                    //push the calling function on the stack
                    for (auto calling_function: calling_functions){
                        stack.push(calling_function);
                    }
                }
            }
            if(!success)std::cerr << "isr could not created, because of recursion" << std::endl;
        }
    }
}


bool create_abstraction_instance(graph::Graph& graph,graph::shared_vertex start_vertex,OS::shared_abb abb,bool before_scheduler_start,std::vector<llvm::Instruction*>* already_visited_calls){

    graph::shared_vertex created_vertex;
    //check which target should be generated
    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::Task).hash_code())){
        //std::cout << "TASKCREATE" << name << ":" << tmp << std::endl;
        created_vertex = create_task(graph,abb,before_scheduler_start,already_visited_calls);
        if(!created_vertex)std::cout << "Task could not created" << std::endl;
    }
    
        
    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::Queue).hash_code())){
        created_vertex = create_queue( graph,abb,before_scheduler_start,already_visited_calls);
        if(!created_vertex)std::cout << "Queue could not created" << std::endl;

    }
    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::Semaphore).hash_code())){
        //set semaphore
        semaphore_type type;
        std::string syscall_name = abb->get_syscall_name();
        if(syscall_name =="xQueueCreateMutex")type = mutex;
        if(syscall_name =="xSemaphoreCreateRecursiveMutex")type = recursive_mutex;
        if(syscall_name =="xQueueCreateCountingSemaphore")type = counting;
        created_vertex = create_semaphore(graph, abb, type, before_scheduler_start,already_visited_calls);
        if(!created_vertex)std::cout << "CountingSemaphore/Mutex could not created" << std::endl;
        
    }						
    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::Timer).hash_code())){
        created_vertex = create_timer( graph,abb,before_scheduler_start,already_visited_calls);
        if(!created_vertex)std::cout << "Timer could not created" << std::endl;
      
    }

    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::EventGroup).hash_code())){
        created_vertex = create_event_group(graph, abb,before_scheduler_start,already_visited_calls);
        if(!created_vertex)std::cout << "Event Group could not created" << std::endl;
    }
    
    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::Buffer).hash_code())){
        //std::cout << callname << std::endl;
        created_vertex = create_buffer(graph, abb,before_scheduler_start,already_visited_calls);
        if(!created_vertex)std::cout << "Buffer could not created" << std::endl;
    }
    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::QueueSet).hash_code())){
        //std::cout << callname << std::endl;
        created_vertex = create_queue_set(graph, abb,before_scheduler_start,already_visited_calls);
        if(!created_vertex)std::cout << "Queue Set could not created" << std::endl;
    }
    
    if(created_vertex){
        auto edge = std::make_shared<graph::Edge>(&graph,abb->get_syscall_name(), start_vertex,created_vertex,abb);
        //store the edge in the graph
        graph.set_edge(edge);
        
        start_vertex->set_outgoing_edge(edge);
        created_vertex->set_ingoing_edge(edge);
        edge->set_instruction_reference( abb->get_syscall_instruction_reference());
        auto arguments = abb->get_syscall_arguments();
        auto syscall_reference = abb->get_syscall_instruction_reference();
        auto specific_arguments=  get_syscall_relative_arguments( &arguments, already_visited_calls,syscall_reference);
        edge->set_specific_call(&specific_arguments);
        return true;
    }else{
        return false;
    }
}


void iterate_called_functions(graph::Graph& graph, graph::shared_vertex start_vertex, OS::shared_function function, llvm::Instruction* call_reference ,std::vector<llvm::Instruction*>* already_visited_calls){
    
    //return if function does not contain a syscall
    if(function == nullptr || function->has_syscall() ==false)return;
    std::hash<std::string> hash_fn;
    
	//search hash value in list of already visited basic blocks
	for(auto tmp_call : *already_visited_calls){
		if(call_reference == tmp_call){
			//basic block already visited
			return;
		}
	}
    already_visited_calls->emplace_back(call_reference);

    //get the abbs of the function
    std::list<OS::shared_abb> abb_list = function->get_atomic_basic_blocks();
    
    //iterate about the abbs
    for(auto &abb : abb_list){
        
        //get the list of referenced llvm bb
        std::vector<llvm::BasicBlock*>* llvm_bbs = abb->get_BasicBlocks();
            
        //check if abb has a syscall instruction
        if( abb->get_call_type()== sys_call){
            
            std::vector<std::size_t> already_visited;

            //validate if sysccall is in loop
            if(validate_loop(llvm_bbs->front(),&already_visited)){
            
                bool before_scheduler_start = false;
                
                if(abb->get_start_scheduler_relation() == before)before_scheduler_start = true;
                                        
                //check if abb syscall is creation syscall
                if(abb->get_syscall_type() == create){
                    
                    if(!create_abstraction_instance( graph,start_vertex,abb,before_scheduler_start,already_visited_calls)){
                        std::cerr << "instance could not created" << std::endl;
                        abb->print_information();
                    }
                    
                }
            }
        }
        
        for(auto& edge : abb->get_outgoing_edges()){
            graph::shared_vertex vertex =edge->get_target_vertex();
            //std::cerr << "edge target " << vertex->get_name() << std::endl;
            if(typeid(OS::Function).hash_code() == vertex->get_type()){
                auto function = std::dynamic_pointer_cast<OS::Function> (vertex);
                iterate_called_functions(graph,start_vertex,function, edge->get_instruction_reference(),already_visited_calls);
                
            }
        }
    }
}


void sort_edges(std::list<graph::shared_edge>* unsorted_outgoing_edges,std::list<graph::shared_edge>* sorted_outgoing_edges,OS::shared_abb abb){
    
    for(auto bb :*abb->get_BasicBlocks()){
        std::vector<graph::shared_edge> tmp_edges;
        for(auto& edge :*unsorted_outgoing_edges){
            if(bb==edge->get_instruction_reference()->getParent()){
                tmp_edges.emplace_back(edge);
            }
        }
        if(tmp_edges.size()==1)sorted_outgoing_edges->emplace_back(tmp_edges.front());
        else{
            //TODO sort edges of bb
            //instruction_before( Instruction *InstA,  Instruction *InstB,DominatorTree *DT)
            
            for(auto edge :tmp_edges){
                sorted_outgoing_edges->emplace_back(edge);
            }
        }
    }
}

void sort_abbs(graph::Graph& graph){
    
    std::list<graph::shared_vertex> vertex_list =  graph.get_type_vertices(typeid(OS::Function).hash_code());
    
    for (auto &vertex : vertex_list) {
        
        
        std::cerr  << "---------------" <<  std::endl;
        std::list<llvm::BasicBlock*> tmp_topological_order;
        
        //cast vertex to abb 
        auto function = std::dynamic_pointer_cast<OS::Function> (vertex);
        //std::cerr << "------------------------------------------------------" << std::endl;
        
        // Use LLVM's Strongly Connected Components (SCCs) iterator to produce
        // a reverse topological sort of SCCs.
        //std::cerr << "SCCs for " << function->get_name() << " in post-order:\n";
        for (scc_iterator<Function *> I = scc_begin(function->get_llvm_reference()), IE = scc_end(function->get_llvm_reference());I != IE; ++I) {
            // Obtain the vector of BBs in this SCC and print it out.
            const std::vector<BasicBlock *> &SCCBBs = *I;
            for (std::vector<BasicBlock *>::const_iterator BBI = SCCBBs.begin(), BBIE = SCCBBs.end(); BBI != BBIE; ++BBI) {
                tmp_topological_order.emplace_front(*BBI);
            }
        }
        
        
        std::vector<llvm::BasicBlock*> topological_order;
         
        for(auto element : tmp_topological_order){
             //std::cerr  << "SCC: "<< element->getName().str() << std::endl;
             topological_order.emplace_back(element);
         }
        
        //generate dominator tree
        llvm::DominatorTree* dominator_tree = function->get_dominator_tree();
        
        std::string type_str;
        llvm::raw_string_ostream rso(type_str);
        //dominator_tree->print(rso);
        //std::cerr << rso.str() << std::endl ;
        
        std::vector<std::tuple<unsigned int , OS::shared_abb>> test;
        std::list<OS::shared_abb> abb_list = function->get_atomic_basic_blocks();
        std::list<OS::shared_abb> sorted_abb_list;
        int counter = 0;
        
        while(counter < topological_order.size()){
            
            auto order_reference = topological_order[counter];
            
            bool flag = false;
            //iterate about the abb list
            for(auto& abb: abb_list){
                
                std::vector<llvm::BasicBlock*>* unsorted_bb_list = abb->get_BasicBlocks();
                
                for(auto bb :*unsorted_bb_list){
                    if(order_reference ==bb)flag = true;
                }
                
                //check if the entry bb of the abb is equal to the topological bb reference
                if(flag){
                    
                    auto unsorted_outgoing_edges = abb->get_outgoing_edges();
                    std::list<graph::shared_edge> sorted_outgoing_edges;
                    //std::cerr  << "abb entry " <<  abb->get_name()  << std::endl;   
                    
                    sorted_abb_list.emplace_back(abb);
                    std::vector<llvm::BasicBlock*> sorted_bb_list(unsorted_bb_list->size());
                    
                    for(auto bb: *unsorted_bb_list){
                        
                        int tmp_counter = 0;
                        //std::cerr << "counter: " << counter << "tmpcounter: " << tmp_counter << "listsize: " << unsorted_bb_list->size() << std::endl;
                        bool tmp_flag = false;
                        while((counter+tmp_counter) < topological_order.size() && tmp_counter < sorted_bb_list.size()){                             
                            if(topological_order[counter + tmp_counter] ==bb){
//                                 //std::cerr  << "bb" <<  (bb->getName().str()) << std::endl;
                                sorted_bb_list.at(tmp_counter) = bb;
                                tmp_flag = true;
                                break;
                             }
                             ++tmp_counter;
                        }
                        
                        if(!tmp_flag)std::cerr << "not found"  << std::endl;
                    }
                    
                    counter += unsorted_bb_list->size();
                    
                    for(int i = 0; i < sorted_bb_list.size();++i){
                        if(!sorted_bb_list.at(i)){
                            //std::cerr  << (unsorted_bb_list->at(i)->getName().str()) << " : ERROR" <<  std::endl;
                            for(auto bb :*unsorted_bb_list){
                                //std::cerr  << "unsorted bb: " << bb->getName().str() << std::endl;
                            }
                            
                        }
                        //else std::cerr  << (unsorted_bb_list->at(i)->getName().str()) << " : " <<  (sorted_bb_list.at(i)->getName().str()) << std::endl;
                    }
                    //sort_edges(&unsorted_outgoing_edges,&sorted_outgoing_edges, abb);
                    
                    
                    break;
                }
            }
            if(!flag){
                std::cerr << "EEEEEEEEEEEEERRROR" << (order_reference->getName().str()) << std::endl;
                ++counter;
            }
        }
        if(abb_list.size() == sorted_abb_list.size()){
            function->set_atomic_basic_blocks(&sorted_abb_list);
            //std::cerr << "sorted and unsorted abb lists have equal length" << std::endl;
        }
        else{

            std::cerr << "sorted and unsorted abb lists have not equal length" << std::endl;
            if(abb_list.size() > sorted_abb_list.size() && function->get_exit_abb() != nullptr && function->get_exit_abb()->get_seed() != sorted_abb_list.back()->get_seed()){
                
                std::cerr  << "success" << std::endl;
                sorted_abb_list.emplace_back(function->get_exit_abb());
                function->set_atomic_basic_blocks(&sorted_abb_list);
            }else{
                for(auto abb :abb_list){
                    std::cerr  << "unsorted abb: " << abb->get_name() << std::endl;
                }
                for(auto abb :sorted_abb_list){
                    std::cerr  << "sorted abb: " << abb->get_name() << std::endl;
                }
                
            }
        }
    }
}


namespace step {

	std::string FreeRTOSInstancesStep::get_name() {
		return "FreeRTOSInstancesStep";
	}

	std::string FreeRTOSInstancesStep::get_description() {
		return "Extracts out of FreeRTOS abstraction instances";
	}

	void FreeRTOSInstancesStep::run(graph::Graph& graph) {
		
        sort_abbs(graph);
        
		std::cout << "Run " << get_name() << std::endl;
			
		std::hash<std::string> hash_fn;
		
		//get function with name main from graph
		std::string start_function_name = "main";  
		
		graph::shared_vertex main_vertex = graph.get_vertex( hash_fn(start_function_name +  typeid(OS::Function).name())); 
		
        OS::shared_function main_function;
        
		//check if graph contains main function
		if(main_vertex != nullptr){
			std::vector<std::size_t> already_visited;
			main_function = std::dynamic_pointer_cast<OS::Function>(main_vertex);
			before_scheduler_instructions(graph, main_function  ,&already_visited);
            
            //iterate about the main function context and detect abstraction instances
            std::vector<llvm::Instruction*> already_visited_calls;
            iterate_called_functions(graph, main_vertex , main_function,nullptr,&already_visited_calls);
		
            
        }else{
            std::cerr << "no main function in programm" << std::endl;
            abort();
        }
        
    
        detect_isrs(graph);
        
      
        //iterate about the isrs
        std::list<size_t> already_visited;
        bool flag = false;
        
        do{
            flag = false;
            
            //get all tasks, which are stored in the graph
            std::list<graph::shared_vertex> vertex_list =  graph.get_type_vertices(typeid(OS::Task).hash_code());
        
            for (auto &vertex : vertex_list) {
                
                if(list_contains_element(&already_visited, vertex->get_seed()))continue;
                else already_visited.emplace_back(vertex->get_seed());
                
                flag = true;
                
                //std::cerr << "task name: " << vertex->get_name() << std::endl;
                auto task = std::dynamic_pointer_cast<OS::Task> (vertex);
                OS::shared_function task_definition = task->get_definition_function();
                //get all interactions of the instance
                std::vector<llvm::Instruction*> already_visited_calls;
                iterate_called_functions(graph, task , task_definition, nullptr ,&already_visited_calls);
            }
        
        
        
            //get all isrs, which are stored in the graph
            vertex_list =  graph.get_type_vertices(typeid(OS::ISR).hash_code());
            //iterate about the isrs
            for (auto &vertex : vertex_list) {
                
                if(list_contains_element(&already_visited, vertex->get_seed()))continue;
                else already_visited.emplace_back(vertex->get_seed());
                
                flag = true;
                
                auto timer = std::dynamic_pointer_cast<OS::ISR> (vertex);
                OS::shared_function timer_definition = timer->get_definition_function();
                //get all interactions of the instance
                std::vector<llvm::Instruction*> already_visited_calls;
                iterate_called_functions(graph, timer , timer_definition, nullptr ,&already_visited_calls);
            }
            
            
        }while(flag);

	
        std::list<graph::shared_vertex> vertex_list =  graph.get_type_vertices(typeid(OS::Function).hash_code());
            
        for (auto &vertex : vertex_list) {
                
                auto function = std::dynamic_pointer_cast<OS::Function> (vertex);
                
                hook_type hook = no_hook;
                if(function->get_name().find("PostTaskHook") != std::string::npos){
                    hook = post_task;
                }else if(function->get_name().find("PreTaskHook") != std::string::npos){
                    hook = pre_task;
                }else if(function->get_name().find("ErrorHook") != std::string::npos){
                    hook = error;
                }else if(function->get_name().find("ShutdownHook") != std::string::npos){
                    hook = shut_down;
                }else if(function->get_name().find("StartupHook") != std::string::npos){
                    hook = start_up;
                }else if(function->get_name().find("vApplicationMallocFailedHook") != std::string::npos){
                    hook = failed;
                }else if(function->get_name().find("vApplicationIdleHook") != std::string::npos){
                    hook = idle;
                }else if(function->get_name().find("vApplicationStackOverflowHook") != std::string::npos){
                    hook = stack_overflow;
                }else if(function->get_name().find("vApplicationTickHook") != std::string::npos){
                    hook = tick;
                }
                
                if(hook != no_hook)function->set_hook_type(hook);
        }
    }
	
	
	std::vector<std::string> FreeRTOSInstancesStep::get_dependencies() {
		return {"ABB_MergeStep"};
	}
}
