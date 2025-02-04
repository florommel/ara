"""Container for Printer."""
from ara.graph import ABBType, CFType, Graph, NodeLevel, CFGView

from .option import Option, String, Choice, Bool
from .step import Step

import pydot
import html
import os
import os.path

import graph_tool.draw


class Printer(Step):
    """Print graphs to dot."""

    SHAPES = {
        ABBType.computation: ("oval", "blue"),
        ABBType.call: ("box", "red"),
        ABBType.syscall: ("diamond", "green")
    }

    dot = Option(name="dot",
                 help="Path to a dot file, '-' will write to stdout.",
                 ty=String())
    graph_name = Option(name="graph_name",
                        help="Name of the graph.",
                        ty=String())
    subgraph = Option(name="subgraph",
                      help="Choose, what subgraph should be printed.",
                      ty=Choice("abbs", "instances", "callgraph"))
    entry_point = Option(name="entry_point",
                         help="system entry point",
                         ty=String())
    from_entry_point = Option(name="from_entry_point",
                              help="Print only from the given entry point.",
                              ty=Bool(),
                              default_value=False)
    gen_html_links = Option(name="gen_html_links",
                            help="Generate source code links (with Pygments)",
                            ty=Bool(),
                            default_value=True)

    def _print_init(self):
        dot = self.dot.get()
        if not dot:
            self._fail("dot file path must be given.")

        name = self.graph_name.get()
        if not name:
            name = ''
        return name

    def _write_dot(self, dot):
        dot_path = self.dot.get()
        assert dot_path
        dot_path = os.path.abspath(dot_path)
        os.makedirs(os.path.dirname(dot_path), exist_ok=True)
        dot.write(dot_path)
        self._log.info(f"Write {self.subgraph.get()} to {dot_path}.")

    def _gen_html_file(self, filename):
        try:
            from pygments import highlight
            from pygments.lexers import CppLexer
            from pygments.formatters import HtmlFormatter
        except ImportError:
            self._log.warn("Pygments not found, skip source code linking")
            return None

        filename = os.path.abspath(filename)

        if filename in self._graph.file_cache:
            return self._graph.file_cache[filename]

        hfile = os.path.join(os.path.dirname(self.dump_prefix.get()),
                             'html_files',
                             os.path.basename(filename) + ".html")
        hfile = os.path.realpath(hfile)
        self._graph.file_cache[filename] = hfile

        with open(filename) as f:
            code = f.read()

        os.makedirs(os.path.dirname(hfile), exist_ok=True)
        with open(hfile, 'w') as g:
            g.write(highlight(code, CppLexer(),
                              HtmlFormatter(linenos='inline',
                                            lineanchors='line', full=True)))
        return hfile

    def print_abbs(self):
        name = self._print_init()

        cfg = self._graph.cfg

        entry_label = self.entry_point.get()
        if self.from_entry_point.get():
            entry_func = self._graph.cfg.get_function_by_name(entry_label)
            functions = self._graph.cfg.reachable_functs(entry_func)
        else:
            functs = self._graph.functs
            functions = functs.vertices()

        dot_nodes = set()
        dot_graph = pydot.Dot(graph_type='digraph', label=name)
        for function in functions:
            function = cfg.vertex(function)
            dot_func = pydot.Cluster(cfg.vp.name[function],
                                     label=cfg.vp.name[function])
            dot_graph.add_subgraph(dot_func)
            for abb in cfg.get_abbs(function):
                if cfg.vp.type[abb] == ABBType.not_implemented:
                    assert not cfg.vp.implemented[function]
                    dot_abb = pydot.Node(str(hash(abb)),
                                         label="",
                                         shape="box")
                    dot_nodes.add(str(hash(abb)))
                    dot_func.set('style', 'filled')
                    dot_func.set('color', '#eeeeee')
                else:
                    dot_abb = pydot.Node(
                        str(hash(abb)),
                        label=cfg.vp.name[abb],
                        shape=self.SHAPES[self._graph.cfg.vp.type[abb]][0],
                        color=self.SHAPES[self._graph.cfg.vp.type[abb]][1]
                    )
                    if cfg.vp.part_of_loop[abb]:
                        dot_abb.set('style', 'dashed')
                    dot_nodes.add(str(hash(abb)))
                dot_func.add_node(dot_abb)
        for edge in cfg.edges():
            if cfg.ep.type[edge] not in [CFType.lcf, CFType.icf]:
                continue
            if not all([str(hash(x)) in dot_nodes
                    for x in [edge.source(), edge.target()]]):
                continue
            color = "black"
            if cfg.ep.type[edge] == CFType.lcf:
                color = "red"
            if cfg.ep.type[edge] == CFType.icf:
                color = "blue"
            dot_graph.add_edge(pydot.Edge(str(hash(edge.source())),
                                          str(hash(edge.target())),
                                          color=color))
        self._write_dot(dot_graph)

    def print_instances(self):
        name = self._print_init()

        instances = self._graph.instances

        dot_graph = pydot.Dot(graph_type='digraph', label=name)

        default_fontsize = 14
        default_fontsize_diff = 2

        def p_str(p_map, key):
            """Convert to a pretty string"""
            value = p_map[key]
            if p_map.python_value_type() == bool:
                value = bool(value)
            return html.escape(str(value))

        for instance in instances.vertices():
            inst_obj = instances.vp.obj[instance]
            if inst_obj and hasattr(inst_obj, 'as_dot'):
                attrs = inst_obj.as_dot()
            else:
                attrs = {}
            if "label" in attrs:
                del attrs["label"]
            attrs["fontsize"] = attrs.get("fontsize", 14)

            if self.gen_html_links.get():
                src_file = instances.vp.file[instance]
                src_line = instances.vp.line[instance]
                if (src_file != '' and src_line != 0):
                    hfile = self._gen_html_file(src_file)
                    if hfile is not None:
                        attrs["URL"] = f"file://{hfile}#line-{src_line}"

            size = attrs["fontsize"] - default_fontsize_diff
            label = instances.vp.label[instance]
            graph_attrs = '<br/>'.join([f"<i>{k}</i>: {p_str(v, instance)}"
                                        for k, v in instances.vp.items()
                                        if k not in ["label", "obj"]])
            graph_attrs = f"<font point-size='{size}'>{graph_attrs}</font>"
            label = f"<{label}<br/>{graph_attrs}<br/><br/>{{}}>"
            sublabel = attrs.get("sublabel", "")
            if len(sublabel) > 0:
                sublabel = f"<font point-size='{size}'>{sublabel}</font>"
            label = label.format(sublabel)
            if "sublabel" in attrs:
                del attrs["sublabel"]

            dot_node = pydot.Node(
                str(hash(instance)),
                label=label,
                **attrs
            )
            dot_graph.add_node(dot_node)
        for edge in self._graph.instances.edges():
            dot_graph.add_edge(pydot.Edge(
                str(hash(edge.source())),
                str(hash(edge.target())),
                label=self._graph.instances.ep.label[edge]))
        self._write_dot(dot_graph)

    def print_callgraph(self):
        name = self._print_init()

        shapes = {
            True: ("box", "green"),
            False: ("box", "black")
        }

        dot_graph = pydot.Dot(graph_type='digraph', label=name)
        callgraph = self._graph.callgraph

        cfg = callgraph.gp.cfg
        for node in callgraph.vertices():
            dot_node = pydot.Node(
                str(hash(node)),
                label=cfg.vp.name[callgraph.vp.function[node]],
                shape=shapes[callgraph.vp.syscall_category_every[node]][0],
                color=shapes[callgraph.vp.syscall_category_every[node]][1]
            )
            if callgraph.vp.recursive[node]:
                dot_node.set('style', 'dashed')
            dot_graph.add_node(dot_node)
        for edge in callgraph.edges():
            dot_graph.add_edge(pydot.Edge(
                str(hash(edge.source())),
                str(hash(edge.target())),
                label=cfg.vp.name[callgraph.ep.callsite[edge]]))
        self._write_dot(dot_graph)

    def run(self):
        subgraph = self.subgraph.get()
        if subgraph == 'abbs':
            self.print_abbs()
        if subgraph == 'instances':
            self.print_instances()
        if subgraph == 'callgraph':
            self.print_callgraph()
