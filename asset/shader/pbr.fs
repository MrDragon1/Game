#version 460 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out int EntityID;

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

// IBL
layout(binding = 0) uniform sampler2D uBRDFLUT;
layout(binding = 1) uniform samplerCube uIrradianceMap;
layout(binding = 2) uniform samplerCube uPrefilterMap;

layout(binding = 3) uniform samplerCube uShadowMap;

layout(binding = 4) uniform sampler2D uAlbedoMap;
layout(binding = 5) uniform sampler2D uNormalMap;
layout(binding = 6) uniform sampler2D uRoughnessMap;

// Material
uniform vec3 uAlbedo;
uniform float uMetallic;
uniform float uRoughness;
uniform float uEmission;

uniform bool uUseAlbedo;
uniform bool uUseRoughness;
uniform bool uUseNormal;
// Lights
// uniform vec3 lightPositions[4];
// uniform vec3 lightColors[4];

// Shadow
uniform vec3 uLightPos;
uniform float uFarPlane;

uniform vec3 uCamPos;
uniform int uEntityID;

const float PI = 3.14159265359;

struct PBRParameters
{
    vec3 Albedo;
    float Roughness;
    float Metalness;

    vec3 Normal;
    vec3 View;
    float NdotV;
} m_Params;

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float NdfGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float GaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float GaSchlickGGX(float cosLi, float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
    return GaSchlickG1(cosLi, k) * GaSchlickG1(NdotV, k);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Shlick's approximation of the Fresnel factor.
vec3 FresnelSchlick(vec3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

// ----------------------------------------------------------------------------
// Shadow Map
float ShadowCalculation(vec3 fragPosWorldSpace)
{
    vec3 projCoords = fragPosWorldSpace - uLightPos;
    vec3 normal = normalize(v_Normal);
    float bias = max(0.05 * (1.0 - dot(normal, normalize(projCoords))), 0.005);
    const float biasModifier = 0.5f;
    
    bias *= 1 / (uFarPlane * biasModifier);
    
    // PCF
    float shadow = 0.0;
    float viewDistance = length(uCamPos - fragPosWorldSpace);
    float diskRadius =  (1.0 + (viewDistance / uFarPlane)) / 25.0;
    float currentDepth = length(projCoords);

    bias = 0.15;
    for (int i = 0; i < 20; ++i)
    {
        //  + sampleOffsetDirections[i] * diskRadius
        float pcfDepth = texture(uShadowMap, projCoords).r * uFarPlane;
        shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
    }
    shadow /= 20;

    return shadow;
}

vec3 IBL(vec3 F0, vec3 Lr)
{
    vec3 irradiance = texture(uIrradianceMap, m_Params.Normal).rgb;
    vec3 F = FresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
    vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
    vec3 diffuseIBL = m_Params.Albedo * irradiance;

    int envRadianceTexLevels = textureQueryLevels(uIrradianceMap);
    float NoV = clamp(m_Params.NdotV, 0.0, 1.0);
    vec3 R = 2.0 * dot(m_Params.View, m_Params.Normal) * m_Params.Normal - m_Params.View;
    vec3 specularIrradiance = textureLod(uPrefilterMap, R, (m_Params.Roughness) * envRadianceTexLevels).rgb;
    //specularIrradiance = vec3(Convert_sRGB_FromLinear(specularIrradiance.r), Convert_sRGB_FromLinear(specularIrradiance.g), Convert_sRGB_FromLinear(specularIrradiance.b));

    // Sample BRDF Lut, 1.0 - roughness for y-coord because texture was generated (in Sparky) for gloss model
    vec2 specularBRDF = texture(uBRDFLUT, vec2(m_Params.NdotV, 1.0 - m_Params.Roughness)).rg;
    vec3 specularIBL = specularIrradiance * (F * specularBRDF.x + specularBRDF.y);

    return kd * diffuseIBL + specularIBL;
}

vec3 CalculateDirLights(vec3 F0)
{
    vec3 result = vec3(0.0);
    for (int i = 0; i < 1; i++) //Only one light for now
    {
        vec3 Li = normalize(uLightPos - v_WorldPos);
        vec3 Lradiance = vec3(1.0f);
        vec3 Lh = normalize(Li + m_Params.View);

        // Calculate angles between surface normal and various light vectors.
        float cosLi = max(0.0, dot(m_Params.Normal, Li));
        float cosLh = max(0.0, dot(m_Params.Normal, Lh));

        vec3 F = FresnelSchlickRoughness(F0, max(0.0, dot(Lh, m_Params.View)), m_Params.Roughness);
        float D = NdfGGX(cosLh, m_Params.Roughness);
        float G = GaSchlickGGX(cosLi, m_Params.NdotV, m_Params.Roughness);

        vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
        vec3 diffuseBRDF = kd * m_Params.Albedo;

        // Cook-Torrance
        vec3 specularBRDF = (F * D * G) / max(0.00001, 4.0 * cosLi * m_Params.NdotV);
        specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
        result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }
    return result;
}

// ----------------------------------------------------------------------------
void main()
{
    m_Params.Albedo     =  uUseAlbedo ? uAlbedo : texture(uAlbedoMap,v_TexCoord).rgb;
    m_Params.Metalness  = uMetallic;
    m_Params.Roughness = uUseRoughness ? uRoughness : texture(uRoughnessMap,v_TexCoord).r;

    m_Params.Normal = uUseNormal ? normalize(v_Normal) : normalize(texture(uNormalMap,v_TexCoord).rgb * 2.0 - 1.0);
    m_Params.View = normalize(uCamPos - v_WorldPos);
    vec3 R = reflect(-m_Params.View, m_Params.Normal);
    m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    // Specular reflection vector
    vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;
    // Fresnel reflectance, metals use albedo
    vec3 F0 = mix(vec3(0.04), m_Params.Albedo, m_Params.Metalness);

    float shadowScale = ShadowCalculation(v_WorldPos);
    shadowScale = clamp(1.0f - shadowScale, 0.0f, 1.0f);

    vec3 lightContribution = CalculateDirLights(F0) * shadowScale;

    vec3 iblContribution = IBL(F0, Lr) * 0.3f;

    vec4 color = vec4(iblContribution + lightContribution + m_Params.Albedo * uEmission , 1.0);

    color = color / (color + vec4(1.0));
    color = pow(color, vec4(1.0/2.2));

    FragColor = vec4(color.xyz,1.0);

    EntityID = uEntityID;
}