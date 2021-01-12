#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform vec3 color;

// texture sampler
uniform sampler2D texture1;

void main()
{    
    FragColor = texture(texture1, TexCoord);
    FragColor *= vec4(color, 1.0);
    // FragColor = vec4(1, 1,1,0);
}