#pragma once

#include "EditorPanel.h"

#include <imgui/imgui.h>
#include <Lumos/Core/Reference.h>
#include <Lumos/Core/DataStructures/Vector.h>

namespace Lumos
{
    class ConsolePanel : public EditorPanel
    {
    public:
        class Message
        {
        public:
            enum Level : uint32_t
            {
                Trace    = 1,
                Debug    = 2,
                Info     = 4,
                Warn     = 8,
                Error    = 16,
                Critical = 32,
            };

        public:
            Message(const std::string& message = "", Level level = Level::Trace, const std::string& source = "", int threadID = 0, const std::string& time = "");
            void OnImGUIRender();
            void IncreaseCount() { m_Count++; };
            size_t GetMessageID() const { return m_MessageID; }

            static const char* GetLevelName(Level level);
            static const char* GetLevelIcon(Level level);
            static glm::vec4 GetRenderColour(Level level);

        public:
            const std::string m_Message;
            const Level m_Level;
            const std::string m_Source;
            const int m_ThreadID;
            std::string m_Time;
            int m_Count = 1;
            size_t m_MessageID;
        };

        ConsolePanel();
        ~ConsolePanel() = default;
        static void Flush();
        void OnImGui() override;

        static void AddMessage(const SharedPtr<Message>& message);

    private:
        void ImGuiRenderHeader();
        void ImGuiRenderMessages();

    private:
        static uint16_t s_MessageBufferCapacity;
        static uint16_t s_MessageBufferSize;
        static uint16_t s_MessageBufferBegin;
        static Vector<SharedPtr<Message>> s_MessageBuffer;
        static bool s_AllowScrollingToBottom;
        static bool s_RequestScrollToBottom;
        static uint32_t s_MessageBufferRenderFilter;
        ImGuiTextFilter Filter;
    };
}
