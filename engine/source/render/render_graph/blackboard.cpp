//#include "blackboard.h"
//
//namespace Chandelier
//{
//    void BlackboardImpl::clear()
//    {
//        m_pass_nodes.clear();
//        m_tex_nodes.clear();
//        m_buf_nodes.clear();
//    }
//
//    std::shared_ptr<PassNode> BlackboardImpl::pass(std::string_view name)
//    {
//        auto node = m_pass_nodes.find(name.data());
//        if (node != m_pass_nodes.end())
//        {
//            return node->second;
//        }
//        return nullptr;
//    }
//    
//    
//    std::shared_ptr<TextureNode> BlackboardImpl::texture(std::string_view name) {
//        auto node = m_tex_nodes.find(name.data());
//        if (node != m_tex_nodes.end())
//        {
//            return node->second;
//        }
//        return nullptr;
//    }
//    std::shared_ptr<BufferNode>  BlackboardImpl::buffer(std::string_view name) {
//        auto node = m_buf_nodes.find(name.data());
//        if (node != m_buf_nodes.end())
//        {
//            return node->second;
//        }
//        return nullptr;
//    }
//
//    std::optional<double> BlackboardImpl::value(std::string_view name) {
//        auto value = m_values.find(name.data());
//        if (value != m_values.end())
//        {
//            return std::optional<double>(value->second);
//        }
//        return std::nullopt;
//    }
//
//    void BlackboardImpl::add_pass(std::string_view name, std::shared_ptr<PassNode> pass) {
//        auto node = m_pass_nodes.find(name.data());
//        if (node != m_pass_nodes.end())
//        {
//            throw;
//        }
//
//        m_pass_nodes.emplace(node->get_name(), pass);
//    }
//
//    void BlackboardImpl::add_texture(std::string_view name, std::shared_ptr<TextureNode> texture) {}
//    void BlackboardImpl::add_buffer(std::string_view name, std::shared_ptr<BufferNode> buffer) {}
//    void BlackboardImpl::set_value(std::string_view name, double v) {}
//
//    void BlackboardImpl::override_pass(std::string_view name, std::shared_ptr<PassNode> pass) {}
//    void BlackboardImpl::override_texture(std::string_view name, std::shared_ptr<TextureNode> texture) {}
//    void BlackboardImpl::override_buffer(std::string_view name, std::shared_ptr<BufferNode> buffer) {}
//
//} // namespace Chandelier