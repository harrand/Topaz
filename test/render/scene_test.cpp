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

tz::test::Case adding()
{
    tz::test::Case test_case("tz::render::Scene addition tests");
    TestScene scene{tz::mem::Block::null()};
    topaz_expect(test_case, scene.size() == 0, "Newly constructed scene had size of ", scene.size(), ", expected 0.");
	auto red = scene.add({1});
	NullSceneElement yellow_ele{2};
	auto yellow = scene.add(yellow_ele);
	auto orange = scene.add({3});
	NullSceneElement black{4};
	topaz_expect(test_case, scene.contains(scene.get(red)), "Scene wrongly believes it doesn't contain the red element.");
	topaz_expect(test_case, scene.contains(scene.get(yellow)), "Scene wrongly believes it doesn't contain the red element.");
	topaz_expect(test_case, scene.contains(scene.get(orange)), "Scene wrongly believes it doesn't contain the red element.");
	topaz_expect(test_case, !scene.contains(black), "Scene wrongly believes it contains the black element.");
	auto yellow_cpy = yellow_ele; // Actual copy of yellow_ele.
	NullSceneElement yellow_alias{2}; // Not explicitly copied but same data as yellow_ele.
	topaz_expect(test_case, scene.contains(yellow_cpy), "Scene wrongly believes it doesn't contain the yellow element (copy)");
	topaz_expect(test_case, scene.contains(yellow_alias), "Scene wrongly believes it doesn't contain the yellow element (alias)");

    return test_case;
}

int main()
{
	tz::test::Unit scene;

	// We require topaz to be initialised.
	{
		tz::core::initialise("Render Scene Tests");
		scene.add(adding());
		tz::core::terminate();
	}
	return scene.result();
}