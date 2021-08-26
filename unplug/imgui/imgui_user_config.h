#pragma once
struct ImGuiContext;
extern thread_local ImGuiContext* ImGuiThreadLocalContext;
#define GImGui ImGuiThreadLocalContext