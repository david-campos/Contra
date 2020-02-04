#include "game_object.h"
#include "component.h"
#include "avancezlib.h"

void GameObject::Create()
{
	SDL_Log("GameObject::Create");

	enabled = false;
}

void GameObject::AddComponent(Component * component)
{
	components.push_back(component);
}


void GameObject::Init()
{
	SDL_Log("GameObject::Init");

	for (auto it = components.begin(); it != components.end(); it++)
		(*it)->Init();

	enabled = true;
}

void GameObject::Update(float dt)
{
	if (!enabled)
		return;

	for (auto it = components.begin(); it != components.end(); it++)
		(*it)->Update(dt);
}

void GameObject::Destroy()
{
	for (auto it = components.begin(); it != components.end(); it++)
		(*it)->Destroy();
}

GameObject::~GameObject()
{
	SDL_Log("GameObject::~GameObject");
}

void GameObject::AddReceiver(GameObject * go)
{
	receivers.push_back(go);
}

void GameObject::Send(Message m)
{
	for (auto i = 0; i < receivers.size(); i++)
	{
		if (!receivers[i]->enabled)
			continue;

		receivers[i]->Receive(m);
	}
}
