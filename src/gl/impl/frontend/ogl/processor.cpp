#if TZ_OGL
#include "gl/impl/frontend/ogl/processor.hpp"
#include "gl/impl/frontend/ogl/convert.hpp"
#include "gl/resource.hpp"
#include "core/report.hpp"
#include "core/profiling/zone.hpp"

namespace tz::gl
{
    ResourceHandle ProcessorBuilderOGL::add_resource(const IResource& resource)
    {
        std::size_t total_resource_size = this->buffer_resources.size() + this->texture_resources.size();
        switch(resource.get_type())
        {
            case ResourceType::Buffer:
                this->buffer_resources.push_back(&resource);
            break;
            case ResourceType::Texture:
                this->texture_resources.push_back(&resource);
            break;
            default:
                tz_error("Unsupported ResourceType");
                return {static_cast<tz::HandleValue>(0)};
            break;
        }
        return static_cast<tz::HandleValue>(total_resource_size);
    }

    const IResource* ProcessorBuilderOGL::get_resource(ResourceHandle handle) const
    {
        auto handle_value = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
        if(handle_value >= this->buffer_resources.size())
        {
            // Handle value doesn't fit within buffer resources, must be a texture resource or invalid.
            if(handle_value < this->buffer_resources.size() + this->texture_resources.size())
            {
                // Is within range, we assume it's a valid texture resource
                return this->texture_resources[handle_value - this->buffer_resources.size()];
            }
            else
            {
                // Invalid, probably someone else's ResourceHsndle
                return nullptr;
            }
        }
        // Is within range, we assume it's a valid buffer resource
        return this->buffer_resources[handle_value];
    }

    void ProcessorBuilderOGL::set_shader(const Shader& shader)
    {
        this->shader = &shader;
    }

    const Shader& ProcessorBuilderOGL::get_shader() const
    {
        tz_assert(this->shader != nullptr, "No shader was set previously so cannot retrieve the current shader");
        return *this->shader;
    }

    std::span<const IResource* const> ProcessorBuilderOGL::ogl_get_buffer_resources() const
    {
        return this->buffer_resources;
    }

    std::span<const IResource* const> ProcessorBuilderOGL::ogl_get_texture_resources() const
    {
        return this->texture_resources;
    }

    ProcessorOGL::ProcessorOGL(ProcessorBuilderOGL builder):
    buffer_resources(),
    texture_resources(),
    buffer_components(),
    texture_components(),
    shader(&builder.get_shader())
    {
        for(const IResource* const buf_res : builder.ogl_get_buffer_resources())
        {
            this->buffer_resources.push_back(buf_res->unique_clone());
        }
        for(const IResource* const tex_res : builder.ogl_get_texture_resources())
        {
            this->texture_resources.push_back(tex_res->unique_clone());
        }
        this->setup_buffers();
        this->setup_textures();
    }

    std::size_t ProcessorOGL::resource_count() const
    {
        return this->buffer_resources.size() + this->texture_resources.size();
    }

    std::size_t ProcessorOGL::resource_count_of(ResourceType type) const
    {
        switch(type)
        {
            case ResourceType::Buffer:
                return this->buffer_resources.size();
            break;
            case ResourceType::Texture:
                return this->texture_resources.size();
            break;
            default:
                tz_error("Unrecognised ResourceType (OGL)");
                return 0;
            break;
        }
    }

    IResource* ProcessorOGL::get_resource(ResourceHandle handle)
    {
        auto handle_value = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
        if(handle_value < this->resource_count_of(ResourceType::Buffer))
        {
            return this->buffer_resources[handle_value].get();
        }
        else if(handle_value < this->resource_count_of(ResourceType::Texture))
        {
            return this->texture_resources[handle_value - this->resource_count_of(ResourceType::Buffer)].get();
        }
        else
        {
            tz_error("Invalid resource handle value %zu", handle_value);
            return nullptr;
        }
    }

    void ProcessorOGL::process()
    {
        //std::printf("ayy lmao\n");
        {
            TZ_PROFZONE("Frontend OGL : Bind Buffer Resources", TZ_PROFCOL_RED);
            for(std::size_t i = 0; i < this->buffer_components.size(); i++)
            {
                const BufferComponentOGL& res_buf = this->buffer_components[i];
                res_buf.get_buffer().bind_base(static_cast<GLuint>(i));
            }
        }
        glUseProgram(this->shader->ogl_get_program_handle());
        glDispatchCompute(1, 1, 1);
    }

    void ProcessorOGL::setup_buffers()
    {
        for(std::size_t i = 0; i < this->buffer_resources.size(); i++)
        {
            IResource* buffer_resource = this->buffer_resources[i].get();
            BufferComponentOGL& buffer = this->buffer_components.emplace_back(buffer_resource);
            std::span<const std::byte> buffer_data = buffer_resource->get_resource_bytes();
            ogl::BufferUsage usage;
            switch(buffer_resource->data_access())
            {
                case RendererInputDataAccess::StaticFixed:
                    usage = ogl::BufferUsage::ReadWrite;
                break;
                case RendererInputDataAccess::DynamicFixed:
                    usage = ogl::BufferUsage::PersistentMapped;
                break;
                default:
                    tz_error("Resource type not yet implemented (OGL)");
                break;
            }
            ogl::BufferType buftype;
            const tz::gl::ShaderMeta& meta = this->shader->get_meta();
            ShaderMetaValue value = meta.try_get_meta_value(i).value_or(ShaderMetaValue::UBO);
            switch(value)
            {
                case ShaderMetaValue::UBO:
                    buftype = ogl::BufferType::Uniform;
                break;
                case ShaderMetaValue::SSBO:
                    buftype = ogl::BufferType::ShaderStorage;
                break;
                default:
                {
                    const char* meta_value_name = detail::meta_value_names[static_cast<int>(value)];
                    tz_error("Unexpected Shader meta value. Expecting a buffer-y meta value, but instead got \"%s\"", meta_value_name);
                    return;
                }
                break;
            }
            
            ogl::Buffer buf{buftype, ogl::BufferPurpose::StaticDraw, usage, buffer_data.size_bytes()};
            buf.write(buffer_resource->get_resource_bytes().data(), buffer_resource->get_resource_bytes().size_bytes());
            if(buffer_resource->data_access() == RendererInputDataAccess::DynamicFixed)
            {
                auto& dynamic_buf = *static_cast<IDynamicResource*>(buffer_resource);
                void* res_data = buf.map_memory();
                dynamic_buf.set_resource_data(static_cast<std::byte*>(res_data));
            }
            buffer.set_buffer(std::move(buf));
        }
    }

    void ProcessorOGL::setup_textures()
    {
        for(std::size_t i = 0; i < texture_resources.size(); i++)
        {
            auto* texture_resource = static_cast<TextureResource*>(texture_resources[i].get());
            tz_report("Texture Resource (ResourceID: %zu, TextureComponentID: %zu, %zu bytes total)", buffer_resources.size() + i, i, texture_resource->get_resource_bytes().size_bytes());

            ogl::Texture::Format format = to_ogl(texture_resource->get_format());

            TextureProperties gl_props = texture_resource->get_properties();
            ogl::TextureParameters ogl_params;
            auto to_ogl_filter = [](TexturePropertyFilter filter) -> GLint
            {
                switch(filter)
                {
                    case TexturePropertyFilter::Nearest:
                        return GL_NEAREST;
                    break;
                    case TexturePropertyFilter::Linear:
                        return GL_LINEAR;
                    break;
                    default:
                        tz_error("Unrecognised Resource TexturePropertyFilter (OpenGL)");
                        return 0;
                    break;
                }
            };

            auto to_ogl_addrmode = [](TextureAddressMode mode) -> GLint
            {
                switch(mode)
                {
                    case TextureAddressMode::ClampToEdge:
                        return GL_CLAMP_TO_EDGE;
                    break;
                    default:
                        tz_error("Unrecognised Resource TextureAddressMode (OpenGL)");
                        return 0;
                    break;
                }
            };

            ogl_params.min_filter = to_ogl_filter(gl_props.min_filter);
            ogl_params.mag_filter = to_ogl_filter(gl_props.mag_filter);
            ogl_params.tex_wrap_s = to_ogl_addrmode(gl_props.address_mode_u);
            ogl_params.tex_wrap_t = to_ogl_addrmode(gl_props.address_mode_v);
            ogl_params.tex_wrap_u = to_ogl_addrmode(gl_props.address_mode_w);
            /*
            glTextureParameteri(tex, GL_TEXTURE_WRAP_S, convert_address_mode(gl_props.address_mode_u));
			glTextureParameteri(tex, GL_TEXTURE_WRAP_T, convert_address_mode(gl_props.address_mode_v));
            glTextureParameteri(tex, GL_TEXTURE_WRAP_R, convert_address_mode(gl_props.address_mode_w));
			glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, convert_filter(gl_props.min_filter));
			glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, convert_filter(gl_props.min_filter));
            */

            GLsizei tex_w = texture_resource->get_width();
            GLsizei tex_h = texture_resource->get_height();
            ogl::Texture tex{tex_w, tex_h, format, ogl_params};
            tex.set_image_data(texture_resource->get_resource_bytes().data(), texture_resource->get_resource_bytes().size_bytes());
            this->texture_components.emplace_back(texture_resource, std::move(tex));
            //glTextureStorage2D(tex, 1, internal_format, tex_w, tex_h);
            //glTextureSubImage2D(tex, 0, 0, 0, tex_w, tex_h, format, type, texture_resource->get_resource_bytes().data());
        }
    }
}

#endif // TZ_OGL