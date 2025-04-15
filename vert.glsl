#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in float aTexId;

uniform mat4 u_MVP;

out vec3 OutColor;
out vec2 TexCoord;
out float TexId;
void main()
{
   gl_Position = u_MVP*vec4(aPos, 1.0);
   OutColor = aColor;
   TexCoord = vec2(aTexCoord);
   TexId = aTexId;
}

