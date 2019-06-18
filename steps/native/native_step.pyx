# cython: language_level=3
"""Common description ob both Python and C++ steps. A step is a part of
Arsa, that fulfils one specific task. There it manipulates the systemgraph in
a specific way.

Steps can have dependencies respectively depend on other steps, the
stepmanager then fulfils this dependencies.
"""

cimport cstep
cimport llvm
cimport FreeRTOSinstances
cimport DetectInteractions
cimport IntermediateAnalysis
cimport Validation
cimport ir_reader
cimport bb_split
cimport test
cimport graph
cimport cgraph

from cython.operator cimport dereference as deref
from libcpp.memory cimport shared_ptr
from backported_memory cimport static_pointer_cast as spc

import logging
import inspect

cdef class SuperStep:
    """Super class for Python and C++ steps. Do not use this class directly.
    """
    cdef public dict _config
    cdef public object _log

    def __init__(self, config: dict):
        """Initialize a Step.

        Arguments:
        config -- the program config. This is a dict of options. Usally the one
                generated by argparse.
        """
        self._config = config
        self._log = logging.getLogger(self.get_name())

    def get_dependencies(self):
        """Define all dependencies of the step.

        Returns a list of dependencies (str). This means, that the step
        depends on _all_ of the defined steps. The elements of the list are
        strings, that match with the names returned by get_name().
        """
        return []

    def get_name(self) -> str:
        """Return a unique name of the step. The name is used as ID for the
        step."""
        pass

    def get_description(self):
        """Return a descriptive string, that explains what the pass is doing."""
        pass

    def run(self, graph.PyGraph g):
        """Do the actual action of the pass.

        Arguments:
        g -- the system graph.
        """
        raise NotImplementedError()

    def get_side_data(self):
        """Provide arbitrary side data, that are not belonging to the system
        graph. This can be used to make analysis based on the system graph and
        extract some data not related to the graph itself.

        Return some step specific kind of side data.
        """
        raise NotImplementedError()

class Step(SuperStep):
    """Python representation of a step. This is the superclass for all other
    steps."""
    def __init__(self, config: dict):
        super().__init__(config)

    def get_name(self):
        return self.__class__.__name__

    def get_description(self) -> str:
        return inspect.cleandoc(self.__doc__)

ctypedef enum steps:
    FreeRTOSInstances_STEP
    DetectInteractions_STEP
    Validation_STEP
    BBSplit_STEP
    IRReader_STEP
    IntermediateAnalysis_STEP
    LLVM_STEP
    TEST0_STEP
    TEST2_STEP
    BBSplitTest_STEP

cdef get_warning_abb(shared_ptr[cgraph.ABB] location):
    cdef pyobj = graph.create_abb(location)
    return pyobj

cdef class NativeStep(SuperStep):
    """Constructs a dummy Python class for a C++ step."""

    # the pointer attribute that holds the C++ object
    cdef cstep.Step* _c_pass

    def __cinit__(self, config: dict, steps step_cls):
        """Constructs an arbitrary C++ step object, that inherits from
        cstep.Step. This has the advantage, that only one Python class is
        necessary to wrap arbitrary C++ steps.

        Arguments:
        config      -- a configuration dict like the one Step.__init__()
                    needs.
        step_cls -- an identifier what specific C++ step the object
                    should wrap.
        """
        # select here what specific step should be constructed
        if step_cls == LLVM_STEP:
            self._c_pass = <cstep.Step*> new llvm.LLVMStep(config)
        elif  step_cls == FreeRTOSInstances_STEP:
            self._c_pass = <cstep.Step*> new FreeRTOSinstances.FreeRTOSInstancesStep(config)
        elif  step_cls == DetectInteractions_STEP:
            self._c_pass = <cstep.Step*> new DetectInteractions.DetectInteractionsStep(config)
        elif  step_cls == Validation_STEP:
            self._c_pass = <cstep.Step*> new Validation.ValidationStep(config)
        elif  step_cls == IRReader_STEP:
            self._c_pass = <cstep.Step*> new ir_reader.IRReader(config)
        elif  step_cls == BBSplit_STEP:
            self._c_pass = <cstep.Step*> new bb_split.BBSplit(config)
        elif  step_cls == IntermediateAnalysis_STEP:
            self._c_pass = <cstep.Step*> new IntermediateAnalysis.IntermediateAnalysisStep(config)
        # for testing purposes (can not be transferred into seperate file)
        elif step_cls == TEST0_STEP:
            self._c_pass = <cstep.Step*> new test.Test0Step(config)
        elif step_cls == TEST2_STEP:
            self._c_pass = <cstep.Step*> new test.Test2Step(config)
        elif step_cls == BBSplitTest_STEP:
            self._c_pass = <cstep.Step*> new test.BBSplitTest(config)
        else:
            raise ValueError("Unknown step class")

        if self._c_pass is NULL:
            raise MemoryError()

    def __init__(self, config: dict, *args):
        super().__init__(config)
        self._c_pass.set_logger(self._log)

    def __dealloc__(self):
        """Destroy the C++ object (if any)."""
        if self._c_pass is not NULL:
            del self._c_pass

    def get_dependencies(self):
        # doing this in one line leads to a compiler error
        deps = self._c_pass.get_dependencies()
        return [x.decode('UTF-8') for x in deps]

    def run(self, graph.PyGraph g):
        self._c_pass.run(g._c_graph)

    def get_name(self) -> str:
        return self._c_pass.get_name().decode('UTF-8')

    def get_description(self):
        return self._c_pass.get_description().decode('UTF-8')

    def get_side_data(self):
        if self.get_name() == "ValidationStep":
            warnings = []
            for warning in (<Validation.ValidationStep*> self._c_pass).get_warnings():
                p_warn = {'type': deref(warning).get_type().decode('UTF-8'),
                          'location': get_warning_abb(deref(warning).warning_position)}
                warnings.append(p_warn)
            return warnings
        super().get_side_data()

def provide_steps(config: dict):
    """Provide a list of all native steps. This also constructs as many
    objects as steps exist.

    Arguments:
    config -- a configuration dict like the one Step.__init__() needs.
    """
    return [NativeStep(config, LLVM_STEP),
            NativeStep(config, FreeRTOSInstances_STEP),
            NativeStep(config, DetectInteractions_STEP),
            NativeStep(config, Validation_STEP),
            NativeStep(config, BBSplit_STEP),
            NativeStep(config, IRReader_STEP),
            NativeStep(config, IntermediateAnalysis_STEP)]

def provide_test_steps(config: dict):
    """Do not use this, only for testing purposes."""
    return [NativeStep(config, TEST0_STEP),
            NativeStep(config, BBSplitTest_STEP),
            NativeStep(config, TEST2_STEP)]
