#pragma once

#include <cassert>
#include <string>

#include "VkCreateInfo.h"

namespace Chandelier
{
    class VKContext;
    class CommandBuffer
    {
        enum Stage : uint8_t
        {
            Initial,
            Recording,
            BetweenRecordingAndSubmitting,
            Submitted,
            Executed,
        };

        struct
        {
            /**
             * Current stage of the command buffer to keep track of inconsistencies & incorrect usage.
             */
            Stage stage = Stage::Initial;

            /**
             * The number of command added to the command buffer since last submission.
             */
            uint64_t recorded_command_counts = 0;
        } m_state;

        bool IsInStage(Stage stage) { return m_state.stage == stage; }

        void SetStage(Stage stage) { m_state.stage = stage; }

        std::string to_string(Stage stage)
        {
            switch (stage)
            {
                case Stage::Initial:
                    return "INITIAL";
                case Stage::Recording:
                    return "RECORDING";
                case Stage::BetweenRecordingAndSubmitting:
                    return "BEFORE_SUBMIT";
                case Stage::Submitted:
                    return "SUBMITTED";
                case Stage::Executed:
                    return "EXECUTED";
            }
            return "UNKNOWN";
        }

        void TransferStage(Stage stage_from, Stage stage_to)
        {
            assert(IsInStage(stage_from));
            (void)(stage_from);
#if 0
            printf(" *** Transfer stage from %s to %s\n",
                   to_string(stage_from).c_str(),
                   to_string(stage_to).c_str());
#endif
            SetStage(stage_to);
        }

    public:
        CommandBuffer() = default;
        ~CommandBuffer();

        friend class VKContext;

        bool Valid();
        void Initialize(std::shared_ptr<VKContext> context, VkCommandPool pool, VkCommandBuffer buffer);
        void Free();
        void BeginRecording();
        void EndRecording();
        void CommandsSubmitted();

        VkCommandBuffer GetCommandBuffer() const { return m_buffer; }

        bool HasRecordedCommands() const { return m_state.recorded_command_counts != 0; }

        void CommandRecorded() { m_state.recorded_command_counts++; }

    private:
        std::shared_ptr<VKContext> m_context;

        VkCommandBuffer m_buffer       = VK_NULL_HANDLE;
        VkCommandPool   m_command_pool = VK_NULL_HANDLE;
    };

} // namespace Chandelier