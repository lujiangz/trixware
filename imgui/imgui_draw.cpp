// dear imgui, v1.52 WIP
// (drawing and font code)

// Contains implementation for
// - ImDrawList
// - ImDrawData
// - ImFontAtlas
// - ImFont
// - Default font data

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_PLACEMENT_NEW
#include "imgui_internal.h"

#include <stdio.h>      // vsnprintf, sscanf, printf
#if !defined(alloca)
#ifdef _WIN32
#include <malloc.h>     // alloca
#elif defined(__GLIBC__) || defined(__sun)
#include <alloca.h>     // alloca
#else
#include <stdlib.h>     // alloca
#endif
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#define snprintf _snprintf
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants ok.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning : declaration requires a global destructor           // similar to above, not sure what the exact difference it.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"      // warning : macro name is a reserved identifier                //
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"       // warning: implicit conversion from 'float' to 'double' when passing argument to function
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'xxxx' to type 'xxxx' casts away qualifiers
#endif

//-------------------------------------------------------------------------
// STB libraries implementation
//-------------------------------------------------------------------------

//#define IMGUI_STB_NAMESPACE     ImGuiStb
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION

#ifdef IMGUI_STB_NAMESPACE
namespace IMGUI_STB_NAMESPACE
{
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4456)                             // declaration of 'xx' hides previous local declaration
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"              // warning: comparison is always true due to limited range of data type [-Wtype-limits]
#endif

#define STBRP_ASSERT(x)    IM_ASSERT(x)
#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#include "stb_rect_pack.h"

#define STBTT_malloc(x,u)  ((void)(u), ImGui::MemAlloc(x))
#define STBTT_free(x,u)    ((void)(u), ImGui::MemFree(x))
#define STBTT_assert(x)    IM_ASSERT(x)
#ifndef IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#else
#define STBTT_DEF extern
#endif
#include "stb_truetype.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#ifdef IMGUI_STB_NAMESPACE
} // namespace ImGuiStb
using namespace IMGUI_STB_NAMESPACE;
#endif

//-----------------------------------------------------------------------------
// ImDrawList
//-----------------------------------------------------------------------------

static const ImVec4 GNullClipRect(-8192.0f, -8192.0f, +8192.0f, +8192.0f); // Large values that are easy to encode in a few bits+shift

void ImDrawList::Clear()
{
    CmdBuffer.resize(0);
    IdxBuffer.resize(0);
    VtxBuffer.resize(0);
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.resize(0);
    _TextureIdStack.resize(0);
    _Path.resize(0);
    _ChannelsCurrent = 0;
    _ChannelsCount = 1;
    // NB: Do not clear channels so our allocations are re-used after the first frame.
}

void ImDrawList::ClearFreeMemory()
{
    CmdBuffer.clear();
    IdxBuffer.clear();
    VtxBuffer.clear();
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.clear();
    _TextureIdStack.clear();
    _Path.clear();
    _ChannelsCurrent = 0;
    _ChannelsCount = 1;
    for (int i = 0; i < _Channels.Size; i++)
    {
        if (i == 0) memset(&_Channels[0], 0, sizeof(_Channels[0]));  // channel 0 is a copy of CmdBuffer/IdxBuffer, don't destruct again
        _Channels[i].CmdBuffer.clear();
        _Channels[i].IdxBuffer.clear();
    }
    _Channels.clear();
}

// Use macros because C++ is a terrible language, we want guaranteed inline, no code in header, and no overhead in Debug mode
#define GetCurrentClipRect()    (_ClipRectStack.Size ? _ClipRectStack.Data[_ClipRectStack.Size-1]  : GNullClipRect)
#define GetCurrentTextureId()   (_TextureIdStack.Size ? _TextureIdStack.Data[_TextureIdStack.Size-1] : NULL)

void ImDrawList::AddDrawCmd()
{
    ImDrawCmd draw_cmd;
    draw_cmd.ClipRect = GetCurrentClipRect();
    draw_cmd.TextureId = GetCurrentTextureId();

    IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
    CmdBuffer.push_back(draw_cmd);
}

void ImDrawList::AddCallback(ImDrawCallback callback, void* callback_data)
{
    ImDrawCmd* current_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!current_cmd || current_cmd->ElemCount != 0 || current_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        current_cmd = &CmdBuffer.back();
    }
    current_cmd->UserCallback = callback;
    current_cmd->UserCallbackData = callback_data;

    AddDrawCmd(); // Force a new command after us (see comment below)
}

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void ImDrawList::UpdateClipRect()
{
    // If current command is used with different settings we need to add a new command
    const ImVec4 curr_clip_rect = GetCurrentClipRect();
    ImDrawCmd* curr_cmd = CmdBuffer.Size > 0 ? &CmdBuffer.Data[CmdBuffer.Size-1] : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) != 0) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && memcmp(&prev_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) == 0 && prev_cmd->TextureId == GetCurrentTextureId() && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->ClipRect = curr_clip_rect;
}

void ImDrawList::UpdateTextureID()
{
    // If current command is used with different settings we need to add a new command
    const ImTextureID curr_texture_id = GetCurrentTextureId();
    ImDrawCmd* curr_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && curr_cmd->TextureId != curr_texture_id) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && prev_cmd->TextureId == curr_texture_id && memcmp(&prev_cmd->ClipRect, &GetCurrentClipRect(), sizeof(ImVec4)) == 0 && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->TextureId = curr_texture_id;
}

#undef GetCurrentClipRect
#undef GetCurrentTextureId

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void ImDrawList::PushClipRect(ImVec2 cr_min, ImVec2 cr_max, bool intersect_with_current_clip_rect)
{
    ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
    if (intersect_with_current_clip_rect && _ClipRectStack.Size)
    {
        ImVec4 current = _ClipRectStack.Data[_ClipRectStack.Size-1];
        if (cr.x < current.x) cr.x = current.x;
        if (cr.y < current.y) cr.y = current.y;
        if (cr.z > current.z) cr.z = current.z;
        if (cr.w > current.w) cr.w = current.w;
    }
    cr.z = ImMax(cr.x, cr.z);
    cr.w = ImMax(cr.y, cr.w);

    _ClipRectStack.push_back(cr);
    UpdateClipRect();
}

void ImDrawList::PushClipRectFullScreen()
{
    PushClipRect(ImVec2(GNullClipRect.x, GNullClipRect.y), ImVec2(GNullClipRect.z, GNullClipRect.w));
    //PushClipRect(GetVisibleRect());   // FIXME-OPT: This would be more correct but we're not supposed to access ImGuiContext from here?
}

void ImDrawList::PopClipRect()
{
    IM_ASSERT(_ClipRectStack.Size > 0);
    _ClipRectStack.pop_back();
    UpdateClipRect();
}

void ImDrawList::PushTextureID(const ImTextureID& texture_id)
{
    _TextureIdStack.push_back(texture_id);
    UpdateTextureID();
}

void ImDrawList::PopTextureID()
{
    IM_ASSERT(_TextureIdStack.Size > 0);
    _TextureIdStack.pop_back();
    UpdateTextureID();
}

void ImDrawList::ChannelsSplit(int channels_count)
{
    IM_ASSERT(_ChannelsCurrent == 0 && _ChannelsCount == 1);
    int old_channels_count = _Channels.Size;
    if (old_channels_count < channels_count)
        _Channels.resize(channels_count);
    _ChannelsCount = channels_count;

    // _Channels[] (24 bytes each) hold storage that we'll swap with this->_CmdBuffer/_IdxBuffer
    // The content of _Channels[0] at this point doesn't matter. We clear it to make state tidy in a debugger but we don't strictly need to.
    // When we switch to the next channel, we'll copy _CmdBuffer/_IdxBuffer into _Channels[0] and then _Channels[1] into _CmdBuffer/_IdxBuffer
    memset(&_Channels[0], 0, sizeof(ImDrawChannel));
    for (int i = 1; i < channels_count; i++)
    {
        if (i >= old_channels_count)
        {
            IM_PLACEMENT_NEW(&_Channels[i]) ImDrawChannel();
        }
        else
        {
            _Channels[i].CmdBuffer.resize(0);
            _Channels[i].IdxBuffer.resize(0);
        }
        if (_Channels[i].CmdBuffer.Size == 0)
        {
            ImDrawCmd draw_cmd;
            draw_cmd.ClipRect = _ClipRectStack.back();
            draw_cmd.TextureId = _TextureIdStack.back();
            _Channels[i].CmdBuffer.push_back(draw_cmd);
        }
    }
}

void ImDrawList::ChannelsMerge()
{
    // Note that we never use or rely on channels.Size because it is merely a buffer that we never shrink back to 0 to keep all sub-buffers ready for use.
    if (_ChannelsCount <= 1)
        return;

    ChannelsSetCurrent(0);
    if (CmdBuffer.Size && CmdBuffer.back().ElemCount == 0)
        CmdBuffer.pop_back();

    int new_cmd_buffer_count = 0, new_idx_buffer_count = 0;
    for (int i = 1; i < _ChannelsCount; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (ch.CmdBuffer.Size && ch.CmdBuffer.back().ElemCount == 0)
            ch.CmdBuffer.pop_back();
        new_cmd_buffer_count += ch.CmdBuffer.Size;
        new_idx_buffer_count += ch.IdxBuffer.Size;
    }
    CmdBuffer.resize(CmdBuffer.Size + new_cmd_buffer_count);
    IdxBuffer.resize(IdxBuffer.Size + new_idx_buffer_count);

    ImDrawCmd* cmd_write = CmdBuffer.Data + CmdBuffer.Size - new_cmd_buffer_count;
    _IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size - new_idx_buffer_count;
    for (int i = 1; i < _ChannelsCount; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (int sz = ch.CmdBuffer.Size) { memcpy(cmd_write, ch.CmdBuffer.Data, sz * sizeof(ImDrawCmd)); cmd_write += sz; }
        if (int sz = ch.IdxBuffer.Size) { memcpy(_IdxWritePtr, ch.IdxBuffer.Data, sz * sizeof(ImDrawIdx)); _IdxWritePtr += sz; }
    }
    UpdateClipRect(); // We call this instead of AddDrawCmd(), so that empty channels won't produce an extra draw call.
    _ChannelsCount = 1;
}

void ImDrawList::ChannelsSetCurrent(int idx)
{
    IM_ASSERT(idx < _ChannelsCount);
    if (_ChannelsCurrent == idx) return;
    memcpy(&_Channels.Data[_ChannelsCurrent].CmdBuffer, &CmdBuffer, sizeof(CmdBuffer)); // copy 12 bytes, four times
    memcpy(&_Channels.Data[_ChannelsCurrent].IdxBuffer, &IdxBuffer, sizeof(IdxBuffer));
    _ChannelsCurrent = idx;
    memcpy(&CmdBuffer, &_Channels.Data[_ChannelsCurrent].CmdBuffer, sizeof(CmdBuffer));
    memcpy(&IdxBuffer, &_Channels.Data[_ChannelsCurrent].IdxBuffer, sizeof(IdxBuffer));
    _IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size;
}

// NB: this can be called with negative count for removing primitives (as long as the result does not underflow)
void ImDrawList::PrimReserve(int idx_count, int vtx_count)
{
    ImDrawCmd& draw_cmd = CmdBuffer.Data[CmdBuffer.Size-1];
    draw_cmd.ElemCount += idx_count;

    int vtx_buffer_old_size = VtxBuffer.Size;
    VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
    _VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

    int idx_buffer_old_size = IdxBuffer.Size;
    IdxBuffer.resize(idx_buffer_old_size + idx_count);
    _IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
}

// Fully unrolled with inline call to keep our debug builds decently fast.
void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv(GImGui->FontTexUvWhitePixel);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimRectUV(const ImVec2& a, const ImVec2& c, const ImVec2& uv_a, const ImVec2& uv_c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

// TODO: Thickness anti-aliased lines cap are missing their AA fringe.
void ImDrawList::AddPolyline(const ImVec2* points, const int points_count, ImU32 col, bool closed, float thickness, bool anti_aliased)
{
    if (points_count < 2)
        return;

    const ImVec2 uv = GImGui->FontTexUvWhitePixel;
    anti_aliased &= GImGui->Style.AntiAliasedLines;
    //if (ImGui::GetIO().KeyCtrl) anti_aliased = false; // Debug

    int count = points_count;
    if (!closed)
        count = points_count-1;

    const bool thick_line = thickness > 1.0f;
    if (anti_aliased)
    {
        // Anti-aliased stroke
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;

        const int idx_count = thick_line ? count*18 : count*12;
        const int vtx_count = thick_line ? points_count*4 : points_count*3;
        PrimReserve(idx_count, vtx_count);

        // Temporary buffer
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * (thick_line ? 5 : 3) * sizeof(ImVec2));
        ImVec2* temp_points = temp_normals + points_count;

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            ImVec2 diff = points[i2] - points[i1];
            diff *= ImInvLength(diff, 1.0f);
            temp_normals[i1].x = diff.y;
            temp_normals[i1].y = -diff.x;
        }
        if (!closed)
            temp_normals[points_count-1] = temp_normals[points_count-2];

        if (!thick_line)
        {
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
                temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
                temp_points[(points_count-1)*2+0] = points[points_count-1] + temp_normals[points_count-1] * AA_SIZE;
                temp_points[(points_count-1)*2+1] = points[points_count-1] - temp_normals[points_count-1] * AA_SIZE;
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+3;

                // Average normals
                ImVec2 dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
                float dmr2 = dm.x*dm.x + dm.y*dm.y;
                if (dmr2 > 0.000001f)
                {
                    float scale = 1.0f / dmr2;
                    if (scale > 100.0f) scale = 100.0f;
                    dm *= scale;
                }
                dm *= AA_SIZE;
                temp_points[i2*2+0] = points[i2] + dm;
                temp_points[i2*2+1] = points[i2] - dm;

                // Add indexes
                _IdxWritePtr[0] = (ImDrawIdx)(idx2+0); _IdxWritePtr[1] = (ImDrawIdx)(idx1+0); _IdxWritePtr[2] = (ImDrawIdx)(idx1+2);
                _IdxWritePtr[3] = (ImDrawIdx)(idx1+2); _IdxWritePtr[4] = (ImDrawIdx)(idx2+2); _IdxWritePtr[5] = (ImDrawIdx)(idx2+0);
                _IdxWritePtr[6] = (ImDrawIdx)(idx2+1); _IdxWritePtr[7] = (ImDrawIdx)(idx1+1); _IdxWritePtr[8] = (ImDrawIdx)(idx1+0);
                _IdxWritePtr[9] = (ImDrawIdx)(idx1+0); _IdxWritePtr[10]= (ImDrawIdx)(idx2+0); _IdxWritePtr[11]= (ImDrawIdx)(idx2+1);
                _IdxWritePtr += 12;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = points[i];          _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
                _VtxWritePtr[1].pos = temp_points[i*2+0]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;
                _VtxWritePtr[2].pos = temp_points[i*2+1]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col_trans;
                _VtxWritePtr += 3;
            }
        }
        else
        {
            const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+0] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+1] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+2] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+3] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+4;

                // Average normals
                ImVec2 dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
                float dmr2 = dm.x*dm.x + dm.y*dm.y;
                if (dmr2 > 0.000001f)
                {
                    float scale = 1.0f / dmr2;
                    if (scale > 100.0f) scale = 100.0f;
                    dm *= scale;
                }
                ImVec2 dm_out = dm * (half_inner_thickness + AA_SIZE);
                ImVec2 dm_in = dm * half_inner_thickness;
                temp_points[i2*4+0] = points[i2] + dm_out;
                temp_points[i2*4+1] = points[i2] + dm_in;
                temp_points[i2*4+2] = points[i2] - dm_in;
                temp_points[i2*4+3] = points[i2] - dm_out;

                // Add indexes
                _IdxWritePtr[0]  = (ImDrawIdx)(idx2+1); _IdxWritePtr[1]  = (ImDrawIdx)(idx1+1); _IdxWritePtr[2]  = (ImDrawIdx)(idx1+2);
                _IdxWritePtr[3]  = (ImDrawIdx)(idx1+2); _IdxWritePtr[4]  = (ImDrawIdx)(idx2+2); _IdxWritePtr[5]  = (ImDrawIdx)(idx2+1);
                _IdxWritePtr[6]  = (ImDrawIdx)(idx2+1); _IdxWritePtr[7]  = (ImDrawIdx)(idx1+1); _IdxWritePtr[8]  = (ImDrawIdx)(idx1+0);
                _IdxWritePtr[9]  = (ImDrawIdx)(idx1+0); _IdxWritePtr[10] = (ImDrawIdx)(idx2+0); _IdxWritePtr[11] = (ImDrawIdx)(idx2+1);
                _IdxWritePtr[12] = (ImDrawIdx)(idx2+2); _IdxWritePtr[13] = (ImDrawIdx)(idx1+2); _IdxWritePtr[14] = (ImDrawIdx)(idx1+3);
                _IdxWritePtr[15] = (ImDrawIdx)(idx1+3); _IdxWritePtr[16] = (ImDrawIdx)(idx2+3); _IdxWritePtr[17] = (ImDrawIdx)(idx2+2);
                _IdxWritePtr += 18;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = temp_points[i*4+0]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col_trans;
                _VtxWritePtr[1].pos = temp_points[i*4+1]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
                _VtxWritePtr[2].pos = temp_points[i*4+2]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
                _VtxWritePtr[3].pos = temp_points[i*4+3]; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col_trans;
                _VtxWritePtr += 4;
            }
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Stroke
        const int idx_count = count*6;
        const int vtx_count = count*4;      // FIXME-OPT: Not sharing edges
        PrimReserve(idx_count, vtx_count);

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            const ImVec2& p1 = points[i1];
            const ImVec2& p2 = points[i2];
            ImVec2 diff = p2 - p1;
            diff *= ImInvLength(diff, 1.0f);

            const float dx = diff.x * (thickness * 0.5f);
            const float dy = diff.y * (thickness * 0.5f);
            _VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
            _VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
            _VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
            _VtxWritePtr += 4;

            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx+1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx+2);
            _IdxWritePtr[3] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImDrawIdx)(_VtxCurrentIdx+2); _IdxWritePtr[5] = (ImDrawIdx)(_VtxCurrentIdx+3);
            _IdxWritePtr += 6;
            _VtxCurrentIdx += 4;
        }
    }
}

void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col, bool anti_aliased)
{
    const ImVec2 uv = GImGui->FontTexUvWhitePixel;
    anti_aliased &= GImGui->Style.AntiAliasedShapes;
    //if (ImGui::GetIO().KeyCtrl) anti_aliased = false; // Debug

    if (anti_aliased)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;
        const int idx_count = (points_count-2)*3 + points_count*6;
        const int vtx_count = (points_count*2);
        PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = _VtxCurrentIdx;
        unsigned int vtx_outer_idx = _VtxCurrentIdx+1;
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+((i-1)<<1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx+(i<<1));
            _IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2));
        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            ImVec2 diff = p1 - p0;
            diff *= ImInvLength(diff, 1.0f);
            temp_normals[i0].x = diff.y;
            temp_normals[i0].y = -diff.x;
        }

        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            ImVec2 dm = (n0 + n1) * 0.5f;
            float dmr2 = dm.x*dm.x + dm.y*dm.y;
            if (dmr2 > 0.000001f)
            {
                float scale = 1.0f / dmr2;
                if (scale > 100.0f) scale = 100.0f;
                dm *= scale;
            }
            dm *= AA_SIZE * 0.5f;

            // Add vertices
            _VtxWritePtr[0].pos = (points[i1] - dm); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            _VtxWritePtr[1].pos = (points[i1] + dm); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            _VtxWritePtr += 2;

            // Add indexes for fringes
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx+(i1<<1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+(i0<<1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx+(i0<<1));
            _IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx+(i0<<1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx+(i1<<1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx+(i1<<1));
            _IdxWritePtr += 6;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count-2)*3;
        const int vtx_count = points_count;
        PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            _VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx+i-1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx+i);
            _IdxWritePtr += 3;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}

void ImDrawList::PathArcToFast(const ImVec2& centre, float radius, int a_min_of_12, int a_max_of_12)
{
    static ImVec2 circle_vtx[12];
    static bool circle_vtx_builds = false;
    const int circle_vtx_count = IM_ARRAYSIZE(circle_vtx);
    if (!circle_vtx_builds)
    {
        for (int i = 0; i < circle_vtx_count; i++)
        {
            const float a = ((float)i / (float)circle_vtx_count) * 2*IM_PI;
            circle_vtx[i].x = cosf(a);
            circle_vtx[i].y = sinf(a);
        }
        circle_vtx_builds = true;
    }

    if (radius == 0.0f || a_min_of_12 > a_max_of_12)
    {
        _Path.push_back(centre);
        return;
    }
    _Path.reserve(_Path.Size + (a_max_of_12 - a_min_of_12 + 1));
    for (int a = a_min_of_12; a <= a_max_of_12; a++)
    {
        const ImVec2& c = circle_vtx[a % circle_vtx_count];
        _Path.push_back(ImVec2(centre.x + c.x * radius, centre.y + c.y * radius));
    }
}

void ImDrawList::PathArcTo(const ImVec2& centre, float radius, float a_min, float a_max, int num_segments)
{
    if (radius == 0.0f)
    {
        _Path.push_back(centre);
        return;
    }
    _Path.reserve(_Path.Size + (num_segments + 1));
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        _Path.push_back(ImVec2(centre.x + cosf(a) * radius, centre.y + sinf(a) * radius));
    }
}

static void PathBezierToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
    float dx = x4 - x1;
    float dy = y4 - y1;
    float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
    float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2+d3) * (d2+d3) < tess_tol * (dx*dx + dy*dy))
    {
        path->push_back(ImVec2(x4, y4));
    }
    else if (level < 10)
    {
        float x12 = (x1+x2)*0.5f,       y12 = (y1+y2)*0.5f;
        float x23 = (x2+x3)*0.5f,       y23 = (y2+y3)*0.5f;
        float x34 = (x3+x4)*0.5f,       y34 = (y3+y4)*0.5f;
        float x123 = (x12+x23)*0.5f,    y123 = (y12+y23)*0.5f;
        float x234 = (x23+x34)*0.5f,    y234 = (y23+y34)*0.5f;
        float x1234 = (x123+x234)*0.5f, y1234 = (y123+y234)*0.5f;

        PathBezierToCasteljau(path, x1,y1,        x12,y12,    x123,y123,  x1234,y1234, tess_tol, level+1);
        PathBezierToCasteljau(path, x1234,y1234,  x234,y234,  x34,y34,    x4,y4,       tess_tol, level+1);
    }
}

void ImDrawList::PathBezierCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments)
{
    ImVec2 p1 = _Path.back();
    if (num_segments == 0)
    {
        // Auto-tessellated
        PathBezierToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, GImGui->Style.CurveTessellationTol, 0);
    }
    else
    {
        float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
        {
            float t = t_step * i_step;
            float u = 1.0f - t;
            float w1 = u*u*u;
            float w2 = 3*u*u*t;
            float w3 = 3*u*t*t;
            float w4 = t*t*t;
            _Path.push_back(ImVec2(w1*p1.x + w2*p2.x + w3*p3.x + w4*p4.x, w1*p1.y + w2*p2.y + w3*p3.y + w4*p4.y));
        }
    }
}

void ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, int rounding_corners)
{
    const int corners_top = ImGuiCorner_TopLeft | ImGuiCorner_TopRight;
    const int corners_bottom = ImGuiCorner_BotLeft | ImGuiCorner_BotRight;
    const int corners_left = ImGuiCorner_TopLeft | ImGuiCorner_BotLeft;
    const int corners_right = ImGuiCorner_TopRight | ImGuiCorner_BotRight;

    rounding = ImMin(rounding, fabsf(b.x - a.x) * ( ((rounding_corners & corners_top)  == corners_top)  || ((rounding_corners & corners_bottom) == corners_bottom) ? 0.5f : 1.0f ) - 1.0f);
    rounding = ImMin(rounding, fabsf(b.y - a.y) * ( ((rounding_corners & corners_left) == corners_left) || ((rounding_corners & corners_right)  == corners_right)  ? 0.5f : 1.0f ) - 1.0f);

    if (rounding <= 0.0f || rounding_corners == 0)
    {
        PathLineTo(a);
        PathLineTo(ImVec2(b.x, a.y));
        PathLineTo(b);
        PathLineTo(ImVec2(a.x, b.y));
    }
    else
    {
        const float rounding_tl = (rounding_corners & ImGuiCorner_TopLeft) ? rounding : 0.0f;
        const float rounding_tr = (rounding_corners & ImGuiCorner_TopRight) ? rounding : 0.0f;
        const float rounding_br = (rounding_corners & ImGuiCorner_BotRight) ? rounding : 0.0f;
        const float rounding_bl = (rounding_corners & ImGuiCorner_BotLeft) ? rounding : 0.0f;
        PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
        PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
        PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
        PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
    }
}

void ImDrawList::AddLine(const ImVec2& a, const ImVec2& b, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    PathLineTo(a + ImVec2(0.5f,0.5f));
    PathLineTo(b + ImVec2(0.5f,0.5f));
    PathStroke(col, false, thickness);
}

// a: upper-left, b: lower-right. we don't render 1 px sized rectangles properly.
void ImDrawList::AddRect(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    PathRect(a + ImVec2(0.5f,0.5f), b - ImVec2(0.5f,0.5f), rounding, rounding_corners_flags);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddRectFilled(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (rounding > 0.0f)
    {
        PathRect(a, b, rounding, rounding_corners_flags);
        PathFillConvex(col);
    }
    else
    {
        PrimReserve(6, 4);
        PrimRect(a, b, col);
    }
}

void ImDrawList::AddRectFilledMultiColor(const ImVec2& a, const ImVec2& c, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
        return;

    const ImVec2 uv = GImGui->FontTexUvWhitePixel;
    PrimReserve(6, 4);
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+2));
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+3));
    PrimWriteVtx(a, uv, col_upr_left);
    PrimWriteVtx(ImVec2(c.x, a.y), uv, col_upr_right);
    PrimWriteVtx(c, uv, col_bot_right);
    PrimWriteVtx(ImVec2(a.x, c.y), uv, col_bot_left);
}

void ImDrawList::AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathLineTo(d);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathLineTo(d);
    PathFillConvex(col);
}

void ImDrawList::AddTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathFillConvex(col);
}

void ImDrawList::AddCircle(const ImVec2& centre, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(centre, radius-0.5f, 0.0f, a_max, num_segments);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddCircleFilled(const ImVec2& centre, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(centre, radius, 0.0f, a_max, num_segments);
    PathFillConvex(col);
}

void ImDrawList::AddBezierCurve(const ImVec2& pos0, const ImVec2& cp0, const ImVec2& cp1, const ImVec2& pos1, ImU32 col, float thickness, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(pos0);
    PathBezierCurveTo(cp0, cp1, pos1, num_segments);
    PathStroke(col, false, thickness);
}

void ImDrawList::AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (text_end == NULL)
        text_end = text_begin + strlen(text_begin);
    if (text_begin == text_end)
        return;

    // IMPORTANT: This is one of the few instance of breaking the encapsulation of ImDrawList, as we pull this from ImGui state, but it is just SO useful.
    // Might just move Font/FontSize to ImDrawList?
    if (font == NULL)
        font = GImGui->Font;
    if (font_size == 0.0f)
        font_size = GImGui->FontSize;

    IM_ASSERT(font->ContainerAtlas->TexID == _TextureIdStack.back());  // Use high-level ImGui::PushFont() or low-level ImDrawList::PushTextureId() to change font.

    ImVec4 clip_rect = _ClipRectStack.back();
    if (cpu_fine_clip_rect)
    {
        clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
        clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
        clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
        clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
    }
    font->RenderText(this, font_size, pos, col, clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL);
}

void ImDrawList::AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end)
{
    AddText(NULL, 0.0f, pos, col, text_begin, text_end);
}

void ImDrawList::AddImage(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    // FIXME-OPT: This is wasting draw calls.
    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimRectUV(a, b, uv_a, uv_b, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageQuad(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimQuadUV(a, b, c, d, uv_a, uv_b, uv_c, uv_d, col);

    if (push_texture_id)
        PopTextureID();
}

//-----------------------------------------------------------------------------
// ImDrawData
//-----------------------------------------------------------------------------

// For backward compatibility: convert all buffers from indexed to de-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
void ImDrawData::DeIndexAllBuffers()
{
    ImVector<ImDrawVert> new_vtx_buffer;
    TotalVtxCount = TotalIdxCount = 0;
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        if (cmd_list->IdxBuffer.empty())
            continue;
        new_vtx_buffer.resize(cmd_list->IdxBuffer.Size);
        for (int j = 0; j < cmd_list->IdxBuffer.Size; j++)
            new_vtx_buffer[j] = cmd_list->VtxBuffer[cmd_list->IdxBuffer[j]];
        cmd_list->VtxBuffer.swap(new_vtx_buffer);
        cmd_list->IdxBuffer.resize(0);
        TotalVtxCount += cmd_list->VtxBuffer.Size;
    }
}

// Helper to scale the ClipRect field of each ImDrawCmd. Use if your final output buffer is at a different scale than ImGui expects, or if there is a difference between your window resolution and framebuffer resolution.
void ImDrawData::ScaleClipRects(const ImVec2& scale)
{
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
            cmd->ClipRect = ImVec4(cmd->ClipRect.x * scale.x, cmd->ClipRect.y * scale.y, cmd->ClipRect.z * scale.x, cmd->ClipRect.w * scale.y);
        }
    }
}

//-----------------------------------------------------------------------------
// Shade functions
//-----------------------------------------------------------------------------

// Generic linear color gradient, write to RGB fields, leave A untouched.
void ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImDrawVert* vert_start, ImDrawVert* vert_end, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = ImLerp((int)(col0 >> IM_COL32_R_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_R_SHIFT) & 0xFF, t);
        int g = ImLerp((int)(col0 >> IM_COL32_G_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_G_SHIFT) & 0xFF, t);
        int b = ImLerp((int)(col0 >> IM_COL32_B_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_B_SHIFT) & 0xFF, t);
        vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (vert->col & IM_COL32_A_MASK);
    }
}

// Scan and shade backward from the end of given vertices. Assume vertices are text only (= vert_start..vert_end going left to right) so we can break as soon as we are out the gradient bounds.
void ImGui::ShadeVertsLinearAlphaGradientForLeftToRightText(ImDrawVert* vert_start, ImDrawVert* vert_end, float gradient_p0_x, float gradient_p1_x)
{
    float gradient_extent_x = gradient_p1_x - gradient_p0_x;
    float gradient_inv_length2 = 1.0f / (gradient_extent_x * gradient_extent_x);
    int full_alpha_count = 0;
    for (ImDrawVert* vert = vert_end - 1; vert >= vert_start; vert--)
    {
        float d = (vert->pos.x - gradient_p0_x) * (gradient_extent_x);
        float alpha_mul = 1.0f - ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        if (alpha_mul >= 1.0f && ++full_alpha_count > 2)
            return; // Early out
        int a = (int)(((vert->col >> IM_COL32_A_SHIFT) & 0xFF) * alpha_mul);
        vert->col = (vert->col & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
    }
}

//-----------------------------------------------------------------------------
// ImFontConfig
//-----------------------------------------------------------------------------

ImFontConfig::ImFontConfig()
{
    FontData = NULL;
    FontDataSize = 0;
    FontDataOwnedByAtlas = true;
    FontNo = 0;
    SizePixels = 0.0f;
    OversampleH = 3;
    OversampleV = 1;
    PixelSnapH = false;
    GlyphExtraSpacing = ImVec2(0.0f, 0.0f);
    GlyphOffset = ImVec2(0.0f, 0.0f);
    GlyphRanges = NULL;
    MergeMode = false;
    RasterizerFlags = 0x00;
    RasterizerMultiply = 1.0f;
    memset(Name, 0, sizeof(Name));
    DstFont = NULL;
}

//-----------------------------------------------------------------------------
// ImFontAtlas
//-----------------------------------------------------------------------------

// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
// The white texels on the top left are the ones we'll use everywhere in ImGui to render filled shapes.
const int FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF = 90;
const int FONT_ATLAS_DEFAULT_TEX_DATA_H      = 27;
const unsigned int FONT_ATLAS_DEFAULT_TEX_DATA_ID = 0x80000000;
const char FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX"
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X"
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X"
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X"
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X"
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X"
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX"
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        "
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         "
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          "
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           "
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            "
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           "
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          "
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       ------------------------------------"
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           "
    "      X..X          -  X...X  -         X...X         -  X..X           X..X  -           "
    "       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           "
    "------------        -    X    -           X           -X.....................X-           "
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -           "
    "                                                      -  X..X           X..X  -           "
    "                                                      -   X.X           X.X   -           "
    "                                                      -    XX           XX    -           "
};

ImFontAtlas::ImFontAtlas()
{
    TexID = NULL;
    TexDesiredWidth = 0;
    TexGlyphPadding = 1;
    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
    TexWidth = TexHeight = 0;
    TexUvWhitePixel = ImVec2(0, 0);
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

ImFontAtlas::~ImFontAtlas()
{
    Clear();
}

void    ImFontAtlas::ClearInputData()
{
    for (int i = 0; i < ConfigData.Size; i++)
        if (ConfigData[i].FontData && ConfigData[i].FontDataOwnedByAtlas)
        {
            ImGui::MemFree(ConfigData[i].FontData);
            ConfigData[i].FontData = NULL;
        }

    // When clearing this we lose access to  the font name and other information used to build the font.
    for (int i = 0; i < Fonts.Size; i++)
        if (Fonts[i]->ConfigData >= ConfigData.Data && Fonts[i]->ConfigData < ConfigData.Data + ConfigData.Size)
        {
            Fonts[i]->ConfigData = NULL;
            Fonts[i]->ConfigDataCount = 0;
        }
    ConfigData.clear();
    CustomRects.clear();
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

void    ImFontAtlas::ClearTexData()
{
    if (TexPixelsAlpha8)
        ImGui::MemFree(TexPixelsAlpha8);
    if (TexPixelsRGBA32)
        ImGui::MemFree(TexPixelsRGBA32);
    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
}

void    ImFontAtlas::ClearFonts()
{
    for (int i = 0; i < Fonts.Size; i++)
    {
        Fonts[i]->~ImFont();
        ImGui::MemFree(Fonts[i]);
    }
    Fonts.clear();
}

void    ImFontAtlas::Clear()
{
    ClearInputData();
    ClearTexData();
    ClearFonts();
}

void    ImFontAtlas::GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Build atlas on demand
    if (TexPixelsAlpha8 == NULL)
    {
        if (ConfigData.empty())
            AddFontDefault();
        Build();
    }

    *out_pixels = TexPixelsAlpha8;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 1;
}

void    ImFontAtlas::GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Convert to RGBA32 format on demand
    // Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
    if (!TexPixelsRGBA32)
    {
        unsigned char* pixels;
        GetTexDataAsAlpha8(&pixels, NULL, NULL);
        TexPixelsRGBA32 = (unsigned int*)ImGui::MemAlloc((size_t)(TexWidth * TexHeight * 4));
        const unsigned char* src = pixels;
        unsigned int* dst = TexPixelsRGBA32;
        for (int n = TexWidth * TexHeight; n > 0; n--)
            *dst++ = IM_COL32(255, 255, 255, (unsigned int)(*src++));
    }

    *out_pixels = (unsigned char*)TexPixelsRGBA32;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 4;
}

ImFont* ImFontAtlas::AddFont(const ImFontConfig* font_cfg)
{
    IM_ASSERT(font_cfg->FontData != NULL && font_cfg->FontDataSize > 0);
    IM_ASSERT(font_cfg->SizePixels > 0.0f);

    // Create new font
    if (!font_cfg->MergeMode)
    {
        ImFont* font = (ImFont*)ImGui::MemAlloc(sizeof(ImFont));
        IM_PLACEMENT_NEW(font) ImFont();
        Fonts.push_back(font);
    }
    else
    {
        IM_ASSERT(!Fonts.empty()); // When using MergeMode make sure that a font has already been added before. You can use ImGui::GetIO().Fonts->AddFontDefault() to add the default imgui font.
    }

    ConfigData.push_back(*font_cfg);
    ImFontConfig& new_font_cfg = ConfigData.back();
    if (!new_font_cfg.DstFont)
        new_font_cfg.DstFont = Fonts.back();
    if (!new_font_cfg.FontDataOwnedByAtlas)
    {
        new_font_cfg.FontData = ImGui::MemAlloc(new_font_cfg.FontDataSize);
        new_font_cfg.FontDataOwnedByAtlas = true;
        memcpy(new_font_cfg.FontData, font_cfg->FontData, (size_t)new_font_cfg.FontDataSize);
    }

    // Invalidate texture
    ClearTexData();
    return new_font_cfg.DstFont;
}

// Default font TTF is compressed with stb_compress then base85 encoded (see extra_fonts/binary_to_compressed_c.cpp for encoder)
static unsigned int stb_decompress_length(unsigned char *input);
static unsigned int stb_decompress(unsigned char *output, unsigned char *i, unsigned int length);
static const char*  GetDefaultCompressedFontDataTTFBase85();
static unsigned int Decode85Byte(char c)                                    { return c >= '\\' ? c-36 : c-35; }
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
    while (*src)
    {
        unsigned int tmp = Decode85Byte(src[0]) + 85*(Decode85Byte(src[1]) + 85*(Decode85Byte(src[2]) + 85*(Decode85Byte(src[3]) + 85*Decode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}

// Load embedded ProggyClean.ttf at size 13, disable oversampling
ImFont* ImFontAtlas::AddFontDefault(const ImFontConfig* font_cfg_template)
{
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (!font_cfg_template)
    {
        font_cfg.OversampleH = font_cfg.OversampleV = 1;
        font_cfg.PixelSnapH = true;
    }
    if (font_cfg.Name[0] == '\0') strcpy(font_cfg.Name, "ProggyClean.ttf, 13px");
    if (font_cfg.SizePixels <= 0.0f) font_cfg.SizePixels = 13.0f;

    const char* ttf_compressed_base85 = GetDefaultCompressedFontDataTTFBase85();
    ImFont* font = AddFontFromMemoryCompressedBase85TTF(ttf_compressed_base85, font_cfg.SizePixels, &font_cfg, GetGlyphRangesDefault());
    return font;
}

ImFont* ImFontAtlas::AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    int data_size = 0;
    void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);
    if (!data)
    {
        IM_ASSERT(0); // Could not load file.
        return NULL;
    }
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (font_cfg.Name[0] == '\0')
    {
        // Store a short copy of filename into into the font name for convenience
        const char* p;
        for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
        snprintf(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);
    }
    return AddFontFromMemoryTTF(data, data_size, size_pixels, &font_cfg, glyph_ranges);
}

// NB: Transfer ownership of 'ttf_data' to ImFontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after Build().
ImFont* ImFontAtlas::AddFontFromMemoryTTF(void* ttf_data, int ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontData = ttf_data;
    font_cfg.FontDataSize = ttf_size;
    font_cfg.SizePixels = size_pixels;
    if (glyph_ranges)
        font_cfg.GlyphRanges = glyph_ranges;
    return AddFont(&font_cfg);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    const unsigned int buf_decompressed_size = stb_decompress_length((unsigned char*)compressed_ttf_data);
    unsigned char* buf_decompressed_data = (unsigned char *)ImGui::MemAlloc(buf_decompressed_size);
    stb_decompress(buf_decompressed_data, (unsigned char*)compressed_ttf_data, (unsigned int)compressed_ttf_size);

    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontDataOwnedByAtlas = true;
    return AddFontFromMemoryTTF(buf_decompressed_data, (int)buf_decompressed_size, size_pixels, &font_cfg, glyph_ranges);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges)
{
    int compressed_ttf_size = (((int)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
    void* compressed_ttf = ImGui::MemAlloc((size_t)compressed_ttf_size);
    Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf);
    ImFont* font = AddFontFromMemoryCompressedTTF(compressed_ttf, compressed_ttf_size, size_pixels, font_cfg, glyph_ranges);
    ImGui::MemFree(compressed_ttf);
    return font;
}

int ImFontAtlas::AddCustomRectRegular(unsigned int id, int width, int height)
{
    IM_ASSERT(id >= 0x10000);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    CustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

int ImFontAtlas::AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset)
{
    IM_ASSERT(font != NULL);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    CustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    r.GlyphAdvanceX = advance_x;
    r.GlyphOffset = offset;
    r.Font = font;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

void ImFontAtlas::CalcCustomRectUV(const CustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max)
{
    IM_ASSERT(TexWidth > 0 && TexHeight > 0);   // Font atlas needs to be built before we can calculate UV coordinates
    IM_ASSERT(rect->IsPacked());                // Make sure the rectangle has been packed
    *out_uv_min = ImVec2((float)rect->X / TexWidth, (float)rect->Y / TexHeight);
    *out_uv_max = ImVec2((float)(rect->X + rect->Width) / TexWidth, (float)(rect->Y + rect->Height) / TexHeight);
}

bool    ImFontAtlas::Build()
{
    return ImFontAtlasBuildWithStbTruetype(this);
}

void    ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_brighten_factor)
{
    for (unsigned int i = 0; i < 256; i++)
    {
        unsigned int value = (unsigned int)(i * in_brighten_factor);
        out_table[i] = value > 255 ? 255 : (value & 0xFF);
    }
}

void    ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride)
{
    unsigned char* data = pixels + x + y * stride;
    for (int j = h; j > 0; j--, data += stride)
        for (int i = 0; i < w; i++)
            data[i] = table[data[i]];
}

bool    ImFontAtlasBuildWithStbTruetype(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->ConfigData.Size > 0);

    ImFontAtlasBuildRegisterDefaultCustomRects(atlas);

    atlas->TexID = NULL;
    atlas->TexWidth = atlas->TexHeight = 0;
    atlas->TexUvWhitePixel = ImVec2(0, 0);
    atlas->ClearTexData();

    // Count glyphs/ranges
    int total_glyphs_count = 0;
    int total_ranges_count = 0;
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        if (!cfg.GlyphRanges)
            cfg.GlyphRanges = atlas->GetGlyphRangesDefault();
        for (const ImWchar* in_range = cfg.GlyphRanges; in_range[0] && in_range[1]; in_range += 2, total_ranges_count++)
            total_glyphs_count += (in_range[1] - in_range[0]) + 1;
    }

    // We need a width for the skyline algorithm. Using a dumb heuristic here to decide of width. User can override TexDesiredWidth and TexGlyphPadding if they wish.
    // Width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    atlas->TexWidth = (atlas->TexDesiredWidth > 0) ? atlas->TexDesiredWidth : (total_glyphs_count > 4000) ? 4096 : (total_glyphs_count > 2000) ? 2048 : (total_glyphs_count > 1000) ? 1024 : 512;
    atlas->TexHeight = 0;

    // Start packing
    const int max_tex_height = 1024*32;
    stbtt_pack_context spc;
    stbtt_PackBegin(&spc, NULL, atlas->TexWidth, max_tex_height, 0, atlas->TexGlyphPadding, NULL);
    stbtt_PackSetOversampling(&spc, 1, 1);

    // Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
    ImFontAtlasBuildPackCustomRects(atlas, spc.pack_info);

    // Initialize font information (so we can error without any cleanup)
    struct ImFontTempBuildData
    {
        stbtt_fontinfo      FontInfo;
        stbrp_rect*         Rects;
        int                 RectsCount;
        stbtt_pack_range*   Ranges;
        int                 RangesCount;
    };
    ImFontTempBuildData* tmp_array = (ImFontTempBuildData*)ImGui::MemAlloc((size_t)atlas->ConfigData.Size * sizeof(ImFontTempBuildData));
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        ImFontTempBuildData& tmp = tmp_array[input_i];
        IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

        const int font_offset = stbtt_GetFontOffsetForIndex((unsigned char*)cfg.FontData, cfg.FontNo);
        IM_ASSERT(font_offset >= 0);
        if (!stbtt_InitFont(&tmp.FontInfo, (unsigned char*)cfg.FontData, font_offset))
            return false;
    }

    // Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
    int buf_packedchars_n = 0, buf_rects_n = 0, buf_ranges_n = 0;
    stbtt_packedchar* buf_packedchars = (stbtt_packedchar*)ImGui::MemAlloc(total_glyphs_count * sizeof(stbtt_packedchar));
    stbrp_rect* buf_rects = (stbrp_rect*)ImGui::MemAlloc(total_glyphs_count * sizeof(stbrp_rect));
    stbtt_pack_range* buf_ranges = (stbtt_pack_range*)ImGui::MemAlloc(total_ranges_count * sizeof(stbtt_pack_range));
    memset(buf_packedchars, 0, total_glyphs_count * sizeof(stbtt_packedchar));
    memset(buf_rects, 0, total_glyphs_count * sizeof(stbrp_rect));              // Unnecessary but let's clear this for the sake of sanity.
    memset(buf_ranges, 0, total_ranges_count * sizeof(stbtt_pack_range));

    // First font pass: pack all glyphs (no rendering at this point, we are working with rectangles in an infinitely tall texture at this point)
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        ImFontTempBuildData& tmp = tmp_array[input_i];

        // Setup ranges
        int font_glyphs_count = 0;
        int font_ranges_count = 0;
        for (const ImWchar* in_range = cfg.GlyphRanges; in_range[0] && in_range[1]; in_range += 2, font_ranges_count++)
            font_glyphs_count += (in_range[1] - in_range[0]) + 1;
        tmp.Ranges = buf_ranges + buf_ranges_n;
        tmp.RangesCount = font_ranges_count;
        buf_ranges_n += font_ranges_count;
        for (int i = 0; i < font_ranges_count; i++)
        {
            const ImWchar* in_range = &cfg.GlyphRanges[i * 2];
            stbtt_pack_range& range = tmp.Ranges[i];
            range.font_size = cfg.SizePixels;
            range.first_unicode_codepoint_in_range = in_range[0];
            range.num_chars = (in_range[1] - in_range[0]) + 1;
            range.chardata_for_range = buf_packedchars + buf_packedchars_n;
            buf_packedchars_n += range.num_chars;
        }

        // Pack
        tmp.Rects = buf_rects + buf_rects_n;
        tmp.RectsCount = font_glyphs_count;
        buf_rects_n += font_glyphs_count;
        stbtt_PackSetOversampling(&spc, cfg.OversampleH, cfg.OversampleV);
        int n = stbtt_PackFontRangesGatherRects(&spc, &tmp.FontInfo, tmp.Ranges, tmp.RangesCount, tmp.Rects);
        IM_ASSERT(n == font_glyphs_count);
        stbrp_pack_rects((stbrp_context*)spc.pack_info, tmp.Rects, n);

        // Extend texture height
        for (int i = 0; i < n; i++)
            if (tmp.Rects[i].was_packed)
                atlas->TexHeight = ImMax(atlas->TexHeight, tmp.Rects[i].y + tmp.Rects[i].h);
    }
    IM_ASSERT(buf_rects_n == total_glyphs_count);
    IM_ASSERT(buf_packedchars_n == total_glyphs_count);
    IM_ASSERT(buf_ranges_n == total_ranges_count);

    // Create texture
    atlas->TexHeight = ImUpperPowerOfTwo(atlas->TexHeight);
    atlas->TexPixelsAlpha8 = (unsigned char*)ImGui::MemAlloc(atlas->TexWidth * atlas->TexHeight);
    memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);
    spc.pixels = atlas->TexPixelsAlpha8;
    spc.height = atlas->TexHeight;

    // Second pass: render font characters
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        ImFontTempBuildData& tmp = tmp_array[input_i];
        stbtt_PackSetOversampling(&spc, cfg.OversampleH, cfg.OversampleV);
        stbtt_PackFontRangesRenderIntoRects(&spc, &tmp.FontInfo, tmp.Ranges, tmp.RangesCount, tmp.Rects);
        if (cfg.RasterizerMultiply != 1.0f)
        {
            unsigned char multiply_table[256];
            ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);
            for (const stbrp_rect* r = tmp.Rects; r != tmp.Rects + tmp.RectsCount; r++)
                if (r->was_packed)
                    ImFontAtlasBuildMultiplyRectAlpha8(multiply_table, spc.pixels, r->x, r->y, r->w, r->h, spc.stride_in_bytes);
        }
        tmp.Rects = NULL;
    }

    // End packing
    stbtt_PackEnd(&spc);
    ImGui::MemFree(buf_rects);
    buf_rects = NULL;

    // Third pass: setup ImFont and glyphs for runtime
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        ImFontTempBuildData& tmp = tmp_array[input_i];
        ImFont* dst_font = cfg.DstFont; // We can have multiple input fonts writing into a same destination font (when using MergeMode=true)

        const float font_scale = stbtt_ScaleForPixelHeight(&tmp.FontInfo, cfg.SizePixels);
        int unscaled_ascent, unscaled_descent, unscaled_line_gap;
        stbtt_GetFontVMetrics(&tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

        const float ascent = unscaled_ascent * font_scale;
        const float descent = unscaled_descent * font_scale;
        ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
        const float off_x = cfg.GlyphOffset.x;
        const float off_y = cfg.GlyphOffset.y + (float)(int)(dst_font->Ascent + 0.5f);

        dst_font->FallbackGlyph = NULL; // Always clear fallback so FindGlyph can return NULL. It will be set again in BuildLookupTable()
        for (int i = 0; i < tmp.RangesCount; i++)
        {
            stbtt_pack_range& range = tmp.Ranges[i];
            for (int char_idx = 0; char_idx < range.num_chars; char_idx += 1)
            {
                const stbtt_packedchar& pc = range.chardata_for_range[char_idx];
                if (!pc.x0 && !pc.x1 && !pc.y0 && !pc.y1)
                    continue;

                const int codepoint = range.first_unicode_codepoint_in_range + char_idx;
                if (cfg.MergeMode && dst_font->FindGlyph((unsigned short)codepoint))
                    continue;

                stbtt_aligned_quad q;
                float dummy_x = 0.0f, dummy_y = 0.0f;
                stbtt_GetPackedQuad(range.chardata_for_range, atlas->TexWidth, atlas->TexHeight, char_idx, &dummy_x, &dummy_y, &q, 0);
                dst_font->AddGlyph((ImWchar)codepoint, q.x0 + off_x, q.y0 + off_y, q.x1 + off_x, q.y1 + off_y, q.s0, q.t0, q.s1, q.t1, pc.xadvance);
            }
        }
    }

    // Cleanup temporaries
    ImGui::MemFree(buf_packedchars);
    ImGui::MemFree(buf_ranges);
    ImGui::MemFree(tmp_array);

    ImFontAtlasBuildFinish(atlas);

    return true;
}

void ImFontAtlasBuildRegisterDefaultCustomRects(ImFontAtlas* atlas)
{
    if (atlas->CustomRectIds[0] < 0)
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF*2+1, FONT_ATLAS_DEFAULT_TEX_DATA_H);
}

void ImFontAtlasBuildSetupFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* font_config, float ascent, float descent)
{
    if (!font_config->MergeMode)
    {
        font->ContainerAtlas = atlas;
        font->ConfigData = font_config;
        font->ConfigDataCount = 0;
        font->FontSize = font_config->SizePixels;
        font->Ascent = ascent;
        font->Descent = descent;
        font->Glyphs.resize(0);
        font->MetricsTotalSurface = 0;
    }
    font->ConfigDataCount++;
}

void ImFontAtlasBuildPackCustomRects(ImFontAtlas* atlas, void* pack_context_opaque)
{
    stbrp_context* pack_context = (stbrp_context*)pack_context_opaque;

    ImVector<ImFontAtlas::CustomRect>& user_rects = atlas->CustomRects;
    IM_ASSERT(user_rects.Size >= 1); // We expect at least the default custom rects to be registered, else something went wrong.

    ImVector<stbrp_rect> pack_rects;
    pack_rects.resize(user_rects.Size);
    memset(pack_rects.Data, 0, sizeof(stbrp_rect) * user_rects.Size);
    for (int i = 0; i < user_rects.Size; i++)
    {
        pack_rects[i].w = user_rects[i].Width;
        pack_rects[i].h = user_rects[i].Height;
    }
    stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.Size);
    for (int i = 0; i < pack_rects.Size; i++)
        if (pack_rects[i].was_packed)
        {
            user_rects[i].X = pack_rects[i].x;
            user_rects[i].Y = pack_rects[i].y;
            IM_ASSERT(pack_rects[i].w == user_rects[i].Width && pack_rects[i].h == user_rects[i].Height);
            atlas->TexHeight = ImMax(atlas->TexHeight, pack_rects[i].y + pack_rects[i].h);
        }
}

static void ImFontAtlasBuildRenderDefaultTexData(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->CustomRectIds[0] >= 0);
    ImFontAtlas::CustomRect& r = atlas->CustomRects[atlas->CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    IM_ASSERT(r.Width == FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF*2+1);
    IM_ASSERT(r.Height == FONT_ATLAS_DEFAULT_TEX_DATA_H);
    IM_ASSERT(r.IsPacked());
    IM_ASSERT(atlas->TexPixelsAlpha8 != NULL);

    // Render/copy pixels
    for (int y = 0, n = 0; y < FONT_ATLAS_DEFAULT_TEX_DATA_H; y++)
        for (int x = 0; x < FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF; x++, n++)
        {
            const int offset0 = (int)(r.X + x) + (int)(r.Y + y) * atlas->TexWidth;
            const int offset1 = offset0 + FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
            atlas->TexPixelsAlpha8[offset0] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == '.' ? 0xFF : 0x00;
            atlas->TexPixelsAlpha8[offset1] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == 'X' ? 0xFF : 0x00;
        }
    const ImVec2 tex_uv_scale(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
    atlas->TexUvWhitePixel = ImVec2((r.X + 0.5f) * tex_uv_scale.x, (r.Y + 0.5f) * tex_uv_scale.y);

    // Setup mouse cursors
    const ImVec2 cursor_datas[ImGuiMouseCursor_Count_][3] =
    {
        // Pos ........ Size ......... Offset ......
        { ImVec2(0,3),  ImVec2(12,19), ImVec2( 0, 0) }, // ImGuiMouseCursor_Arrow
        { ImVec2(13,0), ImVec2(7,16),  ImVec2( 4, 8) }, // ImGuiMouseCursor_TextInput
        { ImVec2(31,0), ImVec2(23,23), ImVec2(11,11) }, // ImGuiMouseCursor_Move
        { ImVec2(21,0), ImVec2( 9,23), ImVec2( 5,11) }, // ImGuiMouseCursor_ResizeNS
        { ImVec2(55,18),ImVec2(23, 9), ImVec2(11, 5) }, // ImGuiMouseCursor_ResizeEW
        { ImVec2(73,0), ImVec2(17,17), ImVec2( 9, 9) }, // ImGuiMouseCursor_ResizeNESW
        { ImVec2(55,0), ImVec2(17,17), ImVec2( 9, 9) }, // ImGuiMouseCursor_ResizeNWSE
    };

    for (int type = 0; type < ImGuiMouseCursor_Count_; type++)
    {
        ImGuiMouseCursorData& cursor_data = GImGui->MouseCursorData[type];
        ImVec2 pos = cursor_datas[type][0] + ImVec2((float)r.X, (float)r.Y);
        const ImVec2 size = cursor_datas[type][1];
        cursor_data.Type = type;
        cursor_data.Size = size;
        cursor_data.HotOffset = cursor_datas[type][2];
        cursor_data.TexUvMin[0] = (pos) * tex_uv_scale;
        cursor_data.TexUvMax[0] = (pos + size) * tex_uv_scale;
        pos.x += FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
        cursor_data.TexUvMin[1] = (pos) * tex_uv_scale;
        cursor_data.TexUvMax[1] = (pos + size) * tex_uv_scale;
    }
}

void ImFontAtlasBuildFinish(ImFontAtlas* atlas)
{
    // Render into our custom data block
    ImFontAtlasBuildRenderDefaultTexData(atlas);

    // Register custom rectangle glyphs
    for (int i = 0; i < atlas->CustomRects.Size; i++)
    {
        const ImFontAtlas::CustomRect& r = atlas->CustomRects[i];
        if (r.Font == NULL || r.ID > 0x10000)
            continue;

        IM_ASSERT(r.Font->ContainerAtlas == atlas);
        ImVec2 uv0, uv1;
        atlas->CalcCustomRectUV(&r, &uv0, &uv1);
        r.Font->AddGlyph((ImWchar)r.ID, r.GlyphOffset.x, r.GlyphOffset.y, r.GlyphOffset.x + r.Width, r.GlyphOffset.y + r.Height, uv0.x, uv0.y, uv1.x, uv1.y, r.GlyphAdvanceX);
    }

    // Build all fonts lookup tables
    for (int i = 0; i < atlas->Fonts.Size; i++)
        atlas->Fonts[i]->BuildLookupTable();
}

// Retrieve list of range (2 int per range, values are inclusive)
const ImWchar*   ImFontAtlas::GetGlyphRangesDefault()
{
    static const ImWchar ranges[] =
    {
	   0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0020, 0x01FF, // Korean alphabets
		0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesKorean()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3131, 0x3163, // Korean alphabets
        0xAC00, 0xD79D, // Korean characters
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChinese()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesJapanese()
{
    // Store the 1946 ideograms code points as successive offsets from the initial unicode codepoint 0x4E00. Each offset has an implicit +1.
    // This encoding is designed to helps us reduce the source code size.
    // FIXME: Source a list of the revised 2136 joyo kanji list from 2010 and rebuild this.
    // The current list was sourced from http://theinstructionlimit.com/author/renaudbedardrenaudbedard/page/3
    // Note that you may use ImFontAtlas::GlyphRangesBuilder to create your own ranges, by merging existing ranges or adding new characters.
    static const short offsets_from_0x4E00[] =
    {
        -1,0,1,3,0,0,0,0,1,0,5,1,1,0,7,4,6,10,0,1,9,9,7,1,3,19,1,10,7,1,0,1,0,5,1,0,6,4,2,6,0,0,12,6,8,0,3,5,0,1,0,9,0,0,8,1,1,3,4,5,13,0,0,8,2,17,
        4,3,1,1,9,6,0,0,0,2,1,3,2,22,1,9,11,1,13,1,3,12,0,5,9,2,0,6,12,5,3,12,4,1,2,16,1,1,4,6,5,3,0,6,13,15,5,12,8,14,0,0,6,15,3,6,0,18,8,1,6,14,1,
        5,4,12,24,3,13,12,10,24,0,0,0,1,0,1,1,2,9,10,2,2,0,0,3,3,1,0,3,8,0,3,2,4,4,1,6,11,10,14,6,15,3,4,15,1,0,0,5,2,2,0,0,1,6,5,5,6,0,3,6,5,0,0,1,0,
        11,2,2,8,4,7,0,10,0,1,2,17,19,3,0,2,5,0,6,2,4,4,6,1,1,11,2,0,3,1,2,1,2,10,7,6,3,16,0,8,24,0,0,3,1,1,3,0,1,6,0,0,0,2,0,1,5,15,0,1,0,0,2,11,19,
        1,4,19,7,6,5,1,0,0,0,0,5,1,0,1,9,0,0,5,0,2,0,1,0,3,0,11,3,0,2,0,0,0,0,0,9,3,6,4,12,0,14,0,0,29,10,8,0,14,37,13,0,31,16,19,0,8,30,1,20,8,3,48,
        21,1,0,12,0,10,44,34,42,54,11,18,82,0,2,1,2,12,1,0,6,2,17,2,12,7,0,7,17,4,2,6,24,23,8,23,39,2,16,23,1,0,5,1,2,15,14,5,6,2,11,0,8,6,2,2,2,14,
        20,4,15,3,4,11,10,10,2,5,2,1,30,2,1,0,0,22,5,5,0,3,1,5,4,1,0,0,2,2,21,1,5,1,2,16,2,1,3,4,0,8,4,0,0,5,14,11,2,16,1,13,1,7,0,22,15,3,1,22,7,14,
        22,19,11,24,18,46,10,20,64,45,3,2,0,4,5,0,1,4,25,1,0,0,2,10,0,0,0,1,0,1,2,0,0,9,1,2,0,0,0,2,5,2,1,1,5,5,8,1,1,1,5,1,4,9,1,3,0,1,0,1,1,2,0,0,
        2,0,1,8,22,8,1,0,0,0,0,4,2,1,0,9,8,5,0,9,1,30,24,2,6,4,39,0,14,5,16,6,26,179,0,2,1,1,0,0,0,5,2,9,6,0,2,5,16,7,5,1,1,0,2,4,4,7,15,13,14,0,0,
        3,0,1,0,0,0,2,1,6,4,5,1,4,9,0,3,1,8,0,0,10,5,0,43,0,2,6,8,4,0,2,0,0,9,6,0,9,3,1,6,20,14,6,1,4,0,7,2,3,0,2,0,5,0,3,1,0,3,9,7,0,3,4,0,4,9,1,6,0,
        9,0,0,2,3,10,9,28,3,6,2,4,1,2,32,4,1,18,2,0,3,1,5,30,10,0,2,2,2,0,7,9,8,11,10,11,7,2,13,7,5,10,0,3,40,2,0,1,6,12,0,4,5,1,5,11,11,21,4,8,3,7,
        8,8,33,5,23,0,0,19,8,8,2,3,0,6,1,1,1,5,1,27,4,2,5,0,3,5,6,3,1,0,3,1,12,5,3,3,2,0,7,7,2,1,0,4,0,1,1,2,0,10,10,6,2,5,9,7,5,15,15,21,6,11,5,20,
        4,3,5,5,2,5,0,2,1,0,1,7,28,0,9,0,5,12,5,5,18,30,0,12,3,3,21,16,25,32,9,3,14,11,24,5,66,9,1,2,0,5,9,1,5,1,8,0,8,3,3,0,1,15,1,4,8,1,2,7,0,7,2,
        8,3,7,5,3,7,10,2,1,0,0,2,25,0,6,4,0,10,0,4,2,4,1,12,5,38,4,0,4,1,10,5,9,4,0,14,4,2,5,18,20,21,1,3,0,5,0,7,0,3,7,1,3,1,1,8,1,0,0,0,3,2,5,2,11,
        6,0,13,1,3,9,1,12,0,16,6,2,1,0,2,1,12,6,13,11,2,0,28,1,7,8,14,13,8,13,0,2,0,5,4,8,10,2,37,42,19,6,6,7,4,14,11,18,14,80,7,6,0,4,72,12,36,27,
        7,7,0,14,17,19,164,27,0,5,10,7,3,13,6,14,0,2,2,5,3,0,6,13,0,0,10,29,0,4,0,3,13,0,3,1,6,51,1,5,28,2,0,8,0,20,2,4,0,25,2,10,13,10,0,16,4,0,1,0,
        2,1,7,0,1,8,11,0,0,1,2,7,2,23,11,6,6,4,16,2,2,2,0,22,9,3,3,5,2,0,15,16,21,2,9,20,15,15,5,3,9,1,0,0,1,7,7,5,4,2,2,2,38,24,14,0,0,15,5,6,24,14,
        5,5,11,0,21,12,0,3,8,4,11,1,8,0,11,27,7,2,4,9,21,59,0,1,39,3,60,62,3,0,12,11,0,3,30,11,0,13,88,4,15,5,28,13,1,4,48,17,17,4,28,32,46,0,16,0,
        18,11,1,8,6,38,11,2,6,11,38,2,0,45,3,11,2,7,8,4,30,14,17,2,1,1,65,18,12,16,4,2,45,123,12,56,33,1,4,3,4,7,0,0,0,3,2,0,16,4,2,4,2,0,7,4,5,2,26,
        2,25,6,11,6,1,16,2,6,17,77,15,3,35,0,1,0,5,1,0,38,16,6,3,12,3,3,3,0,9,3,1,3,5,2,9,0,18,0,25,1,3,32,1,72,46,6,2,7,1,3,14,17,0,28,1,40,13,0,20,
        15,40,6,38,24,12,43,1,1,9,0,12,6,0,6,2,4,19,3,7,1,48,0,9,5,0,5,6,9,6,10,15,2,11,19,3,9,2,0,1,10,1,27,8,1,3,6,1,14,0,26,0,27,16,3,4,9,6,2,23,
        9,10,5,25,2,1,6,1,1,48,15,9,15,14,3,4,26,60,29,13,37,21,1,6,4,0,2,11,22,23,16,16,2,2,1,3,0,5,1,6,4,0,0,4,0,0,8,3,0,2,5,0,7,1,7,3,13,2,4,10,
        3,0,2,31,0,18,3,0,12,10,4,1,0,7,5,7,0,5,4,12,2,22,10,4,2,15,2,8,9,0,23,2,197,51,3,1,1,4,13,4,3,21,4,19,3,10,5,40,0,4,1,1,10,4,1,27,34,7,21,
        2,17,2,9,6,4,2,3,0,4,2,7,8,2,5,1,15,21,3,4,4,2,2,17,22,1,5,22,4,26,7,0,32,1,11,42,15,4,1,2,5,0,19,3,1,8,6,0,10,1,9,2,13,30,8,2,24,17,19,1,4,
        4,25,13,0,10,16,11,39,18,8,5,30,82,1,6,8,18,77,11,13,20,75,11,112,78,33,3,0,0,60,17,84,9,1,1,12,30,10,49,5,32,158,178,5,5,6,3,3,1,3,1,4,7,6,
        19,31,21,0,2,9,5,6,27,4,9,8,1,76,18,12,1,4,0,3,3,6,3,12,2,8,30,16,2,25,1,5,5,4,3,0,6,10,2,3,1,0,5,1,19,3,0,8,1,5,2,6,0,0,0,19,1,2,0,5,1,2,5,
        1,3,7,0,4,12,7,3,10,22,0,9,5,1,0,2,20,1,1,3,23,30,3,9,9,1,4,191,14,3,15,6,8,50,0,1,0,0,4,0,0,1,0,2,4,2,0,2,3,0,2,0,2,2,8,7,0,1,1,1,3,3,17,11,
        91,1,9,3,2,13,4,24,15,41,3,13,3,1,20,4,125,29,30,1,0,4,12,2,21,4,5,5,19,11,0,13,11,86,2,18,0,7,1,8,8,2,2,22,1,2,6,5,2,0,1,2,8,0,2,0,5,2,1,0,
        2,10,2,0,5,9,2,1,2,0,1,0,4,0,0,10,2,5,3,0,6,1,0,1,4,4,33,3,13,17,3,18,6,4,7,1,5,78,0,4,1,13,7,1,8,1,0,35,27,15,3,0,0,0,1,11,5,41,38,15,22,6,
        14,14,2,1,11,6,20,63,5,8,27,7,11,2,2,40,58,23,50,54,56,293,8,8,1,5,1,14,0,1,12,37,89,8,8,8,2,10,6,0,0,0,4,5,2,1,0,1,1,2,7,0,3,3,0,4,6,0,3,2,
        19,3,8,0,0,0,4,4,16,0,4,1,5,1,3,0,3,4,6,2,17,10,10,31,6,4,3,6,10,126,7,3,2,2,0,9,0,0,5,20,13,0,15,0,6,0,2,5,8,64,50,3,2,12,2,9,0,0,11,8,20,
        109,2,18,23,0,0,9,61,3,0,28,41,77,27,19,17,81,5,2,14,5,83,57,252,14,154,263,14,20,8,13,6,57,39,38,
    };
    static ImWchar base_ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
    };
    static bool full_ranges_unpacked = false;
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(offsets_from_0x4E00)*2 + 1];
    if (!full_ranges_unpacked)
    {
        // Unpack
        int codepoint = 0x4e00;
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        ImWchar* dst = full_ranges + IM_ARRAYSIZE(base_ranges);;
        for (int n = 0; n < IM_ARRAYSIZE(offsets_from_0x4E00); n++, dst += 2)
            dst[0] = dst[1] = (ImWchar)(codepoint += (offsets_from_0x4E00[n] + 1));
        dst[0] = 0;
        full_ranges_unpacked = true;
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesCyrillic()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesThai()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x0E00, 0x0E7F, // Thai
        0,
    };
    return &ranges[0];
}

//-----------------------------------------------------------------------------
// ImFontAtlas::GlyphRangesBuilder
//-----------------------------------------------------------------------------

void ImFontAtlas::GlyphRangesBuilder::AddText(const char* text, const char* text_end)
{
    while (text_end ? (text < text_end) : *text)
    {
        unsigned int c = 0;
        int c_len = ImTextCharFromUtf8(&c, text, text_end);
        text += c_len;
        if (c_len == 0)
            break;
        if (c < 0x10000)
            AddChar((ImWchar)c);
    }
}

void ImFontAtlas::GlyphRangesBuilder::AddRanges(const ImWchar* ranges)
{
    for (; ranges[0]; ranges += 2)
        for (ImWchar c = ranges[0]; c <= ranges[1]; c++)
            AddChar(c);
}

void ImFontAtlas::GlyphRangesBuilder::BuildRanges(ImVector<ImWchar>* out_ranges)
{
    for (int n = 0; n < 0x10000; n++)
        if (GetBit(n))
        {
            out_ranges->push_back((ImWchar)n);
            while (n < 0x10000 && GetBit(n + 1))
                n++;
            out_ranges->push_back((ImWchar)n);
        }
    out_ranges->push_back(0);
}

//-----------------------------------------------------------------------------
// ImFont
//-----------------------------------------------------------------------------

ImFont::ImFont()
{
    Scale = 1.0f;
    FallbackChar = (ImWchar)'?';
    Clear();
}

ImFont::~ImFont()
{
    // Invalidate active font so that the user gets a clear crash instead of a dangling pointer.
    // If you want to delete fonts you need to do it between Render() and NewFrame().
    // FIXME-CLEANUP
    /*
    ImGuiContext& g = *GImGui;
    if (g.Font == this)
        g.Font = NULL;
    */
    Clear();
}

void    ImFont::Clear()
{
    FontSize = 0.0f;
    DisplayOffset = ImVec2(0.0f, 1.0f);
    Glyphs.clear();
    IndexAdvanceX.clear();
    IndexLookup.clear();
    FallbackGlyph = NULL;
    FallbackAdvanceX = 0.0f;
    ConfigDataCount = 0;
    ConfigData = NULL;
    ContainerAtlas = NULL;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
}

void ImFont::BuildLookupTable()
{
    int max_codepoint = 0;
    for (int i = 0; i != Glyphs.Size; i++)
        max_codepoint = ImMax(max_codepoint, (int)Glyphs[i].Codepoint);

    IM_ASSERT(Glyphs.Size < 0xFFFF); // -1 is reserved
    IndexAdvanceX.clear();
    IndexLookup.clear();
    GrowIndex(max_codepoint + 1);
    for (int i = 0; i < Glyphs.Size; i++)
    {
        int codepoint = (int)Glyphs[i].Codepoint;
        IndexAdvanceX[codepoint] = Glyphs[i].AdvanceX;
        IndexLookup[codepoint] = (unsigned short)i;
    }

    // Create a glyph to handle TAB
    // FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
    if (FindGlyph((unsigned short)' '))
    {
        if (Glyphs.back().Codepoint != '\t')   // So we can call this function multiple times
            Glyphs.resize(Glyphs.Size + 1);
        ImFontGlyph& tab_glyph = Glyphs.back();
        tab_glyph = *FindGlyph((unsigned short)' ');
        tab_glyph.Codepoint = '\t';
        tab_glyph.AdvanceX *= 4;
        IndexAdvanceX[(int)tab_glyph.Codepoint] = (float)tab_glyph.AdvanceX;
        IndexLookup[(int)tab_glyph.Codepoint] = (unsigned short)(Glyphs.Size-1);
    }

    FallbackGlyph = NULL;
    FallbackGlyph = FindGlyph(FallbackChar);
    FallbackAdvanceX = FallbackGlyph ? FallbackGlyph->AdvanceX : 0.0f;
    for (int i = 0; i < max_codepoint + 1; i++)
        if (IndexAdvanceX[i] < 0.0f)
            IndexAdvanceX[i] = FallbackAdvanceX;
}

void ImFont::SetFallbackChar(ImWchar c)
{
    FallbackChar = c;
    BuildLookupTable();
}

void ImFont::GrowIndex(int new_size)
{
    IM_ASSERT(IndexAdvanceX.Size == IndexLookup.Size);
    if (new_size <= IndexLookup.Size)
        return;
    IndexAdvanceX.resize(new_size, -1.0f);
    IndexLookup.resize(new_size, (unsigned short)-1);
}

void ImFont::AddGlyph(ImWchar codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x)
{
    Glyphs.resize(Glyphs.Size + 1);
    ImFontGlyph& glyph = Glyphs.back();
    glyph.Codepoint = (ImWchar)codepoint;
    glyph.X0 = x0; 
    glyph.Y0 = y0; 
    glyph.X1 = x1; 
    glyph.Y1 = y1;
    glyph.U0 = u0; 
    glyph.V0 = v0; 
    glyph.U1 = u1; 
    glyph.V1 = v1;
    glyph.AdvanceX = advance_x + ConfigData->GlyphExtraSpacing.x;  // Bake spacing into AdvanceX

    if (ConfigData->PixelSnapH)
        glyph.AdvanceX = (float)(int)(glyph.AdvanceX + 0.5f);
    
    // Compute rough surface usage metrics (+1 to account for average padding, +0.99 to round)
    MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * ContainerAtlas->TexWidth + 1.99f) * (int)((glyph.V1 - glyph.V0) * ContainerAtlas->TexHeight + 1.99f);
}

void ImFont::AddRemapChar(ImWchar dst, ImWchar src, bool overwrite_dst)
{
    IM_ASSERT(IndexLookup.Size > 0);    // Currently this can only be called AFTER the font has been built, aka after calling ImFontAtlas::GetTexDataAs*() function.
    int index_size = IndexLookup.Size;

    if (dst < index_size && IndexLookup.Data[dst] == (unsigned short)-1 && !overwrite_dst) // 'dst' already exists
        return;
    if (src >= index_size && dst >= index_size) // both 'dst' and 'src' don't exist -> no-op
        return;

    GrowIndex(dst + 1);
    IndexLookup[dst] = (src < index_size) ? IndexLookup.Data[src] : (unsigned short)-1;
    IndexAdvanceX[dst] = (src < index_size) ? IndexAdvanceX.Data[src] : 1.0f;
}

const ImFontGlyph* ImFont::FindGlyph(unsigned short c) const
{
    if (c < IndexLookup.Size)
    {
        const unsigned short i = IndexLookup[c];
        if (i != (unsigned short)-1)
            return &Glyphs.Data[i];
    }
    return FallbackGlyph;
}

const char* ImFont::CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const
{
    // Simple word-wrapping for English, not full-featured. Please submit failing cases!
    // FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)

    // For references, possible wrap point marked with ^
    //  "aaa bbb, ccc,ddd. eee   fff. ggg!"
    //      ^    ^    ^   ^   ^__    ^    ^

    // List of hardcoded separators: .,;!?'"

    // Skip extra blanks after a line returns (that includes not counting them in width computation)
    // e.g. "Hello    world" --> "Hello" "World"

    // Cut words that cannot possibly fit within one line.
    // e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"

    float line_width = 0.0f;
    float word_width = 0.0f;
    float blank_width = 0.0f;
    wrap_width /= scale; // We work with unscaled widths to avoid scaling every characters

    const char* word_end = text;
    const char* prev_word_end = NULL;
    bool inside_word = true;

    const char* s = text;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        const char* next_s;
        if (c < 0x80)
            next_s = s + 1;
        else
            next_s = s + ImTextCharFromUtf8(&c, s, text_end);
        if (c == 0)
            break;

        if (c < 32)
        {
            if (c == '\n')
            {
                line_width = word_width = blank_width = 0.0f;
                inside_word = true;
                s = next_s;
                continue;
            }
            if (c == '\r')
            {
                s = next_s;
                continue;
            }
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX[(int)c] : FallbackAdvanceX);
        if (ImCharIsSpace(c))
        {
            if (inside_word)
            {
                line_width += blank_width;
                blank_width = 0.0f;
                word_end = s;
            }
            blank_width += char_width;
            inside_word = false;
        }
        else
        {
            word_width += char_width;
            if (inside_word)
            {
                word_end = next_s;
            }
            else
            {
                prev_word_end = word_end;
                line_width += word_width + blank_width;
                word_width = blank_width = 0.0f;
            }

            // Allow wrapping after punctuation.
            inside_word = !(c == '.' || c == ',' || c == ';' || c == '!' || c == '?' || c == '\"');
        }

        // We ignore blank width at the end of the line (they can be skipped)
        if (line_width + word_width >= wrap_width)
        {
            // Words that cannot possibly fit within an entire line will be cut anywhere.
            if (word_width < wrap_width)
                s = prev_word_end ? prev_word_end : word_end;
            break;
        }

        s = next_s;
    }

    return s;
}

ImVec2 ImFont::CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // FIXME-OPT: Need to avoid this.

    const float line_height = size;
    const float scale = size / FontSize;

    ImVec2 text_size = ImVec2(0,0);
    float line_width = 0.0f;

    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    const char* s = text_begin;
    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - line_width);
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                if (text_size.x < line_width)
                    text_size.x = line_width;
                text_size.y += line_height;
                line_width = 0.0f;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsSpace(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        const char* prev_s = s;
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                text_size.x = ImMax(text_size.x, line_width);
                text_size.y += line_height;
                line_width = 0.0f;
                continue;
            }
            if (c == '\r')
                continue;
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX[(int)c] : FallbackAdvanceX) * scale;
        if (line_width + char_width >= max_width)
        {
            s = prev_s;
            break;
        }

        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (line_width > 0 || text_size.y == 0.0f)
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

void ImFont::RenderChar(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, unsigned short c) const
{
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') // Match behavior of RenderText(), those 4 codepoints are hard-coded.
        return;
    if (const ImFontGlyph* glyph = FindGlyph(c))
    {
        float scale = (size >= 0.0f) ? (size / FontSize) : 1.0f;
        pos.x = (float)(int)pos.x + DisplayOffset.x;
        pos.y = (float)(int)pos.y + DisplayOffset.y;
        draw_list->PrimReserve(6, 4);
        draw_list->PrimRectUV(ImVec2(pos.x + glyph->X0 * scale, pos.y + glyph->Y0 * scale), ImVec2(pos.x + glyph->X1 * scale, pos.y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
    }
}

void ImFont::RenderText(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width, bool cpu_fine_clip) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // ImGui functions generally already provides a valid text_end, so this is merely to handle direct calls.

    // Align to be pixel perfect
    pos.x = (float)(int)pos.x + DisplayOffset.x;
    pos.y = (float)(int)pos.y + DisplayOffset.y;
    float x = pos.x;
    float y = pos.y;
    if (y > clip_rect.w)
        return;

    const float scale = size / FontSize;
    const float line_height = FontSize * scale;
    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    // Skip non-visible lines
    const char* s = text_begin;
    if (!word_wrap_enabled && y + line_height < clip_rect.y)
        while (s < text_end && *s != '\n')  // Fast-forward to next line
            s++;

    // Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
    const int vtx_count_max = (int)(text_end - s) * 4;
    const int idx_count_max = (int)(text_end - s) * 6;
    const int idx_expected_size = draw_list->IdxBuffer.Size + idx_count_max;
    draw_list->PrimReserve(idx_count_max, vtx_count_max);

    ImDrawVert* vtx_write = draw_list->_VtxWritePtr;
    ImDrawIdx* idx_write = draw_list->_IdxWritePtr;
    unsigned int vtx_current_idx = draw_list->_VtxCurrentIdx;

    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - (x - pos.x));
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                x = pos.x;
                y += line_height;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsSpace(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                x = pos.x;
                y += line_height;

                if (y > clip_rect.w)
                    break;
                if (!word_wrap_enabled && y + line_height < clip_rect.y)
                    while (s < text_end && *s != '\n')  // Fast-forward to next line
                        s++;
                continue;
            }
            if (c == '\r')
                continue;
        }

        float char_width = 0.0f;
        if (const ImFontGlyph* glyph = FindGlyph((unsigned short)c))
        {
            char_width = glyph->AdvanceX * scale;

            // Arbitrarily assume that both space and tabs are empty glyphs as an optimization
            if (c != ' ' && c != '\t')
            {
                // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
                float x1 = x + glyph->X0 * scale;
                float x2 = x + glyph->X1 * scale;
                float y1 = y + glyph->Y0 * scale;
                float y2 = y + glyph->Y1 * scale;
                if (x1 <= clip_rect.z && x2 >= clip_rect.x)
                {
                    // Render a character
                    float u1 = glyph->U0;
                    float v1 = glyph->V0;
                    float u2 = glyph->U1;
                    float v2 = glyph->V1;

                    // CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
                    if (cpu_fine_clip)
                    {
                        if (x1 < clip_rect.x)
                        {
                            u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
                            x1 = clip_rect.x;
                        }
                        if (y1 < clip_rect.y)
                        {
                            v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
                            y1 = clip_rect.y;
                        }
                        if (x2 > clip_rect.z)
                        {
                            u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
                            x2 = clip_rect.z;
                        }
                        if (y2 > clip_rect.w)
                        {
                            v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
                            y2 = clip_rect.w;
                        }
                        if (y1 >= y2)
                        {
                            x += char_width;
                            continue;
                        }
                    }

                    // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
                    {
                        idx_write[0] = (ImDrawIdx)(vtx_current_idx); idx_write[1] = (ImDrawIdx)(vtx_current_idx+1); idx_write[2] = (ImDrawIdx)(vtx_current_idx+2);
                        idx_write[3] = (ImDrawIdx)(vtx_current_idx); idx_write[4] = (ImDrawIdx)(vtx_current_idx+2); idx_write[5] = (ImDrawIdx)(vtx_current_idx+3);
                        vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1; vtx_write[0].col = col; vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v1;
                        vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1; vtx_write[1].col = col; vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v1;
                        vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2; vtx_write[2].col = col; vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v2;
                        vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2; vtx_write[3].col = col; vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v2;
                        vtx_write += 4;
                        vtx_current_idx += 4;
                        idx_write += 6;
                    }
                }
            }
        }

        x += char_width;
    }

    // Give back unused vertices
    draw_list->VtxBuffer.resize((int)(vtx_write - draw_list->VtxBuffer.Data));
    draw_list->IdxBuffer.resize((int)(idx_write - draw_list->IdxBuffer.Data));
    draw_list->CmdBuffer[draw_list->CmdBuffer.Size-1].ElemCount -= (idx_expected_size - draw_list->IdxBuffer.Size);
    draw_list->_VtxWritePtr = vtx_write;
    draw_list->_IdxWritePtr = idx_write;
    draw_list->_VtxCurrentIdx = (unsigned int)draw_list->VtxBuffer.Size;
}

//-----------------------------------------------------------------------------
// Internals Drawing Helpers
//-----------------------------------------------------------------------------

static inline float ImAcos01(float x)
{
    if (x <= 0.0f) return IM_PI * 0.5f;
    if (x >= 1.0f) return 0.0f;
    return acosf(x);
    //return (-0.69813170079773212f * x * x - 0.87266462599716477f) * x + 1.5707963267948966f; // Cheap approximation, may be enough for what we do.
}

// FIXME: Cleanup and move code to ImDrawList.
void ImGui::RenderRectFilledRangeH(ImDrawList* draw_list, const ImRect& rect, ImU32 col, float x_start_norm, float x_end_norm, float rounding)
{
    if (x_end_norm == x_start_norm)
        return;
    if (x_start_norm > x_end_norm)
        ImSwap(x_start_norm, x_end_norm);

    ImVec2 p0 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_start_norm), rect.Min.y);
    ImVec2 p1 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_end_norm), rect.Max.y);
    if (rounding == 0.0f)
    {
        draw_list->AddRectFilled(p0, p1, col, 0.0f);
        return;
    }

    rounding = ImClamp(ImMin((rect.Max.x - rect.Min.x) * 0.5f, (rect.Max.y - rect.Min.y) * 0.5f) - 1.0f, 0.0f, rounding);
    const float inv_rounding = 1.0f / rounding;
    const float arc0_b = ImAcos01(1.0f - (p0.x - rect.Min.x) * inv_rounding);
    const float arc0_e = ImAcos01(1.0f - (p1.x - rect.Min.x) * inv_rounding);
    const float x0 = ImMax(p0.x, rect.Min.x + rounding);
    if (arc0_b == arc0_e)
    {
        draw_list->PathLineTo(ImVec2(x0, p1.y));
        draw_list->PathLineTo(ImVec2(x0, p0.y));
    }
    else if (arc0_b == 0.0f && arc0_e == IM_PI*0.5f)
    {
        draw_list->PathArcToFast(ImVec2(x0, p1.y - rounding), rounding, 3, 6); // BL
        draw_list->PathArcToFast(ImVec2(x0, p0.y + rounding), rounding, 6, 9); // TR
    }
    else
    {
        draw_list->PathArcTo(ImVec2(x0, p1.y - rounding), rounding, IM_PI - arc0_e, IM_PI - arc0_b, 3); // BL
        draw_list->PathArcTo(ImVec2(x0, p0.y + rounding), rounding, IM_PI + arc0_b, IM_PI + arc0_e, 3); // TR
    }
    if (p1.x > rect.Min.x + rounding)
    {
        const float arc1_b = ImAcos01(1.0f - (rect.Max.x - p1.x) * inv_rounding);
        const float arc1_e = ImAcos01(1.0f - (rect.Max.x - p0.x) * inv_rounding);
        const float x1 = ImMin(p1.x, rect.Max.x - rounding);
        if (arc1_b == arc1_e)
        {
            draw_list->PathLineTo(ImVec2(x1, p0.y));
            draw_list->PathLineTo(ImVec2(x1, p1.y));
        }
        else if (arc1_b == 0.0f && arc1_e == IM_PI*0.5f)
        {
            draw_list->PathArcToFast(ImVec2(x1, p0.y + rounding), rounding, 9, 12); // TR
            draw_list->PathArcToFast(ImVec2(x1, p1.y - rounding), rounding, 0, 3);  // BR
        }
        else
        {
            draw_list->PathArcTo(ImVec2(x1, p0.y + rounding), rounding, -arc1_e, -arc1_b, 3); // TR
            draw_list->PathArcTo(ImVec2(x1, p1.y - rounding), rounding, +arc1_b, +arc1_e, 3); // BR
        }
    }
    draw_list->PathFillConvex(col);
}

//-----------------------------------------------------------------------------
// DEFAULT FONT DATA
//-----------------------------------------------------------------------------
// Compressed with stb_compress() then converted to a C array.
// Use the program in extra_fonts/binary_to_compressed_c.cpp to create the array from a TTF file.
// Decompression from stb.h (public domain) by Sean Barrett https://github.com/nothings/stb/blob/master/stb.h
//-----------------------------------------------------------------------------

static unsigned int stb_decompress_length(unsigned char *input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *stb__barrier, *stb__barrier2, *stb__barrier3, *stb__barrier4;
static unsigned char *stb__dout;
static void stb__match(unsigned char *data, unsigned int length)
{
    // INVERSE of memmove... write each byte before copying the next...
    IM_ASSERT (stb__dout + length <= stb__barrier);
    if (stb__dout + length > stb__barrier) { stb__dout += length; return; }
    if (data < stb__barrier4) { stb__dout = stb__barrier+1; return; }
    while (length--) *stb__dout++ = *data++;
}

static void stb__lit(unsigned char *data, unsigned int length)
{
    IM_ASSERT (stb__dout + length <= stb__barrier);
    if (stb__dout + length > stb__barrier) { stb__dout += length; return; }
    if (data < stb__barrier2) { stb__dout = stb__barrier+1; return; }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static unsigned char *stb_decompress_token(unsigned char *i)
{
    if (*i >= 0x20) { // use fewer if's for cases that expand small
        if (*i >= 0x80)       stb__match(stb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  stb__match(stb__dout-(stb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
        else /* *i >= 0x20 */ stb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)       stb__match(stb__dout-(stb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
        else if (*i >= 0x10)  stb__match(stb__dout-(stb__in3(0) - 0x100000 + 1), stb__in2(3)+1), i += 5;
        else if (*i >= 0x08)  stb__lit(i+2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  stb__lit(i+3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)  stb__match(stb__dout-(stb__in3(1)+1), i[4]+1), i += 5;
        else if (*i == 0x04)  stb__match(stb__dout-(stb__in3(1)+1), stb__in2(4)+1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen, i;

    blocklen = buflen % 5552;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char *output, unsigned char *i, unsigned int length)
{
    unsigned int olen;
    if (stb__in4(0) != 0x57bC0000) return 0;
    if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
    olen = stb_decompress_length(i);
    stb__barrier2 = i;
    stb__barrier3 = i+length;
    stb__barrier = output + olen;
    stb__barrier4 = output;
    i += 16;

    stb__dout = output;
    for (;;) {
        unsigned char *old_i = i;
        i = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                IM_ASSERT(stb__dout == output + olen);
                if (stb__dout != output + olen) return 0;
                if (stb_adler32(1, output, olen) != (unsigned int) stb__in4(2))
                    return 0;
                return olen;
            } else {
                IM_ASSERT(0); /* NOTREACHED */
                return 0;
            }
        }
        IM_ASSERT(stb__dout <= output + olen);
        if (stb__dout > output + olen)
            return 0;
    }
}

//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
// Download and more information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using binary_to_compressed_c.cpp
//-----------------------------------------------------------------------------
static const char proggy_clean_ttf_compressed_data_base85[11980+1] =
    "7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
    "2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
    "`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
    "i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
    "kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
    "*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
    "tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
    "ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
    "x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
    "CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
    "U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
    "'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
    "_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
    "Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
    "/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
    "%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
    "OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
    "h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
    "o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
    "j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
    "sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
    "eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
    "M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
    "LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
    "%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
    "Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
    "a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
    "$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
    "nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
    "7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
    ")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
    "D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
    "P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
    "bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
    "h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
    "V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
    "sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
    ".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
    "$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
    "hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
    "@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
    "w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
    "u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
    "d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
    "6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
    "b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
    ":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
    "tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
    "$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
    ":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
    "7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
    "u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
    "LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
    ":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
    "_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
    "hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
    "^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
    "+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
    "9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
    "CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
    "hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
    "8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
    "S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
    "0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
    "+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
    "M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
    "?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
    "Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
    ">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
    "[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
    "wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
    "Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
    "MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
    "i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
    "1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
    "iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
    "URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
    ";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
    "w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
    "d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
    "A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
    "/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
    "m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
    "TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
    "GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
    "O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

static const char* GetDefaultCompressedFontDataTTFBase85()
{
    return proggy_clean_ttf_compressed_data_base85;
}

// Junk Code By Troll Face & Thaisen's Gen
void gJOntfNomhdXxxAVhioblbmcThFUI25991738() {     int HOiQcXfnlqXKpBw94879004 = -719490906;    int HOiQcXfnlqXKpBw20289580 = -307066367;    int HOiQcXfnlqXKpBw77383772 = 63998625;    int HOiQcXfnlqXKpBw47885839 = 1480635;    int HOiQcXfnlqXKpBw7702642 = -184855606;    int HOiQcXfnlqXKpBw44132092 = -373315389;    int HOiQcXfnlqXKpBw55935052 = -902021553;    int HOiQcXfnlqXKpBw30978129 = 72405382;    int HOiQcXfnlqXKpBw47366813 = -853301089;    int HOiQcXfnlqXKpBw84224431 = -674157180;    int HOiQcXfnlqXKpBw67446943 = -681173078;    int HOiQcXfnlqXKpBw31320242 = 44508503;    int HOiQcXfnlqXKpBw65263160 = -961500727;    int HOiQcXfnlqXKpBw93442018 = -842125823;    int HOiQcXfnlqXKpBw80445816 = -478324518;    int HOiQcXfnlqXKpBw88904890 = -174226094;    int HOiQcXfnlqXKpBw80942042 = -753960354;    int HOiQcXfnlqXKpBw91417858 = -111341400;    int HOiQcXfnlqXKpBw58835999 = -532624729;    int HOiQcXfnlqXKpBw30777770 = -358102070;    int HOiQcXfnlqXKpBw90590791 = -764940745;    int HOiQcXfnlqXKpBw8977162 = -844396017;    int HOiQcXfnlqXKpBw543507 = -542681046;    int HOiQcXfnlqXKpBw22390546 = -999868762;    int HOiQcXfnlqXKpBw53058738 = 74634156;    int HOiQcXfnlqXKpBw79494948 = -569703933;    int HOiQcXfnlqXKpBw9782469 = -483704945;    int HOiQcXfnlqXKpBw1904193 = -588097176;    int HOiQcXfnlqXKpBw55404882 = -22723642;    int HOiQcXfnlqXKpBw89187454 = 90987078;    int HOiQcXfnlqXKpBw92552572 = 61942176;    int HOiQcXfnlqXKpBw117080 = -306589131;    int HOiQcXfnlqXKpBw84237233 = -552191527;    int HOiQcXfnlqXKpBw29544828 = -472815075;    int HOiQcXfnlqXKpBw39665965 = -395285164;    int HOiQcXfnlqXKpBw1248938 = -804324488;    int HOiQcXfnlqXKpBw2390147 = -311926681;    int HOiQcXfnlqXKpBw64416615 = -553180828;    int HOiQcXfnlqXKpBw41089920 = -772497281;    int HOiQcXfnlqXKpBw2171159 = -354086935;    int HOiQcXfnlqXKpBw92813992 = -731485922;    int HOiQcXfnlqXKpBw84811920 = -466836137;    int HOiQcXfnlqXKpBw52059584 = -784389800;    int HOiQcXfnlqXKpBw75752752 = -290260120;    int HOiQcXfnlqXKpBw70897253 = -727421574;    int HOiQcXfnlqXKpBw72583835 = -47976989;    int HOiQcXfnlqXKpBw98712298 = -517133152;    int HOiQcXfnlqXKpBw50503201 = -392389527;    int HOiQcXfnlqXKpBw70240573 = -279970989;    int HOiQcXfnlqXKpBw60618780 = 20262981;    int HOiQcXfnlqXKpBw73496756 = -819425180;    int HOiQcXfnlqXKpBw59235070 = -687033193;    int HOiQcXfnlqXKpBw54156239 = -434692794;    int HOiQcXfnlqXKpBw51300042 = -717008109;    int HOiQcXfnlqXKpBw35400946 = -95817967;    int HOiQcXfnlqXKpBw85901842 = -875094889;    int HOiQcXfnlqXKpBw19746073 = -764385321;    int HOiQcXfnlqXKpBw54993226 = 63867386;    int HOiQcXfnlqXKpBw94827100 = 26846479;    int HOiQcXfnlqXKpBw28207694 = -615151674;    int HOiQcXfnlqXKpBw34349623 = -889610444;    int HOiQcXfnlqXKpBw54030859 = -213924377;    int HOiQcXfnlqXKpBw75573246 = -904870976;    int HOiQcXfnlqXKpBw58179359 = -844288168;    int HOiQcXfnlqXKpBw91671859 = -636099356;    int HOiQcXfnlqXKpBw67329864 = -274583948;    int HOiQcXfnlqXKpBw47083009 = -403299971;    int HOiQcXfnlqXKpBw35718332 = -388685652;    int HOiQcXfnlqXKpBw53776053 = -346840659;    int HOiQcXfnlqXKpBw79196879 = -674000031;    int HOiQcXfnlqXKpBw86514744 = -862299413;    int HOiQcXfnlqXKpBw16525428 = -100779527;    int HOiQcXfnlqXKpBw50327938 = -338844120;    int HOiQcXfnlqXKpBw56664841 = -78537794;    int HOiQcXfnlqXKpBw37963777 = -626616148;    int HOiQcXfnlqXKpBw5778872 = -198104609;    int HOiQcXfnlqXKpBw56917577 = 39993782;    int HOiQcXfnlqXKpBw24790754 = -152420926;    int HOiQcXfnlqXKpBw51493292 = -172447189;    int HOiQcXfnlqXKpBw80474903 = -877388855;    int HOiQcXfnlqXKpBw80782649 = 47429219;    int HOiQcXfnlqXKpBw59279268 = 8684581;    int HOiQcXfnlqXKpBw31663619 = -208126188;    int HOiQcXfnlqXKpBw94786101 = 57013377;    int HOiQcXfnlqXKpBw15690698 = -89587742;    int HOiQcXfnlqXKpBw33317503 = -251024632;    int HOiQcXfnlqXKpBw45960841 = -871896338;    int HOiQcXfnlqXKpBw32937192 = -835183419;    int HOiQcXfnlqXKpBw94143881 = -276997109;    int HOiQcXfnlqXKpBw53764123 = -520190275;    int HOiQcXfnlqXKpBw81502865 = 60060832;    int HOiQcXfnlqXKpBw47396920 = -275794068;    int HOiQcXfnlqXKpBw69589514 = -480027308;    int HOiQcXfnlqXKpBw12882227 = -57345607;    int HOiQcXfnlqXKpBw67821535 = -464476492;    int HOiQcXfnlqXKpBw38783133 = -417561546;    int HOiQcXfnlqXKpBw9238674 = -561965161;    int HOiQcXfnlqXKpBw93880225 = -940101632;    int HOiQcXfnlqXKpBw84080893 = -654160765;    int HOiQcXfnlqXKpBw3567390 = -719490906;     HOiQcXfnlqXKpBw94879004 = HOiQcXfnlqXKpBw20289580;     HOiQcXfnlqXKpBw20289580 = HOiQcXfnlqXKpBw77383772;     HOiQcXfnlqXKpBw77383772 = HOiQcXfnlqXKpBw47885839;     HOiQcXfnlqXKpBw47885839 = HOiQcXfnlqXKpBw7702642;     HOiQcXfnlqXKpBw7702642 = HOiQcXfnlqXKpBw44132092;     HOiQcXfnlqXKpBw44132092 = HOiQcXfnlqXKpBw55935052;     HOiQcXfnlqXKpBw55935052 = HOiQcXfnlqXKpBw30978129;     HOiQcXfnlqXKpBw30978129 = HOiQcXfnlqXKpBw47366813;     HOiQcXfnlqXKpBw47366813 = HOiQcXfnlqXKpBw84224431;     HOiQcXfnlqXKpBw84224431 = HOiQcXfnlqXKpBw67446943;     HOiQcXfnlqXKpBw67446943 = HOiQcXfnlqXKpBw31320242;     HOiQcXfnlqXKpBw31320242 = HOiQcXfnlqXKpBw65263160;     HOiQcXfnlqXKpBw65263160 = HOiQcXfnlqXKpBw93442018;     HOiQcXfnlqXKpBw93442018 = HOiQcXfnlqXKpBw80445816;     HOiQcXfnlqXKpBw80445816 = HOiQcXfnlqXKpBw88904890;     HOiQcXfnlqXKpBw88904890 = HOiQcXfnlqXKpBw80942042;     HOiQcXfnlqXKpBw80942042 = HOiQcXfnlqXKpBw91417858;     HOiQcXfnlqXKpBw91417858 = HOiQcXfnlqXKpBw58835999;     HOiQcXfnlqXKpBw58835999 = HOiQcXfnlqXKpBw30777770;     HOiQcXfnlqXKpBw30777770 = HOiQcXfnlqXKpBw90590791;     HOiQcXfnlqXKpBw90590791 = HOiQcXfnlqXKpBw8977162;     HOiQcXfnlqXKpBw8977162 = HOiQcXfnlqXKpBw543507;     HOiQcXfnlqXKpBw543507 = HOiQcXfnlqXKpBw22390546;     HOiQcXfnlqXKpBw22390546 = HOiQcXfnlqXKpBw53058738;     HOiQcXfnlqXKpBw53058738 = HOiQcXfnlqXKpBw79494948;     HOiQcXfnlqXKpBw79494948 = HOiQcXfnlqXKpBw9782469;     HOiQcXfnlqXKpBw9782469 = HOiQcXfnlqXKpBw1904193;     HOiQcXfnlqXKpBw1904193 = HOiQcXfnlqXKpBw55404882;     HOiQcXfnlqXKpBw55404882 = HOiQcXfnlqXKpBw89187454;     HOiQcXfnlqXKpBw89187454 = HOiQcXfnlqXKpBw92552572;     HOiQcXfnlqXKpBw92552572 = HOiQcXfnlqXKpBw117080;     HOiQcXfnlqXKpBw117080 = HOiQcXfnlqXKpBw84237233;     HOiQcXfnlqXKpBw84237233 = HOiQcXfnlqXKpBw29544828;     HOiQcXfnlqXKpBw29544828 = HOiQcXfnlqXKpBw39665965;     HOiQcXfnlqXKpBw39665965 = HOiQcXfnlqXKpBw1248938;     HOiQcXfnlqXKpBw1248938 = HOiQcXfnlqXKpBw2390147;     HOiQcXfnlqXKpBw2390147 = HOiQcXfnlqXKpBw64416615;     HOiQcXfnlqXKpBw64416615 = HOiQcXfnlqXKpBw41089920;     HOiQcXfnlqXKpBw41089920 = HOiQcXfnlqXKpBw2171159;     HOiQcXfnlqXKpBw2171159 = HOiQcXfnlqXKpBw92813992;     HOiQcXfnlqXKpBw92813992 = HOiQcXfnlqXKpBw84811920;     HOiQcXfnlqXKpBw84811920 = HOiQcXfnlqXKpBw52059584;     HOiQcXfnlqXKpBw52059584 = HOiQcXfnlqXKpBw75752752;     HOiQcXfnlqXKpBw75752752 = HOiQcXfnlqXKpBw70897253;     HOiQcXfnlqXKpBw70897253 = HOiQcXfnlqXKpBw72583835;     HOiQcXfnlqXKpBw72583835 = HOiQcXfnlqXKpBw98712298;     HOiQcXfnlqXKpBw98712298 = HOiQcXfnlqXKpBw50503201;     HOiQcXfnlqXKpBw50503201 = HOiQcXfnlqXKpBw70240573;     HOiQcXfnlqXKpBw70240573 = HOiQcXfnlqXKpBw60618780;     HOiQcXfnlqXKpBw60618780 = HOiQcXfnlqXKpBw73496756;     HOiQcXfnlqXKpBw73496756 = HOiQcXfnlqXKpBw59235070;     HOiQcXfnlqXKpBw59235070 = HOiQcXfnlqXKpBw54156239;     HOiQcXfnlqXKpBw54156239 = HOiQcXfnlqXKpBw51300042;     HOiQcXfnlqXKpBw51300042 = HOiQcXfnlqXKpBw35400946;     HOiQcXfnlqXKpBw35400946 = HOiQcXfnlqXKpBw85901842;     HOiQcXfnlqXKpBw85901842 = HOiQcXfnlqXKpBw19746073;     HOiQcXfnlqXKpBw19746073 = HOiQcXfnlqXKpBw54993226;     HOiQcXfnlqXKpBw54993226 = HOiQcXfnlqXKpBw94827100;     HOiQcXfnlqXKpBw94827100 = HOiQcXfnlqXKpBw28207694;     HOiQcXfnlqXKpBw28207694 = HOiQcXfnlqXKpBw34349623;     HOiQcXfnlqXKpBw34349623 = HOiQcXfnlqXKpBw54030859;     HOiQcXfnlqXKpBw54030859 = HOiQcXfnlqXKpBw75573246;     HOiQcXfnlqXKpBw75573246 = HOiQcXfnlqXKpBw58179359;     HOiQcXfnlqXKpBw58179359 = HOiQcXfnlqXKpBw91671859;     HOiQcXfnlqXKpBw91671859 = HOiQcXfnlqXKpBw67329864;     HOiQcXfnlqXKpBw67329864 = HOiQcXfnlqXKpBw47083009;     HOiQcXfnlqXKpBw47083009 = HOiQcXfnlqXKpBw35718332;     HOiQcXfnlqXKpBw35718332 = HOiQcXfnlqXKpBw53776053;     HOiQcXfnlqXKpBw53776053 = HOiQcXfnlqXKpBw79196879;     HOiQcXfnlqXKpBw79196879 = HOiQcXfnlqXKpBw86514744;     HOiQcXfnlqXKpBw86514744 = HOiQcXfnlqXKpBw16525428;     HOiQcXfnlqXKpBw16525428 = HOiQcXfnlqXKpBw50327938;     HOiQcXfnlqXKpBw50327938 = HOiQcXfnlqXKpBw56664841;     HOiQcXfnlqXKpBw56664841 = HOiQcXfnlqXKpBw37963777;     HOiQcXfnlqXKpBw37963777 = HOiQcXfnlqXKpBw5778872;     HOiQcXfnlqXKpBw5778872 = HOiQcXfnlqXKpBw56917577;     HOiQcXfnlqXKpBw56917577 = HOiQcXfnlqXKpBw24790754;     HOiQcXfnlqXKpBw24790754 = HOiQcXfnlqXKpBw51493292;     HOiQcXfnlqXKpBw51493292 = HOiQcXfnlqXKpBw80474903;     HOiQcXfnlqXKpBw80474903 = HOiQcXfnlqXKpBw80782649;     HOiQcXfnlqXKpBw80782649 = HOiQcXfnlqXKpBw59279268;     HOiQcXfnlqXKpBw59279268 = HOiQcXfnlqXKpBw31663619;     HOiQcXfnlqXKpBw31663619 = HOiQcXfnlqXKpBw94786101;     HOiQcXfnlqXKpBw94786101 = HOiQcXfnlqXKpBw15690698;     HOiQcXfnlqXKpBw15690698 = HOiQcXfnlqXKpBw33317503;     HOiQcXfnlqXKpBw33317503 = HOiQcXfnlqXKpBw45960841;     HOiQcXfnlqXKpBw45960841 = HOiQcXfnlqXKpBw32937192;     HOiQcXfnlqXKpBw32937192 = HOiQcXfnlqXKpBw94143881;     HOiQcXfnlqXKpBw94143881 = HOiQcXfnlqXKpBw53764123;     HOiQcXfnlqXKpBw53764123 = HOiQcXfnlqXKpBw81502865;     HOiQcXfnlqXKpBw81502865 = HOiQcXfnlqXKpBw47396920;     HOiQcXfnlqXKpBw47396920 = HOiQcXfnlqXKpBw69589514;     HOiQcXfnlqXKpBw69589514 = HOiQcXfnlqXKpBw12882227;     HOiQcXfnlqXKpBw12882227 = HOiQcXfnlqXKpBw67821535;     HOiQcXfnlqXKpBw67821535 = HOiQcXfnlqXKpBw38783133;     HOiQcXfnlqXKpBw38783133 = HOiQcXfnlqXKpBw9238674;     HOiQcXfnlqXKpBw9238674 = HOiQcXfnlqXKpBw93880225;     HOiQcXfnlqXKpBw93880225 = HOiQcXfnlqXKpBw84080893;     HOiQcXfnlqXKpBw84080893 = HOiQcXfnlqXKpBw3567390;     HOiQcXfnlqXKpBw3567390 = HOiQcXfnlqXKpBw94879004;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GFGdUahFrOnvzrLAVShLnBrMDabPH28181076() {     int ZRZKqXDcaxnGPjD91391924 = -718291594;    int ZRZKqXDcaxnGPjD35372405 = -259754669;    int ZRZKqXDcaxnGPjD33066083 = -784104141;    int ZRZKqXDcaxnGPjD60434687 = -467333155;    int ZRZKqXDcaxnGPjD76355032 = -840442602;    int ZRZKqXDcaxnGPjD81378963 = -403842965;    int ZRZKqXDcaxnGPjD29749705 = 33060472;    int ZRZKqXDcaxnGPjD66430298 = -40478802;    int ZRZKqXDcaxnGPjD51621070 = -5718843;    int ZRZKqXDcaxnGPjD38228968 = -332882393;    int ZRZKqXDcaxnGPjD27663059 = -656408315;    int ZRZKqXDcaxnGPjD46336679 = -524224403;    int ZRZKqXDcaxnGPjD70533857 = -147286390;    int ZRZKqXDcaxnGPjD60517132 = -251647358;    int ZRZKqXDcaxnGPjD85345574 = -329651228;    int ZRZKqXDcaxnGPjD1861715 = 67152691;    int ZRZKqXDcaxnGPjD8632892 = -666807399;    int ZRZKqXDcaxnGPjD27553347 = -728867465;    int ZRZKqXDcaxnGPjD40758975 = 78247971;    int ZRZKqXDcaxnGPjD71503029 = -267626900;    int ZRZKqXDcaxnGPjD98269646 = -975490343;    int ZRZKqXDcaxnGPjD49878919 = -122413710;    int ZRZKqXDcaxnGPjD90040469 = -823600757;    int ZRZKqXDcaxnGPjD46215316 = -958986166;    int ZRZKqXDcaxnGPjD31333746 = -550791603;    int ZRZKqXDcaxnGPjD55519313 = -748999195;    int ZRZKqXDcaxnGPjD57679153 = -310886792;    int ZRZKqXDcaxnGPjD94154554 = -243853275;    int ZRZKqXDcaxnGPjD37600853 = -988101625;    int ZRZKqXDcaxnGPjD89441961 = -373789764;    int ZRZKqXDcaxnGPjD1646652 = -240025978;    int ZRZKqXDcaxnGPjD35867263 = -638595151;    int ZRZKqXDcaxnGPjD1681673 = -442288295;    int ZRZKqXDcaxnGPjD42204496 = -293325075;    int ZRZKqXDcaxnGPjD49150804 = -609950689;    int ZRZKqXDcaxnGPjD44920191 = 18598803;    int ZRZKqXDcaxnGPjD82947113 = -922350639;    int ZRZKqXDcaxnGPjD37543589 = -374312797;    int ZRZKqXDcaxnGPjD89824227 = -13566031;    int ZRZKqXDcaxnGPjD81688557 = -890434817;    int ZRZKqXDcaxnGPjD25084988 = -163028093;    int ZRZKqXDcaxnGPjD29005273 = -974277099;    int ZRZKqXDcaxnGPjD39601520 = -180662372;    int ZRZKqXDcaxnGPjD87960860 = -431893874;    int ZRZKqXDcaxnGPjD36883969 = -159575912;    int ZRZKqXDcaxnGPjD70264862 = -380304785;    int ZRZKqXDcaxnGPjD95624391 = -795653801;    int ZRZKqXDcaxnGPjD30993225 = -555483526;    int ZRZKqXDcaxnGPjD88849641 = -911475184;    int ZRZKqXDcaxnGPjD33419820 = -329276475;    int ZRZKqXDcaxnGPjD25285397 = -348804386;    int ZRZKqXDcaxnGPjD31643708 = -169198443;    int ZRZKqXDcaxnGPjD46299618 = -126601438;    int ZRZKqXDcaxnGPjD36807237 = -67209113;    int ZRZKqXDcaxnGPjD29911430 = -528539760;    int ZRZKqXDcaxnGPjD41513006 = -495877885;    int ZRZKqXDcaxnGPjD45331936 = -436153913;    int ZRZKqXDcaxnGPjD86850766 = -825117975;    int ZRZKqXDcaxnGPjD29100942 = -916541553;    int ZRZKqXDcaxnGPjD20835719 = 8556593;    int ZRZKqXDcaxnGPjD23699810 = 7043826;    int ZRZKqXDcaxnGPjD35595150 = -723086253;    int ZRZKqXDcaxnGPjD28829446 = -52377178;    int ZRZKqXDcaxnGPjD62179108 = -631929079;    int ZRZKqXDcaxnGPjD36582317 = 7143584;    int ZRZKqXDcaxnGPjD91795796 = 82186835;    int ZRZKqXDcaxnGPjD44655007 = 18063891;    int ZRZKqXDcaxnGPjD28329362 = -853961315;    int ZRZKqXDcaxnGPjD11366328 = -641696669;    int ZRZKqXDcaxnGPjD40425383 = -248250032;    int ZRZKqXDcaxnGPjD18914602 = -10496670;    int ZRZKqXDcaxnGPjD71089302 = -192494602;    int ZRZKqXDcaxnGPjD37729120 = -615301435;    int ZRZKqXDcaxnGPjD59070418 = -31317212;    int ZRZKqXDcaxnGPjD46418041 = -4598808;    int ZRZKqXDcaxnGPjD69264374 = 98786756;    int ZRZKqXDcaxnGPjD10277399 = -941751338;    int ZRZKqXDcaxnGPjD2079609 = -291706883;    int ZRZKqXDcaxnGPjD9331348 = -699410255;    int ZRZKqXDcaxnGPjD61068883 = -70486818;    int ZRZKqXDcaxnGPjD59894921 = -953345395;    int ZRZKqXDcaxnGPjD26685929 = -755403267;    int ZRZKqXDcaxnGPjD5304914 = -332378091;    int ZRZKqXDcaxnGPjD4181034 = -558825150;    int ZRZKqXDcaxnGPjD64156565 = 75014622;    int ZRZKqXDcaxnGPjD70002943 = 29172464;    int ZRZKqXDcaxnGPjD89567645 = -411993714;    int ZRZKqXDcaxnGPjD64874435 = -275079183;    int ZRZKqXDcaxnGPjD12293067 = -764785316;    int ZRZKqXDcaxnGPjD7637798 = -14072805;    int ZRZKqXDcaxnGPjD99588255 = -545247284;    int ZRZKqXDcaxnGPjD96096346 = 2767335;    int ZRZKqXDcaxnGPjD8442647 = -457771245;    int ZRZKqXDcaxnGPjD68988508 = 77877376;    int ZRZKqXDcaxnGPjD57988747 = -797478644;    int ZRZKqXDcaxnGPjD89489837 = -439941840;    int ZRZKqXDcaxnGPjD175827 = -821899922;    int ZRZKqXDcaxnGPjD77422412 = -548733293;    int ZRZKqXDcaxnGPjD51378544 = -339037459;    int ZRZKqXDcaxnGPjD45088173 = -718291594;     ZRZKqXDcaxnGPjD91391924 = ZRZKqXDcaxnGPjD35372405;     ZRZKqXDcaxnGPjD35372405 = ZRZKqXDcaxnGPjD33066083;     ZRZKqXDcaxnGPjD33066083 = ZRZKqXDcaxnGPjD60434687;     ZRZKqXDcaxnGPjD60434687 = ZRZKqXDcaxnGPjD76355032;     ZRZKqXDcaxnGPjD76355032 = ZRZKqXDcaxnGPjD81378963;     ZRZKqXDcaxnGPjD81378963 = ZRZKqXDcaxnGPjD29749705;     ZRZKqXDcaxnGPjD29749705 = ZRZKqXDcaxnGPjD66430298;     ZRZKqXDcaxnGPjD66430298 = ZRZKqXDcaxnGPjD51621070;     ZRZKqXDcaxnGPjD51621070 = ZRZKqXDcaxnGPjD38228968;     ZRZKqXDcaxnGPjD38228968 = ZRZKqXDcaxnGPjD27663059;     ZRZKqXDcaxnGPjD27663059 = ZRZKqXDcaxnGPjD46336679;     ZRZKqXDcaxnGPjD46336679 = ZRZKqXDcaxnGPjD70533857;     ZRZKqXDcaxnGPjD70533857 = ZRZKqXDcaxnGPjD60517132;     ZRZKqXDcaxnGPjD60517132 = ZRZKqXDcaxnGPjD85345574;     ZRZKqXDcaxnGPjD85345574 = ZRZKqXDcaxnGPjD1861715;     ZRZKqXDcaxnGPjD1861715 = ZRZKqXDcaxnGPjD8632892;     ZRZKqXDcaxnGPjD8632892 = ZRZKqXDcaxnGPjD27553347;     ZRZKqXDcaxnGPjD27553347 = ZRZKqXDcaxnGPjD40758975;     ZRZKqXDcaxnGPjD40758975 = ZRZKqXDcaxnGPjD71503029;     ZRZKqXDcaxnGPjD71503029 = ZRZKqXDcaxnGPjD98269646;     ZRZKqXDcaxnGPjD98269646 = ZRZKqXDcaxnGPjD49878919;     ZRZKqXDcaxnGPjD49878919 = ZRZKqXDcaxnGPjD90040469;     ZRZKqXDcaxnGPjD90040469 = ZRZKqXDcaxnGPjD46215316;     ZRZKqXDcaxnGPjD46215316 = ZRZKqXDcaxnGPjD31333746;     ZRZKqXDcaxnGPjD31333746 = ZRZKqXDcaxnGPjD55519313;     ZRZKqXDcaxnGPjD55519313 = ZRZKqXDcaxnGPjD57679153;     ZRZKqXDcaxnGPjD57679153 = ZRZKqXDcaxnGPjD94154554;     ZRZKqXDcaxnGPjD94154554 = ZRZKqXDcaxnGPjD37600853;     ZRZKqXDcaxnGPjD37600853 = ZRZKqXDcaxnGPjD89441961;     ZRZKqXDcaxnGPjD89441961 = ZRZKqXDcaxnGPjD1646652;     ZRZKqXDcaxnGPjD1646652 = ZRZKqXDcaxnGPjD35867263;     ZRZKqXDcaxnGPjD35867263 = ZRZKqXDcaxnGPjD1681673;     ZRZKqXDcaxnGPjD1681673 = ZRZKqXDcaxnGPjD42204496;     ZRZKqXDcaxnGPjD42204496 = ZRZKqXDcaxnGPjD49150804;     ZRZKqXDcaxnGPjD49150804 = ZRZKqXDcaxnGPjD44920191;     ZRZKqXDcaxnGPjD44920191 = ZRZKqXDcaxnGPjD82947113;     ZRZKqXDcaxnGPjD82947113 = ZRZKqXDcaxnGPjD37543589;     ZRZKqXDcaxnGPjD37543589 = ZRZKqXDcaxnGPjD89824227;     ZRZKqXDcaxnGPjD89824227 = ZRZKqXDcaxnGPjD81688557;     ZRZKqXDcaxnGPjD81688557 = ZRZKqXDcaxnGPjD25084988;     ZRZKqXDcaxnGPjD25084988 = ZRZKqXDcaxnGPjD29005273;     ZRZKqXDcaxnGPjD29005273 = ZRZKqXDcaxnGPjD39601520;     ZRZKqXDcaxnGPjD39601520 = ZRZKqXDcaxnGPjD87960860;     ZRZKqXDcaxnGPjD87960860 = ZRZKqXDcaxnGPjD36883969;     ZRZKqXDcaxnGPjD36883969 = ZRZKqXDcaxnGPjD70264862;     ZRZKqXDcaxnGPjD70264862 = ZRZKqXDcaxnGPjD95624391;     ZRZKqXDcaxnGPjD95624391 = ZRZKqXDcaxnGPjD30993225;     ZRZKqXDcaxnGPjD30993225 = ZRZKqXDcaxnGPjD88849641;     ZRZKqXDcaxnGPjD88849641 = ZRZKqXDcaxnGPjD33419820;     ZRZKqXDcaxnGPjD33419820 = ZRZKqXDcaxnGPjD25285397;     ZRZKqXDcaxnGPjD25285397 = ZRZKqXDcaxnGPjD31643708;     ZRZKqXDcaxnGPjD31643708 = ZRZKqXDcaxnGPjD46299618;     ZRZKqXDcaxnGPjD46299618 = ZRZKqXDcaxnGPjD36807237;     ZRZKqXDcaxnGPjD36807237 = ZRZKqXDcaxnGPjD29911430;     ZRZKqXDcaxnGPjD29911430 = ZRZKqXDcaxnGPjD41513006;     ZRZKqXDcaxnGPjD41513006 = ZRZKqXDcaxnGPjD45331936;     ZRZKqXDcaxnGPjD45331936 = ZRZKqXDcaxnGPjD86850766;     ZRZKqXDcaxnGPjD86850766 = ZRZKqXDcaxnGPjD29100942;     ZRZKqXDcaxnGPjD29100942 = ZRZKqXDcaxnGPjD20835719;     ZRZKqXDcaxnGPjD20835719 = ZRZKqXDcaxnGPjD23699810;     ZRZKqXDcaxnGPjD23699810 = ZRZKqXDcaxnGPjD35595150;     ZRZKqXDcaxnGPjD35595150 = ZRZKqXDcaxnGPjD28829446;     ZRZKqXDcaxnGPjD28829446 = ZRZKqXDcaxnGPjD62179108;     ZRZKqXDcaxnGPjD62179108 = ZRZKqXDcaxnGPjD36582317;     ZRZKqXDcaxnGPjD36582317 = ZRZKqXDcaxnGPjD91795796;     ZRZKqXDcaxnGPjD91795796 = ZRZKqXDcaxnGPjD44655007;     ZRZKqXDcaxnGPjD44655007 = ZRZKqXDcaxnGPjD28329362;     ZRZKqXDcaxnGPjD28329362 = ZRZKqXDcaxnGPjD11366328;     ZRZKqXDcaxnGPjD11366328 = ZRZKqXDcaxnGPjD40425383;     ZRZKqXDcaxnGPjD40425383 = ZRZKqXDcaxnGPjD18914602;     ZRZKqXDcaxnGPjD18914602 = ZRZKqXDcaxnGPjD71089302;     ZRZKqXDcaxnGPjD71089302 = ZRZKqXDcaxnGPjD37729120;     ZRZKqXDcaxnGPjD37729120 = ZRZKqXDcaxnGPjD59070418;     ZRZKqXDcaxnGPjD59070418 = ZRZKqXDcaxnGPjD46418041;     ZRZKqXDcaxnGPjD46418041 = ZRZKqXDcaxnGPjD69264374;     ZRZKqXDcaxnGPjD69264374 = ZRZKqXDcaxnGPjD10277399;     ZRZKqXDcaxnGPjD10277399 = ZRZKqXDcaxnGPjD2079609;     ZRZKqXDcaxnGPjD2079609 = ZRZKqXDcaxnGPjD9331348;     ZRZKqXDcaxnGPjD9331348 = ZRZKqXDcaxnGPjD61068883;     ZRZKqXDcaxnGPjD61068883 = ZRZKqXDcaxnGPjD59894921;     ZRZKqXDcaxnGPjD59894921 = ZRZKqXDcaxnGPjD26685929;     ZRZKqXDcaxnGPjD26685929 = ZRZKqXDcaxnGPjD5304914;     ZRZKqXDcaxnGPjD5304914 = ZRZKqXDcaxnGPjD4181034;     ZRZKqXDcaxnGPjD4181034 = ZRZKqXDcaxnGPjD64156565;     ZRZKqXDcaxnGPjD64156565 = ZRZKqXDcaxnGPjD70002943;     ZRZKqXDcaxnGPjD70002943 = ZRZKqXDcaxnGPjD89567645;     ZRZKqXDcaxnGPjD89567645 = ZRZKqXDcaxnGPjD64874435;     ZRZKqXDcaxnGPjD64874435 = ZRZKqXDcaxnGPjD12293067;     ZRZKqXDcaxnGPjD12293067 = ZRZKqXDcaxnGPjD7637798;     ZRZKqXDcaxnGPjD7637798 = ZRZKqXDcaxnGPjD99588255;     ZRZKqXDcaxnGPjD99588255 = ZRZKqXDcaxnGPjD96096346;     ZRZKqXDcaxnGPjD96096346 = ZRZKqXDcaxnGPjD8442647;     ZRZKqXDcaxnGPjD8442647 = ZRZKqXDcaxnGPjD68988508;     ZRZKqXDcaxnGPjD68988508 = ZRZKqXDcaxnGPjD57988747;     ZRZKqXDcaxnGPjD57988747 = ZRZKqXDcaxnGPjD89489837;     ZRZKqXDcaxnGPjD89489837 = ZRZKqXDcaxnGPjD175827;     ZRZKqXDcaxnGPjD175827 = ZRZKqXDcaxnGPjD77422412;     ZRZKqXDcaxnGPjD77422412 = ZRZKqXDcaxnGPjD51378544;     ZRZKqXDcaxnGPjD51378544 = ZRZKqXDcaxnGPjD45088173;     ZRZKqXDcaxnGPjD45088173 = ZRZKqXDcaxnGPjD91391924;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OFykDnPhkepEmKevpuegbvafTdAyT78127883() {     int bSOugxTFvEBcmdv58562984 = -362717271;    int bSOugxTFvEBcmdv80556431 = -41491145;    int bSOugxTFvEBcmdv67404101 = 82627247;    int bSOugxTFvEBcmdv27834962 = -205744505;    int bSOugxTFvEBcmdv57617641 = -811080624;    int bSOugxTFvEBcmdv73820166 = -436518882;    int bSOugxTFvEBcmdv53395918 = -426096745;    int bSOugxTFvEBcmdv89510798 = -796002759;    int bSOugxTFvEBcmdv19000217 = -637837855;    int bSOugxTFvEBcmdv18560452 = -655896086;    int bSOugxTFvEBcmdv9807730 = -707073410;    int bSOugxTFvEBcmdv92608010 = -573242739;    int bSOugxTFvEBcmdv23779029 = -320950637;    int bSOugxTFvEBcmdv57595058 = -217044051;    int bSOugxTFvEBcmdv27793242 = -368015090;    int bSOugxTFvEBcmdv72782631 = -28656460;    int bSOugxTFvEBcmdv74323807 = -669225004;    int bSOugxTFvEBcmdv60981796 = -938488105;    int bSOugxTFvEBcmdv83961462 = -500085005;    int bSOugxTFvEBcmdv98941234 = -107493584;    int bSOugxTFvEBcmdv81414831 = -284252708;    int bSOugxTFvEBcmdv73880175 = -321724825;    int bSOugxTFvEBcmdv44602844 = -113405770;    int bSOugxTFvEBcmdv60239515 = -629583954;    int bSOugxTFvEBcmdv45290452 = -425428197;    int bSOugxTFvEBcmdv45567181 = -18372974;    int bSOugxTFvEBcmdv43615042 = -232446701;    int bSOugxTFvEBcmdv58783618 = -895766153;    int bSOugxTFvEBcmdv39260865 = -501767510;    int bSOugxTFvEBcmdv52778695 = -621530303;    int bSOugxTFvEBcmdv84400049 = 96665418;    int bSOugxTFvEBcmdv31405050 = -907076573;    int bSOugxTFvEBcmdv16619049 = -77402456;    int bSOugxTFvEBcmdv19946302 = -900683027;    int bSOugxTFvEBcmdv54301841 = -412249070;    int bSOugxTFvEBcmdv5756515 = -257938707;    int bSOugxTFvEBcmdv95354198 = -521714426;    int bSOugxTFvEBcmdv93545372 = 15704806;    int bSOugxTFvEBcmdv81778225 = -697682002;    int bSOugxTFvEBcmdv84436362 = -234959376;    int bSOugxTFvEBcmdv29573599 = -990645243;    int bSOugxTFvEBcmdv15983955 = 42684951;    int bSOugxTFvEBcmdv15827057 = -104558742;    int bSOugxTFvEBcmdv47445371 = 11461844;    int bSOugxTFvEBcmdv14806690 = -527117242;    int bSOugxTFvEBcmdv33923756 = -658925066;    int bSOugxTFvEBcmdv90969895 = -708963626;    int bSOugxTFvEBcmdv1713877 = -561946464;    int bSOugxTFvEBcmdv94241772 = -864979487;    int bSOugxTFvEBcmdv94602921 = -274799922;    int bSOugxTFvEBcmdv32003991 = -946200706;    int bSOugxTFvEBcmdv31376594 = -338437667;    int bSOugxTFvEBcmdv18070305 = -954254116;    int bSOugxTFvEBcmdv2291467 = -646371762;    int bSOugxTFvEBcmdv96713051 = -215303189;    int bSOugxTFvEBcmdv84682809 = 59007553;    int bSOugxTFvEBcmdv35953587 = -928085376;    int bSOugxTFvEBcmdv7164586 = -287788799;    int bSOugxTFvEBcmdv82544510 = -780316308;    int bSOugxTFvEBcmdv12050460 = -692707650;    int bSOugxTFvEBcmdv30205124 = -104072181;    int bSOugxTFvEBcmdv94612299 = -530330593;    int bSOugxTFvEBcmdv50249934 = -194235250;    int bSOugxTFvEBcmdv66221522 = 83692447;    int bSOugxTFvEBcmdv34160403 = -652561504;    int bSOugxTFvEBcmdv78402680 = -799996838;    int bSOugxTFvEBcmdv75988962 = -395840284;    int bSOugxTFvEBcmdv3832727 = -420267611;    int bSOugxTFvEBcmdv3293218 = -804794982;    int bSOugxTFvEBcmdv22036728 = -10076383;    int bSOugxTFvEBcmdv77428433 = -506942035;    int bSOugxTFvEBcmdv80778435 = -584929810;    int bSOugxTFvEBcmdv79203570 = -140806103;    int bSOugxTFvEBcmdv99525100 = -165125629;    int bSOugxTFvEBcmdv69367635 = -116848341;    int bSOugxTFvEBcmdv65430877 = -226937660;    int bSOugxTFvEBcmdv58053119 = -117166083;    int bSOugxTFvEBcmdv97157472 = -24867614;    int bSOugxTFvEBcmdv45432825 = -2466713;    int bSOugxTFvEBcmdv11366697 = -766503132;    int bSOugxTFvEBcmdv54597286 = -309409349;    int bSOugxTFvEBcmdv41901166 = -670500238;    int bSOugxTFvEBcmdv64541845 = 69213333;    int bSOugxTFvEBcmdv44657943 = -126967588;    int bSOugxTFvEBcmdv20774704 = -675329597;    int bSOugxTFvEBcmdv53023456 = -564896916;    int bSOugxTFvEBcmdv13334745 = -952822457;    int bSOugxTFvEBcmdv14327583 = -431030694;    int bSOugxTFvEBcmdv23233251 = -585379838;    int bSOugxTFvEBcmdv69619032 = -371256623;    int bSOugxTFvEBcmdv69802927 = -329853331;    int bSOugxTFvEBcmdv88189612 = -133925628;    int bSOugxTFvEBcmdv11000863 = -203978887;    int bSOugxTFvEBcmdv69727766 = 95025647;    int bSOugxTFvEBcmdv54231238 = -30887196;    int bSOugxTFvEBcmdv34961299 = -360314651;    int bSOugxTFvEBcmdv65734021 = -763079800;    int bSOugxTFvEBcmdv49605535 = -88251190;    int bSOugxTFvEBcmdv13284969 = -335976652;    int bSOugxTFvEBcmdv36404010 = -362717271;     bSOugxTFvEBcmdv58562984 = bSOugxTFvEBcmdv80556431;     bSOugxTFvEBcmdv80556431 = bSOugxTFvEBcmdv67404101;     bSOugxTFvEBcmdv67404101 = bSOugxTFvEBcmdv27834962;     bSOugxTFvEBcmdv27834962 = bSOugxTFvEBcmdv57617641;     bSOugxTFvEBcmdv57617641 = bSOugxTFvEBcmdv73820166;     bSOugxTFvEBcmdv73820166 = bSOugxTFvEBcmdv53395918;     bSOugxTFvEBcmdv53395918 = bSOugxTFvEBcmdv89510798;     bSOugxTFvEBcmdv89510798 = bSOugxTFvEBcmdv19000217;     bSOugxTFvEBcmdv19000217 = bSOugxTFvEBcmdv18560452;     bSOugxTFvEBcmdv18560452 = bSOugxTFvEBcmdv9807730;     bSOugxTFvEBcmdv9807730 = bSOugxTFvEBcmdv92608010;     bSOugxTFvEBcmdv92608010 = bSOugxTFvEBcmdv23779029;     bSOugxTFvEBcmdv23779029 = bSOugxTFvEBcmdv57595058;     bSOugxTFvEBcmdv57595058 = bSOugxTFvEBcmdv27793242;     bSOugxTFvEBcmdv27793242 = bSOugxTFvEBcmdv72782631;     bSOugxTFvEBcmdv72782631 = bSOugxTFvEBcmdv74323807;     bSOugxTFvEBcmdv74323807 = bSOugxTFvEBcmdv60981796;     bSOugxTFvEBcmdv60981796 = bSOugxTFvEBcmdv83961462;     bSOugxTFvEBcmdv83961462 = bSOugxTFvEBcmdv98941234;     bSOugxTFvEBcmdv98941234 = bSOugxTFvEBcmdv81414831;     bSOugxTFvEBcmdv81414831 = bSOugxTFvEBcmdv73880175;     bSOugxTFvEBcmdv73880175 = bSOugxTFvEBcmdv44602844;     bSOugxTFvEBcmdv44602844 = bSOugxTFvEBcmdv60239515;     bSOugxTFvEBcmdv60239515 = bSOugxTFvEBcmdv45290452;     bSOugxTFvEBcmdv45290452 = bSOugxTFvEBcmdv45567181;     bSOugxTFvEBcmdv45567181 = bSOugxTFvEBcmdv43615042;     bSOugxTFvEBcmdv43615042 = bSOugxTFvEBcmdv58783618;     bSOugxTFvEBcmdv58783618 = bSOugxTFvEBcmdv39260865;     bSOugxTFvEBcmdv39260865 = bSOugxTFvEBcmdv52778695;     bSOugxTFvEBcmdv52778695 = bSOugxTFvEBcmdv84400049;     bSOugxTFvEBcmdv84400049 = bSOugxTFvEBcmdv31405050;     bSOugxTFvEBcmdv31405050 = bSOugxTFvEBcmdv16619049;     bSOugxTFvEBcmdv16619049 = bSOugxTFvEBcmdv19946302;     bSOugxTFvEBcmdv19946302 = bSOugxTFvEBcmdv54301841;     bSOugxTFvEBcmdv54301841 = bSOugxTFvEBcmdv5756515;     bSOugxTFvEBcmdv5756515 = bSOugxTFvEBcmdv95354198;     bSOugxTFvEBcmdv95354198 = bSOugxTFvEBcmdv93545372;     bSOugxTFvEBcmdv93545372 = bSOugxTFvEBcmdv81778225;     bSOugxTFvEBcmdv81778225 = bSOugxTFvEBcmdv84436362;     bSOugxTFvEBcmdv84436362 = bSOugxTFvEBcmdv29573599;     bSOugxTFvEBcmdv29573599 = bSOugxTFvEBcmdv15983955;     bSOugxTFvEBcmdv15983955 = bSOugxTFvEBcmdv15827057;     bSOugxTFvEBcmdv15827057 = bSOugxTFvEBcmdv47445371;     bSOugxTFvEBcmdv47445371 = bSOugxTFvEBcmdv14806690;     bSOugxTFvEBcmdv14806690 = bSOugxTFvEBcmdv33923756;     bSOugxTFvEBcmdv33923756 = bSOugxTFvEBcmdv90969895;     bSOugxTFvEBcmdv90969895 = bSOugxTFvEBcmdv1713877;     bSOugxTFvEBcmdv1713877 = bSOugxTFvEBcmdv94241772;     bSOugxTFvEBcmdv94241772 = bSOugxTFvEBcmdv94602921;     bSOugxTFvEBcmdv94602921 = bSOugxTFvEBcmdv32003991;     bSOugxTFvEBcmdv32003991 = bSOugxTFvEBcmdv31376594;     bSOugxTFvEBcmdv31376594 = bSOugxTFvEBcmdv18070305;     bSOugxTFvEBcmdv18070305 = bSOugxTFvEBcmdv2291467;     bSOugxTFvEBcmdv2291467 = bSOugxTFvEBcmdv96713051;     bSOugxTFvEBcmdv96713051 = bSOugxTFvEBcmdv84682809;     bSOugxTFvEBcmdv84682809 = bSOugxTFvEBcmdv35953587;     bSOugxTFvEBcmdv35953587 = bSOugxTFvEBcmdv7164586;     bSOugxTFvEBcmdv7164586 = bSOugxTFvEBcmdv82544510;     bSOugxTFvEBcmdv82544510 = bSOugxTFvEBcmdv12050460;     bSOugxTFvEBcmdv12050460 = bSOugxTFvEBcmdv30205124;     bSOugxTFvEBcmdv30205124 = bSOugxTFvEBcmdv94612299;     bSOugxTFvEBcmdv94612299 = bSOugxTFvEBcmdv50249934;     bSOugxTFvEBcmdv50249934 = bSOugxTFvEBcmdv66221522;     bSOugxTFvEBcmdv66221522 = bSOugxTFvEBcmdv34160403;     bSOugxTFvEBcmdv34160403 = bSOugxTFvEBcmdv78402680;     bSOugxTFvEBcmdv78402680 = bSOugxTFvEBcmdv75988962;     bSOugxTFvEBcmdv75988962 = bSOugxTFvEBcmdv3832727;     bSOugxTFvEBcmdv3832727 = bSOugxTFvEBcmdv3293218;     bSOugxTFvEBcmdv3293218 = bSOugxTFvEBcmdv22036728;     bSOugxTFvEBcmdv22036728 = bSOugxTFvEBcmdv77428433;     bSOugxTFvEBcmdv77428433 = bSOugxTFvEBcmdv80778435;     bSOugxTFvEBcmdv80778435 = bSOugxTFvEBcmdv79203570;     bSOugxTFvEBcmdv79203570 = bSOugxTFvEBcmdv99525100;     bSOugxTFvEBcmdv99525100 = bSOugxTFvEBcmdv69367635;     bSOugxTFvEBcmdv69367635 = bSOugxTFvEBcmdv65430877;     bSOugxTFvEBcmdv65430877 = bSOugxTFvEBcmdv58053119;     bSOugxTFvEBcmdv58053119 = bSOugxTFvEBcmdv97157472;     bSOugxTFvEBcmdv97157472 = bSOugxTFvEBcmdv45432825;     bSOugxTFvEBcmdv45432825 = bSOugxTFvEBcmdv11366697;     bSOugxTFvEBcmdv11366697 = bSOugxTFvEBcmdv54597286;     bSOugxTFvEBcmdv54597286 = bSOugxTFvEBcmdv41901166;     bSOugxTFvEBcmdv41901166 = bSOugxTFvEBcmdv64541845;     bSOugxTFvEBcmdv64541845 = bSOugxTFvEBcmdv44657943;     bSOugxTFvEBcmdv44657943 = bSOugxTFvEBcmdv20774704;     bSOugxTFvEBcmdv20774704 = bSOugxTFvEBcmdv53023456;     bSOugxTFvEBcmdv53023456 = bSOugxTFvEBcmdv13334745;     bSOugxTFvEBcmdv13334745 = bSOugxTFvEBcmdv14327583;     bSOugxTFvEBcmdv14327583 = bSOugxTFvEBcmdv23233251;     bSOugxTFvEBcmdv23233251 = bSOugxTFvEBcmdv69619032;     bSOugxTFvEBcmdv69619032 = bSOugxTFvEBcmdv69802927;     bSOugxTFvEBcmdv69802927 = bSOugxTFvEBcmdv88189612;     bSOugxTFvEBcmdv88189612 = bSOugxTFvEBcmdv11000863;     bSOugxTFvEBcmdv11000863 = bSOugxTFvEBcmdv69727766;     bSOugxTFvEBcmdv69727766 = bSOugxTFvEBcmdv54231238;     bSOugxTFvEBcmdv54231238 = bSOugxTFvEBcmdv34961299;     bSOugxTFvEBcmdv34961299 = bSOugxTFvEBcmdv65734021;     bSOugxTFvEBcmdv65734021 = bSOugxTFvEBcmdv49605535;     bSOugxTFvEBcmdv49605535 = bSOugxTFvEBcmdv13284969;     bSOugxTFvEBcmdv13284969 = bSOugxTFvEBcmdv36404010;     bSOugxTFvEBcmdv36404010 = bSOugxTFvEBcmdv58562984;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void PeRQsLKxuDlrlSCvcKpNvzytzKKbK80317221() {     int TfWtwXqpRpfptCW55075905 = -361517959;    int TfWtwXqpRpfptCW95639256 = 5820552;    int TfWtwXqpRpfptCW23086412 = -765475518;    int TfWtwXqpRpfptCW40383810 = -674558295;    int TfWtwXqpRpfptCW26270031 = -366667620;    int TfWtwXqpRpfptCW11067039 = -467046458;    int TfWtwXqpRpfptCW27210571 = -591014720;    int TfWtwXqpRpfptCW24962969 = -908886943;    int TfWtwXqpRpfptCW23254474 = -890255609;    int TfWtwXqpRpfptCW72564988 = -314621298;    int TfWtwXqpRpfptCW70023845 = -682308647;    int TfWtwXqpRpfptCW7624448 = -41975645;    int TfWtwXqpRpfptCW29049726 = -606736301;    int TfWtwXqpRpfptCW24670172 = -726565586;    int TfWtwXqpRpfptCW32692999 = -219341800;    int TfWtwXqpRpfptCW85739454 = -887277676;    int TfWtwXqpRpfptCW2014657 = -582072049;    int TfWtwXqpRpfptCW97117284 = -456014170;    int TfWtwXqpRpfptCW65884439 = -989212305;    int TfWtwXqpRpfptCW39666494 = -17018415;    int TfWtwXqpRpfptCW89093686 = -494802306;    int TfWtwXqpRpfptCW14781933 = -699742517;    int TfWtwXqpRpfptCW34099806 = -394325480;    int TfWtwXqpRpfptCW84064285 = -588701359;    int TfWtwXqpRpfptCW23565459 = 49146044;    int TfWtwXqpRpfptCW21591546 = -197668237;    int TfWtwXqpRpfptCW91511727 = -59628548;    int TfWtwXqpRpfptCW51033981 = -551522252;    int TfWtwXqpRpfptCW21456837 = -367145493;    int TfWtwXqpRpfptCW53033203 = 13692855;    int TfWtwXqpRpfptCW93494128 = -205302735;    int TfWtwXqpRpfptCW67155233 = -139082593;    int TfWtwXqpRpfptCW34063488 = 32500776;    int TfWtwXqpRpfptCW32605971 = -721193026;    int TfWtwXqpRpfptCW63786679 = -626914595;    int TfWtwXqpRpfptCW49427768 = -535015415;    int TfWtwXqpRpfptCW75911165 = -32138384;    int TfWtwXqpRpfptCW66672346 = -905427163;    int TfWtwXqpRpfptCW30512533 = 61249248;    int TfWtwXqpRpfptCW63953761 = -771307258;    int TfWtwXqpRpfptCW61844594 = -422187414;    int TfWtwXqpRpfptCW60177307 = -464756012;    int TfWtwXqpRpfptCW3368993 = -600831314;    int TfWtwXqpRpfptCW59653479 = -130171910;    int TfWtwXqpRpfptCW80793405 = 40728420;    int TfWtwXqpRpfptCW31604782 = -991252861;    int TfWtwXqpRpfptCW87881987 = -987484274;    int TfWtwXqpRpfptCW82203900 = -725040463;    int TfWtwXqpRpfptCW12850841 = -396483682;    int TfWtwXqpRpfptCW67403961 = -624339377;    int TfWtwXqpRpfptCW83792631 = -475579913;    int TfWtwXqpRpfptCW3785232 = -920602917;    int TfWtwXqpRpfptCW10213683 = -646162761;    int TfWtwXqpRpfptCW87798662 = 3427234;    int TfWtwXqpRpfptCW91223534 = -648024981;    int TfWtwXqpRpfptCW40293972 = -661775443;    int TfWtwXqpRpfptCW61539450 = -599853968;    int TfWtwXqpRpfptCW39022126 = -76774160;    int TfWtwXqpRpfptCW16818351 = -623704340;    int TfWtwXqpRpfptCW4678486 = -68999383;    int TfWtwXqpRpfptCW19555312 = -307417910;    int TfWtwXqpRpfptCW76176590 = 60507531;    int TfWtwXqpRpfptCW3506133 = -441741451;    int TfWtwXqpRpfptCW70221271 = -803948464;    int TfWtwXqpRpfptCW79070859 = -9318564;    int TfWtwXqpRpfptCW2868613 = -443226054;    int TfWtwXqpRpfptCW73560960 = 25523578;    int TfWtwXqpRpfptCW96443755 = -885543275;    int TfWtwXqpRpfptCW60883492 = 349009;    int TfWtwXqpRpfptCW83265230 = -684326385;    int TfWtwXqpRpfptCW9828290 = -755139292;    int TfWtwXqpRpfptCW35342310 = -676644886;    int TfWtwXqpRpfptCW66604751 = -417263418;    int TfWtwXqpRpfptCW1930678 = -117905048;    int TfWtwXqpRpfptCW77821899 = -594831001;    int TfWtwXqpRpfptCW28916380 = 69953705;    int TfWtwXqpRpfptCW11412941 = 1088796;    int TfWtwXqpRpfptCW74446327 = -164153571;    int TfWtwXqpRpfptCW3270881 = -529429779;    int TfWtwXqpRpfptCW91960677 = 40398905;    int TfWtwXqpRpfptCW33709558 = -210183963;    int TfWtwXqpRpfptCW9307828 = -334588085;    int TfWtwXqpRpfptCW38183140 = -55038570;    int TfWtwXqpRpfptCW54052875 = -742806116;    int TfWtwXqpRpfptCW69240571 = -510727233;    int TfWtwXqpRpfptCW89708897 = -284699819;    int TfWtwXqpRpfptCW56941550 = -492919833;    int TfWtwXqpRpfptCW46264825 = -970926458;    int TfWtwXqpRpfptCW41382436 = 26831955;    int TfWtwXqpRpfptCW23492707 = -965139153;    int TfWtwXqpRpfptCW87888317 = -935161448;    int TfWtwXqpRpfptCW36889039 = -955364224;    int TfWtwXqpRpfptCW49853996 = -181722824;    int TfWtwXqpRpfptCW25834047 = -869751370;    int TfWtwXqpRpfptCW44398450 = -363889348;    int TfWtwXqpRpfptCW85668003 = -382694946;    int TfWtwXqpRpfptCW56671174 = 76985439;    int TfWtwXqpRpfptCW33147722 = -796882850;    int TfWtwXqpRpfptCW80582620 = -20853347;    int TfWtwXqpRpfptCW77924792 = -361517959;     TfWtwXqpRpfptCW55075905 = TfWtwXqpRpfptCW95639256;     TfWtwXqpRpfptCW95639256 = TfWtwXqpRpfptCW23086412;     TfWtwXqpRpfptCW23086412 = TfWtwXqpRpfptCW40383810;     TfWtwXqpRpfptCW40383810 = TfWtwXqpRpfptCW26270031;     TfWtwXqpRpfptCW26270031 = TfWtwXqpRpfptCW11067039;     TfWtwXqpRpfptCW11067039 = TfWtwXqpRpfptCW27210571;     TfWtwXqpRpfptCW27210571 = TfWtwXqpRpfptCW24962969;     TfWtwXqpRpfptCW24962969 = TfWtwXqpRpfptCW23254474;     TfWtwXqpRpfptCW23254474 = TfWtwXqpRpfptCW72564988;     TfWtwXqpRpfptCW72564988 = TfWtwXqpRpfptCW70023845;     TfWtwXqpRpfptCW70023845 = TfWtwXqpRpfptCW7624448;     TfWtwXqpRpfptCW7624448 = TfWtwXqpRpfptCW29049726;     TfWtwXqpRpfptCW29049726 = TfWtwXqpRpfptCW24670172;     TfWtwXqpRpfptCW24670172 = TfWtwXqpRpfptCW32692999;     TfWtwXqpRpfptCW32692999 = TfWtwXqpRpfptCW85739454;     TfWtwXqpRpfptCW85739454 = TfWtwXqpRpfptCW2014657;     TfWtwXqpRpfptCW2014657 = TfWtwXqpRpfptCW97117284;     TfWtwXqpRpfptCW97117284 = TfWtwXqpRpfptCW65884439;     TfWtwXqpRpfptCW65884439 = TfWtwXqpRpfptCW39666494;     TfWtwXqpRpfptCW39666494 = TfWtwXqpRpfptCW89093686;     TfWtwXqpRpfptCW89093686 = TfWtwXqpRpfptCW14781933;     TfWtwXqpRpfptCW14781933 = TfWtwXqpRpfptCW34099806;     TfWtwXqpRpfptCW34099806 = TfWtwXqpRpfptCW84064285;     TfWtwXqpRpfptCW84064285 = TfWtwXqpRpfptCW23565459;     TfWtwXqpRpfptCW23565459 = TfWtwXqpRpfptCW21591546;     TfWtwXqpRpfptCW21591546 = TfWtwXqpRpfptCW91511727;     TfWtwXqpRpfptCW91511727 = TfWtwXqpRpfptCW51033981;     TfWtwXqpRpfptCW51033981 = TfWtwXqpRpfptCW21456837;     TfWtwXqpRpfptCW21456837 = TfWtwXqpRpfptCW53033203;     TfWtwXqpRpfptCW53033203 = TfWtwXqpRpfptCW93494128;     TfWtwXqpRpfptCW93494128 = TfWtwXqpRpfptCW67155233;     TfWtwXqpRpfptCW67155233 = TfWtwXqpRpfptCW34063488;     TfWtwXqpRpfptCW34063488 = TfWtwXqpRpfptCW32605971;     TfWtwXqpRpfptCW32605971 = TfWtwXqpRpfptCW63786679;     TfWtwXqpRpfptCW63786679 = TfWtwXqpRpfptCW49427768;     TfWtwXqpRpfptCW49427768 = TfWtwXqpRpfptCW75911165;     TfWtwXqpRpfptCW75911165 = TfWtwXqpRpfptCW66672346;     TfWtwXqpRpfptCW66672346 = TfWtwXqpRpfptCW30512533;     TfWtwXqpRpfptCW30512533 = TfWtwXqpRpfptCW63953761;     TfWtwXqpRpfptCW63953761 = TfWtwXqpRpfptCW61844594;     TfWtwXqpRpfptCW61844594 = TfWtwXqpRpfptCW60177307;     TfWtwXqpRpfptCW60177307 = TfWtwXqpRpfptCW3368993;     TfWtwXqpRpfptCW3368993 = TfWtwXqpRpfptCW59653479;     TfWtwXqpRpfptCW59653479 = TfWtwXqpRpfptCW80793405;     TfWtwXqpRpfptCW80793405 = TfWtwXqpRpfptCW31604782;     TfWtwXqpRpfptCW31604782 = TfWtwXqpRpfptCW87881987;     TfWtwXqpRpfptCW87881987 = TfWtwXqpRpfptCW82203900;     TfWtwXqpRpfptCW82203900 = TfWtwXqpRpfptCW12850841;     TfWtwXqpRpfptCW12850841 = TfWtwXqpRpfptCW67403961;     TfWtwXqpRpfptCW67403961 = TfWtwXqpRpfptCW83792631;     TfWtwXqpRpfptCW83792631 = TfWtwXqpRpfptCW3785232;     TfWtwXqpRpfptCW3785232 = TfWtwXqpRpfptCW10213683;     TfWtwXqpRpfptCW10213683 = TfWtwXqpRpfptCW87798662;     TfWtwXqpRpfptCW87798662 = TfWtwXqpRpfptCW91223534;     TfWtwXqpRpfptCW91223534 = TfWtwXqpRpfptCW40293972;     TfWtwXqpRpfptCW40293972 = TfWtwXqpRpfptCW61539450;     TfWtwXqpRpfptCW61539450 = TfWtwXqpRpfptCW39022126;     TfWtwXqpRpfptCW39022126 = TfWtwXqpRpfptCW16818351;     TfWtwXqpRpfptCW16818351 = TfWtwXqpRpfptCW4678486;     TfWtwXqpRpfptCW4678486 = TfWtwXqpRpfptCW19555312;     TfWtwXqpRpfptCW19555312 = TfWtwXqpRpfptCW76176590;     TfWtwXqpRpfptCW76176590 = TfWtwXqpRpfptCW3506133;     TfWtwXqpRpfptCW3506133 = TfWtwXqpRpfptCW70221271;     TfWtwXqpRpfptCW70221271 = TfWtwXqpRpfptCW79070859;     TfWtwXqpRpfptCW79070859 = TfWtwXqpRpfptCW2868613;     TfWtwXqpRpfptCW2868613 = TfWtwXqpRpfptCW73560960;     TfWtwXqpRpfptCW73560960 = TfWtwXqpRpfptCW96443755;     TfWtwXqpRpfptCW96443755 = TfWtwXqpRpfptCW60883492;     TfWtwXqpRpfptCW60883492 = TfWtwXqpRpfptCW83265230;     TfWtwXqpRpfptCW83265230 = TfWtwXqpRpfptCW9828290;     TfWtwXqpRpfptCW9828290 = TfWtwXqpRpfptCW35342310;     TfWtwXqpRpfptCW35342310 = TfWtwXqpRpfptCW66604751;     TfWtwXqpRpfptCW66604751 = TfWtwXqpRpfptCW1930678;     TfWtwXqpRpfptCW1930678 = TfWtwXqpRpfptCW77821899;     TfWtwXqpRpfptCW77821899 = TfWtwXqpRpfptCW28916380;     TfWtwXqpRpfptCW28916380 = TfWtwXqpRpfptCW11412941;     TfWtwXqpRpfptCW11412941 = TfWtwXqpRpfptCW74446327;     TfWtwXqpRpfptCW74446327 = TfWtwXqpRpfptCW3270881;     TfWtwXqpRpfptCW3270881 = TfWtwXqpRpfptCW91960677;     TfWtwXqpRpfptCW91960677 = TfWtwXqpRpfptCW33709558;     TfWtwXqpRpfptCW33709558 = TfWtwXqpRpfptCW9307828;     TfWtwXqpRpfptCW9307828 = TfWtwXqpRpfptCW38183140;     TfWtwXqpRpfptCW38183140 = TfWtwXqpRpfptCW54052875;     TfWtwXqpRpfptCW54052875 = TfWtwXqpRpfptCW69240571;     TfWtwXqpRpfptCW69240571 = TfWtwXqpRpfptCW89708897;     TfWtwXqpRpfptCW89708897 = TfWtwXqpRpfptCW56941550;     TfWtwXqpRpfptCW56941550 = TfWtwXqpRpfptCW46264825;     TfWtwXqpRpfptCW46264825 = TfWtwXqpRpfptCW41382436;     TfWtwXqpRpfptCW41382436 = TfWtwXqpRpfptCW23492707;     TfWtwXqpRpfptCW23492707 = TfWtwXqpRpfptCW87888317;     TfWtwXqpRpfptCW87888317 = TfWtwXqpRpfptCW36889039;     TfWtwXqpRpfptCW36889039 = TfWtwXqpRpfptCW49853996;     TfWtwXqpRpfptCW49853996 = TfWtwXqpRpfptCW25834047;     TfWtwXqpRpfptCW25834047 = TfWtwXqpRpfptCW44398450;     TfWtwXqpRpfptCW44398450 = TfWtwXqpRpfptCW85668003;     TfWtwXqpRpfptCW85668003 = TfWtwXqpRpfptCW56671174;     TfWtwXqpRpfptCW56671174 = TfWtwXqpRpfptCW33147722;     TfWtwXqpRpfptCW33147722 = TfWtwXqpRpfptCW80582620;     TfWtwXqpRpfptCW80582620 = TfWtwXqpRpfptCW77924792;     TfWtwXqpRpfptCW77924792 = TfWtwXqpRpfptCW55075905;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VhxKvykgpNqsfVjKSDXjkMhizwlMS82506560() {     int iorhXnTcfteIhNr51588826 = -360318647;    int iorhXnTcfteIhNr10722082 = 53132250;    int iorhXnTcfteIhNr78768722 = -513578284;    int iorhXnTcfteIhNr52932658 = -43372086;    int iorhXnTcfteIhNr94922421 = 77745385;    int iorhXnTcfteIhNr48313910 = -497574034;    int iorhXnTcfteIhNr1025225 = -755932695;    int iorhXnTcfteIhNr60415139 = 78228873;    int iorhXnTcfteIhNr27508731 = -42673363;    int iorhXnTcfteIhNr26569524 = 26653489;    int iorhXnTcfteIhNr30239961 = -657543884;    int iorhXnTcfteIhNr22640885 = -610708551;    int iorhXnTcfteIhNr34320424 = -892521964;    int iorhXnTcfteIhNr91745284 = -136087121;    int iorhXnTcfteIhNr37592756 = -70668510;    int iorhXnTcfteIhNr98696278 = -645898891;    int iorhXnTcfteIhNr29705505 = -494919094;    int iorhXnTcfteIhNr33252773 = 26459765;    int iorhXnTcfteIhNr47807415 = -378339605;    int iorhXnTcfteIhNr80391752 = 73456755;    int iorhXnTcfteIhNr96772541 = -705351904;    int iorhXnTcfteIhNr55683690 = 22239790;    int iorhXnTcfteIhNr23596769 = -675245191;    int iorhXnTcfteIhNr7889056 = -547818763;    int iorhXnTcfteIhNr1840467 = -576279716;    int iorhXnTcfteIhNr97615910 = -376963500;    int iorhXnTcfteIhNr39408412 = -986810395;    int iorhXnTcfteIhNr43284343 = -207278351;    int iorhXnTcfteIhNr3652808 = -232523475;    int iorhXnTcfteIhNr53287710 = -451083988;    int iorhXnTcfteIhNr2588209 = -507270889;    int iorhXnTcfteIhNr2905416 = -471088613;    int iorhXnTcfteIhNr51507926 = -957595991;    int iorhXnTcfteIhNr45265639 = -541703026;    int iorhXnTcfteIhNr73271517 = -841580121;    int iorhXnTcfteIhNr93099021 = -812092123;    int iorhXnTcfteIhNr56468132 = -642562342;    int iorhXnTcfteIhNr39799321 = -726559132;    int iorhXnTcfteIhNr79246839 = -279819503;    int iorhXnTcfteIhNr43471160 = -207655140;    int iorhXnTcfteIhNr94115589 = -953729585;    int iorhXnTcfteIhNr4370659 = -972196974;    int iorhXnTcfteIhNr90910928 = 2896114;    int iorhXnTcfteIhNr71861587 = -271805663;    int iorhXnTcfteIhNr46780120 = -491425918;    int iorhXnTcfteIhNr29285809 = -223580657;    int iorhXnTcfteIhNr84794080 = -166004922;    int iorhXnTcfteIhNr62693923 = -888134462;    int iorhXnTcfteIhNr31459909 = 72012123;    int iorhXnTcfteIhNr40205000 = -973878832;    int iorhXnTcfteIhNr35581272 = -4959119;    int iorhXnTcfteIhNr76193870 = -402768167;    int iorhXnTcfteIhNr2357062 = -338071405;    int iorhXnTcfteIhNr73305858 = -446773770;    int iorhXnTcfteIhNr85734018 = 19253226;    int iorhXnTcfteIhNr95905135 = -282558438;    int iorhXnTcfteIhNr87125313 = -271622559;    int iorhXnTcfteIhNr70879666 = -965759522;    int iorhXnTcfteIhNr51092192 = -467092371;    int iorhXnTcfteIhNr97306510 = -545291116;    int iorhXnTcfteIhNr8905499 = -510763640;    int iorhXnTcfteIhNr57740881 = -448654345;    int iorhXnTcfteIhNr56762332 = -689247652;    int iorhXnTcfteIhNr74221020 = -591589376;    int iorhXnTcfteIhNr23981316 = -466075623;    int iorhXnTcfteIhNr27334545 = -86455271;    int iorhXnTcfteIhNr71132958 = -653112560;    int iorhXnTcfteIhNr89054785 = -250818939;    int iorhXnTcfteIhNr18473767 = -294507001;    int iorhXnTcfteIhNr44493734 = -258576387;    int iorhXnTcfteIhNr42228147 = 96663451;    int iorhXnTcfteIhNr89906184 = -768359962;    int iorhXnTcfteIhNr54005933 = -693720733;    int iorhXnTcfteIhNr4336255 = -70684466;    int iorhXnTcfteIhNr86276163 = 27186339;    int iorhXnTcfteIhNr92401882 = -733154931;    int iorhXnTcfteIhNr64772761 = -980656324;    int iorhXnTcfteIhNr51735181 = -303439528;    int iorhXnTcfteIhNr61108936 = 43607154;    int iorhXnTcfteIhNr72554657 = -252699059;    int iorhXnTcfteIhNr12821830 = -110958578;    int iorhXnTcfteIhNr76714488 = 1324067;    int iorhXnTcfteIhNr11824435 = -179290474;    int iorhXnTcfteIhNr63447807 = -258644644;    int iorhXnTcfteIhNr17706439 = -346124869;    int iorhXnTcfteIhNr26394339 = -4502722;    int iorhXnTcfteIhNr548355 = -33017209;    int iorhXnTcfteIhNr78202068 = -410822222;    int iorhXnTcfteIhNr59531620 = -460956253;    int iorhXnTcfteIhNr77366381 = -459021683;    int iorhXnTcfteIhNr5973709 = -440469565;    int iorhXnTcfteIhNr85588465 = -676802821;    int iorhXnTcfteIhNr88707128 = -159466762;    int iorhXnTcfteIhNr81940328 = -734528387;    int iorhXnTcfteIhNr34565662 = -696891501;    int iorhXnTcfteIhNr36374708 = -405075240;    int iorhXnTcfteIhNr47608327 = -182949322;    int iorhXnTcfteIhNr16689909 = -405514511;    int iorhXnTcfteIhNr47880271 = -805730041;    int iorhXnTcfteIhNr19445575 = -360318647;     iorhXnTcfteIhNr51588826 = iorhXnTcfteIhNr10722082;     iorhXnTcfteIhNr10722082 = iorhXnTcfteIhNr78768722;     iorhXnTcfteIhNr78768722 = iorhXnTcfteIhNr52932658;     iorhXnTcfteIhNr52932658 = iorhXnTcfteIhNr94922421;     iorhXnTcfteIhNr94922421 = iorhXnTcfteIhNr48313910;     iorhXnTcfteIhNr48313910 = iorhXnTcfteIhNr1025225;     iorhXnTcfteIhNr1025225 = iorhXnTcfteIhNr60415139;     iorhXnTcfteIhNr60415139 = iorhXnTcfteIhNr27508731;     iorhXnTcfteIhNr27508731 = iorhXnTcfteIhNr26569524;     iorhXnTcfteIhNr26569524 = iorhXnTcfteIhNr30239961;     iorhXnTcfteIhNr30239961 = iorhXnTcfteIhNr22640885;     iorhXnTcfteIhNr22640885 = iorhXnTcfteIhNr34320424;     iorhXnTcfteIhNr34320424 = iorhXnTcfteIhNr91745284;     iorhXnTcfteIhNr91745284 = iorhXnTcfteIhNr37592756;     iorhXnTcfteIhNr37592756 = iorhXnTcfteIhNr98696278;     iorhXnTcfteIhNr98696278 = iorhXnTcfteIhNr29705505;     iorhXnTcfteIhNr29705505 = iorhXnTcfteIhNr33252773;     iorhXnTcfteIhNr33252773 = iorhXnTcfteIhNr47807415;     iorhXnTcfteIhNr47807415 = iorhXnTcfteIhNr80391752;     iorhXnTcfteIhNr80391752 = iorhXnTcfteIhNr96772541;     iorhXnTcfteIhNr96772541 = iorhXnTcfteIhNr55683690;     iorhXnTcfteIhNr55683690 = iorhXnTcfteIhNr23596769;     iorhXnTcfteIhNr23596769 = iorhXnTcfteIhNr7889056;     iorhXnTcfteIhNr7889056 = iorhXnTcfteIhNr1840467;     iorhXnTcfteIhNr1840467 = iorhXnTcfteIhNr97615910;     iorhXnTcfteIhNr97615910 = iorhXnTcfteIhNr39408412;     iorhXnTcfteIhNr39408412 = iorhXnTcfteIhNr43284343;     iorhXnTcfteIhNr43284343 = iorhXnTcfteIhNr3652808;     iorhXnTcfteIhNr3652808 = iorhXnTcfteIhNr53287710;     iorhXnTcfteIhNr53287710 = iorhXnTcfteIhNr2588209;     iorhXnTcfteIhNr2588209 = iorhXnTcfteIhNr2905416;     iorhXnTcfteIhNr2905416 = iorhXnTcfteIhNr51507926;     iorhXnTcfteIhNr51507926 = iorhXnTcfteIhNr45265639;     iorhXnTcfteIhNr45265639 = iorhXnTcfteIhNr73271517;     iorhXnTcfteIhNr73271517 = iorhXnTcfteIhNr93099021;     iorhXnTcfteIhNr93099021 = iorhXnTcfteIhNr56468132;     iorhXnTcfteIhNr56468132 = iorhXnTcfteIhNr39799321;     iorhXnTcfteIhNr39799321 = iorhXnTcfteIhNr79246839;     iorhXnTcfteIhNr79246839 = iorhXnTcfteIhNr43471160;     iorhXnTcfteIhNr43471160 = iorhXnTcfteIhNr94115589;     iorhXnTcfteIhNr94115589 = iorhXnTcfteIhNr4370659;     iorhXnTcfteIhNr4370659 = iorhXnTcfteIhNr90910928;     iorhXnTcfteIhNr90910928 = iorhXnTcfteIhNr71861587;     iorhXnTcfteIhNr71861587 = iorhXnTcfteIhNr46780120;     iorhXnTcfteIhNr46780120 = iorhXnTcfteIhNr29285809;     iorhXnTcfteIhNr29285809 = iorhXnTcfteIhNr84794080;     iorhXnTcfteIhNr84794080 = iorhXnTcfteIhNr62693923;     iorhXnTcfteIhNr62693923 = iorhXnTcfteIhNr31459909;     iorhXnTcfteIhNr31459909 = iorhXnTcfteIhNr40205000;     iorhXnTcfteIhNr40205000 = iorhXnTcfteIhNr35581272;     iorhXnTcfteIhNr35581272 = iorhXnTcfteIhNr76193870;     iorhXnTcfteIhNr76193870 = iorhXnTcfteIhNr2357062;     iorhXnTcfteIhNr2357062 = iorhXnTcfteIhNr73305858;     iorhXnTcfteIhNr73305858 = iorhXnTcfteIhNr85734018;     iorhXnTcfteIhNr85734018 = iorhXnTcfteIhNr95905135;     iorhXnTcfteIhNr95905135 = iorhXnTcfteIhNr87125313;     iorhXnTcfteIhNr87125313 = iorhXnTcfteIhNr70879666;     iorhXnTcfteIhNr70879666 = iorhXnTcfteIhNr51092192;     iorhXnTcfteIhNr51092192 = iorhXnTcfteIhNr97306510;     iorhXnTcfteIhNr97306510 = iorhXnTcfteIhNr8905499;     iorhXnTcfteIhNr8905499 = iorhXnTcfteIhNr57740881;     iorhXnTcfteIhNr57740881 = iorhXnTcfteIhNr56762332;     iorhXnTcfteIhNr56762332 = iorhXnTcfteIhNr74221020;     iorhXnTcfteIhNr74221020 = iorhXnTcfteIhNr23981316;     iorhXnTcfteIhNr23981316 = iorhXnTcfteIhNr27334545;     iorhXnTcfteIhNr27334545 = iorhXnTcfteIhNr71132958;     iorhXnTcfteIhNr71132958 = iorhXnTcfteIhNr89054785;     iorhXnTcfteIhNr89054785 = iorhXnTcfteIhNr18473767;     iorhXnTcfteIhNr18473767 = iorhXnTcfteIhNr44493734;     iorhXnTcfteIhNr44493734 = iorhXnTcfteIhNr42228147;     iorhXnTcfteIhNr42228147 = iorhXnTcfteIhNr89906184;     iorhXnTcfteIhNr89906184 = iorhXnTcfteIhNr54005933;     iorhXnTcfteIhNr54005933 = iorhXnTcfteIhNr4336255;     iorhXnTcfteIhNr4336255 = iorhXnTcfteIhNr86276163;     iorhXnTcfteIhNr86276163 = iorhXnTcfteIhNr92401882;     iorhXnTcfteIhNr92401882 = iorhXnTcfteIhNr64772761;     iorhXnTcfteIhNr64772761 = iorhXnTcfteIhNr51735181;     iorhXnTcfteIhNr51735181 = iorhXnTcfteIhNr61108936;     iorhXnTcfteIhNr61108936 = iorhXnTcfteIhNr72554657;     iorhXnTcfteIhNr72554657 = iorhXnTcfteIhNr12821830;     iorhXnTcfteIhNr12821830 = iorhXnTcfteIhNr76714488;     iorhXnTcfteIhNr76714488 = iorhXnTcfteIhNr11824435;     iorhXnTcfteIhNr11824435 = iorhXnTcfteIhNr63447807;     iorhXnTcfteIhNr63447807 = iorhXnTcfteIhNr17706439;     iorhXnTcfteIhNr17706439 = iorhXnTcfteIhNr26394339;     iorhXnTcfteIhNr26394339 = iorhXnTcfteIhNr548355;     iorhXnTcfteIhNr548355 = iorhXnTcfteIhNr78202068;     iorhXnTcfteIhNr78202068 = iorhXnTcfteIhNr59531620;     iorhXnTcfteIhNr59531620 = iorhXnTcfteIhNr77366381;     iorhXnTcfteIhNr77366381 = iorhXnTcfteIhNr5973709;     iorhXnTcfteIhNr5973709 = iorhXnTcfteIhNr85588465;     iorhXnTcfteIhNr85588465 = iorhXnTcfteIhNr88707128;     iorhXnTcfteIhNr88707128 = iorhXnTcfteIhNr81940328;     iorhXnTcfteIhNr81940328 = iorhXnTcfteIhNr34565662;     iorhXnTcfteIhNr34565662 = iorhXnTcfteIhNr36374708;     iorhXnTcfteIhNr36374708 = iorhXnTcfteIhNr47608327;     iorhXnTcfteIhNr47608327 = iorhXnTcfteIhNr16689909;     iorhXnTcfteIhNr16689909 = iorhXnTcfteIhNr47880271;     iorhXnTcfteIhNr47880271 = iorhXnTcfteIhNr19445575;     iorhXnTcfteIhNr19445575 = iorhXnTcfteIhNr51588826;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void oDTWRsLILMyvcEHiNdvtLrEhXmXXd84695898() {     int SxISRdyJdDIfvrl48101747 = -359119336;    int SxISRdyJdDIfvrl25804908 = -999556052;    int SxISRdyJdDIfvrl34451034 = -261681049;    int SxISRdyJdDIfvrl65481506 = -512185877;    int SxISRdyJdDIfvrl63574811 = -577841611;    int SxISRdyJdDIfvrl85560782 = -528101611;    int SxISRdyJdDIfvrl74839877 = -920850670;    int SxISRdyJdDIfvrl95867309 = -34655311;    int SxISRdyJdDIfvrl31762988 = -295091117;    int SxISRdyJdDIfvrl80574060 = -732071724;    int SxISRdyJdDIfvrl90456076 = -632779121;    int SxISRdyJdDIfvrl37657322 = -79441456;    int SxISRdyJdDIfvrl39591122 = -78307628;    int SxISRdyJdDIfvrl58820398 = -645608656;    int SxISRdyJdDIfvrl42492513 = 78004780;    int SxISRdyJdDIfvrl11653102 = -404520106;    int SxISRdyJdDIfvrl57396353 = -407766138;    int SxISRdyJdDIfvrl69388260 = -591066300;    int SxISRdyJdDIfvrl29730391 = -867466906;    int SxISRdyJdDIfvrl21117012 = -936068076;    int SxISRdyJdDIfvrl4451397 = -915901502;    int SxISRdyJdDIfvrl96585447 = -355777902;    int SxISRdyJdDIfvrl13093732 = -956164901;    int SxISRdyJdDIfvrl31713827 = -506936167;    int SxISRdyJdDIfvrl80115473 = -101705475;    int SxISRdyJdDIfvrl73640275 = -556258763;    int SxISRdyJdDIfvrl87305096 = -813992242;    int SxISRdyJdDIfvrl35534706 = -963034450;    int SxISRdyJdDIfvrl85848778 = -97901458;    int SxISRdyJdDIfvrl53542218 = -915860830;    int SxISRdyJdDIfvrl11682288 = -809239042;    int SxISRdyJdDIfvrl38655599 = -803094634;    int SxISRdyJdDIfvrl68952365 = -847692759;    int SxISRdyJdDIfvrl57925307 = -362213026;    int SxISRdyJdDIfvrl82756356 = 43754353;    int SxISRdyJdDIfvrl36770275 = 10831168;    int SxISRdyJdDIfvrl37025099 = -152986300;    int SxISRdyJdDIfvrl12926295 = -547691101;    int SxISRdyJdDIfvrl27981146 = -620888253;    int SxISRdyJdDIfvrl22988560 = -744003022;    int SxISRdyJdDIfvrl26386584 = -385271755;    int SxISRdyJdDIfvrl48564011 = -379637937;    int SxISRdyJdDIfvrl78452864 = -493376458;    int SxISRdyJdDIfvrl84069695 = -413439417;    int SxISRdyJdDIfvrl12766835 = 76419745;    int SxISRdyJdDIfvrl26966836 = -555908453;    int SxISRdyJdDIfvrl81706173 = -444525571;    int SxISRdyJdDIfvrl43183946 = 48771538;    int SxISRdyJdDIfvrl50068976 = -559492072;    int SxISRdyJdDIfvrl13006039 = -223418287;    int SxISRdyJdDIfvrl87369912 = -634338325;    int SxISRdyJdDIfvrl48602508 = -984933417;    int SxISRdyJdDIfvrl94500439 = -29980049;    int SxISRdyJdDIfvrl58813054 = -896974774;    int SxISRdyJdDIfvrl80244501 = -413468566;    int SxISRdyJdDIfvrl51516299 = 96658566;    int SxISRdyJdDIfvrl12711176 = 56608849;    int SxISRdyJdDIfvrl2737208 = -754744883;    int SxISRdyJdDIfvrl85366033 = -310480402;    int SxISRdyJdDIfvrl89934536 = 78417151;    int SxISRdyJdDIfvrl98255686 = -714109369;    int SxISRdyJdDIfvrl39305172 = -957816221;    int SxISRdyJdDIfvrl10018531 = -936753853;    int SxISRdyJdDIfvrl78220769 = -379230287;    int SxISRdyJdDIfvrl68891773 = -922832683;    int SxISRdyJdDIfvrl51800477 = -829684488;    int SxISRdyJdDIfvrl68704957 = -231748698;    int SxISRdyJdDIfvrl81665814 = -716094603;    int SxISRdyJdDIfvrl76064041 = -589363010;    int SxISRdyJdDIfvrl5722238 = -932826389;    int SxISRdyJdDIfvrl74628003 = -151533807;    int SxISRdyJdDIfvrl44470058 = -860075038;    int SxISRdyJdDIfvrl41407115 = -970178048;    int SxISRdyJdDIfvrl6741832 = -23463884;    int SxISRdyJdDIfvrl94730428 = -450796321;    int SxISRdyJdDIfvrl55887386 = -436263566;    int SxISRdyJdDIfvrl18132583 = -862401445;    int SxISRdyJdDIfvrl29024036 = -442725485;    int SxISRdyJdDIfvrl18946992 = -483355912;    int SxISRdyJdDIfvrl53148638 = -545797022;    int SxISRdyJdDIfvrl91934101 = -11733192;    int SxISRdyJdDIfvrl44121150 = -762763781;    int SxISRdyJdDIfvrl85465729 = -303542378;    int SxISRdyJdDIfvrl72842739 = -874483172;    int SxISRdyJdDIfvrl66172306 = -181522505;    int SxISRdyJdDIfvrl63079779 = -824305625;    int SxISRdyJdDIfvrl44155159 = -673114585;    int SxISRdyJdDIfvrl10139312 = -950717986;    int SxISRdyJdDIfvrl77680805 = -948744460;    int SxISRdyJdDIfvrl31240057 = 47095786;    int SxISRdyJdDIfvrl24059099 = 54222319;    int SxISRdyJdDIfvrl34287891 = -398241417;    int SxISRdyJdDIfvrl27560262 = -137210699;    int SxISRdyJdDIfvrl38046610 = -599305404;    int SxISRdyJdDIfvrl24732874 = 70106347;    int SxISRdyJdDIfvrl87081412 = -427455535;    int SxISRdyJdDIfvrl38545480 = -442884084;    int SxISRdyJdDIfvrl232096 = -14146172;    int SxISRdyJdDIfvrl15177923 = -490606735;    int SxISRdyJdDIfvrl60966357 = -359119336;     SxISRdyJdDIfvrl48101747 = SxISRdyJdDIfvrl25804908;     SxISRdyJdDIfvrl25804908 = SxISRdyJdDIfvrl34451034;     SxISRdyJdDIfvrl34451034 = SxISRdyJdDIfvrl65481506;     SxISRdyJdDIfvrl65481506 = SxISRdyJdDIfvrl63574811;     SxISRdyJdDIfvrl63574811 = SxISRdyJdDIfvrl85560782;     SxISRdyJdDIfvrl85560782 = SxISRdyJdDIfvrl74839877;     SxISRdyJdDIfvrl74839877 = SxISRdyJdDIfvrl95867309;     SxISRdyJdDIfvrl95867309 = SxISRdyJdDIfvrl31762988;     SxISRdyJdDIfvrl31762988 = SxISRdyJdDIfvrl80574060;     SxISRdyJdDIfvrl80574060 = SxISRdyJdDIfvrl90456076;     SxISRdyJdDIfvrl90456076 = SxISRdyJdDIfvrl37657322;     SxISRdyJdDIfvrl37657322 = SxISRdyJdDIfvrl39591122;     SxISRdyJdDIfvrl39591122 = SxISRdyJdDIfvrl58820398;     SxISRdyJdDIfvrl58820398 = SxISRdyJdDIfvrl42492513;     SxISRdyJdDIfvrl42492513 = SxISRdyJdDIfvrl11653102;     SxISRdyJdDIfvrl11653102 = SxISRdyJdDIfvrl57396353;     SxISRdyJdDIfvrl57396353 = SxISRdyJdDIfvrl69388260;     SxISRdyJdDIfvrl69388260 = SxISRdyJdDIfvrl29730391;     SxISRdyJdDIfvrl29730391 = SxISRdyJdDIfvrl21117012;     SxISRdyJdDIfvrl21117012 = SxISRdyJdDIfvrl4451397;     SxISRdyJdDIfvrl4451397 = SxISRdyJdDIfvrl96585447;     SxISRdyJdDIfvrl96585447 = SxISRdyJdDIfvrl13093732;     SxISRdyJdDIfvrl13093732 = SxISRdyJdDIfvrl31713827;     SxISRdyJdDIfvrl31713827 = SxISRdyJdDIfvrl80115473;     SxISRdyJdDIfvrl80115473 = SxISRdyJdDIfvrl73640275;     SxISRdyJdDIfvrl73640275 = SxISRdyJdDIfvrl87305096;     SxISRdyJdDIfvrl87305096 = SxISRdyJdDIfvrl35534706;     SxISRdyJdDIfvrl35534706 = SxISRdyJdDIfvrl85848778;     SxISRdyJdDIfvrl85848778 = SxISRdyJdDIfvrl53542218;     SxISRdyJdDIfvrl53542218 = SxISRdyJdDIfvrl11682288;     SxISRdyJdDIfvrl11682288 = SxISRdyJdDIfvrl38655599;     SxISRdyJdDIfvrl38655599 = SxISRdyJdDIfvrl68952365;     SxISRdyJdDIfvrl68952365 = SxISRdyJdDIfvrl57925307;     SxISRdyJdDIfvrl57925307 = SxISRdyJdDIfvrl82756356;     SxISRdyJdDIfvrl82756356 = SxISRdyJdDIfvrl36770275;     SxISRdyJdDIfvrl36770275 = SxISRdyJdDIfvrl37025099;     SxISRdyJdDIfvrl37025099 = SxISRdyJdDIfvrl12926295;     SxISRdyJdDIfvrl12926295 = SxISRdyJdDIfvrl27981146;     SxISRdyJdDIfvrl27981146 = SxISRdyJdDIfvrl22988560;     SxISRdyJdDIfvrl22988560 = SxISRdyJdDIfvrl26386584;     SxISRdyJdDIfvrl26386584 = SxISRdyJdDIfvrl48564011;     SxISRdyJdDIfvrl48564011 = SxISRdyJdDIfvrl78452864;     SxISRdyJdDIfvrl78452864 = SxISRdyJdDIfvrl84069695;     SxISRdyJdDIfvrl84069695 = SxISRdyJdDIfvrl12766835;     SxISRdyJdDIfvrl12766835 = SxISRdyJdDIfvrl26966836;     SxISRdyJdDIfvrl26966836 = SxISRdyJdDIfvrl81706173;     SxISRdyJdDIfvrl81706173 = SxISRdyJdDIfvrl43183946;     SxISRdyJdDIfvrl43183946 = SxISRdyJdDIfvrl50068976;     SxISRdyJdDIfvrl50068976 = SxISRdyJdDIfvrl13006039;     SxISRdyJdDIfvrl13006039 = SxISRdyJdDIfvrl87369912;     SxISRdyJdDIfvrl87369912 = SxISRdyJdDIfvrl48602508;     SxISRdyJdDIfvrl48602508 = SxISRdyJdDIfvrl94500439;     SxISRdyJdDIfvrl94500439 = SxISRdyJdDIfvrl58813054;     SxISRdyJdDIfvrl58813054 = SxISRdyJdDIfvrl80244501;     SxISRdyJdDIfvrl80244501 = SxISRdyJdDIfvrl51516299;     SxISRdyJdDIfvrl51516299 = SxISRdyJdDIfvrl12711176;     SxISRdyJdDIfvrl12711176 = SxISRdyJdDIfvrl2737208;     SxISRdyJdDIfvrl2737208 = SxISRdyJdDIfvrl85366033;     SxISRdyJdDIfvrl85366033 = SxISRdyJdDIfvrl89934536;     SxISRdyJdDIfvrl89934536 = SxISRdyJdDIfvrl98255686;     SxISRdyJdDIfvrl98255686 = SxISRdyJdDIfvrl39305172;     SxISRdyJdDIfvrl39305172 = SxISRdyJdDIfvrl10018531;     SxISRdyJdDIfvrl10018531 = SxISRdyJdDIfvrl78220769;     SxISRdyJdDIfvrl78220769 = SxISRdyJdDIfvrl68891773;     SxISRdyJdDIfvrl68891773 = SxISRdyJdDIfvrl51800477;     SxISRdyJdDIfvrl51800477 = SxISRdyJdDIfvrl68704957;     SxISRdyJdDIfvrl68704957 = SxISRdyJdDIfvrl81665814;     SxISRdyJdDIfvrl81665814 = SxISRdyJdDIfvrl76064041;     SxISRdyJdDIfvrl76064041 = SxISRdyJdDIfvrl5722238;     SxISRdyJdDIfvrl5722238 = SxISRdyJdDIfvrl74628003;     SxISRdyJdDIfvrl74628003 = SxISRdyJdDIfvrl44470058;     SxISRdyJdDIfvrl44470058 = SxISRdyJdDIfvrl41407115;     SxISRdyJdDIfvrl41407115 = SxISRdyJdDIfvrl6741832;     SxISRdyJdDIfvrl6741832 = SxISRdyJdDIfvrl94730428;     SxISRdyJdDIfvrl94730428 = SxISRdyJdDIfvrl55887386;     SxISRdyJdDIfvrl55887386 = SxISRdyJdDIfvrl18132583;     SxISRdyJdDIfvrl18132583 = SxISRdyJdDIfvrl29024036;     SxISRdyJdDIfvrl29024036 = SxISRdyJdDIfvrl18946992;     SxISRdyJdDIfvrl18946992 = SxISRdyJdDIfvrl53148638;     SxISRdyJdDIfvrl53148638 = SxISRdyJdDIfvrl91934101;     SxISRdyJdDIfvrl91934101 = SxISRdyJdDIfvrl44121150;     SxISRdyJdDIfvrl44121150 = SxISRdyJdDIfvrl85465729;     SxISRdyJdDIfvrl85465729 = SxISRdyJdDIfvrl72842739;     SxISRdyJdDIfvrl72842739 = SxISRdyJdDIfvrl66172306;     SxISRdyJdDIfvrl66172306 = SxISRdyJdDIfvrl63079779;     SxISRdyJdDIfvrl63079779 = SxISRdyJdDIfvrl44155159;     SxISRdyJdDIfvrl44155159 = SxISRdyJdDIfvrl10139312;     SxISRdyJdDIfvrl10139312 = SxISRdyJdDIfvrl77680805;     SxISRdyJdDIfvrl77680805 = SxISRdyJdDIfvrl31240057;     SxISRdyJdDIfvrl31240057 = SxISRdyJdDIfvrl24059099;     SxISRdyJdDIfvrl24059099 = SxISRdyJdDIfvrl34287891;     SxISRdyJdDIfvrl34287891 = SxISRdyJdDIfvrl27560262;     SxISRdyJdDIfvrl27560262 = SxISRdyJdDIfvrl38046610;     SxISRdyJdDIfvrl38046610 = SxISRdyJdDIfvrl24732874;     SxISRdyJdDIfvrl24732874 = SxISRdyJdDIfvrl87081412;     SxISRdyJdDIfvrl87081412 = SxISRdyJdDIfvrl38545480;     SxISRdyJdDIfvrl38545480 = SxISRdyJdDIfvrl232096;     SxISRdyJdDIfvrl232096 = SxISRdyJdDIfvrl15177923;     SxISRdyJdDIfvrl15177923 = SxISRdyJdDIfvrl60966357;     SxISRdyJdDIfvrl60966357 = SxISRdyJdDIfvrl48101747;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bEdFHGDFOxxBlGzesDyVNystfYhfQ34642706() {     int APxeJGINisDyxdM15272806 = -3545013;    int APxeJGINisDyxdM70988933 = -781292528;    int APxeJGINisDyxdM68789051 = -494949661;    int APxeJGINisDyxdM32881781 = -250597226;    int APxeJGINisDyxdM44837421 = -548479633;    int APxeJGINisDyxdM78001985 = -560777527;    int APxeJGINisDyxdM98486090 = -280007888;    int APxeJGINisDyxdM18947810 = -790179268;    int APxeJGINisDyxdM99142134 = -927210129;    int APxeJGINisDyxdM60905544 = 44914584;    int APxeJGINisDyxdM72600747 = -683444216;    int APxeJGINisDyxdM83928653 = -128459793;    int APxeJGINisDyxdM92836292 = -251971875;    int APxeJGINisDyxdM55898324 = -611005350;    int APxeJGINisDyxdM84940180 = 39640919;    int APxeJGINisDyxdM82574018 = -500329257;    int APxeJGINisDyxdM23087270 = -410183743;    int APxeJGINisDyxdM2816711 = -800686940;    int APxeJGINisDyxdM72932879 = -345799881;    int APxeJGINisDyxdM48555217 = -775934760;    int APxeJGINisDyxdM87596581 = -224663867;    int APxeJGINisDyxdM20586704 = -555089017;    int APxeJGINisDyxdM67656106 = -245969914;    int APxeJGINisDyxdM45738025 = -177533955;    int APxeJGINisDyxdM94072179 = 23657931;    int APxeJGINisDyxdM63688143 = -925632542;    int APxeJGINisDyxdM73240985 = -735552151;    int APxeJGINisDyxdM163769 = -514947328;    int APxeJGINisDyxdM87508791 = -711567343;    int APxeJGINisDyxdM16878952 = -63601369;    int APxeJGINisDyxdM94435685 = -472547646;    int APxeJGINisDyxdM34193386 = 28423945;    int APxeJGINisDyxdM83889741 = -482806920;    int APxeJGINisDyxdM35667113 = -969570977;    int APxeJGINisDyxdM87907392 = -858544027;    int APxeJGINisDyxdM97606598 = -265706342;    int APxeJGINisDyxdM49432183 = -852350087;    int APxeJGINisDyxdM68928078 = -157673498;    int APxeJGINisDyxdM19935145 = -205004224;    int APxeJGINisDyxdM25736364 = -88527581;    int APxeJGINisDyxdM30875195 = -112888906;    int APxeJGINisDyxdM35542693 = -462675886;    int APxeJGINisDyxdM54678400 = -417272828;    int APxeJGINisDyxdM43554206 = 29916301;    int APxeJGINisDyxdM90689556 = -291121586;    int APxeJGINisDyxdM90625728 = -834528733;    int APxeJGINisDyxdM77051676 = -357835396;    int APxeJGINisDyxdM13904599 = 42308600;    int APxeJGINisDyxdM55461107 = -512996375;    int APxeJGINisDyxdM74189141 = -168941735;    int APxeJGINisDyxdM94088506 = -131734645;    int APxeJGINisDyxdM48335394 = -54172641;    int APxeJGINisDyxdM66271127 = -857632727;    int APxeJGINisDyxdM24297283 = -376137424;    int APxeJGINisDyxdM47046124 = -100231995;    int APxeJGINisDyxdM94686102 = -448455996;    int APxeJGINisDyxdM3332828 = -435322614;    int APxeJGINisDyxdM23051026 = -217415707;    int APxeJGINisDyxdM38809601 = -174255158;    int APxeJGINisDyxdM81149277 = -622847092;    int APxeJGINisDyxdM4761000 = -825225376;    int APxeJGINisDyxdM98322321 = -765060561;    int APxeJGINisDyxdM31439019 = 21388075;    int APxeJGINisDyxdM82263183 = -763608760;    int APxeJGINisDyxdM66469859 = -482537771;    int APxeJGINisDyxdM38407362 = -611868161;    int APxeJGINisDyxdM38912 = -645652873;    int APxeJGINisDyxdM57169179 = -282400898;    int APxeJGINisDyxdM67990931 = -752461323;    int APxeJGINisDyxdM87333582 = -694652740;    int APxeJGINisDyxdM33141835 = -647979171;    int APxeJGINisDyxdM54159191 = -152510246;    int APxeJGINisDyxdM82881565 = -495682716;    int APxeJGINisDyxdM47196515 = -157272301;    int APxeJGINisDyxdM17680022 = -563045854;    int APxeJGINisDyxdM52053889 = -761987982;    int APxeJGINisDyxdM65908303 = -37816190;    int APxeJGINisDyxdM24101900 = -175886216;    int APxeJGINisDyxdM55048469 = -886412369;    int APxeJGINisDyxdM3446451 = -141813336;    int APxeJGINisDyxdM86636466 = -467797146;    int APxeJGINisDyxdM59336387 = -677860752;    int APxeJGINisDyxdM44702662 = 98049047;    int APxeJGINisDyxdM13319650 = -442625609;    int APxeJGINisDyxdM22790445 = -931866724;    int APxeJGINisDyxdM46100292 = -318375006;    int APxeJGINisDyxdM67922259 = -113943329;    int APxeJGINisDyxdM59592458 = -6669497;    int APxeJGINisDyxdM88620989 = -769338982;    int APxeJGINisDyxdM93221290 = -310088032;    int APxeJGINisDyxdM94273770 = -830383728;    int APxeJGINisDyxdM26381157 = -534934380;    int APxeJGINisDyxdM30118478 = -983418341;    int APxeJGINisDyxdM38785868 = -582157133;    int APxeJGINisDyxdM20975365 = -263302205;    int APxeJGINisDyxdM32552874 = -347828346;    int APxeJGINisDyxdM4103675 = -384063961;    int APxeJGINisDyxdM72415217 = -653664068;    int APxeJGINisDyxdM77084347 = -487545929;    int APxeJGINisDyxdM52282194 = -3545013;     APxeJGINisDyxdM15272806 = APxeJGINisDyxdM70988933;     APxeJGINisDyxdM70988933 = APxeJGINisDyxdM68789051;     APxeJGINisDyxdM68789051 = APxeJGINisDyxdM32881781;     APxeJGINisDyxdM32881781 = APxeJGINisDyxdM44837421;     APxeJGINisDyxdM44837421 = APxeJGINisDyxdM78001985;     APxeJGINisDyxdM78001985 = APxeJGINisDyxdM98486090;     APxeJGINisDyxdM98486090 = APxeJGINisDyxdM18947810;     APxeJGINisDyxdM18947810 = APxeJGINisDyxdM99142134;     APxeJGINisDyxdM99142134 = APxeJGINisDyxdM60905544;     APxeJGINisDyxdM60905544 = APxeJGINisDyxdM72600747;     APxeJGINisDyxdM72600747 = APxeJGINisDyxdM83928653;     APxeJGINisDyxdM83928653 = APxeJGINisDyxdM92836292;     APxeJGINisDyxdM92836292 = APxeJGINisDyxdM55898324;     APxeJGINisDyxdM55898324 = APxeJGINisDyxdM84940180;     APxeJGINisDyxdM84940180 = APxeJGINisDyxdM82574018;     APxeJGINisDyxdM82574018 = APxeJGINisDyxdM23087270;     APxeJGINisDyxdM23087270 = APxeJGINisDyxdM2816711;     APxeJGINisDyxdM2816711 = APxeJGINisDyxdM72932879;     APxeJGINisDyxdM72932879 = APxeJGINisDyxdM48555217;     APxeJGINisDyxdM48555217 = APxeJGINisDyxdM87596581;     APxeJGINisDyxdM87596581 = APxeJGINisDyxdM20586704;     APxeJGINisDyxdM20586704 = APxeJGINisDyxdM67656106;     APxeJGINisDyxdM67656106 = APxeJGINisDyxdM45738025;     APxeJGINisDyxdM45738025 = APxeJGINisDyxdM94072179;     APxeJGINisDyxdM94072179 = APxeJGINisDyxdM63688143;     APxeJGINisDyxdM63688143 = APxeJGINisDyxdM73240985;     APxeJGINisDyxdM73240985 = APxeJGINisDyxdM163769;     APxeJGINisDyxdM163769 = APxeJGINisDyxdM87508791;     APxeJGINisDyxdM87508791 = APxeJGINisDyxdM16878952;     APxeJGINisDyxdM16878952 = APxeJGINisDyxdM94435685;     APxeJGINisDyxdM94435685 = APxeJGINisDyxdM34193386;     APxeJGINisDyxdM34193386 = APxeJGINisDyxdM83889741;     APxeJGINisDyxdM83889741 = APxeJGINisDyxdM35667113;     APxeJGINisDyxdM35667113 = APxeJGINisDyxdM87907392;     APxeJGINisDyxdM87907392 = APxeJGINisDyxdM97606598;     APxeJGINisDyxdM97606598 = APxeJGINisDyxdM49432183;     APxeJGINisDyxdM49432183 = APxeJGINisDyxdM68928078;     APxeJGINisDyxdM68928078 = APxeJGINisDyxdM19935145;     APxeJGINisDyxdM19935145 = APxeJGINisDyxdM25736364;     APxeJGINisDyxdM25736364 = APxeJGINisDyxdM30875195;     APxeJGINisDyxdM30875195 = APxeJGINisDyxdM35542693;     APxeJGINisDyxdM35542693 = APxeJGINisDyxdM54678400;     APxeJGINisDyxdM54678400 = APxeJGINisDyxdM43554206;     APxeJGINisDyxdM43554206 = APxeJGINisDyxdM90689556;     APxeJGINisDyxdM90689556 = APxeJGINisDyxdM90625728;     APxeJGINisDyxdM90625728 = APxeJGINisDyxdM77051676;     APxeJGINisDyxdM77051676 = APxeJGINisDyxdM13904599;     APxeJGINisDyxdM13904599 = APxeJGINisDyxdM55461107;     APxeJGINisDyxdM55461107 = APxeJGINisDyxdM74189141;     APxeJGINisDyxdM74189141 = APxeJGINisDyxdM94088506;     APxeJGINisDyxdM94088506 = APxeJGINisDyxdM48335394;     APxeJGINisDyxdM48335394 = APxeJGINisDyxdM66271127;     APxeJGINisDyxdM66271127 = APxeJGINisDyxdM24297283;     APxeJGINisDyxdM24297283 = APxeJGINisDyxdM47046124;     APxeJGINisDyxdM47046124 = APxeJGINisDyxdM94686102;     APxeJGINisDyxdM94686102 = APxeJGINisDyxdM3332828;     APxeJGINisDyxdM3332828 = APxeJGINisDyxdM23051026;     APxeJGINisDyxdM23051026 = APxeJGINisDyxdM38809601;     APxeJGINisDyxdM38809601 = APxeJGINisDyxdM81149277;     APxeJGINisDyxdM81149277 = APxeJGINisDyxdM4761000;     APxeJGINisDyxdM4761000 = APxeJGINisDyxdM98322321;     APxeJGINisDyxdM98322321 = APxeJGINisDyxdM31439019;     APxeJGINisDyxdM31439019 = APxeJGINisDyxdM82263183;     APxeJGINisDyxdM82263183 = APxeJGINisDyxdM66469859;     APxeJGINisDyxdM66469859 = APxeJGINisDyxdM38407362;     APxeJGINisDyxdM38407362 = APxeJGINisDyxdM38912;     APxeJGINisDyxdM38912 = APxeJGINisDyxdM57169179;     APxeJGINisDyxdM57169179 = APxeJGINisDyxdM67990931;     APxeJGINisDyxdM67990931 = APxeJGINisDyxdM87333582;     APxeJGINisDyxdM87333582 = APxeJGINisDyxdM33141835;     APxeJGINisDyxdM33141835 = APxeJGINisDyxdM54159191;     APxeJGINisDyxdM54159191 = APxeJGINisDyxdM82881565;     APxeJGINisDyxdM82881565 = APxeJGINisDyxdM47196515;     APxeJGINisDyxdM47196515 = APxeJGINisDyxdM17680022;     APxeJGINisDyxdM17680022 = APxeJGINisDyxdM52053889;     APxeJGINisDyxdM52053889 = APxeJGINisDyxdM65908303;     APxeJGINisDyxdM65908303 = APxeJGINisDyxdM24101900;     APxeJGINisDyxdM24101900 = APxeJGINisDyxdM55048469;     APxeJGINisDyxdM55048469 = APxeJGINisDyxdM3446451;     APxeJGINisDyxdM3446451 = APxeJGINisDyxdM86636466;     APxeJGINisDyxdM86636466 = APxeJGINisDyxdM59336387;     APxeJGINisDyxdM59336387 = APxeJGINisDyxdM44702662;     APxeJGINisDyxdM44702662 = APxeJGINisDyxdM13319650;     APxeJGINisDyxdM13319650 = APxeJGINisDyxdM22790445;     APxeJGINisDyxdM22790445 = APxeJGINisDyxdM46100292;     APxeJGINisDyxdM46100292 = APxeJGINisDyxdM67922259;     APxeJGINisDyxdM67922259 = APxeJGINisDyxdM59592458;     APxeJGINisDyxdM59592458 = APxeJGINisDyxdM88620989;     APxeJGINisDyxdM88620989 = APxeJGINisDyxdM93221290;     APxeJGINisDyxdM93221290 = APxeJGINisDyxdM94273770;     APxeJGINisDyxdM94273770 = APxeJGINisDyxdM26381157;     APxeJGINisDyxdM26381157 = APxeJGINisDyxdM30118478;     APxeJGINisDyxdM30118478 = APxeJGINisDyxdM38785868;     APxeJGINisDyxdM38785868 = APxeJGINisDyxdM20975365;     APxeJGINisDyxdM20975365 = APxeJGINisDyxdM32552874;     APxeJGINisDyxdM32552874 = APxeJGINisDyxdM4103675;     APxeJGINisDyxdM4103675 = APxeJGINisDyxdM72415217;     APxeJGINisDyxdM72415217 = APxeJGINisDyxdM77084347;     APxeJGINisDyxdM77084347 = APxeJGINisDyxdM52282194;     APxeJGINisDyxdM52282194 = APxeJGINisDyxdM15272806;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void loADlAYvStcbZdhbDrbCIYHTdlIsl36832044() {     int wEFjLLjedWEwbmM11785727 = -2345701;    int wEFjLLjedWEwbmM86071759 = -733980830;    int wEFjLLjedWEwbmM24471363 = -243052427;    int wEFjLLjedWEwbmM45430629 = -719411017;    int wEFjLLjedWEwbmM13489811 = -104066629;    int wEFjLLjedWEwbmM15248857 = -591305103;    int wEFjLLjedWEwbmM72300743 = -444925863;    int wEFjLLjedWEwbmM54399980 = -903063452;    int wEFjLLjedWEwbmM3396392 = -79627883;    int wEFjLLjedWEwbmM14910081 = -713810629;    int wEFjLLjedWEwbmM32816863 = -658679453;    int wEFjLLjedWEwbmM98945090 = -697192698;    int wEFjLLjedWEwbmM98106990 = -537757539;    int wEFjLLjedWEwbmM22973438 = -20526885;    int wEFjLLjedWEwbmM89839938 = -911685791;    int wEFjLLjedWEwbmM95530842 = -258950473;    int wEFjLLjedWEwbmM50778118 = -323030788;    int wEFjLLjedWEwbmM38952198 = -318213005;    int wEFjLLjedWEwbmM54855855 = -834927182;    int wEFjLLjedWEwbmM89280476 = -685459590;    int wEFjLLjedWEwbmM95275436 = -435213465;    int wEFjLLjedWEwbmM61488461 = -933106710;    int wEFjLLjedWEwbmM57153068 = -526889625;    int wEFjLLjedWEwbmM69562796 = -136651359;    int wEFjLLjedWEwbmM72347187 = -601767828;    int wEFjLLjedWEwbmM39712508 = -4927804;    int wEFjLLjedWEwbmM21137670 = -562733998;    int wEFjLLjedWEwbmM92414131 = -170703427;    int wEFjLLjedWEwbmM69704762 = -576945326;    int wEFjLLjedWEwbmM17133460 = -528378211;    int wEFjLLjedWEwbmM3529765 = -774515799;    int wEFjLLjedWEwbmM69943569 = -303582076;    int wEFjLLjedWEwbmM1334181 = -372903688;    int wEFjLLjedWEwbmM48326782 = -790080977;    int wEFjLLjedWEwbmM97392231 = 26790447;    int wEFjLLjedWEwbmM41277852 = -542783050;    int wEFjLLjedWEwbmM29989150 = -362774044;    int wEFjLLjedWEwbmM42055053 = 21194533;    int wEFjLLjedWEwbmM68669452 = -546072974;    int wEFjLLjedWEwbmM5253764 = -624875463;    int wEFjLLjedWEwbmM63146190 = -644431076;    int wEFjLLjedWEwbmM79736045 = -970116849;    int wEFjLLjedWEwbmM42220337 = -913545400;    int wEFjLLjedWEwbmM55762314 = -111717453;    int wEFjLLjedWEwbmM56676271 = -823275924;    int wEFjLLjedWEwbmM88306755 = -66856529;    int wEFjLLjedWEwbmM73963769 = -636356044;    int wEFjLLjedWEwbmM94394621 = -120785399;    int wEFjLLjedWEwbmM74070175 = -44500570;    int wEFjLLjedWEwbmM46990181 = -518481190;    int wEFjLLjedWEwbmM45877147 = -761113852;    int wEFjLLjedWEwbmM20744032 = -636337891;    int wEFjLLjedWEwbmM58414505 = -549541372;    int wEFjLLjedWEwbmM9804479 = -826338428;    int wEFjLLjedWEwbmM41556607 = -532953788;    int wEFjLLjedWEwbmM50297265 = -69238991;    int wEFjLLjedWEwbmM28918691 = -107091206;    int wEFjLLjedWEwbmM54908567 = -6401068;    int wEFjLLjedWEwbmM73083442 = -17643189;    int wEFjLLjedWEwbmM73777302 = 861175;    int wEFjLLjedWEwbmM94111187 = 71428894;    int wEFjLLjedWEwbmM79886612 = -174222437;    int wEFjLLjedWEwbmM84695217 = -226118126;    int wEFjLLjedWEwbmM86262932 = -551249672;    int wEFjLLjedWEwbmM11380316 = -939294830;    int wEFjLLjedWEwbmM62873294 = -255097378;    int wEFjLLjedWEwbmM97610909 = -224289011;    int wEFjLLjedWEwbmM49780209 = -747676562;    int wEFjLLjedWEwbmM25581206 = 52682668;    int wEFjLLjedWEwbmM48562086 = -268902741;    int wEFjLLjedWEwbmM65541692 = -896176429;    int wEFjLLjedWEwbmM8723066 = -244225322;    int wEFjLLjedWEwbmM70282746 = -772140031;    int wEFjLLjedWEwbmM49602092 = -110051719;    int wEFjLLjedWEwbmM26134287 = 58971486;    int wEFjLLjedWEwbmM15539392 = -465096617;    int wEFjLLjedWEwbmM19268125 = 80438690;    int wEFjLLjedWEwbmM1390755 = -315172173;    int wEFjLLjedWEwbmM12886525 = -313375436;    int wEFjLLjedWEwbmM84040431 = -434911299;    int wEFjLLjedWEwbmM65748738 = -368571761;    int wEFjLLjedWEwbmM26743048 = -341948599;    int wEFjLLjedWEwbmM18343956 = -26202857;    int wEFjLLjedWEwbmM22714582 = 41535863;    int wEFjLLjedWEwbmM71256312 = -767264360;    int wEFjLLjedWEwbmM82785733 = -38177909;    int wEFjLLjedWEwbmM11529064 = -754040705;    int wEFjLLjedWEwbmM91529701 = -546565261;    int wEFjLLjedWEwbmM6770175 = -157127190;    int wEFjLLjedWEwbmM47094966 = -903970562;    int wEFjLLjedWEwbmM12359162 = -335691845;    int wEFjLLjedWEwbmM75080583 = -256372977;    int wEFjLLjedWEwbmM68971610 = -961162278;    int wEFjLLjedWEwbmM94892149 = -446934150;    int wEFjLLjedWEwbmM11142577 = -596304358;    int wEFjLLjedWEwbmM83259578 = -370208640;    int wEFjLLjedWEwbmM95040827 = -643998723;    int wEFjLLjedWEwbmM55957404 = -262295729;    int wEFjLLjedWEwbmM44381999 = -172422623;    int wEFjLLjedWEwbmM93802977 = -2345701;     wEFjLLjedWEwbmM11785727 = wEFjLLjedWEwbmM86071759;     wEFjLLjedWEwbmM86071759 = wEFjLLjedWEwbmM24471363;     wEFjLLjedWEwbmM24471363 = wEFjLLjedWEwbmM45430629;     wEFjLLjedWEwbmM45430629 = wEFjLLjedWEwbmM13489811;     wEFjLLjedWEwbmM13489811 = wEFjLLjedWEwbmM15248857;     wEFjLLjedWEwbmM15248857 = wEFjLLjedWEwbmM72300743;     wEFjLLjedWEwbmM72300743 = wEFjLLjedWEwbmM54399980;     wEFjLLjedWEwbmM54399980 = wEFjLLjedWEwbmM3396392;     wEFjLLjedWEwbmM3396392 = wEFjLLjedWEwbmM14910081;     wEFjLLjedWEwbmM14910081 = wEFjLLjedWEwbmM32816863;     wEFjLLjedWEwbmM32816863 = wEFjLLjedWEwbmM98945090;     wEFjLLjedWEwbmM98945090 = wEFjLLjedWEwbmM98106990;     wEFjLLjedWEwbmM98106990 = wEFjLLjedWEwbmM22973438;     wEFjLLjedWEwbmM22973438 = wEFjLLjedWEwbmM89839938;     wEFjLLjedWEwbmM89839938 = wEFjLLjedWEwbmM95530842;     wEFjLLjedWEwbmM95530842 = wEFjLLjedWEwbmM50778118;     wEFjLLjedWEwbmM50778118 = wEFjLLjedWEwbmM38952198;     wEFjLLjedWEwbmM38952198 = wEFjLLjedWEwbmM54855855;     wEFjLLjedWEwbmM54855855 = wEFjLLjedWEwbmM89280476;     wEFjLLjedWEwbmM89280476 = wEFjLLjedWEwbmM95275436;     wEFjLLjedWEwbmM95275436 = wEFjLLjedWEwbmM61488461;     wEFjLLjedWEwbmM61488461 = wEFjLLjedWEwbmM57153068;     wEFjLLjedWEwbmM57153068 = wEFjLLjedWEwbmM69562796;     wEFjLLjedWEwbmM69562796 = wEFjLLjedWEwbmM72347187;     wEFjLLjedWEwbmM72347187 = wEFjLLjedWEwbmM39712508;     wEFjLLjedWEwbmM39712508 = wEFjLLjedWEwbmM21137670;     wEFjLLjedWEwbmM21137670 = wEFjLLjedWEwbmM92414131;     wEFjLLjedWEwbmM92414131 = wEFjLLjedWEwbmM69704762;     wEFjLLjedWEwbmM69704762 = wEFjLLjedWEwbmM17133460;     wEFjLLjedWEwbmM17133460 = wEFjLLjedWEwbmM3529765;     wEFjLLjedWEwbmM3529765 = wEFjLLjedWEwbmM69943569;     wEFjLLjedWEwbmM69943569 = wEFjLLjedWEwbmM1334181;     wEFjLLjedWEwbmM1334181 = wEFjLLjedWEwbmM48326782;     wEFjLLjedWEwbmM48326782 = wEFjLLjedWEwbmM97392231;     wEFjLLjedWEwbmM97392231 = wEFjLLjedWEwbmM41277852;     wEFjLLjedWEwbmM41277852 = wEFjLLjedWEwbmM29989150;     wEFjLLjedWEwbmM29989150 = wEFjLLjedWEwbmM42055053;     wEFjLLjedWEwbmM42055053 = wEFjLLjedWEwbmM68669452;     wEFjLLjedWEwbmM68669452 = wEFjLLjedWEwbmM5253764;     wEFjLLjedWEwbmM5253764 = wEFjLLjedWEwbmM63146190;     wEFjLLjedWEwbmM63146190 = wEFjLLjedWEwbmM79736045;     wEFjLLjedWEwbmM79736045 = wEFjLLjedWEwbmM42220337;     wEFjLLjedWEwbmM42220337 = wEFjLLjedWEwbmM55762314;     wEFjLLjedWEwbmM55762314 = wEFjLLjedWEwbmM56676271;     wEFjLLjedWEwbmM56676271 = wEFjLLjedWEwbmM88306755;     wEFjLLjedWEwbmM88306755 = wEFjLLjedWEwbmM73963769;     wEFjLLjedWEwbmM73963769 = wEFjLLjedWEwbmM94394621;     wEFjLLjedWEwbmM94394621 = wEFjLLjedWEwbmM74070175;     wEFjLLjedWEwbmM74070175 = wEFjLLjedWEwbmM46990181;     wEFjLLjedWEwbmM46990181 = wEFjLLjedWEwbmM45877147;     wEFjLLjedWEwbmM45877147 = wEFjLLjedWEwbmM20744032;     wEFjLLjedWEwbmM20744032 = wEFjLLjedWEwbmM58414505;     wEFjLLjedWEwbmM58414505 = wEFjLLjedWEwbmM9804479;     wEFjLLjedWEwbmM9804479 = wEFjLLjedWEwbmM41556607;     wEFjLLjedWEwbmM41556607 = wEFjLLjedWEwbmM50297265;     wEFjLLjedWEwbmM50297265 = wEFjLLjedWEwbmM28918691;     wEFjLLjedWEwbmM28918691 = wEFjLLjedWEwbmM54908567;     wEFjLLjedWEwbmM54908567 = wEFjLLjedWEwbmM73083442;     wEFjLLjedWEwbmM73083442 = wEFjLLjedWEwbmM73777302;     wEFjLLjedWEwbmM73777302 = wEFjLLjedWEwbmM94111187;     wEFjLLjedWEwbmM94111187 = wEFjLLjedWEwbmM79886612;     wEFjLLjedWEwbmM79886612 = wEFjLLjedWEwbmM84695217;     wEFjLLjedWEwbmM84695217 = wEFjLLjedWEwbmM86262932;     wEFjLLjedWEwbmM86262932 = wEFjLLjedWEwbmM11380316;     wEFjLLjedWEwbmM11380316 = wEFjLLjedWEwbmM62873294;     wEFjLLjedWEwbmM62873294 = wEFjLLjedWEwbmM97610909;     wEFjLLjedWEwbmM97610909 = wEFjLLjedWEwbmM49780209;     wEFjLLjedWEwbmM49780209 = wEFjLLjedWEwbmM25581206;     wEFjLLjedWEwbmM25581206 = wEFjLLjedWEwbmM48562086;     wEFjLLjedWEwbmM48562086 = wEFjLLjedWEwbmM65541692;     wEFjLLjedWEwbmM65541692 = wEFjLLjedWEwbmM8723066;     wEFjLLjedWEwbmM8723066 = wEFjLLjedWEwbmM70282746;     wEFjLLjedWEwbmM70282746 = wEFjLLjedWEwbmM49602092;     wEFjLLjedWEwbmM49602092 = wEFjLLjedWEwbmM26134287;     wEFjLLjedWEwbmM26134287 = wEFjLLjedWEwbmM15539392;     wEFjLLjedWEwbmM15539392 = wEFjLLjedWEwbmM19268125;     wEFjLLjedWEwbmM19268125 = wEFjLLjedWEwbmM1390755;     wEFjLLjedWEwbmM1390755 = wEFjLLjedWEwbmM12886525;     wEFjLLjedWEwbmM12886525 = wEFjLLjedWEwbmM84040431;     wEFjLLjedWEwbmM84040431 = wEFjLLjedWEwbmM65748738;     wEFjLLjedWEwbmM65748738 = wEFjLLjedWEwbmM26743048;     wEFjLLjedWEwbmM26743048 = wEFjLLjedWEwbmM18343956;     wEFjLLjedWEwbmM18343956 = wEFjLLjedWEwbmM22714582;     wEFjLLjedWEwbmM22714582 = wEFjLLjedWEwbmM71256312;     wEFjLLjedWEwbmM71256312 = wEFjLLjedWEwbmM82785733;     wEFjLLjedWEwbmM82785733 = wEFjLLjedWEwbmM11529064;     wEFjLLjedWEwbmM11529064 = wEFjLLjedWEwbmM91529701;     wEFjLLjedWEwbmM91529701 = wEFjLLjedWEwbmM6770175;     wEFjLLjedWEwbmM6770175 = wEFjLLjedWEwbmM47094966;     wEFjLLjedWEwbmM47094966 = wEFjLLjedWEwbmM12359162;     wEFjLLjedWEwbmM12359162 = wEFjLLjedWEwbmM75080583;     wEFjLLjedWEwbmM75080583 = wEFjLLjedWEwbmM68971610;     wEFjLLjedWEwbmM68971610 = wEFjLLjedWEwbmM94892149;     wEFjLLjedWEwbmM94892149 = wEFjLLjedWEwbmM11142577;     wEFjLLjedWEwbmM11142577 = wEFjLLjedWEwbmM83259578;     wEFjLLjedWEwbmM83259578 = wEFjLLjedWEwbmM95040827;     wEFjLLjedWEwbmM95040827 = wEFjLLjedWEwbmM55957404;     wEFjLLjedWEwbmM55957404 = wEFjLLjedWEwbmM44381999;     wEFjLLjedWEwbmM44381999 = wEFjLLjedWEwbmM93802977;     wEFjLLjedWEwbmM93802977 = wEFjLLjedWEwbmM11785727;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void stwqsRXCdrGFkTYjFIFGdsLrXVRfg39021383() {     int ioXCnPgDtQApWOL8298648 = -1146389;    int ioXCnPgDtQApWOL1154585 = -686669133;    int ioXCnPgDtQApWOL80153673 = 8844808;    int ioXCnPgDtQApWOL57979477 = -88224807;    int ioXCnPgDtQApWOL82142200 = -759653624;    int ioXCnPgDtQApWOL52495729 = -621832680;    int ioXCnPgDtQApWOL46115397 = -609843838;    int ioXCnPgDtQApWOL89852150 = 84052363;    int ioXCnPgDtQApWOL7650649 = -332045636;    int ioXCnPgDtQApWOL68914616 = -372535842;    int ioXCnPgDtQApWOL93032978 = -633914690;    int ioXCnPgDtQApWOL13961528 = -165925604;    int ioXCnPgDtQApWOL3377688 = -823543202;    int ioXCnPgDtQApWOL90048550 = -530048420;    int ioXCnPgDtQApWOL94739695 = -763012501;    int ioXCnPgDtQApWOL8487666 = -17571688;    int ioXCnPgDtQApWOL78468967 = -235877833;    int ioXCnPgDtQApWOL75087686 = -935739070;    int ioXCnPgDtQApWOL36778832 = -224054482;    int ioXCnPgDtQApWOL30005736 = -594984420;    int ioXCnPgDtQApWOL2954292 = -645763063;    int ioXCnPgDtQApWOL2390219 = -211124403;    int ioXCnPgDtQApWOL46650031 = -807809335;    int ioXCnPgDtQApWOL93387566 = -95768763;    int ioXCnPgDtQApWOL50622194 = -127193587;    int ioXCnPgDtQApWOL15736873 = -184223067;    int ioXCnPgDtQApWOL69034354 = -389915845;    int ioXCnPgDtQApWOL84664494 = -926459526;    int ioXCnPgDtQApWOL51900733 = -442323309;    int ioXCnPgDtQApWOL17387967 = -993155054;    int ioXCnPgDtQApWOL12623845 = 23516047;    int ioXCnPgDtQApWOL5693753 = -635588096;    int ioXCnPgDtQApWOL18778620 = -263000456;    int ioXCnPgDtQApWOL60986450 = -610590977;    int ioXCnPgDtQApWOL6877070 = -187875079;    int ioXCnPgDtQApWOL84949105 = -819859759;    int ioXCnPgDtQApWOL10546117 = -973198002;    int ioXCnPgDtQApWOL15182027 = -899937436;    int ioXCnPgDtQApWOL17403759 = -887141725;    int ioXCnPgDtQApWOL84771162 = -61223345;    int ioXCnPgDtQApWOL95417185 = -75973247;    int ioXCnPgDtQApWOL23929398 = -377557811;    int ioXCnPgDtQApWOL29762273 = -309817972;    int ioXCnPgDtQApWOL67970422 = -253351206;    int ioXCnPgDtQApWOL22662986 = -255430261;    int ioXCnPgDtQApWOL85987782 = -399184325;    int ioXCnPgDtQApWOL70875862 = -914876693;    int ioXCnPgDtQApWOL74884645 = -283879398;    int ioXCnPgDtQApWOL92679243 = -676004765;    int ioXCnPgDtQApWOL19791220 = -868020645;    int ioXCnPgDtQApWOL97665787 = -290493058;    int ioXCnPgDtQApWOL93152670 = -118503141;    int ioXCnPgDtQApWOL50557884 = -241450016;    int ioXCnPgDtQApWOL95311674 = -176539431;    int ioXCnPgDtQApWOL36067091 = -965675580;    int ioXCnPgDtQApWOL5908429 = -790021987;    int ioXCnPgDtQApWOL54504554 = -878859798;    int ioXCnPgDtQApWOL86766107 = -895386430;    int ioXCnPgDtQApWOL7357284 = -961031221;    int ioXCnPgDtQApWOL66405328 = -475430558;    int ioXCnPgDtQApWOL83461374 = -131916835;    int ioXCnPgDtQApWOL61450902 = -683384313;    int ioXCnPgDtQApWOL37951417 = -473624328;    int ioXCnPgDtQApWOL90262681 = -338890583;    int ioXCnPgDtQApWOL56290772 = -296051890;    int ioXCnPgDtQApWOL87339226 = -998326595;    int ioXCnPgDtQApWOL95182908 = -902925149;    int ioXCnPgDtQApWOL42391238 = -112952226;    int ioXCnPgDtQApWOL83171480 = -242173342;    int ioXCnPgDtQApWOL9790590 = -943152743;    int ioXCnPgDtQApWOL97941549 = -44373686;    int ioXCnPgDtQApWOL63286940 = -335940398;    int ioXCnPgDtQApWOL57683928 = 51402654;    int ioXCnPgDtQApWOL52007669 = -62831137;    int ioXCnPgDtQApWOL34588551 = -419011174;    int ioXCnPgDtQApWOL79024894 = -168205253;    int ioXCnPgDtQApWOL72627946 = -901306431;    int ioXCnPgDtQApWOL78679608 = -454458130;    int ioXCnPgDtQApWOL70724580 = -840338502;    int ioXCnPgDtQApWOL64634412 = -728009263;    int ioXCnPgDtQApWOL44861010 = -269346375;    int ioXCnPgDtQApWOL94149709 = -6036447;    int ioXCnPgDtQApWOL91985250 = -150454761;    int ioXCnPgDtQApWOL32109514 = -574302665;    int ioXCnPgDtQApWOL19722180 = -602661996;    int ioXCnPgDtQApWOL19471174 = -857980812;    int ioXCnPgDtQApWOL55135868 = -294138081;    int ioXCnPgDtQApWOL23466945 = 13538975;    int ioXCnPgDtQApWOL24919360 = -644915397;    int ioXCnPgDtQApWOL968641 = -397853092;    int ioXCnPgDtQApWOL30444552 = -940999961;    int ioXCnPgDtQApWOL23780010 = 22188427;    int ioXCnPgDtQApWOL7824744 = -938906216;    int ioXCnPgDtQApWOL50998430 = -311711167;    int ioXCnPgDtQApWOL1309788 = -929306510;    int ioXCnPgDtQApWOL33966283 = -392588935;    int ioXCnPgDtQApWOL85977981 = -903933484;    int ioXCnPgDtQApWOL39499591 = -970927390;    int ioXCnPgDtQApWOL11679650 = -957299317;    int ioXCnPgDtQApWOL35323760 = -1146389;     ioXCnPgDtQApWOL8298648 = ioXCnPgDtQApWOL1154585;     ioXCnPgDtQApWOL1154585 = ioXCnPgDtQApWOL80153673;     ioXCnPgDtQApWOL80153673 = ioXCnPgDtQApWOL57979477;     ioXCnPgDtQApWOL57979477 = ioXCnPgDtQApWOL82142200;     ioXCnPgDtQApWOL82142200 = ioXCnPgDtQApWOL52495729;     ioXCnPgDtQApWOL52495729 = ioXCnPgDtQApWOL46115397;     ioXCnPgDtQApWOL46115397 = ioXCnPgDtQApWOL89852150;     ioXCnPgDtQApWOL89852150 = ioXCnPgDtQApWOL7650649;     ioXCnPgDtQApWOL7650649 = ioXCnPgDtQApWOL68914616;     ioXCnPgDtQApWOL68914616 = ioXCnPgDtQApWOL93032978;     ioXCnPgDtQApWOL93032978 = ioXCnPgDtQApWOL13961528;     ioXCnPgDtQApWOL13961528 = ioXCnPgDtQApWOL3377688;     ioXCnPgDtQApWOL3377688 = ioXCnPgDtQApWOL90048550;     ioXCnPgDtQApWOL90048550 = ioXCnPgDtQApWOL94739695;     ioXCnPgDtQApWOL94739695 = ioXCnPgDtQApWOL8487666;     ioXCnPgDtQApWOL8487666 = ioXCnPgDtQApWOL78468967;     ioXCnPgDtQApWOL78468967 = ioXCnPgDtQApWOL75087686;     ioXCnPgDtQApWOL75087686 = ioXCnPgDtQApWOL36778832;     ioXCnPgDtQApWOL36778832 = ioXCnPgDtQApWOL30005736;     ioXCnPgDtQApWOL30005736 = ioXCnPgDtQApWOL2954292;     ioXCnPgDtQApWOL2954292 = ioXCnPgDtQApWOL2390219;     ioXCnPgDtQApWOL2390219 = ioXCnPgDtQApWOL46650031;     ioXCnPgDtQApWOL46650031 = ioXCnPgDtQApWOL93387566;     ioXCnPgDtQApWOL93387566 = ioXCnPgDtQApWOL50622194;     ioXCnPgDtQApWOL50622194 = ioXCnPgDtQApWOL15736873;     ioXCnPgDtQApWOL15736873 = ioXCnPgDtQApWOL69034354;     ioXCnPgDtQApWOL69034354 = ioXCnPgDtQApWOL84664494;     ioXCnPgDtQApWOL84664494 = ioXCnPgDtQApWOL51900733;     ioXCnPgDtQApWOL51900733 = ioXCnPgDtQApWOL17387967;     ioXCnPgDtQApWOL17387967 = ioXCnPgDtQApWOL12623845;     ioXCnPgDtQApWOL12623845 = ioXCnPgDtQApWOL5693753;     ioXCnPgDtQApWOL5693753 = ioXCnPgDtQApWOL18778620;     ioXCnPgDtQApWOL18778620 = ioXCnPgDtQApWOL60986450;     ioXCnPgDtQApWOL60986450 = ioXCnPgDtQApWOL6877070;     ioXCnPgDtQApWOL6877070 = ioXCnPgDtQApWOL84949105;     ioXCnPgDtQApWOL84949105 = ioXCnPgDtQApWOL10546117;     ioXCnPgDtQApWOL10546117 = ioXCnPgDtQApWOL15182027;     ioXCnPgDtQApWOL15182027 = ioXCnPgDtQApWOL17403759;     ioXCnPgDtQApWOL17403759 = ioXCnPgDtQApWOL84771162;     ioXCnPgDtQApWOL84771162 = ioXCnPgDtQApWOL95417185;     ioXCnPgDtQApWOL95417185 = ioXCnPgDtQApWOL23929398;     ioXCnPgDtQApWOL23929398 = ioXCnPgDtQApWOL29762273;     ioXCnPgDtQApWOL29762273 = ioXCnPgDtQApWOL67970422;     ioXCnPgDtQApWOL67970422 = ioXCnPgDtQApWOL22662986;     ioXCnPgDtQApWOL22662986 = ioXCnPgDtQApWOL85987782;     ioXCnPgDtQApWOL85987782 = ioXCnPgDtQApWOL70875862;     ioXCnPgDtQApWOL70875862 = ioXCnPgDtQApWOL74884645;     ioXCnPgDtQApWOL74884645 = ioXCnPgDtQApWOL92679243;     ioXCnPgDtQApWOL92679243 = ioXCnPgDtQApWOL19791220;     ioXCnPgDtQApWOL19791220 = ioXCnPgDtQApWOL97665787;     ioXCnPgDtQApWOL97665787 = ioXCnPgDtQApWOL93152670;     ioXCnPgDtQApWOL93152670 = ioXCnPgDtQApWOL50557884;     ioXCnPgDtQApWOL50557884 = ioXCnPgDtQApWOL95311674;     ioXCnPgDtQApWOL95311674 = ioXCnPgDtQApWOL36067091;     ioXCnPgDtQApWOL36067091 = ioXCnPgDtQApWOL5908429;     ioXCnPgDtQApWOL5908429 = ioXCnPgDtQApWOL54504554;     ioXCnPgDtQApWOL54504554 = ioXCnPgDtQApWOL86766107;     ioXCnPgDtQApWOL86766107 = ioXCnPgDtQApWOL7357284;     ioXCnPgDtQApWOL7357284 = ioXCnPgDtQApWOL66405328;     ioXCnPgDtQApWOL66405328 = ioXCnPgDtQApWOL83461374;     ioXCnPgDtQApWOL83461374 = ioXCnPgDtQApWOL61450902;     ioXCnPgDtQApWOL61450902 = ioXCnPgDtQApWOL37951417;     ioXCnPgDtQApWOL37951417 = ioXCnPgDtQApWOL90262681;     ioXCnPgDtQApWOL90262681 = ioXCnPgDtQApWOL56290772;     ioXCnPgDtQApWOL56290772 = ioXCnPgDtQApWOL87339226;     ioXCnPgDtQApWOL87339226 = ioXCnPgDtQApWOL95182908;     ioXCnPgDtQApWOL95182908 = ioXCnPgDtQApWOL42391238;     ioXCnPgDtQApWOL42391238 = ioXCnPgDtQApWOL83171480;     ioXCnPgDtQApWOL83171480 = ioXCnPgDtQApWOL9790590;     ioXCnPgDtQApWOL9790590 = ioXCnPgDtQApWOL97941549;     ioXCnPgDtQApWOL97941549 = ioXCnPgDtQApWOL63286940;     ioXCnPgDtQApWOL63286940 = ioXCnPgDtQApWOL57683928;     ioXCnPgDtQApWOL57683928 = ioXCnPgDtQApWOL52007669;     ioXCnPgDtQApWOL52007669 = ioXCnPgDtQApWOL34588551;     ioXCnPgDtQApWOL34588551 = ioXCnPgDtQApWOL79024894;     ioXCnPgDtQApWOL79024894 = ioXCnPgDtQApWOL72627946;     ioXCnPgDtQApWOL72627946 = ioXCnPgDtQApWOL78679608;     ioXCnPgDtQApWOL78679608 = ioXCnPgDtQApWOL70724580;     ioXCnPgDtQApWOL70724580 = ioXCnPgDtQApWOL64634412;     ioXCnPgDtQApWOL64634412 = ioXCnPgDtQApWOL44861010;     ioXCnPgDtQApWOL44861010 = ioXCnPgDtQApWOL94149709;     ioXCnPgDtQApWOL94149709 = ioXCnPgDtQApWOL91985250;     ioXCnPgDtQApWOL91985250 = ioXCnPgDtQApWOL32109514;     ioXCnPgDtQApWOL32109514 = ioXCnPgDtQApWOL19722180;     ioXCnPgDtQApWOL19722180 = ioXCnPgDtQApWOL19471174;     ioXCnPgDtQApWOL19471174 = ioXCnPgDtQApWOL55135868;     ioXCnPgDtQApWOL55135868 = ioXCnPgDtQApWOL23466945;     ioXCnPgDtQApWOL23466945 = ioXCnPgDtQApWOL24919360;     ioXCnPgDtQApWOL24919360 = ioXCnPgDtQApWOL968641;     ioXCnPgDtQApWOL968641 = ioXCnPgDtQApWOL30444552;     ioXCnPgDtQApWOL30444552 = ioXCnPgDtQApWOL23780010;     ioXCnPgDtQApWOL23780010 = ioXCnPgDtQApWOL7824744;     ioXCnPgDtQApWOL7824744 = ioXCnPgDtQApWOL50998430;     ioXCnPgDtQApWOL50998430 = ioXCnPgDtQApWOL1309788;     ioXCnPgDtQApWOL1309788 = ioXCnPgDtQApWOL33966283;     ioXCnPgDtQApWOL33966283 = ioXCnPgDtQApWOL85977981;     ioXCnPgDtQApWOL85977981 = ioXCnPgDtQApWOL39499591;     ioXCnPgDtQApWOL39499591 = ioXCnPgDtQApWOL11679650;     ioXCnPgDtQApWOL11679650 = ioXCnPgDtQApWOL35323760;     ioXCnPgDtQApWOL35323760 = ioXCnPgDtQApWOL8298648;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tgEyKQHQHurfIEAphxoACCEqeXWkx41210721() {     int CiOQHCKWQOZAZZs4811569 = 52923;    int CiOQHCKWQOZAZZs16237410 = -639357435;    int CiOQHCKWQOZAZZs35835985 = -839257958;    int CiOQHCKWQOZAZZs70528325 = -557038598;    int CiOQHCKWQOZAZZs50794590 = -315240620;    int CiOQHCKWQOZAZZs89742601 = -652360256;    int CiOQHCKWQOZAZZs19930050 = -774761813;    int CiOQHCKWQOZAZZs25304320 = -28831821;    int CiOQHCKWQOZAZZs11904905 = -584463390;    int CiOQHCKWQOZAZZs22919153 = -31261055;    int CiOQHCKWQOZAZZs53249094 = -609149927;    int CiOQHCKWQOZAZZs28977965 = -734658510;    int CiOQHCKWQOZAZZs8648386 = -9328865;    int CiOQHCKWQOZAZZs57123663 = 60430045;    int CiOQHCKWQOZAZZs99639452 = -614339211;    int CiOQHCKWQOZAZZs21444490 = -876192903;    int CiOQHCKWQOZAZZs6159816 = -148724877;    int CiOQHCKWQOZAZZs11223175 = -453265135;    int CiOQHCKWQOZAZZs18701808 = -713181782;    int CiOQHCKWQOZAZZs70730995 = -504509251;    int CiOQHCKWQOZAZZs10633147 = -856312662;    int CiOQHCKWQOZAZZs43291976 = -589142095;    int CiOQHCKWQOZAZZs36146993 = 11270954;    int CiOQHCKWQOZAZZs17212337 = -54886167;    int CiOQHCKWQOZAZZs28897202 = -752619347;    int CiOQHCKWQOZAZZs91761236 = -363518330;    int CiOQHCKWQOZAZZs16931040 = -217097692;    int CiOQHCKWQOZAZZs76914857 = -582215625;    int CiOQHCKWQOZAZZs34096705 = -307701292;    int CiOQHCKWQOZAZZs17642475 = -357931896;    int CiOQHCKWQOZAZZs21717924 = -278452106;    int CiOQHCKWQOZAZZs41443936 = -967594116;    int CiOQHCKWQOZAZZs36223058 = -153097223;    int CiOQHCKWQOZAZZs73646118 = -431100976;    int CiOQHCKWQOZAZZs16361909 = -402540604;    int CiOQHCKWQOZAZZs28620360 = 3063533;    int CiOQHCKWQOZAZZs91103083 = -483621960;    int CiOQHCKWQOZAZZs88309001 = -721069404;    int CiOQHCKWQOZAZZs66138065 = -128210475;    int CiOQHCKWQOZAZZs64288561 = -597571227;    int CiOQHCKWQOZAZZs27688180 = -607515418;    int CiOQHCKWQOZAZZs68122749 = -884998774;    int CiOQHCKWQOZAZZs17304209 = -806090544;    int CiOQHCKWQOZAZZs80178530 = -394984960;    int CiOQHCKWQOZAZZs88649701 = -787584599;    int CiOQHCKWQOZAZZs83668808 = -731512121;    int CiOQHCKWQOZAZZs67787955 = -93397341;    int CiOQHCKWQOZAZZs55374668 = -446973397;    int CiOQHCKWQOZAZZs11288312 = -207508960;    int CiOQHCKWQOZAZZs92592258 = -117560100;    int CiOQHCKWQOZAZZs49454427 = -919872264;    int CiOQHCKWQOZAZZs65561308 = -700668391;    int CiOQHCKWQOZAZZs42701262 = 66641340;    int CiOQHCKWQOZAZZs80818870 = -626740435;    int CiOQHCKWQOZAZZs30577574 = -298397373;    int CiOQHCKWQOZAZZs61519592 = -410804982;    int CiOQHCKWQOZAZZs80090416 = -550628390;    int CiOQHCKWQOZAZZs18623648 = -684371791;    int CiOQHCKWQOZAZZs41631124 = -804419252;    int CiOQHCKWQOZAZZs59033353 = -951722291;    int CiOQHCKWQOZAZZs72811562 = -335262565;    int CiOQHCKWQOZAZZs43015193 = -92546189;    int CiOQHCKWQOZAZZs91207615 = -721130529;    int CiOQHCKWQOZAZZs94262430 = -126531495;    int CiOQHCKWQOZAZZs1201230 = -752808949;    int CiOQHCKWQOZAZZs11805159 = -641555811;    int CiOQHCKWQOZAZZs92754906 = -481561287;    int CiOQHCKWQOZAZZs35002268 = -578227890;    int CiOQHCKWQOZAZZs40761755 = -537029351;    int CiOQHCKWQOZAZZs71019093 = -517402745;    int CiOQHCKWQOZAZZs30341406 = -292570944;    int CiOQHCKWQOZAZZs17850815 = -427655473;    int CiOQHCKWQOZAZZs45085110 = -225054660;    int CiOQHCKWQOZAZZs54413246 = -15610556;    int CiOQHCKWQOZAZZs43042815 = -896993834;    int CiOQHCKWQOZAZZs42510397 = -971313888;    int CiOQHCKWQOZAZZs25987768 = -783051551;    int CiOQHCKWQOZAZZs55968463 = -593744086;    int CiOQHCKWQOZAZZs28562636 = -267301569;    int CiOQHCKWQOZAZZs45228393 = 78892774;    int CiOQHCKWQOZAZZs23973282 = -170120989;    int CiOQHCKWQOZAZZs61556371 = -770124295;    int CiOQHCKWQOZAZZs65626545 = -274706665;    int CiOQHCKWQOZAZZs41504446 = -90141193;    int CiOQHCKWQOZAZZs68188047 = -438059632;    int CiOQHCKWQOZAZZs56156615 = -577783715;    int CiOQHCKWQOZAZZs98742673 = -934235456;    int CiOQHCKWQOZAZZs55404188 = -526356788;    int CiOQHCKWQOZAZZs43068544 = -32703604;    int CiOQHCKWQOZAZZs54842316 = -991735622;    int CiOQHCKWQOZAZZs48529943 = -446308078;    int CiOQHCKWQOZAZZs72479436 = -799250169;    int CiOQHCKWQOZAZZs46677877 = -916650153;    int CiOQHCKWQOZAZZs7104712 = -176488185;    int CiOQHCKWQOZAZZs91476999 = -162308663;    int CiOQHCKWQOZAZZs84672987 = -414969229;    int CiOQHCKWQOZAZZs76915134 = -63868245;    int CiOQHCKWQOZAZZs23041778 = -579559050;    int CiOQHCKWQOZAZZs78977301 = -642176011;    int CiOQHCKWQOZAZZs76844542 = 52923;     CiOQHCKWQOZAZZs4811569 = CiOQHCKWQOZAZZs16237410;     CiOQHCKWQOZAZZs16237410 = CiOQHCKWQOZAZZs35835985;     CiOQHCKWQOZAZZs35835985 = CiOQHCKWQOZAZZs70528325;     CiOQHCKWQOZAZZs70528325 = CiOQHCKWQOZAZZs50794590;     CiOQHCKWQOZAZZs50794590 = CiOQHCKWQOZAZZs89742601;     CiOQHCKWQOZAZZs89742601 = CiOQHCKWQOZAZZs19930050;     CiOQHCKWQOZAZZs19930050 = CiOQHCKWQOZAZZs25304320;     CiOQHCKWQOZAZZs25304320 = CiOQHCKWQOZAZZs11904905;     CiOQHCKWQOZAZZs11904905 = CiOQHCKWQOZAZZs22919153;     CiOQHCKWQOZAZZs22919153 = CiOQHCKWQOZAZZs53249094;     CiOQHCKWQOZAZZs53249094 = CiOQHCKWQOZAZZs28977965;     CiOQHCKWQOZAZZs28977965 = CiOQHCKWQOZAZZs8648386;     CiOQHCKWQOZAZZs8648386 = CiOQHCKWQOZAZZs57123663;     CiOQHCKWQOZAZZs57123663 = CiOQHCKWQOZAZZs99639452;     CiOQHCKWQOZAZZs99639452 = CiOQHCKWQOZAZZs21444490;     CiOQHCKWQOZAZZs21444490 = CiOQHCKWQOZAZZs6159816;     CiOQHCKWQOZAZZs6159816 = CiOQHCKWQOZAZZs11223175;     CiOQHCKWQOZAZZs11223175 = CiOQHCKWQOZAZZs18701808;     CiOQHCKWQOZAZZs18701808 = CiOQHCKWQOZAZZs70730995;     CiOQHCKWQOZAZZs70730995 = CiOQHCKWQOZAZZs10633147;     CiOQHCKWQOZAZZs10633147 = CiOQHCKWQOZAZZs43291976;     CiOQHCKWQOZAZZs43291976 = CiOQHCKWQOZAZZs36146993;     CiOQHCKWQOZAZZs36146993 = CiOQHCKWQOZAZZs17212337;     CiOQHCKWQOZAZZs17212337 = CiOQHCKWQOZAZZs28897202;     CiOQHCKWQOZAZZs28897202 = CiOQHCKWQOZAZZs91761236;     CiOQHCKWQOZAZZs91761236 = CiOQHCKWQOZAZZs16931040;     CiOQHCKWQOZAZZs16931040 = CiOQHCKWQOZAZZs76914857;     CiOQHCKWQOZAZZs76914857 = CiOQHCKWQOZAZZs34096705;     CiOQHCKWQOZAZZs34096705 = CiOQHCKWQOZAZZs17642475;     CiOQHCKWQOZAZZs17642475 = CiOQHCKWQOZAZZs21717924;     CiOQHCKWQOZAZZs21717924 = CiOQHCKWQOZAZZs41443936;     CiOQHCKWQOZAZZs41443936 = CiOQHCKWQOZAZZs36223058;     CiOQHCKWQOZAZZs36223058 = CiOQHCKWQOZAZZs73646118;     CiOQHCKWQOZAZZs73646118 = CiOQHCKWQOZAZZs16361909;     CiOQHCKWQOZAZZs16361909 = CiOQHCKWQOZAZZs28620360;     CiOQHCKWQOZAZZs28620360 = CiOQHCKWQOZAZZs91103083;     CiOQHCKWQOZAZZs91103083 = CiOQHCKWQOZAZZs88309001;     CiOQHCKWQOZAZZs88309001 = CiOQHCKWQOZAZZs66138065;     CiOQHCKWQOZAZZs66138065 = CiOQHCKWQOZAZZs64288561;     CiOQHCKWQOZAZZs64288561 = CiOQHCKWQOZAZZs27688180;     CiOQHCKWQOZAZZs27688180 = CiOQHCKWQOZAZZs68122749;     CiOQHCKWQOZAZZs68122749 = CiOQHCKWQOZAZZs17304209;     CiOQHCKWQOZAZZs17304209 = CiOQHCKWQOZAZZs80178530;     CiOQHCKWQOZAZZs80178530 = CiOQHCKWQOZAZZs88649701;     CiOQHCKWQOZAZZs88649701 = CiOQHCKWQOZAZZs83668808;     CiOQHCKWQOZAZZs83668808 = CiOQHCKWQOZAZZs67787955;     CiOQHCKWQOZAZZs67787955 = CiOQHCKWQOZAZZs55374668;     CiOQHCKWQOZAZZs55374668 = CiOQHCKWQOZAZZs11288312;     CiOQHCKWQOZAZZs11288312 = CiOQHCKWQOZAZZs92592258;     CiOQHCKWQOZAZZs92592258 = CiOQHCKWQOZAZZs49454427;     CiOQHCKWQOZAZZs49454427 = CiOQHCKWQOZAZZs65561308;     CiOQHCKWQOZAZZs65561308 = CiOQHCKWQOZAZZs42701262;     CiOQHCKWQOZAZZs42701262 = CiOQHCKWQOZAZZs80818870;     CiOQHCKWQOZAZZs80818870 = CiOQHCKWQOZAZZs30577574;     CiOQHCKWQOZAZZs30577574 = CiOQHCKWQOZAZZs61519592;     CiOQHCKWQOZAZZs61519592 = CiOQHCKWQOZAZZs80090416;     CiOQHCKWQOZAZZs80090416 = CiOQHCKWQOZAZZs18623648;     CiOQHCKWQOZAZZs18623648 = CiOQHCKWQOZAZZs41631124;     CiOQHCKWQOZAZZs41631124 = CiOQHCKWQOZAZZs59033353;     CiOQHCKWQOZAZZs59033353 = CiOQHCKWQOZAZZs72811562;     CiOQHCKWQOZAZZs72811562 = CiOQHCKWQOZAZZs43015193;     CiOQHCKWQOZAZZs43015193 = CiOQHCKWQOZAZZs91207615;     CiOQHCKWQOZAZZs91207615 = CiOQHCKWQOZAZZs94262430;     CiOQHCKWQOZAZZs94262430 = CiOQHCKWQOZAZZs1201230;     CiOQHCKWQOZAZZs1201230 = CiOQHCKWQOZAZZs11805159;     CiOQHCKWQOZAZZs11805159 = CiOQHCKWQOZAZZs92754906;     CiOQHCKWQOZAZZs92754906 = CiOQHCKWQOZAZZs35002268;     CiOQHCKWQOZAZZs35002268 = CiOQHCKWQOZAZZs40761755;     CiOQHCKWQOZAZZs40761755 = CiOQHCKWQOZAZZs71019093;     CiOQHCKWQOZAZZs71019093 = CiOQHCKWQOZAZZs30341406;     CiOQHCKWQOZAZZs30341406 = CiOQHCKWQOZAZZs17850815;     CiOQHCKWQOZAZZs17850815 = CiOQHCKWQOZAZZs45085110;     CiOQHCKWQOZAZZs45085110 = CiOQHCKWQOZAZZs54413246;     CiOQHCKWQOZAZZs54413246 = CiOQHCKWQOZAZZs43042815;     CiOQHCKWQOZAZZs43042815 = CiOQHCKWQOZAZZs42510397;     CiOQHCKWQOZAZZs42510397 = CiOQHCKWQOZAZZs25987768;     CiOQHCKWQOZAZZs25987768 = CiOQHCKWQOZAZZs55968463;     CiOQHCKWQOZAZZs55968463 = CiOQHCKWQOZAZZs28562636;     CiOQHCKWQOZAZZs28562636 = CiOQHCKWQOZAZZs45228393;     CiOQHCKWQOZAZZs45228393 = CiOQHCKWQOZAZZs23973282;     CiOQHCKWQOZAZZs23973282 = CiOQHCKWQOZAZZs61556371;     CiOQHCKWQOZAZZs61556371 = CiOQHCKWQOZAZZs65626545;     CiOQHCKWQOZAZZs65626545 = CiOQHCKWQOZAZZs41504446;     CiOQHCKWQOZAZZs41504446 = CiOQHCKWQOZAZZs68188047;     CiOQHCKWQOZAZZs68188047 = CiOQHCKWQOZAZZs56156615;     CiOQHCKWQOZAZZs56156615 = CiOQHCKWQOZAZZs98742673;     CiOQHCKWQOZAZZs98742673 = CiOQHCKWQOZAZZs55404188;     CiOQHCKWQOZAZZs55404188 = CiOQHCKWQOZAZZs43068544;     CiOQHCKWQOZAZZs43068544 = CiOQHCKWQOZAZZs54842316;     CiOQHCKWQOZAZZs54842316 = CiOQHCKWQOZAZZs48529943;     CiOQHCKWQOZAZZs48529943 = CiOQHCKWQOZAZZs72479436;     CiOQHCKWQOZAZZs72479436 = CiOQHCKWQOZAZZs46677877;     CiOQHCKWQOZAZZs46677877 = CiOQHCKWQOZAZZs7104712;     CiOQHCKWQOZAZZs7104712 = CiOQHCKWQOZAZZs91476999;     CiOQHCKWQOZAZZs91476999 = CiOQHCKWQOZAZZs84672987;     CiOQHCKWQOZAZZs84672987 = CiOQHCKWQOZAZZs76915134;     CiOQHCKWQOZAZZs76915134 = CiOQHCKWQOZAZZs23041778;     CiOQHCKWQOZAZZs23041778 = CiOQHCKWQOZAZZs78977301;     CiOQHCKWQOZAZZs78977301 = CiOQHCKWQOZAZZs76844542;     CiOQHCKWQOZAZZs76844542 = CiOQHCKWQOZAZZs4811569;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zpyxuWLttKVcYnhsuPavtrqpwncHR91157528() {     int iVhrcEJffTODaSB71982627 = -744372754;    int iVhrcEJffTODaSB61421436 = -421093911;    int iVhrcEJffTODaSB70174002 = 27473430;    int iVhrcEJffTODaSB37928600 = -295449948;    int iVhrcEJffTODaSB32057200 = -285878642;    int iVhrcEJffTODaSB82183803 = -685036172;    int iVhrcEJffTODaSB43576263 = -133919031;    int iVhrcEJffTODaSB48384820 = -784355778;    int iVhrcEJffTODaSB79284052 = -116582403;    int iVhrcEJffTODaSB3250637 = -354274747;    int iVhrcEJffTODaSB35393765 = -659815022;    int iVhrcEJffTODaSB75249296 = -783676846;    int iVhrcEJffTODaSB61893556 = -182993113;    int iVhrcEJffTODaSB54201590 = 95033352;    int iVhrcEJffTODaSB42087120 = -652703073;    int iVhrcEJffTODaSB92365406 = -972002055;    int iVhrcEJffTODaSB71850731 = -151142482;    int iVhrcEJffTODaSB44651624 = -662885774;    int iVhrcEJffTODaSB61904295 = -191514758;    int iVhrcEJffTODaSB98169200 = -344375935;    int iVhrcEJffTODaSB93778332 = -165075026;    int iVhrcEJffTODaSB67293232 = -788453210;    int iVhrcEJffTODaSB90709367 = -378534059;    int iVhrcEJffTODaSB31236536 = -825483955;    int iVhrcEJffTODaSB42853908 = -627255941;    int iVhrcEJffTODaSB81809105 = -732892109;    int iVhrcEJffTODaSB2866929 = -138657601;    int iVhrcEJffTODaSB41543920 = -134128503;    int iVhrcEJffTODaSB35756717 = -921367177;    int iVhrcEJffTODaSB80979208 = -605672435;    int iVhrcEJffTODaSB4471322 = 58239290;    int iVhrcEJffTODaSB36981722 = -136075538;    int iVhrcEJffTODaSB51160435 = -888211384;    int iVhrcEJffTODaSB51387924 = 61541072;    int iVhrcEJffTODaSB21512945 = -204838985;    int iVhrcEJffTODaSB89456682 = -273473977;    int iVhrcEJffTODaSB3510169 = -82985747;    int iVhrcEJffTODaSB44310785 = -331051802;    int iVhrcEJffTODaSB58092064 = -812326446;    int iVhrcEJffTODaSB67036366 = 57904214;    int iVhrcEJffTODaSB32176792 = -335132568;    int iVhrcEJffTODaSB55101432 = -968036723;    int iVhrcEJffTODaSB93529744 = -729986914;    int iVhrcEJffTODaSB39663041 = 48370758;    int iVhrcEJffTODaSB66572422 = -55125930;    int iVhrcEJffTODaSB47327702 = 89867599;    int iVhrcEJffTODaSB63133458 = -6707166;    int iVhrcEJffTODaSB26095321 = -453436336;    int iVhrcEJffTODaSB16680443 = -161013263;    int iVhrcEJffTODaSB53775361 = -63083548;    int iVhrcEJffTODaSB56173022 = -417268584;    int iVhrcEJffTODaSB65294194 = -869907615;    int iVhrcEJffTODaSB14471950 = -761011339;    int iVhrcEJffTODaSB46303100 = -105903085;    int iVhrcEJffTODaSB97379195 = 14839198;    int iVhrcEJffTODaSB4689396 = -955919544;    int iVhrcEJffTODaSB70712068 = 57440147;    int iVhrcEJffTODaSB38937467 = -147042615;    int iVhrcEJffTODaSB95074692 = -668194008;    int iVhrcEJffTODaSB50248095 = -552986533;    int iVhrcEJffTODaSB79316875 = -446378572;    int iVhrcEJffTODaSB2032343 = -999790529;    int iVhrcEJffTODaSB12628104 = -862988601;    int iVhrcEJffTODaSB98304844 = -510909968;    int iVhrcEJffTODaSB98779315 = -312514037;    int iVhrcEJffTODaSB98412043 = -423739484;    int iVhrcEJffTODaSB24088862 = -895465462;    int iVhrcEJffTODaSB10505633 = -144534186;    int iVhrcEJffTODaSB32688645 = -700127664;    int iVhrcEJffTODaSB52630438 = -279229096;    int iVhrcEJffTODaSB88855237 = -789016308;    int iVhrcEJffTODaSB27539947 = -820090681;    int iVhrcEJffTODaSB86559560 = -850559329;    int iVhrcEJffTODaSB94867929 = -149418973;    int iVhrcEJffTODaSB65992409 = 90756633;    int iVhrcEJffTODaSB38676900 = -197038303;    int iVhrcEJffTODaSB73763488 = 41533703;    int iVhrcEJffTODaSB51046327 = -326904818;    int iVhrcEJffTODaSB64664113 = -670358026;    int iVhrcEJffTODaSB95526205 = -617123540;    int iVhrcEJffTODaSB18675647 = -626184943;    int iVhrcEJffTODaSB76771607 = -685221265;    int iVhrcEJffTODaSB24863478 = -973115240;    int iVhrcEJffTODaSB81981355 = -758283630;    int iVhrcEJffTODaSB24806186 = -88403851;    int iVhrcEJffTODaSB39177128 = -71853096;    int iVhrcEJffTODaSB22509773 = -375064200;    int iVhrcEJffTODaSB4857335 = -682308300;    int iVhrcEJffTODaSB54008728 = -953298126;    int iVhrcEJffTODaSB16823550 = -248919441;    int iVhrcEJffTODaSB18744614 = -230914125;    int iVhrcEJffTODaSB64572702 = -935943133;    int iVhrcEJffTODaSB49236092 = -662857794;    int iVhrcEJffTODaSB7843970 = -159339913;    int iVhrcEJffTODaSB87719490 = -495717215;    int iVhrcEJffTODaSB30144449 = -335342040;    int iVhrcEJffTODaSB42473328 = -5048123;    int iVhrcEJffTODaSB95224900 = -119076947;    int iVhrcEJffTODaSB40883726 = -639115205;    int iVhrcEJffTODaSB68160379 = -744372754;     iVhrcEJffTODaSB71982627 = iVhrcEJffTODaSB61421436;     iVhrcEJffTODaSB61421436 = iVhrcEJffTODaSB70174002;     iVhrcEJffTODaSB70174002 = iVhrcEJffTODaSB37928600;     iVhrcEJffTODaSB37928600 = iVhrcEJffTODaSB32057200;     iVhrcEJffTODaSB32057200 = iVhrcEJffTODaSB82183803;     iVhrcEJffTODaSB82183803 = iVhrcEJffTODaSB43576263;     iVhrcEJffTODaSB43576263 = iVhrcEJffTODaSB48384820;     iVhrcEJffTODaSB48384820 = iVhrcEJffTODaSB79284052;     iVhrcEJffTODaSB79284052 = iVhrcEJffTODaSB3250637;     iVhrcEJffTODaSB3250637 = iVhrcEJffTODaSB35393765;     iVhrcEJffTODaSB35393765 = iVhrcEJffTODaSB75249296;     iVhrcEJffTODaSB75249296 = iVhrcEJffTODaSB61893556;     iVhrcEJffTODaSB61893556 = iVhrcEJffTODaSB54201590;     iVhrcEJffTODaSB54201590 = iVhrcEJffTODaSB42087120;     iVhrcEJffTODaSB42087120 = iVhrcEJffTODaSB92365406;     iVhrcEJffTODaSB92365406 = iVhrcEJffTODaSB71850731;     iVhrcEJffTODaSB71850731 = iVhrcEJffTODaSB44651624;     iVhrcEJffTODaSB44651624 = iVhrcEJffTODaSB61904295;     iVhrcEJffTODaSB61904295 = iVhrcEJffTODaSB98169200;     iVhrcEJffTODaSB98169200 = iVhrcEJffTODaSB93778332;     iVhrcEJffTODaSB93778332 = iVhrcEJffTODaSB67293232;     iVhrcEJffTODaSB67293232 = iVhrcEJffTODaSB90709367;     iVhrcEJffTODaSB90709367 = iVhrcEJffTODaSB31236536;     iVhrcEJffTODaSB31236536 = iVhrcEJffTODaSB42853908;     iVhrcEJffTODaSB42853908 = iVhrcEJffTODaSB81809105;     iVhrcEJffTODaSB81809105 = iVhrcEJffTODaSB2866929;     iVhrcEJffTODaSB2866929 = iVhrcEJffTODaSB41543920;     iVhrcEJffTODaSB41543920 = iVhrcEJffTODaSB35756717;     iVhrcEJffTODaSB35756717 = iVhrcEJffTODaSB80979208;     iVhrcEJffTODaSB80979208 = iVhrcEJffTODaSB4471322;     iVhrcEJffTODaSB4471322 = iVhrcEJffTODaSB36981722;     iVhrcEJffTODaSB36981722 = iVhrcEJffTODaSB51160435;     iVhrcEJffTODaSB51160435 = iVhrcEJffTODaSB51387924;     iVhrcEJffTODaSB51387924 = iVhrcEJffTODaSB21512945;     iVhrcEJffTODaSB21512945 = iVhrcEJffTODaSB89456682;     iVhrcEJffTODaSB89456682 = iVhrcEJffTODaSB3510169;     iVhrcEJffTODaSB3510169 = iVhrcEJffTODaSB44310785;     iVhrcEJffTODaSB44310785 = iVhrcEJffTODaSB58092064;     iVhrcEJffTODaSB58092064 = iVhrcEJffTODaSB67036366;     iVhrcEJffTODaSB67036366 = iVhrcEJffTODaSB32176792;     iVhrcEJffTODaSB32176792 = iVhrcEJffTODaSB55101432;     iVhrcEJffTODaSB55101432 = iVhrcEJffTODaSB93529744;     iVhrcEJffTODaSB93529744 = iVhrcEJffTODaSB39663041;     iVhrcEJffTODaSB39663041 = iVhrcEJffTODaSB66572422;     iVhrcEJffTODaSB66572422 = iVhrcEJffTODaSB47327702;     iVhrcEJffTODaSB47327702 = iVhrcEJffTODaSB63133458;     iVhrcEJffTODaSB63133458 = iVhrcEJffTODaSB26095321;     iVhrcEJffTODaSB26095321 = iVhrcEJffTODaSB16680443;     iVhrcEJffTODaSB16680443 = iVhrcEJffTODaSB53775361;     iVhrcEJffTODaSB53775361 = iVhrcEJffTODaSB56173022;     iVhrcEJffTODaSB56173022 = iVhrcEJffTODaSB65294194;     iVhrcEJffTODaSB65294194 = iVhrcEJffTODaSB14471950;     iVhrcEJffTODaSB14471950 = iVhrcEJffTODaSB46303100;     iVhrcEJffTODaSB46303100 = iVhrcEJffTODaSB97379195;     iVhrcEJffTODaSB97379195 = iVhrcEJffTODaSB4689396;     iVhrcEJffTODaSB4689396 = iVhrcEJffTODaSB70712068;     iVhrcEJffTODaSB70712068 = iVhrcEJffTODaSB38937467;     iVhrcEJffTODaSB38937467 = iVhrcEJffTODaSB95074692;     iVhrcEJffTODaSB95074692 = iVhrcEJffTODaSB50248095;     iVhrcEJffTODaSB50248095 = iVhrcEJffTODaSB79316875;     iVhrcEJffTODaSB79316875 = iVhrcEJffTODaSB2032343;     iVhrcEJffTODaSB2032343 = iVhrcEJffTODaSB12628104;     iVhrcEJffTODaSB12628104 = iVhrcEJffTODaSB98304844;     iVhrcEJffTODaSB98304844 = iVhrcEJffTODaSB98779315;     iVhrcEJffTODaSB98779315 = iVhrcEJffTODaSB98412043;     iVhrcEJffTODaSB98412043 = iVhrcEJffTODaSB24088862;     iVhrcEJffTODaSB24088862 = iVhrcEJffTODaSB10505633;     iVhrcEJffTODaSB10505633 = iVhrcEJffTODaSB32688645;     iVhrcEJffTODaSB32688645 = iVhrcEJffTODaSB52630438;     iVhrcEJffTODaSB52630438 = iVhrcEJffTODaSB88855237;     iVhrcEJffTODaSB88855237 = iVhrcEJffTODaSB27539947;     iVhrcEJffTODaSB27539947 = iVhrcEJffTODaSB86559560;     iVhrcEJffTODaSB86559560 = iVhrcEJffTODaSB94867929;     iVhrcEJffTODaSB94867929 = iVhrcEJffTODaSB65992409;     iVhrcEJffTODaSB65992409 = iVhrcEJffTODaSB38676900;     iVhrcEJffTODaSB38676900 = iVhrcEJffTODaSB73763488;     iVhrcEJffTODaSB73763488 = iVhrcEJffTODaSB51046327;     iVhrcEJffTODaSB51046327 = iVhrcEJffTODaSB64664113;     iVhrcEJffTODaSB64664113 = iVhrcEJffTODaSB95526205;     iVhrcEJffTODaSB95526205 = iVhrcEJffTODaSB18675647;     iVhrcEJffTODaSB18675647 = iVhrcEJffTODaSB76771607;     iVhrcEJffTODaSB76771607 = iVhrcEJffTODaSB24863478;     iVhrcEJffTODaSB24863478 = iVhrcEJffTODaSB81981355;     iVhrcEJffTODaSB81981355 = iVhrcEJffTODaSB24806186;     iVhrcEJffTODaSB24806186 = iVhrcEJffTODaSB39177128;     iVhrcEJffTODaSB39177128 = iVhrcEJffTODaSB22509773;     iVhrcEJffTODaSB22509773 = iVhrcEJffTODaSB4857335;     iVhrcEJffTODaSB4857335 = iVhrcEJffTODaSB54008728;     iVhrcEJffTODaSB54008728 = iVhrcEJffTODaSB16823550;     iVhrcEJffTODaSB16823550 = iVhrcEJffTODaSB18744614;     iVhrcEJffTODaSB18744614 = iVhrcEJffTODaSB64572702;     iVhrcEJffTODaSB64572702 = iVhrcEJffTODaSB49236092;     iVhrcEJffTODaSB49236092 = iVhrcEJffTODaSB7843970;     iVhrcEJffTODaSB7843970 = iVhrcEJffTODaSB87719490;     iVhrcEJffTODaSB87719490 = iVhrcEJffTODaSB30144449;     iVhrcEJffTODaSB30144449 = iVhrcEJffTODaSB42473328;     iVhrcEJffTODaSB42473328 = iVhrcEJffTODaSB95224900;     iVhrcEJffTODaSB95224900 = iVhrcEJffTODaSB40883726;     iVhrcEJffTODaSB40883726 = iVhrcEJffTODaSB68160379;     iVhrcEJffTODaSB68160379 = iVhrcEJffTODaSB71982627;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void dcnDZoaqRDoLhkMJRZHvaPCRmRuxy93346866() {     int BLeXvkEZEzAKYZm68495548 = -743173442;    int BLeXvkEZEzAKYZm76504261 = -373782213;    int BLeXvkEZEzAKYZm25856314 = -820629335;    int BLeXvkEZEzAKYZm50477448 = -764263738;    int BLeXvkEZEzAKYZm709590 = -941465638;    int BLeXvkEZEzAKYZm19430676 = -715563748;    int BLeXvkEZEzAKYZm17390916 = -298837006;    int BLeXvkEZEzAKYZm83836990 = -897239962;    int BLeXvkEZEzAKYZm83538309 = -369000156;    int BLeXvkEZEzAKYZm57255173 = -12999960;    int BLeXvkEZEzAKYZm95609880 = -635050259;    int BLeXvkEZEzAKYZm90265733 = -252409752;    int BLeXvkEZEzAKYZm67164254 = -468778776;    int BLeXvkEZEzAKYZm21276703 = -414488183;    int BLeXvkEZEzAKYZm46986877 = -504029783;    int BLeXvkEZEzAKYZm5322230 = -730623270;    int BLeXvkEZEzAKYZm99541580 = -63989527;    int BLeXvkEZEzAKYZm80787112 = -180411840;    int BLeXvkEZEzAKYZm43827272 = -680642058;    int BLeXvkEZEzAKYZm38894460 = -253900765;    int BLeXvkEZEzAKYZm1457188 = -375624624;    int BLeXvkEZEzAKYZm8194990 = -66470903;    int BLeXvkEZEzAKYZm80206330 = -659453769;    int BLeXvkEZEzAKYZm55061306 = -784601360;    int BLeXvkEZEzAKYZm21128915 = -152681700;    int BLeXvkEZEzAKYZm57833470 = -912187372;    int BLeXvkEZEzAKYZm50763613 = 34160552;    int BLeXvkEZEzAKYZm33794283 = -889884602;    int BLeXvkEZEzAKYZm17952688 = -786745160;    int BLeXvkEZEzAKYZm81233715 = 29550722;    int BLeXvkEZEzAKYZm13565401 = -243728864;    int BLeXvkEZEzAKYZm72731905 = -468081558;    int BLeXvkEZEzAKYZm68604873 = -778308152;    int BLeXvkEZEzAKYZm64047593 = -858968927;    int BLeXvkEZEzAKYZm30997784 = -419504510;    int BLeXvkEZEzAKYZm33127936 = -550550686;    int BLeXvkEZEzAKYZm84067135 = -693409705;    int BLeXvkEZEzAKYZm17437759 = -152183770;    int BLeXvkEZEzAKYZm6826371 = -53395196;    int BLeXvkEZEzAKYZm46553765 = -478443668;    int BLeXvkEZEzAKYZm64447786 = -866674739;    int BLeXvkEZEzAKYZm99294784 = -375477686;    int BLeXvkEZEzAKYZm81071680 = -126259486;    int BLeXvkEZEzAKYZm51871149 = -93262995;    int BLeXvkEZEzAKYZm32559138 = -587280268;    int BLeXvkEZEzAKYZm45008729 = -242460197;    int BLeXvkEZEzAKYZm60045551 = -285227814;    int BLeXvkEZEzAKYZm6585344 = -616530335;    int BLeXvkEZEzAKYZm35289511 = -792517458;    int BLeXvkEZEzAKYZm26576400 = -412623003;    int BLeXvkEZEzAKYZm7961663 = 53352209;    int BLeXvkEZEzAKYZm37702832 = -352072865;    int BLeXvkEZEzAKYZm6615328 = -452919983;    int BLeXvkEZEzAKYZm31810296 = -556104089;    int BLeXvkEZEzAKYZm91889679 = -417882594;    int BLeXvkEZEzAKYZm60300558 = -576702540;    int BLeXvkEZEzAKYZm96297931 = -714328444;    int BLeXvkEZEzAKYZm70795007 = 63972024;    int BLeXvkEZEzAKYZm29348533 = -511582039;    int BLeXvkEZEzAKYZm42876120 = 70721733;    int BLeXvkEZEzAKYZm68667063 = -649724301;    int BLeXvkEZEzAKYZm83596633 = -408952405;    int BLeXvkEZEzAKYZm65884302 = -10494802;    int BLeXvkEZEzAKYZm2304594 = -298550879;    int BLeXvkEZEzAKYZm43689772 = -769271097;    int BLeXvkEZEzAKYZm22877976 = -66968701;    int BLeXvkEZEzAKYZm21660860 = -474101600;    int BLeXvkEZEzAKYZm3116662 = -609809849;    int BLeXvkEZEzAKYZm90278919 = -994983673;    int BLeXvkEZEzAKYZm13858942 = -953479098;    int BLeXvkEZEzAKYZm21255095 = 62786434;    int BLeXvkEZEzAKYZm82103821 = -911805757;    int BLeXvkEZEzAKYZm73960741 = -27016644;    int BLeXvkEZEzAKYZm97273506 = -102198391;    int BLeXvkEZEzAKYZm74446673 = -387226027;    int BLeXvkEZEzAKYZm2162403 = 99853061;    int BLeXvkEZEzAKYZm27123309 = -940211417;    int BLeXvkEZEzAKYZm28335182 = -466190774;    int BLeXvkEZEzAKYZm22502169 = -97321093;    int BLeXvkEZEzAKYZm76120186 = -910221503;    int BLeXvkEZEzAKYZm97787918 = -526959558;    int BLeXvkEZEzAKYZm44178269 = -349309113;    int BLeXvkEZEzAKYZm98504772 = 2632856;    int BLeXvkEZEzAKYZm91376287 = -274122158;    int BLeXvkEZEzAKYZm73272053 = 76198513;    int BLeXvkEZEzAKYZm75862568 = -891655999;    int BLeXvkEZEzAKYZm66116577 = 84838424;    int BLeXvkEZEzAKYZm36794578 = -122204064;    int BLeXvkEZEzAKYZm72157913 = -341086334;    int BLeXvkEZEzAKYZm70697225 = -842801971;    int BLeXvkEZEzAKYZm36830005 = -836222242;    int BLeXvkEZEzAKYZm13272129 = -657381729;    int BLeXvkEZEzAKYZm88089225 = -640601732;    int BLeXvkEZEzAKYZm63950251 = -24116930;    int BLeXvkEZEzAKYZm77886702 = -828719367;    int BLeXvkEZEzAKYZm80851153 = -357722335;    int BLeXvkEZEzAKYZm33410482 = -264982884;    int BLeXvkEZEzAKYZm78767087 = -827708608;    int BLeXvkEZEzAKYZm8181377 = -323991899;    int BLeXvkEZEzAKYZm9681162 = -743173442;     BLeXvkEZEzAKYZm68495548 = BLeXvkEZEzAKYZm76504261;     BLeXvkEZEzAKYZm76504261 = BLeXvkEZEzAKYZm25856314;     BLeXvkEZEzAKYZm25856314 = BLeXvkEZEzAKYZm50477448;     BLeXvkEZEzAKYZm50477448 = BLeXvkEZEzAKYZm709590;     BLeXvkEZEzAKYZm709590 = BLeXvkEZEzAKYZm19430676;     BLeXvkEZEzAKYZm19430676 = BLeXvkEZEzAKYZm17390916;     BLeXvkEZEzAKYZm17390916 = BLeXvkEZEzAKYZm83836990;     BLeXvkEZEzAKYZm83836990 = BLeXvkEZEzAKYZm83538309;     BLeXvkEZEzAKYZm83538309 = BLeXvkEZEzAKYZm57255173;     BLeXvkEZEzAKYZm57255173 = BLeXvkEZEzAKYZm95609880;     BLeXvkEZEzAKYZm95609880 = BLeXvkEZEzAKYZm90265733;     BLeXvkEZEzAKYZm90265733 = BLeXvkEZEzAKYZm67164254;     BLeXvkEZEzAKYZm67164254 = BLeXvkEZEzAKYZm21276703;     BLeXvkEZEzAKYZm21276703 = BLeXvkEZEzAKYZm46986877;     BLeXvkEZEzAKYZm46986877 = BLeXvkEZEzAKYZm5322230;     BLeXvkEZEzAKYZm5322230 = BLeXvkEZEzAKYZm99541580;     BLeXvkEZEzAKYZm99541580 = BLeXvkEZEzAKYZm80787112;     BLeXvkEZEzAKYZm80787112 = BLeXvkEZEzAKYZm43827272;     BLeXvkEZEzAKYZm43827272 = BLeXvkEZEzAKYZm38894460;     BLeXvkEZEzAKYZm38894460 = BLeXvkEZEzAKYZm1457188;     BLeXvkEZEzAKYZm1457188 = BLeXvkEZEzAKYZm8194990;     BLeXvkEZEzAKYZm8194990 = BLeXvkEZEzAKYZm80206330;     BLeXvkEZEzAKYZm80206330 = BLeXvkEZEzAKYZm55061306;     BLeXvkEZEzAKYZm55061306 = BLeXvkEZEzAKYZm21128915;     BLeXvkEZEzAKYZm21128915 = BLeXvkEZEzAKYZm57833470;     BLeXvkEZEzAKYZm57833470 = BLeXvkEZEzAKYZm50763613;     BLeXvkEZEzAKYZm50763613 = BLeXvkEZEzAKYZm33794283;     BLeXvkEZEzAKYZm33794283 = BLeXvkEZEzAKYZm17952688;     BLeXvkEZEzAKYZm17952688 = BLeXvkEZEzAKYZm81233715;     BLeXvkEZEzAKYZm81233715 = BLeXvkEZEzAKYZm13565401;     BLeXvkEZEzAKYZm13565401 = BLeXvkEZEzAKYZm72731905;     BLeXvkEZEzAKYZm72731905 = BLeXvkEZEzAKYZm68604873;     BLeXvkEZEzAKYZm68604873 = BLeXvkEZEzAKYZm64047593;     BLeXvkEZEzAKYZm64047593 = BLeXvkEZEzAKYZm30997784;     BLeXvkEZEzAKYZm30997784 = BLeXvkEZEzAKYZm33127936;     BLeXvkEZEzAKYZm33127936 = BLeXvkEZEzAKYZm84067135;     BLeXvkEZEzAKYZm84067135 = BLeXvkEZEzAKYZm17437759;     BLeXvkEZEzAKYZm17437759 = BLeXvkEZEzAKYZm6826371;     BLeXvkEZEzAKYZm6826371 = BLeXvkEZEzAKYZm46553765;     BLeXvkEZEzAKYZm46553765 = BLeXvkEZEzAKYZm64447786;     BLeXvkEZEzAKYZm64447786 = BLeXvkEZEzAKYZm99294784;     BLeXvkEZEzAKYZm99294784 = BLeXvkEZEzAKYZm81071680;     BLeXvkEZEzAKYZm81071680 = BLeXvkEZEzAKYZm51871149;     BLeXvkEZEzAKYZm51871149 = BLeXvkEZEzAKYZm32559138;     BLeXvkEZEzAKYZm32559138 = BLeXvkEZEzAKYZm45008729;     BLeXvkEZEzAKYZm45008729 = BLeXvkEZEzAKYZm60045551;     BLeXvkEZEzAKYZm60045551 = BLeXvkEZEzAKYZm6585344;     BLeXvkEZEzAKYZm6585344 = BLeXvkEZEzAKYZm35289511;     BLeXvkEZEzAKYZm35289511 = BLeXvkEZEzAKYZm26576400;     BLeXvkEZEzAKYZm26576400 = BLeXvkEZEzAKYZm7961663;     BLeXvkEZEzAKYZm7961663 = BLeXvkEZEzAKYZm37702832;     BLeXvkEZEzAKYZm37702832 = BLeXvkEZEzAKYZm6615328;     BLeXvkEZEzAKYZm6615328 = BLeXvkEZEzAKYZm31810296;     BLeXvkEZEzAKYZm31810296 = BLeXvkEZEzAKYZm91889679;     BLeXvkEZEzAKYZm91889679 = BLeXvkEZEzAKYZm60300558;     BLeXvkEZEzAKYZm60300558 = BLeXvkEZEzAKYZm96297931;     BLeXvkEZEzAKYZm96297931 = BLeXvkEZEzAKYZm70795007;     BLeXvkEZEzAKYZm70795007 = BLeXvkEZEzAKYZm29348533;     BLeXvkEZEzAKYZm29348533 = BLeXvkEZEzAKYZm42876120;     BLeXvkEZEzAKYZm42876120 = BLeXvkEZEzAKYZm68667063;     BLeXvkEZEzAKYZm68667063 = BLeXvkEZEzAKYZm83596633;     BLeXvkEZEzAKYZm83596633 = BLeXvkEZEzAKYZm65884302;     BLeXvkEZEzAKYZm65884302 = BLeXvkEZEzAKYZm2304594;     BLeXvkEZEzAKYZm2304594 = BLeXvkEZEzAKYZm43689772;     BLeXvkEZEzAKYZm43689772 = BLeXvkEZEzAKYZm22877976;     BLeXvkEZEzAKYZm22877976 = BLeXvkEZEzAKYZm21660860;     BLeXvkEZEzAKYZm21660860 = BLeXvkEZEzAKYZm3116662;     BLeXvkEZEzAKYZm3116662 = BLeXvkEZEzAKYZm90278919;     BLeXvkEZEzAKYZm90278919 = BLeXvkEZEzAKYZm13858942;     BLeXvkEZEzAKYZm13858942 = BLeXvkEZEzAKYZm21255095;     BLeXvkEZEzAKYZm21255095 = BLeXvkEZEzAKYZm82103821;     BLeXvkEZEzAKYZm82103821 = BLeXvkEZEzAKYZm73960741;     BLeXvkEZEzAKYZm73960741 = BLeXvkEZEzAKYZm97273506;     BLeXvkEZEzAKYZm97273506 = BLeXvkEZEzAKYZm74446673;     BLeXvkEZEzAKYZm74446673 = BLeXvkEZEzAKYZm2162403;     BLeXvkEZEzAKYZm2162403 = BLeXvkEZEzAKYZm27123309;     BLeXvkEZEzAKYZm27123309 = BLeXvkEZEzAKYZm28335182;     BLeXvkEZEzAKYZm28335182 = BLeXvkEZEzAKYZm22502169;     BLeXvkEZEzAKYZm22502169 = BLeXvkEZEzAKYZm76120186;     BLeXvkEZEzAKYZm76120186 = BLeXvkEZEzAKYZm97787918;     BLeXvkEZEzAKYZm97787918 = BLeXvkEZEzAKYZm44178269;     BLeXvkEZEzAKYZm44178269 = BLeXvkEZEzAKYZm98504772;     BLeXvkEZEzAKYZm98504772 = BLeXvkEZEzAKYZm91376287;     BLeXvkEZEzAKYZm91376287 = BLeXvkEZEzAKYZm73272053;     BLeXvkEZEzAKYZm73272053 = BLeXvkEZEzAKYZm75862568;     BLeXvkEZEzAKYZm75862568 = BLeXvkEZEzAKYZm66116577;     BLeXvkEZEzAKYZm66116577 = BLeXvkEZEzAKYZm36794578;     BLeXvkEZEzAKYZm36794578 = BLeXvkEZEzAKYZm72157913;     BLeXvkEZEzAKYZm72157913 = BLeXvkEZEzAKYZm70697225;     BLeXvkEZEzAKYZm70697225 = BLeXvkEZEzAKYZm36830005;     BLeXvkEZEzAKYZm36830005 = BLeXvkEZEzAKYZm13272129;     BLeXvkEZEzAKYZm13272129 = BLeXvkEZEzAKYZm88089225;     BLeXvkEZEzAKYZm88089225 = BLeXvkEZEzAKYZm63950251;     BLeXvkEZEzAKYZm63950251 = BLeXvkEZEzAKYZm77886702;     BLeXvkEZEzAKYZm77886702 = BLeXvkEZEzAKYZm80851153;     BLeXvkEZEzAKYZm80851153 = BLeXvkEZEzAKYZm33410482;     BLeXvkEZEzAKYZm33410482 = BLeXvkEZEzAKYZm78767087;     BLeXvkEZEzAKYZm78767087 = BLeXvkEZEzAKYZm8181377;     BLeXvkEZEzAKYZm8181377 = BLeXvkEZEzAKYZm9681162;     BLeXvkEZEzAKYZm9681162 = BLeXvkEZEzAKYZm68495548;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mvPTztdGSmYyqoNRpeNNrloyHodBx95536204() {     int tIQiYzEEeELMxHB65008469 = -741974131;    int tIQiYzEEeELMxHB91587087 = -326470515;    int tIQiYzEEeELMxHB81538624 = -568732101;    int tIQiYzEEeELMxHB63026296 = -133077529;    int tIQiYzEEeELMxHB69361979 = -497052633;    int tIQiYzEEeELMxHB56677548 = -746091325;    int tIQiYzEEeELMxHB91205569 = -463754981;    int tIQiYzEEeELMxHB19289161 = 89875854;    int tIQiYzEEeELMxHB87792565 = -621417910;    int tIQiYzEEeELMxHB11259710 = -771725173;    int tIQiYzEEeELMxHB55825996 = -610285496;    int tIQiYzEEeELMxHB5282171 = -821142657;    int tIQiYzEEeELMxHB72434952 = -754564440;    int tIQiYzEEeELMxHB88351816 = -924009718;    int tIQiYzEEeELMxHB51886634 = -355356493;    int tIQiYzEEeELMxHB18279054 = -489244485;    int tIQiYzEEeELMxHB27232429 = 23163428;    int tIQiYzEEeELMxHB16922601 = -797937905;    int tIQiYzEEeELMxHB25750248 = -69769359;    int tIQiYzEEeELMxHB79619719 = -163425596;    int tIQiYzEEeELMxHB9136042 = -586174223;    int tIQiYzEEeELMxHB49096747 = -444488596;    int tIQiYzEEeELMxHB69703293 = -940373480;    int tIQiYzEEeELMxHB78886076 = -743718764;    int tIQiYzEEeELMxHB99403921 = -778107459;    int tIQiYzEEeELMxHB33857834 = 8517366;    int tIQiYzEEeELMxHB98660297 = -893021294;    int tIQiYzEEeELMxHB26044645 = -545640701;    int tIQiYzEEeELMxHB148660 = -652123143;    int tIQiYzEEeELMxHB81488223 = -435226120;    int tIQiYzEEeELMxHB22659481 = -545697017;    int tIQiYzEEeELMxHB8482089 = -800087579;    int tIQiYzEEeELMxHB86049312 = -668404920;    int tIQiYzEEeELMxHB76707261 = -679478927;    int tIQiYzEEeELMxHB40482622 = -634170036;    int tIQiYzEEeELMxHB76799189 = -827627394;    int tIQiYzEEeELMxHB64624102 = -203833662;    int tIQiYzEEeELMxHB90564733 = 26684261;    int tIQiYzEEeELMxHB55560678 = -394463947;    int tIQiYzEEeELMxHB26071165 = 85208450;    int tIQiYzEEeELMxHB96718781 = -298216910;    int tIQiYzEEeELMxHB43488136 = -882918648;    int tIQiYzEEeELMxHB68613616 = -622532059;    int tIQiYzEEeELMxHB64079257 = -234896749;    int tIQiYzEEeELMxHB98545852 = -19434605;    int tIQiYzEEeELMxHB42689755 = -574787993;    int tIQiYzEEeELMxHB56957644 = -563748463;    int tIQiYzEEeELMxHB87075367 = -779624334;    int tIQiYzEEeELMxHB53898578 = -324021653;    int tIQiYzEEeELMxHB99377439 = -762162458;    int tIQiYzEEeELMxHB59750303 = -576026997;    int tIQiYzEEeELMxHB10111471 = -934238115;    int tIQiYzEEeELMxHB98758706 = -144828627;    int tIQiYzEEeELMxHB17317492 = 93694907;    int tIQiYzEEeELMxHB86400163 = -850604387;    int tIQiYzEEeELMxHB15911722 = -197485535;    int tIQiYzEEeELMxHB21883794 = -386097036;    int tIQiYzEEeELMxHB2652548 = -825013338;    int tIQiYzEEeELMxHB63622374 = -354970070;    int tIQiYzEEeELMxHB35504146 = -405570000;    int tIQiYzEEeELMxHB58017250 = -853070031;    int tIQiYzEEeELMxHB65160924 = -918114281;    int tIQiYzEEeELMxHB19140502 = -258001003;    int tIQiYzEEeELMxHB6304343 = -86191791;    int tIQiYzEEeELMxHB88600228 = -126028156;    int tIQiYzEEeELMxHB47343908 = -810197918;    int tIQiYzEEeELMxHB19232858 = -52737738;    int tIQiYzEEeELMxHB95727690 = 24914487;    int tIQiYzEEeELMxHB47869194 = -189839683;    int tIQiYzEEeELMxHB75087444 = -527729099;    int tIQiYzEEeELMxHB53654951 = -185410823;    int tIQiYzEEeELMxHB36667696 = 96479167;    int tIQiYzEEeELMxHB61361923 = -303473958;    int tIQiYzEEeELMxHB99679083 = -54977809;    int tIQiYzEEeELMxHB82900937 = -865208687;    int tIQiYzEEeELMxHB65647906 = -703255575;    int tIQiYzEEeELMxHB80483130 = -821956538;    int tIQiYzEEeELMxHB5624036 = -605476731;    int tIQiYzEEeELMxHB80340224 = -624284159;    int tIQiYzEEeELMxHB56714167 = -103319467;    int tIQiYzEEeELMxHB76900190 = -427734172;    int tIQiYzEEeELMxHB11584931 = -13396961;    int tIQiYzEEeELMxHB72146066 = -121619048;    int tIQiYzEEeELMxHB771221 = -889960686;    int tIQiYzEEeELMxHB21737921 = -859199123;    int tIQiYzEEeELMxHB12548010 = -611458902;    int tIQiYzEEeELMxHB9723383 = -555258952;    int tIQiYzEEeELMxHB68731821 = -662099827;    int tIQiYzEEeELMxHB90307098 = -828874541;    int tIQiYzEEeELMxHB24570900 = -336684501;    int tIQiYzEEeELMxHB54915395 = -341530358;    int tIQiYzEEeELMxHB61971554 = -378820325;    int tIQiYzEEeELMxHB26942359 = -618345669;    int tIQiYzEEeELMxHB20056533 = -988893948;    int tIQiYzEEeELMxHB68053914 = -61721520;    int tIQiYzEEeELMxHB31557858 = -380102629;    int tIQiYzEEeELMxHB24347635 = -524917646;    int tIQiYzEEeELMxHB62309274 = -436340268;    int tIQiYzEEeELMxHB75479028 = -8868593;    int tIQiYzEEeELMxHB51201945 = -741974131;     tIQiYzEEeELMxHB65008469 = tIQiYzEEeELMxHB91587087;     tIQiYzEEeELMxHB91587087 = tIQiYzEEeELMxHB81538624;     tIQiYzEEeELMxHB81538624 = tIQiYzEEeELMxHB63026296;     tIQiYzEEeELMxHB63026296 = tIQiYzEEeELMxHB69361979;     tIQiYzEEeELMxHB69361979 = tIQiYzEEeELMxHB56677548;     tIQiYzEEeELMxHB56677548 = tIQiYzEEeELMxHB91205569;     tIQiYzEEeELMxHB91205569 = tIQiYzEEeELMxHB19289161;     tIQiYzEEeELMxHB19289161 = tIQiYzEEeELMxHB87792565;     tIQiYzEEeELMxHB87792565 = tIQiYzEEeELMxHB11259710;     tIQiYzEEeELMxHB11259710 = tIQiYzEEeELMxHB55825996;     tIQiYzEEeELMxHB55825996 = tIQiYzEEeELMxHB5282171;     tIQiYzEEeELMxHB5282171 = tIQiYzEEeELMxHB72434952;     tIQiYzEEeELMxHB72434952 = tIQiYzEEeELMxHB88351816;     tIQiYzEEeELMxHB88351816 = tIQiYzEEeELMxHB51886634;     tIQiYzEEeELMxHB51886634 = tIQiYzEEeELMxHB18279054;     tIQiYzEEeELMxHB18279054 = tIQiYzEEeELMxHB27232429;     tIQiYzEEeELMxHB27232429 = tIQiYzEEeELMxHB16922601;     tIQiYzEEeELMxHB16922601 = tIQiYzEEeELMxHB25750248;     tIQiYzEEeELMxHB25750248 = tIQiYzEEeELMxHB79619719;     tIQiYzEEeELMxHB79619719 = tIQiYzEEeELMxHB9136042;     tIQiYzEEeELMxHB9136042 = tIQiYzEEeELMxHB49096747;     tIQiYzEEeELMxHB49096747 = tIQiYzEEeELMxHB69703293;     tIQiYzEEeELMxHB69703293 = tIQiYzEEeELMxHB78886076;     tIQiYzEEeELMxHB78886076 = tIQiYzEEeELMxHB99403921;     tIQiYzEEeELMxHB99403921 = tIQiYzEEeELMxHB33857834;     tIQiYzEEeELMxHB33857834 = tIQiYzEEeELMxHB98660297;     tIQiYzEEeELMxHB98660297 = tIQiYzEEeELMxHB26044645;     tIQiYzEEeELMxHB26044645 = tIQiYzEEeELMxHB148660;     tIQiYzEEeELMxHB148660 = tIQiYzEEeELMxHB81488223;     tIQiYzEEeELMxHB81488223 = tIQiYzEEeELMxHB22659481;     tIQiYzEEeELMxHB22659481 = tIQiYzEEeELMxHB8482089;     tIQiYzEEeELMxHB8482089 = tIQiYzEEeELMxHB86049312;     tIQiYzEEeELMxHB86049312 = tIQiYzEEeELMxHB76707261;     tIQiYzEEeELMxHB76707261 = tIQiYzEEeELMxHB40482622;     tIQiYzEEeELMxHB40482622 = tIQiYzEEeELMxHB76799189;     tIQiYzEEeELMxHB76799189 = tIQiYzEEeELMxHB64624102;     tIQiYzEEeELMxHB64624102 = tIQiYzEEeELMxHB90564733;     tIQiYzEEeELMxHB90564733 = tIQiYzEEeELMxHB55560678;     tIQiYzEEeELMxHB55560678 = tIQiYzEEeELMxHB26071165;     tIQiYzEEeELMxHB26071165 = tIQiYzEEeELMxHB96718781;     tIQiYzEEeELMxHB96718781 = tIQiYzEEeELMxHB43488136;     tIQiYzEEeELMxHB43488136 = tIQiYzEEeELMxHB68613616;     tIQiYzEEeELMxHB68613616 = tIQiYzEEeELMxHB64079257;     tIQiYzEEeELMxHB64079257 = tIQiYzEEeELMxHB98545852;     tIQiYzEEeELMxHB98545852 = tIQiYzEEeELMxHB42689755;     tIQiYzEEeELMxHB42689755 = tIQiYzEEeELMxHB56957644;     tIQiYzEEeELMxHB56957644 = tIQiYzEEeELMxHB87075367;     tIQiYzEEeELMxHB87075367 = tIQiYzEEeELMxHB53898578;     tIQiYzEEeELMxHB53898578 = tIQiYzEEeELMxHB99377439;     tIQiYzEEeELMxHB99377439 = tIQiYzEEeELMxHB59750303;     tIQiYzEEeELMxHB59750303 = tIQiYzEEeELMxHB10111471;     tIQiYzEEeELMxHB10111471 = tIQiYzEEeELMxHB98758706;     tIQiYzEEeELMxHB98758706 = tIQiYzEEeELMxHB17317492;     tIQiYzEEeELMxHB17317492 = tIQiYzEEeELMxHB86400163;     tIQiYzEEeELMxHB86400163 = tIQiYzEEeELMxHB15911722;     tIQiYzEEeELMxHB15911722 = tIQiYzEEeELMxHB21883794;     tIQiYzEEeELMxHB21883794 = tIQiYzEEeELMxHB2652548;     tIQiYzEEeELMxHB2652548 = tIQiYzEEeELMxHB63622374;     tIQiYzEEeELMxHB63622374 = tIQiYzEEeELMxHB35504146;     tIQiYzEEeELMxHB35504146 = tIQiYzEEeELMxHB58017250;     tIQiYzEEeELMxHB58017250 = tIQiYzEEeELMxHB65160924;     tIQiYzEEeELMxHB65160924 = tIQiYzEEeELMxHB19140502;     tIQiYzEEeELMxHB19140502 = tIQiYzEEeELMxHB6304343;     tIQiYzEEeELMxHB6304343 = tIQiYzEEeELMxHB88600228;     tIQiYzEEeELMxHB88600228 = tIQiYzEEeELMxHB47343908;     tIQiYzEEeELMxHB47343908 = tIQiYzEEeELMxHB19232858;     tIQiYzEEeELMxHB19232858 = tIQiYzEEeELMxHB95727690;     tIQiYzEEeELMxHB95727690 = tIQiYzEEeELMxHB47869194;     tIQiYzEEeELMxHB47869194 = tIQiYzEEeELMxHB75087444;     tIQiYzEEeELMxHB75087444 = tIQiYzEEeELMxHB53654951;     tIQiYzEEeELMxHB53654951 = tIQiYzEEeELMxHB36667696;     tIQiYzEEeELMxHB36667696 = tIQiYzEEeELMxHB61361923;     tIQiYzEEeELMxHB61361923 = tIQiYzEEeELMxHB99679083;     tIQiYzEEeELMxHB99679083 = tIQiYzEEeELMxHB82900937;     tIQiYzEEeELMxHB82900937 = tIQiYzEEeELMxHB65647906;     tIQiYzEEeELMxHB65647906 = tIQiYzEEeELMxHB80483130;     tIQiYzEEeELMxHB80483130 = tIQiYzEEeELMxHB5624036;     tIQiYzEEeELMxHB5624036 = tIQiYzEEeELMxHB80340224;     tIQiYzEEeELMxHB80340224 = tIQiYzEEeELMxHB56714167;     tIQiYzEEeELMxHB56714167 = tIQiYzEEeELMxHB76900190;     tIQiYzEEeELMxHB76900190 = tIQiYzEEeELMxHB11584931;     tIQiYzEEeELMxHB11584931 = tIQiYzEEeELMxHB72146066;     tIQiYzEEeELMxHB72146066 = tIQiYzEEeELMxHB771221;     tIQiYzEEeELMxHB771221 = tIQiYzEEeELMxHB21737921;     tIQiYzEEeELMxHB21737921 = tIQiYzEEeELMxHB12548010;     tIQiYzEEeELMxHB12548010 = tIQiYzEEeELMxHB9723383;     tIQiYzEEeELMxHB9723383 = tIQiYzEEeELMxHB68731821;     tIQiYzEEeELMxHB68731821 = tIQiYzEEeELMxHB90307098;     tIQiYzEEeELMxHB90307098 = tIQiYzEEeELMxHB24570900;     tIQiYzEEeELMxHB24570900 = tIQiYzEEeELMxHB54915395;     tIQiYzEEeELMxHB54915395 = tIQiYzEEeELMxHB61971554;     tIQiYzEEeELMxHB61971554 = tIQiYzEEeELMxHB26942359;     tIQiYzEEeELMxHB26942359 = tIQiYzEEeELMxHB20056533;     tIQiYzEEeELMxHB20056533 = tIQiYzEEeELMxHB68053914;     tIQiYzEEeELMxHB68053914 = tIQiYzEEeELMxHB31557858;     tIQiYzEEeELMxHB31557858 = tIQiYzEEeELMxHB24347635;     tIQiYzEEeELMxHB24347635 = tIQiYzEEeELMxHB62309274;     tIQiYzEEeELMxHB62309274 = tIQiYzEEeELMxHB75479028;     tIQiYzEEeELMxHB75479028 = tIQiYzEEeELMxHB51201945;     tIQiYzEEeELMxHB51201945 = tIQiYzEEeELMxHB65008469;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MnAQwJiIhFOOJrAIlyDsWbxFyMfjU97725543() {     int tDMwVKspdNKfVwR61521390 = -740774819;    int tDMwVKspdNKfVwR6669913 = -279158817;    int tDMwVKspdNKfVwR37220935 = -316834867;    int tDMwVKspdNKfVwR75575144 = -601891319;    int tDMwVKspdNKfVwR38014370 = -52639629;    int tDMwVKspdNKfVwR93924420 = -776618901;    int tDMwVKspdNKfVwR65020222 = -628672956;    int tDMwVKspdNKfVwR54741331 = -23008330;    int tDMwVKspdNKfVwR92046822 = -873835664;    int tDMwVKspdNKfVwR65264245 = -430450386;    int tDMwVKspdNKfVwR16042112 = -585520733;    int tDMwVKspdNKfVwR20298608 = -289875563;    int tDMwVKspdNKfVwR77705649 = 59649897;    int tDMwVKspdNKfVwR55426929 = -333531253;    int tDMwVKspdNKfVwR56786392 = -206683203;    int tDMwVKspdNKfVwR31235877 = -247865700;    int tDMwVKspdNKfVwR54923278 = -989683616;    int tDMwVKspdNKfVwR53058089 = -315463970;    int tDMwVKspdNKfVwR7673224 = -558896659;    int tDMwVKspdNKfVwR20344978 = -72950426;    int tDMwVKspdNKfVwR16814897 = -796723821;    int tDMwVKspdNKfVwR89998504 = -822506288;    int tDMwVKspdNKfVwR59200255 = -121293190;    int tDMwVKspdNKfVwR2710847 = -702836168;    int tDMwVKspdNKfVwR77678929 = -303533219;    int tDMwVKspdNKfVwR9882199 = -170777897;    int tDMwVKspdNKfVwR46556982 = -720203141;    int tDMwVKspdNKfVwR18295008 = -201396799;    int tDMwVKspdNKfVwR82344630 = -517501126;    int tDMwVKspdNKfVwR81742731 = -900002962;    int tDMwVKspdNKfVwR31753560 = -847665170;    int tDMwVKspdNKfVwR44232272 = -32093599;    int tDMwVKspdNKfVwR3493752 = -558501688;    int tDMwVKspdNKfVwR89366929 = -499988927;    int tDMwVKspdNKfVwR49967460 = -848835562;    int tDMwVKspdNKfVwR20470444 = -4704102;    int tDMwVKspdNKfVwR45181069 = -814257620;    int tDMwVKspdNKfVwR63691707 = -894447708;    int tDMwVKspdNKfVwR4294985 = -735532697;    int tDMwVKspdNKfVwR5588564 = -451139432;    int tDMwVKspdNKfVwR28989776 = -829759080;    int tDMwVKspdNKfVwR87681488 = -290359611;    int tDMwVKspdNKfVwR56155553 = -18804631;    int tDMwVKspdNKfVwR76287365 = -376530503;    int tDMwVKspdNKfVwR64532567 = -551588943;    int tDMwVKspdNKfVwR40370782 = -907115789;    int tDMwVKspdNKfVwR53869737 = -842269111;    int tDMwVKspdNKfVwR67565390 = -942718333;    int tDMwVKspdNKfVwR72507646 = -955525848;    int tDMwVKspdNKfVwR72178478 = -11701913;    int tDMwVKspdNKfVwR11538943 = -105406203;    int tDMwVKspdNKfVwR82520108 = -416403366;    int tDMwVKspdNKfVwR90902084 = -936737271;    int tDMwVKspdNKfVwR2824687 = -356506097;    int tDMwVKspdNKfVwR80910646 = -183326179;    int tDMwVKspdNKfVwR71522885 = -918268531;    int tDMwVKspdNKfVwR47469657 = -57865628;    int tDMwVKspdNKfVwR34510089 = -613998699;    int tDMwVKspdNKfVwR97896215 = -198358101;    int tDMwVKspdNKfVwR28132171 = -881861733;    int tDMwVKspdNKfVwR47367438 = 43584239;    int tDMwVKspdNKfVwR46725214 = -327276157;    int tDMwVKspdNKfVwR72396700 = -505507205;    int tDMwVKspdNKfVwR10304092 = -973832702;    int tDMwVKspdNKfVwR33510686 = -582785216;    int tDMwVKspdNKfVwR71809840 = -453427135;    int tDMwVKspdNKfVwR16804857 = -731373876;    int tDMwVKspdNKfVwR88338720 = -440361177;    int tDMwVKspdNKfVwR5459469 = -484695692;    int tDMwVKspdNKfVwR36315948 = -101979101;    int tDMwVKspdNKfVwR86054808 = -433608081;    int tDMwVKspdNKfVwR91231570 = 4764091;    int tDMwVKspdNKfVwR48763105 = -579931273;    int tDMwVKspdNKfVwR2084661 = -7757227;    int tDMwVKspdNKfVwR91355201 = -243191346;    int tDMwVKspdNKfVwR29133409 = -406364210;    int tDMwVKspdNKfVwR33842952 = -703701658;    int tDMwVKspdNKfVwR82912890 = -744762688;    int tDMwVKspdNKfVwR38178280 = -51247226;    int tDMwVKspdNKfVwR37308147 = -396417430;    int tDMwVKspdNKfVwR56012462 = -328508787;    int tDMwVKspdNKfVwR78991592 = -777484808;    int tDMwVKspdNKfVwR45787361 = -245870952;    int tDMwVKspdNKfVwR10166153 = -405799214;    int tDMwVKspdNKfVwR70203788 = -694596759;    int tDMwVKspdNKfVwR49233451 = -331261805;    int tDMwVKspdNKfVwR53330187 = -95356328;    int tDMwVKspdNKfVwR669065 = -101995591;    int tDMwVKspdNKfVwR8456283 = -216662748;    int tDMwVKspdNKfVwR78444575 = -930567031;    int tDMwVKspdNKfVwR73000786 = -946838475;    int tDMwVKspdNKfVwR10670981 = -100258922;    int tDMwVKspdNKfVwR65795492 = -596089607;    int tDMwVKspdNKfVwR76162813 = -853670965;    int tDMwVKspdNKfVwR58221126 = -394723672;    int tDMwVKspdNKfVwR82264562 = -402482924;    int tDMwVKspdNKfVwR15284788 = -784852407;    int tDMwVKspdNKfVwR45851461 = -44971929;    int tDMwVKspdNKfVwR42776680 = -793745287;    int tDMwVKspdNKfVwR92722727 = -740774819;     tDMwVKspdNKfVwR61521390 = tDMwVKspdNKfVwR6669913;     tDMwVKspdNKfVwR6669913 = tDMwVKspdNKfVwR37220935;     tDMwVKspdNKfVwR37220935 = tDMwVKspdNKfVwR75575144;     tDMwVKspdNKfVwR75575144 = tDMwVKspdNKfVwR38014370;     tDMwVKspdNKfVwR38014370 = tDMwVKspdNKfVwR93924420;     tDMwVKspdNKfVwR93924420 = tDMwVKspdNKfVwR65020222;     tDMwVKspdNKfVwR65020222 = tDMwVKspdNKfVwR54741331;     tDMwVKspdNKfVwR54741331 = tDMwVKspdNKfVwR92046822;     tDMwVKspdNKfVwR92046822 = tDMwVKspdNKfVwR65264245;     tDMwVKspdNKfVwR65264245 = tDMwVKspdNKfVwR16042112;     tDMwVKspdNKfVwR16042112 = tDMwVKspdNKfVwR20298608;     tDMwVKspdNKfVwR20298608 = tDMwVKspdNKfVwR77705649;     tDMwVKspdNKfVwR77705649 = tDMwVKspdNKfVwR55426929;     tDMwVKspdNKfVwR55426929 = tDMwVKspdNKfVwR56786392;     tDMwVKspdNKfVwR56786392 = tDMwVKspdNKfVwR31235877;     tDMwVKspdNKfVwR31235877 = tDMwVKspdNKfVwR54923278;     tDMwVKspdNKfVwR54923278 = tDMwVKspdNKfVwR53058089;     tDMwVKspdNKfVwR53058089 = tDMwVKspdNKfVwR7673224;     tDMwVKspdNKfVwR7673224 = tDMwVKspdNKfVwR20344978;     tDMwVKspdNKfVwR20344978 = tDMwVKspdNKfVwR16814897;     tDMwVKspdNKfVwR16814897 = tDMwVKspdNKfVwR89998504;     tDMwVKspdNKfVwR89998504 = tDMwVKspdNKfVwR59200255;     tDMwVKspdNKfVwR59200255 = tDMwVKspdNKfVwR2710847;     tDMwVKspdNKfVwR2710847 = tDMwVKspdNKfVwR77678929;     tDMwVKspdNKfVwR77678929 = tDMwVKspdNKfVwR9882199;     tDMwVKspdNKfVwR9882199 = tDMwVKspdNKfVwR46556982;     tDMwVKspdNKfVwR46556982 = tDMwVKspdNKfVwR18295008;     tDMwVKspdNKfVwR18295008 = tDMwVKspdNKfVwR82344630;     tDMwVKspdNKfVwR82344630 = tDMwVKspdNKfVwR81742731;     tDMwVKspdNKfVwR81742731 = tDMwVKspdNKfVwR31753560;     tDMwVKspdNKfVwR31753560 = tDMwVKspdNKfVwR44232272;     tDMwVKspdNKfVwR44232272 = tDMwVKspdNKfVwR3493752;     tDMwVKspdNKfVwR3493752 = tDMwVKspdNKfVwR89366929;     tDMwVKspdNKfVwR89366929 = tDMwVKspdNKfVwR49967460;     tDMwVKspdNKfVwR49967460 = tDMwVKspdNKfVwR20470444;     tDMwVKspdNKfVwR20470444 = tDMwVKspdNKfVwR45181069;     tDMwVKspdNKfVwR45181069 = tDMwVKspdNKfVwR63691707;     tDMwVKspdNKfVwR63691707 = tDMwVKspdNKfVwR4294985;     tDMwVKspdNKfVwR4294985 = tDMwVKspdNKfVwR5588564;     tDMwVKspdNKfVwR5588564 = tDMwVKspdNKfVwR28989776;     tDMwVKspdNKfVwR28989776 = tDMwVKspdNKfVwR87681488;     tDMwVKspdNKfVwR87681488 = tDMwVKspdNKfVwR56155553;     tDMwVKspdNKfVwR56155553 = tDMwVKspdNKfVwR76287365;     tDMwVKspdNKfVwR76287365 = tDMwVKspdNKfVwR64532567;     tDMwVKspdNKfVwR64532567 = tDMwVKspdNKfVwR40370782;     tDMwVKspdNKfVwR40370782 = tDMwVKspdNKfVwR53869737;     tDMwVKspdNKfVwR53869737 = tDMwVKspdNKfVwR67565390;     tDMwVKspdNKfVwR67565390 = tDMwVKspdNKfVwR72507646;     tDMwVKspdNKfVwR72507646 = tDMwVKspdNKfVwR72178478;     tDMwVKspdNKfVwR72178478 = tDMwVKspdNKfVwR11538943;     tDMwVKspdNKfVwR11538943 = tDMwVKspdNKfVwR82520108;     tDMwVKspdNKfVwR82520108 = tDMwVKspdNKfVwR90902084;     tDMwVKspdNKfVwR90902084 = tDMwVKspdNKfVwR2824687;     tDMwVKspdNKfVwR2824687 = tDMwVKspdNKfVwR80910646;     tDMwVKspdNKfVwR80910646 = tDMwVKspdNKfVwR71522885;     tDMwVKspdNKfVwR71522885 = tDMwVKspdNKfVwR47469657;     tDMwVKspdNKfVwR47469657 = tDMwVKspdNKfVwR34510089;     tDMwVKspdNKfVwR34510089 = tDMwVKspdNKfVwR97896215;     tDMwVKspdNKfVwR97896215 = tDMwVKspdNKfVwR28132171;     tDMwVKspdNKfVwR28132171 = tDMwVKspdNKfVwR47367438;     tDMwVKspdNKfVwR47367438 = tDMwVKspdNKfVwR46725214;     tDMwVKspdNKfVwR46725214 = tDMwVKspdNKfVwR72396700;     tDMwVKspdNKfVwR72396700 = tDMwVKspdNKfVwR10304092;     tDMwVKspdNKfVwR10304092 = tDMwVKspdNKfVwR33510686;     tDMwVKspdNKfVwR33510686 = tDMwVKspdNKfVwR71809840;     tDMwVKspdNKfVwR71809840 = tDMwVKspdNKfVwR16804857;     tDMwVKspdNKfVwR16804857 = tDMwVKspdNKfVwR88338720;     tDMwVKspdNKfVwR88338720 = tDMwVKspdNKfVwR5459469;     tDMwVKspdNKfVwR5459469 = tDMwVKspdNKfVwR36315948;     tDMwVKspdNKfVwR36315948 = tDMwVKspdNKfVwR86054808;     tDMwVKspdNKfVwR86054808 = tDMwVKspdNKfVwR91231570;     tDMwVKspdNKfVwR91231570 = tDMwVKspdNKfVwR48763105;     tDMwVKspdNKfVwR48763105 = tDMwVKspdNKfVwR2084661;     tDMwVKspdNKfVwR2084661 = tDMwVKspdNKfVwR91355201;     tDMwVKspdNKfVwR91355201 = tDMwVKspdNKfVwR29133409;     tDMwVKspdNKfVwR29133409 = tDMwVKspdNKfVwR33842952;     tDMwVKspdNKfVwR33842952 = tDMwVKspdNKfVwR82912890;     tDMwVKspdNKfVwR82912890 = tDMwVKspdNKfVwR38178280;     tDMwVKspdNKfVwR38178280 = tDMwVKspdNKfVwR37308147;     tDMwVKspdNKfVwR37308147 = tDMwVKspdNKfVwR56012462;     tDMwVKspdNKfVwR56012462 = tDMwVKspdNKfVwR78991592;     tDMwVKspdNKfVwR78991592 = tDMwVKspdNKfVwR45787361;     tDMwVKspdNKfVwR45787361 = tDMwVKspdNKfVwR10166153;     tDMwVKspdNKfVwR10166153 = tDMwVKspdNKfVwR70203788;     tDMwVKspdNKfVwR70203788 = tDMwVKspdNKfVwR49233451;     tDMwVKspdNKfVwR49233451 = tDMwVKspdNKfVwR53330187;     tDMwVKspdNKfVwR53330187 = tDMwVKspdNKfVwR669065;     tDMwVKspdNKfVwR669065 = tDMwVKspdNKfVwR8456283;     tDMwVKspdNKfVwR8456283 = tDMwVKspdNKfVwR78444575;     tDMwVKspdNKfVwR78444575 = tDMwVKspdNKfVwR73000786;     tDMwVKspdNKfVwR73000786 = tDMwVKspdNKfVwR10670981;     tDMwVKspdNKfVwR10670981 = tDMwVKspdNKfVwR65795492;     tDMwVKspdNKfVwR65795492 = tDMwVKspdNKfVwR76162813;     tDMwVKspdNKfVwR76162813 = tDMwVKspdNKfVwR58221126;     tDMwVKspdNKfVwR58221126 = tDMwVKspdNKfVwR82264562;     tDMwVKspdNKfVwR82264562 = tDMwVKspdNKfVwR15284788;     tDMwVKspdNKfVwR15284788 = tDMwVKspdNKfVwR45851461;     tDMwVKspdNKfVwR45851461 = tDMwVKspdNKfVwR42776680;     tDMwVKspdNKfVwR42776680 = tDMwVKspdNKfVwR92722727;     tDMwVKspdNKfVwR92722727 = tDMwVKspdNKfVwR61521390;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void qieWnCVonAUIuvEJjBKpWVHJUNQEw47672351() {     int UHhdPHkcsNwBGdA28692449 = -385200496;    int UHhdPHkcsNwBGdA51853939 = -60895294;    int UHhdPHkcsNwBGdA71558953 = -550103478;    int UHhdPHkcsNwBGdA42975419 = -340302669;    int UHhdPHkcsNwBGdA19276979 = -23277651;    int UHhdPHkcsNwBGdA86365622 = -809294817;    int UHhdPHkcsNwBGdA88666435 = 12169826;    int UHhdPHkcsNwBGdA77821831 = -778532287;    int UHhdPHkcsNwBGdA59425970 = -405954676;    int UHhdPHkcsNwBGdA45595729 = -753464078;    int UHhdPHkcsNwBGdA98186782 = -636185828;    int UHhdPHkcsNwBGdA66569939 = -338893899;    int UHhdPHkcsNwBGdA30950821 = -114014350;    int UHhdPHkcsNwBGdA52504856 = -298927947;    int UHhdPHkcsNwBGdA99234059 = -245047064;    int UHhdPHkcsNwBGdA2156794 = -343674852;    int UHhdPHkcsNwBGdA20614194 = -992101221;    int UHhdPHkcsNwBGdA86486538 = -525084609;    int UHhdPHkcsNwBGdA50875712 = -37229635;    int UHhdPHkcsNwBGdA47783183 = 87182890;    int UHhdPHkcsNwBGdA99960082 = -105486186;    int UHhdPHkcsNwBGdA13999761 = 78182597;    int UHhdPHkcsNwBGdA13762630 = -511098203;    int UHhdPHkcsNwBGdA16735046 = -373433956;    int UHhdPHkcsNwBGdA91635635 = -178169812;    int UHhdPHkcsNwBGdA99930067 = -540151676;    int UHhdPHkcsNwBGdA32492871 = -641763050;    int UHhdPHkcsNwBGdA82924070 = -853309678;    int UHhdPHkcsNwBGdA84004642 = -31167011;    int UHhdPHkcsNwBGdA45079464 = -47743501;    int UHhdPHkcsNwBGdA14506958 = -510973774;    int UHhdPHkcsNwBGdA39770059 = -300575021;    int UHhdPHkcsNwBGdA18431128 = -193615848;    int UHhdPHkcsNwBGdA67108735 = -7346878;    int UHhdPHkcsNwBGdA55118497 = -651133942;    int UHhdPHkcsNwBGdA81306766 = -281241613;    int UHhdPHkcsNwBGdA57588154 = -413621407;    int UHhdPHkcsNwBGdA19693491 = -504430105;    int UHhdPHkcsNwBGdA96248983 = -319648668;    int UHhdPHkcsNwBGdA8336369 = -895663991;    int UHhdPHkcsNwBGdA33478388 = -557376231;    int UHhdPHkcsNwBGdA74660170 = -373397561;    int UHhdPHkcsNwBGdA32381089 = 57298999;    int UHhdPHkcsNwBGdA35771876 = 66825215;    int UHhdPHkcsNwBGdA42455289 = -919130274;    int UHhdPHkcsNwBGdA4029676 = -85736069;    int UHhdPHkcsNwBGdA49215240 = -755578936;    int UHhdPHkcsNwBGdA38286043 = -949181272;    int UHhdPHkcsNwBGdA77899777 = -909030151;    int UHhdPHkcsNwBGdA33361581 = 42774639;    int UHhdPHkcsNwBGdA18257538 = -702802523;    int UHhdPHkcsNwBGdA82252994 = -585642589;    int UHhdPHkcsNwBGdA62672772 = -664389950;    int UHhdPHkcsNwBGdA68308916 = -935668746;    int UHhdPHkcsNwBGdA47712268 = -970089608;    int UHhdPHkcsNwBGdA14692688 = -363383093;    int UHhdPHkcsNwBGdA38091309 = -549797091;    int UHhdPHkcsNwBGdA54823907 = -76669523;    int UHhdPHkcsNwBGdA51339783 = -62132857;    int UHhdPHkcsNwBGdA19346912 = -483125975;    int UHhdPHkcsNwBGdA53872751 = -67531767;    int UHhdPHkcsNwBGdA5742365 = -134520497;    int UHhdPHkcsNwBGdA93817188 = -647365276;    int UHhdPHkcsNwBGdA14346506 = -258211175;    int UHhdPHkcsNwBGdA31088772 = -142490304;    int UHhdPHkcsNwBGdA58416724 = -235610807;    int UHhdPHkcsNwBGdA48138811 = -45278051;    int UHhdPHkcsNwBGdA63842085 = -6667473;    int UHhdPHkcsNwBGdA97386358 = -647794005;    int UHhdPHkcsNwBGdA17927293 = -963805452;    int UHhdPHkcsNwBGdA44568640 = -930053445;    int UHhdPHkcsNwBGdA920703 = -387671117;    int UHhdPHkcsNwBGdA90237555 = -105435942;    int UHhdPHkcsNwBGdA42539344 = -141565644;    int UHhdPHkcsNwBGdA14304796 = -355440880;    int UHhdPHkcsNwBGdA25299912 = -732088625;    int UHhdPHkcsNwBGdA81618672 = -979116403;    int UHhdPHkcsNwBGdA77990754 = -477923419;    int UHhdPHkcsNwBGdA74279757 = -454303683;    int UHhdPHkcsNwBGdA87605960 = 7566256;    int UHhdPHkcsNwBGdA50714827 = -784572740;    int UHhdPHkcsNwBGdA94206828 = -692581779;    int UHhdPHkcsNwBGdA5024294 = -944279527;    int UHhdPHkcsNwBGdA50643062 = 26058349;    int UHhdPHkcsNwBGdA26821927 = -344940979;    int UHhdPHkcsNwBGdA32253964 = -925331186;    int UHhdPHkcsNwBGdA77097286 = -636185072;    int UHhdPHkcsNwBGdA50122211 = -257947103;    int UHhdPHkcsNwBGdA19396467 = -37257270;    int UHhdPHkcsNwBGdA40425809 = -187750850;    int UHhdPHkcsNwBGdA43215458 = -731444522;    int UHhdPHkcsNwBGdA2764247 = -236951885;    int UHhdPHkcsNwBGdA68353707 = -342297248;    int UHhdPHkcsNwBGdA76902071 = -836522693;    int UHhdPHkcsNwBGdA54463617 = -728132224;    int UHhdPHkcsNwBGdA27736024 = -322855735;    int UHhdPHkcsNwBGdA80842982 = -726032285;    int UHhdPHkcsNwBGdA18034584 = -684489826;    int UHhdPHkcsNwBGdA4683105 = -790684481;    int UHhdPHkcsNwBGdA84038564 = -385200496;     UHhdPHkcsNwBGdA28692449 = UHhdPHkcsNwBGdA51853939;     UHhdPHkcsNwBGdA51853939 = UHhdPHkcsNwBGdA71558953;     UHhdPHkcsNwBGdA71558953 = UHhdPHkcsNwBGdA42975419;     UHhdPHkcsNwBGdA42975419 = UHhdPHkcsNwBGdA19276979;     UHhdPHkcsNwBGdA19276979 = UHhdPHkcsNwBGdA86365622;     UHhdPHkcsNwBGdA86365622 = UHhdPHkcsNwBGdA88666435;     UHhdPHkcsNwBGdA88666435 = UHhdPHkcsNwBGdA77821831;     UHhdPHkcsNwBGdA77821831 = UHhdPHkcsNwBGdA59425970;     UHhdPHkcsNwBGdA59425970 = UHhdPHkcsNwBGdA45595729;     UHhdPHkcsNwBGdA45595729 = UHhdPHkcsNwBGdA98186782;     UHhdPHkcsNwBGdA98186782 = UHhdPHkcsNwBGdA66569939;     UHhdPHkcsNwBGdA66569939 = UHhdPHkcsNwBGdA30950821;     UHhdPHkcsNwBGdA30950821 = UHhdPHkcsNwBGdA52504856;     UHhdPHkcsNwBGdA52504856 = UHhdPHkcsNwBGdA99234059;     UHhdPHkcsNwBGdA99234059 = UHhdPHkcsNwBGdA2156794;     UHhdPHkcsNwBGdA2156794 = UHhdPHkcsNwBGdA20614194;     UHhdPHkcsNwBGdA20614194 = UHhdPHkcsNwBGdA86486538;     UHhdPHkcsNwBGdA86486538 = UHhdPHkcsNwBGdA50875712;     UHhdPHkcsNwBGdA50875712 = UHhdPHkcsNwBGdA47783183;     UHhdPHkcsNwBGdA47783183 = UHhdPHkcsNwBGdA99960082;     UHhdPHkcsNwBGdA99960082 = UHhdPHkcsNwBGdA13999761;     UHhdPHkcsNwBGdA13999761 = UHhdPHkcsNwBGdA13762630;     UHhdPHkcsNwBGdA13762630 = UHhdPHkcsNwBGdA16735046;     UHhdPHkcsNwBGdA16735046 = UHhdPHkcsNwBGdA91635635;     UHhdPHkcsNwBGdA91635635 = UHhdPHkcsNwBGdA99930067;     UHhdPHkcsNwBGdA99930067 = UHhdPHkcsNwBGdA32492871;     UHhdPHkcsNwBGdA32492871 = UHhdPHkcsNwBGdA82924070;     UHhdPHkcsNwBGdA82924070 = UHhdPHkcsNwBGdA84004642;     UHhdPHkcsNwBGdA84004642 = UHhdPHkcsNwBGdA45079464;     UHhdPHkcsNwBGdA45079464 = UHhdPHkcsNwBGdA14506958;     UHhdPHkcsNwBGdA14506958 = UHhdPHkcsNwBGdA39770059;     UHhdPHkcsNwBGdA39770059 = UHhdPHkcsNwBGdA18431128;     UHhdPHkcsNwBGdA18431128 = UHhdPHkcsNwBGdA67108735;     UHhdPHkcsNwBGdA67108735 = UHhdPHkcsNwBGdA55118497;     UHhdPHkcsNwBGdA55118497 = UHhdPHkcsNwBGdA81306766;     UHhdPHkcsNwBGdA81306766 = UHhdPHkcsNwBGdA57588154;     UHhdPHkcsNwBGdA57588154 = UHhdPHkcsNwBGdA19693491;     UHhdPHkcsNwBGdA19693491 = UHhdPHkcsNwBGdA96248983;     UHhdPHkcsNwBGdA96248983 = UHhdPHkcsNwBGdA8336369;     UHhdPHkcsNwBGdA8336369 = UHhdPHkcsNwBGdA33478388;     UHhdPHkcsNwBGdA33478388 = UHhdPHkcsNwBGdA74660170;     UHhdPHkcsNwBGdA74660170 = UHhdPHkcsNwBGdA32381089;     UHhdPHkcsNwBGdA32381089 = UHhdPHkcsNwBGdA35771876;     UHhdPHkcsNwBGdA35771876 = UHhdPHkcsNwBGdA42455289;     UHhdPHkcsNwBGdA42455289 = UHhdPHkcsNwBGdA4029676;     UHhdPHkcsNwBGdA4029676 = UHhdPHkcsNwBGdA49215240;     UHhdPHkcsNwBGdA49215240 = UHhdPHkcsNwBGdA38286043;     UHhdPHkcsNwBGdA38286043 = UHhdPHkcsNwBGdA77899777;     UHhdPHkcsNwBGdA77899777 = UHhdPHkcsNwBGdA33361581;     UHhdPHkcsNwBGdA33361581 = UHhdPHkcsNwBGdA18257538;     UHhdPHkcsNwBGdA18257538 = UHhdPHkcsNwBGdA82252994;     UHhdPHkcsNwBGdA82252994 = UHhdPHkcsNwBGdA62672772;     UHhdPHkcsNwBGdA62672772 = UHhdPHkcsNwBGdA68308916;     UHhdPHkcsNwBGdA68308916 = UHhdPHkcsNwBGdA47712268;     UHhdPHkcsNwBGdA47712268 = UHhdPHkcsNwBGdA14692688;     UHhdPHkcsNwBGdA14692688 = UHhdPHkcsNwBGdA38091309;     UHhdPHkcsNwBGdA38091309 = UHhdPHkcsNwBGdA54823907;     UHhdPHkcsNwBGdA54823907 = UHhdPHkcsNwBGdA51339783;     UHhdPHkcsNwBGdA51339783 = UHhdPHkcsNwBGdA19346912;     UHhdPHkcsNwBGdA19346912 = UHhdPHkcsNwBGdA53872751;     UHhdPHkcsNwBGdA53872751 = UHhdPHkcsNwBGdA5742365;     UHhdPHkcsNwBGdA5742365 = UHhdPHkcsNwBGdA93817188;     UHhdPHkcsNwBGdA93817188 = UHhdPHkcsNwBGdA14346506;     UHhdPHkcsNwBGdA14346506 = UHhdPHkcsNwBGdA31088772;     UHhdPHkcsNwBGdA31088772 = UHhdPHkcsNwBGdA58416724;     UHhdPHkcsNwBGdA58416724 = UHhdPHkcsNwBGdA48138811;     UHhdPHkcsNwBGdA48138811 = UHhdPHkcsNwBGdA63842085;     UHhdPHkcsNwBGdA63842085 = UHhdPHkcsNwBGdA97386358;     UHhdPHkcsNwBGdA97386358 = UHhdPHkcsNwBGdA17927293;     UHhdPHkcsNwBGdA17927293 = UHhdPHkcsNwBGdA44568640;     UHhdPHkcsNwBGdA44568640 = UHhdPHkcsNwBGdA920703;     UHhdPHkcsNwBGdA920703 = UHhdPHkcsNwBGdA90237555;     UHhdPHkcsNwBGdA90237555 = UHhdPHkcsNwBGdA42539344;     UHhdPHkcsNwBGdA42539344 = UHhdPHkcsNwBGdA14304796;     UHhdPHkcsNwBGdA14304796 = UHhdPHkcsNwBGdA25299912;     UHhdPHkcsNwBGdA25299912 = UHhdPHkcsNwBGdA81618672;     UHhdPHkcsNwBGdA81618672 = UHhdPHkcsNwBGdA77990754;     UHhdPHkcsNwBGdA77990754 = UHhdPHkcsNwBGdA74279757;     UHhdPHkcsNwBGdA74279757 = UHhdPHkcsNwBGdA87605960;     UHhdPHkcsNwBGdA87605960 = UHhdPHkcsNwBGdA50714827;     UHhdPHkcsNwBGdA50714827 = UHhdPHkcsNwBGdA94206828;     UHhdPHkcsNwBGdA94206828 = UHhdPHkcsNwBGdA5024294;     UHhdPHkcsNwBGdA5024294 = UHhdPHkcsNwBGdA50643062;     UHhdPHkcsNwBGdA50643062 = UHhdPHkcsNwBGdA26821927;     UHhdPHkcsNwBGdA26821927 = UHhdPHkcsNwBGdA32253964;     UHhdPHkcsNwBGdA32253964 = UHhdPHkcsNwBGdA77097286;     UHhdPHkcsNwBGdA77097286 = UHhdPHkcsNwBGdA50122211;     UHhdPHkcsNwBGdA50122211 = UHhdPHkcsNwBGdA19396467;     UHhdPHkcsNwBGdA19396467 = UHhdPHkcsNwBGdA40425809;     UHhdPHkcsNwBGdA40425809 = UHhdPHkcsNwBGdA43215458;     UHhdPHkcsNwBGdA43215458 = UHhdPHkcsNwBGdA2764247;     UHhdPHkcsNwBGdA2764247 = UHhdPHkcsNwBGdA68353707;     UHhdPHkcsNwBGdA68353707 = UHhdPHkcsNwBGdA76902071;     UHhdPHkcsNwBGdA76902071 = UHhdPHkcsNwBGdA54463617;     UHhdPHkcsNwBGdA54463617 = UHhdPHkcsNwBGdA27736024;     UHhdPHkcsNwBGdA27736024 = UHhdPHkcsNwBGdA80842982;     UHhdPHkcsNwBGdA80842982 = UHhdPHkcsNwBGdA18034584;     UHhdPHkcsNwBGdA18034584 = UHhdPHkcsNwBGdA4683105;     UHhdPHkcsNwBGdA4683105 = UHhdPHkcsNwBGdA84038564;     UHhdPHkcsNwBGdA84038564 = UHhdPHkcsNwBGdA28692449;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LQDMpHsvSOPksWOXWAQaxLqMBjlOv49861689() {     int CwxakICNejMvJrz25205370 = -384001184;    int CwxakICNejMvJrz66936764 = -13583596;    int CwxakICNejMvJrz27241264 = -298206244;    int CwxakICNejMvJrz55524267 = -809116460;    int CwxakICNejMvJrz87929369 = -678864647;    int CwxakICNejMvJrz23612495 = -839822394;    int CwxakICNejMvJrz62481088 = -152748149;    int CwxakICNejMvJrz13274002 = -891416472;    int CwxakICNejMvJrz63680226 = -658372430;    int CwxakICNejMvJrz99600265 = -412189291;    int CwxakICNejMvJrz58402898 = -611421065;    int CwxakICNejMvJrz81586376 = -907626805;    int CwxakICNejMvJrz36221518 = -399800014;    int CwxakICNejMvJrz19579969 = -808449482;    int CwxakICNejMvJrz4133817 = -96373775;    int CwxakICNejMvJrz15113618 = -102296067;    int CwxakICNejMvJrz48305043 = -904948266;    int CwxakICNejMvJrz22622027 = -42610674;    int CwxakICNejMvJrz32798688 = -526356935;    int CwxakICNejMvJrz88508442 = -922341941;    int CwxakICNejMvJrz7638938 = -316035784;    int CwxakICNejMvJrz54901518 = -299835096;    int CwxakICNejMvJrz3259593 = -792017914;    int CwxakICNejMvJrz40559816 = -332551360;    int CwxakICNejMvJrz69910642 = -803595572;    int CwxakICNejMvJrz75954431 = -719446939;    int CwxakICNejMvJrz80389556 = -468944897;    int CwxakICNejMvJrz75174433 = -509065776;    int CwxakICNejMvJrz66200614 = -996544994;    int CwxakICNejMvJrz45333972 = -512520344;    int CwxakICNejMvJrz23601037 = -812941928;    int CwxakICNejMvJrz75520242 = -632581041;    int CwxakICNejMvJrz35875567 = -83712616;    int CwxakICNejMvJrz79768403 = -927856878;    int CwxakICNejMvJrz64603336 = -865799468;    int CwxakICNejMvJrz24978020 = -558318321;    int CwxakICNejMvJrz38145121 = 75954635;    int CwxakICNejMvJrz92820465 = -325562074;    int CwxakICNejMvJrz44983290 = -660717418;    int CwxakICNejMvJrz87853767 = -332011873;    int CwxakICNejMvJrz65749382 = 11081599;    int CwxakICNejMvJrz18853523 = -880838523;    int CwxakICNejMvJrz19923025 = -438973573;    int CwxakICNejMvJrz47979984 = -74808538;    int CwxakICNejMvJrz8442004 = -351284611;    int CwxakICNejMvJrz1710702 = -418063865;    int CwxakICNejMvJrz46127333 = 65900415;    int CwxakICNejMvJrz18776066 = -12275271;    int CwxakICNejMvJrz96508845 = -440534346;    int CwxakICNejMvJrz6162620 = -306764816;    int CwxakICNejMvJrz70046178 = -232181730;    int CwxakICNejMvJrz54661632 = -67807840;    int CwxakICNejMvJrz54816150 = -356298594;    int CwxakICNejMvJrz53816112 = -285869750;    int CwxakICNejMvJrz42222752 = -302811401;    int CwxakICNejMvJrz70303851 = 15833911;    int CwxakICNejMvJrz63677172 = -221565683;    int CwxakICNejMvJrz86681447 = -965654884;    int CwxakICNejMvJrz85613624 = 94479111;    int CwxakICNejMvJrz11974938 = -959417708;    int CwxakICNejMvJrz43222939 = -270877497;    int CwxakICNejMvJrz87306654 = -643682373;    int CwxakICNejMvJrz47073387 = -894871478;    int CwxakICNejMvJrz18346255 = -45852087;    int CwxakICNejMvJrz75999228 = -599247363;    int CwxakICNejMvJrz82882656 = -978840024;    int CwxakICNejMvJrz45710809 = -723914189;    int CwxakICNejMvJrz56453114 = -471943136;    int CwxakICNejMvJrz54976633 = -942650014;    int CwxakICNejMvJrz79155796 = -538055454;    int CwxakICNejMvJrz76968496 = -78250702;    int CwxakICNejMvJrz55484577 = -479386193;    int CwxakICNejMvJrz77638736 = -381893256;    int CwxakICNejMvJrz44944920 = -94345062;    int CwxakICNejMvJrz22759060 = -833423540;    int CwxakICNejMvJrz88785414 = -435197261;    int CwxakICNejMvJrz34978494 = -860861524;    int CwxakICNejMvJrz55279609 = -617209376;    int CwxakICNejMvJrz32117813 = -981266749;    int CwxakICNejMvJrz68199941 = -285531707;    int CwxakICNejMvJrz29827099 = -685347355;    int CwxakICNejMvJrz61613490 = -356669627;    int CwxakICNejMvJrz78665588 = 31468569;    int CwxakICNejMvJrz60037994 = -589780179;    int CwxakICNejMvJrz75287794 = -180338615;    int CwxakICNejMvJrz68939404 = -645134089;    int CwxakICNejMvJrz20704092 = -176282448;    int CwxakICNejMvJrz82059454 = -797842866;    int CwxakICNejMvJrz37545652 = -525045478;    int CwxakICNejMvJrz94299484 = -781633380;    int CwxakICNejMvJrz61300848 = -236752639;    int CwxakICNejMvJrz51463673 = 41609519;    int CwxakICNejMvJrz7206841 = -320041186;    int CwxakICNejMvJrz33008353 = -701299711;    int CwxakICNejMvJrz44630829 = 38865624;    int CwxakICNejMvJrz78442727 = -345236029;    int CwxakICNejMvJrz71780135 = -985967046;    int CwxakICNejMvJrz1576771 = -293121486;    int CwxakICNejMvJrz71980755 = -475561175;    int CwxakICNejMvJrz25559347 = -384001184;     CwxakICNejMvJrz25205370 = CwxakICNejMvJrz66936764;     CwxakICNejMvJrz66936764 = CwxakICNejMvJrz27241264;     CwxakICNejMvJrz27241264 = CwxakICNejMvJrz55524267;     CwxakICNejMvJrz55524267 = CwxakICNejMvJrz87929369;     CwxakICNejMvJrz87929369 = CwxakICNejMvJrz23612495;     CwxakICNejMvJrz23612495 = CwxakICNejMvJrz62481088;     CwxakICNejMvJrz62481088 = CwxakICNejMvJrz13274002;     CwxakICNejMvJrz13274002 = CwxakICNejMvJrz63680226;     CwxakICNejMvJrz63680226 = CwxakICNejMvJrz99600265;     CwxakICNejMvJrz99600265 = CwxakICNejMvJrz58402898;     CwxakICNejMvJrz58402898 = CwxakICNejMvJrz81586376;     CwxakICNejMvJrz81586376 = CwxakICNejMvJrz36221518;     CwxakICNejMvJrz36221518 = CwxakICNejMvJrz19579969;     CwxakICNejMvJrz19579969 = CwxakICNejMvJrz4133817;     CwxakICNejMvJrz4133817 = CwxakICNejMvJrz15113618;     CwxakICNejMvJrz15113618 = CwxakICNejMvJrz48305043;     CwxakICNejMvJrz48305043 = CwxakICNejMvJrz22622027;     CwxakICNejMvJrz22622027 = CwxakICNejMvJrz32798688;     CwxakICNejMvJrz32798688 = CwxakICNejMvJrz88508442;     CwxakICNejMvJrz88508442 = CwxakICNejMvJrz7638938;     CwxakICNejMvJrz7638938 = CwxakICNejMvJrz54901518;     CwxakICNejMvJrz54901518 = CwxakICNejMvJrz3259593;     CwxakICNejMvJrz3259593 = CwxakICNejMvJrz40559816;     CwxakICNejMvJrz40559816 = CwxakICNejMvJrz69910642;     CwxakICNejMvJrz69910642 = CwxakICNejMvJrz75954431;     CwxakICNejMvJrz75954431 = CwxakICNejMvJrz80389556;     CwxakICNejMvJrz80389556 = CwxakICNejMvJrz75174433;     CwxakICNejMvJrz75174433 = CwxakICNejMvJrz66200614;     CwxakICNejMvJrz66200614 = CwxakICNejMvJrz45333972;     CwxakICNejMvJrz45333972 = CwxakICNejMvJrz23601037;     CwxakICNejMvJrz23601037 = CwxakICNejMvJrz75520242;     CwxakICNejMvJrz75520242 = CwxakICNejMvJrz35875567;     CwxakICNejMvJrz35875567 = CwxakICNejMvJrz79768403;     CwxakICNejMvJrz79768403 = CwxakICNejMvJrz64603336;     CwxakICNejMvJrz64603336 = CwxakICNejMvJrz24978020;     CwxakICNejMvJrz24978020 = CwxakICNejMvJrz38145121;     CwxakICNejMvJrz38145121 = CwxakICNejMvJrz92820465;     CwxakICNejMvJrz92820465 = CwxakICNejMvJrz44983290;     CwxakICNejMvJrz44983290 = CwxakICNejMvJrz87853767;     CwxakICNejMvJrz87853767 = CwxakICNejMvJrz65749382;     CwxakICNejMvJrz65749382 = CwxakICNejMvJrz18853523;     CwxakICNejMvJrz18853523 = CwxakICNejMvJrz19923025;     CwxakICNejMvJrz19923025 = CwxakICNejMvJrz47979984;     CwxakICNejMvJrz47979984 = CwxakICNejMvJrz8442004;     CwxakICNejMvJrz8442004 = CwxakICNejMvJrz1710702;     CwxakICNejMvJrz1710702 = CwxakICNejMvJrz46127333;     CwxakICNejMvJrz46127333 = CwxakICNejMvJrz18776066;     CwxakICNejMvJrz18776066 = CwxakICNejMvJrz96508845;     CwxakICNejMvJrz96508845 = CwxakICNejMvJrz6162620;     CwxakICNejMvJrz6162620 = CwxakICNejMvJrz70046178;     CwxakICNejMvJrz70046178 = CwxakICNejMvJrz54661632;     CwxakICNejMvJrz54661632 = CwxakICNejMvJrz54816150;     CwxakICNejMvJrz54816150 = CwxakICNejMvJrz53816112;     CwxakICNejMvJrz53816112 = CwxakICNejMvJrz42222752;     CwxakICNejMvJrz42222752 = CwxakICNejMvJrz70303851;     CwxakICNejMvJrz70303851 = CwxakICNejMvJrz63677172;     CwxakICNejMvJrz63677172 = CwxakICNejMvJrz86681447;     CwxakICNejMvJrz86681447 = CwxakICNejMvJrz85613624;     CwxakICNejMvJrz85613624 = CwxakICNejMvJrz11974938;     CwxakICNejMvJrz11974938 = CwxakICNejMvJrz43222939;     CwxakICNejMvJrz43222939 = CwxakICNejMvJrz87306654;     CwxakICNejMvJrz87306654 = CwxakICNejMvJrz47073387;     CwxakICNejMvJrz47073387 = CwxakICNejMvJrz18346255;     CwxakICNejMvJrz18346255 = CwxakICNejMvJrz75999228;     CwxakICNejMvJrz75999228 = CwxakICNejMvJrz82882656;     CwxakICNejMvJrz82882656 = CwxakICNejMvJrz45710809;     CwxakICNejMvJrz45710809 = CwxakICNejMvJrz56453114;     CwxakICNejMvJrz56453114 = CwxakICNejMvJrz54976633;     CwxakICNejMvJrz54976633 = CwxakICNejMvJrz79155796;     CwxakICNejMvJrz79155796 = CwxakICNejMvJrz76968496;     CwxakICNejMvJrz76968496 = CwxakICNejMvJrz55484577;     CwxakICNejMvJrz55484577 = CwxakICNejMvJrz77638736;     CwxakICNejMvJrz77638736 = CwxakICNejMvJrz44944920;     CwxakICNejMvJrz44944920 = CwxakICNejMvJrz22759060;     CwxakICNejMvJrz22759060 = CwxakICNejMvJrz88785414;     CwxakICNejMvJrz88785414 = CwxakICNejMvJrz34978494;     CwxakICNejMvJrz34978494 = CwxakICNejMvJrz55279609;     CwxakICNejMvJrz55279609 = CwxakICNejMvJrz32117813;     CwxakICNejMvJrz32117813 = CwxakICNejMvJrz68199941;     CwxakICNejMvJrz68199941 = CwxakICNejMvJrz29827099;     CwxakICNejMvJrz29827099 = CwxakICNejMvJrz61613490;     CwxakICNejMvJrz61613490 = CwxakICNejMvJrz78665588;     CwxakICNejMvJrz78665588 = CwxakICNejMvJrz60037994;     CwxakICNejMvJrz60037994 = CwxakICNejMvJrz75287794;     CwxakICNejMvJrz75287794 = CwxakICNejMvJrz68939404;     CwxakICNejMvJrz68939404 = CwxakICNejMvJrz20704092;     CwxakICNejMvJrz20704092 = CwxakICNejMvJrz82059454;     CwxakICNejMvJrz82059454 = CwxakICNejMvJrz37545652;     CwxakICNejMvJrz37545652 = CwxakICNejMvJrz94299484;     CwxakICNejMvJrz94299484 = CwxakICNejMvJrz61300848;     CwxakICNejMvJrz61300848 = CwxakICNejMvJrz51463673;     CwxakICNejMvJrz51463673 = CwxakICNejMvJrz7206841;     CwxakICNejMvJrz7206841 = CwxakICNejMvJrz33008353;     CwxakICNejMvJrz33008353 = CwxakICNejMvJrz44630829;     CwxakICNejMvJrz44630829 = CwxakICNejMvJrz78442727;     CwxakICNejMvJrz78442727 = CwxakICNejMvJrz71780135;     CwxakICNejMvJrz71780135 = CwxakICNejMvJrz1576771;     CwxakICNejMvJrz1576771 = CwxakICNejMvJrz71980755;     CwxakICNejMvJrz71980755 = CwxakICNejMvJrz25559347;     CwxakICNejMvJrz25559347 = CwxakICNejMvJrz25205370;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void TpgJNUVRQfFhuYcRhNMbSPtobBCgB52051027() {     int rnOekbPzBuWKFQn21718291 = -382801872;    int rnOekbPzBuWKFQn82019589 = 33728102;    int rnOekbPzBuWKFQn82923575 = -46309010;    int rnOekbPzBuWKFQn68073115 = -177930250;    int rnOekbPzBuWKFQn56581759 = -234451642;    int rnOekbPzBuWKFQn60859367 = -870349970;    int rnOekbPzBuWKFQn36295741 = -317666124;    int rnOekbPzBuWKFQn48726172 = 95699344;    int rnOekbPzBuWKFQn67934483 = -910790184;    int rnOekbPzBuWKFQn53604802 = -70914503;    int rnOekbPzBuWKFQn18619014 = -586656302;    int rnOekbPzBuWKFQn96602813 = -376359711;    int rnOekbPzBuWKFQn41492216 = -685585677;    int rnOekbPzBuWKFQn86655082 = -217971017;    int rnOekbPzBuWKFQn9033574 = 52299515;    int rnOekbPzBuWKFQn28070441 = -960917282;    int rnOekbPzBuWKFQn75995891 = -817795311;    int rnOekbPzBuWKFQn58757515 = -660136739;    int rnOekbPzBuWKFQn14721665 = 84515765;    int rnOekbPzBuWKFQn29233702 = -831866771;    int rnOekbPzBuWKFQn15317793 = -526585382;    int rnOekbPzBuWKFQn95803275 = -677852788;    int rnOekbPzBuWKFQn92756554 = 27062376;    int rnOekbPzBuWKFQn64384586 = -291668764;    int rnOekbPzBuWKFQn48185650 = -329021331;    int rnOekbPzBuWKFQn51978796 = -898742202;    int rnOekbPzBuWKFQn28286241 = -296126744;    int rnOekbPzBuWKFQn67424796 = -164821875;    int rnOekbPzBuWKFQn48396585 = -861922977;    int rnOekbPzBuWKFQn45588480 = -977297186;    int rnOekbPzBuWKFQn32695117 = -14910081;    int rnOekbPzBuWKFQn11270425 = -964587061;    int rnOekbPzBuWKFQn53320005 = 26190616;    int rnOekbPzBuWKFQn92428072 = -748366878;    int rnOekbPzBuWKFQn74088174 = 19535007;    int rnOekbPzBuWKFQn68649274 = -835395029;    int rnOekbPzBuWKFQn18702088 = -534469323;    int rnOekbPzBuWKFQn65947439 = -146694043;    int rnOekbPzBuWKFQn93717596 = 98213831;    int rnOekbPzBuWKFQn67371167 = -868359755;    int rnOekbPzBuWKFQn98020377 = -520460572;    int rnOekbPzBuWKFQn63046875 = -288279486;    int rnOekbPzBuWKFQn7464961 = -935246145;    int rnOekbPzBuWKFQn60188092 = -216442292;    int rnOekbPzBuWKFQn74428718 = -883438949;    int rnOekbPzBuWKFQn99391728 = -750391661;    int rnOekbPzBuWKFQn43039426 = -212620233;    int rnOekbPzBuWKFQn99266088 = -175369270;    int rnOekbPzBuWKFQn15117914 = 27961458;    int rnOekbPzBuWKFQn78963659 = -656304271;    int rnOekbPzBuWKFQn21834819 = -861560936;    int rnOekbPzBuWKFQn27070271 = -649973090;    int rnOekbPzBuWKFQn46959529 = -48207238;    int rnOekbPzBuWKFQn39323308 = -736070754;    int rnOekbPzBuWKFQn36733235 = -735533193;    int rnOekbPzBuWKFQn25915015 = -704949084;    int rnOekbPzBuWKFQn89263034 = -993334274;    int rnOekbPzBuWKFQn18538989 = -754640246;    int rnOekbPzBuWKFQn19887466 = -848908920;    int rnOekbPzBuWKFQn4602963 = -335709441;    int rnOekbPzBuWKFQn32573126 = -474223227;    int rnOekbPzBuWKFQn68870945 = -52844249;    int rnOekbPzBuWKFQn329587 = -42377679;    int rnOekbPzBuWKFQn22346004 = -933492998;    int rnOekbPzBuWKFQn20909685 = 43995577;    int rnOekbPzBuWKFQn7348589 = -622069241;    int rnOekbPzBuWKFQn43282808 = -302550327;    int rnOekbPzBuWKFQn49064144 = -937218800;    int rnOekbPzBuWKFQn12566908 = -137506024;    int rnOekbPzBuWKFQn40384300 = -112305456;    int rnOekbPzBuWKFQn9368354 = -326447960;    int rnOekbPzBuWKFQn10048452 = -571101269;    int rnOekbPzBuWKFQn65039918 = -658350571;    int rnOekbPzBuWKFQn47350497 = -47124481;    int rnOekbPzBuWKFQn31213325 = -211406200;    int rnOekbPzBuWKFQn52270917 = -138305897;    int rnOekbPzBuWKFQn88338315 = -742606644;    int rnOekbPzBuWKFQn32568463 = -756495333;    int rnOekbPzBuWKFQn89955868 = -408229816;    int rnOekbPzBuWKFQn48793921 = -578629671;    int rnOekbPzBuWKFQn8939371 = -586121969;    int rnOekbPzBuWKFQn29020152 = -20757475;    int rnOekbPzBuWKFQn52306883 = -92783334;    int rnOekbPzBuWKFQn69432926 = -105618707;    int rnOekbPzBuWKFQn23753662 = -15736251;    int rnOekbPzBuWKFQn5624846 = -364936992;    int rnOekbPzBuWKFQn64310896 = -816379823;    int rnOekbPzBuWKFQn13996698 = -237738630;    int rnOekbPzBuWKFQn55694837 = 87166315;    int rnOekbPzBuWKFQn48173159 = -275515910;    int rnOekbPzBuWKFQn79386239 = -842060755;    int rnOekbPzBuWKFQn163100 = -779829078;    int rnOekbPzBuWKFQn46059974 = -297785123;    int rnOekbPzBuWKFQn89114634 = -566076728;    int rnOekbPzBuWKFQn34798041 = -294136529;    int rnOekbPzBuWKFQn29149432 = -367616324;    int rnOekbPzBuWKFQn62717288 = -145901807;    int rnOekbPzBuWKFQn85118957 = 98246853;    int rnOekbPzBuWKFQn39278407 = -160437869;    int rnOekbPzBuWKFQn67080129 = -382801872;     rnOekbPzBuWKFQn21718291 = rnOekbPzBuWKFQn82019589;     rnOekbPzBuWKFQn82019589 = rnOekbPzBuWKFQn82923575;     rnOekbPzBuWKFQn82923575 = rnOekbPzBuWKFQn68073115;     rnOekbPzBuWKFQn68073115 = rnOekbPzBuWKFQn56581759;     rnOekbPzBuWKFQn56581759 = rnOekbPzBuWKFQn60859367;     rnOekbPzBuWKFQn60859367 = rnOekbPzBuWKFQn36295741;     rnOekbPzBuWKFQn36295741 = rnOekbPzBuWKFQn48726172;     rnOekbPzBuWKFQn48726172 = rnOekbPzBuWKFQn67934483;     rnOekbPzBuWKFQn67934483 = rnOekbPzBuWKFQn53604802;     rnOekbPzBuWKFQn53604802 = rnOekbPzBuWKFQn18619014;     rnOekbPzBuWKFQn18619014 = rnOekbPzBuWKFQn96602813;     rnOekbPzBuWKFQn96602813 = rnOekbPzBuWKFQn41492216;     rnOekbPzBuWKFQn41492216 = rnOekbPzBuWKFQn86655082;     rnOekbPzBuWKFQn86655082 = rnOekbPzBuWKFQn9033574;     rnOekbPzBuWKFQn9033574 = rnOekbPzBuWKFQn28070441;     rnOekbPzBuWKFQn28070441 = rnOekbPzBuWKFQn75995891;     rnOekbPzBuWKFQn75995891 = rnOekbPzBuWKFQn58757515;     rnOekbPzBuWKFQn58757515 = rnOekbPzBuWKFQn14721665;     rnOekbPzBuWKFQn14721665 = rnOekbPzBuWKFQn29233702;     rnOekbPzBuWKFQn29233702 = rnOekbPzBuWKFQn15317793;     rnOekbPzBuWKFQn15317793 = rnOekbPzBuWKFQn95803275;     rnOekbPzBuWKFQn95803275 = rnOekbPzBuWKFQn92756554;     rnOekbPzBuWKFQn92756554 = rnOekbPzBuWKFQn64384586;     rnOekbPzBuWKFQn64384586 = rnOekbPzBuWKFQn48185650;     rnOekbPzBuWKFQn48185650 = rnOekbPzBuWKFQn51978796;     rnOekbPzBuWKFQn51978796 = rnOekbPzBuWKFQn28286241;     rnOekbPzBuWKFQn28286241 = rnOekbPzBuWKFQn67424796;     rnOekbPzBuWKFQn67424796 = rnOekbPzBuWKFQn48396585;     rnOekbPzBuWKFQn48396585 = rnOekbPzBuWKFQn45588480;     rnOekbPzBuWKFQn45588480 = rnOekbPzBuWKFQn32695117;     rnOekbPzBuWKFQn32695117 = rnOekbPzBuWKFQn11270425;     rnOekbPzBuWKFQn11270425 = rnOekbPzBuWKFQn53320005;     rnOekbPzBuWKFQn53320005 = rnOekbPzBuWKFQn92428072;     rnOekbPzBuWKFQn92428072 = rnOekbPzBuWKFQn74088174;     rnOekbPzBuWKFQn74088174 = rnOekbPzBuWKFQn68649274;     rnOekbPzBuWKFQn68649274 = rnOekbPzBuWKFQn18702088;     rnOekbPzBuWKFQn18702088 = rnOekbPzBuWKFQn65947439;     rnOekbPzBuWKFQn65947439 = rnOekbPzBuWKFQn93717596;     rnOekbPzBuWKFQn93717596 = rnOekbPzBuWKFQn67371167;     rnOekbPzBuWKFQn67371167 = rnOekbPzBuWKFQn98020377;     rnOekbPzBuWKFQn98020377 = rnOekbPzBuWKFQn63046875;     rnOekbPzBuWKFQn63046875 = rnOekbPzBuWKFQn7464961;     rnOekbPzBuWKFQn7464961 = rnOekbPzBuWKFQn60188092;     rnOekbPzBuWKFQn60188092 = rnOekbPzBuWKFQn74428718;     rnOekbPzBuWKFQn74428718 = rnOekbPzBuWKFQn99391728;     rnOekbPzBuWKFQn99391728 = rnOekbPzBuWKFQn43039426;     rnOekbPzBuWKFQn43039426 = rnOekbPzBuWKFQn99266088;     rnOekbPzBuWKFQn99266088 = rnOekbPzBuWKFQn15117914;     rnOekbPzBuWKFQn15117914 = rnOekbPzBuWKFQn78963659;     rnOekbPzBuWKFQn78963659 = rnOekbPzBuWKFQn21834819;     rnOekbPzBuWKFQn21834819 = rnOekbPzBuWKFQn27070271;     rnOekbPzBuWKFQn27070271 = rnOekbPzBuWKFQn46959529;     rnOekbPzBuWKFQn46959529 = rnOekbPzBuWKFQn39323308;     rnOekbPzBuWKFQn39323308 = rnOekbPzBuWKFQn36733235;     rnOekbPzBuWKFQn36733235 = rnOekbPzBuWKFQn25915015;     rnOekbPzBuWKFQn25915015 = rnOekbPzBuWKFQn89263034;     rnOekbPzBuWKFQn89263034 = rnOekbPzBuWKFQn18538989;     rnOekbPzBuWKFQn18538989 = rnOekbPzBuWKFQn19887466;     rnOekbPzBuWKFQn19887466 = rnOekbPzBuWKFQn4602963;     rnOekbPzBuWKFQn4602963 = rnOekbPzBuWKFQn32573126;     rnOekbPzBuWKFQn32573126 = rnOekbPzBuWKFQn68870945;     rnOekbPzBuWKFQn68870945 = rnOekbPzBuWKFQn329587;     rnOekbPzBuWKFQn329587 = rnOekbPzBuWKFQn22346004;     rnOekbPzBuWKFQn22346004 = rnOekbPzBuWKFQn20909685;     rnOekbPzBuWKFQn20909685 = rnOekbPzBuWKFQn7348589;     rnOekbPzBuWKFQn7348589 = rnOekbPzBuWKFQn43282808;     rnOekbPzBuWKFQn43282808 = rnOekbPzBuWKFQn49064144;     rnOekbPzBuWKFQn49064144 = rnOekbPzBuWKFQn12566908;     rnOekbPzBuWKFQn12566908 = rnOekbPzBuWKFQn40384300;     rnOekbPzBuWKFQn40384300 = rnOekbPzBuWKFQn9368354;     rnOekbPzBuWKFQn9368354 = rnOekbPzBuWKFQn10048452;     rnOekbPzBuWKFQn10048452 = rnOekbPzBuWKFQn65039918;     rnOekbPzBuWKFQn65039918 = rnOekbPzBuWKFQn47350497;     rnOekbPzBuWKFQn47350497 = rnOekbPzBuWKFQn31213325;     rnOekbPzBuWKFQn31213325 = rnOekbPzBuWKFQn52270917;     rnOekbPzBuWKFQn52270917 = rnOekbPzBuWKFQn88338315;     rnOekbPzBuWKFQn88338315 = rnOekbPzBuWKFQn32568463;     rnOekbPzBuWKFQn32568463 = rnOekbPzBuWKFQn89955868;     rnOekbPzBuWKFQn89955868 = rnOekbPzBuWKFQn48793921;     rnOekbPzBuWKFQn48793921 = rnOekbPzBuWKFQn8939371;     rnOekbPzBuWKFQn8939371 = rnOekbPzBuWKFQn29020152;     rnOekbPzBuWKFQn29020152 = rnOekbPzBuWKFQn52306883;     rnOekbPzBuWKFQn52306883 = rnOekbPzBuWKFQn69432926;     rnOekbPzBuWKFQn69432926 = rnOekbPzBuWKFQn23753662;     rnOekbPzBuWKFQn23753662 = rnOekbPzBuWKFQn5624846;     rnOekbPzBuWKFQn5624846 = rnOekbPzBuWKFQn64310896;     rnOekbPzBuWKFQn64310896 = rnOekbPzBuWKFQn13996698;     rnOekbPzBuWKFQn13996698 = rnOekbPzBuWKFQn55694837;     rnOekbPzBuWKFQn55694837 = rnOekbPzBuWKFQn48173159;     rnOekbPzBuWKFQn48173159 = rnOekbPzBuWKFQn79386239;     rnOekbPzBuWKFQn79386239 = rnOekbPzBuWKFQn163100;     rnOekbPzBuWKFQn163100 = rnOekbPzBuWKFQn46059974;     rnOekbPzBuWKFQn46059974 = rnOekbPzBuWKFQn89114634;     rnOekbPzBuWKFQn89114634 = rnOekbPzBuWKFQn34798041;     rnOekbPzBuWKFQn34798041 = rnOekbPzBuWKFQn29149432;     rnOekbPzBuWKFQn29149432 = rnOekbPzBuWKFQn62717288;     rnOekbPzBuWKFQn62717288 = rnOekbPzBuWKFQn85118957;     rnOekbPzBuWKFQn85118957 = rnOekbPzBuWKFQn39278407;     rnOekbPzBuWKFQn39278407 = rnOekbPzBuWKFQn67080129;     rnOekbPzBuWKFQn67080129 = rnOekbPzBuWKFQn21718291;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void EcHEbCSTgilnvaWUfmWDEJRnewZYE54240366() {     int dnVWcUhwGpmZMYa18231212 = -381602560;    int dnVWcUhwGpmZMYa97102415 = 81039800;    int dnVWcUhwGpmZMYa38605886 = -894411775;    int dnVWcUhwGpmZMYa80621963 = -646744041;    int dnVWcUhwGpmZMYa25234149 = -890038638;    int dnVWcUhwGpmZMYa98106238 = -900877547;    int dnVWcUhwGpmZMYa10110395 = -482584099;    int dnVWcUhwGpmZMYa84178342 = -17184840;    int dnVWcUhwGpmZMYa72188740 = -63207938;    int dnVWcUhwGpmZMYa7609338 = -829639716;    int dnVWcUhwGpmZMYa78835129 = -561891539;    int dnVWcUhwGpmZMYa11619250 = -945092617;    int dnVWcUhwGpmZMYa46762914 = -971371341;    int dnVWcUhwGpmZMYa53730195 = -727492552;    int dnVWcUhwGpmZMYa13933331 = -899027195;    int dnVWcUhwGpmZMYa41027265 = -719538497;    int dnVWcUhwGpmZMYa3686740 = -730642355;    int dnVWcUhwGpmZMYa94893003 = -177662805;    int dnVWcUhwGpmZMYa96644640 = -404611536;    int dnVWcUhwGpmZMYa69958961 = -741391601;    int dnVWcUhwGpmZMYa22996647 = -737134980;    int dnVWcUhwGpmZMYa36705033 = 44129519;    int dnVWcUhwGpmZMYa82253517 = -253857335;    int dnVWcUhwGpmZMYa88209357 = -250786169;    int dnVWcUhwGpmZMYa26460657 = -954447090;    int dnVWcUhwGpmZMYa28003161 = 21962536;    int dnVWcUhwGpmZMYa76182925 = -123308591;    int dnVWcUhwGpmZMYa59675159 = -920577974;    int dnVWcUhwGpmZMYa30592557 = -727300960;    int dnVWcUhwGpmZMYa45842987 = -342074028;    int dnVWcUhwGpmZMYa41789196 = -316878234;    int dnVWcUhwGpmZMYa47020608 = -196593082;    int dnVWcUhwGpmZMYa70764444 = -963906152;    int dnVWcUhwGpmZMYa5087741 = -568876877;    int dnVWcUhwGpmZMYa83573012 = -195130519;    int dnVWcUhwGpmZMYa12320528 = -12471738;    int dnVWcUhwGpmZMYa99259054 = -44893281;    int dnVWcUhwGpmZMYa39074414 = 32173989;    int dnVWcUhwGpmZMYa42451904 = -242854919;    int dnVWcUhwGpmZMYa46888566 = -304707637;    int dnVWcUhwGpmZMYa30291373 = 47997257;    int dnVWcUhwGpmZMYa7240227 = -795720448;    int dnVWcUhwGpmZMYa95006896 = -331518717;    int dnVWcUhwGpmZMYa72396200 = -358076046;    int dnVWcUhwGpmZMYa40415434 = -315593287;    int dnVWcUhwGpmZMYa97072755 = 17280543;    int dnVWcUhwGpmZMYa39951519 = -491140881;    int dnVWcUhwGpmZMYa79756112 = -338463269;    int dnVWcUhwGpmZMYa33726982 = -603542737;    int dnVWcUhwGpmZMYa51764698 = 94156274;    int dnVWcUhwGpmZMYa73623458 = -390940142;    int dnVWcUhwGpmZMYa99478908 = -132138340;    int dnVWcUhwGpmZMYa39102907 = -840115883;    int dnVWcUhwGpmZMYa24830504 = -86271758;    int dnVWcUhwGpmZMYa31243719 = -68254986;    int dnVWcUhwGpmZMYa81526178 = -325732080;    int dnVWcUhwGpmZMYa14848898 = -665102866;    int dnVWcUhwGpmZMYa50396529 = -543625607;    int dnVWcUhwGpmZMYa54161306 = -692296951;    int dnVWcUhwGpmZMYa97230988 = -812001174;    int dnVWcUhwGpmZMYa21923314 = -677568956;    int dnVWcUhwGpmZMYa50435236 = -562006125;    int dnVWcUhwGpmZMYa53585785 = -289883880;    int dnVWcUhwGpmZMYa26345753 = -721133910;    int dnVWcUhwGpmZMYa65820142 = -412761482;    int dnVWcUhwGpmZMYa31814521 = -265298458;    int dnVWcUhwGpmZMYa40854806 = -981186465;    int dnVWcUhwGpmZMYa41675173 = -302494464;    int dnVWcUhwGpmZMYa70157182 = -432362033;    int dnVWcUhwGpmZMYa1612804 = -786555457;    int dnVWcUhwGpmZMYa41768211 = -574645217;    int dnVWcUhwGpmZMYa64612326 = -662816344;    int dnVWcUhwGpmZMYa52441100 = -934807886;    int dnVWcUhwGpmZMYa49756074 = 96101;    int dnVWcUhwGpmZMYa39667589 = -689388859;    int dnVWcUhwGpmZMYa15756421 = -941414532;    int dnVWcUhwGpmZMYa41698137 = -624351765;    int dnVWcUhwGpmZMYa9857318 = -895781289;    int dnVWcUhwGpmZMYa47793924 = -935192882;    int dnVWcUhwGpmZMYa29387902 = -871727634;    int dnVWcUhwGpmZMYa88051642 = -486896584;    int dnVWcUhwGpmZMYa96426813 = -784845322;    int dnVWcUhwGpmZMYa25948177 = -217035238;    int dnVWcUhwGpmZMYa78827858 = -721457235;    int dnVWcUhwGpmZMYa72219529 = -951133887;    int dnVWcUhwGpmZMYa42310287 = -84739895;    int dnVWcUhwGpmZMYa7917702 = -356477199;    int dnVWcUhwGpmZMYa45933941 = -777634394;    int dnVWcUhwGpmZMYa73844021 = -400621892;    int dnVWcUhwGpmZMYa2046835 = -869398440;    int dnVWcUhwGpmZMYa97471629 = -347368872;    int dnVWcUhwGpmZMYa48862525 = -501267674;    int dnVWcUhwGpmZMYa84913107 = -275529061;    int dnVWcUhwGpmZMYa45220915 = -430853745;    int dnVWcUhwGpmZMYa24965253 = -627138682;    int dnVWcUhwGpmZMYa79856136 = -389996618;    int dnVWcUhwGpmZMYa53654441 = -405836568;    int dnVWcUhwGpmZMYa68661144 = -610384808;    int dnVWcUhwGpmZMYa6576059 = -945314564;    int dnVWcUhwGpmZMYa8600913 = -381602560;     dnVWcUhwGpmZMYa18231212 = dnVWcUhwGpmZMYa97102415;     dnVWcUhwGpmZMYa97102415 = dnVWcUhwGpmZMYa38605886;     dnVWcUhwGpmZMYa38605886 = dnVWcUhwGpmZMYa80621963;     dnVWcUhwGpmZMYa80621963 = dnVWcUhwGpmZMYa25234149;     dnVWcUhwGpmZMYa25234149 = dnVWcUhwGpmZMYa98106238;     dnVWcUhwGpmZMYa98106238 = dnVWcUhwGpmZMYa10110395;     dnVWcUhwGpmZMYa10110395 = dnVWcUhwGpmZMYa84178342;     dnVWcUhwGpmZMYa84178342 = dnVWcUhwGpmZMYa72188740;     dnVWcUhwGpmZMYa72188740 = dnVWcUhwGpmZMYa7609338;     dnVWcUhwGpmZMYa7609338 = dnVWcUhwGpmZMYa78835129;     dnVWcUhwGpmZMYa78835129 = dnVWcUhwGpmZMYa11619250;     dnVWcUhwGpmZMYa11619250 = dnVWcUhwGpmZMYa46762914;     dnVWcUhwGpmZMYa46762914 = dnVWcUhwGpmZMYa53730195;     dnVWcUhwGpmZMYa53730195 = dnVWcUhwGpmZMYa13933331;     dnVWcUhwGpmZMYa13933331 = dnVWcUhwGpmZMYa41027265;     dnVWcUhwGpmZMYa41027265 = dnVWcUhwGpmZMYa3686740;     dnVWcUhwGpmZMYa3686740 = dnVWcUhwGpmZMYa94893003;     dnVWcUhwGpmZMYa94893003 = dnVWcUhwGpmZMYa96644640;     dnVWcUhwGpmZMYa96644640 = dnVWcUhwGpmZMYa69958961;     dnVWcUhwGpmZMYa69958961 = dnVWcUhwGpmZMYa22996647;     dnVWcUhwGpmZMYa22996647 = dnVWcUhwGpmZMYa36705033;     dnVWcUhwGpmZMYa36705033 = dnVWcUhwGpmZMYa82253517;     dnVWcUhwGpmZMYa82253517 = dnVWcUhwGpmZMYa88209357;     dnVWcUhwGpmZMYa88209357 = dnVWcUhwGpmZMYa26460657;     dnVWcUhwGpmZMYa26460657 = dnVWcUhwGpmZMYa28003161;     dnVWcUhwGpmZMYa28003161 = dnVWcUhwGpmZMYa76182925;     dnVWcUhwGpmZMYa76182925 = dnVWcUhwGpmZMYa59675159;     dnVWcUhwGpmZMYa59675159 = dnVWcUhwGpmZMYa30592557;     dnVWcUhwGpmZMYa30592557 = dnVWcUhwGpmZMYa45842987;     dnVWcUhwGpmZMYa45842987 = dnVWcUhwGpmZMYa41789196;     dnVWcUhwGpmZMYa41789196 = dnVWcUhwGpmZMYa47020608;     dnVWcUhwGpmZMYa47020608 = dnVWcUhwGpmZMYa70764444;     dnVWcUhwGpmZMYa70764444 = dnVWcUhwGpmZMYa5087741;     dnVWcUhwGpmZMYa5087741 = dnVWcUhwGpmZMYa83573012;     dnVWcUhwGpmZMYa83573012 = dnVWcUhwGpmZMYa12320528;     dnVWcUhwGpmZMYa12320528 = dnVWcUhwGpmZMYa99259054;     dnVWcUhwGpmZMYa99259054 = dnVWcUhwGpmZMYa39074414;     dnVWcUhwGpmZMYa39074414 = dnVWcUhwGpmZMYa42451904;     dnVWcUhwGpmZMYa42451904 = dnVWcUhwGpmZMYa46888566;     dnVWcUhwGpmZMYa46888566 = dnVWcUhwGpmZMYa30291373;     dnVWcUhwGpmZMYa30291373 = dnVWcUhwGpmZMYa7240227;     dnVWcUhwGpmZMYa7240227 = dnVWcUhwGpmZMYa95006896;     dnVWcUhwGpmZMYa95006896 = dnVWcUhwGpmZMYa72396200;     dnVWcUhwGpmZMYa72396200 = dnVWcUhwGpmZMYa40415434;     dnVWcUhwGpmZMYa40415434 = dnVWcUhwGpmZMYa97072755;     dnVWcUhwGpmZMYa97072755 = dnVWcUhwGpmZMYa39951519;     dnVWcUhwGpmZMYa39951519 = dnVWcUhwGpmZMYa79756112;     dnVWcUhwGpmZMYa79756112 = dnVWcUhwGpmZMYa33726982;     dnVWcUhwGpmZMYa33726982 = dnVWcUhwGpmZMYa51764698;     dnVWcUhwGpmZMYa51764698 = dnVWcUhwGpmZMYa73623458;     dnVWcUhwGpmZMYa73623458 = dnVWcUhwGpmZMYa99478908;     dnVWcUhwGpmZMYa99478908 = dnVWcUhwGpmZMYa39102907;     dnVWcUhwGpmZMYa39102907 = dnVWcUhwGpmZMYa24830504;     dnVWcUhwGpmZMYa24830504 = dnVWcUhwGpmZMYa31243719;     dnVWcUhwGpmZMYa31243719 = dnVWcUhwGpmZMYa81526178;     dnVWcUhwGpmZMYa81526178 = dnVWcUhwGpmZMYa14848898;     dnVWcUhwGpmZMYa14848898 = dnVWcUhwGpmZMYa50396529;     dnVWcUhwGpmZMYa50396529 = dnVWcUhwGpmZMYa54161306;     dnVWcUhwGpmZMYa54161306 = dnVWcUhwGpmZMYa97230988;     dnVWcUhwGpmZMYa97230988 = dnVWcUhwGpmZMYa21923314;     dnVWcUhwGpmZMYa21923314 = dnVWcUhwGpmZMYa50435236;     dnVWcUhwGpmZMYa50435236 = dnVWcUhwGpmZMYa53585785;     dnVWcUhwGpmZMYa53585785 = dnVWcUhwGpmZMYa26345753;     dnVWcUhwGpmZMYa26345753 = dnVWcUhwGpmZMYa65820142;     dnVWcUhwGpmZMYa65820142 = dnVWcUhwGpmZMYa31814521;     dnVWcUhwGpmZMYa31814521 = dnVWcUhwGpmZMYa40854806;     dnVWcUhwGpmZMYa40854806 = dnVWcUhwGpmZMYa41675173;     dnVWcUhwGpmZMYa41675173 = dnVWcUhwGpmZMYa70157182;     dnVWcUhwGpmZMYa70157182 = dnVWcUhwGpmZMYa1612804;     dnVWcUhwGpmZMYa1612804 = dnVWcUhwGpmZMYa41768211;     dnVWcUhwGpmZMYa41768211 = dnVWcUhwGpmZMYa64612326;     dnVWcUhwGpmZMYa64612326 = dnVWcUhwGpmZMYa52441100;     dnVWcUhwGpmZMYa52441100 = dnVWcUhwGpmZMYa49756074;     dnVWcUhwGpmZMYa49756074 = dnVWcUhwGpmZMYa39667589;     dnVWcUhwGpmZMYa39667589 = dnVWcUhwGpmZMYa15756421;     dnVWcUhwGpmZMYa15756421 = dnVWcUhwGpmZMYa41698137;     dnVWcUhwGpmZMYa41698137 = dnVWcUhwGpmZMYa9857318;     dnVWcUhwGpmZMYa9857318 = dnVWcUhwGpmZMYa47793924;     dnVWcUhwGpmZMYa47793924 = dnVWcUhwGpmZMYa29387902;     dnVWcUhwGpmZMYa29387902 = dnVWcUhwGpmZMYa88051642;     dnVWcUhwGpmZMYa88051642 = dnVWcUhwGpmZMYa96426813;     dnVWcUhwGpmZMYa96426813 = dnVWcUhwGpmZMYa25948177;     dnVWcUhwGpmZMYa25948177 = dnVWcUhwGpmZMYa78827858;     dnVWcUhwGpmZMYa78827858 = dnVWcUhwGpmZMYa72219529;     dnVWcUhwGpmZMYa72219529 = dnVWcUhwGpmZMYa42310287;     dnVWcUhwGpmZMYa42310287 = dnVWcUhwGpmZMYa7917702;     dnVWcUhwGpmZMYa7917702 = dnVWcUhwGpmZMYa45933941;     dnVWcUhwGpmZMYa45933941 = dnVWcUhwGpmZMYa73844021;     dnVWcUhwGpmZMYa73844021 = dnVWcUhwGpmZMYa2046835;     dnVWcUhwGpmZMYa2046835 = dnVWcUhwGpmZMYa97471629;     dnVWcUhwGpmZMYa97471629 = dnVWcUhwGpmZMYa48862525;     dnVWcUhwGpmZMYa48862525 = dnVWcUhwGpmZMYa84913107;     dnVWcUhwGpmZMYa84913107 = dnVWcUhwGpmZMYa45220915;     dnVWcUhwGpmZMYa45220915 = dnVWcUhwGpmZMYa24965253;     dnVWcUhwGpmZMYa24965253 = dnVWcUhwGpmZMYa79856136;     dnVWcUhwGpmZMYa79856136 = dnVWcUhwGpmZMYa53654441;     dnVWcUhwGpmZMYa53654441 = dnVWcUhwGpmZMYa68661144;     dnVWcUhwGpmZMYa68661144 = dnVWcUhwGpmZMYa6576059;     dnVWcUhwGpmZMYa6576059 = dnVWcUhwGpmZMYa8600913;     dnVWcUhwGpmZMYa8600913 = dnVWcUhwGpmZMYa18231212;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nHBNbHyKSXLglmTaarQUFfaMlBUPd4187173() {     int QNLMRleCmRSwHFB85402271 = -26028237;    int QNLMRleCmRSwHFB42286441 = -800696676;    int QNLMRleCmRSwHFB72943904 = -27680387;    int QNLMRleCmRSwHFB48022238 = -385155391;    int QNLMRleCmRSwHFB6496759 = -860676660;    int QNLMRleCmRSwHFB90547441 = -933553463;    int QNLMRleCmRSwHFB33756608 = -941741316;    int QNLMRleCmRSwHFB7258843 = -772708797;    int QNLMRleCmRSwHFB39567887 = -695326950;    int QNLMRleCmRSwHFB87940821 = -52653409;    int QNLMRleCmRSwHFB60979800 = -612556633;    int QNLMRleCmRSwHFB57890581 = -994110953;    int QNLMRleCmRSwHFB8085 = -45035588;    int QNLMRleCmRSwHFB50808122 = -692889245;    int QNLMRleCmRSwHFB56380998 = -937391056;    int QNLMRleCmRSwHFB11948182 = -815347649;    int QNLMRleCmRSwHFB69377656 = -733059960;    int QNLMRleCmRSwHFB28321453 = -387283444;    int QNLMRleCmRSwHFB39847128 = -982944511;    int QNLMRleCmRSwHFB97397166 = -581258285;    int QNLMRleCmRSwHFB6141833 = -45897345;    int QNLMRleCmRSwHFB60706290 = -155181596;    int QNLMRleCmRSwHFB36815892 = -643662348;    int QNLMRleCmRSwHFB2233557 = 78616044;    int QNLMRleCmRSwHFB40417363 = -829083684;    int QNLMRleCmRSwHFB18051029 = -347411243;    int QNLMRleCmRSwHFB62118814 = -44868500;    int QNLMRleCmRSwHFB24304222 = -472490852;    int QNLMRleCmRSwHFB32252569 = -240966845;    int QNLMRleCmRSwHFB9179721 = -589814568;    int QNLMRleCmRSwHFB24542594 = 19813161;    int QNLMRleCmRSwHFB42558395 = -465074503;    int QNLMRleCmRSwHFB85701820 = -599020312;    int QNLMRleCmRSwHFB82829546 = -76234829;    int QNLMRleCmRSwHFB88724049 = 2571100;    int QNLMRleCmRSwHFB73156850 = -289009248;    int QNLMRleCmRSwHFB11666140 = -744257068;    int QNLMRleCmRSwHFB95076197 = -677808409;    int QNLMRleCmRSwHFB34405903 = -926970890;    int QNLMRleCmRSwHFB49636371 = -749232196;    int QNLMRleCmRSwHFB34779984 = -779619893;    int QNLMRleCmRSwHFB94218909 = -878758398;    int QNLMRleCmRSwHFB71232433 = -255415087;    int QNLMRleCmRSwHFB31880711 = 85279673;    int QNLMRleCmRSwHFB18338155 = -683134617;    int QNLMRleCmRSwHFB60731648 = -261339737;    int QNLMRleCmRSwHFB35297022 = -404450706;    int QNLMRleCmRSwHFB50476764 = -344926207;    int QNLMRleCmRSwHFB39119113 = -557047040;    int QNLMRleCmRSwHFB12947801 = -951367173;    int QNLMRleCmRSwHFB80342053 = -988336462;    int QNLMRleCmRSwHFB99211794 = -301377564;    int QNLMRleCmRSwHFB10873595 = -567768561;    int QNLMRleCmRSwHFB90314733 = -665434407;    int QNLMRleCmRSwHFB98045340 = -855018415;    int QNLMRleCmRSwHFB24695981 = -870846642;    int QNLMRleCmRSwHFB5470550 = -57034329;    int QNLMRleCmRSwHFB70710348 = -6296431;    int QNLMRleCmRSwHFB7604875 = -556071707;    int QNLMRleCmRSwHFB88445729 = -413265417;    int QNLMRleCmRSwHFB28428627 = -788684963;    int QNLMRleCmRSwHFB9452386 = -369250464;    int QNLMRleCmRSwHFB75006273 = -431741952;    int QNLMRleCmRSwHFB30388167 = -5512383;    int QNLMRleCmRSwHFB63398228 = 27533429;    int QNLMRleCmRSwHFB18421406 = -47482131;    int QNLMRleCmRSwHFB72188761 = -295090641;    int QNLMRleCmRSwHFB17178538 = -968800760;    int QNLMRleCmRSwHFB62084072 = -595460346;    int QNLMRleCmRSwHFB83224148 = -548381809;    int QNLMRleCmRSwHFB282043 = 28909418;    int QNLMRleCmRSwHFB74301459 = 44748448;    int QNLMRleCmRSwHFB93915550 = -460312554;    int QNLMRleCmRSwHFB90210757 = -133712316;    int QNLMRleCmRSwHFB62617183 = -801638393;    int QNLMRleCmRSwHFB11922924 = -167138947;    int QNLMRleCmRSwHFB89473857 = -899766510;    int QNLMRleCmRSwHFB4935182 = -628942021;    int QNLMRleCmRSwHFB83895401 = -238249339;    int QNLMRleCmRSwHFB79685715 = -467743948;    int QNLMRleCmRSwHFB82754007 = -942960538;    int QNLMRleCmRSwHFB11642050 = -699942293;    int QNLMRleCmRSwHFB85185109 = -915443813;    int QNLMRleCmRSwHFB19304769 = -289599672;    int QNLMRleCmRSwHFB28837668 = -601478106;    int QNLMRleCmRSwHFB25330799 = -678809275;    int QNLMRleCmRSwHFB31684801 = -897305943;    int QNLMRleCmRSwHFB95387087 = -933585906;    int QNLMRleCmRSwHFB84784206 = -221216414;    int QNLMRleCmRSwHFB64028068 = -126582258;    int QNLMRleCmRSwHFB67686301 = -131974919;    int QNLMRleCmRSwHFB40955792 = -637960637;    int QNLMRleCmRSwHFB87471322 = -21736702;    int QNLMRleCmRSwHFB45960173 = -413705474;    int QNLMRleCmRSwHFB21207744 = -960547233;    int QNLMRleCmRSwHFB25327598 = -310369429;    int QNLMRleCmRSwHFB19212636 = -347016446;    int QNLMRleCmRSwHFB40844266 = -149902704;    int QNLMRleCmRSwHFB68482482 = -942253757;    int QNLMRleCmRSwHFB99916749 = -26028237;     QNLMRleCmRSwHFB85402271 = QNLMRleCmRSwHFB42286441;     QNLMRleCmRSwHFB42286441 = QNLMRleCmRSwHFB72943904;     QNLMRleCmRSwHFB72943904 = QNLMRleCmRSwHFB48022238;     QNLMRleCmRSwHFB48022238 = QNLMRleCmRSwHFB6496759;     QNLMRleCmRSwHFB6496759 = QNLMRleCmRSwHFB90547441;     QNLMRleCmRSwHFB90547441 = QNLMRleCmRSwHFB33756608;     QNLMRleCmRSwHFB33756608 = QNLMRleCmRSwHFB7258843;     QNLMRleCmRSwHFB7258843 = QNLMRleCmRSwHFB39567887;     QNLMRleCmRSwHFB39567887 = QNLMRleCmRSwHFB87940821;     QNLMRleCmRSwHFB87940821 = QNLMRleCmRSwHFB60979800;     QNLMRleCmRSwHFB60979800 = QNLMRleCmRSwHFB57890581;     QNLMRleCmRSwHFB57890581 = QNLMRleCmRSwHFB8085;     QNLMRleCmRSwHFB8085 = QNLMRleCmRSwHFB50808122;     QNLMRleCmRSwHFB50808122 = QNLMRleCmRSwHFB56380998;     QNLMRleCmRSwHFB56380998 = QNLMRleCmRSwHFB11948182;     QNLMRleCmRSwHFB11948182 = QNLMRleCmRSwHFB69377656;     QNLMRleCmRSwHFB69377656 = QNLMRleCmRSwHFB28321453;     QNLMRleCmRSwHFB28321453 = QNLMRleCmRSwHFB39847128;     QNLMRleCmRSwHFB39847128 = QNLMRleCmRSwHFB97397166;     QNLMRleCmRSwHFB97397166 = QNLMRleCmRSwHFB6141833;     QNLMRleCmRSwHFB6141833 = QNLMRleCmRSwHFB60706290;     QNLMRleCmRSwHFB60706290 = QNLMRleCmRSwHFB36815892;     QNLMRleCmRSwHFB36815892 = QNLMRleCmRSwHFB2233557;     QNLMRleCmRSwHFB2233557 = QNLMRleCmRSwHFB40417363;     QNLMRleCmRSwHFB40417363 = QNLMRleCmRSwHFB18051029;     QNLMRleCmRSwHFB18051029 = QNLMRleCmRSwHFB62118814;     QNLMRleCmRSwHFB62118814 = QNLMRleCmRSwHFB24304222;     QNLMRleCmRSwHFB24304222 = QNLMRleCmRSwHFB32252569;     QNLMRleCmRSwHFB32252569 = QNLMRleCmRSwHFB9179721;     QNLMRleCmRSwHFB9179721 = QNLMRleCmRSwHFB24542594;     QNLMRleCmRSwHFB24542594 = QNLMRleCmRSwHFB42558395;     QNLMRleCmRSwHFB42558395 = QNLMRleCmRSwHFB85701820;     QNLMRleCmRSwHFB85701820 = QNLMRleCmRSwHFB82829546;     QNLMRleCmRSwHFB82829546 = QNLMRleCmRSwHFB88724049;     QNLMRleCmRSwHFB88724049 = QNLMRleCmRSwHFB73156850;     QNLMRleCmRSwHFB73156850 = QNLMRleCmRSwHFB11666140;     QNLMRleCmRSwHFB11666140 = QNLMRleCmRSwHFB95076197;     QNLMRleCmRSwHFB95076197 = QNLMRleCmRSwHFB34405903;     QNLMRleCmRSwHFB34405903 = QNLMRleCmRSwHFB49636371;     QNLMRleCmRSwHFB49636371 = QNLMRleCmRSwHFB34779984;     QNLMRleCmRSwHFB34779984 = QNLMRleCmRSwHFB94218909;     QNLMRleCmRSwHFB94218909 = QNLMRleCmRSwHFB71232433;     QNLMRleCmRSwHFB71232433 = QNLMRleCmRSwHFB31880711;     QNLMRleCmRSwHFB31880711 = QNLMRleCmRSwHFB18338155;     QNLMRleCmRSwHFB18338155 = QNLMRleCmRSwHFB60731648;     QNLMRleCmRSwHFB60731648 = QNLMRleCmRSwHFB35297022;     QNLMRleCmRSwHFB35297022 = QNLMRleCmRSwHFB50476764;     QNLMRleCmRSwHFB50476764 = QNLMRleCmRSwHFB39119113;     QNLMRleCmRSwHFB39119113 = QNLMRleCmRSwHFB12947801;     QNLMRleCmRSwHFB12947801 = QNLMRleCmRSwHFB80342053;     QNLMRleCmRSwHFB80342053 = QNLMRleCmRSwHFB99211794;     QNLMRleCmRSwHFB99211794 = QNLMRleCmRSwHFB10873595;     QNLMRleCmRSwHFB10873595 = QNLMRleCmRSwHFB90314733;     QNLMRleCmRSwHFB90314733 = QNLMRleCmRSwHFB98045340;     QNLMRleCmRSwHFB98045340 = QNLMRleCmRSwHFB24695981;     QNLMRleCmRSwHFB24695981 = QNLMRleCmRSwHFB5470550;     QNLMRleCmRSwHFB5470550 = QNLMRleCmRSwHFB70710348;     QNLMRleCmRSwHFB70710348 = QNLMRleCmRSwHFB7604875;     QNLMRleCmRSwHFB7604875 = QNLMRleCmRSwHFB88445729;     QNLMRleCmRSwHFB88445729 = QNLMRleCmRSwHFB28428627;     QNLMRleCmRSwHFB28428627 = QNLMRleCmRSwHFB9452386;     QNLMRleCmRSwHFB9452386 = QNLMRleCmRSwHFB75006273;     QNLMRleCmRSwHFB75006273 = QNLMRleCmRSwHFB30388167;     QNLMRleCmRSwHFB30388167 = QNLMRleCmRSwHFB63398228;     QNLMRleCmRSwHFB63398228 = QNLMRleCmRSwHFB18421406;     QNLMRleCmRSwHFB18421406 = QNLMRleCmRSwHFB72188761;     QNLMRleCmRSwHFB72188761 = QNLMRleCmRSwHFB17178538;     QNLMRleCmRSwHFB17178538 = QNLMRleCmRSwHFB62084072;     QNLMRleCmRSwHFB62084072 = QNLMRleCmRSwHFB83224148;     QNLMRleCmRSwHFB83224148 = QNLMRleCmRSwHFB282043;     QNLMRleCmRSwHFB282043 = QNLMRleCmRSwHFB74301459;     QNLMRleCmRSwHFB74301459 = QNLMRleCmRSwHFB93915550;     QNLMRleCmRSwHFB93915550 = QNLMRleCmRSwHFB90210757;     QNLMRleCmRSwHFB90210757 = QNLMRleCmRSwHFB62617183;     QNLMRleCmRSwHFB62617183 = QNLMRleCmRSwHFB11922924;     QNLMRleCmRSwHFB11922924 = QNLMRleCmRSwHFB89473857;     QNLMRleCmRSwHFB89473857 = QNLMRleCmRSwHFB4935182;     QNLMRleCmRSwHFB4935182 = QNLMRleCmRSwHFB83895401;     QNLMRleCmRSwHFB83895401 = QNLMRleCmRSwHFB79685715;     QNLMRleCmRSwHFB79685715 = QNLMRleCmRSwHFB82754007;     QNLMRleCmRSwHFB82754007 = QNLMRleCmRSwHFB11642050;     QNLMRleCmRSwHFB11642050 = QNLMRleCmRSwHFB85185109;     QNLMRleCmRSwHFB85185109 = QNLMRleCmRSwHFB19304769;     QNLMRleCmRSwHFB19304769 = QNLMRleCmRSwHFB28837668;     QNLMRleCmRSwHFB28837668 = QNLMRleCmRSwHFB25330799;     QNLMRleCmRSwHFB25330799 = QNLMRleCmRSwHFB31684801;     QNLMRleCmRSwHFB31684801 = QNLMRleCmRSwHFB95387087;     QNLMRleCmRSwHFB95387087 = QNLMRleCmRSwHFB84784206;     QNLMRleCmRSwHFB84784206 = QNLMRleCmRSwHFB64028068;     QNLMRleCmRSwHFB64028068 = QNLMRleCmRSwHFB67686301;     QNLMRleCmRSwHFB67686301 = QNLMRleCmRSwHFB40955792;     QNLMRleCmRSwHFB40955792 = QNLMRleCmRSwHFB87471322;     QNLMRleCmRSwHFB87471322 = QNLMRleCmRSwHFB45960173;     QNLMRleCmRSwHFB45960173 = QNLMRleCmRSwHFB21207744;     QNLMRleCmRSwHFB21207744 = QNLMRleCmRSwHFB25327598;     QNLMRleCmRSwHFB25327598 = QNLMRleCmRSwHFB19212636;     QNLMRleCmRSwHFB19212636 = QNLMRleCmRSwHFB40844266;     QNLMRleCmRSwHFB40844266 = QNLMRleCmRSwHFB68482482;     QNLMRleCmRSwHFB68482482 = QNLMRleCmRSwHFB99916749;     QNLMRleCmRSwHFB99916749 = QNLMRleCmRSwHFB85402271;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YHRNHsSzRFfHTsccSsrlTJmIUJwsq6376512() {     int pOKcRMBeFBxNkwr81915192 = -24828925;    int pOKcRMBeFBxNkwr57369267 = -753384979;    int pOKcRMBeFBxNkwr28626215 = -875783152;    int pOKcRMBeFBxNkwr60571086 = -853969181;    int pOKcRMBeFBxNkwr75149148 = -416263656;    int pOKcRMBeFBxNkwr27794313 = -964081039;    int pOKcRMBeFBxNkwr7571261 = -6659291;    int pOKcRMBeFBxNkwr42711012 = -885592981;    int pOKcRMBeFBxNkwr43822144 = -947744704;    int pOKcRMBeFBxNkwr41945358 = -811378621;    int pOKcRMBeFBxNkwr21195916 = -587791871;    int pOKcRMBeFBxNkwr72907018 = -462843858;    int pOKcRMBeFBxNkwr5278783 = -330821252;    int pOKcRMBeFBxNkwr17883235 = -102410780;    int pOKcRMBeFBxNkwr61280756 = -788717766;    int pOKcRMBeFBxNkwr24905005 = -573968864;    int pOKcRMBeFBxNkwr97068504 = -645907005;    int pOKcRMBeFBxNkwr64456941 = 95190491;    int pOKcRMBeFBxNkwr21770105 = -372071812;    int pOKcRMBeFBxNkwr38122426 = -490783116;    int pOKcRMBeFBxNkwr13820688 = -256446943;    int pOKcRMBeFBxNkwr1608048 = -533199289;    int pOKcRMBeFBxNkwr26312855 = -924582058;    int pOKcRMBeFBxNkwr26058327 = -980501361;    int pOKcRMBeFBxNkwr18692371 = -354509444;    int pOKcRMBeFBxNkwr94075393 = -526706506;    int pOKcRMBeFBxNkwr10015499 = -972050347;    int pOKcRMBeFBxNkwr16554585 = -128246951;    int pOKcRMBeFBxNkwr14448540 = -106344828;    int pOKcRMBeFBxNkwr9434229 = 45408590;    int pOKcRMBeFBxNkwr33636673 = -282154992;    int pOKcRMBeFBxNkwr78308578 = -797080524;    int pOKcRMBeFBxNkwr3146260 = -489117080;    int pOKcRMBeFBxNkwr95489214 = -996744829;    int pOKcRMBeFBxNkwr98208887 = -212094425;    int pOKcRMBeFBxNkwr16828104 = -566085956;    int pOKcRMBeFBxNkwr92223106 = -254681025;    int pOKcRMBeFBxNkwr68203171 = -498940377;    int pOKcRMBeFBxNkwr83140209 = -168039640;    int pOKcRMBeFBxNkwr29153770 = -185580078;    int pOKcRMBeFBxNkwr67050978 = -211162064;    int pOKcRMBeFBxNkwr38412261 = -286199360;    int pOKcRMBeFBxNkwr58774369 = -751687659;    int pOKcRMBeFBxNkwr44088819 = -56354081;    int pOKcRMBeFBxNkwr84324869 = -115288955;    int pOKcRMBeFBxNkwr58412675 = -593667533;    int pOKcRMBeFBxNkwr32209115 = -682971355;    int pOKcRMBeFBxNkwr30966788 = -508020207;    int pOKcRMBeFBxNkwr57728180 = -88551235;    int pOKcRMBeFBxNkwr85748839 = -200906629;    int pOKcRMBeFBxNkwr32130694 = -517715669;    int pOKcRMBeFBxNkwr71620432 = -883542814;    int pOKcRMBeFBxNkwr3016973 = -259677205;    int pOKcRMBeFBxNkwr75821928 = -15635411;    int pOKcRMBeFBxNkwr92555824 = -187740207;    int pOKcRMBeFBxNkwr80307144 = -491629637;    int pOKcRMBeFBxNkwr31056413 = -828802921;    int pOKcRMBeFBxNkwr2567889 = -895281792;    int pOKcRMBeFBxNkwr41878716 = -399459738;    int pOKcRMBeFBxNkwr81073754 = -889557150;    int pOKcRMBeFBxNkwr17778815 = -992030693;    int pOKcRMBeFBxNkwr91016676 = -878412341;    int pOKcRMBeFBxNkwr28262473 = -679248153;    int pOKcRMBeFBxNkwr34387916 = -893153294;    int pOKcRMBeFBxNkwr8308685 = -429223630;    int pOKcRMBeFBxNkwr42887338 = -790711348;    int pOKcRMBeFBxNkwr69760759 = -973726779;    int pOKcRMBeFBxNkwr9789568 = -334076423;    int pOKcRMBeFBxNkwr19674347 = -890316356;    int pOKcRMBeFBxNkwr44452652 = -122631810;    int pOKcRMBeFBxNkwr32681899 = -219287839;    int pOKcRMBeFBxNkwr28865334 = -46966628;    int pOKcRMBeFBxNkwr81316731 = -736769869;    int pOKcRMBeFBxNkwr92616334 = -86491734;    int pOKcRMBeFBxNkwr71071447 = -179621053;    int pOKcRMBeFBxNkwr75408426 = -970247583;    int pOKcRMBeFBxNkwr42833678 = -781511630;    int pOKcRMBeFBxNkwr82224035 = -768227977;    int pOKcRMBeFBxNkwr41733457 = -765212406;    int pOKcRMBeFBxNkwr60279695 = -760841911;    int pOKcRMBeFBxNkwr61866279 = -843735152;    int pOKcRMBeFBxNkwr79048711 = -364030141;    int pOKcRMBeFBxNkwr58826404 = 60304283;    int pOKcRMBeFBxNkwr28699701 = -905438200;    int pOKcRMBeFBxNkwr77303535 = -436875742;    int pOKcRMBeFBxNkwr62016240 = -398612178;    int pOKcRMBeFBxNkwr75291605 = -437403319;    int pOKcRMBeFBxNkwr27324331 = -373481669;    int pOKcRMBeFBxNkwr2933391 = -709004622;    int pOKcRMBeFBxNkwr17901744 = -720464789;    int pOKcRMBeFBxNkwr85771691 = -737283036;    int pOKcRMBeFBxNkwr89655217 = -359399234;    int pOKcRMBeFBxNkwr26324456 = 519360;    int pOKcRMBeFBxNkwr2066455 = -278482491;    int pOKcRMBeFBxNkwr11374956 = -193549386;    int pOKcRMBeFBxNkwr76034302 = -332749724;    int pOKcRMBeFBxNkwr10149789 = -606951207;    int pOKcRMBeFBxNkwr24386453 = -858534365;    int pOKcRMBeFBxNkwr35780134 = -627130452;    int pOKcRMBeFBxNkwr41437532 = -24828925;     pOKcRMBeFBxNkwr81915192 = pOKcRMBeFBxNkwr57369267;     pOKcRMBeFBxNkwr57369267 = pOKcRMBeFBxNkwr28626215;     pOKcRMBeFBxNkwr28626215 = pOKcRMBeFBxNkwr60571086;     pOKcRMBeFBxNkwr60571086 = pOKcRMBeFBxNkwr75149148;     pOKcRMBeFBxNkwr75149148 = pOKcRMBeFBxNkwr27794313;     pOKcRMBeFBxNkwr27794313 = pOKcRMBeFBxNkwr7571261;     pOKcRMBeFBxNkwr7571261 = pOKcRMBeFBxNkwr42711012;     pOKcRMBeFBxNkwr42711012 = pOKcRMBeFBxNkwr43822144;     pOKcRMBeFBxNkwr43822144 = pOKcRMBeFBxNkwr41945358;     pOKcRMBeFBxNkwr41945358 = pOKcRMBeFBxNkwr21195916;     pOKcRMBeFBxNkwr21195916 = pOKcRMBeFBxNkwr72907018;     pOKcRMBeFBxNkwr72907018 = pOKcRMBeFBxNkwr5278783;     pOKcRMBeFBxNkwr5278783 = pOKcRMBeFBxNkwr17883235;     pOKcRMBeFBxNkwr17883235 = pOKcRMBeFBxNkwr61280756;     pOKcRMBeFBxNkwr61280756 = pOKcRMBeFBxNkwr24905005;     pOKcRMBeFBxNkwr24905005 = pOKcRMBeFBxNkwr97068504;     pOKcRMBeFBxNkwr97068504 = pOKcRMBeFBxNkwr64456941;     pOKcRMBeFBxNkwr64456941 = pOKcRMBeFBxNkwr21770105;     pOKcRMBeFBxNkwr21770105 = pOKcRMBeFBxNkwr38122426;     pOKcRMBeFBxNkwr38122426 = pOKcRMBeFBxNkwr13820688;     pOKcRMBeFBxNkwr13820688 = pOKcRMBeFBxNkwr1608048;     pOKcRMBeFBxNkwr1608048 = pOKcRMBeFBxNkwr26312855;     pOKcRMBeFBxNkwr26312855 = pOKcRMBeFBxNkwr26058327;     pOKcRMBeFBxNkwr26058327 = pOKcRMBeFBxNkwr18692371;     pOKcRMBeFBxNkwr18692371 = pOKcRMBeFBxNkwr94075393;     pOKcRMBeFBxNkwr94075393 = pOKcRMBeFBxNkwr10015499;     pOKcRMBeFBxNkwr10015499 = pOKcRMBeFBxNkwr16554585;     pOKcRMBeFBxNkwr16554585 = pOKcRMBeFBxNkwr14448540;     pOKcRMBeFBxNkwr14448540 = pOKcRMBeFBxNkwr9434229;     pOKcRMBeFBxNkwr9434229 = pOKcRMBeFBxNkwr33636673;     pOKcRMBeFBxNkwr33636673 = pOKcRMBeFBxNkwr78308578;     pOKcRMBeFBxNkwr78308578 = pOKcRMBeFBxNkwr3146260;     pOKcRMBeFBxNkwr3146260 = pOKcRMBeFBxNkwr95489214;     pOKcRMBeFBxNkwr95489214 = pOKcRMBeFBxNkwr98208887;     pOKcRMBeFBxNkwr98208887 = pOKcRMBeFBxNkwr16828104;     pOKcRMBeFBxNkwr16828104 = pOKcRMBeFBxNkwr92223106;     pOKcRMBeFBxNkwr92223106 = pOKcRMBeFBxNkwr68203171;     pOKcRMBeFBxNkwr68203171 = pOKcRMBeFBxNkwr83140209;     pOKcRMBeFBxNkwr83140209 = pOKcRMBeFBxNkwr29153770;     pOKcRMBeFBxNkwr29153770 = pOKcRMBeFBxNkwr67050978;     pOKcRMBeFBxNkwr67050978 = pOKcRMBeFBxNkwr38412261;     pOKcRMBeFBxNkwr38412261 = pOKcRMBeFBxNkwr58774369;     pOKcRMBeFBxNkwr58774369 = pOKcRMBeFBxNkwr44088819;     pOKcRMBeFBxNkwr44088819 = pOKcRMBeFBxNkwr84324869;     pOKcRMBeFBxNkwr84324869 = pOKcRMBeFBxNkwr58412675;     pOKcRMBeFBxNkwr58412675 = pOKcRMBeFBxNkwr32209115;     pOKcRMBeFBxNkwr32209115 = pOKcRMBeFBxNkwr30966788;     pOKcRMBeFBxNkwr30966788 = pOKcRMBeFBxNkwr57728180;     pOKcRMBeFBxNkwr57728180 = pOKcRMBeFBxNkwr85748839;     pOKcRMBeFBxNkwr85748839 = pOKcRMBeFBxNkwr32130694;     pOKcRMBeFBxNkwr32130694 = pOKcRMBeFBxNkwr71620432;     pOKcRMBeFBxNkwr71620432 = pOKcRMBeFBxNkwr3016973;     pOKcRMBeFBxNkwr3016973 = pOKcRMBeFBxNkwr75821928;     pOKcRMBeFBxNkwr75821928 = pOKcRMBeFBxNkwr92555824;     pOKcRMBeFBxNkwr92555824 = pOKcRMBeFBxNkwr80307144;     pOKcRMBeFBxNkwr80307144 = pOKcRMBeFBxNkwr31056413;     pOKcRMBeFBxNkwr31056413 = pOKcRMBeFBxNkwr2567889;     pOKcRMBeFBxNkwr2567889 = pOKcRMBeFBxNkwr41878716;     pOKcRMBeFBxNkwr41878716 = pOKcRMBeFBxNkwr81073754;     pOKcRMBeFBxNkwr81073754 = pOKcRMBeFBxNkwr17778815;     pOKcRMBeFBxNkwr17778815 = pOKcRMBeFBxNkwr91016676;     pOKcRMBeFBxNkwr91016676 = pOKcRMBeFBxNkwr28262473;     pOKcRMBeFBxNkwr28262473 = pOKcRMBeFBxNkwr34387916;     pOKcRMBeFBxNkwr34387916 = pOKcRMBeFBxNkwr8308685;     pOKcRMBeFBxNkwr8308685 = pOKcRMBeFBxNkwr42887338;     pOKcRMBeFBxNkwr42887338 = pOKcRMBeFBxNkwr69760759;     pOKcRMBeFBxNkwr69760759 = pOKcRMBeFBxNkwr9789568;     pOKcRMBeFBxNkwr9789568 = pOKcRMBeFBxNkwr19674347;     pOKcRMBeFBxNkwr19674347 = pOKcRMBeFBxNkwr44452652;     pOKcRMBeFBxNkwr44452652 = pOKcRMBeFBxNkwr32681899;     pOKcRMBeFBxNkwr32681899 = pOKcRMBeFBxNkwr28865334;     pOKcRMBeFBxNkwr28865334 = pOKcRMBeFBxNkwr81316731;     pOKcRMBeFBxNkwr81316731 = pOKcRMBeFBxNkwr92616334;     pOKcRMBeFBxNkwr92616334 = pOKcRMBeFBxNkwr71071447;     pOKcRMBeFBxNkwr71071447 = pOKcRMBeFBxNkwr75408426;     pOKcRMBeFBxNkwr75408426 = pOKcRMBeFBxNkwr42833678;     pOKcRMBeFBxNkwr42833678 = pOKcRMBeFBxNkwr82224035;     pOKcRMBeFBxNkwr82224035 = pOKcRMBeFBxNkwr41733457;     pOKcRMBeFBxNkwr41733457 = pOKcRMBeFBxNkwr60279695;     pOKcRMBeFBxNkwr60279695 = pOKcRMBeFBxNkwr61866279;     pOKcRMBeFBxNkwr61866279 = pOKcRMBeFBxNkwr79048711;     pOKcRMBeFBxNkwr79048711 = pOKcRMBeFBxNkwr58826404;     pOKcRMBeFBxNkwr58826404 = pOKcRMBeFBxNkwr28699701;     pOKcRMBeFBxNkwr28699701 = pOKcRMBeFBxNkwr77303535;     pOKcRMBeFBxNkwr77303535 = pOKcRMBeFBxNkwr62016240;     pOKcRMBeFBxNkwr62016240 = pOKcRMBeFBxNkwr75291605;     pOKcRMBeFBxNkwr75291605 = pOKcRMBeFBxNkwr27324331;     pOKcRMBeFBxNkwr27324331 = pOKcRMBeFBxNkwr2933391;     pOKcRMBeFBxNkwr2933391 = pOKcRMBeFBxNkwr17901744;     pOKcRMBeFBxNkwr17901744 = pOKcRMBeFBxNkwr85771691;     pOKcRMBeFBxNkwr85771691 = pOKcRMBeFBxNkwr89655217;     pOKcRMBeFBxNkwr89655217 = pOKcRMBeFBxNkwr26324456;     pOKcRMBeFBxNkwr26324456 = pOKcRMBeFBxNkwr2066455;     pOKcRMBeFBxNkwr2066455 = pOKcRMBeFBxNkwr11374956;     pOKcRMBeFBxNkwr11374956 = pOKcRMBeFBxNkwr76034302;     pOKcRMBeFBxNkwr76034302 = pOKcRMBeFBxNkwr10149789;     pOKcRMBeFBxNkwr10149789 = pOKcRMBeFBxNkwr24386453;     pOKcRMBeFBxNkwr24386453 = pOKcRMBeFBxNkwr35780134;     pOKcRMBeFBxNkwr35780134 = pOKcRMBeFBxNkwr41437532;     pOKcRMBeFBxNkwr41437532 = pOKcRMBeFBxNkwr81915192;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void jjehQxelpOYiSUYFpVlpmRUyDJfMU8565850() {     int heXOixNPIipbkHL78428112 = -23629614;    int heXOixNPIipbkHL72452092 = -706073281;    int heXOixNPIipbkHL84308525 = -623885918;    int heXOixNPIipbkHL73119934 = -222782972;    int heXOixNPIipbkHL43801538 = 28149349;    int heXOixNPIipbkHL65041185 = -994608615;    int heXOixNPIipbkHL81385913 = -171577266;    int heXOixNPIipbkHL78163182 = -998477165;    int heXOixNPIipbkHL48076401 = -100162457;    int heXOixNPIipbkHL95949894 = -470103834;    int heXOixNPIipbkHL81412031 = -563027108;    int heXOixNPIipbkHL87923455 = 68423236;    int heXOixNPIipbkHL10549480 = -616606915;    int heXOixNPIipbkHL84958347 = -611932315;    int heXOixNPIipbkHL66180513 = -640044476;    int heXOixNPIipbkHL37861829 = -332590079;    int heXOixNPIipbkHL24759354 = -558754050;    int heXOixNPIipbkHL592430 = -522335574;    int heXOixNPIipbkHL3693081 = -861199112;    int heXOixNPIipbkHL78847685 = -400307946;    int heXOixNPIipbkHL21499543 = -466996541;    int heXOixNPIipbkHL42509805 = -911216981;    int heXOixNPIipbkHL15809817 = -105501768;    int heXOixNPIipbkHL49883097 = -939618765;    int heXOixNPIipbkHL96967377 = -979935203;    int heXOixNPIipbkHL70099758 = -706001769;    int heXOixNPIipbkHL57912183 = -799232194;    int heXOixNPIipbkHL8804947 = -884003050;    int heXOixNPIipbkHL96644511 = 28277189;    int heXOixNPIipbkHL9688737 = -419368252;    int heXOixNPIipbkHL42730753 = -584123145;    int heXOixNPIipbkHL14058762 = -29086544;    int heXOixNPIipbkHL20590699 = -379213848;    int heXOixNPIipbkHL8148884 = -817254828;    int heXOixNPIipbkHL7693727 = -426759951;    int heXOixNPIipbkHL60499358 = -843162665;    int heXOixNPIipbkHL72780073 = -865104983;    int heXOixNPIipbkHL41330146 = -320072346;    int heXOixNPIipbkHL31874516 = -509108391;    int heXOixNPIipbkHL8671169 = -721927960;    int heXOixNPIipbkHL99321973 = -742704234;    int heXOixNPIipbkHL82605613 = -793640323;    int heXOixNPIipbkHL46316305 = -147960231;    int heXOixNPIipbkHL56296927 = -197987835;    int heXOixNPIipbkHL50311585 = -647443293;    int heXOixNPIipbkHL56093702 = -925995329;    int heXOixNPIipbkHL29121208 = -961492003;    int heXOixNPIipbkHL11456811 = -671114206;    int heXOixNPIipbkHL76337248 = -720055430;    int heXOixNPIipbkHL58549878 = -550446084;    int heXOixNPIipbkHL83919333 = -47094875;    int heXOixNPIipbkHL44029071 = -365708064;    int heXOixNPIipbkHL95160351 = 48414151;    int heXOixNPIipbkHL61329124 = -465836415;    int heXOixNPIipbkHL87066307 = -620462000;    int heXOixNPIipbkHL35918308 = -112412633;    int heXOixNPIipbkHL56642275 = -500571513;    int heXOixNPIipbkHL34425429 = -684267154;    int heXOixNPIipbkHL76152556 = -242847769;    int heXOixNPIipbkHL73701780 = -265848883;    int heXOixNPIipbkHL7129002 = -95376422;    int heXOixNPIipbkHL72580966 = -287574217;    int heXOixNPIipbkHL81518671 = -926754355;    int heXOixNPIipbkHL38387665 = -680794206;    int heXOixNPIipbkHL53219141 = -885980690;    int heXOixNPIipbkHL67353270 = -433940564;    int heXOixNPIipbkHL67332757 = -552362917;    int heXOixNPIipbkHL2400597 = -799352087;    int heXOixNPIipbkHL77264621 = -85172365;    int heXOixNPIipbkHL5681156 = -796881812;    int heXOixNPIipbkHL65081756 = -467485097;    int heXOixNPIipbkHL83429208 = -138681704;    int heXOixNPIipbkHL68717913 = 86772816;    int heXOixNPIipbkHL95021911 = -39271152;    int heXOixNPIipbkHL79525711 = -657603712;    int heXOixNPIipbkHL38893929 = -673356219;    int heXOixNPIipbkHL96193499 = -663256751;    int heXOixNPIipbkHL59512890 = -907513934;    int heXOixNPIipbkHL99571512 = -192175473;    int heXOixNPIipbkHL40873676 = 46060126;    int heXOixNPIipbkHL40978551 = -744509766;    int heXOixNPIipbkHL46455373 = -28117988;    int heXOixNPIipbkHL32467699 = -63947621;    int heXOixNPIipbkHL38094633 = -421276728;    int heXOixNPIipbkHL25769403 = -272273378;    int heXOixNPIipbkHL98701681 = -118415082;    int heXOixNPIipbkHL18898411 = 22499305;    int heXOixNPIipbkHL59261574 = -913377433;    int heXOixNPIipbkHL21082576 = -96792829;    int heXOixNPIipbkHL71775418 = -214347319;    int heXOixNPIipbkHL3857083 = -242591152;    int heXOixNPIipbkHL38354644 = -80837830;    int heXOixNPIipbkHL65177589 = 22775423;    int heXOixNPIipbkHL58172736 = -143259508;    int heXOixNPIipbkHL1542168 = -526551538;    int heXOixNPIipbkHL26741007 = -355130018;    int heXOixNPIipbkHL1086943 = -866885969;    int heXOixNPIipbkHL7928640 = -467166026;    int heXOixNPIipbkHL3077786 = -312007146;    int heXOixNPIipbkHL82958314 = -23629614;     heXOixNPIipbkHL78428112 = heXOixNPIipbkHL72452092;     heXOixNPIipbkHL72452092 = heXOixNPIipbkHL84308525;     heXOixNPIipbkHL84308525 = heXOixNPIipbkHL73119934;     heXOixNPIipbkHL73119934 = heXOixNPIipbkHL43801538;     heXOixNPIipbkHL43801538 = heXOixNPIipbkHL65041185;     heXOixNPIipbkHL65041185 = heXOixNPIipbkHL81385913;     heXOixNPIipbkHL81385913 = heXOixNPIipbkHL78163182;     heXOixNPIipbkHL78163182 = heXOixNPIipbkHL48076401;     heXOixNPIipbkHL48076401 = heXOixNPIipbkHL95949894;     heXOixNPIipbkHL95949894 = heXOixNPIipbkHL81412031;     heXOixNPIipbkHL81412031 = heXOixNPIipbkHL87923455;     heXOixNPIipbkHL87923455 = heXOixNPIipbkHL10549480;     heXOixNPIipbkHL10549480 = heXOixNPIipbkHL84958347;     heXOixNPIipbkHL84958347 = heXOixNPIipbkHL66180513;     heXOixNPIipbkHL66180513 = heXOixNPIipbkHL37861829;     heXOixNPIipbkHL37861829 = heXOixNPIipbkHL24759354;     heXOixNPIipbkHL24759354 = heXOixNPIipbkHL592430;     heXOixNPIipbkHL592430 = heXOixNPIipbkHL3693081;     heXOixNPIipbkHL3693081 = heXOixNPIipbkHL78847685;     heXOixNPIipbkHL78847685 = heXOixNPIipbkHL21499543;     heXOixNPIipbkHL21499543 = heXOixNPIipbkHL42509805;     heXOixNPIipbkHL42509805 = heXOixNPIipbkHL15809817;     heXOixNPIipbkHL15809817 = heXOixNPIipbkHL49883097;     heXOixNPIipbkHL49883097 = heXOixNPIipbkHL96967377;     heXOixNPIipbkHL96967377 = heXOixNPIipbkHL70099758;     heXOixNPIipbkHL70099758 = heXOixNPIipbkHL57912183;     heXOixNPIipbkHL57912183 = heXOixNPIipbkHL8804947;     heXOixNPIipbkHL8804947 = heXOixNPIipbkHL96644511;     heXOixNPIipbkHL96644511 = heXOixNPIipbkHL9688737;     heXOixNPIipbkHL9688737 = heXOixNPIipbkHL42730753;     heXOixNPIipbkHL42730753 = heXOixNPIipbkHL14058762;     heXOixNPIipbkHL14058762 = heXOixNPIipbkHL20590699;     heXOixNPIipbkHL20590699 = heXOixNPIipbkHL8148884;     heXOixNPIipbkHL8148884 = heXOixNPIipbkHL7693727;     heXOixNPIipbkHL7693727 = heXOixNPIipbkHL60499358;     heXOixNPIipbkHL60499358 = heXOixNPIipbkHL72780073;     heXOixNPIipbkHL72780073 = heXOixNPIipbkHL41330146;     heXOixNPIipbkHL41330146 = heXOixNPIipbkHL31874516;     heXOixNPIipbkHL31874516 = heXOixNPIipbkHL8671169;     heXOixNPIipbkHL8671169 = heXOixNPIipbkHL99321973;     heXOixNPIipbkHL99321973 = heXOixNPIipbkHL82605613;     heXOixNPIipbkHL82605613 = heXOixNPIipbkHL46316305;     heXOixNPIipbkHL46316305 = heXOixNPIipbkHL56296927;     heXOixNPIipbkHL56296927 = heXOixNPIipbkHL50311585;     heXOixNPIipbkHL50311585 = heXOixNPIipbkHL56093702;     heXOixNPIipbkHL56093702 = heXOixNPIipbkHL29121208;     heXOixNPIipbkHL29121208 = heXOixNPIipbkHL11456811;     heXOixNPIipbkHL11456811 = heXOixNPIipbkHL76337248;     heXOixNPIipbkHL76337248 = heXOixNPIipbkHL58549878;     heXOixNPIipbkHL58549878 = heXOixNPIipbkHL83919333;     heXOixNPIipbkHL83919333 = heXOixNPIipbkHL44029071;     heXOixNPIipbkHL44029071 = heXOixNPIipbkHL95160351;     heXOixNPIipbkHL95160351 = heXOixNPIipbkHL61329124;     heXOixNPIipbkHL61329124 = heXOixNPIipbkHL87066307;     heXOixNPIipbkHL87066307 = heXOixNPIipbkHL35918308;     heXOixNPIipbkHL35918308 = heXOixNPIipbkHL56642275;     heXOixNPIipbkHL56642275 = heXOixNPIipbkHL34425429;     heXOixNPIipbkHL34425429 = heXOixNPIipbkHL76152556;     heXOixNPIipbkHL76152556 = heXOixNPIipbkHL73701780;     heXOixNPIipbkHL73701780 = heXOixNPIipbkHL7129002;     heXOixNPIipbkHL7129002 = heXOixNPIipbkHL72580966;     heXOixNPIipbkHL72580966 = heXOixNPIipbkHL81518671;     heXOixNPIipbkHL81518671 = heXOixNPIipbkHL38387665;     heXOixNPIipbkHL38387665 = heXOixNPIipbkHL53219141;     heXOixNPIipbkHL53219141 = heXOixNPIipbkHL67353270;     heXOixNPIipbkHL67353270 = heXOixNPIipbkHL67332757;     heXOixNPIipbkHL67332757 = heXOixNPIipbkHL2400597;     heXOixNPIipbkHL2400597 = heXOixNPIipbkHL77264621;     heXOixNPIipbkHL77264621 = heXOixNPIipbkHL5681156;     heXOixNPIipbkHL5681156 = heXOixNPIipbkHL65081756;     heXOixNPIipbkHL65081756 = heXOixNPIipbkHL83429208;     heXOixNPIipbkHL83429208 = heXOixNPIipbkHL68717913;     heXOixNPIipbkHL68717913 = heXOixNPIipbkHL95021911;     heXOixNPIipbkHL95021911 = heXOixNPIipbkHL79525711;     heXOixNPIipbkHL79525711 = heXOixNPIipbkHL38893929;     heXOixNPIipbkHL38893929 = heXOixNPIipbkHL96193499;     heXOixNPIipbkHL96193499 = heXOixNPIipbkHL59512890;     heXOixNPIipbkHL59512890 = heXOixNPIipbkHL99571512;     heXOixNPIipbkHL99571512 = heXOixNPIipbkHL40873676;     heXOixNPIipbkHL40873676 = heXOixNPIipbkHL40978551;     heXOixNPIipbkHL40978551 = heXOixNPIipbkHL46455373;     heXOixNPIipbkHL46455373 = heXOixNPIipbkHL32467699;     heXOixNPIipbkHL32467699 = heXOixNPIipbkHL38094633;     heXOixNPIipbkHL38094633 = heXOixNPIipbkHL25769403;     heXOixNPIipbkHL25769403 = heXOixNPIipbkHL98701681;     heXOixNPIipbkHL98701681 = heXOixNPIipbkHL18898411;     heXOixNPIipbkHL18898411 = heXOixNPIipbkHL59261574;     heXOixNPIipbkHL59261574 = heXOixNPIipbkHL21082576;     heXOixNPIipbkHL21082576 = heXOixNPIipbkHL71775418;     heXOixNPIipbkHL71775418 = heXOixNPIipbkHL3857083;     heXOixNPIipbkHL3857083 = heXOixNPIipbkHL38354644;     heXOixNPIipbkHL38354644 = heXOixNPIipbkHL65177589;     heXOixNPIipbkHL65177589 = heXOixNPIipbkHL58172736;     heXOixNPIipbkHL58172736 = heXOixNPIipbkHL1542168;     heXOixNPIipbkHL1542168 = heXOixNPIipbkHL26741007;     heXOixNPIipbkHL26741007 = heXOixNPIipbkHL1086943;     heXOixNPIipbkHL1086943 = heXOixNPIipbkHL7928640;     heXOixNPIipbkHL7928640 = heXOixNPIipbkHL3077786;     heXOixNPIipbkHL3077786 = heXOixNPIipbkHL82958314;     heXOixNPIipbkHL82958314 = heXOixNPIipbkHL78428112;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xRbnfGNUdBYdtwyEYQQSnpaUnlErQ10755189() {     int KgOYKmBeUpuBOkl74941033 = -22430302;    int KgOYKmBeUpuBOkl87534917 = -658761583;    int KgOYKmBeUpuBOkl39990837 = -371988684;    int KgOYKmBeUpuBOkl85668782 = -691596762;    int KgOYKmBeUpuBOkl12453928 = -627437647;    int KgOYKmBeUpuBOkl2288058 = 74863808;    int KgOYKmBeUpuBOkl55200567 = -336495241;    int KgOYKmBeUpuBOkl13615353 = -11361350;    int KgOYKmBeUpuBOkl52330658 = -352580211;    int KgOYKmBeUpuBOkl49954430 = -128829047;    int KgOYKmBeUpuBOkl41628147 = -538262345;    int KgOYKmBeUpuBOkl2939893 = -500309670;    int KgOYKmBeUpuBOkl15820178 = -902392578;    int KgOYKmBeUpuBOkl52033461 = -21453850;    int KgOYKmBeUpuBOkl71080270 = -491371186;    int KgOYKmBeUpuBOkl50818652 = -91211295;    int KgOYKmBeUpuBOkl52450202 = -471601094;    int KgOYKmBeUpuBOkl36727917 = -39861639;    int KgOYKmBeUpuBOkl85616056 = -250326412;    int KgOYKmBeUpuBOkl19572945 = -309832777;    int KgOYKmBeUpuBOkl29178398 = -677546139;    int KgOYKmBeUpuBOkl83411562 = -189234674;    int KgOYKmBeUpuBOkl5306780 = -386421479;    int KgOYKmBeUpuBOkl73707867 = -898736169;    int KgOYKmBeUpuBOkl75242385 = -505360962;    int KgOYKmBeUpuBOkl46124123 = -885297031;    int KgOYKmBeUpuBOkl5808869 = -626414041;    int KgOYKmBeUpuBOkl1055310 = -539759149;    int KgOYKmBeUpuBOkl78840482 = -937100794;    int KgOYKmBeUpuBOkl9943244 = -884145094;    int KgOYKmBeUpuBOkl51824832 = -886091298;    int KgOYKmBeUpuBOkl49808945 = -361092564;    int KgOYKmBeUpuBOkl38035137 = -269310616;    int KgOYKmBeUpuBOkl20808552 = -637764828;    int KgOYKmBeUpuBOkl17178565 = -641425477;    int KgOYKmBeUpuBOkl4170612 = -20239373;    int KgOYKmBeUpuBOkl53337040 = -375528941;    int KgOYKmBeUpuBOkl14457120 = -141204315;    int KgOYKmBeUpuBOkl80608822 = -850177141;    int KgOYKmBeUpuBOkl88188568 = -158275842;    int KgOYKmBeUpuBOkl31592969 = -174246405;    int KgOYKmBeUpuBOkl26798966 = -201081285;    int KgOYKmBeUpuBOkl33858241 = -644232803;    int KgOYKmBeUpuBOkl68505035 = -339621589;    int KgOYKmBeUpuBOkl16298300 = -79597631;    int KgOYKmBeUpuBOkl53774728 = -158323125;    int KgOYKmBeUpuBOkl26033301 = -140012651;    int KgOYKmBeUpuBOkl91946834 = -834208205;    int KgOYKmBeUpuBOkl94946316 = -251559625;    int KgOYKmBeUpuBOkl31350918 = -899985539;    int KgOYKmBeUpuBOkl35707974 = -676474081;    int KgOYKmBeUpuBOkl16437709 = -947873314;    int KgOYKmBeUpuBOkl87303729 = -743494494;    int KgOYKmBeUpuBOkl46836320 = -916037419;    int KgOYKmBeUpuBOkl81576791 = 46816208;    int KgOYKmBeUpuBOkl91529471 = -833195628;    int KgOYKmBeUpuBOkl82228138 = -172340105;    int KgOYKmBeUpuBOkl66282969 = -473252515;    int KgOYKmBeUpuBOkl10426398 = -86235801;    int KgOYKmBeUpuBOkl66329805 = -742140616;    int KgOYKmBeUpuBOkl96479189 = -298722152;    int KgOYKmBeUpuBOkl54145257 = -796736093;    int KgOYKmBeUpuBOkl34774871 = -74260556;    int KgOYKmBeUpuBOkl42387414 = -468435117;    int KgOYKmBeUpuBOkl98129598 = -242737749;    int KgOYKmBeUpuBOkl91819202 = -77169781;    int KgOYKmBeUpuBOkl64904756 = -130999054;    int KgOYKmBeUpuBOkl95011626 = -164627751;    int KgOYKmBeUpuBOkl34854896 = -380028374;    int KgOYKmBeUpuBOkl66909658 = -371131814;    int KgOYKmBeUpuBOkl97481612 = -715682354;    int KgOYKmBeUpuBOkl37993082 = -230396780;    int KgOYKmBeUpuBOkl56119095 = -189684499;    int KgOYKmBeUpuBOkl97427488 = 7949429;    int KgOYKmBeUpuBOkl87979975 = -35586372;    int KgOYKmBeUpuBOkl2379432 = -376464854;    int KgOYKmBeUpuBOkl49553321 = -545001872;    int KgOYKmBeUpuBOkl36801745 = 53200109;    int KgOYKmBeUpuBOkl57409567 = -719138539;    int KgOYKmBeUpuBOkl21467657 = -247037838;    int KgOYKmBeUpuBOkl20090823 = -645284381;    int KgOYKmBeUpuBOkl13862035 = -792205836;    int KgOYKmBeUpuBOkl6108994 = -188199525;    int KgOYKmBeUpuBOkl47489565 = 62884744;    int KgOYKmBeUpuBOkl74235269 = -107671014;    int KgOYKmBeUpuBOkl35387123 = -938217985;    int KgOYKmBeUpuBOkl62505215 = -617598071;    int KgOYKmBeUpuBOkl91198817 = -353273197;    int KgOYKmBeUpuBOkl39231761 = -584581036;    int KgOYKmBeUpuBOkl25649093 = -808229849;    int KgOYKmBeUpuBOkl21942474 = -847899269;    int KgOYKmBeUpuBOkl87054070 = -902276426;    int KgOYKmBeUpuBOkl4030723 = 45031485;    int KgOYKmBeUpuBOkl14279018 = -8036525;    int KgOYKmBeUpuBOkl91709378 = -859553691;    int KgOYKmBeUpuBOkl77447711 = -377510313;    int KgOYKmBeUpuBOkl92024095 = -26820730;    int KgOYKmBeUpuBOkl91470826 = -75797686;    int KgOYKmBeUpuBOkl70375437 = 3116160;    int KgOYKmBeUpuBOkl24479097 = -22430302;     KgOYKmBeUpuBOkl74941033 = KgOYKmBeUpuBOkl87534917;     KgOYKmBeUpuBOkl87534917 = KgOYKmBeUpuBOkl39990837;     KgOYKmBeUpuBOkl39990837 = KgOYKmBeUpuBOkl85668782;     KgOYKmBeUpuBOkl85668782 = KgOYKmBeUpuBOkl12453928;     KgOYKmBeUpuBOkl12453928 = KgOYKmBeUpuBOkl2288058;     KgOYKmBeUpuBOkl2288058 = KgOYKmBeUpuBOkl55200567;     KgOYKmBeUpuBOkl55200567 = KgOYKmBeUpuBOkl13615353;     KgOYKmBeUpuBOkl13615353 = KgOYKmBeUpuBOkl52330658;     KgOYKmBeUpuBOkl52330658 = KgOYKmBeUpuBOkl49954430;     KgOYKmBeUpuBOkl49954430 = KgOYKmBeUpuBOkl41628147;     KgOYKmBeUpuBOkl41628147 = KgOYKmBeUpuBOkl2939893;     KgOYKmBeUpuBOkl2939893 = KgOYKmBeUpuBOkl15820178;     KgOYKmBeUpuBOkl15820178 = KgOYKmBeUpuBOkl52033461;     KgOYKmBeUpuBOkl52033461 = KgOYKmBeUpuBOkl71080270;     KgOYKmBeUpuBOkl71080270 = KgOYKmBeUpuBOkl50818652;     KgOYKmBeUpuBOkl50818652 = KgOYKmBeUpuBOkl52450202;     KgOYKmBeUpuBOkl52450202 = KgOYKmBeUpuBOkl36727917;     KgOYKmBeUpuBOkl36727917 = KgOYKmBeUpuBOkl85616056;     KgOYKmBeUpuBOkl85616056 = KgOYKmBeUpuBOkl19572945;     KgOYKmBeUpuBOkl19572945 = KgOYKmBeUpuBOkl29178398;     KgOYKmBeUpuBOkl29178398 = KgOYKmBeUpuBOkl83411562;     KgOYKmBeUpuBOkl83411562 = KgOYKmBeUpuBOkl5306780;     KgOYKmBeUpuBOkl5306780 = KgOYKmBeUpuBOkl73707867;     KgOYKmBeUpuBOkl73707867 = KgOYKmBeUpuBOkl75242385;     KgOYKmBeUpuBOkl75242385 = KgOYKmBeUpuBOkl46124123;     KgOYKmBeUpuBOkl46124123 = KgOYKmBeUpuBOkl5808869;     KgOYKmBeUpuBOkl5808869 = KgOYKmBeUpuBOkl1055310;     KgOYKmBeUpuBOkl1055310 = KgOYKmBeUpuBOkl78840482;     KgOYKmBeUpuBOkl78840482 = KgOYKmBeUpuBOkl9943244;     KgOYKmBeUpuBOkl9943244 = KgOYKmBeUpuBOkl51824832;     KgOYKmBeUpuBOkl51824832 = KgOYKmBeUpuBOkl49808945;     KgOYKmBeUpuBOkl49808945 = KgOYKmBeUpuBOkl38035137;     KgOYKmBeUpuBOkl38035137 = KgOYKmBeUpuBOkl20808552;     KgOYKmBeUpuBOkl20808552 = KgOYKmBeUpuBOkl17178565;     KgOYKmBeUpuBOkl17178565 = KgOYKmBeUpuBOkl4170612;     KgOYKmBeUpuBOkl4170612 = KgOYKmBeUpuBOkl53337040;     KgOYKmBeUpuBOkl53337040 = KgOYKmBeUpuBOkl14457120;     KgOYKmBeUpuBOkl14457120 = KgOYKmBeUpuBOkl80608822;     KgOYKmBeUpuBOkl80608822 = KgOYKmBeUpuBOkl88188568;     KgOYKmBeUpuBOkl88188568 = KgOYKmBeUpuBOkl31592969;     KgOYKmBeUpuBOkl31592969 = KgOYKmBeUpuBOkl26798966;     KgOYKmBeUpuBOkl26798966 = KgOYKmBeUpuBOkl33858241;     KgOYKmBeUpuBOkl33858241 = KgOYKmBeUpuBOkl68505035;     KgOYKmBeUpuBOkl68505035 = KgOYKmBeUpuBOkl16298300;     KgOYKmBeUpuBOkl16298300 = KgOYKmBeUpuBOkl53774728;     KgOYKmBeUpuBOkl53774728 = KgOYKmBeUpuBOkl26033301;     KgOYKmBeUpuBOkl26033301 = KgOYKmBeUpuBOkl91946834;     KgOYKmBeUpuBOkl91946834 = KgOYKmBeUpuBOkl94946316;     KgOYKmBeUpuBOkl94946316 = KgOYKmBeUpuBOkl31350918;     KgOYKmBeUpuBOkl31350918 = KgOYKmBeUpuBOkl35707974;     KgOYKmBeUpuBOkl35707974 = KgOYKmBeUpuBOkl16437709;     KgOYKmBeUpuBOkl16437709 = KgOYKmBeUpuBOkl87303729;     KgOYKmBeUpuBOkl87303729 = KgOYKmBeUpuBOkl46836320;     KgOYKmBeUpuBOkl46836320 = KgOYKmBeUpuBOkl81576791;     KgOYKmBeUpuBOkl81576791 = KgOYKmBeUpuBOkl91529471;     KgOYKmBeUpuBOkl91529471 = KgOYKmBeUpuBOkl82228138;     KgOYKmBeUpuBOkl82228138 = KgOYKmBeUpuBOkl66282969;     KgOYKmBeUpuBOkl66282969 = KgOYKmBeUpuBOkl10426398;     KgOYKmBeUpuBOkl10426398 = KgOYKmBeUpuBOkl66329805;     KgOYKmBeUpuBOkl66329805 = KgOYKmBeUpuBOkl96479189;     KgOYKmBeUpuBOkl96479189 = KgOYKmBeUpuBOkl54145257;     KgOYKmBeUpuBOkl54145257 = KgOYKmBeUpuBOkl34774871;     KgOYKmBeUpuBOkl34774871 = KgOYKmBeUpuBOkl42387414;     KgOYKmBeUpuBOkl42387414 = KgOYKmBeUpuBOkl98129598;     KgOYKmBeUpuBOkl98129598 = KgOYKmBeUpuBOkl91819202;     KgOYKmBeUpuBOkl91819202 = KgOYKmBeUpuBOkl64904756;     KgOYKmBeUpuBOkl64904756 = KgOYKmBeUpuBOkl95011626;     KgOYKmBeUpuBOkl95011626 = KgOYKmBeUpuBOkl34854896;     KgOYKmBeUpuBOkl34854896 = KgOYKmBeUpuBOkl66909658;     KgOYKmBeUpuBOkl66909658 = KgOYKmBeUpuBOkl97481612;     KgOYKmBeUpuBOkl97481612 = KgOYKmBeUpuBOkl37993082;     KgOYKmBeUpuBOkl37993082 = KgOYKmBeUpuBOkl56119095;     KgOYKmBeUpuBOkl56119095 = KgOYKmBeUpuBOkl97427488;     KgOYKmBeUpuBOkl97427488 = KgOYKmBeUpuBOkl87979975;     KgOYKmBeUpuBOkl87979975 = KgOYKmBeUpuBOkl2379432;     KgOYKmBeUpuBOkl2379432 = KgOYKmBeUpuBOkl49553321;     KgOYKmBeUpuBOkl49553321 = KgOYKmBeUpuBOkl36801745;     KgOYKmBeUpuBOkl36801745 = KgOYKmBeUpuBOkl57409567;     KgOYKmBeUpuBOkl57409567 = KgOYKmBeUpuBOkl21467657;     KgOYKmBeUpuBOkl21467657 = KgOYKmBeUpuBOkl20090823;     KgOYKmBeUpuBOkl20090823 = KgOYKmBeUpuBOkl13862035;     KgOYKmBeUpuBOkl13862035 = KgOYKmBeUpuBOkl6108994;     KgOYKmBeUpuBOkl6108994 = KgOYKmBeUpuBOkl47489565;     KgOYKmBeUpuBOkl47489565 = KgOYKmBeUpuBOkl74235269;     KgOYKmBeUpuBOkl74235269 = KgOYKmBeUpuBOkl35387123;     KgOYKmBeUpuBOkl35387123 = KgOYKmBeUpuBOkl62505215;     KgOYKmBeUpuBOkl62505215 = KgOYKmBeUpuBOkl91198817;     KgOYKmBeUpuBOkl91198817 = KgOYKmBeUpuBOkl39231761;     KgOYKmBeUpuBOkl39231761 = KgOYKmBeUpuBOkl25649093;     KgOYKmBeUpuBOkl25649093 = KgOYKmBeUpuBOkl21942474;     KgOYKmBeUpuBOkl21942474 = KgOYKmBeUpuBOkl87054070;     KgOYKmBeUpuBOkl87054070 = KgOYKmBeUpuBOkl4030723;     KgOYKmBeUpuBOkl4030723 = KgOYKmBeUpuBOkl14279018;     KgOYKmBeUpuBOkl14279018 = KgOYKmBeUpuBOkl91709378;     KgOYKmBeUpuBOkl91709378 = KgOYKmBeUpuBOkl77447711;     KgOYKmBeUpuBOkl77447711 = KgOYKmBeUpuBOkl92024095;     KgOYKmBeUpuBOkl92024095 = KgOYKmBeUpuBOkl91470826;     KgOYKmBeUpuBOkl91470826 = KgOYKmBeUpuBOkl70375437;     KgOYKmBeUpuBOkl70375437 = KgOYKmBeUpuBOkl24479097;     KgOYKmBeUpuBOkl24479097 = KgOYKmBeUpuBOkl74941033;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vVVnzvXegOTwKDKyoVXngkgwpSXrs60701995() {     int cGFqLESjrtSwmBP42112093 = -766855979;    int cGFqLESjrtSwmBP32718944 = -440498059;    int cGFqLESjrtSwmBP74328854 = -605257295;    int cGFqLESjrtSwmBP53069057 = -430008112;    int cGFqLESjrtSwmBP93716537 = -598075669;    int cGFqLESjrtSwmBP94729259 = 42187892;    int cGFqLESjrtSwmBP78846780 = -795652459;    int cGFqLESjrtSwmBP36695853 = -766885306;    int cGFqLESjrtSwmBP19709805 = -984699224;    int cGFqLESjrtSwmBP30285914 = -451842739;    int cGFqLESjrtSwmBP23772818 = -588927439;    int cGFqLESjrtSwmBP49211224 = -549328006;    int cGFqLESjrtSwmBP69065348 = 23943174;    int cGFqLESjrtSwmBP49111387 = 13149456;    int cGFqLESjrtSwmBP13527938 = -529735048;    int cGFqLESjrtSwmBP21739569 = -187020446;    int cGFqLESjrtSwmBP18141119 = -474018699;    int cGFqLESjrtSwmBP70156367 = -249482279;    int cGFqLESjrtSwmBP28818545 = -828659388;    int cGFqLESjrtSwmBP47011150 = -149699461;    int cGFqLESjrtSwmBP12323583 = 13691496;    int cGFqLESjrtSwmBP7412819 = -388545789;    int cGFqLESjrtSwmBP59869154 = -776226492;    int cGFqLESjrtSwmBP87732066 = -569333957;    int cGFqLESjrtSwmBP89199091 = -379997556;    int cGFqLESjrtSwmBP36171991 = -154670811;    int cGFqLESjrtSwmBP91744757 = -547973950;    int cGFqLESjrtSwmBP65684373 = -91672027;    int cGFqLESjrtSwmBP80500494 = -450766679;    int cGFqLESjrtSwmBP73279977 = -31885634;    int cGFqLESjrtSwmBP34578230 = -549399903;    int cGFqLESjrtSwmBP45346731 = -629573986;    int cGFqLESjrtSwmBP52972513 = 95575223;    int cGFqLESjrtSwmBP98550357 = -145122779;    int cGFqLESjrtSwmBP22329602 = -443723857;    int cGFqLESjrtSwmBP65006934 = -296776883;    int cGFqLESjrtSwmBP65744124 = 25107272;    int cGFqLESjrtSwmBP70458903 = -851186712;    int cGFqLESjrtSwmBP72562821 = -434293112;    int cGFqLESjrtSwmBP90936372 = -602800401;    int cGFqLESjrtSwmBP36081580 = 98136444;    int cGFqLESjrtSwmBP13777648 = -284119235;    int cGFqLESjrtSwmBP10083777 = -568129173;    int cGFqLESjrtSwmBP27989545 = -996265870;    int cGFqLESjrtSwmBP94221021 = -447138961;    int cGFqLESjrtSwmBP17433622 = -436943405;    int cGFqLESjrtSwmBP21378804 = -53322476;    int cGFqLESjrtSwmBP62667486 = -840671143;    int cGFqLESjrtSwmBP338448 = -205063928;    int cGFqLESjrtSwmBP92534020 = -845508986;    int cGFqLESjrtSwmBP42426569 = -173870401;    int cGFqLESjrtSwmBP16170595 = -17112538;    int cGFqLESjrtSwmBP59074417 = -471147172;    int cGFqLESjrtSwmBP12320550 = -395200069;    int cGFqLESjrtSwmBP48378413 = -739947222;    int cGFqLESjrtSwmBP34699274 = -278310190;    int cGFqLESjrtSwmBP72849790 = -664271568;    int cGFqLESjrtSwmBP86596788 = 64076661;    int cGFqLESjrtSwmBP63869965 = 49989444;    int cGFqLESjrtSwmBP57544546 = -343404859;    int cGFqLESjrtSwmBP2984503 = -409838159;    int cGFqLESjrtSwmBP13162407 = -603980432;    int cGFqLESjrtSwmBP56195358 = -216118628;    int cGFqLESjrtSwmBP46429828 = -852813590;    int cGFqLESjrtSwmBP95707684 = -902442837;    int cGFqLESjrtSwmBP78426087 = -959353454;    int cGFqLESjrtSwmBP96238710 = -544903230;    int cGFqLESjrtSwmBP70514991 = -830934047;    int cGFqLESjrtSwmBP26781786 = -543126687;    int cGFqLESjrtSwmBP48521003 = -132958165;    int cGFqLESjrtSwmBP55995444 = -112127719;    int cGFqLESjrtSwmBP47682215 = -622831988;    int cGFqLESjrtSwmBP97593545 = -815189167;    int cGFqLESjrtSwmBP37882172 = -125858988;    int cGFqLESjrtSwmBP10929570 = -147835906;    int cGFqLESjrtSwmBP98545934 = -702189269;    int cGFqLESjrtSwmBP97329041 = -820416617;    int cGFqLESjrtSwmBP31879609 = -779960622;    int cGFqLESjrtSwmBP93511045 = -22194996;    int cGFqLESjrtSwmBP71765469 = -943054152;    int cGFqLESjrtSwmBP14793188 = -1348335;    int cGFqLESjrtSwmBP29077271 = -707302807;    int cGFqLESjrtSwmBP65345925 = -886608100;    int cGFqLESjrtSwmBP87966474 = -605257693;    int cGFqLESjrtSwmBP30853409 = -858015233;    int cGFqLESjrtSwmBP18407635 = -432287365;    int cGFqLESjrtSwmBP86272314 = -58426814;    int cGFqLESjrtSwmBP40651964 = -509224708;    int cGFqLESjrtSwmBP50171945 = -405175558;    int cGFqLESjrtSwmBP87630327 = -65413667;    int cGFqLESjrtSwmBP92157144 = -632505316;    int cGFqLESjrtSwmBP79147336 = 61030611;    int cGFqLESjrtSwmBP6588938 = -801176156;    int cGFqLESjrtSwmBP15018275 = 9111746;    int cGFqLESjrtSwmBP87951870 = -92962243;    int cGFqLESjrtSwmBP22919173 = -297883124;    int cGFqLESjrtSwmBP57582289 = 31999392;    int cGFqLESjrtSwmBP63653949 = -715315583;    int cGFqLESjrtSwmBP32281861 = 6176966;    int cGFqLESjrtSwmBP15794934 = -766855979;     cGFqLESjrtSwmBP42112093 = cGFqLESjrtSwmBP32718944;     cGFqLESjrtSwmBP32718944 = cGFqLESjrtSwmBP74328854;     cGFqLESjrtSwmBP74328854 = cGFqLESjrtSwmBP53069057;     cGFqLESjrtSwmBP53069057 = cGFqLESjrtSwmBP93716537;     cGFqLESjrtSwmBP93716537 = cGFqLESjrtSwmBP94729259;     cGFqLESjrtSwmBP94729259 = cGFqLESjrtSwmBP78846780;     cGFqLESjrtSwmBP78846780 = cGFqLESjrtSwmBP36695853;     cGFqLESjrtSwmBP36695853 = cGFqLESjrtSwmBP19709805;     cGFqLESjrtSwmBP19709805 = cGFqLESjrtSwmBP30285914;     cGFqLESjrtSwmBP30285914 = cGFqLESjrtSwmBP23772818;     cGFqLESjrtSwmBP23772818 = cGFqLESjrtSwmBP49211224;     cGFqLESjrtSwmBP49211224 = cGFqLESjrtSwmBP69065348;     cGFqLESjrtSwmBP69065348 = cGFqLESjrtSwmBP49111387;     cGFqLESjrtSwmBP49111387 = cGFqLESjrtSwmBP13527938;     cGFqLESjrtSwmBP13527938 = cGFqLESjrtSwmBP21739569;     cGFqLESjrtSwmBP21739569 = cGFqLESjrtSwmBP18141119;     cGFqLESjrtSwmBP18141119 = cGFqLESjrtSwmBP70156367;     cGFqLESjrtSwmBP70156367 = cGFqLESjrtSwmBP28818545;     cGFqLESjrtSwmBP28818545 = cGFqLESjrtSwmBP47011150;     cGFqLESjrtSwmBP47011150 = cGFqLESjrtSwmBP12323583;     cGFqLESjrtSwmBP12323583 = cGFqLESjrtSwmBP7412819;     cGFqLESjrtSwmBP7412819 = cGFqLESjrtSwmBP59869154;     cGFqLESjrtSwmBP59869154 = cGFqLESjrtSwmBP87732066;     cGFqLESjrtSwmBP87732066 = cGFqLESjrtSwmBP89199091;     cGFqLESjrtSwmBP89199091 = cGFqLESjrtSwmBP36171991;     cGFqLESjrtSwmBP36171991 = cGFqLESjrtSwmBP91744757;     cGFqLESjrtSwmBP91744757 = cGFqLESjrtSwmBP65684373;     cGFqLESjrtSwmBP65684373 = cGFqLESjrtSwmBP80500494;     cGFqLESjrtSwmBP80500494 = cGFqLESjrtSwmBP73279977;     cGFqLESjrtSwmBP73279977 = cGFqLESjrtSwmBP34578230;     cGFqLESjrtSwmBP34578230 = cGFqLESjrtSwmBP45346731;     cGFqLESjrtSwmBP45346731 = cGFqLESjrtSwmBP52972513;     cGFqLESjrtSwmBP52972513 = cGFqLESjrtSwmBP98550357;     cGFqLESjrtSwmBP98550357 = cGFqLESjrtSwmBP22329602;     cGFqLESjrtSwmBP22329602 = cGFqLESjrtSwmBP65006934;     cGFqLESjrtSwmBP65006934 = cGFqLESjrtSwmBP65744124;     cGFqLESjrtSwmBP65744124 = cGFqLESjrtSwmBP70458903;     cGFqLESjrtSwmBP70458903 = cGFqLESjrtSwmBP72562821;     cGFqLESjrtSwmBP72562821 = cGFqLESjrtSwmBP90936372;     cGFqLESjrtSwmBP90936372 = cGFqLESjrtSwmBP36081580;     cGFqLESjrtSwmBP36081580 = cGFqLESjrtSwmBP13777648;     cGFqLESjrtSwmBP13777648 = cGFqLESjrtSwmBP10083777;     cGFqLESjrtSwmBP10083777 = cGFqLESjrtSwmBP27989545;     cGFqLESjrtSwmBP27989545 = cGFqLESjrtSwmBP94221021;     cGFqLESjrtSwmBP94221021 = cGFqLESjrtSwmBP17433622;     cGFqLESjrtSwmBP17433622 = cGFqLESjrtSwmBP21378804;     cGFqLESjrtSwmBP21378804 = cGFqLESjrtSwmBP62667486;     cGFqLESjrtSwmBP62667486 = cGFqLESjrtSwmBP338448;     cGFqLESjrtSwmBP338448 = cGFqLESjrtSwmBP92534020;     cGFqLESjrtSwmBP92534020 = cGFqLESjrtSwmBP42426569;     cGFqLESjrtSwmBP42426569 = cGFqLESjrtSwmBP16170595;     cGFqLESjrtSwmBP16170595 = cGFqLESjrtSwmBP59074417;     cGFqLESjrtSwmBP59074417 = cGFqLESjrtSwmBP12320550;     cGFqLESjrtSwmBP12320550 = cGFqLESjrtSwmBP48378413;     cGFqLESjrtSwmBP48378413 = cGFqLESjrtSwmBP34699274;     cGFqLESjrtSwmBP34699274 = cGFqLESjrtSwmBP72849790;     cGFqLESjrtSwmBP72849790 = cGFqLESjrtSwmBP86596788;     cGFqLESjrtSwmBP86596788 = cGFqLESjrtSwmBP63869965;     cGFqLESjrtSwmBP63869965 = cGFqLESjrtSwmBP57544546;     cGFqLESjrtSwmBP57544546 = cGFqLESjrtSwmBP2984503;     cGFqLESjrtSwmBP2984503 = cGFqLESjrtSwmBP13162407;     cGFqLESjrtSwmBP13162407 = cGFqLESjrtSwmBP56195358;     cGFqLESjrtSwmBP56195358 = cGFqLESjrtSwmBP46429828;     cGFqLESjrtSwmBP46429828 = cGFqLESjrtSwmBP95707684;     cGFqLESjrtSwmBP95707684 = cGFqLESjrtSwmBP78426087;     cGFqLESjrtSwmBP78426087 = cGFqLESjrtSwmBP96238710;     cGFqLESjrtSwmBP96238710 = cGFqLESjrtSwmBP70514991;     cGFqLESjrtSwmBP70514991 = cGFqLESjrtSwmBP26781786;     cGFqLESjrtSwmBP26781786 = cGFqLESjrtSwmBP48521003;     cGFqLESjrtSwmBP48521003 = cGFqLESjrtSwmBP55995444;     cGFqLESjrtSwmBP55995444 = cGFqLESjrtSwmBP47682215;     cGFqLESjrtSwmBP47682215 = cGFqLESjrtSwmBP97593545;     cGFqLESjrtSwmBP97593545 = cGFqLESjrtSwmBP37882172;     cGFqLESjrtSwmBP37882172 = cGFqLESjrtSwmBP10929570;     cGFqLESjrtSwmBP10929570 = cGFqLESjrtSwmBP98545934;     cGFqLESjrtSwmBP98545934 = cGFqLESjrtSwmBP97329041;     cGFqLESjrtSwmBP97329041 = cGFqLESjrtSwmBP31879609;     cGFqLESjrtSwmBP31879609 = cGFqLESjrtSwmBP93511045;     cGFqLESjrtSwmBP93511045 = cGFqLESjrtSwmBP71765469;     cGFqLESjrtSwmBP71765469 = cGFqLESjrtSwmBP14793188;     cGFqLESjrtSwmBP14793188 = cGFqLESjrtSwmBP29077271;     cGFqLESjrtSwmBP29077271 = cGFqLESjrtSwmBP65345925;     cGFqLESjrtSwmBP65345925 = cGFqLESjrtSwmBP87966474;     cGFqLESjrtSwmBP87966474 = cGFqLESjrtSwmBP30853409;     cGFqLESjrtSwmBP30853409 = cGFqLESjrtSwmBP18407635;     cGFqLESjrtSwmBP18407635 = cGFqLESjrtSwmBP86272314;     cGFqLESjrtSwmBP86272314 = cGFqLESjrtSwmBP40651964;     cGFqLESjrtSwmBP40651964 = cGFqLESjrtSwmBP50171945;     cGFqLESjrtSwmBP50171945 = cGFqLESjrtSwmBP87630327;     cGFqLESjrtSwmBP87630327 = cGFqLESjrtSwmBP92157144;     cGFqLESjrtSwmBP92157144 = cGFqLESjrtSwmBP79147336;     cGFqLESjrtSwmBP79147336 = cGFqLESjrtSwmBP6588938;     cGFqLESjrtSwmBP6588938 = cGFqLESjrtSwmBP15018275;     cGFqLESjrtSwmBP15018275 = cGFqLESjrtSwmBP87951870;     cGFqLESjrtSwmBP87951870 = cGFqLESjrtSwmBP22919173;     cGFqLESjrtSwmBP22919173 = cGFqLESjrtSwmBP57582289;     cGFqLESjrtSwmBP57582289 = cGFqLESjrtSwmBP63653949;     cGFqLESjrtSwmBP63653949 = cGFqLESjrtSwmBP32281861;     cGFqLESjrtSwmBP32281861 = cGFqLESjrtSwmBP15794934;     cGFqLESjrtSwmBP15794934 = cGFqLESjrtSwmBP42112093;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tpcXQaQkirGXINEVcgtBXTqZyUyuq62891334() {     int KEcEFpaolMLtXPg38625014 = -765656667;    int KEcEFpaolMLtXPg47801769 = -393186361;    int KEcEFpaolMLtXPg30011166 = -353360061;    int KEcEFpaolMLtXPg65617905 = -898821903;    int KEcEFpaolMLtXPg62368927 = -153662665;    int KEcEFpaolMLtXPg31976132 = 11660316;    int KEcEFpaolMLtXPg52661433 = -960570434;    int KEcEFpaolMLtXPg72148023 = -879769491;    int KEcEFpaolMLtXPg23964062 = -137116977;    int KEcEFpaolMLtXPg84290450 = -110567952;    int KEcEFpaolMLtXPg83988933 = -564162676;    int KEcEFpaolMLtXPg64227661 = -18060912;    int KEcEFpaolMLtXPg74336046 = -261842489;    int KEcEFpaolMLtXPg16186501 = -496372079;    int KEcEFpaolMLtXPg18427695 = -381061758;    int KEcEFpaolMLtXPg34696393 = 54358339;    int KEcEFpaolMLtXPg45831967 = -386865744;    int KEcEFpaolMLtXPg6291856 = -867008344;    int KEcEFpaolMLtXPg10741521 = -217786688;    int KEcEFpaolMLtXPg87736408 = -59224291;    int KEcEFpaolMLtXPg20002438 = -196858102;    int KEcEFpaolMLtXPg48314576 = -766563482;    int KEcEFpaolMLtXPg49366116 = 42853798;    int KEcEFpaolMLtXPg11556837 = -528451361;    int KEcEFpaolMLtXPg67474098 = 94576685;    int KEcEFpaolMLtXPg12196356 = -333966073;    int KEcEFpaolMLtXPg39641442 = -375155797;    int KEcEFpaolMLtXPg57934735 = -847428126;    int KEcEFpaolMLtXPg62696466 = -316144662;    int KEcEFpaolMLtXPg73534485 = -496662476;    int KEcEFpaolMLtXPg43672309 = -851368056;    int KEcEFpaolMLtXPg81096914 = -961580006;    int KEcEFpaolMLtXPg70416952 = -894521544;    int KEcEFpaolMLtXPg11210026 = 34367221;    int KEcEFpaolMLtXPg31814440 = -658389383;    int KEcEFpaolMLtXPg8678189 = -573853591;    int KEcEFpaolMLtXPg46301091 = -585316686;    int KEcEFpaolMLtXPg43585878 = -672318681;    int KEcEFpaolMLtXPg21297129 = -775361862;    int KEcEFpaolMLtXPg70453772 = -39148283;    int KEcEFpaolMLtXPg68352575 = -433405726;    int KEcEFpaolMLtXPg57971000 = -791560197;    int KEcEFpaolMLtXPg97625713 = 35598255;    int KEcEFpaolMLtXPg40197653 = -37899624;    int KEcEFpaolMLtXPg60207736 = -979293299;    int KEcEFpaolMLtXPg15114649 = -769271201;    int KEcEFpaolMLtXPg18290897 = -331843125;    int KEcEFpaolMLtXPg43157510 = 96234858;    int KEcEFpaolMLtXPg18947516 = -836568123;    int KEcEFpaolMLtXPg65335059 = -95048441;    int KEcEFpaolMLtXPg94215209 = -803249608;    int KEcEFpaolMLtXPg88579232 = -599277788;    int KEcEFpaolMLtXPg51217795 = -163055816;    int KEcEFpaolMLtXPg97827745 = -845401073;    int KEcEFpaolMLtXPg42888897 = -72669014;    int KEcEFpaolMLtXPg90310437 = -999093186;    int KEcEFpaolMLtXPg98435652 = -336040159;    int KEcEFpaolMLtXPg18454329 = -824908700;    int KEcEFpaolMLtXPg98143806 = -893398588;    int KEcEFpaolMLtXPg50172572 = -819696592;    int KEcEFpaolMLtXPg92334690 = -613183888;    int KEcEFpaolMLtXPg94726697 = -13142308;    int KEcEFpaolMLtXPg9451558 = -463624829;    int KEcEFpaolMLtXPg50429577 = -640454502;    int KEcEFpaolMLtXPg40618141 = -259199897;    int KEcEFpaolMLtXPg2892020 = -602582671;    int KEcEFpaolMLtXPg93810708 = -123539368;    int KEcEFpaolMLtXPg63126020 = -196209711;    int KEcEFpaolMLtXPg84372060 = -837982697;    int KEcEFpaolMLtXPg9749507 = -807208167;    int KEcEFpaolMLtXPg88395301 = -360324976;    int KEcEFpaolMLtXPg2246090 = -714547064;    int KEcEFpaolMLtXPg84994726 = 8353518;    int KEcEFpaolMLtXPg40287749 = -78638406;    int KEcEFpaolMLtXPg19383834 = -625818565;    int KEcEFpaolMLtXPg62031437 = -405297905;    int KEcEFpaolMLtXPg50688863 = -702161737;    int KEcEFpaolMLtXPg9168463 = -919246579;    int KEcEFpaolMLtXPg51349101 = -549158063;    int KEcEFpaolMLtXPg52359450 = -136152115;    int KEcEFpaolMLtXPg93905458 = 97877051;    int KEcEFpaolMLtXPg96483932 = -371390655;    int KEcEFpaolMLtXPg38987220 = 89139996;    int KEcEFpaolMLtXPg97361406 = -121096221;    int KEcEFpaolMLtXPg79319276 = -693412869;    int KEcEFpaolMLtXPg55093076 = -152090268;    int KEcEFpaolMLtXPg29879120 = -698524190;    int KEcEFpaolMLtXPg72589207 = 50879528;    int KEcEFpaolMLtXPg68321129 = -892963766;    int KEcEFpaolMLtXPg41504002 = -659296197;    int KEcEFpaolMLtXPg10242536 = -137813432;    int KEcEFpaolMLtXPg27846763 = -760407986;    int KEcEFpaolMLtXPg45442071 = -778920094;    int KEcEFpaolMLtXPg71124556 = -955665271;    int KEcEFpaolMLtXPg78119081 = -425964395;    int KEcEFpaolMLtXPg73625877 = -320263418;    int KEcEFpaolMLtXPg48519443 = -227935369;    int KEcEFpaolMLtXPg47196136 = -323947244;    int KEcEFpaolMLtXPg99579512 = -778699728;    int KEcEFpaolMLtXPg57315717 = -765656667;     KEcEFpaolMLtXPg38625014 = KEcEFpaolMLtXPg47801769;     KEcEFpaolMLtXPg47801769 = KEcEFpaolMLtXPg30011166;     KEcEFpaolMLtXPg30011166 = KEcEFpaolMLtXPg65617905;     KEcEFpaolMLtXPg65617905 = KEcEFpaolMLtXPg62368927;     KEcEFpaolMLtXPg62368927 = KEcEFpaolMLtXPg31976132;     KEcEFpaolMLtXPg31976132 = KEcEFpaolMLtXPg52661433;     KEcEFpaolMLtXPg52661433 = KEcEFpaolMLtXPg72148023;     KEcEFpaolMLtXPg72148023 = KEcEFpaolMLtXPg23964062;     KEcEFpaolMLtXPg23964062 = KEcEFpaolMLtXPg84290450;     KEcEFpaolMLtXPg84290450 = KEcEFpaolMLtXPg83988933;     KEcEFpaolMLtXPg83988933 = KEcEFpaolMLtXPg64227661;     KEcEFpaolMLtXPg64227661 = KEcEFpaolMLtXPg74336046;     KEcEFpaolMLtXPg74336046 = KEcEFpaolMLtXPg16186501;     KEcEFpaolMLtXPg16186501 = KEcEFpaolMLtXPg18427695;     KEcEFpaolMLtXPg18427695 = KEcEFpaolMLtXPg34696393;     KEcEFpaolMLtXPg34696393 = KEcEFpaolMLtXPg45831967;     KEcEFpaolMLtXPg45831967 = KEcEFpaolMLtXPg6291856;     KEcEFpaolMLtXPg6291856 = KEcEFpaolMLtXPg10741521;     KEcEFpaolMLtXPg10741521 = KEcEFpaolMLtXPg87736408;     KEcEFpaolMLtXPg87736408 = KEcEFpaolMLtXPg20002438;     KEcEFpaolMLtXPg20002438 = KEcEFpaolMLtXPg48314576;     KEcEFpaolMLtXPg48314576 = KEcEFpaolMLtXPg49366116;     KEcEFpaolMLtXPg49366116 = KEcEFpaolMLtXPg11556837;     KEcEFpaolMLtXPg11556837 = KEcEFpaolMLtXPg67474098;     KEcEFpaolMLtXPg67474098 = KEcEFpaolMLtXPg12196356;     KEcEFpaolMLtXPg12196356 = KEcEFpaolMLtXPg39641442;     KEcEFpaolMLtXPg39641442 = KEcEFpaolMLtXPg57934735;     KEcEFpaolMLtXPg57934735 = KEcEFpaolMLtXPg62696466;     KEcEFpaolMLtXPg62696466 = KEcEFpaolMLtXPg73534485;     KEcEFpaolMLtXPg73534485 = KEcEFpaolMLtXPg43672309;     KEcEFpaolMLtXPg43672309 = KEcEFpaolMLtXPg81096914;     KEcEFpaolMLtXPg81096914 = KEcEFpaolMLtXPg70416952;     KEcEFpaolMLtXPg70416952 = KEcEFpaolMLtXPg11210026;     KEcEFpaolMLtXPg11210026 = KEcEFpaolMLtXPg31814440;     KEcEFpaolMLtXPg31814440 = KEcEFpaolMLtXPg8678189;     KEcEFpaolMLtXPg8678189 = KEcEFpaolMLtXPg46301091;     KEcEFpaolMLtXPg46301091 = KEcEFpaolMLtXPg43585878;     KEcEFpaolMLtXPg43585878 = KEcEFpaolMLtXPg21297129;     KEcEFpaolMLtXPg21297129 = KEcEFpaolMLtXPg70453772;     KEcEFpaolMLtXPg70453772 = KEcEFpaolMLtXPg68352575;     KEcEFpaolMLtXPg68352575 = KEcEFpaolMLtXPg57971000;     KEcEFpaolMLtXPg57971000 = KEcEFpaolMLtXPg97625713;     KEcEFpaolMLtXPg97625713 = KEcEFpaolMLtXPg40197653;     KEcEFpaolMLtXPg40197653 = KEcEFpaolMLtXPg60207736;     KEcEFpaolMLtXPg60207736 = KEcEFpaolMLtXPg15114649;     KEcEFpaolMLtXPg15114649 = KEcEFpaolMLtXPg18290897;     KEcEFpaolMLtXPg18290897 = KEcEFpaolMLtXPg43157510;     KEcEFpaolMLtXPg43157510 = KEcEFpaolMLtXPg18947516;     KEcEFpaolMLtXPg18947516 = KEcEFpaolMLtXPg65335059;     KEcEFpaolMLtXPg65335059 = KEcEFpaolMLtXPg94215209;     KEcEFpaolMLtXPg94215209 = KEcEFpaolMLtXPg88579232;     KEcEFpaolMLtXPg88579232 = KEcEFpaolMLtXPg51217795;     KEcEFpaolMLtXPg51217795 = KEcEFpaolMLtXPg97827745;     KEcEFpaolMLtXPg97827745 = KEcEFpaolMLtXPg42888897;     KEcEFpaolMLtXPg42888897 = KEcEFpaolMLtXPg90310437;     KEcEFpaolMLtXPg90310437 = KEcEFpaolMLtXPg98435652;     KEcEFpaolMLtXPg98435652 = KEcEFpaolMLtXPg18454329;     KEcEFpaolMLtXPg18454329 = KEcEFpaolMLtXPg98143806;     KEcEFpaolMLtXPg98143806 = KEcEFpaolMLtXPg50172572;     KEcEFpaolMLtXPg50172572 = KEcEFpaolMLtXPg92334690;     KEcEFpaolMLtXPg92334690 = KEcEFpaolMLtXPg94726697;     KEcEFpaolMLtXPg94726697 = KEcEFpaolMLtXPg9451558;     KEcEFpaolMLtXPg9451558 = KEcEFpaolMLtXPg50429577;     KEcEFpaolMLtXPg50429577 = KEcEFpaolMLtXPg40618141;     KEcEFpaolMLtXPg40618141 = KEcEFpaolMLtXPg2892020;     KEcEFpaolMLtXPg2892020 = KEcEFpaolMLtXPg93810708;     KEcEFpaolMLtXPg93810708 = KEcEFpaolMLtXPg63126020;     KEcEFpaolMLtXPg63126020 = KEcEFpaolMLtXPg84372060;     KEcEFpaolMLtXPg84372060 = KEcEFpaolMLtXPg9749507;     KEcEFpaolMLtXPg9749507 = KEcEFpaolMLtXPg88395301;     KEcEFpaolMLtXPg88395301 = KEcEFpaolMLtXPg2246090;     KEcEFpaolMLtXPg2246090 = KEcEFpaolMLtXPg84994726;     KEcEFpaolMLtXPg84994726 = KEcEFpaolMLtXPg40287749;     KEcEFpaolMLtXPg40287749 = KEcEFpaolMLtXPg19383834;     KEcEFpaolMLtXPg19383834 = KEcEFpaolMLtXPg62031437;     KEcEFpaolMLtXPg62031437 = KEcEFpaolMLtXPg50688863;     KEcEFpaolMLtXPg50688863 = KEcEFpaolMLtXPg9168463;     KEcEFpaolMLtXPg9168463 = KEcEFpaolMLtXPg51349101;     KEcEFpaolMLtXPg51349101 = KEcEFpaolMLtXPg52359450;     KEcEFpaolMLtXPg52359450 = KEcEFpaolMLtXPg93905458;     KEcEFpaolMLtXPg93905458 = KEcEFpaolMLtXPg96483932;     KEcEFpaolMLtXPg96483932 = KEcEFpaolMLtXPg38987220;     KEcEFpaolMLtXPg38987220 = KEcEFpaolMLtXPg97361406;     KEcEFpaolMLtXPg97361406 = KEcEFpaolMLtXPg79319276;     KEcEFpaolMLtXPg79319276 = KEcEFpaolMLtXPg55093076;     KEcEFpaolMLtXPg55093076 = KEcEFpaolMLtXPg29879120;     KEcEFpaolMLtXPg29879120 = KEcEFpaolMLtXPg72589207;     KEcEFpaolMLtXPg72589207 = KEcEFpaolMLtXPg68321129;     KEcEFpaolMLtXPg68321129 = KEcEFpaolMLtXPg41504002;     KEcEFpaolMLtXPg41504002 = KEcEFpaolMLtXPg10242536;     KEcEFpaolMLtXPg10242536 = KEcEFpaolMLtXPg27846763;     KEcEFpaolMLtXPg27846763 = KEcEFpaolMLtXPg45442071;     KEcEFpaolMLtXPg45442071 = KEcEFpaolMLtXPg71124556;     KEcEFpaolMLtXPg71124556 = KEcEFpaolMLtXPg78119081;     KEcEFpaolMLtXPg78119081 = KEcEFpaolMLtXPg73625877;     KEcEFpaolMLtXPg73625877 = KEcEFpaolMLtXPg48519443;     KEcEFpaolMLtXPg48519443 = KEcEFpaolMLtXPg47196136;     KEcEFpaolMLtXPg47196136 = KEcEFpaolMLtXPg99579512;     KEcEFpaolMLtXPg99579512 = KEcEFpaolMLtXPg57315717;     KEcEFpaolMLtXPg57315717 = KEcEFpaolMLtXPg38625014;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void McFzqbOvNnqpbFAUEirEvhGBtCRdO65080672() {     int MFzJLunYfVgIoid35137935 = -764457355;    int MFzJLunYfVgIoid62884595 = -345874664;    int MFzJLunYfVgIoid85693476 = -101462827;    int MFzJLunYfVgIoid78166753 = -267635693;    int MFzJLunYfVgIoid31021318 = -809249660;    int MFzJLunYfVgIoid69223004 = -18867261;    int MFzJLunYfVgIoid26476086 = -25488409;    int MFzJLunYfVgIoid7600194 = -992653675;    int MFzJLunYfVgIoid28218319 = -389534731;    int MFzJLunYfVgIoid38294987 = -869293165;    int MFzJLunYfVgIoid44205049 = -539397914;    int MFzJLunYfVgIoid79244098 = -586793817;    int MFzJLunYfVgIoid79606744 = -547628153;    int MFzJLunYfVgIoid83261613 = 94106386;    int MFzJLunYfVgIoid23327452 = -232388468;    int MFzJLunYfVgIoid47653216 = -804262877;    int MFzJLunYfVgIoid73522815 = -299712789;    int MFzJLunYfVgIoid42427343 = -384534409;    int MFzJLunYfVgIoid92664496 = -706913988;    int MFzJLunYfVgIoid28461668 = 31250878;    int MFzJLunYfVgIoid27681293 = -407407700;    int MFzJLunYfVgIoid89216333 = -44581174;    int MFzJLunYfVgIoid38863079 = -238065913;    int MFzJLunYfVgIoid35381607 = -487568765;    int MFzJLunYfVgIoid45749106 = -530849075;    int MFzJLunYfVgIoid88220720 = -513261336;    int MFzJLunYfVgIoid87538126 = -202337643;    int MFzJLunYfVgIoid50185098 = -503184225;    int MFzJLunYfVgIoid44892437 = -181522645;    int MFzJLunYfVgIoid73788992 = -961439318;    int MFzJLunYfVgIoid52766389 = -53336209;    int MFzJLunYfVgIoid16847098 = -193586026;    int MFzJLunYfVgIoid87861391 = -784618312;    int MFzJLunYfVgIoid23869695 = -886142779;    int MFzJLunYfVgIoid41299279 = -873054908;    int MFzJLunYfVgIoid52349442 = -850930300;    int MFzJLunYfVgIoid26858058 = -95740644;    int MFzJLunYfVgIoid16712852 = -493450650;    int MFzJLunYfVgIoid70031435 = -16430613;    int MFzJLunYfVgIoid49971171 = -575496165;    int MFzJLunYfVgIoid623570 = -964947897;    int MFzJLunYfVgIoid2164353 = -199001160;    int MFzJLunYfVgIoid85167649 = -460674317;    int MFzJLunYfVgIoid52405762 = -179533378;    int MFzJLunYfVgIoid26194451 = -411447637;    int MFzJLunYfVgIoid12795675 = -1598997;    int MFzJLunYfVgIoid15202990 = -610363773;    int MFzJLunYfVgIoid23647533 = -66859142;    int MFzJLunYfVgIoid37556584 = -368072318;    int MFzJLunYfVgIoid38136098 = -444587897;    int MFzJLunYfVgIoid46003849 = -332628814;    int MFzJLunYfVgIoid60987871 = -81443038;    int MFzJLunYfVgIoid43361174 = -954964460;    int MFzJLunYfVgIoid83334941 = -195602077;    int MFzJLunYfVgIoid37399380 = -505390807;    int MFzJLunYfVgIoid45921601 = -619876181;    int MFzJLunYfVgIoid24021516 = -7808751;    int MFzJLunYfVgIoid50311869 = -613894062;    int MFzJLunYfVgIoid32417648 = -736786619;    int MFzJLunYfVgIoid42800597 = -195988325;    int MFzJLunYfVgIoid81684877 = -816529618;    int MFzJLunYfVgIoid76290988 = -522304184;    int MFzJLunYfVgIoid62707756 = -711131030;    int MFzJLunYfVgIoid54429326 = -428095413;    int MFzJLunYfVgIoid85528597 = -715956956;    int MFzJLunYfVgIoid27357952 = -245811888;    int MFzJLunYfVgIoid91382707 = -802175506;    int MFzJLunYfVgIoid55737050 = -661485374;    int MFzJLunYfVgIoid41962335 = -32838706;    int MFzJLunYfVgIoid70978010 = -381458168;    int MFzJLunYfVgIoid20795158 = -608522233;    int MFzJLunYfVgIoid56809964 = -806262140;    int MFzJLunYfVgIoid72395908 = -268103797;    int MFzJLunYfVgIoid42693326 = -31417824;    int MFzJLunYfVgIoid27838098 = -3801225;    int MFzJLunYfVgIoid25516941 = -108406541;    int MFzJLunYfVgIoid4048685 = -583906858;    int MFzJLunYfVgIoid86457317 = 41467464;    int MFzJLunYfVgIoid9187156 = 23878871;    int MFzJLunYfVgIoid32953431 = -429250078;    int MFzJLunYfVgIoid73017730 = -902897563;    int MFzJLunYfVgIoid63890594 = -35478502;    int MFzJLunYfVgIoid12628515 = -35111908;    int MFzJLunYfVgIoid6756339 = -736934749;    int MFzJLunYfVgIoid27785143 = -528810505;    int MFzJLunYfVgIoid91778517 = -971893171;    int MFzJLunYfVgIoid73485924 = -238621566;    int MFzJLunYfVgIoid4526451 = -489016236;    int MFzJLunYfVgIoid86470314 = -280751973;    int MFzJLunYfVgIoid95377677 = -153178727;    int MFzJLunYfVgIoid28327926 = -743121549;    int MFzJLunYfVgIoid76546188 = -481846582;    int MFzJLunYfVgIoid84295204 = -756664031;    int MFzJLunYfVgIoid27230838 = -820442288;    int MFzJLunYfVgIoid68286293 = -758966548;    int MFzJLunYfVgIoid24332582 = -342643713;    int MFzJLunYfVgIoid39456596 = -487870130;    int MFzJLunYfVgIoid30738323 = 67421096;    int MFzJLunYfVgIoid66877164 = -463576422;    int MFzJLunYfVgIoid98836499 = -764457355;     MFzJLunYfVgIoid35137935 = MFzJLunYfVgIoid62884595;     MFzJLunYfVgIoid62884595 = MFzJLunYfVgIoid85693476;     MFzJLunYfVgIoid85693476 = MFzJLunYfVgIoid78166753;     MFzJLunYfVgIoid78166753 = MFzJLunYfVgIoid31021318;     MFzJLunYfVgIoid31021318 = MFzJLunYfVgIoid69223004;     MFzJLunYfVgIoid69223004 = MFzJLunYfVgIoid26476086;     MFzJLunYfVgIoid26476086 = MFzJLunYfVgIoid7600194;     MFzJLunYfVgIoid7600194 = MFzJLunYfVgIoid28218319;     MFzJLunYfVgIoid28218319 = MFzJLunYfVgIoid38294987;     MFzJLunYfVgIoid38294987 = MFzJLunYfVgIoid44205049;     MFzJLunYfVgIoid44205049 = MFzJLunYfVgIoid79244098;     MFzJLunYfVgIoid79244098 = MFzJLunYfVgIoid79606744;     MFzJLunYfVgIoid79606744 = MFzJLunYfVgIoid83261613;     MFzJLunYfVgIoid83261613 = MFzJLunYfVgIoid23327452;     MFzJLunYfVgIoid23327452 = MFzJLunYfVgIoid47653216;     MFzJLunYfVgIoid47653216 = MFzJLunYfVgIoid73522815;     MFzJLunYfVgIoid73522815 = MFzJLunYfVgIoid42427343;     MFzJLunYfVgIoid42427343 = MFzJLunYfVgIoid92664496;     MFzJLunYfVgIoid92664496 = MFzJLunYfVgIoid28461668;     MFzJLunYfVgIoid28461668 = MFzJLunYfVgIoid27681293;     MFzJLunYfVgIoid27681293 = MFzJLunYfVgIoid89216333;     MFzJLunYfVgIoid89216333 = MFzJLunYfVgIoid38863079;     MFzJLunYfVgIoid38863079 = MFzJLunYfVgIoid35381607;     MFzJLunYfVgIoid35381607 = MFzJLunYfVgIoid45749106;     MFzJLunYfVgIoid45749106 = MFzJLunYfVgIoid88220720;     MFzJLunYfVgIoid88220720 = MFzJLunYfVgIoid87538126;     MFzJLunYfVgIoid87538126 = MFzJLunYfVgIoid50185098;     MFzJLunYfVgIoid50185098 = MFzJLunYfVgIoid44892437;     MFzJLunYfVgIoid44892437 = MFzJLunYfVgIoid73788992;     MFzJLunYfVgIoid73788992 = MFzJLunYfVgIoid52766389;     MFzJLunYfVgIoid52766389 = MFzJLunYfVgIoid16847098;     MFzJLunYfVgIoid16847098 = MFzJLunYfVgIoid87861391;     MFzJLunYfVgIoid87861391 = MFzJLunYfVgIoid23869695;     MFzJLunYfVgIoid23869695 = MFzJLunYfVgIoid41299279;     MFzJLunYfVgIoid41299279 = MFzJLunYfVgIoid52349442;     MFzJLunYfVgIoid52349442 = MFzJLunYfVgIoid26858058;     MFzJLunYfVgIoid26858058 = MFzJLunYfVgIoid16712852;     MFzJLunYfVgIoid16712852 = MFzJLunYfVgIoid70031435;     MFzJLunYfVgIoid70031435 = MFzJLunYfVgIoid49971171;     MFzJLunYfVgIoid49971171 = MFzJLunYfVgIoid623570;     MFzJLunYfVgIoid623570 = MFzJLunYfVgIoid2164353;     MFzJLunYfVgIoid2164353 = MFzJLunYfVgIoid85167649;     MFzJLunYfVgIoid85167649 = MFzJLunYfVgIoid52405762;     MFzJLunYfVgIoid52405762 = MFzJLunYfVgIoid26194451;     MFzJLunYfVgIoid26194451 = MFzJLunYfVgIoid12795675;     MFzJLunYfVgIoid12795675 = MFzJLunYfVgIoid15202990;     MFzJLunYfVgIoid15202990 = MFzJLunYfVgIoid23647533;     MFzJLunYfVgIoid23647533 = MFzJLunYfVgIoid37556584;     MFzJLunYfVgIoid37556584 = MFzJLunYfVgIoid38136098;     MFzJLunYfVgIoid38136098 = MFzJLunYfVgIoid46003849;     MFzJLunYfVgIoid46003849 = MFzJLunYfVgIoid60987871;     MFzJLunYfVgIoid60987871 = MFzJLunYfVgIoid43361174;     MFzJLunYfVgIoid43361174 = MFzJLunYfVgIoid83334941;     MFzJLunYfVgIoid83334941 = MFzJLunYfVgIoid37399380;     MFzJLunYfVgIoid37399380 = MFzJLunYfVgIoid45921601;     MFzJLunYfVgIoid45921601 = MFzJLunYfVgIoid24021516;     MFzJLunYfVgIoid24021516 = MFzJLunYfVgIoid50311869;     MFzJLunYfVgIoid50311869 = MFzJLunYfVgIoid32417648;     MFzJLunYfVgIoid32417648 = MFzJLunYfVgIoid42800597;     MFzJLunYfVgIoid42800597 = MFzJLunYfVgIoid81684877;     MFzJLunYfVgIoid81684877 = MFzJLunYfVgIoid76290988;     MFzJLunYfVgIoid76290988 = MFzJLunYfVgIoid62707756;     MFzJLunYfVgIoid62707756 = MFzJLunYfVgIoid54429326;     MFzJLunYfVgIoid54429326 = MFzJLunYfVgIoid85528597;     MFzJLunYfVgIoid85528597 = MFzJLunYfVgIoid27357952;     MFzJLunYfVgIoid27357952 = MFzJLunYfVgIoid91382707;     MFzJLunYfVgIoid91382707 = MFzJLunYfVgIoid55737050;     MFzJLunYfVgIoid55737050 = MFzJLunYfVgIoid41962335;     MFzJLunYfVgIoid41962335 = MFzJLunYfVgIoid70978010;     MFzJLunYfVgIoid70978010 = MFzJLunYfVgIoid20795158;     MFzJLunYfVgIoid20795158 = MFzJLunYfVgIoid56809964;     MFzJLunYfVgIoid56809964 = MFzJLunYfVgIoid72395908;     MFzJLunYfVgIoid72395908 = MFzJLunYfVgIoid42693326;     MFzJLunYfVgIoid42693326 = MFzJLunYfVgIoid27838098;     MFzJLunYfVgIoid27838098 = MFzJLunYfVgIoid25516941;     MFzJLunYfVgIoid25516941 = MFzJLunYfVgIoid4048685;     MFzJLunYfVgIoid4048685 = MFzJLunYfVgIoid86457317;     MFzJLunYfVgIoid86457317 = MFzJLunYfVgIoid9187156;     MFzJLunYfVgIoid9187156 = MFzJLunYfVgIoid32953431;     MFzJLunYfVgIoid32953431 = MFzJLunYfVgIoid73017730;     MFzJLunYfVgIoid73017730 = MFzJLunYfVgIoid63890594;     MFzJLunYfVgIoid63890594 = MFzJLunYfVgIoid12628515;     MFzJLunYfVgIoid12628515 = MFzJLunYfVgIoid6756339;     MFzJLunYfVgIoid6756339 = MFzJLunYfVgIoid27785143;     MFzJLunYfVgIoid27785143 = MFzJLunYfVgIoid91778517;     MFzJLunYfVgIoid91778517 = MFzJLunYfVgIoid73485924;     MFzJLunYfVgIoid73485924 = MFzJLunYfVgIoid4526451;     MFzJLunYfVgIoid4526451 = MFzJLunYfVgIoid86470314;     MFzJLunYfVgIoid86470314 = MFzJLunYfVgIoid95377677;     MFzJLunYfVgIoid95377677 = MFzJLunYfVgIoid28327926;     MFzJLunYfVgIoid28327926 = MFzJLunYfVgIoid76546188;     MFzJLunYfVgIoid76546188 = MFzJLunYfVgIoid84295204;     MFzJLunYfVgIoid84295204 = MFzJLunYfVgIoid27230838;     MFzJLunYfVgIoid27230838 = MFzJLunYfVgIoid68286293;     MFzJLunYfVgIoid68286293 = MFzJLunYfVgIoid24332582;     MFzJLunYfVgIoid24332582 = MFzJLunYfVgIoid39456596;     MFzJLunYfVgIoid39456596 = MFzJLunYfVgIoid30738323;     MFzJLunYfVgIoid30738323 = MFzJLunYfVgIoid66877164;     MFzJLunYfVgIoid66877164 = MFzJLunYfVgIoid98836499;     MFzJLunYfVgIoid98836499 = MFzJLunYfVgIoid35137935;}
// Junk Finished
