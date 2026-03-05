////////////////////////////////////////////////////////////////////////////////
// Filename: ComponentArray.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _COMPONENT_ARRAY_H_
#define _COMPONENT_ARRAY_H_

//////////////
// INCLUDES //
//////////////

#include <unordered_map>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "IComponentArray.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: ComponentArray
////////////////////////////////////////////////////////////////////////////////

namespace Kaizen::Logic
{
	template<typename T>
	class ComponentArray : public IComponentArray
	{
	public:
		void InsertData(Entity entity, T component)
		{
			assert(m_EntityToIndexMap.find(entity) == m_EntityToIndexMap.end() && "Component added to same entity more than once.");

			// Put new entry at end and update the maps.
			size_t newIndex = m_Size;
			m_EntityToIndexMap[entity] = newIndex;
			m_IndexToEntityMap[newIndex] = entity;
			m_ComponentArray[newIndex] = component;
			++m_Size;
		}

		void RemoveData(Entity entity)
		{
			assert(m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end() && "Removing non-existent component.");

			// Copy element at end into deleted element's place to maintain density.
			size_t indexOfRemovedEntity = m_EntityToIndexMap[entity];
			size_t indexOfLastElement = m_Size - 1;
			m_ComponentArray[indexOfRemovedEntity] = m_ComponentArray[indexOfLastElement];

			// Update map to point to moved spot.
			Entity entityOfLastElement = m_IndexToEntityMap[indexOfLastElement];
			m_EntityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
			m_IndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

			m_EntityToIndexMap.erase(entity);
			m_IndexToEntityMap.erase(indexOfLastElement);

			--m_Size;
		}

		T& GetData(Entity entity)
		{
			assert(m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end() && "Retrieving non-existent component.");

			// Return a reference to the entity's component.
			return m_ComponentArray[m_EntityToIndexMap[entity]];
		}

		void EntityDestroyed(Entity entity) override
		{
			// Remove the entity's component if it exists.
			if (m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end())
				RemoveData(entity);
		}

	private:
		size_t								m_Size;
		std::array<T, MAX_ENTITIES>			m_ComponentArray;
		std::unordered_map<Entity, size_t>	m_EntityToIndexMap;
		std::unordered_map<size_t, Entity>	m_IndexToEntityMap;
	};
}

#endif // !_COMPONENT_ARRAY_H_