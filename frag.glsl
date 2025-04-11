#version 450 core

in vec3 OutColor;
in vec2 TexCoord;
uniform sampler2D CardTex;
uniform sampler2D FontTex;

out vec4 FragColor;

void main()
{
   // FragColor = texture(CardTex, TexCoord) * vec4(OutColor, 1.0);
   FragColor = texture(FontTex, TexCoord) * vec4(OutColor, 1.0);
}

