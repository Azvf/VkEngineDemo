#include "dependency_graph.h"

#include <lemon/list_graph.h>

namespace Chandelier
{
    using namespace lemon;

    using Node = DependencyGraphNode;
    using Edge = DependencyGraphEdge;

    class DependencyGraphImpl : public DependencyGraph
    {
        using Vertex  = ListDigraph::Node;
        using Arc     = ListDigraph::Arc;
        using VertMap = ListDigraph::NodeMap<Node*>;
        using EdgeMap = ListDigraph::ArcMap<Edge*>;
        using DAG     = ListDigraph;

    public:
        DependencyGraphImpl() : m_vert_map(m_graph), m_edge_map(m_graph) {}

        virtual RDG_ID Insert(Node* node) override
        {
            auto added_node = m_graph.addNode();
            node->m_id            = m_graph.id(added_node);
            node->m_graph         = this;
            m_vert_map.set(added_node, node);
            node->OnInsert();
            return node->m_id;
        }

        virtual Node* Get(RDG_ID id) override
        {
            auto node = m_graph.nodeFromId(id);
            return m_vert_map[node];
        }

        virtual void Remove(RDG_ID id) override
        {
            auto node = m_graph.nodeFromId(id);
            m_vert_map[node]->OnRemove();
            m_graph.erase(node);
        }

        virtual void Remove(Node* node) override { Remove(node->GetID()); }

        virtual void Clear() override { m_graph.clear(); }

        virtual void Link(Node* from, Node* to, Edge* edge) override
        {
            auto graph_from_node = m_graph.nodeFromId(from->GetID());
            auto graph_to_node   = m_graph.nodeFromId(to->GetID());
            auto graph_arc       = m_graph.addArc(graph_from_node, graph_to_node);

            edge->m_grpah = this;
            edge->m_from  = from->GetID();
            edge->m_to    = to->GetID();
            m_edge_map.set(graph_arc, edge);
            edge->OnConnect();
        }

        virtual Node* FromNode(Edge* edge) override { return Get(edge->m_from); }

        virtual Node* ToNode(Edge* edge) override { return Get(edge->m_to); }

    private:
        DAG m_graph;
        VertMap m_vert_map;
        EdgeMap m_edge_map;
    };


    std::shared_ptr<DependencyGraph> DependencyGraph::Build()
    {
        return std::make_shared<DependencyGraphImpl>();
    }

    
}