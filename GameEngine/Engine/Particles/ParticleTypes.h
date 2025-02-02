#pragma once

#include "glm/glm.hpp"
#include <random>

class Entity;

enum class ParticleTypeVertex : int
{
	DEFAULT,
	MAX,
};

enum class ParticleTypeFragment : int
{
	DEFAULT,
	MAX
};

struct Range
{
	float min = 0.0f;
	float max = 1.0f;
};

struct CreateParticleInfo
{
	std::string texturePath = "";
	ParticleTypeVertex vertexType = ParticleTypeVertex::DEFAULT;
	ParticleTypeFragment fragmentType = ParticleTypeFragment::DEFAULT;
	glm::vec3 startingPos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 velocity = { 1.0f, 1.0f, 1.0f };
	float degreesPerFrame = 0.0f;
	float size = 0.0f;
	float lifeTime = 0.0f;
	bool applyGravity = false;
	bool lighting = false;
};

struct Particle
{
	glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	glm::vec3 acceleration = { 0.0f, 0.0f, 0.0f };
	Entity* entity = nullptr;
	float degreesPerFrame = 0.0f;
	float currentRotation = 0.0f;
	float lifeTime = 0.0f;
	float size = 0.0f;
	bool active = false;
};

typedef std::uniform_real_distribution<> UniformDist;
struct ParticleEmitter
{
	glm::vec3 position = {0.0f, 1.0f, 0.0f};
	std::string texture = "Textures/missingTexture.jpg";
	float rate = 1.0f;
	UniformDist scale;
	UniformDist lifeTime;
	UniformDist angularVel;
	UniformDist velocity;
	bool active = false;
};


struct CreateParticleEmitter
{
	glm::vec3 position = {0.0f, 1.0f, 0.0f};
	std::string texture = "Textures/missingTexture.jpg";
	float rate = 1.0f;
	Range scale;
	Range lifeTime;
	Range angularVel;
	Range velocity;
};
