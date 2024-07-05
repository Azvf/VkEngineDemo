//#pragma once
//
//#include <memory>
//#include <string>
//#include <unordered_map>
//#include <optional>
//
//namespace Chandelier
//{
//    class PassNode;
//    class TextureNode;
//    class BufferNode;
//
//    class Blackboard
//    {
//    public:
//        static std::shared_ptr<Blackboard> Create();
//        // static void        Destroy(Blackboard* blackboard);
//
//        virtual ~Blackboard() = default;
//
//        virtual void                         clear()                                 = 0;
//        virtual std::shared_ptr<PassNode>    pass(std::string_view name)             = 0;
//        virtual std::shared_ptr<TextureNode> texture(std::string_view name)          = 0;
//        virtual std::shared_ptr<BufferNode>  buffer(std::string_view name)           = 0;
//        virtual std::optional<double>        value(std::string_view name) = 0;
//
//        virtual void add_pass(std::string_view name, std::shared_ptr<PassNode> pass)          = 0;
//        virtual void add_texture(std::string_view name, std::shared_ptr<TextureNode> texture) = 0;
//        virtual void add_buffer(std::string_view name, std::shared_ptr<BufferNode> buffer)    = 0;
//        virtual void set_value(std::string_view name, double v)                               = 0;
//
//        virtual void override_pass(std::string_view name, std::shared_ptr<PassNode> pass)          = 0;
//        virtual void override_texture(std::string_view name, std::shared_ptr<TextureNode> texture) = 0;
//        virtual void override_buffer(std::string_view name, std::shared_ptr<BufferNode> buffer)    = 0;
//    };
//
//    class BlackboardImpl : public Blackboard
//    {
//    public:
//        virtual void                         clear() override;
//        virtual std::shared_ptr<PassNode>    pass(std::string_view name) override;
//        virtual std::shared_ptr<TextureNode> texture(std::string_view name) override;
//        virtual std::shared_ptr<BufferNode>  buffer(std::string_view name) override;
//        virtual std::optional<double>                         value(std::string_view name) override;
//
//        virtual void add_pass(std::string_view name, std::shared_ptr<PassNode> pass) override;
//        virtual void add_texture(std::string_view name, std::shared_ptr<TextureNode> texture) override;
//        virtual void add_buffer(std::string_view name, std::shared_ptr<BufferNode> buffer) override;
//        virtual void set_value(std::string_view name, double v) override;
//
//        virtual void override_pass(std::string_view name, std::shared_ptr<PassNode> pass) override;
//        virtual void override_texture(std::string_view name, std::shared_ptr<TextureNode> texture) override;
//        virtual void override_buffer(std::string_view name, std::shared_ptr<BufferNode> buffer) override;
//
//    private:
//        template<typename T>
//        using StringHashMap = std::unordered_map<std::string, T>;
//
//        StringHashMap<std::shared_ptr<PassNode>>    m_pass_nodes;
//        StringHashMap<std::shared_ptr<TextureNode>> m_tex_nodes;
//        StringHashMap<std::shared_ptr<BufferNode>>  m_buf_nodes;
//        StringHashMap<double>                       m_values;
//    };
//
//}