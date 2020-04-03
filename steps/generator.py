"""Container for Generator."""
import os
import sys
import graph

from native_step import Step
from .option import Option, Choice, String

from generator import *
from generator.Generator import Generator as GenImpl



class Generator(Step):
    """Template for a new Python step."""

    arch_choices = {'arm':ArmArch}

    os_choices = {'freertos': FreeRTOSGenericOS}

    syscall_choices= {'vanilla': VanillaSystemCalls,
                      'generic_static': StaticFullSystemCalls,
                      'generic_initialized': InitializedFullSystemCalls,
                      'instance_specialized': lambda: exec('raise NotImplementedError()'),
    }

    def _fill_options(self):
        self.arch = Option(name="arch",
                           help='the hardware architecture',
                           step_name=self.get_name(),
                           ty=Choice(*self.arch_choices.keys()))
        self.os = Option(name="os",
                         help='the os api',
                         step_name=self.get_name(),
                         ty=Choice(*self.os_choices.keys()))
        self.syscall_style = Option(name="syscall_style",
                                    help='style of resulting syscalls',
                                    step_name=self.get_name(),
                                    ty=Choice(*self.syscall_choices.keys()))
        self.out_file = Option('output_file',
                                 help='file to write the generated OS into',
                                 step_name=self.get_name(),
                                 glob=True,
                                 ty=String())
        self.dep_file = Option('dependency_file',
                               help='file to write make-style dependencies into for build system integration',
                               step_name=self.get_name(),
                               glob=True,
                               ty=String())
        self.opts.append(self.arch)
        self.opts.append(self.os)
        self.opts.append(self.syscall_style)
        self.opts.append(self.out_file)
        self.opts.append(self.dep_file)

    def get_dependencies(self):
        return ['InstanceGraph']

    def run(self, g: graph.Graph):
        # self._log.info("Executing Generator step.")
        # opt = self.dummy_option.get()
        # if opt:
        #     self._log.info(f"Option is {opt}.")
        assert self.out_file.get(), "No output folder given"

        arch_rules = self.arch_choices[self.arch.get()]()
        os_rules = self.os_choices[self.os.get()]()
        syscall_rules = self.syscall_choices[self.syscall_style.get()]()
        gen = GenImpl(ara_graph=g,
                      arch_rules=arch_rules,
                      os_rules=os_rules,
                      syscall_rules=syscall_rules,
                      logger=self._log)

        gen.generate(self.out_file.get())

        dep_file = self.dep_file.get()
        if dep_file:
            ara_file = sys.modules['__main__'].__file__
            base_path = os.path.dirname(ara_file)
            src_files = [getattr(m, '__file__') for m in sys.modules.values()
                         if base_path in (getattr(m, '__file__', '') or "")]
            with open(dep_file, 'w') as fd:
                fd.write(gen.file_prefix + ": ")
                fd.write("\\\n".join(src_files))
