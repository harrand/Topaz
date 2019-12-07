namespace tz::gl
{
	template<typename... Args>
	OGLMeshElement& OGLMeshBuffer::emplace_mesh(Args&&... args)
	{
		this->meshes.push_back(std::make_unique<OGLMeshElement>(std::forward<Args>(args)...));
		OGLMeshElement& mesh = *this->meshes.back();
		// Upload all its data to GPU.
		this->integrate_mesh(mesh);
		return mesh;
	}
}