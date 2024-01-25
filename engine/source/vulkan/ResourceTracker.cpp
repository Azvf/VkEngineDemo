#include "ResourceTracker.h"

#include "VkContext.h"

namespace Chandelier
{
    bool SubmissionTracker::Changed(VKContext* context)
    {
        auto& command_buffers = context->GetCommandBuffers();
        auto& current_id      = command_buffers.GetSubmissionId();
        if (m_last_subid != current_id)
        {
            m_last_subid = current_id;
            return true;
        }
        return false;
    }

} // namespace Chandelier