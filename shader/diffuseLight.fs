#version 300 es

precision mediump float;
out mediump vec4 fragColor;
in mediump vec3 t_color;

in vec3 Normal;
in vec3 myPos;

void main()
{
    vec3 lightPos = vec3(3.,1.,3.);
    vec3 lightColor = vec3(1.,1.,1.);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - myPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 ambient = vec3(0.3,0.3,0.3);
    vec3 result = (ambient + diffuse) * t_color;

    fragColor = vec4(result, 1.0);
}

