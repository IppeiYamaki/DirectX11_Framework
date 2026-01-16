#include "Common/Common.hlsl"

void main(in VS_IN In, out PS_IN Out)
{
    Out.Position = TransformPosition(In.Position);
    Out.TexCoord = In.TexCoord;

    Out.Color = In.Color * g_material.BaseColor;
    Out.NormalWS = TransformNormalWS(In.Normal);
}
