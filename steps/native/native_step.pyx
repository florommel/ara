# cython: language_level=3
"""Common description ob both Python and C++ steps. A step is a part of
Arsa, that fulfils one specific task. There it manipulates the systemgraph in
a specific way.

Steps can have dependencies respectively depend on other steps, the
stepmanager then fulfils this dependencies.
"""

cimport cstep
cimport graph
cimport cgraph

from bb_split cimport BBSplit
from comp_insert cimport CompInsert
from detect_interactions cimport DetectInteractionsStep
from fn_single_exit cimport FnSingleExit
from freertos_instances cimport FreeRTOSInstancesStep
from intermediate_analysis cimport IntermediateAnalysisStep
from ir_reader cimport IRReader
from llvm_basic_optimization cimport LLVMBasicOptimization
from validation cimport ValidationStep
from llvm cimport LLVMStep
from cdummy cimport CDummy

from test cimport (BBSplitTest,
                   CompInsertTest,
                   FnSingleExitTest,
                   Test0Step,
                   Test2Step)

from cython.operator cimport dereference as deref
from libcpp.memory cimport shared_ptr
from backported_memory cimport static_pointer_cast as spc
from cstep cimport step_fac

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


cdef get_warning_abb(shared_ptr[cgraph.ABB] location):
    cdef pyobj = graph.create_abb(location)
    return pyobj


cdef class NativeStep(SuperStep):
    """Constructs a dummy Python class for a C++ step.

    Use native_fac() to construct a NativeStep.
    """

    # the pointer attribute that holds the C++ object
    cdef cstep.Step* _c_pass

    def __init__(self, config: dict, *args):
        """Fake constructor. Prevent usage of super constructor. Must not
        calle directly

        Use native_fac() to construct a NativeStep.
        """
        pass

    def init(self, config: dict):
        """The actual constructor function. Must not called directly.

        Use native_fac() to construct a NativeStep.
        """
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
            for warning in (<ValidationStep*> self._c_pass).get_warnings():
                p_warn = {'type': deref(warning).get_type().decode('UTF-8'),
                          'location': get_warning_abb(deref(warning).warning_position)}
                warnings.append(p_warn)
            return warnings
        super().get_side_data()


cdef _native_fac(config: dict, cstep.Step* step):
    """Construct a NativeStep. Expects an already constructed C++-Step pointer.
    This pointer can be retrieved with step_fac[...](config).

    Don't use this function. Use provide_steps to get all steps.
    """
    n_step = NativeStep(config)
    n_step._c_pass = step
    n_step.init(config)
    return n_step


def provide_steps(config: dict):
    """Provide a list of all native steps. This also constructs as many
    objects as steps exist.

    Arguments:
    config -- a configuration dict like the one Step.__init__() needs.
    """
    return [_native_fac(config, step_fac[BBSplit](config)),
            _native_fac(config, step_fac[CDummy](config)),
            _native_fac(config, step_fac[CompInsert](config)),
            _native_fac(config, step_fac[DetectInteractionsStep](config)),
            _native_fac(config, step_fac[FnSingleExit](config)),
            _native_fac(config, step_fac[FreeRTOSInstancesStep](config)),
            _native_fac(config, step_fac[IRReader](config)),
            _native_fac(config, step_fac[IntermediateAnalysisStep](config)),
            _native_fac(config, step_fac[LLVMStep](config)),
            _native_fac(config, step_fac[LLVMBasicOptimization](config)),
            _native_fac(config, step_fac[ValidationStep](config))]


def provide_test_steps(config: dict):
    """Do not use this, only for testing purposes."""
    return [_native_fac(config, step_fac[BBSplitTest](config)),
            _native_fac(config, step_fac[CompInsertTest](config)),
            _native_fac(config, step_fac[FnSingleExitTest](config)),
            _native_fac(config, step_fac[Test0Step](config)),
            _native_fac(config, step_fac[Test2Step](config))]
