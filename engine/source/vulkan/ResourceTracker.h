#pragma once

#include "VkCommon.h"

namespace Chandelier
{

    struct SubmissionID
    {
    private:
        int64_t id_ = -1;

    public:
        SubmissionID() = default;

    private:
        /**
         * Reset the submission id.
         *
         * This should only be called during initialization of the command buffer.
         * As it leads to undesired behavior after resources are already tracking
         * the submission id.
         */
        void reset() { id_ = 0; }

        /**
         * Change the submission id.
         *
         * Is called when submitting a command buffer to the queue. In this case resource
         * known that the next time it is used that it can free its sub resources used by
         * the previous submission.
         */
        void next() { id_++; }

    public:
        const SubmissionID& operator=(const SubmissionID& other)
        {
            id_ = other.id_;
            return *this;
        }

        bool operator==(const SubmissionID& other) { return id_ == other.id_; }

        bool operator!=(const SubmissionID& other) { return id_ != other.id_; }

        friend class CommandBuffers;
    };

    /**
     * Submission tracker keeps track of the last known submission id of the
     * command buffer.
     */
    class SubmissionTracker
    {
        SubmissionID m_last_subid;

    public:
        /**
         * Check if the submission_id has changed since the last time it was called
         * on this SubmissionTracker.
         */
        bool Changed(VKContext* context);
    };

    /**
     * ResourceTracker will keep track of resources.
     */
    template<class T>
    class ResourceTracker
    {
        SubmissionTracker               m_sub_tracker;
        std::vector<std::unique_ptr<T>> m_resources;

    protected:
        ResourceTracker<T>() = default;
        ResourceTracker<T>(ResourceTracker<T>&& other) :
            m_sub_tracker(other.m_sub_tracker), m_resources(std::move(other.m_resources))
        {}

        ResourceTracker<T>& operator=(ResourceTracker<T>&& other)
        {
            m_sub_tracker = other.m_sub_tracker;
            m_resources   = std::move(other.m_resources);
            return *this;
        }

        virtual ~ResourceTracker() { FreeTrackedResources(); }

        /**
         * Get a resource what can be used by the resource tracker.
         *
         * When a different submission was detected all previous resources
         * will be freed and a new resource will be returned.
         *
         * When still in the same submission and we need to update the resource
         * (is_dirty=true) then a new resource will be returned. Otherwise
         * the previous used resource will be used.
         *
         * When no resources exists, a new resource will be created.
         *
         * The resource given back is owned by this resource tracker. And
         * the resource should not be stored outside this class as it might
         * be destroyed when the next submission is detected.
         */
        std::unique_ptr<T>& UpdateResources(VKContext* context, const bool is_dirty)
        {
            if (m_sub_tracker.Changed(context))
            {
                FreeTrackedResources();
                m_resources.push_back(CreateResource(context));
            }
            else if (is_dirty || m_resources.is_empty())
            {
                m_resources.push_back(CreateResource(context));
            }
            return GetResource();
        }

        /**
         * Callback to create a new resource. Can be called by the `tracked_resource_for` method.
         */
        virtual std::unique_ptr<T> CreateResource() = 0;

        /**
         * Does this instance have an active resource.
         */
        bool HasActiveResource() { return !m_resources.is_empty(); }

        /**
         * Return the active resource of the tracker.
         */
        std::unique_ptr<T>& GetResource() { return m_resources.last(); }

    private:
        void FreeTrackedResources() { m_resources.clear(); }
    };

} // namespace Chandelier