cimport cgraph

from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "llvm_basic_optimization.h" namespace "step":
    cdef cppclass LLVMBasicOptimization:
        LLVMBasicOptimization(dict config) except +
        string get_name()
        string get_description()
        vector[string] get_dependencies()
        void run(cgraph.Graph a)
