#version 460 core
#ifdef vertex
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec4 a_Border;

layout(push_constant) uniform PushConstants{
	mat4 ViewProjection;
};

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoords;
layout(location = 2) out vec4 v_Border;

void main(){
	v_Color = a_Color;
	v_TexCoords = a_TexCoords.xy;
	v_Border = a_Border;

	gl_Position = ViewProjection * vec4(a_Position, 1.0);
	gl_Position.y *= -1.0;
}
#endif

#ifdef fragment

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoords;
layout(location = 2) in vec4 v_Border;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 0) uniform texture2D u_FontAtlas;
layout(set = 0, binding = 1) uniform sampler u_Sampler;

layout(push_constant) uniform PushConstants{
	mat4 ViewProjection;
};

const float pxRange = 2.0;

const vec4 bgColor = vec4(0.0);

float screenPxRange() {
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(sampler2D(u_FontAtlas, u_Sampler), 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(v_TexCoords);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float linearstep(float lo, float hi, float x)
{
	return (clamp(x, lo, hi) - lo) / (hi - lo);
}

float sampleSDF(vec2 coords){
	vec3 msd = texture(sampler2D(u_FontAtlas,u_Sampler), coords).rgb;
    float sd = median(msd.r, msd.g, msd.b);
	return sd - 0.5;
}

#define TYPE 0

#if TYPE == 0

const float glyphEdge = 0.5;

float contour(float dist, float edge, float width) {
  return clamp(smoothstep(edge - width, edge + width, dist), 0.0, 1.0);
}

void main() {
  vec4 tex = texture(sampler2D(u_FontAtlas,u_Sampler), v_TexCoords);
  float dist  = median(tex.r,tex.g,tex.b);
  float width = fwidth(dist);
  vec4 textColor = clamp(v_Color, 0.0, 1.0);
  float outerEdge = glyphEdge;

   float alpha = contour(dist, outerEdge, width);
   o_Color = vec4(textColor.rgb, textColor.a * alpha);


   outerEdge = outerEdge - v_Border.a;

   float outlineOuterAlpha = clamp(smoothstep(outerEdge - width, outerEdge + width, dist), 0.0, 1.0);
   float outlineAlpha = outlineOuterAlpha - alpha;

   o_Color.rgb = mix(v_Border.rgb, o_Color.rgb, alpha);
   o_Color.a = max(o_Color.a, 1.0 * outlineOuterAlpha);
   if(o_Color.a == 0.0)
	discard;
}

#elif TYPE == 1
void main() {
	
	const float thickness = 0.0;

	vec3 borderColor = v_Border.rgb;
	float border = v_Border.a;

	vec3 msd = texture(sampler2D(u_FontAtlas, u_Sampler), v_TexCoords).rgb;

	vec2 msdfSize = vec2(textureSize(sampler2D(u_FontAtlas, u_Sampler), 0));//vec2(textureSize(MSDF, 0));
    float pxSize = min(0.5/pxRange*(fwidth(v_TexCoords.x)*msdfSize.x+fwidth(v_TexCoords.y)*msdfSize.y), 0.25);

    float sd = 2.0*median(msd.r, msd.g, msd.b)-1.0 + thickness;
    float inside = linearstep(-border-pxSize, -border+pxSize, sd);
    float outsideBorder = border > 0.0 
        ? linearstep(+border-pxSize, +border+pxSize, sd) 
        : 1.0;

    vec4 fg = vec4(mix(borderColor, v_Color.rgb,
                       outsideBorder), 
                   inside);

//    msd = texture(iChannel0, cuv - shadowVector).rgb;
//    sd = 2.0*median(msd.r, msd.g, msd.b)-1.0 + border+thickness;
//    float shadow = shadowOpacity*linearstep(-shadowSoftness-pxSize,
//                                            +shadowSoftness+pxSize, sd);
//
	o_Color = vec4(mix(vec3(0.0), fg.rgb, fg.a), 
	                 1.0);
    //o_Color = mix(fragColor, shadowColor, (1.0 - inside));
}

#elif TYPE == 2

void main() {
	
	const float smoothness = 0.10;

	float sd  = sampleSDF(v_TexCoords);

	float opacity = smoothstep(-smoothness,smoothness,sd);

	if(opacity == 0.0)
		discard;

	o_Color = mix(vec4(0.0),v_Color,opacity);

	if(o_Color.a == 0.0)
		discard;
}

#elif TYPE == 3
	
#endif




/*

// Simplified SDF shader:
// - No Shading Option (bevel / bump / env map)
// - No Glow Option
// - Softness is applied on both side of the outline

Shader "TextMeshPro/Mobile/Distance Field" {

Properties {
	[HDR]_FaceColor     ("Face Color", Color) = (1,1,1,1)
	_FaceDilate			("Face Dilate", Range(-1,1)) = 0

	[HDR]_OutlineColor	("Outline Color", Color) = (0,0,0,1)
	_OutlineWidth		("Outline Thickness", Range(0,1)) = 0
	_OutlineSoftness	("Outline Softness", Range(0,1)) = 0

	_WeightNormal		("Weight Normal", float) = 0
	_WeightBold			("Weight Bold", float) = .5

	_ShaderFlags		("Flags", float) = 0
	_ScaleRatioA		("Scale RatioA", float) = 1
	_ScaleRatioB		("Scale RatioB", float) = 1
	_ScaleRatioC		("Scale RatioC", float) = 1

	_MainTex			("Font Atlas", 2D) = "white" {}
	_TextureWidth		("Texture Width", float) = 512
	_TextureHeight		("Texture Height", float) = 512
	_GradientScale		("Gradient Scale", float) = 5
	_ScaleX				("Scale X", float) = 1
	_ScaleY				("Scale Y", float) = 1
	_PerspectiveFilter	("Perspective Correction", Range(0, 1)) = 0.875
	_Sharpness			("Sharpness", Range(-1,1)) = 0

	_VertexOffsetX		("Vertex OffsetX", float) = 0
	_VertexOffsetY		("Vertex OffsetY", float) = 0

	_ClipRect			("Clip Rect", vector) = (-32767, -32767, 32767, 32767)
	_MaskSoftnessX		("Mask SoftnessX", float) = 0
	_MaskSoftnessY		("Mask SoftnessY", float) = 0

	_StencilComp		("Stencil Comparison", Float) = 8
	_Stencil			("Stencil ID", Float) = 0
	_StencilOp			("Stencil Operation", Float) = 0
	_StencilWriteMask	("Stencil Write Mask", Float) = 255
	_StencilReadMask	("Stencil Read Mask", Float) = 255

	_CullMode			("Cull Mode", Float) = 0
	_ColorMask			("Color Mask", Float) = 15
}

SubShader {
	Pass {
		CGPROGRAM
		#pragma vertex VertShader
		#pragma fragment PixShader
		#pragma shader_feature __ OUTLINE_ON

		#pragma multi_compile __ UNITY_UI_CLIP_RECT
		#pragma multi_compile __ UNITY_UI_ALPHACLIP

		#include "UnityCG.cginc"
		#include "UnityUI.cginc"
		#include "TMPro_Properties.cginc"

		struct vertex_t {
			UNITY_VERTEX_INPUT_INSTANCE_ID
			float4	vertex			: POSITION;
			float3	normal			: NORMAL;
			fixed4	color			: COLOR;
			float2	texcoord0		: TEXCOORD0;
			float2	texcoord1		: TEXCOORD1;
		};

		struct pixel_t {
			UNITY_VERTEX_INPUT_INSTANCE_ID
			UNITY_VERTEX_OUTPUT_STEREO
			float4	vertex			: SV_POSITION;
			fixed4	faceColor		: COLOR;
			fixed4	outlineColor	: COLOR1;
			float4	texcoord0		: TEXCOORD0;			// Texture UV, Mask UV
			half4	param			: TEXCOORD1;			// Scale(x), BiasIn(y), BiasOut(z), Bias(w)
			half4	mask			: TEXCOORD2;			// Position in clip space(xy), Softness(zw)
		};


		pixel_t VertShader(vertex_t input)
		{
			pixel_t output;

			UNITY_INITIALIZE_OUTPUT(pixel_t, output);
			UNITY_SETUP_INSTANCE_ID(input);
			UNITY_TRANSFER_INSTANCE_ID(input, output);
			UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(output);

			float bold = step(input.texcoord1.y, 0);

			float4 vert = input.vertex;
			vert.x += _VertexOffsetX;
			vert.y += _VertexOffsetY;
			float4 vPosition = UnityObjectToClipPos(vert);

			float2 pixelSize = vPosition.w;
			pixelSize /= float2(_ScaleX, _ScaleY) * abs(mul((float2x2)UNITY_MATRIX_P, _ScreenParams.xy));

			float scale = rsqrt(dot(pixelSize, pixelSize));
			scale *= abs(input.texcoord1.y) * _GradientScale * (_Sharpness + 1);
			if(UNITY_MATRIX_P[3][3] == 0) scale = lerp(abs(scale) * (1 - _PerspectiveFilter), scale, abs(dot(UnityObjectToWorldNormal(input.normal.xyz), normalize(WorldSpaceViewDir(vert)))));

			float weight = lerp(_WeightNormal, _WeightBold, bold) / 4.0;
			weight = (weight + _FaceDilate) * _ScaleRatioA * 0.5;

			float layerScale = scale;

			scale /= 1 + (_OutlineSoftness * _ScaleRatioA * scale);
			float bias = (0.5 - weight) * scale - 0.5;
			float outline = _OutlineWidth * _ScaleRatioA * 0.5 * scale;

			float opacity = input.color.a;

			fixed4 faceColor = fixed4(input.color.rgb, opacity) * _FaceColor;
			faceColor.rgb *= faceColor.a;

			fixed4 outlineColor = _OutlineColor;
			outlineColor.a *= opacity;
			outlineColor.rgb *= outlineColor.a;
			outlineColor = lerp(faceColor, outlineColor, sqrt(min(1.0, (outline * 2))));

			// Generate UV for the Masking Texture
			float4 clampedRect = clamp(_ClipRect, -2e10, 2e10);
			float2 maskUV = (vert.xy - clampedRect.xy) / (clampedRect.zw - clampedRect.xy);

			// Populate structure for pixel shader
			output.vertex = vPosition;
			output.faceColor = faceColor;
			output.outlineColor = outlineColor;
			output.texcoord0 = float4(input.texcoord0.x, input.texcoord0.y, maskUV.x, maskUV.y);
			output.param = half4(scale, bias - outline, bias + outline, bias);
			output.mask = half4(vert.xy * 2 - clampedRect.xy - clampedRect.zw, 0.25 / (0.25 * half2(_MaskSoftnessX, _MaskSoftnessY) + pixelSize.xy));

			return output;
		}


		// PIXEL SHADER
		fixed4 PixShader(pixel_t input) : SV_Target
		{
			UNITY_SETUP_INSTANCE_ID(input);

			half d = tex2D(_MainTex, input.texcoord0.xy).a * input.param.x;
			half4 c = input.faceColor * saturate(d - input.param.w);

			#ifdef OUTLINE_ON
			c = lerp(input.outlineColor, input.faceColor, saturate(d - input.param.z));
			c *= saturate(d - input.param.y);
			#endif

			// Alternative implementation to UnityGet2DClipping with support for softness.
			#if UNITY_UI_CLIP_RECT
			half2 m = saturate((_ClipRect.zw - _ClipRect.xy - abs(input.mask.xy)) * input.mask.zw);
			c *= m.x * m.y;
			#endif

			#if UNITY_UI_ALPHACLIP
			clip(c.a - 0.001);
			#endif

			return c;
		}
		ENDCG
	}
}

CustomEditor "TMPro.EditorUtilities.TMP_SDFShaderGUI"
}


*/
#endif
