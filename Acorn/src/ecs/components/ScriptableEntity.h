#pragma once

#include "ecs/Entity.h"

namespace Acorn
{
    class ScriptableEntity
    {
    public:
        virtual ~ScriptableEntity() = default;

        template <typename T>
        bool HasComponent()
        {
            AC_CORE_ASSERT(m_Scene != nullptr, "Scene is null");
            AC_CORE_ASSERT(m_EntityHandle != entt::null, "Entity is null");

            return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
        }

        template <typename... T>
        bool HasComponents()
        {
            return m_Entity.HasComponents<T>();
        }

        template <typename T>
        T &GetComponent()
        {
            return m_Entity.GetComponent<T>();
        }

        template <typename T, typename... Args>
        T &AddComponent(Args &&...args)
        {
            return m_Entity.AddComponent<T>(std::forward<Args>(args)...);
        }

        template <typename T>
        void RemoveComponent()
        {
            m_Entity.RemoveComponent<T>();
        }

    protected:
        virtual void OnCreate(){};
        virtual void OnDestroy(){};
        virtual void OnUpdate(Timestep ts){};

    private:
        Entity m_Entity;
        friend class Scene;
    };
}