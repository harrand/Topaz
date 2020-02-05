#include "gl/manager.hpp"

namespace tz::gl
{
    Manager::Manager(): o() // Unspecified initial values for Buffer ids.
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
                    std::size_t end_index = this->static_data()->size();
                    this->static_data()->safe_resize(end_index + soa.size_bytes());
                    // Now we definitely have enough space.
                    // Start chucking it all in.
                    // Positions
                    {
                        tz::mem::Block pos_block{soa.positions.data(), soa.positions.size() * sizeof(float) * 3};
                        this->static_data()->send(end_index, pos_block);
                        this->static_data()->region(end_index, pos_block.size(), names.positions_name);
                        end_index += pos_block.size();
                    }
                    // Texcoords
                    {
                        tz::mem::Block tex_block{soa.texture_coordinates.data(), soa.texture_coordinates.size() * sizeof(float) * 2};
                        this->static_data()->send(end_index, tex_block);
                        this->static_data()->region(end_index, tex_block.size(), names.texcoords_name);
                        end_index += tex_block.size();
                    }
                    // Normals
                    {
                        tz::mem::Block nrm_block{soa.normals.data(), soa.normals.size() * sizeof(float) * 3};
                        this->static_data()->send(end_index, nrm_block);
                        this->static_data()->region(end_index, nrm_block.size(), names.normals_name);
                        end_index += nrm_block.size();
                    }
                    // Tangents
                    {
                        tz::mem::Block tng_block{soa.tangents.data(), soa.tangents.size() * sizeof(float) * 3};
                        this->static_data()->send(end_index, tng_block);
                        this->static_data()->region(end_index, tng_block.size(), names.tangents_name);
                        end_index += tng_block.size();
                    }
                    // Bi-tangents
                    {
                        tz::mem::Block btn_block{soa.bi_tangents.data(), soa.bi_tangents.size() * sizeof(float) * 3};
                        this->static_data()->send(end_index, btn_block);
                        this->static_data()->region(end_index, btn_block.size(), names.bi_tangents_name);
                        end_index += btn_block.size();
                    }
                    topaz_assert(end_index == this->static_data()->size(), "tz::gl::Manager::add_data(...): Static data store had unexpected size after adding. Expected ", end_index, "B in size, but it is ", this->static_data()->size(), "B in size instead.");
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

    void Manager::add_indices(tz::gl::Indices type, const MeshIndices& indices, std::string region_name)
    {
        switch(type)
        {
            case Indices::Static:
                {
                    std::size_t end_index = this->static_indices()->size();
                    this->static_indices()->safe_resize(end_index + (indices.indices.size() * sizeof(unsigned int)));
                    // TODO: Implement tz::mem::ConstBlock and use it here instead of this dirty cast.
                    // While it's safe (we're not editing it), it's a bit disgusting and I'm pretty ashamed of this.
                    tz::mem::Block blk{const_cast<unsigned int*>(indices.indices.data()), indices.indices.size() * sizeof(unsigned int)};
                    this->static_indices()->send(end_index, blk);
                    this->static_indices()->region(end_index, blk.size(), region_name);
                }
            break;
            case Indices::Dynamic:
                topaz_assert(false, "tz::gl::Manager::add_indices(<DYNAMIC>): Not Yet Implemented.");
            break;
            default:
                topaz_assert(false, "tz::gl::Manager::add_indices(...): Invalid Indices specifier. Ignoring add_indices request.");
            break;
        }
    }

    void Manager::add_mesh(Data dtype, Indices itype, const Mesh& mesh)
    {
        this->add_data(dtype, mesh.data, mesh.data_names);
        this->add_indices(itype, mesh.indices, mesh.indices_names);
    }

    tz::mem::OwningBlock Manager::get_data(Data type, const std::string& region_name)
    {
        switch(type)
        {
            case Data::Static:
                {
                    auto region = (*this->static_data())[region_name];
                    // Create an owning block to store the data in.
                    tz::mem::OwningBlock oblk{region.size_bytes};
                    this->static_data()->retrieve(region.offset, region.size_bytes, oblk.begin);
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

    tz::mem::OwningBlock Manager::get_indices(Indices type, const std::string& region_name)
    {
        switch(type)
        {
            case Indices::Static:
                {
                    auto region = (*this->static_indices())[region_name];
                    // Create an owning block to store the data in.
                    tz::mem::OwningBlock oblk{region.size_bytes};
                    this->static_indices()->retrieve(region.offset, region.size_bytes, oblk.begin);
                    return std::move(oblk);
                }
                break;
            case Indices::Dynamic:
                topaz_assert(false, "tz::gl::Manager::get_indices(<DYNAMIC DATA>): Not Yet Implemented.");
                break;
            default:
                topaz_assert(false, "tz::gl::Manager::get_indices(...): Invalid Indices specifier. Would return empty block.");
                return {0};
                break;
        }
    }

    void Manager::attrib(Data type, std::string region_name, tz::gl::Format fmt)
    {
        std::size_t idx;
        tz::gl::IManagedBuffer* buf;
        switch(type)
        {
            case Data::Static:
                idx = this->static_data_id;
                buf = this->static_data();
            break;
            case Data::Dynamic:
                idx = this->dynamic_data_id;
                buf = this->dynamic_data();
            break;
        }
        auto region = buf->get(region_name);
        fmt.offset = region.offset;
        this->o.format(idx, fmt);
    }

    tz::gl::Object* Manager::operator->()
    {
        return &(this->o);
    }

    const tz::gl::Object* Manager::operator->() const
    {
        return &(this->o);
    }

    tz::gl::Object& Manager::operator*()
    {
        return this->o;
    }

    const tz::gl::Object& Manager::operator*() const
    {
        return this->o;
    }

    void Manager::init()
    {
        this->static_data_id = this->o.emplace_managed_buffer<tz::gl::BufferType::Array>();
        this->dynamic_data_id = this->o.emplace_managed_terminal_buffer<tz::gl::BufferType::Array>(tz::gl::deferred_terminal_tag{});
        this->static_indices_id = this->o.emplace_managed_buffer<tz::gl::BufferType::Index>();
        this->dynamic_indices_id = this->o.emplace_managed_terminal_buffer<tz::gl::BufferType::Index>(tz::gl::deferred_terminal_tag{});
    }

    tz::gl::ManagedVBO* Manager::static_data()
    {
        return this->o.get_managed<tz::gl::BufferType::Array>(this->static_data_id);
    }

    tz::gl::ManagedTVBO* Manager::dynamic_data()
    {
        return this->o.get_managed_terminal<tz::gl::BufferType::Array>(this->dynamic_data_id);
    }

    tz::gl::ManagedIBO* Manager::static_indices()
    {
        return this->o.get_managed<tz::gl::BufferType::Index>(this->static_indices_id);
    }

    tz::gl::ManagedTIBO* Manager::dynamic_indices()
    {
        return this->o.get_managed_terminal<tz::gl::BufferType::Index>(this->dynamic_indices_id);
    }
}