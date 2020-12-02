#include "GameObjectManager.h"
#include "Renderer.h"

template <> GameObjectManager* Singleton<GameObjectManager>::msSingleton = 0;

GameObjectManager* GameObjectManager::getSingletonPtr()
{
	return msSingleton;
}

GameObjectManager& GameObjectManager::getSingleton()
{
	assert(msSingleton); return (*msSingleton);
}

GameObjectManager::GameObjectManager() 
{
}

GameObjectManager::~GameObjectManager() 
{
	std::cout << "delete GameObjectManager" << std::endl;
	for (auto gObject : m_gameObjects)
	{
		delete gObject;
	}
}

void GameObjectManager::setCameraId(int id)
{
	assert(id < m_cameras.size());
	m_currentCameraId = id;
}

int GameObjectManager::addGameObject(std::string modelPath, std::string texturePath, glm::vec3 pos, PipelineTypes type)
{
	VkDescriptorSetLayout layout;
	GraphicsPipeline* pipeline = PipelineManager::getSingleton().createOrGetPipeline(type, layout);
	
	GameObject* gameObject = new GameObject(modelPath, texturePath, pos, layout);
	gameObject->setPosition(pos);
	m_gameObjects.push_back(gameObject);
	
	Renderer::getSingleton().bindTexturedMeshToPipeline(gameObject->getMeshPtr(), pipeline);
	return static_cast<int>(m_gameObjects.size()-1);
}

int GameObjectManager::addGameObject(std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::vec3 pos, PipelineTypes type)
{
	VkDescriptorSetLayout layout;
	GraphicsPipeline* pipeline = PipelineManager::getSingleton().createOrGetPipeline(type, layout);
	
	GameObject* gameObject = new GameObject(vertices, indices, pos, layout);
	gameObject->setPosition(pos);
	m_gameObjects.push_back(gameObject);
	
	Renderer::getSingleton().bindTexturedMeshToPipeline(gameObject->getMeshPtr(), pipeline);
	return static_cast<int>(m_gameObjects.size()-1);
}

int GameObjectManager::addCamera(BaseCamera* camera)
{
	m_cameras.push_back(camera);
	return static_cast<int>(m_cameras.size()-1);
}

GameObject* GameObjectManager::getGameObjectPtrById(int id)
{
	assert(id < m_gameObjects.size() && id >= 0);
	return m_gameObjects[id];
}

void GameObjectManager::updateViewMatricies()
{
	glm::mat4 currentView = m_cameras[m_currentCameraId]->getView();
	for (GameObject* object : m_gameObjects)
	{
		object->getMeshPtr()->setViewMatrix(currentView);
	}
}

void GameObjectManager::updateGameObjectPosition(int id, glm::vec3 pos)
{
	assert(id < m_gameObjects.size());
	GameObject* g = getGameObjectPtrById(id);
	g->setPosition(pos);
}
