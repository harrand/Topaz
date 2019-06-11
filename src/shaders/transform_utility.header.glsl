vec3 transform3f(vec3 vec, mat4 mat, bool homogeneous)
{
    if(homogeneous)
        return (mat * vec4(vec, 1.0)).xyz;
    else
        return (mat * vec4(vec, 0.0)).xyz;
}