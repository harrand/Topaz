template<class ObjectType>
RenderableBuffer<ObjectType>::RenderableBuffer(GLuint layout_qualifier_id): objects(), mesh_buffer(), data_ssbo(layout_qualifier_id), matrix_pool(this->data_ssbo.persistently_map<Matrix4x4>(RenderableBuffer::capacity, false)){}

template<class ObjectType>
std::size_t RenderableBuffer<ObjectType>::insert(ObjectType object)
{
    std::size_t id = this->objects.size();
    this->objects[id] = std::make_unique<ObjectType>(std::move(object));
    return id;
}

template<class ObjectType>
void RenderableBuffer<ObjectType>::render(RenderPass pass)
{
    // Update the mvp matrix pool. Write "uniform" data.
    for(const auto&[id, object_ptr] : this->objects)
        matrix_pool[id] = pass.get_camera().projection(pass.get_window().get_width(), pass.get_window().get_height()) * pass.get_camera().view() * object_ptr->transform.model();
    // After the pool is updated. bind the SSBO and initiate one render call.
    this->data_ssbo.bind();
    this->mesh_buffer.render();
}