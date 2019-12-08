const vec4 assert_failure_colour = vec4(1.0, 0.0, 1.0, 1.0);
#static_print("Shader runtime assert failure fragment colour = vec4(1.0, 0.0, 1.0, 1.0)")

vec4 assert4(bool expression, vec4 colour)
{
    if(expression)
    {
        return colour;
    }
    else
    {
        return assert_failure_colour;
    }
}

vec3 assert3(bool expression, vec3 colour)
{
    if(expression)
    {
        return colour;
    }
    else
    {
        return assert_failure_colour.xyz;
    }
}