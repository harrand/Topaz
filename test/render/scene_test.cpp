//
// Created by Harrand on 26/10/2020.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "render/scene.hpp"

struct NullSceneElement
{
	int val;
	tz::gl::Transform get_transform() const{return {};}
	tz::gl::CameraData get_camera_data() const{return {};}
	tz::render::AssetBuffer::Index get_mesh_index() const{return {};}
	bool operator==(const NullSceneElement& rhs) const
	{
		return this->val == rhs.val;
	}
};

using TestScene = tz::render::Scene<NullSceneElement>;

namespace tz::render
{
	template<>
	class ElementWriter<NullSceneElement, tz::gl::TransformResourceWriter>
	{
	public:
        static void write(tz::gl::TransformResourceWriter& writer, const NullSceneElement& element)
        {
			const tz::gl::Transform& trans = element.get_transform();
			const tz::gl::CameraData& cam = element.get_camera_data();
            writer.write(trans.position, trans.rotation, trans.scale, cam.position, cam.rotation, cam.fov, cam.aspect_ratio, cam.near, cam.far);
        }
	};
}

TZ_TEST_BEGIN(adding)
    TestScene scene{tz::mem::Block::null()};
    topaz_expect(scene.size() == 0, "Newly constructed scene had size of ", scene.size(), ", expected 0.");
	auto red = scene.add({1});
	NullSceneElement yellow_ele{2};
	auto yellow = scene.add(yellow_ele);
	auto orange = scene.add({3});
	NullSceneElement black{4};
	topaz_expect(scene.contains(scene.get(red)), "Scene wrongly believes it doesn't contain the red element.");
	topaz_expect(scene.contains(scene.get(yellow)), "Scene wrongly believes it doesn't contain the red element.");
	topaz_expect(scene.contains(scene.get(orange)), "Scene wrongly believes it doesn't contain the red element.");
	topaz_expect(!scene.contains(black), "Scene wrongly believes it contains the black element.");
	auto yellow_cpy = yellow_ele; // Actual copy of yellow_ele.
	NullSceneElement yellow_alias{2}; // Not explicitly copied but same data as yellow_ele.
	topaz_expect(scene.contains(yellow_cpy), "Scene wrongly believes it doesn't contain the yellow element (copy)");
	topaz_expect(scene.contains(yellow_alias), "Scene wrongly believes it doesn't contain the yellow element (alias)");
TZ_TEST_END

TZ_TEST_BEGIN(removing)
	TestScene scene{tz::mem::Block::null()};
	for(int i = 0; i < 100; i++)
	{
		scene.add({i});
	}
	topaz_expect(scene.size() == 100, "Scene has unexpected size. Expected 100, got ", scene.size());
	topaz_expect(scene.erase(NullSceneElement{99}), "Scene erasure unexpectedly returned false.");
	topaz_expect(scene.size() == 99, "Scene has unexpected size. Expected 99, got ", scene.size());
	for(const NullSceneElement& ele : scene)
	{
		topaz_expect(ele.val != 99, "Scene contained element data after deletion (Range-based for)");
	}
	topaz_expect(!scene.contains(NullSceneElement{99}) && !scene.contains(TestScene::Handle{99}), "Scene contained element data after deletion (Scene::contains)");
	for(std::size_t i = 50; i < 99; i++)
	{
		topaz_assert(scene.contains(TestScene::Handle{i}), "Scene does not contain element which I haven't erased yet (", i, ")");
		topaz_expect(scene.erase(TestScene::Handle{i}), "Scene erasure unexpectedly returned false");
	}
	topaz_expect(scene.size() == 50, "Scene had unexpected size. Expected 50, got ", scene.size());
	scene.pack();
TZ_TEST_END

int main()
{
	tz::test::Unit scene;

	// We require topaz to be initialised.
	{
		tz::core::initialise("Render Scene Tests", tz::core::invisible_tag);
		scene.add(adding());
		scene.add(removing());
		tz::core::terminate();
	}
	return scene.result();
}