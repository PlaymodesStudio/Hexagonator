#version 400

in vec4 hexColor;
out vec4 out_color;

void main()
{
    out_color = vec4( hexColor.rgb, 1.0 );
}
