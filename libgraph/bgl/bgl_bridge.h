#pragma once

#include "bgl_wrapper.h"
#include "boost/graph/graph_concepts.hpp"
#include "boost/graph/subgraph.hpp"
#include "boost/iterator/transform_iterator.hpp"

#include <functional>
#include <memory>

namespace ara::bgl_wrapper {

	template <typename Wrapper, typename BoostIterator>
	class GraphIter : public GraphIterator<Wrapper> {
	  private:
		BoostIterator iter;

	  public:
		GraphIter(BoostIterator it) : iter(std::move(it)) {}

		virtual std::unique_ptr<Wrapper> operator*() override { return *iter; }
		virtual GraphIterator<Wrapper>& operator++() override {
			iter.operator++();
			return *this;
		}
		virtual GraphIterator<Wrapper>& operator--() override {
			iter.operator--();
			return *this;
		}

		virtual bool operator==(const GraphIterator<Wrapper>& o) override {
			const auto& ot = static_cast<const GraphIter<Wrapper, BoostIterator>&>(o);
			return this->iter == ot.iter;
		}
		virtual bool operator!=(const GraphIterator<Wrapper>& o) override {
			const auto& ot = static_cast<const GraphIter<Wrapper, BoostIterator>&>(o);
			return this->iter != ot.iter;
		}
	};

	class PredicateImpl {};

	template <typename Wrapper, typename WrapperT, typename Graph, typename BoostIterator, typename ValueType>
	static inline std::pair<std::unique_ptr<GraphIterator<Wrapper>>, std::unique_ptr<GraphIterator<Wrapper>>>
	convert_it(Graph& g, const SamePair<BoostIterator>& its) {
		using TransformIterator =
		    boost::transform_iterator<std::function<std::unique_ptr<WrapperT>(const ValueType&)>, BoostIterator>;
		std::function<std::unique_ptr<WrapperT>(const ValueType&)> convert_to_ptr =
		    [&](const ValueType& p) -> std::unique_ptr<WrapperT> { return std::make_unique<WrapperT>(g, p); };

		TransformIterator tfirst = boost::make_transform_iterator(std::move(its.first), convert_to_ptr);
		TransformIterator tsecond = boost::make_transform_iterator(std::move(its.second), convert_to_ptr);
		std::unique_ptr<GraphIterator<Wrapper>> first = std::make_unique<GraphIter<Wrapper, TransformIterator>>(tfirst);
		std::unique_ptr<GraphIterator<Wrapper>> second =
		    std::make_unique<GraphIter<Wrapper, TransformIterator>>(tsecond);
		return std::make_pair(std::move(first), std::move(second));
	}

	template<typename Graph> class GraphImpl;
	template <typename Graph, typename SubGraph>
	class SubGraphImpl;
	template <typename Graph>
	class EdgeImpl;

	template<typename Graph>
	class VertexImpl : public VertexWrapper {
		public:
		  VertexImpl(Graph& g, typename Graph::vertex_descriptor v) : g(g), v(std::move(v)) {}
		  virtual ~VertexImpl() {}

		  virtual SamePair<std::unique_ptr<GraphIterator<EdgeWrapper>>> in_edges() override {
			  return convert_it<EdgeWrapper, EdgeImpl<Graph>, Graph, typename Graph::in_edge_iterator,
			                    typename Graph::edge_descriptor>(g, boost::in_edges(v, g));
		}

		virtual SamePair<std::unique_ptr<GraphIterator<EdgeWrapper>>> out_edges() override {
			return convert_it<EdgeWrapper, EdgeImpl<Graph>, Graph, typename Graph::out_edge_iterator,
			                  typename Graph::edge_descriptor>(g, boost::out_edges(v, g));
		}

		virtual uint64_t in_degree() override { return convert_size(boost::in_degree(v, g)); }

		virtual uint64_t out_degree() override { return convert_size(boost::out_degree(v, g)); }

		virtual uint64_t degree() override { return convert_size(boost::degree(v, g)); }

		virtual SamePair<std::unique_ptr<GraphIterator<VertexWrapper>>> adjacent_vertices() override {
			return convert_it<VertexWrapper, VertexImpl<Graph>, Graph, typename Graph::adjacency_iterator,
			                  typename Graph::vertex_descriptor>(g, boost::adjacent_vertices(v, g));
		}

		// virtual SamePair<std::unique_ptr<GraphIterator<VertexWrapper>>> inv_adjacent_vertices() override {
		// 	return convert_it<VertexWrapper, VertexImpl<Graph>, Graph, boost::adjacency_list::inv_adjacency_iterator,
		// typename Graph::vertex_descriptor>(g, boost::inv_adjacent_vertices(v, g));
		// }

		// virtual void clear_in_edges() override {
		// 	boost::clear_in_edges(v, g);
		// }

		// virtual void clear_out_edges() override {
		// 	boost::clear_out_edges(v, g);
		// }

		virtual void clear_edges() override { boost::clear_vertex(v, g); }

		virtual ara::graph::BoostProperty& get_property_obj() override {
			return g[v];
		}

		virtual uint64_t get_id() override {
			/* TODO only unique with regard to the Graph g, hash somehow the graph also in */
			return static_cast<uint64_t>(v);
		}

		friend class GraphImpl<Graph>;
		template <class G, class T>
		friend class SubGraphImpl;

	  private:
		uint64_t convert_size(typename boost::graph_traits<Graph>::degree_size_type size) {
			return static_cast<uint64_t>(size);
		}

		Graph& g;
		typename Graph::vertex_descriptor v;
	};

	template <typename Graph>
	class EdgeImpl : public EdgeWrapper {
	  public:
		EdgeImpl(Graph& g, typename Graph::edge_descriptor e) : g(g), e(std::move(e)) {}
		virtual ~EdgeImpl() {}

		virtual std::unique_ptr<VertexWrapper> source() override {
			return std::make_unique<VertexImpl<Graph>>(g, boost::source(e, g));
		}

		virtual std::unique_ptr<VertexWrapper> target() override {
			return std::make_unique<VertexImpl<Graph>>(g, boost::target(e, g));
		}

		friend class GraphImpl<Graph>;
		template <class G, class T>
		friend class SubGraphImpl;

	  private:
		Graph& g;
		typename Graph::edge_descriptor e;
	};

	template<typename Graph>
	class GraphImpl : public GraphWrapper{
		public:
		  GraphImpl(Graph& g) : graph(g) {}
		  virtual ~GraphImpl() {}

		  virtual std::pair<std::unique_ptr<GraphIterator<VertexWrapper>>,
		                    std::unique_ptr<GraphIterator<VertexWrapper>>>
		  vertices() override {
			  return convert_it<VertexWrapper, VertexImpl<Graph>, Graph, typename Graph::vertex_iterator,
			                    typename Graph::vertex_descriptor>(graph, boost::vertices(graph));
		  }

		virtual uint64_t num_vertices() override {
			auto size = boost::num_vertices(graph);
			return static_cast<uint64_t>(size);
		}

		virtual SamePair<std::unique_ptr<GraphIterator<EdgeWrapper>>> edges() override {
			return convert_it<EdgeWrapper, EdgeImpl<Graph>, Graph, typename Graph::edge_iterator,
			                  typename Graph::edge_descriptor>(graph, boost::edges(graph));
		}

		virtual uint64_t num_edges() override {
			auto size = boost::num_edges(graph);
			return static_cast<uint64_t>(size);
		}

		virtual std::unique_ptr<VertexWrapper> add_vertex() override {
			auto v = boost::add_vertex(graph);
			return std::make_unique<VertexImpl<Graph>>(graph, v);
		}

		/* TODO under construction (in Boost!), see boost/graph/subgraph.h */
		/*
		virtual void remove_vertex(VertexWrapper& vertex) override {
		    auto& v = static_cast<VertexImpl<Graph>&>(vertex);
		    boost::remove_vertex(v.v, graph);
		}
		*/

		virtual std::unique_ptr<EdgeWrapper> add_edge(VertexWrapper& source, VertexWrapper& target) override {
			auto& s = static_cast<VertexImpl<Graph>&>(source);
			auto& t = static_cast<VertexImpl<Graph>&>(target);
			auto e = boost::add_edge(s.v, t.v, graph);
			/* TODO use the bool type of add_edge */
			return std::make_unique<EdgeImpl<Graph>>(graph, e.first);
		}

		virtual void remove_edge(EdgeWrapper& edge) override {
			auto& e = static_cast<EdgeImpl<Graph>&>(edge);
			boost::remove_edge(e.e, graph);
		}
			protected:
		Graph& graph;
	};

	template <typename Graph, typename SubGraph>
	class SubGraphImpl : public GraphImpl<Graph> {

	  public:
		using GraphImpl<Graph>::GraphImpl;

		//subgraph functions
		virtual std::unique_ptr<GraphWrapper> create_subgraph() override {
			auto& g = this->graph.create_subgraph();
			return std::make_unique<SubGraphImpl<SubGraph, SubGraph>>(g);
		}

		virtual bool is_root() override { return this->graph.is_root(); }

		virtual std::unique_ptr<GraphWrapper> root() override {
			auto& g = this->graph.root();
			return std::make_unique<SubGraphImpl<SubGraph, SubGraph>>(g);
		}

		virtual std::unique_ptr<GraphWrapper> parent() override {
			auto& g = this->graph.parent();
			return std::make_unique<SubGraphImpl<SubGraph, SubGraph>>(g);
		}

		virtual SamePair<std::unique_ptr<GraphIterator<GraphWrapper>>> children() override {
			// slightly different logic of convert_to_ptr here, so avoiding convert_it
			auto its = this->graph.children();

			using TransformIterator =
			    boost::transform_iterator<std::function<std::unique_ptr<SubGraphImpl<SubGraph, SubGraph>>(SubGraph&)>,
			                              typename Graph::children_iterator>;
			std::function<std::unique_ptr<SubGraphImpl<SubGraph, SubGraph>>(SubGraph&)> convert_to_ptr =
			    [&](SubGraph& p) -> std::unique_ptr<SubGraphImpl<SubGraph, SubGraph>> {
				return std::make_unique<SubGraphImpl<SubGraph, SubGraph>>(p);
			};

			TransformIterator tfirst = boost::make_transform_iterator(std::move(its.first), convert_to_ptr);
			TransformIterator tsecond = boost::make_transform_iterator(std::move(its.second), convert_to_ptr);
			std::unique_ptr<GraphIterator<GraphWrapper>> first =
			    std::make_unique<GraphIter<GraphWrapper, TransformIterator>>(tfirst);
			std::unique_ptr<GraphIterator<GraphWrapper>> second =
			    std::make_unique<GraphIter<GraphWrapper, TransformIterator>>(tsecond);
			return std::make_pair(std::move(first), std::move(second));
		}

		virtual std::unique_ptr<VertexWrapper> local_to_global(VertexWrapper& vertex) override {
			auto& v = static_cast<VertexImpl<Graph>&>(vertex);
			return std::make_unique<VertexImpl<Graph>>(this->graph, this->graph.local_to_global(v.v));
		}
		virtual std::unique_ptr<EdgeWrapper> local_to_global(EdgeWrapper& edge) override {
			auto& e = static_cast<EdgeImpl<Graph>&>(edge);
			return std::make_unique<EdgeImpl<Graph>>(this->graph, this->graph.local_to_global(e.e));
		}

		virtual std::unique_ptr<VertexWrapper> global_to_local(VertexWrapper& vertex) override {
			auto& v = static_cast<VertexImpl<Graph>&>(vertex);
			return std::make_unique<VertexImpl<Graph>>(this->graph, this->graph.global_to_local(v.v));
		}
		virtual std::unique_ptr<EdgeWrapper> global_to_local(EdgeWrapper& edge) override {
			auto& e = static_cast<EdgeImpl<Graph>&>(edge);
			return std::make_unique<EdgeImpl<Graph>>(this->graph, this->graph.global_to_local(e.e));
		}

		virtual std::unique_ptr<GraphWrapper> filter_by(Predicate, Predicate) override {
			/* TODO */
			return nullptr;
		}
	};
}
