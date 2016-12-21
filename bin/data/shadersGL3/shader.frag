#version 410

flat in vec4 hexColor;
//in vec4 hexColor;
uniform sampler2D uTexture;
out vec4 out_color;
// inputs
in vec2 vTexCoord;

void main()
{
    out_color = vec4( hexColor.rgb, 1.0 );
//    out_color = texelFetch(uTexture,vTexCoord,0);
//    out_color = texelFetch(uTexture, ivec2(0.3,0.3),0);
//    out_color = texture(uTexture, ivec2(0.3,0.3));
    //    out_color = texture2D(uTexture, vTexCoord) + 0.25;
        out_color = texture(uTexture, vTexCoord);
}
