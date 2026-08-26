#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D mainTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

uniform vec2 texelSize;
uniform float outlineThickness;
uniform float normalThreshold;
uniform float depthThreshold;
uniform vec4 outlineColor;
uniform int outputMode;

float GetLinearDepth(vec2 uv) {
    float z = texture(depthTexture, uv).r * 2.0 - 1.0;
    return (2.0 * 0.1 * 100.0) / (100.0 + 0.1 - z * (100.0 - 0.1));
}

float SobelDepth(vec2 uv, vec2 offset) {
    float dTL = GetLinearDepth(uv + vec2(-offset.x,  offset.y));
    float dT  = GetLinearDepth(uv + vec2( 0.0,       offset.y));
    float dTR = GetLinearDepth(uv + vec2( offset.x,  offset.y));
    float dL  = GetLinearDepth(uv + vec2(-offset.x,  0.0));
    float dR  = GetLinearDepth(uv + vec2( offset.x,  0.0));
    float dBL = GetLinearDepth(uv + vec2(-offset.x, -offset.y));
    float dB  = GetLinearDepth(uv + vec2( 0.0,      -offset.y));
    float dBR = GetLinearDepth(uv + vec2( offset.x, -offset.y));

    float edgeX = dTL + 2.0 * dL + dBL - dTR - 2.0 * dR - dBR;
    float edgeY = dTL + 2.0 * dT + dTR - dBL - 2.0 * dB - dBR;
    
    return sqrt(edgeX * edgeX + edgeY * edgeY);
}

float NormalEdge(vec2 uv, vec2 offset) {
    vec3 centerNormal = texture(normalTexture, uv).rgb;
    
    vec3 nTL = texture(normalTexture, uv + vec2(-offset.x,  offset.y)).rgb;
    vec3 nTR = texture(normalTexture, uv + vec2( offset.x,  offset.y)).rgb;
    vec3 nBL = texture(normalTexture, uv + vec2(-offset.x, -offset.y)).rgb;
    vec3 nBR = texture(normalTexture, uv + vec2( offset.x, -offset.y)).rgb;

    float d1 = dot(centerNormal, nTL);
    float d2 = dot(centerNormal, nTR);
    float d3 = dot(centerNormal, nBL);
    float d4 = dot(centerNormal, nBR);

    return (1.0 - d1) + (1.0 - d2) + (1.0 - d3) + (1.0 - d4);
}

void main() {
    vec4 screenColor = texture(mainTexture, TexCoords);
    float rawDepth = texture(depthTexture, TexCoords).r;

    if (outputMode == 2) {
        FragColor = vec4(texture(normalTexture, TexCoords).rgb, 1.0);
        return;
    }
    
    if (outputMode == 3) {
        float linearZ = GetLinearDepth(TexCoords) / 50.0; 
        FragColor = vec4(vec3(linearZ), 1.0);
        return;
    }

    if (rawDepth >= 0.999) {
        if (outputMode == 1) FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        else FragColor = screenColor;
        return;
    }

    float linearZ = GetLinearDepth(TexCoords);
    vec2 offset = texelSize * outlineThickness;
    
    float dynamicDepthThreshold = depthThreshold * linearZ * 0.5;
    
    float maskD = step(dynamicDepthThreshold, SobelDepth(TexCoords, offset));
    float maskN = step(normalThreshold, NormalEdge(TexCoords, offset));
    float edgeMask = max(maskD, maskN);

    if (outputMode == 1) {
        FragColor = vec4(outlineColor.rgb * edgeMask, 1.0);
    } else {
        FragColor = mix(screenColor, outlineColor, edgeMask);
    }
}