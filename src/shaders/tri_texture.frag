#version 410 core

out vec4 FragColor;

in vec3 frag_pos;
in vec3 normal;
in vec3 color;
in vec2 tex_coord;

uniform vec3 view_pos;
uniform vec3 light_pos;
uniform vec3 light_color;

uniform vec3 eye_direction;

uniform sampler2D using_texture;

void main()
{
    vec4 texture_color = texture(using_texture, tex_coord);

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

    vec3 result;

    result = (ambient + diffuse * intensity + specular * intensity) * vec3(texture_color);

    // if(theta > cutoff)
    //     result = (ambient + diffuse + specular) * color;
    // else
    //     result = ambient * color;

    FragColor = vec4(result, 1.0);
}