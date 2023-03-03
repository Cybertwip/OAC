#include "SceneSettingsPanel.h"
#include "Editor.h"

#include <Lumos/Scene/Scene.h>
#include <Lumos/Physics/LumosPhysicsEngine/LumosPhysicsEngine.h>

namespace Lumos
{
    SceneSettingsPanel::SceneSettingsPanel()
    {
        m_Name       = "SceneSettings###scenesettings";
        m_SimpleName = "Scene Settings";
    }

    void SceneSettingsPanel::OnImGui()
    {
        LUMOS_PROFILE_FUNCTION();

        if(!m_CurrentScene)
            return;

        if(ImGui::Begin(m_Name.c_str(), &m_Active, 0))
        {
            Lumos::ImGuiUtilities::ScopedStyle(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
            Lumos::ImGuiUtilities::PushID();
            {
                auto sceneName      = m_CurrentScene->GetSceneName();
                int sceneVersion    = m_CurrentScene->GetSceneVersion();
                auto& sceneSettings = m_CurrentScene->GetSettings();

                ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
                ImGui::PushItemWidth(contentRegionAvailable.x * 0.5f);

                {
                    ImGuiUtilities::ScopedFont boldFont(ImGui::GetIO().Fonts->Fonts[1]);
                    if(ImGuiUtilities::InputText(sceneName))
                    {
                        m_CurrentScene->SetName(sceneName);
                    }
                }

                ImGui::SameLine();
                ImGui::Text("Version : %i", (int)sceneVersion);

                ImGui::Columns(1);
                if(ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Columns(2);
                    ImGuiUtilities::Property("Renderer 2D Enabled", sceneSettings.RenderSettings.Renderer2DEnabled);
                    ImGuiUtilities::Property("Renderer 3D Enabled", sceneSettings.RenderSettings.Renderer3DEnabled);
                    ImGuiUtilities::Property("Shadow Enabled", sceneSettings.RenderSettings.ShadowsEnabled);
                    ImGuiUtilities::Property("Skybox Render Enabled", sceneSettings.RenderSettings.SkyboxRenderEnabled);
                    ImGuiUtilities::Property("Debug Renderer Enabled", sceneSettings.RenderSettings.DebugRenderEnabled);
                    ImGuiUtilities::Property("FXAA Enabled", sceneSettings.RenderSettings.FXAAEnabled);
                    ImGuiUtilities::Property("Debanding Enabled", sceneSettings.RenderSettings.DebandingEnabled);
                    ImGuiUtilities::Property("ChromaticAberation Enabled", sceneSettings.RenderSettings.ChromaticAberationEnabled);
                    ImGuiUtilities::Property("Filmic Grain Enabled", sceneSettings.RenderSettings.FilmicGrainEnabled);
                    ImGuiUtilities::Property("SSAO Enabled", sceneSettings.RenderSettings.SSAOEnabled);
                    ImGuiUtilities::Property("Bloom Enabled", sceneSettings.RenderSettings.BloomEnabled);
                    ImGuiUtilities::Property("Bloom Intensity", sceneSettings.RenderSettings.m_BloomIntensity);
                    ImGuiUtilities::Property("Bloom Upsample Scale", sceneSettings.RenderSettings.BloomUpsampleScale);
                    ImGuiUtilities::Property("Bloom Knee", sceneSettings.RenderSettings.BloomKnee);
                    ImGuiUtilities::Property("Bloom Threshold", sceneSettings.RenderSettings.BloomThreshold);
                    ImGuiUtilities::Property("Depth Of Field Enabled", sceneSettings.RenderSettings.DepthOfFieldEnabled);
                    ImGuiUtilities::Property("Depth Of Field Strength", sceneSettings.RenderSettings.DepthOfFieldStrength);
                    ImGuiUtilities::Property("Depth Of Field Distance", sceneSettings.RenderSettings.DepthOfFieldDistance);

                    static const char* toneMaps[7] = {
                        "None",
                        "Linear",
                        "Simple Reinhard",
                        "Luma Reinhard",
                        "White Preserved Luma Reinard",
                        "Uncharted 2",
                        "Aces"
                    };

                    ImGuiUtilities::PropertyDropdown("ToneMap", toneMaps, 7, (int*)&m_CurrentScene->GetSettings().RenderSettings.m_ToneMapIndex);

                    auto& registry  = m_CurrentScene->GetRegistry();
                    int entityCount = (int)registry.size();
                    ImGuiUtilities::Property("Entity Count", entityCount, 0, 0, ImGuiUtilities::PropertyFlag::ReadOnly);
                }

                ImGui::Columns(1);
                if(ImGui::CollapsingHeader("Systems"))
                {
                    ImGui::Columns(2);
                    ImGuiUtilities::Property("Audio Enabled", sceneSettings.AudioEnabled);
                    ImGuiUtilities::Property("Physics 2D Enabled", sceneSettings.PhysicsEnabled2D);
                    ImGuiUtilities::Property("Physics 3D Enabled", sceneSettings.PhysicsEnabled3D);
                }

                ImGui::Columns(1);
                if(ImGui::CollapsingHeader("Physics3D"))
                {
                    ImGui::Columns(2);
                    auto physicsSystem = Application::Get().GetSystem<LumosPhysicsEngine>();

                    if(physicsSystem)
                    {
                        if(ImGuiUtilities::Property("Dampening", sceneSettings.Physics3DSettings.Dampening))
                            physicsSystem->SetDampingFactor(sceneSettings.Physics3DSettings.Dampening);
                        if(ImGuiUtilities::Property("Max Updates Per Frame", sceneSettings.Physics3DSettings.m_MaxUpdatesPerFrame))
                            physicsSystem->SetMaxUpdatesPerFrame(sceneSettings.Physics3DSettings.m_MaxUpdatesPerFrame);
                        if(ImGuiUtilities::Property("Position Iterations", sceneSettings.Physics3DSettings.PositionIterations))
                            physicsSystem->SetPositionIterations(sceneSettings.Physics3DSettings.PositionIterations);
                        if(ImGuiUtilities::Property("Velocity Iterations", sceneSettings.Physics3DSettings.VelocityIterations))
                            physicsSystem->SetPositionIterations(sceneSettings.Physics3DSettings.VelocityIterations);
                        if(ImGuiUtilities::Property("Gravity", sceneSettings.Physics3DSettings.Gravity))
                            physicsSystem->SetGravity(sceneSettings.Physics3DSettings.Gravity);

                        if(ImGuiUtilities::Property("BroadPhaseIndex", sceneSettings.Physics3DSettings.BroadPhaseTypeIndex))
                            physicsSystem->SetBroadphaseType((BroadphaseType)sceneSettings.Physics3DSettings.BroadPhaseTypeIndex);
                        if(ImGuiUtilities::Property("IntegrationType", sceneSettings.Physics3DSettings.IntegrationTypeIndex))
                            physicsSystem->SetIntegrationType((IntegrationType)sceneSettings.Physics3DSettings.IntegrationTypeIndex);
                    }
                }

                ImGui::Columns(1);
                if(ImGui::CollapsingHeader("Physics2D"))
                {
                    ImGui::Columns(2);
                    ImGuiUtilities::Property("Dampening", sceneSettings.Physics2DSettings.Dampening);
                    ImGuiUtilities::Property("Position Iterations", sceneSettings.Physics2DSettings.PositionIterations);
                    ImGuiUtilities::Property("Velocity Iterations", sceneSettings.Physics2DSettings.VelocityIterations);
                    ImGuiUtilities::Property("Gravity", sceneSettings.Physics2DSettings.Gravity);
                    ImGuiUtilities::Property("IntegrationType", sceneSettings.Physics3DSettings.IntegrationTypeIndex);
                }
            }
            ImGui::Columns(1);
            Lumos::ImGuiUtilities::PopID();
        }
        ImGui::End();
    }
}
