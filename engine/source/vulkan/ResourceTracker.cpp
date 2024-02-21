#include "ResourceTracker.h"

#include "VkContext.h"

namespace Chandelier
{
    bool SubmissionTracker::Changed(VKContext* context)
    {
        auto& command_manager = context->GetCommandManager();
        auto& current_id      = command_manager.GetSubmissionId();
        if (m_last_subid != current_id)
        {
            m_last_subid = current_id;
            return true;
        }
        return false;
    }

} // namespace Chandelier