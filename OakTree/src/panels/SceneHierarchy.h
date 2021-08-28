#pragma once

#include "Acorn.h"

#include <imgui.h>

namespace Acorn
{
    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Ref<Scene> &context);

        void SetContext(const Ref<Scene> &context);

        void OnImGuiRender();

    private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);

    private:
        Ref<Scene> m_Context;
        Entity m_SelectionContext;
    };
}
