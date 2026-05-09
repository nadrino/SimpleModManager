#ifndef SIMPLEMODMANAGER_SYSTEMSTATUSOVERLAY_H
#define SIMPLEMODMANAGER_SYSTEMSTATUSOVERLAY_H

#include <borealis/frame_context.hpp>
#include <borealis/style.hpp>
#include <nanovg/nanovg.h>

namespace SystemStatusOverlay {

void draw(NVGcontext* vg, int x, int y, unsigned width, brls::Style* style, brls::FrameContext* ctx);
void shutdown();

}

#endif //SIMPLEMODMANAGER_SYSTEMSTATUSOVERLAY_H
