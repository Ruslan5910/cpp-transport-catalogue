#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>
#include <string>

namespace graph {

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight>
struct Edge {
    VertexId from;
    VertexId to;
    Weight weight;
    std::string route_name;
};

template <typename Weight>
class DirectedWeightedGraph {
private:
    // вектор инцидентных ребер
    using IncidenceList = std::vector<EdgeId>;
    
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

public:
    DirectedWeightedGraph() = default;
    
    // параметризованный конструктор, задает количество вершин
    explicit DirectedWeightedGraph(size_t vertex_count);
    
    EdgeId AddEdge(const Edge<Weight>& edge);

    size_t GetVertexCount() const;
    
    size_t GetEdgeCount() const;
    
    const Edge<Weight>& GetEdge(EdgeId edge_id) const;
    
    // Принимает идентификатор на вершину и возвращает диапазон инцидентных ребер
    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

private:
    std::vector<Edge<Weight>> edges_;
    
    // вектор списков инцидентных ребер (элемент вектора - номер вершины. содержатся номера ребер)
    std::vector<IncidenceList> incidence_lists_;
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {
}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    incidence_lists_.at(edge.from).push_back(id);
    return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
    return incidence_lists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
    return edges_.size();
}

template <typename Weight>
const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
    return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
    return ranges::AsRange(incidence_lists_.at(vertex));
}
}  // namespace graph
