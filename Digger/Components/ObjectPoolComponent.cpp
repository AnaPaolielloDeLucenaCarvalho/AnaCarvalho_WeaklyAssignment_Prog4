#include "ObjectPoolComponent.h"
#include <imgui.h>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <memory>

namespace dae
{
    struct Transform
    {
        float matrix[16] =
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };
    };

    struct GameObject3D
    {
        Transform transform{};
        int ID{};
    };

    struct GameObject3DAlt
    {
        // DESIGN DECISION - Pointer Chasing Hazard
        // Storing a raw pointer here forces the CPU to drive across town (RAM) to find the Transform. This will purposely cause a Cache Miss and demonstrate memory thrashing.
        Transform* transform{};
        int ID{};
    };

    // DESIGN PATTERN - Template Metaprogramming
    // Using a template here raises the level of abstraction. Instead of writing three nearly identical  loops for Int, GameObject3D, and GameObject3DAlt, the compiler generates them for us safely.
    template <typename T>
    void BenchmarkTrashCache(size_t bufferSize, int samples, std::vector<float>& outputTimings)
    {
        // DESIGN DECISION - RAII and Heap Allocation
        // We use std::make_unique to allocate the massive buffer on the heap. RAII guarantees this memory is automatically freed when the function ends, preventing leaks.
        auto arr = std::make_unique<T[]>(bufferSize);
        outputTimings.clear();

        for (int step = 1; step <= 1024; step *= 2)
        {
            std::vector<float> currentTimings;
            currentTimings.reserve(samples);

            for (int s = 0; s < samples; ++s)
            {
                auto start = std::chrono::high_resolution_clock::now();

                // DESIGN DECISION - Data Locality & The Step Size Trap
                // When step == 1, we pull a full Cache Line (64 bytes) and process it instantly.
                // When step == 16, we throw the Cache Line away after 1 read, forcing a slow trip to RAM.
                for (size_t i = 0; i < bufferSize; i += step)
                {
                    // Compile-time branch (constexpr) so there is zero runtime penalty for checking types
                    if constexpr (std::is_same_v<T, int>) arr[i] *= 2;
                    else arr[i].ID *= 2;
                }

                auto end = std::chrono::high_resolution_clock::now();
                auto elapsed = static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
                currentTimings.push_back(elapsed);
            }

            // Remove the highest and lowest outliers to clean up the graph data
            auto [minIt, maxIt] = std::minmax_element(currentTimings.begin(), currentTimings.end());
            float minVal = *minIt;
            float maxVal = *maxIt;

            // DESIGN DECISION - Standard Algorithms
            // Using std::accumulate is much safer and cleaner than writing a raw for-loop to sum the array.
            float totalSum = std::accumulate(currentTimings.begin(), currentTimings.end(), 0.0f);

            float avg = (totalSum - minVal - maxVal) / (samples - 2);
            outputTimings.push_back(avg / 1000.f);
        }
    }

    ObjectPoolComponent::ObjectPoolComponent(GameObject* pOwner) : Component(pOwner)
    {
        for (int step = 1; step <= 1024; step *= 2)
        {
            m_xData.push_back(static_cast<float>(step));
        }
    }

    void ObjectPoolComponent::Update(float /*deltaTime*/) {}

    void ObjectPoolComponent::Render() const
    {
        ImGui::Begin("Programming 4 Assignment");

        ImGui::InputInt("# samples", &m_samples);
        if (m_samples < 1)
        {
            m_samples = 1;
        }

        // Exercise 1
        if (ImGui::Button("Trash the cache (Int)"))
        {
            RunExercise1();
        }
        if (!m_timingsInt.empty())
        {
            float maxVal = *std::max_element(m_timingsInt.begin(), m_timingsInt.end());
            ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0f, 0.5f, 0.0f, 1.0f)); // Orange
            ImGui::PlotLines("##IntPlot", m_timingsInt.data(), static_cast<int>(m_timingsInt.size()), 0, "Exercise 1", 0.0f, maxVal * 1.1f, ImVec2(400, 150));
            ImGui::PopStyleColor();
        }
        ImGui::Separator();

        // Exercise 2
        if (ImGui::Button("Trash the cache with GameObject3D"))
        {
            RunExercise2();
        }
        if (!m_timingsGO3D.empty())
        {
            float maxVal = *std::max_element(m_timingsGO3D.begin(), m_timingsGO3D.end());
            ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0f, 0.5f, 1.0f, 1.0f)); // Blue
            ImGui::PlotLines("##GO3DPlot", m_timingsGO3D.data(), static_cast<int>(m_timingsGO3D.size()), 0, "Exercise 2", 0.0f, maxVal * 1.1f, ImVec2(400, 150));
            ImGui::PopStyleColor();
        }

		// Exercise 2 Alternative
        if (ImGui::Button("Trash the cache with GameObject3DAlt"))
        {
            RunExercise2Alt();
        }
        if (!m_timingsGO3DAlt.empty())
        {
            float maxVal = *std::max_element(m_timingsGO3DAlt.begin(), m_timingsGO3DAlt.end());
            ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0f, 1.0f, 0.5f, 1.0f)); // Green
            ImGui::PlotLines("##GO3DAltPlot", m_timingsGO3DAlt.data(), static_cast<int>(m_timingsGO3DAlt.size()), 0, "Exercise 2 Alt", 0.0f, maxVal * 1.1f, ImVec2(400, 150));
            ImGui::PopStyleColor();
        }
        ImGui::Separator();

		// Combined Exercise 2 and Alternative
        if (!m_timingsGO3D.empty() && !m_timingsGO3DAlt.empty())
        {
            ImGui::Text("Combined:");

            float maxGO3D = *std::max_element(m_timingsGO3D.begin(), m_timingsGO3D.end());
            float maxGO3DAlt = *std::max_element(m_timingsGO3DAlt.begin(), m_timingsGO3DAlt.end());
            float globalMax = std::max(maxGO3D, maxGO3DAlt);

            ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));
            ImGui::PlotLines("##CombinedPlot1", m_timingsGO3D.data(), static_cast<int>(m_timingsGO3D.size()), 0, "", 0.0f, globalMax * 1.1f, ImVec2(400, 150));
            ImGui::PopStyleColor();

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 154);

            ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0f, 1.0f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
            ImGui::PlotLines("##CombinedPlot2", m_timingsGO3DAlt.data(), static_cast<int>(m_timingsGO3DAlt.size()), 0, "", 0.0f, globalMax * 1.1f, ImVec2(400, 150));
            ImGui::PopStyleColor(2);
        }

        ImGui::End();
    }

    void ObjectPoolComponent::RunExercise1() const
    {
        const size_t bufferSize = 1 << 26;
        BenchmarkTrashCache<int>(bufferSize, m_samples, m_timingsInt);
    }

    void ObjectPoolComponent::RunExercise2() const
    {
        const size_t bufferSize = 1 << 24;
        BenchmarkTrashCache<GameObject3D>(bufferSize, m_samples, m_timingsGO3D);
    }

    void ObjectPoolComponent::RunExercise2Alt() const
    {
        const size_t bufferSize = 1 << 24;

        auto transforms = std::make_unique<Transform[]>(bufferSize);
        auto objects = std::make_unique<GameObject3DAlt[]>(bufferSize);

        for (size_t i = 0; i < bufferSize; ++i)
        {
            objects[i].transform = &transforms[i];
            objects[i].ID = static_cast<int>(i);
        }

        m_timingsGO3DAlt.clear();
        for (int step = 1; step <= 1024; step *= 2)
        {
            std::vector<float> currentTimings;
            currentTimings.reserve(m_samples);

            for (int s = 0; s < m_samples; ++s)
            {
                auto start = std::chrono::high_resolution_clock::now();
                for (size_t i = 0; i < bufferSize; i += step)
                {
                    objects[i].ID *= 2;
                }
                auto end = std::chrono::high_resolution_clock::now();
                currentTimings.push_back(static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()));
            }

            auto [minIt, maxIt] = std::minmax_element(currentTimings.begin(), currentTimings.end());
            float total = std::accumulate(currentTimings.begin(), currentTimings.end(), 0.0f);
            m_timingsGO3DAlt.push_back(((total - *minIt - *maxIt) / (m_samples - 2)) / 1000.f);
        }
    }
}