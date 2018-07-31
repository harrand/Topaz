#include "core/topaz.hpp"
#include "data/vector.hpp"
#include "lib/test_util.hpp"

void test();

int main()
{
    // we need not tz::initialise and terminate here.
    std::cerr << "Vector Test Results: " << std::endl;
    try
    {
        test();
        std::cerr << "PASS\n";
    }catch(const TestFailureException& fail)
    {
        std::cerr << "FAIL: " << fail.what() << "\n";
    }
    return 0;
}

void test_initialisation_2D()
{
    // test normal construction:
    Vector2F temp(1, 2);
    tz::assert::equal(temp.x, 1);
    tz::assert::equal(temp.y, 2);
    // this is guaranteed to be zeroed by the standard. ensure that happens.
    Vector2F vec2;
    tz::assert::equal(vec2.x, 0);
    tz::assert::equal(vec2.y, 0);
    float dx = 12.54f, dy = 98798.9f;
    vec2.x = dx;
    vec2.y = dy;
    // member assign to discrete values.
    // ensure copy construction works:
    Vector2F vec2_2(vec2);
    tz::assert::equal(vec2_2.x, dx);
    tz::assert::equal(vec2_2.y, dy);
    // ensure move construction works:
    Vector2F vec2_3(std::move(vec2_2));
    tz::assert::equal(vec2_3.x, dx);
    tz::assert::equal(vec2_3.y, dy);
    // ensure copy assignment works:
    vec2_3 = temp;
    tz::assert::equal(vec2_3.x, 1);
    tz::assert::equal(vec2_3.y, 2);
}

void test_swizzle_2D()
{
    // test swizzles
    Vector2 yes{0, 1};
    Vector2 yesyes = yes.xy();
    tz::assert::equal(yesyes.x, 0);
    tz::assert::equal(yesyes.y, 1);
    Vector2 sey = yes.yx();
    tz::assert::equal(sey.x, 1);
    tz::assert::equal(sey.y, 0);
}

void test_methods_2D()
{
    Vector2F a{1, 2};
    // test Vector2F::normalised();
    Vector2F normal = a.normalised();
    tz::assert::equal(std::round(normal.length()), 1);
    // normalise (1, 2) = 1/sqrt5, 2/sqrt5
    float inverse_sqrt5 = 1.0f / std::sqrt(5.0f);
    // test Vector2F::length()
    tz::assert::equal(normal.x, inverse_sqrt5);
    tz::assert::equal(normal.y, 2.0f * inverse_sqrt5);
    tz::assert::equal(a.length(), std::sqrt(5.0f));

    // [1, 2] . [2, 1] = 1*2 + 2*1 = 4
    tz::assert::equal(a.dot(Vector2F{2, 1}), 4.0f);
    // operators.
    tz::assert::equal((a + Vector2F{1, 0}), Vector2F{2, 2});
    tz::assert::equal((a - Vector2F{1, 2}), Vector2F{});
    tz::assert::equal((a * 2.0f), Vector2F{2, 4});
    tz::assert::equal((a / 2.0f), Vector2F{0.5f, 1.0f});
    tz::assert::that(a < Vector2F{2, 3});
    tz::assert::that(a > Vector2F{0, 1});
    tz::assert::that((a <= Vector2F{1, 2}) && (a >= Vector2F{1, 2}));
}

void test_initialisation_3D()
{
    // test normal construction:
    Vector3F temp(1, 2, 3);
    tz::assert::equal(temp.x, 1);
    tz::assert::equal(temp.y, 2);
    tz::assert::equal(temp.z, 3);
    // test partial constructions:
    Vector3F temp1(Vector2F(1, 2), 3);
    tz::assert::equal(temp1.x, 1);
    tz::assert::equal(temp1.y, 2);
    tz::assert::equal(temp1.z, 3);

    Vector3F temp2(1, Vector2F(2, 3));
    tz::assert::equal(temp2.x, 1);
    tz::assert::equal(temp2.y, 2);
    tz::assert::equal(temp2.z, 3);
    // this is guaranteed to be zeroed by the standard. ensure that happens.
    Vector3F vec3;
    tz::assert::equal(vec3.x, 0);
    tz::assert::equal(vec3.y, 0);
    tz::assert::equal(vec3.z, 0);
    float dx = 12.54f, dy = 98798.9f, dz = 0.00123f;
    vec3.x = dx;
    vec3.y = dy;
    vec3.z = dz;
    // member assign to discrete values.
    // ensure copy construction works:
    Vector3F vec3_2(vec3);
    tz::assert::equal(vec3_2.x, dx);
    tz::assert::equal(vec3_2.y, dy);
    tz::assert::equal(vec3_2.z, dz);
    // ensure move construction works:
    Vector3F vec3_3(std::move(vec3_2));
    tz::assert::equal(vec3_3.x, dx);
    tz::assert::equal(vec3_3.y, dy);
    tz::assert::equal(vec3_3.z, dz);
    // ensure copy assignment works:
    vec3_3 = temp;
    tz::assert::equal(vec3_3.x, 1);
    tz::assert::equal(vec3_3.y, 2);
    tz::assert::equal(vec3_3.z, 3);
}

void test_swizzle_3D()
{
    // test swizzles:
    Vector3 yes{1, 2, 3};
    Vector3 yesyes = yes.xyz();
    tz::assert::equal(yesyes.x, 1);
    tz::assert::equal(yesyes.y, 2);
    tz::assert::equal(yesyes.z, 3);
    Vector3 sey = yes.zyx();
    tz::assert::equal(sey.x, 3);
    tz::assert::equal(sey.y, 2);
    tz::assert::equal(sey.z, 1);
    Vector3 eys = yes.yxz();
    tz::assert::equal(eys.x, 2);
    tz::assert::equal(eys.y, 1);
    tz::assert::equal(eys.z, 3);
    Vector3 yse = yes.xzy();
    tz::assert::equal(yse.x, 1);
    tz::assert::equal(yse.y, 3);
    tz::assert::equal(yse.z, 2);
    Vector3 esy = yes.yzx();
    tz::assert::equal(esy.x, 2);
    tz::assert::equal(esy.y, 3);
    tz::assert::equal(esy.z, 1);
    Vector3 sye = yes.zxy();
    tz::assert::equal(sye.x, 3);
    tz::assert::equal(sye.y, 1);
    tz::assert::equal(sye.z, 2);
}

void test_methods_3D()
{
    Vector3F a{1, 2, 3};
    // test Vector2F::normalised();
    Vector3F normal = a.normalised();
    tz::assert::equal(std::round(normal.length()), 1);
    // normalise (1, 2, 3) = 1/sqrt14, 2/sqrt14
    float inverse_sqrt14 = 1.0f / std::sqrt(14.0f);
    // test Vector2F::length()
    tz::assert::equal(normal.x, inverse_sqrt14);
    tz::assert::equal(normal.y, 2.0f * inverse_sqrt14);
    tz::assert::equal(normal.z, 3.0f * inverse_sqrt14);
    tz::assert::equal(a.length(), std::sqrt(14.0f));

    // [1, 2, 3] . [3, 2, 1] = 1*3 + 2*2 + 3*1 = 3 + 4 + 3 = 10
    tz::assert::equal(a.dot(Vector3F{3, 2, 1}), 10.0f);
    // operators.
    tz::assert::equal((a + Vector3F{1, 0, -1}), Vector3F{2, 2, 2});
    tz::assert::equal((a - Vector3F{1, 2, 3}), Vector3F{});
    tz::assert::equal((a * 2.0f), Vector3F{2, 4, 6});
    tz::assert::equal((a / 2.0f), Vector3F{0.5f, 1.0f, 1.5f});
    tz::assert::that(a < Vector3F{2, 3, 4});
    tz::assert::that(a > Vector3F{0, 1, 2});
    tz::assert::that((a <= Vector3F{1, 2, 3}) && (a >= Vector3F{1, 2, 3}));
}

void test_initialisation_4D()
{
    // test normal construction:
    Vector4F temp(1, 2, 3, 4);
    tz::assert::equal(temp.x, 1);
    tz::assert::equal(temp.y, 2);
    tz::assert::equal(temp.z, 3);
    tz::assert::equal(temp.w, 4);
    // test partial constructions:

    Vector4F temp1(Vector2F(1, 2), Vector2F(3, 4));
    tz::assert::equal(temp1.x, 1);
    tz::assert::equal(temp1.y, 2);
    tz::assert::equal(temp1.z, 3);
    tz::assert::equal(temp1.w, 4);

    Vector4F temp2(Vector3F(1, 2, 3), 4);
    tz::assert::equal(temp2.x, 1);
    tz::assert::equal(temp2.y, 2);
    tz::assert::equal(temp2.z, 3);
    tz::assert::equal(temp2.w, 4);

    Vector4F temp3(1, Vector3F(2, 3, 4));
    tz::assert::equal(temp3.x, 1);
    tz::assert::equal(temp3.y, 2);
    tz::assert::equal(temp3.z, 3);
    tz::assert::equal(temp3.w, 4);
    // this is guaranteed to be zeroed by the standard. ensure that happens.
    Vector4F vec4;
    tz::assert::equal(vec4.x, 0);
    tz::assert::equal(vec4.y, 0);
    tz::assert::equal(vec4.z, 0);
    tz::assert::equal(vec4.w, 0);
    float dx = 12.54f, dy = 98798.9f, dz = 0.00123f, dw = 99.999f;
    vec4.x = dx;
    vec4.y = dy;
    vec4.z = dz;
    vec4.w = dw;
    // member assign to discrete values.
    // ensure copy construction works:
    Vector4F vec4_2(vec4);
    tz::assert::equal(vec4_2.x, dx);
    tz::assert::equal(vec4_2.y, dy);
    tz::assert::equal(vec4_2.z, dz);
    tz::assert::equal(vec4_2.w, dw);
    // ensure move construction works:
    Vector4F vec4_3(std::move(vec4_2));
    tz::assert::equal(vec4_3.x, dx);
    tz::assert::equal(vec4_3.y, dy);
    tz::assert::equal(vec4_3.z, dz);
    tz::assert::equal(vec4_3.w, dw);
    // ensure copy assignment works:
    vec4_3 = temp;
    tz::assert::equal(vec4_3.x, 1);
    tz::assert::equal(vec4_3.y, 2);
    tz::assert::equal(vec4_3.z, 3);
    tz::assert::equal(vec4_3.w, 4);
}

void test_swizzle_4D()
{
    Vector4F xyzw = {1, 2, 3, 4};
    Vector4F xyzw2 = xyzw.xyzw();
    tz::assert::equal(xyzw2.x, 1);
    tz::assert::equal(xyzw2.y, 2);
    tz::assert::equal(xyzw2.z, 3);
    tz::assert::equal(xyzw2.w, 4);
    Vector4F xywz = xyzw.xywz();
    tz::assert::equal(xywz.x, 1);
    tz::assert::equal(xywz.y, 2);
    tz::assert::equal(xywz.z, 4);
    tz::assert::equal(xywz.w, 3);
    Vector4F xzyw = xyzw.xzyw();
    tz::assert::equal(xzyw.x, 1);
    tz::assert::equal(xzyw.y, 3);
    tz::assert::equal(xzyw.z, 2);
    tz::assert::equal(xzyw.w, 4);
    Vector4F xzwy = xyzw.xzwy();
    tz::assert::equal(xzwy.x, 1);
    tz::assert::equal(xzwy.y, 3);
    tz::assert::equal(xzwy.z, 4);
    tz::assert::equal(xzwy.w, 2);
    Vector4F xwyz = xyzw.xwyz();
    tz::assert::equal(xwyz.x, 1);
    tz::assert::equal(xwyz.y, 4);
    tz::assert::equal(xwyz.z, 2);
    tz::assert::equal(xwyz.w, 3);
    Vector4F xwzy = xyzw.xwzy();
    tz::assert::equal(xwzy.x, 1);
    tz::assert::equal(xwzy.y, 4);
    tz::assert::equal(xwzy.z, 3);
    tz::assert::equal(xwzy.w, 2);
    Vector4F yxzw = xyzw.yxzw();
    tz::assert::equal(yxzw.x, 2);
    tz::assert::equal(yxzw.y, 1);
    tz::assert::equal(yxzw.z, 3);
    tz::assert::equal(yxzw.w, 4);
    Vector4F yxwz = xyzw.yxwz();
    tz::assert::equal(yxwz.x, 2);
    tz::assert::equal(yxwz.y, 1);
    tz::assert::equal(yxwz.z, 4);
    tz::assert::equal(yxwz.w, 3);
    Vector4F yzxw = xyzw.yzxw();
    tz::assert::equal(yzxw.x, 2);
    tz::assert::equal(yzxw.y, 3);
    tz::assert::equal(yzxw.z, 1);
    tz::assert::equal(yzxw.w, 4);
    Vector4F yzwx = xyzw.yzwx();
    tz::assert::equal(yzwx.x, 2);
    tz::assert::equal(yzwx.y, 3);
    tz::assert::equal(yzwx.z, 4);
    tz::assert::equal(yzwx.w, 1);
    Vector4F ywxz = xyzw.ywxz();
    tz::assert::equal(ywxz.x, 2);
    tz::assert::equal(ywxz.y, 4);
    tz::assert::equal(ywxz.z, 1);
    tz::assert::equal(ywxz.w, 3);
    Vector4F ywzx = xyzw.ywzx();
    tz::assert::equal(ywzx.x, 2);
    tz::assert::equal(ywzx.y, 4);
    tz::assert::equal(ywzx.z, 3);
    tz::assert::equal(ywzx.w, 1);
    Vector4F zxyw = xyzw.zxyw();
    tz::assert::equal(zxyw.x, 3);
    tz::assert::equal(zxyw.y, 1);
    tz::assert::equal(zxyw.z, 2);
    tz::assert::equal(zxyw.w, 4);
    Vector4F zxwy = xyzw.zxwy();
    tz::assert::equal(zxwy.x, 3);
    tz::assert::equal(zxwy.y, 1);
    tz::assert::equal(zxwy.z, 4);
    tz::assert::equal(zxwy.w, 2);
    Vector4F zyxw = xyzw.zyxw();
    tz::assert::equal(zyxw.x, 3);
    tz::assert::equal(zyxw.y, 2);
    tz::assert::equal(zyxw.z, 1);
    tz::assert::equal(zyxw.w, 4);
    Vector4F zywx = xyzw.zywx();
    tz::assert::equal(zywx.x, 3);
    tz::assert::equal(zywx.y, 2);
    tz::assert::equal(zywx.z, 4);
    tz::assert::equal(zywx.w, 1);
    Vector4F zwxy = xyzw.zwxy();
    tz::assert::equal(zwxy.x, 3);
    tz::assert::equal(zwxy.y, 4);
    tz::assert::equal(zwxy.z, 1);
    tz::assert::equal(zwxy.w, 2);
    Vector4F zwyx = xyzw.zwyx();
    tz::assert::equal(zwyx.x, 3);
    tz::assert::equal(zwyx.y, 4);
    tz::assert::equal(zwyx.z, 2);
    tz::assert::equal(zwyx.w, 1);
    Vector4F wxyz = xyzw.wxyz();
    tz::assert::equal(wxyz.x, 4);
    tz::assert::equal(wxyz.y, 1);
    tz::assert::equal(wxyz.z, 2);
    tz::assert::equal(wxyz.w, 3);
    Vector4F wxzy = xyzw.wxzy();
    tz::assert::equal(wxzy.x, 4);
    tz::assert::equal(wxzy.y, 1);
    tz::assert::equal(wxzy.z, 3);
    tz::assert::equal(wxzy.w, 2);
    Vector4F wyxz = xyzw.wyxz();
    tz::assert::equal(wyxz.x, 4);
    tz::assert::equal(wyxz.y, 2);
    tz::assert::equal(wyxz.z, 1);
    tz::assert::equal(wyxz.w, 3);
    Vector4F wyzx = xyzw.wyzx();
    tz::assert::equal(wyzx.x, 4);
    tz::assert::equal(wyzx.y, 2);
    tz::assert::equal(wyzx.z, 3);
    tz::assert::equal(wyzx.w, 1);
    Vector4F wzxy = xyzw.wzxy();
    tz::assert::equal(wzxy.x, 4);
    tz::assert::equal(wzxy.y, 3);
    tz::assert::equal(wzxy.z, 1);
    tz::assert::equal(wzxy.w, 2);
    Vector4F wzyx = xyzw.wzyx();
    tz::assert::equal(wzyx.x, 4);
    tz::assert::equal(wzyx.y, 3);
    tz::assert::equal(wzyx.z, 2);
    tz::assert::equal(wzyx.w, 1);
}

void test_methods_4D()
{
    Vector4F a{1, 2, 3, 4};
    // test Vector2F::normalised();
    Vector4F normal = a.normalised();
    tz::assert::equal(std::round(normal.length()), 1);
    // length [1, 2, 3, 4] = 1 + 4 + 9 + 16 = 30
    float inverse_sqrt30 = 1.0f / std::sqrt(30.0f);
    // test Vector2F::length()
    tz::assert::equal(normal.x, inverse_sqrt30);
    tz::assert::equal(normal.y, 2.0f * inverse_sqrt30);
    tz::assert::equal(std::floor(normal.z), std::floor(3.0f * inverse_sqrt30));
    tz::assert::equal(normal.w, 4.0f * inverse_sqrt30);
    tz::assert::equal(a.length(), std::sqrt(30.0f));

    // [1, 2, 3, 4] . [4, 3, 2, 1] = 4 + 6 + 6 + 4 = 20
    tz::assert::equal(a.dot(Vector4F{4, 3, 2, 1}), 20.0f);
    // operators.
    tz::assert::equal((a + Vector4F{1, 0, -1, -2}), Vector4F{2, 2, 2, 2});
    tz::assert::equal((a - Vector4F{1, 2, 3, 4}), Vector4F{});
    tz::assert::equal((a * 2.0f), Vector4F{2, 4, 6, 8});
    tz::assert::equal((a / 2.0f), Vector4F{0.5f, 1.0f, 1.5f, 2.0f});
    tz::assert::that(a < Vector4F{2, 3, 4, 5});
    tz::assert::that(a > Vector4F{0, 1, 2, 3});
    tz::assert::that((a <= Vector4F{1, 2, 3, 4}) && (a >= Vector4F{1, 2, 3, 4}));
}

void test()
{
    // test base vector class.
    Vector<10, int> vector10i{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    std::array<int, 10> vector_data = vector10i.data();
    for (int x : vector_data)
        tz::assert::equal(x, 0);
    // test the default subclasses Vector2F, Vector3F and Vector4F and their swizzles.
    test_initialisation_2D();
    test_swizzle_2D();
    test_methods_2D();
    test_initialisation_3D();
    test_swizzle_3D();
    test_methods_3D();
    test_initialisation_4D();
    test_swizzle_4D();
    test_methods_4D();
}