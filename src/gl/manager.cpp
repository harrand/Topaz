#include "gl/manager.hpp"

namespace tz::gl
{
    Manager::Manager(): o(), static_data(nullptr), dynamic_data(nullptr), static_indices(nullptr), dynamic_indices(nullptr)
    {
        this->init();   
    }

    void Manager::add_data(Data type, const MeshData& data, const StandardDataRegionNames& names)
    {
        // Organise data into something we recognise.
        tz::gl::VertexList soa{data.vertices.data(), data.vertices.size()};
        
        switch(type)
        {
            case Data::Static:
                {
                    std::size_t end_index = this->static_data->size();
                    this->static_data->safe_resize(end_index + soa.size_bytes());
                    // Now we definitely have enough space.
                    // Start chucking it all in.
                    // Positions
                    {
                        tz::mem::Block pos_block{soa.positions.data(), soa.positions.size() * sizeof(float) * 3};
                        this->static_data->send(end_index, pos_block);
                        this->static_data->region(end_index, pos_block.size(), names.positions_name);
                        end_index += pos_block.size();
                    }
                    // Texcoords
                    {
                        tz::mem::Block tex_block{soa.texture_coordinates.data(), soa.texture_coordinates.size() * sizeof(float) * 2};
                        this->static_data->send(end_index, tex_block);
                        this->static_data->region(end_index, tex_block.size(), names.texcoords_name);
                        end_index += tex_block.size();
                    }
                    // Normals
                    {
                        tz::mem::Block nrm_block{soa.normals.data(), soa.normals.size() * sizeof(float) * 3};
                        this->static_data->send(end_index, nrm_block);
                        this->static_data->region(end_index, nrm_block.size(), names.normals_name);
                        end_index += nrm_block.size();
                    }
                    // Tangents
                    {
                        tz::mem::Block tng_block{soa.tangents.data(), soa.tangents.size() * sizeof(float) * 3};
                        this->static_data->send(end_index, tng_block);
                        this->static_data->region(end_index, tng_block.size(), names.tangents_name);
                        end_index += tng_block.size();
                    }
                    // Bi-tangents
                    {
                        tz::mem::Block btn_block{soa.bi_tangents.data(), soa.bi_tangents.size() * sizeof(float) * 3};
                        this->static_data->send(end_index, btn_block);
                        this->static_data->region(end_index, btn_block.size(), names.bi_tangents_name);
                        end_index += btn_block.size();
                    }
                    topaz_assert(end_index == this->static_data->size(), "tz::gl::Manager::add_data(...): Static data store had unexpected size after adding. Expected ", end_index, "B in size, but it is ", this->static_data->size(), "B in size instead.");
                }
                break;
            case Data::Dynamic:
                topaz_assert(false, "tz::gl::Manager::add_data(<DYNAMIC DATA>): Not Yet Implemented.");
                break;
            default:
                topaz_assert(false, "tz::gl::Manager::add_data(...): Invalid Data specifier. Ignoring add_data request.");
                break;
        }
    }

    tz::mem::OwningBlock Manager::get_data(Data type, const std::string& region_name)
    {
        switch(type)
        {
            case Data::Static:
                {
                    auto region = (*this->static_data)[region_name];
                    // Create an owning block to store the data in.
                    tz::mem::OwningBlock oblk{region.size_bytes};
                    this->static_data->retrieve(region.offset, region.size_bytes, oblk.begin);
                    return std::move(oblk);
                }
                break;
            case Data::Dynamic:
                topaz_assert(false, "tz::gl::Manager::get_data(<DYNAMIC DATA>): Not Yet Implemented.");
                break;
            default:
                topaz_assert(false, "tz::gl::Manager::get_data(...): Invalid Data specifier. Would return empty block.");
                return {0};
                break;
        }
    }

    void Manager::init()
    {
        std::size_t sd = this->o.emplace_managed_buffer<tz::gl::BufferType::Array>();
        this->static_data = this->o.get_managed<tz::gl::BufferType::Array>(sd);

        std::size_t dd = this->o.emplace_managed_terminal_buffer<tz::gl::BufferType::Array>(tz::gl::deferred_terminal_tag{});
        this->dynamic_data = this->o.get_managed_terminal<tz::gl::BufferType::Array>(dd);

        std::size_t si = this->o.emplace_managed_buffer<tz::gl::BufferType::Index>();
        this->static_indices = this->o.get_managed<tz::gl::BufferType::Index>(si);

        std::size_t di = this->o.emplace_managed_terminal_buffer<tz::gl::BufferType::Index>(tz::gl::deferred_terminal_tag{});
        this->dynamic_indices = this->o.get_managed_terminal<tz::gl::BufferType::Index>(di);
    }
}