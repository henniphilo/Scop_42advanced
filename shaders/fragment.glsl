#version 410 core

in vec3 vColor;
in vec2 vUV;
in vec3 vNormal;
in vec3 vWorldPos;

uniform sampler2D uTexture;
uniform float uTexMix;
uniform vec3 uLightDir;
uniform vec3 uCameraPos;

out vec4 FragColor;

void main() {
	vec3 n = normalize(vNormal);
	vec3 lightDir = normalize(-uLightDir);
	vec3 viewDir = normalize(uCameraPos - vWorldPos);
	vec3 halfDir = normalize(lightDir + viewDir);

	float ndl = max(dot(n, lightDir), 0.0);
	float ambient = 0.28;
	float diffuse = 0.55 * ndl;

	// Stronger specular when textured → glossy rainbow look
	float gloss = mix(16.0, 64.0, clamp(uTexMix, 0.0, 1.0));
	float specStrength = mix(0.12, 0.85, clamp(uTexMix, 0.0, 1.0));
	float spec = pow(max(dot(n, halfDir), 0.0), gloss) * specStrength;

	vec3 baseColor = mix(vColor, texture(uTexture, vUV).rgb, clamp(uTexMix, 0.0, 1.0));
	vec3 lit = baseColor * (ambient + diffuse) + vec3(spec);
	FragColor = vec4(lit, 1.0);
}
