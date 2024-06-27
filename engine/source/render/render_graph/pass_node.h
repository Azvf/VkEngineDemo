#pragma once

#include <vector>

#include "render_graph_base.h"

namespace Chandelier
{
    class PassNode : public RenderGraphNode
    {
        friend class RenderGraph;

        public:


        protected:
            PassNode(EPassType pass_type, uint32_t order);

    };

}