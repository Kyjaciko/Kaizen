////////////////////////////////////////////////////////////////////////////////
// Filename: SystemManager.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _SYSTEM_MANAGER_H_
#define _SYSTEM_MANAGER_H_

//////////////
// INCLUDES //
//////////////

#include <memory>
#include <unordered_map>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "System.h"
#include "EntityManager.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: SystemManager
////////////////////////////////////////////////////////////////////////////////

namespace Kaizen::Logic
{
	class SystemManager
	{
	public:
		template<typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			const char* typeName = typeid(T).name();

			assert(m_Systems.find(typeName) == m_Systems.end() && "Registering system more than once.");

			auto system = std::make_shared<T>();
			m_Systems.insert({ typeName, system });
			return system;
		}

		template<typename T>
		void SetSignature(Signature signature)
		{
			const char* typeName = typeid(T).name();

			assert(m_Systems.find(typeName) != m_Systems.end() && "System used before registered.");

			// Set the signature for this system.
			m_Signatures.insert({ typeName, signature });
		}

		void EntityDestroyed(Entity entity)
		{
			// Erase a destroyed entity from all system lists.
			// m_Entities is a set so no check is needed.
			for (auto const& pair : m_Systems)
			{
				auto const& system = pair.second;
				system->m_Entities.erase(entity);
			}
		}

		void EntitySignatureChanged(Entity entity, Signature entitySignature)
		{
			// Notify each system that an entity's signature changed.
			for (auto const& pair : m_Systems)
			{
				auto const& type = pair.first;
				auto const& system = pair.second;
				auto const& systemSignature = m_Signatures[type];

				// Entity signature matches system signature, insert into set.
				if ((entitySignature & systemSignature) == systemSignature)
					system->m_Entities.insert(entity);
				else // Entity signature does not match system signature, erase from set.
					system->m_Entities.erase(entity);
			}
		}

	private:
		std::unordered_map<const char*, Signature> m_Signatures{};
		std::unordered_map<const char*, std::shared_ptr<System>> m_Systems{};
	};
}

#endif // !_SYSTEM_MANAGER_H_