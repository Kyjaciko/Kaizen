////////////////////////////////////////////////////////////////////////////////
// Filename: ComponentManager.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _COMPONENT_MANAGER_H_
#define _COMPONENT_MANAGER_H_

//////////////
// INCLUDES //
//////////////

#include <memory>
#include <unordered_map>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "ComponentArray.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: ComponentManager
////////////////////////////////////////////////////////////////////////////////

namespace Kaizen::Logic
{
	class ComponentManager
	{
	public:
		template<typename T>
		void RegisterComponent()
		{
			const char* typeName = typeid(T).name();

			assert(m_ComponentTypes.find(typeName) == m_ComponentTypes.end() && "Registering component type more than once.");

			m_ComponentTypes.insert({ typeName, m_NextComponentType });
			m_ComponentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });
			++m_NextComponentType;
		}

		template<typename T>
		ComponentType GetComponentType()
		{
			const char* typeName = typeid(T).name();

			assert(m_ComponentTypes.find(typeName) != m_ComponentTypes.end() && "Component not registered before use.");

			// Return this component's type, used for creating signatures.
			return m_ComponentTypes[typeName];
		}

		template<typename T>
		void AddComponent(Entity entity, T component)
		{
			GetComponentArray<T>()->InsertData(entity, component);
		}

		template<typename T>
		void RemoveComponent(Entity entity)
		{
			GetComponentArray<T>()->RemoveData(entity);
		}

		template<typename T>
		T& GetComponent(Entity entity)
		{
			return GetComponentArray<T>()->GetData(entity);
		}

		void EntityDestroyed(Entity entity)
		{
			// Notify each component array that an entity has been destroyed.
			// If it has a component for that entity, it will remove it.
			for (auto const& pair : m_ComponentArrays)
			{
				auto const& component = pair.second;
				component->EntityDestroyed(entity);
			}
		}

	private:
		ComponentType														m_NextComponentType{}; // The component type to be assigned to the next registered component, starting at 0.
		std::unordered_map<const char*, ComponentType>						m_ComponentTypes{};
		std::unordered_map<const char*, std::shared_ptr<IComponentArray>>	m_ComponentArrays{};

	private:
		// Convenience function to get the statically casted pointer to the ComponentArray of type T.
		template<typename T>
		std::shared_ptr<ComponentArray<T>> GetComponentArray()
		{
			const char* typeName = typeid(T).name();

			assert(m_ComponentTypes.find(typeName) != m_ComponentTypes.end() && "Component not registered before use.");

			return std::static_pointer_cast<ComponentArray<T>>(m_ComponentArrays[typeName]);
		}
	};
}

#endif // !_COMPONENT_MANAGER_H_