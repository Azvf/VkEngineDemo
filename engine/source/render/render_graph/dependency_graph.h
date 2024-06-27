#pragma once

#include <stdint.h>
#include <memory>

namespace Chandelier
{
    class DependencyGraphNode;
    class DependencyGraphEdge;

    using RDG_ID = int32_t;

    class DependencyGraph
    {
    protected:
        using Node = DependencyGraphNode;
        using Edge = DependencyGraphEdge;

    public:
        DependencyGraph()          = default;
        virtual ~DependencyGraph() = default;

        static std::shared_ptr<DependencyGraph> Build();

        virtual RDG_ID Insert(Node* node)   = 0;
        virtual Node*  Get(RDG_ID id)       = 0;
        virtual Node*  FromNode(Edge* edge) = 0;
        virtual Node*  ToNode(Edge* edge)   = 0;

        virtual void Link(Node* from, Node* to, Edge* edge) = 0;
        virtual void Remove(RDG_ID id)                      = 0;
        virtual void Remove(Node* node)                     = 0;
        virtual void Clear()                                = 0;
    };

    class DependencyGraphNode
    {
    public:
        friend class DependencyGraphImpl;
        
        DependencyGraphNode()          = default;
        virtual ~DependencyGraphNode() = default;

        RDG_ID GetID() { return m_id; }

        virtual void OnInsert() {}
        virtual void OnRemove() {}

    protected:
        DependencyGraph* m_graph = nullptr;
        RDG_ID           m_id    = -1;
    };

    class DependencyGraphEdge
    {
    public:
        friend class DependencyGraphImpl;

        DependencyGraphEdge() = default;
        virtual ~DependencyGraphEdge() = default;

        virtual void OnConnect() {}
        virtual void OnDisconnect() {}

        DependencyGraphNode* From() { return m_grpah->Get(m_from); }
        DependencyGraphNode* to() { return m_grpah->Get(m_to); }

    protected:
        DependencyGraph* m_grpah = nullptr;
        RDG_ID           m_from  = -1;
        RDG_ID           m_to    = -1;
    };

}