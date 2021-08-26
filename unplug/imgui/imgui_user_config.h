struct ImGuiContext;
extern thread_local ImGuiContext* ImGuiThreadLocalContext;
#define GImGui ImGuiThreadLocalContext

struct ImPlotContext;
extern thread_local ImPlotContext* ImPlotThreadLocalContext;
#define GImPlot ImPlotThreadLocalContext