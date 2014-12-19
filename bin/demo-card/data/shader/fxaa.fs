#version 330

//----------------------------------------------
// NVIDIA FXAA 3.9 by TIMOTHY LOTTES
//----------------------------------------------

// Requires "#version 130" or better
#define FXAA_GLSL_130

/*--------------------------------------------------------------------------*/
#ifndef FXAA_DISCARD
	// 
	// Only valid for PC OpenGL currently.
	// 
	// 1 = Use discard on pixels which don't need AA.
	//     For APIs which enable concurrent TEX+ROP from same surface.
	// 0 = Return unchanged color on pixels which don't need AA.
	// 
	#define FXAA_DISCARD 1
#endif    

/*============================================================================
                         FXAA QUALITY - TUNING KNOBS
============================================================================*/
#ifndef FXAA_QUALITY__EDGE_THRESHOLD
	//
	// The minimum amount of local contrast required to apply algorithm.
	//
	// 1/3 - too little
	// 1/4 - low quality
	// 1/6 - default
	// 1/8 - high quality
	// 1/16 - overkill
	//
	#define FXAA_QUALITY__EDGE_THRESHOLD (1.0/6.0)
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_QUALITY__EDGE_THRESHOLD_MIN
	//
	// Trims the algorithm from processing darks.
	//
	// 1/32 - visible limit
	// 1/16 - high quality
	// 1/12 - upper limit (default, the start of visible unfiltered edges)
	//
	#define FXAA_QUALITY__EDGE_THRESHOLD_MIN (1.0/12.0)
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_QUALITY__SUBPIX
	//
	// Choose the amount of sub-pixel aliasing removal.
	//
	// 1   - upper limit (softer)
	// 3/4 - default amount of filtering
	// 1/2 - lower limit (sharper, less sub-pixel aliasing removal)
	//
	//
	#define FXAA_QUALITY__SUBPIX (3.0/4.0)
#endif

/*--------------------------------------------------------------------------*/

const vec3 LumaFactor = vec3(0.299, 0.587, 0.114); 

#ifdef FXAA_GLSL_130
	// Requires "#version 130" or better
	#define half float
	#define half2 vec2
	#define half3 vec3
	#define half4 vec4
	#define int2 ivec2
	#define float2 vec2
	#define float3 vec3
	#define float4 vec4
	#define FxaaInt2 ivec2
	#define FxaaFloat2 vec2
	#define FxaaFloat3 vec3
	#define FxaaFloat4 vec4
	#define FxaaDiscard discard
	#define FxaaDot3(a, b) dot(a, b)
	#define FxaaSat(x) clamp(x, 0.0, 1.0)
	#define FxaaLerp(x,y,s) mix(x,y,s)
	#define FxaaTex sampler2D
	#define FxaaTexTop(t, p) textureLod(t, p, 0.0)
	#define FxaaTexOff(t, p, o, r) textureLodOffset(t, p, 0.0, o)
	#define FxaaLuma(c) dot(c.xyz,LumaFactor)
#endif

float4 FxaaPixelShader(
	// {xy} = center of pixel
	float2 pos,
	// {xyzw} = not used on FXAA3 Quality
	float4 posPos,       
	// {rgb_} = color in linear or perceptual color space
	// {___a} = luma in perceptual color space (not linear) 
	FxaaTex tex,
	// This must be from a constant/uniform.
	// {x_} = 1.0/screenWidthInPixels
	// {_y} = 1.0/screenHeightInPixels
	float2 rcpFrame,
	// {xyzw} = not used on FXAA3 Quality
	float4 rcpFrameOpt 
) {   
/*--------------------------------------------------------------------------*/
	float2 posM;
	posM.x = pos.x;
	posM.y = pos.y;

	float4 rgbyM = FxaaTexTop(tex, posM);
	float lumaM = FxaaLuma(rgbyM);
	float lumaS = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0, 1), rcpFrame.xy));
	float lumaE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 0), rcpFrame.xy));
	float lumaN = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0,-1), rcpFrame.xy));
	float lumaW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 0), rcpFrame.xy));
/*--------------------------------------------------------------------------*/
	float maxSM = max(lumaS, lumaM);
	float minSM = min(lumaS, lumaM);
	float maxESM = max(lumaE, maxSM); 
	float minESM = min(lumaE, minSM); 
	float maxWN = max(lumaN, lumaW);
	float minWN = min(lumaN, lumaW);
	float rangeMax = max(maxWN, maxESM);
	float rangeMin = min(minWN, minESM);
	float rangeMaxScaled = rangeMax * FXAA_QUALITY__EDGE_THRESHOLD;
	float range = rangeMax - rangeMin;
	float rangeMaxClamped = max(FXAA_QUALITY__EDGE_THRESHOLD_MIN, rangeMaxScaled);
/*--------------------------------------------------------------------------*/
	if(range < rangeMaxClamped) {
		rgbyM.w = 1.0;
		return rgbyM;
	}
/*--------------------------------------------------------------------------*/
	float lumaNW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1,-1), rcpFrame.xy));
	float lumaSE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 1), rcpFrame.xy));
	float lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1,-1), rcpFrame.xy));
	float lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1), rcpFrame.xy));
/*--------------------------------------------------------------------------*/
	float lumaNS = lumaN + lumaS;
	float lumaWE = lumaW + lumaE;
	float subpixRcpRange = 1.0/range;
	float subpixNSWE = lumaNS + lumaWE;
	float edgeHorz1 = (-2.0 * lumaM) + lumaNS;
	float edgeVert1 = (-2.0 * lumaM) + lumaWE;
/*--------------------------------------------------------------------------*/
	float lumaNESE = lumaNE + lumaSE;
	float lumaNWNE = lumaNW + lumaNE;
	float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
	float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;
/*--------------------------------------------------------------------------*/
	float lumaNWSW = lumaNW + lumaSW;
	float lumaSWSE = lumaSW + lumaSE;
	float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
	float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
	float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
	float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;
	float edgeHorz = abs(edgeHorz3) + edgeHorz4;
	float edgeVert = abs(edgeVert3) + edgeVert4;
/*--------------------------------------------------------------------------*/
	float subpixNWSWNESE = lumaNWSW + lumaNESE; 
	float lengthSign = rcpFrame.x;
	bool horzSpan = edgeHorz >= edgeVert;
	float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE; 
/*--------------------------------------------------------------------------*/
	if(!horzSpan) lumaN = lumaW; 
	if(!horzSpan) lumaS = lumaE;
	if(horzSpan) lengthSign = rcpFrame.y;
	float subpixB = (subpixA * (1.0/12.0)) - lumaM;
/*--------------------------------------------------------------------------*/
	float gradientN = lumaN - lumaM;
	float gradientS = lumaS - lumaM;
	float lumaNN = lumaN + lumaM;
	float lumaSS = lumaS + lumaM;
	bool pairN = abs(gradientN) >= abs(gradientS);
	float gradient = max(abs(gradientN), abs(gradientS));
	if(pairN) lengthSign = -lengthSign;
	float subpixC = FxaaSat(abs(subpixB) * subpixRcpRange);
/*--------------------------------------------------------------------------*/
	float2 posB;
	posB.x = posM.x;
	posB.y = posM.y;
	float2 offNP;
	offNP.x = (!horzSpan) ? 0.0 : rcpFrame.x;
	offNP.y = ( horzSpan) ? 0.0 : rcpFrame.y;
	if(!horzSpan) posB.x += lengthSign * 0.5;
	if( horzSpan) posB.y += lengthSign * 0.5;
/*--------------------------------------------------------------------------*/
	float2 posN;
	posN.x = posB.x - offNP.x;
	posN.y = posB.y - offNP.y;
	float2 posP;
	posP.x = posB.x + offNP.x;
	posP.y = posB.y + offNP.y;
	float subpixD = ((-2.0)*subpixC) + 3.0;
	float lumaEndN = FxaaLuma(FxaaTexTop(tex, posN));
	float subpixE = subpixC * subpixC;
	float lumaEndP = FxaaLuma(FxaaTexTop(tex, posP));
/*--------------------------------------------------------------------------*/
	if(!pairN) lumaNN = lumaSS;
	float gradientScaled = gradient * 1.0/4.0;
	float lumaMM = lumaM - lumaNN * 0.5;
	float subpixF = subpixD * subpixE;
	bool lumaMLTZero = lumaMM < 0.0;
/*--------------------------------------------------------------------------*/
	lumaEndN -= lumaNN * 0.5;
	lumaEndP -= lumaNN * 0.5;
	bool doneN = abs(lumaEndN) >= gradientScaled;
	bool doneP = abs(lumaEndP) >= gradientScaled;
	if(!doneN) posN.x -= offNP.x * 1.5;
	if(!doneN) posN.y -= offNP.y * 1.5;
	bool doneNP = (!doneN) || (!doneP);
	if(!doneP) posP.x += offNP.x * 1.5;
	if(!doneP) posP.y += offNP.y * 1.5;
	if(doneNP) {
/*--------------------------------------------------------------------------*/
		if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
		if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * 2.0;
		if(!doneN) posN.y -= offNP.y * 2.0;
		doneNP = (!doneN) || (!doneP);
		if(!doneP) posP.x += offNP.x * 2.0;
		if(!doneP) posP.y += offNP.y * 2.0;
		if(doneNP) {
/*--------------------------------------------------------------------------*/
			if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
			if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
			if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
			if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
			doneN = abs(lumaEndN) >= gradientScaled;
			doneP = abs(lumaEndP) >= gradientScaled;
			if(!doneN) posN.x -= offNP.x * 2.0;
			if(!doneN) posN.y -= offNP.y * 2.0;
			doneNP = (!doneN) || (!doneP);
			if(!doneP) posP.x += offNP.x * 2.0;
			if(!doneP) posP.y += offNP.y * 2.0;
			if(doneNP) {
/*--------------------------------------------------------------------------*/
				if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
				if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
				if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
				if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
				doneN = abs(lumaEndN) >= gradientScaled;
				doneP = abs(lumaEndP) >= gradientScaled;
				if(!doneN) posN.x -= offNP.x * 4.0;
				if(!doneN) posN.y -= offNP.y * 4.0;
				doneNP = (!doneN) || (!doneP);
				if(!doneP) posP.x += offNP.x * 4.0;
				if(!doneP) posP.y += offNP.y * 4.0;
				if(doneNP) {
/*--------------------------------------------------------------------------*/
					if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
					if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
					if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
					if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
					doneN = abs(lumaEndN) >= gradientScaled;
					doneP = abs(lumaEndP) >= gradientScaled;
					if(!doneN) posN.x -= offNP.x * 2.0;
					if(!doneN) posN.y -= offNP.y * 2.0;
					if(!doneP) posP.x += offNP.x * 2.0; 
					if(!doneP) posP.y += offNP.y * 2.0; } } } }
/*--------------------------------------------------------------------------*/
	float dstN = posM.x - posN.x;
	float dstP = posP.x - posM.x;
	if(!horzSpan) dstN = posM.y - posN.y;
	if(!horzSpan) dstP = posP.y - posM.y;
/*--------------------------------------------------------------------------*/
	bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
	float spanLength = (dstP + dstN);
	bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
	float spanLengthRcp = 1.0/spanLength;
/*--------------------------------------------------------------------------*/
	bool directionN = dstN < dstP;
	float dst = min(dstN, dstP);
	bool goodSpan = directionN ? goodSpanN : goodSpanP;
	float subpixG = subpixF * subpixF;
	float pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
	float subpixH = subpixG * FXAA_QUALITY__SUBPIX;
/*--------------------------------------------------------------------------*/
	float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
	float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
	if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
	if( horzSpan) posM.y += pixelOffsetSubpix * lengthSign;
	rgbyM = FxaaTexTop(tex, posM); 
	rgbyM.w = 1;
	return rgbyM;
}


// screen texture
uniform sampler2D screen_sample;
// {x_} = 1.0/screenWidthInPixels
// {_y} = 1.0/screenHeightInPixels
uniform vec2 screen_size;

in vec2 f_texcoord;

void main()
{
	vec4 dummy = vec4(0,0,0,0);
	gl_FragColor = FxaaPixelShader(f_texcoord,dummy,screen_sample,screen_size,dummy);
}

