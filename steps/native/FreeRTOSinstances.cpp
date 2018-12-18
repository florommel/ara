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

using namespace llvm;


//print the argument

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
            std::cerr << std::any_cast<int>(element)   <<'\n';
        }else if(tmp_double == tmp){ 
            std::cerr << std::any_cast<double>(element)  << '\n';
        }else if(tmp_string == tmp){
            std::cerr << std::any_cast<std::string>(element)  <<'\n';  
        }else if(tmp_long == tmp){
            std::cerr << std::any_cast<long>(element)   <<'\n';  
        }else{
            std::cerr << "[warning: cast not possible] type: " <<element.type().name()   <<'\n';  
        }
        std::cerr << ", ";
    }
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



//return false if instruction is  before scheduler start or and the scheduler start does  dominate the instruction
bool validate_start_scheduler_instruction_relations(std::vector<llvm::Instruction*> *start_scheduler_func_calls, llvm::Instruction* instruction,llvm::DominatorTree *dominator_tree){
	bool success = false;
	for(auto * start_scheduler:*start_scheduler_func_calls){
		if((dominator_tree->dominates(start_scheduler,instruction))){
			success = true;
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

//TODO case: where in func call uncertain start scheduler exists
//TODO BFS instead of DFS for basic block order
start_scheduler_relation before_scheduler_instructions(graph::Graph& graph,OS::shared_function function,std::vector<std::size_t> *already_visited){

	
	start_scheduler_relation state = uncertain;
	
	//check if valid function pointer was committed
	if(function == nullptr) return not_defined;
	
	std::vector<llvm::Instruction*> start_scheduler_func_calls;
	std::vector<llvm::Instruction*> uncertain_start_scheduler_func_calls;
	std::vector<llvm::Instruction*> return_instructions;
		
	std::hash<std::string> hash_fn;

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
	
	//get all start scheduler instructions of the function
	start_scheduler_func_calls = get_start_scheduler_instruction(function);
	
	//generate bfs order of basicblocks //TODO
	//std::vector<std::tuple<int, llvm::BasicBlock*>> bfs = generate_bfs(function->get_llvm_reference());
	
	//generate dominator tree
	llvm::DominatorTree* dominator_tree = function->get_dominator_tree();
	
	
	std::string type_str;
	llvm::raw_string_ostream rso(type_str);
	
	//check if scheduler start instruction was after in function
	if(start_scheduler_func_calls.size() == 0){
		state = before;
	}else{
		
	
		for(auto &abb : function->get_atomic_basic_blocks()){
			for(llvm::BasicBlock* bb : abb->get_BasicBlocks()){
				for(auto &instr:*bb){
					if(isa<ReturnInst>(instr)){
						//std::cerr << "return instruction after" << std::endl;
						return_instructions.emplace_back(&instr);
					}
				}
			}
		}
		
		for(auto& instruction:start_scheduler_func_calls){
			if(dominator_tree->dominates(  &function->get_llvm_reference()->getEntryBlock(),instruction->getParent()))state= after;
		}
	}
	
	//iterate about the basic blocks of the abb
	for(auto &abb : function->get_atomic_basic_blocks()){
			
		for(llvm::BasicBlock* bb : abb->get_BasicBlocks()){
			
			for(auto &instruction:*bb){
				if(abb->get_call_type()== func_call){
					start_scheduler_relation result;
					for(auto *instr :abb->get_call_instruction_references()){
						if(llvm::CallInst* call_instruction = dyn_cast<CallInst>(instr)){
							llvm::Function* called_function = call_instruction->getCalledFunction();
							if(called_function != nullptr){
								graph::shared_vertex target_vertex = graph.get_vertex( hash_fn(called_function->getName().str() +  typeid(OS::Function).name())); 
								auto target_function = std::dynamic_pointer_cast<OS::Function>(target_vertex);
								result = before_scheduler_instructions(graph,target_function  ,already_visited);
								if(result == after){
									start_scheduler_func_calls.emplace_back(call_instruction);
									//state = after;
								}
								else{
									if (result == uncertain){
                                        uncertain_start_scheduler_func_calls.emplace_back(call_instruction);
                                        if (state != after)state = uncertain; 
                                    }
								}
							}
						}else if(llvm::InvokeInst* call_instruction = dyn_cast<InvokeInst>(instr)){
							llvm::Function* called_function = call_instruction->getCalledFunction();
							if(called_function != nullptr){
								graph::shared_vertex target_vertex = graph.get_vertex( hash_fn(called_function->getName().str() +  typeid(OS::Function).name())); 
								auto target_function = std::dynamic_pointer_cast<OS::Function>(target_vertex);
								result = before_scheduler_instructions(graph,target_function  ,already_visited);
								if(result == after){
									start_scheduler_func_calls.emplace_back(call_instruction);
									//state = after;
								}
								else{
									if (result == uncertain){
                                        uncertain_start_scheduler_func_calls.emplace_back(call_instruction);
                                        if (state != after)state = uncertain; 
                                    }
                                }
							}
						}
					}
				}
			}
		}
	}
	if(state !=before){
		for(auto &abb : function->get_atomic_basic_blocks()){
			for(llvm::BasicBlock* bb : abb->get_BasicBlocks()){
				if(abb->get_call_type()== sys_call){
					
					for(auto &instruction:*bb){
						//check if the abb contains a syscall
						//continue if, no start scheduler syscall is in function, the instruction is before all the start scheduler instruction and the scheduler instrcutions dont dominate the instrcution 
						if(validate_start_scheduler_instruction_relations(&start_scheduler_func_calls,&instruction, dominator_tree )){
							//if instruction is a syscall set the instruction in the before scheduler start list
							abb->set_start_scheduler_relation(before);
							
						}else{
							//check if the instruction is reachable from a start scheduler instruction 
							std::vector<size_t> already_visited_abbs;
							if(is_reachable(&start_scheduler_func_calls,instruction.getParent(),&already_visited_abbs)){
								abb->set_start_scheduler_relation(uncertain);
							}
						}
					}
				}
			}
		}
		//check return value if a start scheduler call is certainly or uncertainly called in the function
		//a unceratain call exits if one return instruction has a start scheduler call as dominator
		bool uncertain_flag = false;
		//a certain call exists, if all return instructions have a start scheduler call as dominator
		bool after_flag = true;
		for(auto instr : return_instructions){
			if(validate_start_scheduler_instruction_relations(&start_scheduler_func_calls,instr, dominator_tree ))uncertain_flag = true;
			else after_flag = false;
		}
		
		if(after_flag)state = after;
		else{
			if(uncertain_flag)state = uncertain;
		}
	}
	
	return state;
	
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

//function to extract the handler name of the instance
std::string get_handler_name(llvm::Instruction * instruction, unsigned int argument_index){
	//llvm::Instruction * next_instruction = instruction->getNextNonDebugInstruction();
	
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

std::any get_call_relative_argument(argument_data argument,std::vector<llvm::Instruction*>*call_references){
    
    //check if multiple argument values are possible
    if(argument.multiple ==false)return argument.any_list.front();
    else{
        std::vector<std::tuple<char,std::vector<char>>> valid_candidates;
        char index = 0;
        for(auto argument_calles :argument.argument_calles_list){
            auto tmp_argument_calles = argument_calles;
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
            if(tmp_argument_calles.empty())valid_candidates.emplace_back( std::make_tuple(index,missmatch_list));
            ++index;
        }
        if(valid_candidates.size() == 1){
            return argument.any_list.at(std::get<0>(valid_candidates.front()));
        }else{
            //TODO
        }
    }
}



//xTaskCreate
bool create_task(graph::Graph& graph,OS::shared_abb abb, bool before_scheduler_start,std::vector<llvm::Instruction*>*call_references){
	
	bool success = true;
	
	llvm::Instruction* instruction = abb->get_syscall_instruction_reference();
	
	
	//create reference list for all arguments types of the task creation syscall
	std::vector<argument_data>argument_list;
    
    
	for(auto & argument : *(abb->get_syscall_arguments())){
		argument_list.emplace_back(argument);
	}

	//load the arguments
	argument_data argument= argument_list.at(0);
	auto specific_argument = get_call_relative_argument(argument,call_references);
	std::string function_reference_name =  std::any_cast<std::string>(specific_argument);
	
	argument = argument_list.at(1);
	specific_argument = get_call_relative_argument(argument,call_references);
	std::string task_name =  std::any_cast<std::string>(specific_argument);
	
	argument = argument_list.at(2);
	specific_argument = get_call_relative_argument(argument,call_references);
	unsigned long stacksize =  std::any_cast<long>(specific_argument);
	
	argument = argument_list.at(3);
	specific_argument = get_call_relative_argument(argument,call_references);
	std::string task_argument =  std::any_cast<std::string>(specific_argument);
	
	argument = argument_list.at(4);
	specific_argument = get_call_relative_argument(argument,call_references);
	unsigned long priority =  std::any_cast<long>(specific_argument);
	
	argument = argument_list.at(5);
	specific_argument = get_call_relative_argument(argument,call_references);
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

	return success;
}





//..Semaphore...Create
//{ binary, counting, mutex, recursive_mutex }semaphore_type;
bool create_semaphore(graph::Graph& graph,OS::shared_abb abb,semaphore_type type , bool before_scheduler_start,std::vector<llvm::Instruction*>* call_references ){
	
	bool success = true;
	
	llvm::Instruction* instruction = abb->get_syscall_instruction_reference();
		
	//create reference list for all arguments types of the task creation syscall
	std::vector<argument_data>argument_list;
	
	for(auto & argument : *abb->get_syscall_arguments()){
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
		
			std::cout << "binary semaphore successfully created"<< std::endl;
			break;
		}
		
		case counting:{
			
			//load the arguments
			argument_data argument  = argument_list.at(1);
            auto specific_argument = get_call_relative_argument(argument,call_references);
			unsigned long initial_count =  std::any_cast<long>(specific_argument);
			
			argument  = argument_list.at(0);
			specific_argument = get_call_relative_argument(argument,call_references);
			unsigned long max_count =  std::any_cast<long>(specific_argument);
			

			semaphore->set_initial_count(initial_count);
			semaphore->set_max_count(max_count);

			std::cout << "counting semaphore successfully created"<< std::endl;
			
			break;
		}
		
		
		case mutex:{
			
			
			//load the arguments
			argument_data argument  = argument_list.at(0);
			auto specific_argument = get_call_relative_argument(argument,call_references);
			unsigned long mutex_type =  std::any_cast<long>(specific_argument);
			
			//set the mutex type (mutex, recursive mutex)
			type = (semaphore_type) mutex_type;
			
			semaphore->set_semaphore_type(type);
			if(type == mutex)std::cout << "mutex successfully created"<< std::endl;
			if(type == recursive_mutex)std::cout << "recursive mutex successfully created"<< std::endl;

			break;
		
		}
		default:{
			success = false;
			std::cout << "wrong semaphore type" << std::endl;
			break;
		}
	}
	if(success)graph.set_vertex(semaphore);
	return success;
}

//xQueueCreate
bool create_queue(graph::Graph& graph, OS::shared_abb abb ,bool before_scheduler_start,std::vector<llvm::Instruction*>* call_references){
	
	bool success = true;
	
	
	llvm::Instruction* instruction = abb->get_syscall_instruction_reference();
	
	
	//create reference list for all arguments types of the task creation syscall
	std::vector<argument_data>argument_list;
	
	for(auto & argument : *abb->get_syscall_arguments()){
		argument_list.emplace_back(argument);
	}
	
	//load the arguments
	argument_data argument   = argument_list.at(0);
	auto specific_argument = get_call_relative_argument(argument,call_references);
	long queue_length =  std::any_cast<long>(specific_argument);
	
	argument  = argument_list.at(1);
	specific_argument = get_call_relative_argument(argument,call_references);
	long item_size =  std::any_cast<long>(specific_argument);

	argument  = argument_list.at(2);
	specific_argument = get_call_relative_argument(argument,call_references);
	long queue_type =  std::any_cast<long>(specific_argument);
	
	semaphore_type type = (semaphore_type) queue_type;
	
	if(type != binary){
		
		std::string handler_name = get_handler_name(instruction, 1);
		
		
		//create queue and set properties
		auto queue = std::make_shared<OS::Queue>(&graph,handler_name);
		
		queue->set_handler_name(handler_name);
		queue->set_length(queue_length);
		queue->set_item_size(item_size);
		queue->set_start_scheduler_creation_flag(before_scheduler_start);
		graph.set_vertex(queue);
		
		std::cout << "queue successfully created"<< std::endl;
	}else{
		success = create_semaphore(graph,abb,binary, before_scheduler_start,call_references);
	}

	return success;
}


//xQueueCreate
bool create_event_group(graph::Graph& graph,OS::shared_abb abb, bool before_scheduler_start,std::vector<llvm::Instruction*>* call_references){
	
	bool success = true;
	
	llvm::Instruction* instruction = abb->get_syscall_instruction_reference();
	
	
	//create reference list for all arguments types of the task creation syscall
	std::vector<argument_data>argument_list;
	
	for(auto & argument : *abb->get_syscall_arguments()){
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
		
	
	return success;
}




//xQueueSet
bool create_queue_set(graph::Graph& graph, OS::shared_abb abb,  bool before_scheduler_start,std::vector<llvm::Instruction*>* call_references){
	
	bool success = true;

	llvm::Instruction* instruction = abb->get_syscall_instruction_reference();
	
	
	//create reference list for all arguments types of the task creation syscall
	std::vector<argument_data>argument_list;
	
	for(auto & argument : *abb->get_syscall_arguments()){
		argument_list.emplace_back(argument);
	}
	
	//load the arguments
    argument_data argument   = argument_list.at(0);
	auto specific_argument = get_call_relative_argument(argument,call_references);
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
	
	return success;
}

//xTimerCreate
bool create_timer(graph::Graph& graph,OS::shared_abb abb, bool before_scheduler_start,std::vector<llvm::Instruction*>* call_references){
	
	bool success = true;
	
	std::vector<argument_data>argument_list;
	
	for(auto & argument : *abb->get_syscall_arguments()){
		argument_list.emplace_back(argument);
	}
	//load the arguments
	argument_data argument   = argument_list.at(0);
	auto specific_argument = get_call_relative_argument(argument,call_references);
	std::string timer_name =  std::any_cast<std::string>(specific_argument);
	
	argument  = argument_list.at(1);
	specific_argument = get_call_relative_argument(argument,call_references);
	long timer_periode =  std::any_cast<long>(specific_argument);

	argument  = argument_list.at(2);
	specific_argument = get_call_relative_argument(argument,call_references);
	long timer_autoreload =  std::any_cast<long>(specific_argument);
	
	argument  = argument_list.at(3);
	specific_argument = get_call_relative_argument(argument,call_references);
	std::string timer_id =  std::any_cast<std::string>(specific_argument);
	
	argument  = argument_list.at(4);
	specific_argument = get_call_relative_argument(argument,call_references);
	std::string timer_definition_function =  std::any_cast<std::string>(specific_argument);
	
	//create timer and set properties 
	auto timer = std::make_shared<OS::Timer>(&graph,timer_name);
	
	timer->set_periode(timer_periode);
	//TODO extract timer id
	//std::cout << timer_id << std::endl;
	//timer->set_timer_id(timer_id);
	timer->set_definition_function(timer_definition_function);
	if(timer_autoreload == 0) timer->set_timer_type(oneshot);
	else timer->set_timer_type(autoreload);
	std::cout << "timer successfully created"<< std::endl;
	//set timer to graph
	timer->set_start_scheduler_creation_flag(before_scheduler_start);
	graph.set_vertex(timer);
	
	return success;
}


//xTimerCreate
bool create_buffer(graph::Graph& graph,OS::shared_abb abb, bool before_scheduler_start,std::vector<llvm::Instruction*>* call_references){
	
	bool success = true;
	
	llvm::Instruction* instruction = abb->get_syscall_instruction_reference();
	//create reference list for all arguments types of the task creation syscall
	

	//get the typeid hashcode of the expected arguments
		std::vector<argument_data>argument_list;
	
	for(auto & argument : *abb->get_syscall_arguments()){
		argument_list.emplace_back(argument);
	}
			
	//load the arguments
	argument_data argument   = argument_list.at(2);
	auto specific_argument = get_call_relative_argument(argument,call_references);
	buffer_type type = (buffer_type) std::any_cast<long>(specific_argument);
	
	//std::cout << "buffer type: "<< std::any_cast<long>(argument)<< std::endl;
	
	argument  = argument_list.at(1);
	specific_argument = get_call_relative_argument(argument,call_references);
	long trigger_level =  std::any_cast<long>(specific_argument);

	argument  = argument_list.at(0);
	specific_argument = get_call_relative_argument(argument,call_references);
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
	
	return success;
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
            
            std::cerr << abb->get_syscall_name() << std::endl;
            
           
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



bool create_abstraction_instance(graph::Graph& graph,OS::shared_abb abb,bool before_scheduler_start,std::vector<llvm::Instruction*>* already_visited_calls ){

    //check which target should be generated
    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::Task).hash_code())){
        //std::cout << "TASKCREATE" << name << ":" << tmp << std::endl;
        if(!create_task(graph,abb,before_scheduler_start,already_visited_calls))std::cout << "Task could not created" << std::endl;
    }
    
        
    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::Queue).hash_code())){
        if(!create_queue( graph,abb,before_scheduler_start,already_visited_calls))std::cout << "Queue could not created" << std::endl;

    }
    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::Semaphore).hash_code())){
        //set semaphore
        semaphore_type type;
        std::string syscall_name = abb->get_syscall_name();
        if(syscall_name =="xQueueCreateMutex")type = mutex;
        if(syscall_name =="xSemaphoreCreateRecursiveMutex")type = recursive_mutex;
        if(syscall_name =="xQueueCreateCountingSemaphore")type = counting;
    
        if(!create_semaphore(graph, abb, type, before_scheduler_start,already_visited_calls))std::cout << "CountingSemaphore/Mutex could not created" << std::endl;
        
    }						
    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::Timer).hash_code())){
        if(!create_timer( graph,abb,before_scheduler_start,already_visited_calls))std::cout << "Timer could not created" << std::endl;
    }

    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::EventGroup).hash_code())){
        if(!create_event_group(graph, abb,before_scheduler_start,already_visited_calls))std::cout << "Event Group could not created" << std::endl;
    }
    
    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::Buffer).hash_code())){
        //std::cout << callname << std::endl;
        if(!create_buffer(graph, abb,before_scheduler_start,already_visited_calls))std::cout << "Buffer could not created" << std::endl;
    }
    if(list_contains_element(abb->get_call_target_instances(),typeid(OS::QueueSet).hash_code())){
        //std::cout << callname << std::endl;
        if(!create_queue_set(graph, abb,before_scheduler_start,already_visited_calls))std::cout << "Queue Set could not created" << std::endl;
    }
}


void iterate_called_functions(graph::Graph& graph, graph::shared_vertex start_vertex, OS::shared_function function, llvm::Instruction* call_reference ,std::vector<llvm::Instruction*>* already_visited_calls ){
    
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
        std::list<llvm::BasicBlock*> llvm_bbs = abb->get_BasicBlocks();
            
        //check if abb has a syscall instruction
        if( abb->get_call_type()== sys_call){
            
            std::vector<std::size_t> already_visited;

            //validate if sysccall is in loop
            if(validate_loop(llvm_bbs.front(),&already_visited)){
            
                bool before_scheduler_start = false;
                
                if(abb->get_start_scheduler_relation() == before)before_scheduler_start = true;
                                        
                //check if abb syscall is creation syscall
                if(abb->get_syscall_type() == create){
                    
                    create_abstraction_instance( graph,abb,before_scheduler_start,already_visited_calls);
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



namespace step {

	std::string FreeRTOSInstancesStep::get_name() {
		return "FreeRTOSInstancesStep";
	}

	std::string FreeRTOSInstancesStep::get_description() {
		return "Extracts out of FreeRTOS abstraction instances";
	}

	void FreeRTOSInstancesStep::run(graph::Graph& graph) {
		
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
            already_visited.clear();
            
            //iterate about the abbs of the function
            for(auto & abb : main_function->get_atomic_basic_blocks()){
                //iterate about the called functions of the abb
                for (auto called_function : abb->get_called_functions()){
                    //update the values for the successor functions from func call
                    update_called_functions(graph, called_function  ,&already_visited,abb->get_start_scheduler_relation());
                }
            }
            
            //iterate about the main function context and detect abstraction instances
            std::vector<llvm::Instruction*> already_visited_calls;
            iterate_called_functions(graph, main_vertex , main_function, nullptr ,&already_visited_calls);
		
            
        }else{
            std::cerr << "no main function in programm" << std::endl;
            abort();
        }
        
       
        
        //detect the isrs
        detect_isrs(graph);
        
       //get all tasks, which are stored in the graph
        std::list<graph::shared_vertex> vertex_list =  graph.get_type_vertices(typeid(OS::Task).hash_code());
        //iterate about the isrs
        for (auto &vertex : vertex_list) {
            //std::cerr << "task name: " << vertex->get_name() << std::endl;
            std::vector<size_t> already_visited;
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
            //std::cerr << "isr name: " << vertex->get_name() << std::endl;
            std::vector<size_t> already_visited;
            auto timer = std::dynamic_pointer_cast<OS::ISR> (vertex);
            OS::shared_function timer_definition = timer->get_definition_function();
            //get all interactions of the instance
            std::vector<llvm::Instruction*> already_visited_calls;
            iterate_called_functions(graph, timer , timer_definition, nullptr ,&already_visited_calls);
        }
           
        //TODO check if this is allowed
        //get all timers of the graph
        vertex_list =  graph.get_type_vertices(typeid(OS::Timer).hash_code());
        //iterate about the timers
        for (auto &vertex : vertex_list) {
            //std::cerr << "timer name: " << vertex->get_name() << std::endl;
            std::vector<size_t> already_visited;
            auto isr = std::dynamic_pointer_cast<OS::Timer> (vertex);
            OS::shared_function isr_definition = isr->get_definition_function();
            //get all interactions of the instance
            std::vector<llvm::Instruction*> already_visited_calls;
            iterate_called_functions(graph, isr , isr_definition, nullptr ,&already_visited_calls);
        }	
	}
	
	std::vector<std::string> FreeRTOSInstancesStep::get_dependencies() {
		return {"ABB_MergeStep"};
	}
}
