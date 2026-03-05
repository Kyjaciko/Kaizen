////////////////////////////////////////////////////////////////////////////////
// Filename: EntityManager.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _ENTITY_MANAGER_H_
#define _ENTITY_MANAGER_H_

//////////////
// INCLUDES //
//////////////

#include <array>
#include <queue>
#include <bitset>
#include <cassert>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

typedef uint32_t Entity;
inline constexpr Entity MAX_ENTITIES = 5000;

typedef uint8_t ComponentType;
inline constexpr ComponentType MAX_COMPONENTS = 32;

typedef std::bitset<MAX_COMPONENTS> Signature;

////////////////////////////////////////////////////////////////////////////////
// Class name: EntityManager
////////////////////////////////////////////////////////////////////////////////

namespace Kaizen::Logic
{
    class EntityManager
    {
	public:
		EntityManager()
		{
			// Initialize the queue with all possible entity IDs.
			for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
				m_AvailableEntities.push(entity);
		}

		Entity CreateEntity()
		{
			assert(m_LivingEntityCount < MAX_ENTITIES && "Too many entities in existence.");

			// Take an ID from the front of the queue.
			Entity id = m_AvailableEntities.front();
			m_AvailableEntities.pop();
			++m_LivingEntityCount;

			return id;
		}

		void DestroyEntity(Entity entity)
		{
			assert(entity < MAX_ENTITIES && "Entity out of range.");

			// Invalidate the destroyed entity's signature.
			m_Signatures[entity].reset();

			// Put the destroyed ID at the back of the queue.
			m_AvailableEntities.push(entity);
			--m_LivingEntityCount;
		}

		void SetSignature(Entity entity, Signature signature)
		{
			assert(entity < MAX_ENTITIES && "Entity out of range.");

			// Put this entity's signature into the array.
			m_Signatures[entity] = signature;
		}

		Signature GetSignature(Entity entity) const
		{
			assert(entity < MAX_ENTITIES && "Entity out of range.");

			// Get this entity's signature from the array.
			return m_Signatures[entity];
		}

	private:
		uint32_t							m_LivingEntityCount{};
		std::queue<Entity>					m_AvailableEntities{};
		std::array<Signature, MAX_ENTITIES> m_Signatures{};
    };
}

#endif // !_ENTITY_MANAGER_H_