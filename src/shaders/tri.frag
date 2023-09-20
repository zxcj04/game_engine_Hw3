#version 410 core

out vec4 FragColor;

in vec3 frag_pos;
in vec3 normal;
in vec4 color;
in vec2 tex_coord;

uniform vec3 view_pos;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform bool enable_light;
uniform int culling;

uniform vec3 eye_direction;

void main()
{
    if(!enable_light)
    {
        FragColor = color;

        return;
    }

    vec3 light_pos = view_pos;

    vec3 norm = normalize(normal);
    vec3 light_direction = normalize(light_pos - frag_pos);
    vec3 view_direction = normalize(view_pos - frag_pos);

    // if (dot(norm, view_direction) < 0)
	// {
    //     norm = -norm;
    // }

    vec3 reflect_direction = reflect(-light_direction, norm);

    float ambient_strength = 0.2;
    vec3 ambient = ambient_strength * light_color;

    float diff = max(dot(norm, light_direction), 0.0);
    vec3 diffuse = diff * light_color;

    float specular_strength = 0.5;
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color;

    float cutoff = cos(radians(5.0f));
    float outer_cutoff = cos(radians(50.0f));
    float epsilon = (cutoff - outer_cutoff);

    float theta = dot(light_direction, -eye_direction);
    float intensity = clamp((theta - outer_cutoff) / epsilon, 0.0, 1.0);

    vec4 _color = color;

    switch(culling)
    {
        case -1:
            break;
        case 0:
            _color = vec4(1.0, 0.0, 0.0, 1.0);
            break;
        case 1:
            _color = vec4(0.0, 1.0, 0.0, 1.0);
            break;
        case 2:
            _color = vec4(0.0, 0.0, 1.0, 1.0);
            break;
    }

    vec4 result;

    if(culling == -1)
        result.rgb = (ambient + diffuse * intensity + specular * intensity) * _color.rgb;
    else
        result.rgb = (ambient + diffuse + specular) * _color.rgb;

    result.a = _color.a;

    // // if(theta > cutoff)
    // //     result = (ambient + diffuse + specular) * color;
    // // else
    // //     result = ambient * color;

    FragColor = result;
}