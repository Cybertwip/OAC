#include "HierarchyPanel.h"
#include "Editor.h"
#include "ApplicationInfoPanel.h"

#include <Lumos/Graphics/RHI/GraphicsContext.h>
#include <Lumos/Core/Application.h>
#include <Lumos/Scene/SceneManager.h>
#include <Lumos/Core/Engine.h>
#include <Lumos/Graphics/Renderers/SceneRenderer.h>
#include <Lumos/Graphics/GBuffer.h>
#include <Lumos/Events/ApplicationEvent.h>
#include <Lumos/ImGui/ImGuiUtilities.h>
#include <imgui/imgui.h>

namespace Lumos
{
    ApplicationInfoPanel::ApplicationInfoPanel()
    {
        m_Name       = "ApplicationInfo";
        m_SimpleName = "ApplicationInfo";
    }

    void ApplicationInfoPanel::OnImGui()
    {
        auto flags = ImGuiWindowFlags_NoCollapse;
        ImGui::Begin(m_Name.c_str(), &m_Active, flags);
        {
            if(ImGui::TreeNodeEx("Application", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto systems = Application::Get().GetSystemManager();

                if(ImGui::TreeNode("Systems"))
                {
                    systems->OnImGui();
                    ImGui::TreePop();
                }

                auto SceneRenderer = Application::Get().GetSceneRenderer();
                if(ImGui::TreeNode("SceneRenderer"))
                {
                    SceneRenderer->OnImGui();
                    ImGui::TreePop();
                }

                ImGui::NewLine();
                ImGui::Columns(2);
                bool VSync = Application::Get().GetWindow()->GetVSync();
                if(ImGuiUtilities::Property("VSync", VSync))
                {
                    auto editor = m_Editor;
                    Application::Get().QueueEvent([VSync, editor]
                                                  {
                        Application::Get().GetWindow()->SetVSync(VSync);
                        Application::Get().GetWindow()->GetSwapChain()->SetVSync(VSync);
                        Graphics::Renderer::GetRenderer()->OnResize(Application::Get().GetWindow()->GetWidth(), Application::Get().GetWindow()->GetHeight()); });
                }
                ImGui::Columns(1);
                ImGui::Text("FPS : %5.2i", Engine::Get().Statistics().FramesPerSecond);
                ImGui::Text("UPS : %5.2i", Engine::Get().Statistics().UpdatesPerSecond);
                ImGui::Text("Frame Time : %5.2f ms", Engine::Get().Statistics().FrameTime);
                ImGui::NewLine();
                ImGui::Text("Scene : %s", Application::Get().GetSceneManager()->GetCurrentScene()->GetSceneName().c_str());
                ImGui::TreePop();
            };
        }
        ImGui::End();
    }
}
