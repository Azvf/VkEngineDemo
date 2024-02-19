//#pragma once
//
//namespace Chandelier
//{
//    enum BindType : uint8_t
//    {
//        None_Type = 0,
//        Bind_Uniform_Buffer,
//        Bind_Storage_Buffer,
//        Bind_Sampler,
//        Bind_Image,
//    };
//
//    /**
//     * Super class for resources that can be bound to a shader.
//     */
//    class BindableResource
//    {
//    protected:
//        virtual ~BindableResource();
//
//    public:
//        /**
//         * Bind the resource to the shader.
//         */
//        virtual void bind(int binding, BindType type) = 0;
//
//    protected:
//        void unbind_from_active_context();
//    };
//} // namespace Chandelier