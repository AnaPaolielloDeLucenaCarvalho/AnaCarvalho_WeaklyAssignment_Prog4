#include <string>
#include "GameObject.h"

void dae::GameObject::Update(float deltaTime)
{
	for (auto& component : m_Components)
	{
		component->Update(deltaTime);
	}

	std::erase_if(m_Components, [](const std::unique_ptr<Component>& component){ return component->IsMarkedForDestroy(); });
}

void dae::GameObject::Render() const
{
	for (const auto& component : m_Components)
	{
		component->Render();
	}
}

// W02
void dae::GameObject::SetLocalPosition(float x, float y)
{
	m_localTransform.SetPosition(x, y, 0.0f);
	SetPositionDirty();
}

const dae::Transform& dae::GameObject::GetTransform()
{
	if (m_positionIsDirty)
	{
		UpdateWorldTransform();
	}
	return m_worldTransform;
}

void dae::GameObject::SetPositionDirty()
{
	m_positionIsDirty = true;

	// If position changes, all children change
	for (auto child : m_pChildren)
	{
		child->SetPositionDirty();
	}
}

void dae::GameObject::UpdateWorldTransform()
{
	if (m_positionIsDirty)
	{
		if (m_pParent == nullptr)
		{
			m_worldTransform = m_localTransform;
		}
		else
		{
			const auto& parentPos = m_pParent->GetTransform().GetPosition();
			const auto& localPos = m_localTransform.GetPosition();

			m_worldTransform.SetPosition(parentPos.x + localPos.x, parentPos.y + localPos.y, 0.0f);
		}
	}
	m_positionIsDirty = false;
}

void dae::GameObject::SetParent(GameObject* parent, bool keepWorldPosition)
{
	// Validate
	if (IsChild(parent) || parent == this || m_pParent == parent)
	{
		return;
	}

	// Update Position
	if (parent == nullptr)
	{
		SetLocalPosition(GetTransform().GetPosition().x, GetTransform().GetPosition().y);
	}
	else
	{
		if (keepWorldPosition)
		{
			SetLocalPosition(GetTransform().GetPosition().x - parent->GetTransform().GetPosition().x, GetTransform().GetPosition().y - parent->GetTransform().GetPosition().y);
		}
		SetPositionDirty();
	}

	// Remove itself from previous parent
	if (m_pParent)
	{
		m_pParent->RemoveChild(this);
	}

	// Set the new parent pointer
	m_pParent = parent;

	// Add itself to the new parent's list
	if (m_pParent)
	{
		m_pParent->AddChild(this);
	}
}

void dae::GameObject::AddChild(GameObject* child)
{
	if (child == nullptr || child == this)
	{
		return;
	}

	child->m_pParent = this;
	m_pChildren.push_back(child);

	child->SetPositionDirty();
}

void dae::GameObject::RemoveChild(GameObject* child)
{
	if (child == nullptr || child->m_pParent != this)
	{
		return;
	}

	child->SetLocalPosition(child->GetTransform().GetPosition().x, child->GetTransform().GetPosition().y);
	child->SetPositionDirty();

	m_pChildren.erase(std::remove(m_pChildren.begin(), m_pChildren.end(), child), m_pChildren.end());

	child->m_pParent = nullptr;
}

bool dae::GameObject::IsChild(GameObject* child) const
{
	for (const auto& c : m_pChildren)
	{
		if (c == child || c->IsChild(child))
		{
			return true;
		}
	}
	return false;
}