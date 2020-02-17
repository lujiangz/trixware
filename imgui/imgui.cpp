// dear imgui, v1.52 WIP
// (main code and documentation)

// See ImGui::ShowTestWindow() in imgui_demo.cpp for demo code.
// Newcomers, read 'Programmer guide' below for notes on how to setup Dear ImGui in your codebase.
// Get latest version at https://github.com/ocornut/imgui
// Releases change-log at https://github.com/ocornut/imgui/releases
// Gallery (please post your screenshots/video there!): https://github.com/ocornut/imgui/issues/1269
// Developed by Omar Cornut and every direct or indirect contributors to the GitHub.
// This library is free but I need your support to sustain development and maintenance.
// If you work for a company, please consider financial support, e.g: https://www.patreon.com/imgui

/*

 Index
 - MISSION STATEMENT
 - END-USER GUIDE
 - PROGRAMMER GUIDE (read me!)
   - Read first
   - How to update to a newer version of Dear ImGui
   - Getting started with integrating Dear ImGui in your code/engine
 - API BREAKING CHANGES (read me when you update!)
 - ISSUES & TODO LIST
 - FREQUENTLY ASKED QUESTIONS (FAQ), TIPS
   - How can I help?
   - What is ImTextureID and how do I display an image?
   - I integrated Dear ImGui in my engine and the text or lines are blurry..
   - I integrated Dear ImGui in my engine and some elements are clipping or disappearing when I move windows around..
   - How can I have multiple widgets with the same label? Can I have widget without a label? (Yes). A primer on labels/IDs.
   - How can I tell when Dear ImGui wants my mouse/keyboard inputs VS when I can pass them to my application?
   - How can I load a different font than the default?
   - How can I easily use icons in my application?
   - How can I load multiple fonts?
   - How can I display and input non-latin characters such as Chinese, Japanese, Korean, Cyrillic?
   - How can I preserve my Dear ImGui context across reloading a DLL? (loss of the global/static variables)
   - How can I use the drawing facilities without an ImGui window? (using ImDrawList API)
 - ISSUES & TODO-LIST
 - CODE


 MISSION STATEMENT
 =================

 - Easy to use to create code-driven and data-driven tools
 - Easy to use to create ad hoc short-lived tools and long-lived, more elaborate tools
 - Easy to hack and improve
 - Minimize screen real-estate usage
 - Minimize setup and maintenance
 - Minimize state storage on user side
 - Portable, minimize dependencies, run on target (consoles, phones, etc.)
 - Efficient runtime and memory consumption (NB- we do allocate when "growing" content e.g. creating a window, opening a tree node 
   for the first time, etc. but a typical frame won't allocate anything)

 Designed for developers and content-creators, not the typical end-user! Some of the weaknesses includes:
 - Doesn't look fancy, doesn't animate
 - Limited layout features, intricate layouts are typically crafted in code


 END-USER GUIDE
 ==============

 - Double-click title bar to collapse window
 - Click upper right corner to close a window, available when 'bool* p_open' is passed to ImGui::Begin()
 - Click and drag on lower right corner to resize window
 - Click and drag on any empty space to move window
 - Double-click/double-tap on lower right corner grip to auto-fit to content
 - TAB/SHIFT+TAB to cycle through keyboard editable fields
 - Use mouse wheel to scroll
 - Use CTRL+mouse wheel to zoom window contents (if io.FontAllowScaling is true)
 - CTRL+Click on a slider or drag box to input value as text
 - Text editor:
   - Hold SHIFT or use mouse to select text.
   - CTRL+Left/Right to word jump
   - CTRL+Shift+Left/Right to select words
   - CTRL+A our Double-Click to select all
   - CTRL+X,CTRL+C,CTRL+V to use OS clipboard
   - CTRL+Z,CTRL+Y to undo/redo
   - ESCAPE to revert text to its original value
   - You can apply arithmetic operators +,*,/ on numerical values. Use +- to subtract (because - would set a negative value!)
   - Controls are automatically adjusted for OSX to match standard OSX text editing operations.


 PROGRAMMER GUIDE
 ================

 READ FIRST

 - Read the FAQ below this section!
 - Your code creates the UI, if your code doesn't run the UI is gone! == very dynamic UI, no construction/destructions steps, less data retention
   on your side, no state duplication, less sync, less bugs.
 - Call and read ImGui::ShowTestWindow() for demo code demonstrating most features.
 - You can learn about immediate-mode gui principles at http://www.johno.se/book/imgui.html or watch http://mollyrocket.com/861

 HOW TO UPDATE TO A NEWER VERSION OF DEAR IMGUI

 - Overwrite all the sources files except for imconfig.h (if you have made modification to your copy of imconfig.h)
 - Read the "API BREAKING CHANGES" section (below). This is where we list occasional API breaking changes. 
   If a function/type has been renamed / or marked obsolete, try to fix the name in your code before it is permanently removed from the public API.
   If you have a problem with a missing function/symbols, search for its name in the code, there will likely be a comment about it. 
   Please report any issue to the GitHub page!
 - Try to keep your copy of dear imgui reasonably up to date.

 GETTING STARTED WITH INTEGRATING DEAR IMGUI IN YOUR CODE/ENGINE

 - Add the Dear ImGui source files to your projects, using your preferred build system. 
   It is recommended you build the .cpp files as part of your project and not as a library.
 - You can later customize the imconfig.h file to tweak some compilation time behavior, such as integrating imgui types with your own maths types.
 - See examples/ folder for standalone sample applications. To understand the integration process, you can read examples/opengl2_example/ because 
   it is short, then switch to the one more appropriate to your use case.
 - You may be able to grab and copy a ready made imgui_impl_*** file from the examples/.
 - When using Dear ImGui, your programming IDE is your friend: follow the declaration of variables, functions and types to find comments about them.

 - Init: retrieve the ImGuiIO structure with ImGui::GetIO() and fill the fields marked 'Settings': at minimum you need to set io.DisplaySize
   (application resolution). Later on you will fill your keyboard mapping, clipboard handlers, and other advanced features but for a basic 
   integration you don't need to worry about it all.
 - Init: call io.Fonts->GetTexDataAsRGBA32(...), it will build the font atlas texture, then load the texture pixels into graphics memory.
 - Every frame:
    - In your main loop as early a possible, fill the IO fields marked 'Input' (e.g. mouse position, buttons, keyboard info, etc.)
    - Call ImGui::NewFrame() to begin the frame
    - You can use any ImGui function you want between NewFrame() and Render()
    - Call ImGui::Render() as late as you can to end the frame and finalize render data. it will call your io.RenderDrawListFn handler.
       (Even if you don't render, call Render() and ignore the callback, or call EndFrame() instead. Otherwhise some features will break)
 - All rendering information are stored into command-lists until ImGui::Render() is called.
 - Dear ImGui never touches or knows about your GPU state. the only function that knows about GPU is the RenderDrawListFn handler that you provide.
 - Effectively it means you can create widgets at any time in your code, regardless of considerations of being in "update" vs "render" phases 
   of your own application.
 - Refer to the examples applications in the examples/ folder for instruction on how to setup your code.
 - A minimal application skeleton may be:

     // Application init
     ImGuiIO& io = ImGui::GetIO();
     io.DisplaySize.x = 1920.0f;
     io.DisplaySize.y = 1280.0f;
     io.RenderDrawListsFn = MyRenderFunction;  // Setup a render function, or set to NULL and call GetDrawData() after Render() to access render data.
     // TODO: Fill others settings of the io structure later.

     // Load texture atlas (there is a default font so you don't need to care about choosing a font yet)
     unsigned char* pixels;
     int width, height;
     io.Fonts->GetTexDataAsRGBA32(pixels, &width, &height);
     // TODO: At this points you've got the texture data and you need to upload that your your graphic system:
     MyTexture* texture = MyEngine::CreateTextureFromMemoryPixels(pixels, width, height, TEXTURE_TYPE_RGBA)
     // TODO: Store your texture pointer/identifier (whatever your engine uses) in 'io.Fonts->TexID'. This will be passed back to your via the renderer.
     io.Fonts->TexID = (void*)texture;

     // Application main loop
     while (true)
     {
        // Setup low-level inputs (e.g. on Win32, GetKeyboardState(), or write to those fields from your Windows message loop handlers, etc.)
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = 1.0f/60.0f;
        io.MousePos = mouse_pos;
        io.MouseDown[0] = mouse_button_0;
        io.MouseDown[1] = mouse_button_1;

        // Call NewFrame(), after this point you can use ImGui::* functions anytime
        ImGui::NewFrame();

        // Most of your application code here
        MyGameUpdate(); // may use any ImGui functions, e.g. ImGui::Begin("My window"); ImGui::Text("Hello, world!"); ImGui::End();
        MyGameRender(); // may use any ImGui functions as well!
     
        // Render & swap video buffers
        ImGui::Render();
        SwapBuffers();
     }

 - A minimal render function skeleton may be:

    void void MyRenderFunction(ImDrawData* draw_data)(ImDrawData* draw_data)
    {
       // TODO: Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
       // TODO: Setup viewport, orthographic projection matrix
       // TODO: Setup shader: vertex { float2 pos, float2 uv, u32 color }, fragment shader sample color from 1 texture, multiply by vertex color.
       for (int n = 0; n < draw_data->CmdListsCount; n++)
       {
          const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;  // vertex buffer generated by ImGui
          const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;   // index buffer generated by ImGui
          for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
          {
             const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
             if (pcmd->UserCallback)
             {
                 pcmd->UserCallback(cmd_list, pcmd);
             }
             else
             {
                 // The texture for the draw call is specified by pcmd->TextureId. 
                 // The vast majority of draw calls with use the imgui texture atlas, which value you have set yourself during initialization. 
                 MyEngineBindTexture(pcmd->TextureId);

                 // We are using scissoring to clip some objects. All low-level graphics API supports it.
                 // If your engine doesn't support scissoring yet, you will get some small glitches (some elements outside their bounds) which you can fix later.
                 MyEngineScissor((int)pcmd->ClipRect.x, (int)pcmd->ClipRect.y, (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));

                 // Render 'pcmd->ElemCount/3' indexed triangles.
                 // By default the indices ImDrawIdx are 16-bits, you can change them to 32-bits if your engine doesn't support 16-bits indices.
                 MyEngineDrawIndexedTriangles(pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer, vtx_buffer);
             }
             idx_buffer += pcmd->ElemCount;
          }
       }
    }

 - The examples/ folders contains many functional implementation of the pseudo-code above.
 - When calling NewFrame(), the 'io.WantCaptureMouse'/'io.WantCaptureKeyboard'/'io.WantTextInput' flags are updated. 
   They tell you if ImGui intends to use your inputs. So for example, if 'io.WantCaptureMouse' is set you would typically want to hide 
   mouse inputs from the rest of your application. Read the FAQ below for more information about those flags.



 API BREAKING CHANGES
 ====================

 Occasionally introducing changes that are breaking the API. The breakage are generally minor and easy to fix.
 Here is a change-log of API breaking changes, if you are using one of the functions listed, expect to have to fix some code.
 Also read releases logs https://github.com/ocornut/imgui/releases for more details.

 - 2017/10/17 (1.52) - marked the old 5-parameters version of Begin() as obsolete (still available). Use SetNextWindowSize()+Begin() instead!
 - 2017/10/11 (1.52) - renamed AlignFirstTextHeightToWidgets() to AlignTextToFramePadding(). Kept inline redirection function (will obsolete).
 - 2017/09/25 (1.52) - removed SetNextWindowPosCenter() because SetNextWindowPos() now has the optional pivot information to do the same and more. Kept redirection function (will obsolete). 
 - 2017/08/25 (1.52) - io.MousePos needs to be set to ImVec2(-FLT_MAX,-FLT_MAX) when mouse is unavailable/missing. Previously ImVec2(-1,-1) was enough but we now accept negative mouse coordinates. In your binding if you need to support unavailable mouse, make sure to replace "io.MousePos = ImVec2(-1,-1)" with "io.MousePos = ImVec2(-FLT_MAX,-FLT_MAX)".
 - 2017/08/22 (1.51) - renamed IsItemHoveredRect() to IsItemRectHovered(). Kept inline redirection function (will obsolete).
                     - renamed IsMouseHoveringAnyWindow() to IsAnyWindowHovered() for consistency. Kept inline redirection function (will obsolete).
                     - renamed IsMouseHoveringWindow() to IsWindowRectHovered() for consistency. Kept inline redirection function (will obsolete).
 - 2017/08/20 (1.51) - renamed GetStyleColName() to GetStyleColorName() for consistency.
 - 2017/08/20 (1.51) - added PushStyleColor(ImGuiCol idx, ImU32 col) overload, which _might_ cause an "ambiguous call" compilation error if you are using ImColor() with implicit cast. Cast to ImU32 or ImVec4 explicily to fix.
 - 2017/08/15 (1.51) - marked the weird IMGUI_ONCE_UPON_A_FRAME helper macro as obsolete. prefer using the more explicit ImGuiOnceUponAFrame.
 - 2017/08/15 (1.51) - changed parameter order for BeginPopupContextWindow() from (const char*,int buttons,bool also_over_items) to (const char*,int buttons,bool also_over_items). Note that most calls relied on default parameters completely.
 - 2017/08/13 (1.51) - renamed ImGuiCol_Columns*** to ImGuiCol_Separator***. Kept redirection enums (will obsolete).
 - 2017/08/11 (1.51) - renamed ImGuiSetCond_*** types and flags to ImGuiCond_***. Kept redirection enums (will obsolete).
 - 2017/08/09 (1.51) - removed ValueColor() helpers, they are equivalent to calling Text(label) + SameLine() + ColorButton().
 - 2017/08/08 (1.51) - removed ColorEditMode() and ImGuiColorEditMode in favor of ImGuiColorEditFlags and parameters to the various Color*() functions. The SetColorEditOptions() allows to initialize default but the user can still change them with right-click context menu.
                     - changed prototype of 'ColorEdit4(const char* label, float col[4], bool show_alpha = true)' to 'ColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags = 0)', where passing flags = 0x01 is a safe no-op (hello dodgy backward compatibility!). - check and run the demo window, under "Color/Picker Widgets", to understand the various new options.
                     - changed prototype of rarely used 'ColorButton(ImVec4 col, bool small_height = false, bool outline_border = true)' to 'ColorButton(const char* desc_id, ImVec4 col, ImGuiColorEditFlags flags = 0, ImVec2 size = ImVec2(0,0))'
 - 2017/07/20 (1.51) - removed IsPosHoveringAnyWindow(ImVec2), which was partly broken and misleading. ASSERT + redirect user to io.WantCaptureMouse
 - 2017/05/26 (1.50) - removed ImFontConfig::MergeGlyphCenterV in favor of a more multipurpose ImFontConfig::GlyphOffset.
 - 2017/05/01 (1.50) - renamed ImDrawList::PathFill() (rarely used directly) to ImDrawList::PathFillConvex() for clarity.
 - 2016/11/06 (1.50) - BeginChild(const char*) now applies the stack id to the provided label, consistently with other functions as it should always have been. It shouldn't affect you unless (extremely unlikely) you were appending multiple times to a same child from different locations of the stack id. If that's the case, generate an id with GetId() and use it instead of passing string to BeginChild().
 - 2016/10/15 (1.50) - avoid 'void* user_data' parameter to io.SetClipboardTextFn/io.GetClipboardTextFn pointers. We pass io.ClipboardUserData to it.
 - 2016/09/25 (1.50) - style.WindowTitleAlign is now a ImVec2 (ImGuiAlign enum was removed). set to (0.5f,0.5f) for horizontal+vertical centering, (0.0f,0.0f) for upper-left, etc.
 - 2016/07/30 (1.50) - SameLine(x) with x>0.0f is now relative to left of column/group if any, and not always to left of window. This was sort of always the intent and hopefully breakage should be minimal.
 - 2016/05/12 (1.49) - title bar (using ImGuiCol_TitleBg/ImGuiCol_TitleBgActive colors) isn't rendered over a window background (ImGuiCol_WindowBg color) anymore. 
                       If your TitleBg/TitleBgActive alpha was 1.0f or you are using the default theme it will not affect you. 
                       However if your TitleBg/TitleBgActive alpha was <1.0f you need to tweak your custom theme to readjust for the fact that we don't draw a WindowBg background behind the title bar.
                       This helper function will convert an old TitleBg/TitleBgActive color into a new one with the same visual output, given the OLD color and the OLD WindowBg color.
                           ImVec4 ConvertTitleBgCol(const ImVec4& win_bg_col, const ImVec4& title_bg_col)
                           {
                               float new_a = 1.0f - ((1.0f - win_bg_col.w) * (1.0f - title_bg_col.w)), k = title_bg_col.w / new_a;
                               return ImVec4((win_bg_col.x * win_bg_col.w + title_bg_col.x) * k, (win_bg_col.y * win_bg_col.w + title_bg_col.y) * k, (win_bg_col.z * win_bg_col.w + title_bg_col.z) * k, new_a);
                           }
                       If this is confusing, pick the RGB value from title bar from an old screenshot and apply this as TitleBg/TitleBgActive. Or you may just create TitleBgActive from a tweaked TitleBg color.
 - 2016/05/07 (1.49) - removed confusing set of GetInternalState(), GetInternalStateSize(), SetInternalState() functions. Now using CreateContext(), DestroyContext(), GetCurrentContext(), SetCurrentContext().
 - 2016/05/02 (1.49) - renamed SetNextTreeNodeOpened() to SetNextTreeNodeOpen(), no redirection.
 - 2016/05/01 (1.49) - obsoleted old signature of CollapsingHeader(const char* label, const char* str_id = NULL, bool display_frame = true, bool default_open = false) as extra parameters were badly designed and rarely used. You can replace the "default_open = true" flag in new API with CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen).
 - 2016/04/26 (1.49) - changed ImDrawList::PushClipRect(ImVec4 rect) to ImDraw::PushClipRect(Imvec2 min,ImVec2 max,bool intersect_with_current_clip_rect=false). Note that higher-level ImGui::PushClipRect() is preferable because it will clip at logic/widget level, whereas ImDrawList::PushClipRect() only affect your renderer.
 - 2016/04/03 (1.48) - removed style.WindowFillAlphaDefault setting which was redundant. Bake default BG alpha inside style.Colors[ImGuiCol_WindowBg] and all other Bg color values. (ref github issue #337).
 - 2016/04/03 (1.48) - renamed ImGuiCol_TooltipBg to ImGuiCol_PopupBg, used by popups/menus and tooltips. popups/menus were previously using ImGuiCol_WindowBg. (ref github issue #337)
 - 2016/03/21 (1.48) - renamed GetWindowFont() to GetFont(), GetWindowFontSize() to GetFontSize(). Kept inline redirection function (will obsolete).
 - 2016/03/02 (1.48) - InputText() completion/history/always callbacks: if you modify the text buffer manually (without using DeleteChars()/InsertChars() helper) you need to maintain the BufTextLen field. added an assert.
 - 2016/01/23 (1.48) - fixed not honoring exact width passed to PushItemWidth(), previously it would add extra FramePadding.x*2 over that width. if you had manual pixel-perfect alignment in place it might affect you.
 - 2015/12/27 (1.48) - fixed ImDrawList::AddRect() which used to render a rectangle 1 px too large on each axis.
 - 2015/12/04 (1.47) - renamed Color() helpers to ValueColor() - dangerously named, rarely used and probably to be made obsolete.
 - 2015/08/29 (1.45) - with the addition of horizontal scrollbar we made various fixes to inconsistencies with dealing with cursor position.
                       GetCursorPos()/SetCursorPos() functions now include the scrolled amount. It shouldn't affect the majority of users, but take note that SetCursorPosX(100.0f) puts you at +100 from the starting x position which may include scrolling, not at +100 from the window left side.
                       GetContentRegionMax()/GetWindowContentRegionMin()/GetWindowContentRegionMax() functions allow include the scrolled amount. Typically those were used in cases where no scrolling would happen so it may not be a problem, but watch out!
 - 2015/08/29 (1.45) - renamed style.ScrollbarWidth to style.ScrollbarSize
 - 2015/08/05 (1.44) - split imgui.cpp into extra files: imgui_demo.cpp imgui_draw.cpp imgui_internal.h that you need to add to your project.
 - 2015/07/18 (1.44) - fixed angles in ImDrawList::PathArcTo(), PathArcToFast() (introduced in 1.43) being off by an extra PI for no justifiable reason
 - 2015/07/14 (1.43) - add new ImFontAtlas::AddFont() API. For the old AddFont***, moved the 'font_no' parameter of ImFontAtlas::AddFont** functions to the ImFontConfig structure.
                       you need to render your textured triangles with bilinear filtering to benefit from sub-pixel positioning of text.
 - 2015/07/08 (1.43) - switched rendering data to use indexed rendering. this is saving a fair amount of CPU/GPU and enables us to get anti-aliasing for a marginal cost.
                       this necessary change will break your rendering function! the fix should be very easy. sorry for that :(
                     - if you are using a vanilla copy of one of the imgui_impl_XXXX.cpp provided in the example, you just need to update your copy and you can ignore the rest.
                     - the signature of the io.RenderDrawListsFn handler has changed!
                            ImGui_XXXX_RenderDrawLists(ImDrawList** const cmd_lists, int cmd_lists_count)
                       became:
                            ImGui_XXXX_RenderDrawLists(ImDrawData* draw_data).
                              argument   'cmd_lists'        -> 'draw_data->CmdLists'
                              argument   'cmd_lists_count'  -> 'draw_data->CmdListsCount'
                              ImDrawList 'commands'         -> 'CmdBuffer'
                              ImDrawList 'vtx_buffer'       -> 'VtxBuffer'
                              ImDrawList  n/a               -> 'IdxBuffer' (new)
                              ImDrawCmd  'vtx_count'        -> 'ElemCount'
                              ImDrawCmd  'clip_rect'        -> 'ClipRect'
                              ImDrawCmd  'user_callback'    -> 'UserCallback'
                              ImDrawCmd  'texture_id'       -> 'TextureId'
                     - each ImDrawList now contains both a vertex buffer and an index buffer. For each command, render ElemCount/3 triangles using indices from the index buffer.
                     - if you REALLY cannot render indexed primitives, you can call the draw_data->DeIndexAllBuffers() method to de-index the buffers. This is slow and a waste of CPU/GPU. Prefer using indexed rendering!
                     - refer to code in the examples/ folder or ask on the GitHub if you are unsure of how to upgrade. please upgrade!
 - 2015/07/10 (1.43) - changed SameLine() parameters from int to float.
 - 2015/07/02 (1.42) - renamed SetScrollPosHere() to SetScrollFromCursorPos(). Kept inline redirection function (will obsolete).
 - 2015/07/02 (1.42) - renamed GetScrollPosY() to GetScrollY(). Necessary to reduce confusion along with other scrolling functions, because positions (e.g. cursor position) are not equivalent to scrolling amount.
 - 2015/06/14 (1.41) - changed ImageButton() default bg_col parameter from (0,0,0,1) (black) to (0,0,0,0) (transparent) - makes a difference when texture have transparence
 - 2015/06/14 (1.41) - changed Selectable() API from (label, selected, size) to (label, selected, flags, size). Size override should have been rarely be used. Sorry!
 - 2015/05/31 (1.40) - renamed GetWindowCollapsed() to IsWindowCollapsed() for consistency. Kept inline redirection function (will obsolete).
 - 2015/05/31 (1.40) - renamed IsRectClipped() to IsRectVisible() for consistency. Note that return value is opposite! Kept inline redirection function (will obsolete).
 - 2015/05/27 (1.40) - removed the third 'repeat_if_held' parameter from Button() - sorry! it was rarely used and inconsistent. Use PushButtonRepeat(true) / PopButtonRepeat() to enable repeat on desired buttons.
 - 2015/05/11 (1.40) - changed BeginPopup() API, takes a string identifier instead of a bool. ImGui needs to manage the open/closed state of popups. Call OpenPopup() to actually set the "open" state of a popup. BeginPopup() returns true if the popup is opened.
 - 2015/05/03 (1.40) - removed style.AutoFitPadding, using style.WindowPadding makes more sense (the default values were already the same).
 - 2015/04/13 (1.38) - renamed IsClipped() to IsRectClipped(). Kept inline redirection function until 1.50.
 - 2015/04/09 (1.38) - renamed ImDrawList::AddArc() to ImDrawList::AddArcFast() for compatibility with future API
 - 2015/04/03 (1.38) - removed ImGuiCol_CheckHovered, ImGuiCol_CheckActive, replaced with the more general ImGuiCol_FrameBgHovered, ImGuiCol_FrameBgActive.
 - 2014/04/03 (1.38) - removed support for passing -FLT_MAX..+FLT_MAX as the range for a SliderFloat(). Use DragFloat() or Inputfloat() instead.
 - 2015/03/17 (1.36) - renamed GetItemBoxMin()/GetItemBoxMax()/IsMouseHoveringBox() to GetItemRectMin()/GetItemRectMax()/IsMouseHoveringRect(). Kept inline redirection function until 1.50.
 - 2015/03/15 (1.36) - renamed style.TreeNodeSpacing to style.IndentSpacing, ImGuiStyleVar_TreeNodeSpacing to ImGuiStyleVar_IndentSpacing
 - 2015/03/13 (1.36) - renamed GetWindowIsFocused() to IsWindowFocused(). Kept inline redirection function until 1.50.
 - 2015/03/08 (1.35) - renamed style.ScrollBarWidth to style.ScrollbarWidth (casing)
 - 2015/02/27 (1.34) - renamed OpenNextNode(bool) to SetNextTreeNodeOpened(bool, ImGuiSetCond). Kept inline redirection function until 1.50.
 - 2015/02/27 (1.34) - renamed ImGuiSetCondition_*** to ImGuiSetCond_***, and _FirstUseThisSession becomes _Once.
 - 2015/02/11 (1.32) - changed text input callback ImGuiTextEditCallback return type from void-->int. reserved for future use, return 0 for now.
 - 2015/02/10 (1.32) - renamed GetItemWidth() to CalcItemWidth() to clarify its evolving behavior
 - 2015/02/08 (1.31) - renamed GetTextLineSpacing() to GetTextLineHeightWithSpacing()
 - 2015/02/01 (1.31) - removed IO.MemReallocFn (unused)
 - 2015/01/19 (1.30) - renamed ImGuiStorage::GetIntPtr()/GetFloatPtr() to GetIntRef()/GetIntRef() because Ptr was conflicting with actual pointer storage functions.
 - 2015/01/11 (1.30) - big font/image API change! now loads TTF file. allow for multiple fonts. no need for a PNG loader.
              (1.30) - removed GetDefaultFontData(). uses io.Fonts->GetTextureData*() API to retrieve uncompressed pixels.
                       this sequence:
                           const void* png_data;
                           unsigned int png_size;
                           ImGui::GetDefaultFontData(NULL, NULL, &png_data, &png_size);
                           // <Copy to GPU>
                       became:
                           unsigned char* pixels;
                           int width, height;
                           io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
                           // <Copy to GPU>
                           io.Fonts->TexID = (your_texture_identifier);
                       you now have much more flexibility to load multiple TTF fonts and manage the texture buffer for internal needs.
                       it is now recommended that you sample the font texture with bilinear interpolation.
              (1.30) - added texture identifier in ImDrawCmd passed to your render function (we can now render images). make sure to set io.Fonts->TexID.
              (1.30) - removed IO.PixelCenterOffset (unnecessary, can be handled in user projection matrix)
              (1.30) - removed ImGui::IsItemFocused() in favor of ImGui::IsItemActive() which handles all widgets
 - 2014/12/10 (1.18) - removed SetNewWindowDefaultPos() in favor of new generic API SetNextWindowPos(pos, ImGuiSetCondition_FirstUseEver)
 - 2014/11/28 (1.17) - moved IO.Font*** options to inside the IO.Font-> structure (FontYOffset, FontTexUvForWhite, FontBaseScale, FontFallbackGlyph)
 - 2014/11/26 (1.17) - reworked syntax of IMGUI_ONCE_UPON_A_FRAME helper macro to increase compiler compatibility
 - 2014/11/07 (1.15) - renamed IsHovered() to IsItemHovered()
 - 2014/10/02 (1.14) - renamed IMGUI_INCLUDE_IMGUI_USER_CPP to IMGUI_INCLUDE_IMGUI_USER_INL and imgui_user.cpp to imgui_user.inl (more IDE friendly)
 - 2014/09/25 (1.13) - removed 'text_end' parameter from IO.SetClipboardTextFn (the string is now always zero-terminated for simplicity)
 - 2014/09/24 (1.12) - renamed SetFontScale() to SetWindowFontScale()
 - 2014/09/24 (1.12) - moved IM_MALLOC/IM_REALLOC/IM_FREE preprocessor defines to IO.MemAllocFn/IO.MemReallocFn/IO.MemFreeFn
 - 2014/08/30 (1.09) - removed IO.FontHeight (now computed automatically)
 - 2014/08/30 (1.09) - moved IMGUI_FONT_TEX_UV_FOR_WHITE preprocessor define to IO.FontTexUvForWhite
 - 2014/08/28 (1.09) - changed the behavior of IO.PixelCenterOffset following various rendering fixes


 ISSUES & TODO-LIST
 ==================
 See TODO.txt


 FREQUENTLY ASKED QUESTIONS (FAQ), TIPS
 ======================================

 Q: How can I help?
 A: - If you are experienced enough with Dear ImGui and with C/C++, look at the todo list and see how you want/can help!
    - Become a Patron/donate! Convince your company to become a Patron or provide serious funding for development time! See http://www.patreon.com/imgui

 Q: What is ImTextureID and how do I display an image?
 A: ImTextureID is a void* used to pass renderer-agnostic texture references around until it hits your render function.
    Dear ImGui knows nothing about what those bits represent, it just passes them around. It is up to you to decide what you want the void* to carry!
    It could be an identifier to your OpenGL texture (cast GLuint to void*), a pointer to your custom engine material (cast MyMaterial* to void*), etc.
    At the end of the chain, your renderer takes this void* to cast it back into whatever it needs to select a current texture to render.
    Refer to examples applications, where each renderer (in a imgui_impl_xxxx.cpp file) is treating ImTextureID as a different thing.
    (c++ tip: OpenGL uses integers to identify textures. You can safely store an integer into a void*, just cast it to void*, don't take it's address!)
    To display a custom image/texture within an ImGui window, you may use ImGui::Image(), ImGui::ImageButton(), ImDrawList::AddImage() functions.
    Dear ImGui will generate the geometry and draw calls using the ImTextureID that you passed and which your renderer can use.
    It is your responsibility to get textures uploaded to your GPU.

 Q: I integrated Dear ImGui in my engine and the text or lines are blurry..
 A: In your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f).
    Also make sure your orthographic projection matrix and io.DisplaySize matches your actual framebuffer dimension.

 Q: I integrated Dear ImGui in my engine and some elements are clipping or disappearing when I move windows around..
 A: You are probably mishandling the clipping rectangles in your render function. 
    Rectangles provided by ImGui are defined as (x1=left,y1=top,x2=right,y2=bottom) and NOT as (x1,y1,width,height).

 Q: Can I have multiple widgets with the same label? Can I have widget without a label?
 A: Yes. A primer on the use of labels/IDs in Dear ImGui..

   - Elements that are not clickable, such as Text() items don't need an ID.

   - Interactive widgets require state to be carried over multiple frames (most typically Dear ImGui often needs to remember what is 
     the "active" widget). to do so they need a unique ID. unique ID are typically derived from a string label, an integer index or a pointer.

       Button("OK");        // Label = "OK",     ID = hash of "OK"
       Button("Cancel");    // Label = "Cancel", ID = hash of "Cancel"

   - ID are uniquely scoped within windows, tree nodes, etc. so no conflict can happen if you have two buttons called "OK"
     in two different windows or in two different locations of a tree.

   - If you have a same ID twice in the same location, you'll have a conflict:

       Button("OK");
       Button("OK");           // ID collision! Both buttons will be treated as the same.

     Fear not! this is easy to solve and there are many ways to solve it!

   - When passing a label you can optionally specify extra unique ID information within string itself. 
     This helps solving the simpler collision cases. Use "##" to pass a complement to the ID that won't be visible to the end-user:

       Button("Play");         // Label = "Play",   ID = hash of "Play"
       Button("Play##foo1");   // Label = "Play",   ID = hash of "Play##foo1" (different from above)
       Button("Play##foo2");   // Label = "Play",   ID = hash of "Play##foo2" (different from above)

   - If you want to completely hide the label, but still need an ID:

       Checkbox("##On", &b);   // Label = "",       ID = hash of "##On" (no label!)

   - Occasionally/rarely you might want change a label while preserving a constant ID. This allows you to animate labels.
     For example you may want to include varying information in a window title bar (and windows are uniquely identified by their ID.. obviously)
     Use "###" to pass a label that isn't part of ID:

       Button("Hello###ID";   // Label = "Hello",  ID = hash of "ID"
       Button("World###ID";   // Label = "World",  ID = hash of "ID" (same as above)

       sprintf(buf, "My game (%f FPS)###MyGame");
       Begin(buf);            // Variable label,   ID = hash of "MyGame"

   - Use PushID() / PopID() to create scopes and avoid ID conflicts within the same Window.
     This is the most convenient way of distinguishing ID if you are iterating and creating many UI elements.
     You can push a pointer, a string or an integer value. Remember that ID are formed from the concatenation of everything in the ID stack!

       for (int i = 0; i < 100; i++)
       {
         PushID(i);
         Button("Click");   // Label = "Click",  ID = hash of integer + "label" (unique)
         PopID();
       }

       for (int i = 0; i < 100; i++)
       {
         MyObject* obj = Objects[i];
         PushID(obj);
         Button("Click");   // Label = "Click",  ID = hash of pointer + "label" (unique)
         PopID();
       }

       for (int i = 0; i < 100; i++)
       {
         MyObject* obj = Objects[i];
         PushID(obj->Name);
         Button("Click");   // Label = "Click",  ID = hash of string + "label" (unique)
         PopID();
       }

   - More example showing that you can stack multiple prefixes into the ID stack:

       Button("Click");     // Label = "Click",  ID = hash of "Click"
       PushID("node");
       Button("Click");     // Label = "Click",  ID = hash of "node" + "Click"
         PushID(my_ptr);
           Button("Click"); // Label = "Click",  ID = hash of "node" + ptr + "Click"
         PopID();
       PopID();

   - Tree nodes implicitly creates a scope for you by calling PushID().

       Button("Click");     // Label = "Click",  ID = hash of "Click"
       if (TreeNode("node"))
       {
         Button("Click");   // Label = "Click",  ID = hash of "node" + "Click"
         TreePop();
       }

   - When working with trees, ID are used to preserve the open/close state of each tree node.
     Depending on your use cases you may want to use strings, indices or pointers as ID.
      e.g. when displaying a single object that may change over time (dynamic 1-1 relationship), using a static string as ID will preserve your
       node open/closed state when the targeted object change.
      e.g. when displaying a list of objects, using indices or pointers as ID will preserve the node open/closed state differently. 
       experiment and see what makes more sense!

 Q: How can I tell when Dear ImGui wants my mouse/keyboard inputs VS when I can pass them to my application?
 A: You can read the 'io.WantCaptureMouse'/'io.WantCaptureKeyboard'/'ioWantTextInput' flags from the ImGuiIO structure. 
    - When 'io.WantCaptureMouse' or 'io.WantCaptureKeyboard' flags are set you may want to discard/hide the inputs from the rest of your application.
    - When 'io.WantTextInput' is set to may want to notify your OS to popup an on-screen keyboard, if available (e.g. on a mobile phone, or console OS).
    Preferably read the flags after calling ImGui::NewFrame() to avoid them lagging by one frame. But reading those flags before calling NewFrame() is
    also generally ok, as the bool toggles fairly rarely and you don't generally expect to interact with either Dear ImGui or your application during
    the same frame when that transition occurs. Dear ImGui is tracking dragging and widget activity that may occur outside the boundary of a window, 
    so 'io.WantCaptureMouse' is more accurate and correct than checking if a window is hovered. 
    (Advanced note: text input releases focus on Return 'KeyDown', so the following Return 'KeyUp' event that your application receive will typically 
     have 'io.WantCaptureKeyboard=false'. Depending on your application logic it may or not be inconvenient. You might want to track which key-downs
     were for Dear ImGui, e.g. with an array of bool, and filter out the corresponding key-ups.)

 Q: How can I load a different font than the default? (default is an embedded version of ProggyClean.ttf, rendered at size 13)
 A: Use the font atlas to load the TTF/OTF file you want:

      ImGuiIO& io = ImGui::GetIO();
      io.Fonts->AddFontFromFileTTF("myfontfile.ttf", size_in_pixels);
      io.Fonts->GetTexDataAsRGBA32() or GetTexDataAsAlpha8()

 Q: How can I easily use icons in my application?
 A: The most convenient and practical way is to merge an icon font such as FontAwesome inside you main font. Then you can refer to icons within your 
    strings. Read 'How can I load multiple fonts?' and the file 'extra_fonts/README.txt' for instructions and useful header files.

 Q: How can I load multiple fonts?
 A: Use the font atlas to pack them into a single texture:
    (Read extra_fonts/README.txt and the code in ImFontAtlas for more details.)

      ImGuiIO& io = ImGui::GetIO();
      ImFont* font0 = io.Fonts->AddFontDefault();
      ImFont* font1 = io.Fonts->AddFontFromFileTTF("myfontfile.ttf", size_in_pixels);
      ImFont* font2 = io.Fonts->AddFontFromFileTTF("myfontfile2.ttf", size_in_pixels);
      io.Fonts->GetTexDataAsRGBA32() or GetTexDataAsAlpha8()
      // the first loaded font gets used by default
      // use ImGui::PushFont()/ImGui::PopFont() to change the font at runtime

      // Options
      ImFontConfig config;
      config.OversampleH = 3;
      config.OversampleV = 1;
      config.GlyphOffset.y -= 2.0f;      // Move everything by 2 pixels up
      config.GlyphExtraSpacing.x = 1.0f; // Increase spacing between characters
      io.Fonts->LoadFromFileTTF("myfontfile.ttf", size_pixels, &config);

      // Combine multiple fonts into one (e.g. for icon fonts)
      ImWchar ranges[] = { 0xf000, 0xf3ff, 0 };
      ImFontConfig config;
      config.MergeMode = true;
      io.Fonts->AddFontDefault();
      io.Fonts->LoadFromFileTTF("fontawesome-webfont.ttf", 16.0f, &config, ranges); // Merge icon font
      io.Fonts->LoadFromFileTTF("myfontfile.ttf", size_pixels, NULL, &config, io.Fonts->GetGlyphRangesJapanese()); // Merge japanese glyphs

 Q: How can I display and input non-Latin characters such as Chinese, Japanese, Korean, Cyrillic?
 A: When loading a font, pass custom Unicode ranges to specify the glyphs to load. 

      // Add default Japanese ranges
      io.Fonts->AddFontFromFileTTF("myfontfile.ttf", size_in_pixels, NULL, io.Fonts->GetGlyphRangesJapanese());
   
      // Or create your own custom ranges (e.g. for a game you can feed your entire game script and only build the characters the game need)
      ImVector<ImWchar> ranges;
      ImFontAtlas::GlyphRangesBuilder builder;
      builder.AddText("Hello world");                        // Add a string (here "Hello world" contains 7 unique characters)
      builder.AddChar(0x7262);                               // Add a specific character
      builder.AddRanges(io.Fonts->GetGlyphRangesJapanese()); // Add one of the default ranges
      builder.BuildRanges(&ranges);                          // Build the final result (ordered ranges with all the unique characters submitted)
      io.Fonts->AddFontFromFileTTF("myfontfile.ttf", size_in_pixels, NULL, ranges.Data);

    All your strings needs to use UTF-8 encoding. In C++11 you can encode a string literal in UTF-8 by using the u8"hello" syntax. 
    Specifying literal in your source code using a local code page (such as CP-923 for Japanese or CP-1251 for Cyrillic) will NOT work!
    Otherwise you can convert yourself to UTF-8 or load text data from file already saved as UTF-8.

    Text input: it is up to your application to pass the right character code to io.AddInputCharacter(). The applications in examples/ are doing that.
    For languages using IME, on Windows you can copy the Hwnd of your application to io.ImeWindowHandle.
    The default implementation of io.ImeSetInputScreenPosFn() on Windows will set your IME position correctly.

 Q: How can I preserve my Dear ImGui context across reloading a DLL? (loss of the global/static variables)
 A: Create your own context 'ctx = CreateContext()' + 'SetCurrentContext(ctx)' and your own font atlas 'ctx->GetIO().Fonts = new ImFontAtlas()' 
    so you don't rely on the default globals.

 Q: How can I use the drawing facilities without an ImGui window? (using ImDrawList API)
 A: The easiest way is to create a dummy window. Call Begin() with NoTitleBar|NoResize|NoMove|NoScrollbar|NoSavedSettings|NoInputs flag, 
    zero background alpha, then retrieve the ImDrawList* via GetWindowDrawList() and draw to it in any way you like.
    You can also perfectly create a standalone ImDrawList instance _but_ you need ImGui to be initialized because ImDrawList pulls from ImGui 
    data to retrieve the coordinates of the white pixel.

 - tip: you can call Begin() multiple times with the same name during the same frame, it will keep appending to the same window. 
   this is also useful to set yourself in the context of another window (to get/set other settings)
 - tip: you can create widgets without a Begin()/End() block, they will go in an implicit window called "Debug".
 - tip: the ImGuiOnceUponAFrame helper will allow run the block of code only once a frame. You can use it to quickly add custom UI in the middle
   of a deep nested inner loop in your code.
 - tip: you can call Render() multiple times (e.g for VR renders).
 - tip: call and read the ShowTestWindow() code in imgui_demo.cpp for more example of how to use ImGui!

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_PLACEMENT_NEW
#include "imgui_internal.h"

#include <ctype.h>      // toupper, isprint
#include <stdlib.h>     // NULL, malloc, free, qsort, atoi
#include <stdio.h>      // vsnprintf, sscanf, printf
#include <limits.h>     // INT_MIN, INT_MAX
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>     // intptr_t
#else
#include <stdint.h>     // intptr_t
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4127) // condition expression is constant
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning (disable: 4305) // condition expression is constant
#endif

// Clang warnings with -Weverything
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunknown-pragmas"        // warning : unknown warning group '-Wformat-pedantic *'        // not all warnings are known by all clang versions.. so ignoring warnings triggers new warnings on some configuration. great!
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants (typically 0.0f) is ok.
#pragma clang diagnostic ignored "-Wformat-nonliteral"      // warning : format string is not a string literal              // passing non-literal to vsnformat(). yes, user passing incorrect format strings can crash the code.
#pragma clang diagnostic ignored "-Wexit-time-destructors"  // warning : declaration requires an exit-time destructor       // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. ImGui coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning : declaration requires a global destructor           // similar to above, not sure what the exact difference it.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#pragma clang diagnostic ignored "-Wformat-pedantic"        // warning : format specifies type 'void *' but the argument has type 'xxxx *' // unreasonable, would lead to casting every %p arg to void*. probably enabled by -pedantic. 
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast" // warning : cast to 'void *' from smaller integer type 'int' //
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"      // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wformat"                   // warning: format '%p' expects argument of type 'void*', but argument 6 has type 'ImGuiWindow*'
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'xxxx' to type 'xxxx' casts away qualifiers
#pragma GCC diagnostic ignored "-Wformat-nonliteral"        // warning: format not a string literal, format string not checked
#endif

//-------------------------------------------------------------------------
// Forward Declarations
//-------------------------------------------------------------------------

static float            GetDraggedColumnOffset(int column_index);

static bool             IsKeyPressedMap(ImGuiKey key, bool repeat = true);

static ImFont*          GetDefaultFont();
static void             SetCurrentFont(ImFont* font);
static void             SetCurrentWindow(ImGuiWindow* window);
static void             SetWindowScrollY(ImGuiWindow* window, float new_scroll_y);
static void             SetWindowPos(ImGuiWindow* window, const ImVec2& pos, ImGuiCond cond);
static void             SetWindowSize(ImGuiWindow* window, const ImVec2& size, ImGuiCond cond);
static void             SetWindowCollapsed(ImGuiWindow* window, bool collapsed, ImGuiCond cond);
static ImGuiWindow*     FindHoveredWindow(ImVec2 pos, bool excluding_childs);
static ImGuiWindow*     CreateNewWindow(const char* name, ImVec2 size, ImGuiWindowFlags flags);
static void             ClearSetNextWindowData();
static void             CheckStacksSize(ImGuiWindow* window, bool write);
static ImVec2           CalcNextScrollFromScrollTargetAndClamp(ImGuiWindow* window);

static void             AddDrawListToRenderList(ImVector<ImDrawList*>& out_render_list, ImDrawList* draw_list);
static void             AddWindowToRenderList(ImVector<ImDrawList*>& out_render_list, ImGuiWindow* window);
static void             AddWindowToSortedBuffer(ImVector<ImGuiWindow*>& out_sorted_windows, ImGuiWindow* window);

static ImGuiIniData*    FindWindowSettings(const char* name);
static ImGuiIniData*    AddWindowSettings(const char* name);
static void             LoadIniSettingsFromDisk(const char* ini_filename);
static void             SaveIniSettingsToDisk(const char* ini_filename);
static void             MarkIniSettingsDirty(ImGuiWindow* window);

static ImRect           GetVisibleRect();

static void             CloseInactivePopups();
static void             ClosePopupToLevel(int remaining);
static ImGuiWindow*     GetFrontMostModalRootWindow();
static ImVec2           FindBestPopupWindowPos(const ImVec2& base_pos, const ImVec2& size, int* last_dir, const ImRect& rect_to_avoid);

static bool             InputTextFilterCharacter(unsigned int* p_char, ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* user_data);
static int              InputTextCalcTextLenAndLineCount(const char* text_begin, const char** out_text_end);
static ImVec2           InputTextCalcTextSizeW(const ImWchar* text_begin, const ImWchar* text_end, const ImWchar** remaining = NULL, ImVec2* out_offset = NULL, bool stop_on_new_line = false);

static inline void      DataTypeFormatString(ImGuiDataType data_type, void* data_ptr, const char* display_format, char* buf, int buf_size);
static inline void      DataTypeFormatString(ImGuiDataType data_type, void* data_ptr, int decimal_precision, char* buf, int buf_size);
static void             DataTypeApplyOp(ImGuiDataType data_type, int op, void* value1, const void* value2);
static bool             DataTypeApplyOpFromText(const char* buf, const char* initial_value_buf, ImGuiDataType data_type, void* data_ptr, const char* scalar_format);

//-----------------------------------------------------------------------------
// Platform dependent default implementations
//-----------------------------------------------------------------------------

static const char*      GetClipboardTextFn_DefaultImpl(void* user_data);
static void             SetClipboardTextFn_DefaultImpl(void* user_data, const char* text);
static void             ImeSetInputScreenPosFn_DefaultImpl(int x, int y);

//-----------------------------------------------------------------------------
// Context
//-----------------------------------------------------------------------------

// Default font atlas storage.
// New contexts always point by default to this font atlas. It can be changed by reassigning the GetIO().Fonts variable.
static ImFontAtlas      GImDefaultFontAtlas;

// Default context storage + current context pointer.
// Implicitely used by all ImGui functions. Always assumed to be != NULL. Change to a different context by calling ImGui::SetCurrentContext()
// If you are hot-reloading this code in a DLL you will lose the static/global variables. Create your own context+font atlas instead of relying on those default (see FAQ entry "How can I preserve my ImGui context across reloading a DLL?").
// ImGui is currently not thread-safe because of this variable. If you want thread-safety to allow N threads to access N different contexts, you might work around it by:
// - Having multiple instances of the ImGui code compiled inside different namespace (easiest/safest, if you have a finite number of contexts)
// - or: Changing this variable to be TLS. You may #define GImGui in imconfig.h for further custom hackery. Future development aim to make this context pointer explicit to all calls. Also read https://github.com/ocornut/imgui/issues/586
#ifndef GImGui
static ImGuiContext     GImDefaultContext;
ImGuiContext*           GImGui = &GImDefaultContext;
#endif

//-----------------------------------------------------------------------------
// User facing structures
//-----------------------------------------------------------------------------

ImGuiStyle::ImGuiStyle()
{
    Alpha                   = 1.0f;             // Global alpha applies to everything in ImGui
    WindowPadding           = ImVec2(8,8);      // Padding within a window
    WindowMinSize           = ImVec2(32,32);    // Minimum window size
    WindowRounding          = 9.0f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows
    WindowTitleAlign        = ImVec2(0.0f,0.5f);// Alignment for title bar text
    ChildWindowRounding     = 0.0f;             // Radius of child window corners rounding. Set to 0.0f to have rectangular child windows
    FramePadding            = ImVec2(4,3);      // Padding within a framed rectangle (used by most widgets)
    FrameRounding           = 0.0f;             // Radius of frame corners rounding. Set to 0.0f to have rectangular frames (used by most widgets).
    ItemSpacing             = ImVec2(8,4);      // Horizontal and vertical spacing between widgets/lines
    ItemInnerSpacing        = ImVec2(4,4);      // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
    TouchExtraPadding       = ImVec2(0,0);      // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
    IndentSpacing           = 21.0f;            // Horizontal spacing when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
    ColumnsMinSpacing       = 6.0f;             // Minimum horizontal spacing between two columns
    ScrollbarSize           = 16.0f;            // Width of the vertical scrollbar, Height of the horizontal scrollbar
    ScrollbarRounding       = 9.0f;             // Radius of grab corners rounding for scrollbar
    GrabMinSize             = 10.0f;            // Minimum width/height of a grab box for slider/scrollbar
    GrabRounding            = 0.0f;             // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
    ButtonTextAlign         = ImVec2(0.5f,0.5f);// Alignment of button text when button is larger than text.
    DisplayWindowPadding    = ImVec2(22,22);    // Window positions are clamped to be visible within the display area by at least this amount. Only covers regular windows.
    DisplaySafeAreaPadding  = ImVec2(4,4);      // If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
    AntiAliasedLines        = true;             // Enable anti-aliasing on lines/borders. Disable if you are really short on CPU/GPU.
    AntiAliasedShapes       = true;             // Enable anti-aliasing on filled shapes (rounded rectangles, circles, etc.)
    CurveTessellationTol    = 1.25f;            // Tessellation tolerance. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality
	WindowPadThickness		= 0.f;				// 

    ImGui::StyleColorsClassic(this);
}

void ImGui::StyleColorsClassic(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_ChildWindowBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
    colors[ImGuiCol_Border]                 = ImVec4(0.70f, 0.70f, 0.70f, 0.40f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.80f, 0.80f, 0.80f, 0.30f);   // Background of checkbox, radio button, plot, slider, text input
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.27f, 0.27f, 0.54f, 0.83f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.80f, 0.50f, 0.50f, 0.40f);
    colors[ImGuiCol_ComboBg]                = ImVec4(0.20f, 0.20f, 0.20f, 0.99f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.65f, 0.65f, 0.65f, 0.30f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.67f, 0.40f, 0.40f, 0.60f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.67f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
    colors[ImGuiCol_Separator]              = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    colors[ImGuiCol_CloseButton]            = ImVec4(0.50f, 0.50f, 0.90f, 0.50f);
    colors[ImGuiCol_CloseButtonHovered]     = ImVec4(0.70f, 0.70f, 0.90f, 0.60f);
    colors[ImGuiCol_CloseButtonActive]      = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    colors[ImGuiCol_ModalWindowDarkening]   = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

// To scale your entire UI (e.g. if you want your app to use High DPI or generally be DPI aware) you may use this helper function. Scaling the fonts is done separately and is up to you.
// Tips: if you need to change your scale multiple times, prefer calling this on a freshly initialized ImGuiStyle structure rather than scaling multiple times (because floating point multiplications are lossy).
void ImGuiStyle::ScaleAllSizes(float scale_factor)
{
    WindowPadding *= scale_factor;
    WindowMinSize *= scale_factor;
    WindowRounding *= scale_factor;
    ChildWindowRounding *= scale_factor;
    FramePadding *= scale_factor;
    FrameRounding *= scale_factor;
    ItemSpacing *= scale_factor;
    ItemInnerSpacing *= scale_factor;
    TouchExtraPadding *= scale_factor;
    IndentSpacing *= scale_factor;
    ColumnsMinSpacing *= scale_factor;
    ScrollbarSize *= scale_factor;
    ScrollbarRounding *= scale_factor;
    GrabMinSize *= scale_factor;
    GrabRounding *= scale_factor;
    DisplayWindowPadding *= scale_factor;
    DisplaySafeAreaPadding *= scale_factor;
}

ImGuiIO::ImGuiIO()
{
    // Most fields are initialized with zero
    memset(this, 0, sizeof(*this));

    // Settings
    DisplaySize = ImVec2(-1.0f, -1.0f);
    DeltaTime = 1.0f/60.0f;
    IniSavingRate = 5.0f;
    IniFilename = "imgui.ini";
    LogFilename = "imgui_log.txt";
    MouseDoubleClickTime = 0.30f;
    MouseDoubleClickMaxDist = 6.0f;
    for (int i = 0; i < ImGuiKey_COUNT; i++)
        KeyMap[i] = -1;
    KeyRepeatDelay = 0.250f;
    KeyRepeatRate = 0.050f;
    UserData = NULL;

    Fonts = &GImDefaultFontAtlas;
    FontGlobalScale = 1.0f;
    FontDefault = NULL;
    FontAllowUserScaling = false;
    DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    DisplayVisibleMin = DisplayVisibleMax = ImVec2(0.0f, 0.0f);

    // User functions
    RenderDrawListsFn = NULL;
    MemAllocFn = malloc;
    MemFreeFn = free;
    GetClipboardTextFn = GetClipboardTextFn_DefaultImpl;   // Platform dependent default implementations
    SetClipboardTextFn = SetClipboardTextFn_DefaultImpl;
    ClipboardUserData = NULL;
    ImeSetInputScreenPosFn = ImeSetInputScreenPosFn_DefaultImpl;
    ImeWindowHandle = NULL;

    // Input (NB: we already have memset zero the entire structure)
    MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    MousePosPrev = ImVec2(-FLT_MAX, -FLT_MAX);
    MouseDragThreshold = 6.0f;
    for (int i = 0; i < IM_ARRAYSIZE(MouseDownDuration); i++) MouseDownDuration[i] = MouseDownDurationPrev[i] = -1.0f;
    for (int i = 0; i < IM_ARRAYSIZE(KeysDownDuration); i++) KeysDownDuration[i] = KeysDownDurationPrev[i] = -1.0f;

    // Set OS X style defaults based on __APPLE__ compile time flag
#ifdef __APPLE__
    OSXBehaviors = true;
#endif
}

// Pass in translated ASCII characters for text input.
// - with glfw you can get those from the callback set in glfwSetCharCallback()
// - on Windows you can get those using ToAscii+keyboard state, or via the WM_CHAR message
void ImGuiIO::AddInputCharacter(ImWchar c)
{
    const int n = ImStrlenW(InputCharacters);
    if (n + 1 < IM_ARRAYSIZE(InputCharacters))
    {
        InputCharacters[n] = c;
        InputCharacters[n+1] = '\0';
    }
}

void ImGuiIO::AddInputCharactersUTF8(const char* utf8_chars)
{
    // We can't pass more wchars than ImGuiIO::InputCharacters[] can hold so don't convert more
    const int wchars_buf_len = sizeof(ImGuiIO::InputCharacters) / sizeof(ImWchar);
    ImWchar wchars[wchars_buf_len];
    ImTextStrFromUtf8(wchars, wchars_buf_len, utf8_chars, NULL);
    for (int i = 0; i < wchars_buf_len && wchars[i] != 0; i++)
        AddInputCharacter(wchars[i]);
}

//-----------------------------------------------------------------------------
// HELPERS
//-----------------------------------------------------------------------------

#define IM_F32_TO_INT8_UNBOUND(_VAL)    ((int)((_VAL) * 255.0f + ((_VAL)>=0 ? 0.5f : -0.5f)))   // Unsaturated, for display purpose 
#define IM_F32_TO_INT8_SAT(_VAL)        ((int)(ImSaturate(_VAL) * 255.0f + 0.5f))               // Saturated, always output 0..255

// Play it nice with Windows users. Notepad in 2015 still doesn't display text data with Unix-style \n.
#ifdef _WIN32
#define IM_NEWLINE "\r\n"
#else
#define IM_NEWLINE "\n"
#endif

ImVec2 ImLineClosestPoint(const ImVec2& a, const ImVec2& b, const ImVec2& p)
{
    ImVec2 ap = p - a;
    ImVec2 ab_dir = b - a;
    float ab_len = sqrtf(ab_dir.x * ab_dir.x + ab_dir.y * ab_dir.y);
    ab_dir *= 1.0f / ab_len;
    float dot = ap.x * ab_dir.x + ap.y * ab_dir.y;
    if (dot < 0.0f)
        return a;
    if (dot > ab_len)
        return b;
    return a + ab_dir * dot;
}

bool ImTriangleContainsPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p)
{
    bool b1 = ((p.x - b.x) * (a.y - b.y) - (p.y - b.y) * (a.x - b.x)) < 0.0f;
    bool b2 = ((p.x - c.x) * (b.y - c.y) - (p.y - c.y) * (b.x - c.x)) < 0.0f;
    bool b3 = ((p.x - a.x) * (c.y - a.y) - (p.y - a.y) * (c.x - a.x)) < 0.0f;
    return ((b1 == b2) && (b2 == b3));
}

void ImTriangleBarycentricCoords(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p, float& out_u, float& out_v, float& out_w)
{
    ImVec2 v0 = b - a;
    ImVec2 v1 = c - a;
    ImVec2 v2 = p - a;
    const float denom = v0.x * v1.y - v1.x * v0.y;
    out_v = (v2.x * v1.y - v1.x * v2.y) / denom;
    out_w = (v0.x * v2.y - v2.x * v0.y) / denom;
    out_u = 1.0f - out_v - out_w;
}

ImVec2 ImTriangleClosestPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p)
{
    ImVec2 proj_ab = ImLineClosestPoint(a, b, p);
    ImVec2 proj_bc = ImLineClosestPoint(b, c, p);
    ImVec2 proj_ca = ImLineClosestPoint(c, a, p);
    float dist2_ab = ImLengthSqr(p - proj_ab);
    float dist2_bc = ImLengthSqr(p - proj_bc);
    float dist2_ca = ImLengthSqr(p - proj_ca);
    float m = ImMin(dist2_ab, ImMin(dist2_bc, dist2_ca));
    if (m == dist2_ab)
        return proj_ab;
    if (m == dist2_bc)
        return proj_bc;
    return proj_ca;
}

int ImStricmp(const char* str1, const char* str2)
{
    int d;
    while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; }
    return d;
}

int ImStrnicmp(const char* str1, const char* str2, int count)
{
    int d = 0;
    while (count > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; count--; }
    return d;
}

void ImStrncpy(char* dst, const char* src, int count)
{
    if (count < 1) return;
    strncpy(dst, src, (size_t)count);
    dst[count-1] = 0;
}

char* ImStrdup(const char *str)
{
    size_t len = strlen(str) + 1;
    void* buff = ImGui::MemAlloc(len);
    return (char*)memcpy(buff, (const void*)str, len);
}

int ImStrlenW(const ImWchar* str)
{
    int n = 0;
    while (*str++) n++;
    return n;
}

const ImWchar* ImStrbolW(const ImWchar* buf_mid_line, const ImWchar* buf_begin) // find beginning-of-line
{
    while (buf_mid_line > buf_begin && buf_mid_line[-1] != '\n')
        buf_mid_line--;
    return buf_mid_line;
}

const char* ImStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end)
{
    if (!needle_end)
        needle_end = needle + strlen(needle);

    const char un0 = (char)toupper(*needle);
    while ((!haystack_end && *haystack) || (haystack_end && haystack < haystack_end))
    {
        if (toupper(*haystack) == un0)
        {
            const char* b = needle + 1;
            for (const char* a = haystack + 1; b < needle_end; a++, b++)
                if (toupper(*a) != toupper(*b))
                    break;
            if (b == needle_end)
                return haystack;
        }
        haystack++;
    }
    return NULL;
}

static const char* ImAtoi(const char* src, int* output)
{
    int negative = 0;
    if (*src == '-') { negative = 1; src++; }
    if (*src == '+') { src++; }
    int v = 0;
    while (*src >= '0' && *src <= '9')
        v = (v * 10) + (*src++ - '0');
    *output = negative ? -v : v;
    return src;
}

// MSVC version appears to return -1 on overflow, whereas glibc appears to return total count (which may be >= buf_size). 
// Ideally we would test for only one of those limits at runtime depending on the behavior the vsnprintf(), but trying to deduct it at compile time sounds like a pandora can of worm.
int ImFormatString(char* buf, int buf_size, const char* fmt, ...)
{
    IM_ASSERT(buf_size > 0);
    va_list args;
    va_start(args, fmt);
    int w = vsnprintf(buf, buf_size, fmt, args);
    va_end(args);
    if (w == -1 || w >= buf_size)
        w = buf_size - 1;
    buf[w] = 0;
    return w;
}

int ImFormatStringV(char* buf, int buf_size, const char* fmt, va_list args)
{
    IM_ASSERT(buf_size > 0);
    int w = vsnprintf(buf, buf_size, fmt, args);
    if (w == -1 || w >= buf_size)
        w = buf_size - 1;
    buf[w] = 0;
    return w;
}

// Pass data_size==0 for zero-terminated strings
// FIXME-OPT: Replace with e.g. FNV1a hash? CRC32 pretty much randomly access 1KB. Need to do proper measurements.
ImU32 ImHash(const void* data, int data_size, ImU32 seed)
{
    static ImU32 crc32_lut[256] = { 0 };
    if (!crc32_lut[1])
    {
        const ImU32 polynomial = 0xEDB88320;
        for (ImU32 i = 0; i < 256; i++)
        {
            ImU32 crc = i;
            for (ImU32 j = 0; j < 8; j++)
                crc = (crc >> 1) ^ (ImU32(-int(crc & 1)) & polynomial);
            crc32_lut[i] = crc;
        }
    }

    seed = ~seed;
    ImU32 crc = seed;
    const unsigned char* current = (const unsigned char*)data;

    if (data_size > 0)
    {
        // Known size
        while (data_size--)
            crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ *current++];
    }
    else
    {
        // Zero-terminated string
        while (unsigned char c = *current++)
        {
            // We support a syntax of "label###id" where only "###id" is included in the hash, and only "label" gets displayed.
            // Because this syntax is rarely used we are optimizing for the common case.
            // - If we reach ### in the string we discard the hash so far and reset to the seed.
            // - We don't do 'current += 2; continue;' after handling ### to keep the code smaller.
            if (c == '#' && current[0] == '#' && current[1] == '#')
                crc = seed;
            crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
        }
    }
    return ~crc;
}

//-----------------------------------------------------------------------------
// ImText* helpers
//-----------------------------------------------------------------------------

// Convert UTF-8 to 32-bits character, process single character input.
// Based on stb_from_utf8() from github.com/nothings/stb/
// We handle UTF-8 decoding error by skipping forward.
int ImTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end)
{
    unsigned int c = (unsigned int)-1;
    const unsigned char* str = (const unsigned char*)in_text;
    if (!(*str & 0x80))
    {
        c = (unsigned int)(*str++);
        *out_char = c;
        return 1;
    }
    if ((*str & 0xe0) == 0xc0)
    {
        *out_char = 0xFFFD; // will be invalid but not end of string
        if (in_text_end && in_text_end - (const char*)str < 2) return 1;
        if (*str < 0xc2) return 2;
        c = (unsigned int)((*str++ & 0x1f) << 6);
        if ((*str & 0xc0) != 0x80) return 2;
        c += (*str++ & 0x3f);
        *out_char = c;
        return 2;
    }
    if ((*str & 0xf0) == 0xe0)
    {
        *out_char = 0xFFFD; // will be invalid but not end of string
        if (in_text_end && in_text_end - (const char*)str < 3) return 1;
        if (*str == 0xe0 && (str[1] < 0xa0 || str[1] > 0xbf)) return 3;
        if (*str == 0xed && str[1] > 0x9f) return 3; // str[1] < 0x80 is checked below
        c = (unsigned int)((*str++ & 0x0f) << 12);
        if ((*str & 0xc0) != 0x80) return 3;
        c += (unsigned int)((*str++ & 0x3f) << 6);
        if ((*str & 0xc0) != 0x80) return 3;
        c += (*str++ & 0x3f);
        *out_char = c;
        return 3;
    }
    if ((*str & 0xf8) == 0xf0)
    {
        *out_char = 0xFFFD; // will be invalid but not end of string
        if (in_text_end && in_text_end - (const char*)str < 4) return 1;
        if (*str > 0xf4) return 4;
        if (*str == 0xf0 && (str[1] < 0x90 || str[1] > 0xbf)) return 4;
        if (*str == 0xf4 && str[1] > 0x8f) return 4; // str[1] < 0x80 is checked below
        c = (unsigned int)((*str++ & 0x07) << 18);
        if ((*str & 0xc0) != 0x80) return 4;
        c += (unsigned int)((*str++ & 0x3f) << 12);
        if ((*str & 0xc0) != 0x80) return 4;
        c += (unsigned int)((*str++ & 0x3f) << 6);
        if ((*str & 0xc0) != 0x80) return 4;
        c += (*str++ & 0x3f);
        // utf-8 encodings of values used in surrogate pairs are invalid
        if ((c & 0xFFFFF800) == 0xD800) return 4;
        *out_char = c;
        return 4;
    }
    *out_char = 0;
    return 0;
}

int ImTextStrFromUtf8(ImWchar* buf, int buf_size, const char* in_text, const char* in_text_end, const char** in_text_remaining)
{
    ImWchar* buf_out = buf;
    ImWchar* buf_end = buf + buf_size;
    while (buf_out < buf_end-1 && (!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c;
        in_text += ImTextCharFromUtf8(&c, in_text, in_text_end);
        if (c == 0)
            break;
        if (c < 0x10000)    // FIXME: Losing characters that don't fit in 2 bytes
            *buf_out++ = (ImWchar)c;
    }
    *buf_out = 0;
    if (in_text_remaining)
        *in_text_remaining = in_text;
    return (int)(buf_out - buf);
}

int ImTextCountCharsFromUtf8(const char* in_text, const char* in_text_end)
{
    int char_count = 0;
    while ((!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c;
        in_text += ImTextCharFromUtf8(&c, in_text, in_text_end);
        if (c == 0)
            break;
        if (c < 0x10000)
            char_count++;
    }
    return char_count;
}

// Based on stb_to_utf8() from github.com/nothings/stb/
static inline int ImTextCharToUtf8(char* buf, int buf_size, unsigned int c)
{
    if (c < 0x80)
    {
        buf[0] = (char)c;
        return 1;
    }
    if (c < 0x800)
    {
        if (buf_size < 2) return 0;
        buf[0] = (char)(0xc0 + (c >> 6));
        buf[1] = (char)(0x80 + (c & 0x3f));
        return 2;
    }
    if (c >= 0xdc00 && c < 0xe000)
    {
        return 0;
    }
    if (c >= 0xd800 && c < 0xdc00)
    {
        if (buf_size < 4) return 0;
        buf[0] = (char)(0xf0 + (c >> 18));
        buf[1] = (char)(0x80 + ((c >> 12) & 0x3f));
        buf[2] = (char)(0x80 + ((c >> 6) & 0x3f));
        buf[3] = (char)(0x80 + ((c ) & 0x3f));
        return 4;
    }
    //else if (c < 0x10000)
    {
        if (buf_size < 3) return 0;
        buf[0] = (char)(0xe0 + (c >> 12));
        buf[1] = (char)(0x80 + ((c>> 6) & 0x3f));
        buf[2] = (char)(0x80 + ((c ) & 0x3f));
        return 3;
    }
}

static inline int ImTextCountUtf8BytesFromChar(unsigned int c)
{
    if (c < 0x80) return 1;
    if (c < 0x800) return 2;
    if (c >= 0xdc00 && c < 0xe000) return 0;
    if (c >= 0xd800 && c < 0xdc00) return 4;
    return 3;
}

int ImTextStrToUtf8(char* buf, int buf_size, const ImWchar* in_text, const ImWchar* in_text_end)
{
    char* buf_out = buf;
    const char* buf_end = buf + buf_size;
    while (buf_out < buf_end-1 && (!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c = (unsigned int)(*in_text++);
        if (c < 0x80)
            *buf_out++ = (char)c;
        else
            buf_out += ImTextCharToUtf8(buf_out, (int)(buf_end-buf_out-1), c);
    }
    *buf_out = 0;
    return (int)(buf_out - buf);
}

int ImTextCountUtf8BytesFromStr(const ImWchar* in_text, const ImWchar* in_text_end)
{
    int bytes_count = 0;
    while ((!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c = (unsigned int)(*in_text++);
        if (c < 0x80)
            bytes_count++;
        else
            bytes_count += ImTextCountUtf8BytesFromChar(c);
    }
    return bytes_count;
}

ImVec4 ImGui::ColorConvertU32ToFloat4(ImU32 in)
{
    float s = 1.0f/255.0f;
    return ImVec4(
        ((in >> IM_COL32_R_SHIFT) & 0xFF) * s,
        ((in >> IM_COL32_G_SHIFT) & 0xFF) * s,
        ((in >> IM_COL32_B_SHIFT) & 0xFF) * s,
        ((in >> IM_COL32_A_SHIFT) & 0xFF) * s);
}

ImU32 ImGui::ColorConvertFloat4ToU32(const ImVec4& in)
{
    ImU32 out;
    out  = ((ImU32)IM_F32_TO_INT8_SAT(in.x)) << IM_COL32_R_SHIFT;
    out |= ((ImU32)IM_F32_TO_INT8_SAT(in.y)) << IM_COL32_G_SHIFT;
    out |= ((ImU32)IM_F32_TO_INT8_SAT(in.z)) << IM_COL32_B_SHIFT;
    out |= ((ImU32)IM_F32_TO_INT8_SAT(in.w)) << IM_COL32_A_SHIFT;
    return out;
}

ImU32 ImGui::GetColorU32(ImGuiCol idx, float alpha_mul)  
{ 
    ImGuiStyle& style = GImGui->Style;
    ImVec4 c = style.Colors[idx]; 
    c.w *= style.Alpha * alpha_mul; 
    return ColorConvertFloat4ToU32(c); 
}

ImU32 ImGui::GetColorU32(const ImVec4& col)
{ 
    ImGuiStyle& style = GImGui->Style;
    ImVec4 c = col; 
    c.w *= style.Alpha; 
    return ColorConvertFloat4ToU32(c); 
}

const ImVec4& ImGui::GetStyleColorVec4(ImGuiCol idx)
{ 
    ImGuiStyle& style = GImGui->Style;
    return style.Colors[idx];
}

ImU32 ImGui::GetColorU32(ImU32 col)
{ 
    float style_alpha = GImGui->Style.Alpha;
    if (style_alpha >= 1.0f)
        return col;
    int a = (col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT;
    a = (int)(a * style_alpha); // We don't need to clamp 0..255 because Style.Alpha is in 0..1 range.
    return (col & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
}

// Convert rgb floats ([0-1],[0-1],[0-1]) to hsv floats ([0-1],[0-1],[0-1]), from Foley & van Dam p592
// Optimized http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
void ImGui::ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v)
{
    float K = 0.f;
    if (g < b)
    {
        ImSwap(g, b);
        K = -1.f;
    }
    if (r < g)
    {
        ImSwap(r, g);
        K = -2.f / 6.f - K;
    }

    const float chroma = r - (g < b ? g : b);
    out_h = fabsf(K + (g - b) / (6.f * chroma + 1e-20f));
    out_s = chroma / (r + 1e-20f);
    out_v = r;
}

// Convert hsv floats ([0-1],[0-1],[0-1]) to rgb floats ([0-1],[0-1],[0-1]), from Foley & van Dam p593
// also http://en.wikipedia.org/wiki/HSL_and_HSV
void ImGui::ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b)
{
    if (s == 0.0f)
    {
        // gray
        out_r = out_g = out_b = v;
        return;
    }

    h = fmodf(h, 1.0f) / (60.0f/360.0f);
    int   i = (int)h;
    float f = h - (float)i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (i)
    {
    case 0: out_r = v; out_g = t; out_b = p; break;
    case 1: out_r = q; out_g = v; out_b = p; break;
    case 2: out_r = p; out_g = v; out_b = t; break;
    case 3: out_r = p; out_g = q; out_b = v; break;
    case 4: out_r = t; out_g = p; out_b = v; break;
    case 5: default: out_r = v; out_g = p; out_b = q; break;
    }
}

FILE* ImFileOpen(const char* filename, const char* mode)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
    // We need a fopen() wrapper because MSVC/Windows fopen doesn't handle UTF-8 filenames. Converting both strings from UTF-8 to wchar format (using a single allocation, because we can)
    const int filename_wsize = ImTextCountCharsFromUtf8(filename, NULL) + 1;
    const int mode_wsize = ImTextCountCharsFromUtf8(mode, NULL) + 1;
    ImVector<ImWchar> buf;
    buf.resize(filename_wsize + mode_wsize);
    ImTextStrFromUtf8(&buf[0], filename_wsize, filename, NULL);
    ImTextStrFromUtf8(&buf[filename_wsize], mode_wsize, mode, NULL);
    return _wfopen((wchar_t*)&buf[0], (wchar_t*)&buf[filename_wsize]);
#else
    return fopen(filename, mode);
#endif
}

// Load file content into memory
// Memory allocated with ImGui::MemAlloc(), must be freed by user using ImGui::MemFree()
void* ImFileLoadToMemory(const char* filename, const char* file_open_mode, int* out_file_size, int padding_bytes)
{
    IM_ASSERT(filename && file_open_mode);
    if (out_file_size)
        *out_file_size = 0;

    FILE* f;
    if ((f = ImFileOpen(filename, file_open_mode)) == NULL)
        return NULL;

    long file_size_signed;
    if (fseek(f, 0, SEEK_END) || (file_size_signed = ftell(f)) == -1 || fseek(f, 0, SEEK_SET))
    {
        fclose(f);
        return NULL;
    }

    int file_size = (int)file_size_signed;
    void* file_data = ImGui::MemAlloc(file_size + padding_bytes);
    if (file_data == NULL)
    {
        fclose(f);
        return NULL;
    }
    if (fread(file_data, 1, (size_t)file_size, f) != (size_t)file_size)
    {
        fclose(f);
        ImGui::MemFree(file_data);
        return NULL;
    }
    if (padding_bytes > 0)
        memset((void *)(((char*)file_data) + file_size), 0, padding_bytes);

    fclose(f);
    if (out_file_size)
        *out_file_size = file_size;

    return file_data;
}

//-----------------------------------------------------------------------------
// ImGuiStorage
//-----------------------------------------------------------------------------

// Helper: Key->value storage
void ImGuiStorage::Clear()
{
    Data.clear();
}

// std::lower_bound but without the bullshit
static ImVector<ImGuiStorage::Pair>::iterator LowerBound(ImVector<ImGuiStorage::Pair>& data, ImGuiID key)
{
    ImVector<ImGuiStorage::Pair>::iterator first = data.begin();
    ImVector<ImGuiStorage::Pair>::iterator last = data.end();
    int count = (int)(last - first);
    while (count > 0)
    {
        int count2 = count / 2;
        ImVector<ImGuiStorage::Pair>::iterator mid = first + count2;
        if (mid->key < key)
        {
            first = ++mid;
            count -= count2 + 1;
        }
        else
        {
            count = count2;
        }
    }
    return first;
}

int ImGuiStorage::GetInt(ImGuiID key, int default_val) const
{
    ImVector<Pair>::iterator it = LowerBound(const_cast<ImVector<ImGuiStorage::Pair>&>(Data), key);
    if (it == Data.end() || it->key != key)
        return default_val;
    return it->val_i;
}

bool ImGuiStorage::GetBool(ImGuiID key, bool default_val) const
{
    return GetInt(key, default_val ? 1 : 0) != 0;
}

float ImGuiStorage::GetFloat(ImGuiID key, float default_val) const
{
    ImVector<Pair>::iterator it = LowerBound(const_cast<ImVector<ImGuiStorage::Pair>&>(Data), key);
    if (it == Data.end() || it->key != key)
        return default_val;
    return it->val_f;
}

void* ImGuiStorage::GetVoidPtr(ImGuiID key) const
{
    ImVector<Pair>::iterator it = LowerBound(const_cast<ImVector<ImGuiStorage::Pair>&>(Data), key);
    if (it == Data.end() || it->key != key)
        return NULL;
    return it->val_p;
}

// References are only valid until a new value is added to the storage. Calling a Set***() function or a Get***Ref() function invalidates the pointer.
int* ImGuiStorage::GetIntRef(ImGuiID key, int default_val)
{
    ImVector<Pair>::iterator it = LowerBound(Data, key);
    if (it == Data.end() || it->key != key)
        it = Data.insert(it, Pair(key, default_val));
    return &it->val_i;
}

bool* ImGuiStorage::GetBoolRef(ImGuiID key, bool default_val)
{
    return (bool*)GetIntRef(key, default_val ? 1 : 0);
}

float* ImGuiStorage::GetFloatRef(ImGuiID key, float default_val)
{
    ImVector<Pair>::iterator it = LowerBound(Data, key);
    if (it == Data.end() || it->key != key)
        it = Data.insert(it, Pair(key, default_val));
    return &it->val_f;
}

void** ImGuiStorage::GetVoidPtrRef(ImGuiID key, void* default_val)
{
    ImVector<Pair>::iterator it = LowerBound(Data, key);
    if (it == Data.end() || it->key != key)
        it = Data.insert(it, Pair(key, default_val));
    return &it->val_p;
}

// FIXME-OPT: Need a way to reuse the result of lower_bound when doing GetInt()/SetInt() - not too bad because it only happens on explicit interaction (maximum one a frame)
void ImGuiStorage::SetInt(ImGuiID key, int val)
{
    ImVector<Pair>::iterator it = LowerBound(Data, key);
    if (it == Data.end() || it->key != key)
    {
        Data.insert(it, Pair(key, val));
        return;
    }
    it->val_i = val;
}

void ImGuiStorage::SetBool(ImGuiID key, bool val)
{
    SetInt(key, val ? 1 : 0);
}

void ImGuiStorage::SetFloat(ImGuiID key, float val)
{
    ImVector<Pair>::iterator it = LowerBound(Data, key);
    if (it == Data.end() || it->key != key)
    {
        Data.insert(it, Pair(key, val));
        return;
    }
    it->val_f = val;
}

void ImGuiStorage::SetVoidPtr(ImGuiID key, void* val)
{
    ImVector<Pair>::iterator it = LowerBound(Data, key);
    if (it == Data.end() || it->key != key)
    {
        Data.insert(it, Pair(key, val));
        return;
    }
    it->val_p = val;
}

void ImGuiStorage::SetAllInt(int v)
{
    for (int i = 0; i < Data.Size; i++)
        Data[i].val_i = v;
}

//-----------------------------------------------------------------------------
// ImGuiTextFilter
//-----------------------------------------------------------------------------

// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]"
ImGuiTextFilter::ImGuiTextFilter(const char* default_filter)
{
    if (default_filter)
    {
        ImStrncpy(InputBuf, default_filter, IM_ARRAYSIZE(InputBuf));
        Build();
    }
    else
    {
        InputBuf[0] = 0;
        CountGrep = 0;
    }
}

bool ImGuiTextFilter::Draw(const char* label, float width)
{
    if (width != 0.0f)
        ImGui::PushItemWidth(width);
    bool value_changed = ImGui::InputText(label, InputBuf, IM_ARRAYSIZE(InputBuf));
    if (width != 0.0f)
        ImGui::PopItemWidth();
    if (value_changed)
        Build();
    return value_changed;
}

void ImGuiTextFilter::TextRange::split(char separator, ImVector<TextRange>& out)
{
    out.resize(0);
    const char* wb = b;
    const char* we = wb;
    while (we < e)
    {
        if (*we == separator)
        {
            out.push_back(TextRange(wb, we));
            wb = we + 1;
        }
        we++;
    }
    if (wb != we)
        out.push_back(TextRange(wb, we));
}

void ImGuiTextFilter::Build()
{
    Filters.resize(0);
    TextRange input_range(InputBuf, InputBuf+strlen(InputBuf));
    input_range.split(',', Filters);

    CountGrep = 0;
    for (int i = 0; i != Filters.Size; i++)
    {
        Filters[i].trim_blanks();
        if (Filters[i].empty())
            continue;
        if (Filters[i].front() != '-')
            CountGrep += 1;
    }
}

bool ImGuiTextFilter::PassFilter(const char* text, const char* text_end) const
{
    if (Filters.empty())
        return true;

    if (text == NULL)
        text = "";

    for (int i = 0; i != Filters.Size; i++)
    {
        const TextRange& f = Filters[i];
        if (f.empty())
            continue;
        if (f.front() == '-')
        {
            // Subtract
            if (ImStristr(text, text_end, f.begin()+1, f.end()) != NULL)
                return false;
        }
        else
        {
            // Grep
            if (ImStristr(text, text_end, f.begin(), f.end()) != NULL)
                return true;
        }
    }

    // Implicit * grep
    if (CountGrep == 0)
        return true;

    return false;
}

//-----------------------------------------------------------------------------
// ImGuiTextBuffer
//-----------------------------------------------------------------------------

// On some platform vsnprintf() takes va_list by reference and modifies it.
// va_copy is the 'correct' way to copy a va_list but Visual Studio prior to 2013 doesn't have it.
#ifndef va_copy
#define va_copy(dest, src) (dest = src)
#endif

// Helper: Text buffer for logging/accumulating text
void ImGuiTextBuffer::appendv(const char* fmt, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);

    int len = vsnprintf(NULL, 0, fmt, args);         // FIXME-OPT: could do a first pass write attempt, likely successful on first pass.
    if (len <= 0)
        return;

    const int write_off = Buf.Size;
    const int needed_sz = write_off + len;
    if (write_off + len >= Buf.Capacity)
    {
        int double_capacity = Buf.Capacity * 2;
        Buf.reserve(needed_sz > double_capacity ? needed_sz : double_capacity);
    }

    Buf.resize(needed_sz);
    ImFormatStringV(&Buf[write_off] - 1, len+1, fmt, args_copy);
}

void ImGuiTextBuffer::append(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    appendv(fmt, args);
    va_end(args);
}

//-----------------------------------------------------------------------------
// ImGuiSimpleColumns
//-----------------------------------------------------------------------------

ImGuiSimpleColumns::ImGuiSimpleColumns()
{
    Count = 0;
    Spacing = Width = NextWidth = 0.0f;
    memset(Pos, 0, sizeof(Pos));
    memset(NextWidths, 0, sizeof(NextWidths));
}

void ImGuiSimpleColumns::Update(int count, float spacing, bool clear)
{
    IM_ASSERT(Count <= IM_ARRAYSIZE(Pos));
    Count = count;
    Width = NextWidth = 0.0f;
    Spacing = spacing;
    if (clear) memset(NextWidths, 0, sizeof(NextWidths));
    for (int i = 0; i < Count; i++)
    {
        if (i > 0 && NextWidths[i] > 0.0f)
            Width += Spacing;
        Pos[i] = (float)(int)Width;
        Width += NextWidths[i];
        NextWidths[i] = 0.0f;
    }
}

float ImGuiSimpleColumns::DeclColumns(float w0, float w1, float w2) // not using va_arg because they promote float to double
{
    NextWidth = 0.0f;
    NextWidths[0] = ImMax(NextWidths[0], w0);
    NextWidths[1] = ImMax(NextWidths[1], w1);
    NextWidths[2] = ImMax(NextWidths[2], w2);
    for (int i = 0; i < 3; i++)
        NextWidth += NextWidths[i] + ((i > 0 && NextWidths[i] > 0.0f) ? Spacing : 0.0f);
    return ImMax(Width, NextWidth);
}

float ImGuiSimpleColumns::CalcExtraSpace(float avail_w)
{
    return ImMax(0.0f, avail_w - Width);
}

//-----------------------------------------------------------------------------
// ImGuiListClipper
//-----------------------------------------------------------------------------

static void SetCursorPosYAndSetupDummyPrevLine(float pos_y, float line_height)
{
    // Set cursor position and a few other things so that SetScrollHere() and Columns() can work when seeking cursor. 
    // FIXME: It is problematic that we have to do that here, because custom/equivalent end-user code would stumble on the same issue. Consider moving within SetCursorXXX functions?
    ImGui::SetCursorPosY(pos_y);
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    window->DC.CursorPosPrevLine.y = window->DC.CursorPos.y - line_height;      // Setting those fields so that SetScrollHere() can properly function after the end of our clipper usage.
    window->DC.PrevLineHeight = (line_height - GImGui->Style.ItemSpacing.y);    // If we end up needing more accurate data (to e.g. use SameLine) we may as well make the clipper have a fourth step to let user process and display the last item in their list.
    if (window->DC.ColumnsCount > 1)
        window->DC.ColumnsCellMinY = window->DC.CursorPos.y;                    // Setting this so that cell Y position are set properly
}

// Use case A: Begin() called from constructor with items_height<0, then called again from Sync() in StepNo 1
// Use case B: Begin() called from constructor with items_height>0
// FIXME-LEGACY: Ideally we should remove the Begin/End functions but they are part of the legacy API we still support. This is why some of the code in Step() calling Begin() and reassign some fields, spaghetti style.
void ImGuiListClipper::Begin(int count, float items_height)
{
    StartPosY = ImGui::GetCursorPosY();
    ItemsHeight = items_height;
    ItemsCount = count;
    StepNo = 0;
    DisplayEnd = DisplayStart = -1;
    if (ItemsHeight > 0.0f)
    {
        ImGui::CalcListClipping(ItemsCount, ItemsHeight, &DisplayStart, &DisplayEnd); // calculate how many to clip/display
        if (DisplayStart > 0)
            SetCursorPosYAndSetupDummyPrevLine(StartPosY + DisplayStart * ItemsHeight, ItemsHeight); // advance cursor
        StepNo = 2;
    }
}

void ImGuiListClipper::End()
{
    if (ItemsCount < 0)
        return;
    // In theory here we should assert that ImGui::GetCursorPosY() == StartPosY + DisplayEnd * ItemsHeight, but it feels saner to just seek at the end and not assert/crash the user.
    if (ItemsCount < INT_MAX)
        SetCursorPosYAndSetupDummyPrevLine(StartPosY + ItemsCount * ItemsHeight, ItemsHeight); // advance cursor
    ItemsCount = -1;
    StepNo = 3;
}

bool ImGuiListClipper::Step()
{
    if (ItemsCount == 0 || ImGui::GetCurrentWindowRead()->SkipItems)
    {
        ItemsCount = -1; 
        return false; 
    }
    if (StepNo == 0) // Step 0: the clipper let you process the first element, regardless of it being visible or not, so we can measure the element height.
    {
        DisplayStart = 0;
        DisplayEnd = 1;
        StartPosY = ImGui::GetCursorPosY();
        StepNo = 1;
        return true;
    }
    if (StepNo == 1) // Step 1: the clipper infer height from first element, calculate the actual range of elements to display, and position the cursor before the first element.
    {
        if (ItemsCount == 1) { ItemsCount = -1; return false; }
        float items_height = ImGui::GetCursorPosY() - StartPosY;
        IM_ASSERT(items_height > 0.0f);   // If this triggers, it means Item 0 hasn't moved the cursor vertically
        Begin(ItemsCount-1, items_height);
        DisplayStart++;
        DisplayEnd++;
        StepNo = 3;
        return true;
    }
    if (StepNo == 2) // Step 2: dummy step only required if an explicit items_height was passed to constructor or Begin() and user still call Step(). Does nothing and switch to Step 3.
    {
        IM_ASSERT(DisplayStart >= 0 && DisplayEnd >= 0);
        StepNo = 3;
        return true;
    }
    if (StepNo == 3) // Step 3: the clipper validate that we have reached the expected Y position (corresponding to element DisplayEnd), advance the cursor to the end of the list and then returns 'false' to end the loop.
        End();
    return false;
}

//-----------------------------------------------------------------------------
// ImGuiWindow
//-----------------------------------------------------------------------------

ImGuiWindow::ImGuiWindow(const char* name)
{
    Name = ImStrdup(name);
    ID = ImHash(name, 0);
    IDStack.push_back(ID);
    Flags = 0;
    OrderWithinParent = 0;
    PosFloat = Pos = ImVec2(0.0f, 0.0f);
    Size = SizeFull = ImVec2(0.0f, 0.0f);
    SizeContents = SizeContentsExplicit = ImVec2(0.0f, 0.0f);
    WindowPadding = ImVec2(0.0f, 0.0f);
    MoveId = GetID("#MOVE");
    Scroll = ImVec2(0.0f, 0.0f);
    ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);
    ScrollTargetCenterRatio = ImVec2(0.5f, 0.5f);
    ScrollbarX = ScrollbarY = false;
    ScrollbarSizes = ImVec2(0.0f, 0.0f);
    BorderSize = 0.0f;
    Active = WasActive = false;
    Accessed = false;
    Collapsed = false;
    SkipItems = false;
    Appearing = false;
    BeginCount = 0;
    PopupId = 0;
    AutoFitFramesX = AutoFitFramesY = -1;
    AutoFitOnlyGrows = false;
    AutoFitChildAxises = 0x00;
    AutoPosLastDirection = -1;
    HiddenFrames = 0;
    SetWindowPosAllowFlags = SetWindowSizeAllowFlags = SetWindowCollapsedAllowFlags = ImGuiCond_Always | ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing;
    SetWindowPosVal = SetWindowPosPivot = ImVec2(FLT_MAX, FLT_MAX);

    LastFrameActive = -1;
    ItemWidthDefault = 0.0f;
    FontWindowScale = 1.0f;

    DrawList = (ImDrawList*)ImGui::MemAlloc(sizeof(ImDrawList));
    IM_PLACEMENT_NEW(DrawList) ImDrawList();
    DrawList->_OwnerName = Name;
    ParentWindow = NULL;
    RootWindow = NULL;
    RootNonPopupWindow = NULL;

    FocusIdxAllCounter = FocusIdxTabCounter = -1;
    FocusIdxAllRequestCurrent = FocusIdxTabRequestCurrent = INT_MAX;
    FocusIdxAllRequestNext = FocusIdxTabRequestNext = INT_MAX;
}

ImGuiWindow::~ImGuiWindow()
{
    DrawList->~ImDrawList();
    ImGui::MemFree(DrawList);
    DrawList = NULL;
    ImGui::MemFree(Name);
    Name = NULL;
}

ImGuiID ImGuiWindow::GetID(const char* str, const char* str_end)
{
    ImGuiID seed = IDStack.back();
    ImGuiID id = ImHash(str, str_end ? (int)(str_end - str) : 0, seed);
    ImGui::KeepAliveID(id);
    return id;
}

ImGuiID ImGuiWindow::GetID(const void* ptr)
{
    ImGuiID seed = IDStack.back();
    ImGuiID id = ImHash(&ptr, sizeof(void*), seed);
    ImGui::KeepAliveID(id);
    return id;
}

ImGuiID ImGuiWindow::GetIDNoKeepAlive(const char* str, const char* str_end)
{
    ImGuiID seed = IDStack.back();
    return ImHash(str, str_end ? (int)(str_end - str) : 0, seed);
}

//-----------------------------------------------------------------------------
// Internal API exposed in imgui_internal.h
//-----------------------------------------------------------------------------

static void SetCurrentWindow(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    g.CurrentWindow = window;
    if (window)
        g.FontSize = window->CalcFontSize();
}

ImGuiWindow* ImGui::GetParentWindow()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.CurrentWindowStack.Size >= 2);
    return g.CurrentWindowStack[(unsigned int)g.CurrentWindowStack.Size - 2];
}

void ImGui::SetActiveID(ImGuiID id, ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    g.ActiveIdIsJustActivated = (g.ActiveId != id);
    g.ActiveId = id;
    g.ActiveIdAllowOverlap = false;
    g.ActiveIdIsAlive |= (id != 0);
    g.ActiveIdWindow = window;
}

void ImGui::ClearActiveID()
{
    SetActiveID(0, NULL);
}

void ImGui::SetHoveredID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    g.HoveredId = id;
    g.HoveredIdAllowOverlap = false;
}

void ImGui::KeepAliveID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId == id)
        g.ActiveIdIsAlive = true;
}

static inline bool IsWindowContentHoverable(ImGuiWindow* window)
{
    // An active popup disable hovering on other windows (apart from its own children)
    // FIXME-OPT: This could be cached/stored within the window.
    ImGuiContext& g = *GImGui;
    if (g.NavWindow)
        if (ImGuiWindow* focused_root_window = g.NavWindow->RootWindow)
            if ((focused_root_window->Flags & ImGuiWindowFlags_Popup) != 0 && focused_root_window->WasActive && focused_root_window != window->RootWindow)
                return false;

    return true;
}

// Advance cursor given item size for layout.
void ImGui::ItemSize(const ImVec2& size, float text_offset_y)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return;

    // Always align ourselves on pixel boundaries
    const float line_height = ImMax(window->DC.CurrentLineHeight, size.y);
    const float text_base_offset = ImMax(window->DC.CurrentLineTextBaseOffset, text_offset_y);
    //if (g.IO.KeyAlt) window->DrawList->AddRect(window->DC.CursorPos, window->DC.CursorPos + ImVec2(size.x, line_height), IM_COL32(255,0,0,200)); // [DEBUG]
    window->DC.CursorPosPrevLine = ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y);
    window->DC.CursorPos = ImVec2((float)(int)(window->Pos.x + window->DC.IndentX + window->DC.ColumnsOffsetX), (float)(int)(window->DC.CursorPos.y + line_height + g.Style.ItemSpacing.y));
    window->DC.CursorMaxPos.x = ImMax(window->DC.CursorMaxPos.x, window->DC.CursorPosPrevLine.x);
    window->DC.CursorMaxPos.y = ImMax(window->DC.CursorMaxPos.y, window->DC.CursorPos.y);
    //if (g.IO.KeyAlt) window->DrawList->AddCircle(window->DC.CursorMaxPos, 3.0f, IM_COL32(255,0,0,255), 4); // [DEBUG]

    window->DC.PrevLineHeight = line_height;
    window->DC.PrevLineTextBaseOffset = text_base_offset;
    window->DC.CurrentLineHeight = window->DC.CurrentLineTextBaseOffset = 0.0f;

    // Horizontal layout mode
    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
        SameLine();
}

void ImGui::ItemSize(const ImRect& bb, float text_offset_y)
{
    ItemSize(bb.GetSize(), text_offset_y);
}

// Declare item bounding box for clipping and interaction.
// Note that the size can be different than the one provided to ItemSize(). Typically, widgets that spread over available surface
// declares their minimum size requirement to ItemSize() and then use a larger region for drawing/interaction, which is passed to ItemAdd().
bool ImGui::ItemAdd(const ImRect& bb, ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    const bool is_clipped = IsClippedEx(bb, id, false);
    window->DC.LastItemId = id;
    window->DC.LastItemRect = bb;
    window->DC.LastItemRectHoveredRect = false;
    if (is_clipped)
        return false;
    //if (g.IO.KeyAlt) window->DrawList->AddRect(bb.Min, bb.Max, IM_COL32(255,255,0,120)); // [DEBUG]

    // We need to calculate this now to take account of the current clipping rectangle (as items like Selectable may change them)
    window->DC.LastItemRectHoveredRect = IsMouseHoveringRect(bb.Min, bb.Max);
    return true;
}

// This is roughly matching the behavior of internal-facing ItemHoverable()
// - we allow hovering to be true when ActiveId==window->MoveID, so that clicking on non-interactive items such as a Text() item still returns true with IsItemHovered())
// - this should work even for non-interactive items that have no ID, so we cannot use LastItemId
bool ImGui::IsItemHovered()
{
    ImGuiContext& g = *GImGui;

    ImGuiWindow* window = g.CurrentWindow;
    if (!window->DC.LastItemRectHoveredRect)
        return false;
    // [2017/10/16] Reverted commit 344d48be3 and testing RootWindow instead. I believe it is correct to NOT test for RootWindow but this leaves us unable to use IsItemHovered() after EndChild() itself.
    // Until a solution is found I believe reverting to the test from 2017/09/27 is safe since this was the test that has been running for a long while.
    //if (g.HoveredWindow != window)
    //    return false;
    if (g.HoveredRootWindow != window->RootWindow)
        return false;
    if (g.ActiveId != 0 && g.ActiveId != window->DC.LastItemId && !g.ActiveIdAllowOverlap && g.ActiveId != window->MoveId)
        return false;
    if (!IsWindowContentHoverable(window))
        return false;
    return true;
}

bool ImGui::IsItemRectHovered()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.LastItemRectHoveredRect;
}

// Internal facing ItemHoverable() used when submitting widgets. Differs slightly from IsItemHovered().
bool ImGui::ItemHoverable(const ImRect& bb, ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    if (g.HoveredId != 0 && g.HoveredId != id && !g.HoveredIdAllowOverlap)
        return false;

    ImGuiWindow* window = g.CurrentWindow;
    if (g.HoveredWindow != window)
        return false;
    if (g.ActiveId != 0 && g.ActiveId != id && !g.ActiveIdAllowOverlap)
        return false;
    if (!IsMouseHoveringRect(bb.Min, bb.Max))
        return false;
    if (!IsWindowContentHoverable(window))
        return false;

    SetHoveredID(id);
    return true;
}

bool ImGui::IsClippedEx(const ImRect& bb, ImGuiID id, bool clip_even_when_logged)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (!bb.Overlaps(window->ClipRect))
        if (id == 0 || id != g.ActiveId)
            if (clip_even_when_logged || !g.LogEnabled)
                return true;
    return false;
}

bool ImGui::FocusableItemRegister(ImGuiWindow* window, ImGuiID id, bool tab_stop)
{
    ImGuiContext& g = *GImGui;

    const bool allow_keyboard_focus = (window->DC.ItemFlags & ImGuiItemFlags_AllowKeyboardFocus) != 0;
    window->FocusIdxAllCounter++;
    if (allow_keyboard_focus)
        window->FocusIdxTabCounter++;

    // Process keyboard input at this point: TAB/Shift-TAB to tab out of the currently focused item.
    // Note that we can always TAB out of a widget that doesn't allow tabbing in.
    if (tab_stop && (g.ActiveId == id) && window->FocusIdxAllRequestNext == INT_MAX && window->FocusIdxTabRequestNext == INT_MAX && !g.IO.KeyCtrl && IsKeyPressedMap(ImGuiKey_Tab))
        window->FocusIdxTabRequestNext = window->FocusIdxTabCounter + (g.IO.KeyShift ? (allow_keyboard_focus ? -1 : 0) : +1); // Modulo on index will be applied at the end of frame once we've got the total counter of items.

    if (window->FocusIdxAllCounter == window->FocusIdxAllRequestCurrent)
        return true;

    if (allow_keyboard_focus)
        if (window->FocusIdxTabCounter == window->FocusIdxTabRequestCurrent)
            return true;

    return false;
}

void ImGui::FocusableItemUnregister(ImGuiWindow* window)
{
    window->FocusIdxAllCounter--;
    window->FocusIdxTabCounter--;
}

ImVec2 ImGui::CalcItemSize(ImVec2 size, float default_x, float default_y)
{
    ImGuiContext& g = *GImGui;
    ImVec2 content_max;
    if (size.x < 0.0f || size.y < 0.0f)
        content_max = g.CurrentWindow->Pos + GetContentRegionMax();
    if (size.x <= 0.0f)
        size.x = (size.x == 0.0f) ? default_x : ImMax(content_max.x - g.CurrentWindow->DC.CursorPos.x, 4.0f) + size.x;
    if (size.y <= 0.0f)
        size.y = (size.y == 0.0f) ? default_y : ImMax(content_max.y - g.CurrentWindow->DC.CursorPos.y, 4.0f) + size.y;
    return size;
}

float ImGui::CalcWrapWidthForPos(const ImVec2& pos, float wrap_pos_x)
{
    if (wrap_pos_x < 0.0f)
        return 0.0f;

    ImGuiWindow* window = GetCurrentWindowRead();
    if (wrap_pos_x == 0.0f)
        wrap_pos_x = GetContentRegionMax().x + window->Pos.x;
    else if (wrap_pos_x > 0.0f)
        wrap_pos_x += window->Pos.x - window->Scroll.x; // wrap_pos_x is provided is window local space

    return ImMax(wrap_pos_x - pos.x, 1.0f);
}

//-----------------------------------------------------------------------------

void* ImGui::MemAlloc(size_t sz)
{
    GImGui->IO.MetricsAllocs++;
    return GImGui->IO.MemAllocFn(sz);
}

void ImGui::MemFree(void* ptr)
{
    if (ptr) GImGui->IO.MetricsAllocs--;
    return GImGui->IO.MemFreeFn(ptr);
}

const char* ImGui::GetClipboardText()
{
    return GImGui->IO.GetClipboardTextFn ? GImGui->IO.GetClipboardTextFn(GImGui->IO.ClipboardUserData) : "";
}

void ImGui::SetClipboardText(const char* text)
{
    if (GImGui->IO.SetClipboardTextFn)
        GImGui->IO.SetClipboardTextFn(GImGui->IO.ClipboardUserData, text);
}

const char* ImGui::GetVersion()
{
    return IMGUI_VERSION;
}

// Internal state access - if you want to share ImGui state between modules (e.g. DLL) or allocate it yourself
// Note that we still point to some static data and members (such as GFontAtlas), so the state instance you end up using will point to the static data within its module
ImGuiContext* ImGui::GetCurrentContext()
{
    return GImGui;
}

void ImGui::SetCurrentContext(ImGuiContext* ctx)
{
#ifdef IMGUI_SET_CURRENT_CONTEXT_FUNC
    IMGUI_SET_CURRENT_CONTEXT_FUNC(ctx); // For custom thread-based hackery you may want to have control over this.
#else
    GImGui = ctx;
#endif
}

ImGuiContext* ImGui::CreateContext(void* (*malloc_fn)(size_t), void (*free_fn)(void*))
{
    if (!malloc_fn) malloc_fn = malloc;
    ImGuiContext* ctx = (ImGuiContext*)malloc_fn(sizeof(ImGuiContext));
    IM_PLACEMENT_NEW(ctx) ImGuiContext();
    ctx->IO.MemAllocFn = malloc_fn;
    ctx->IO.MemFreeFn = free_fn ? free_fn : free;
    return ctx;
}

void ImGui::DestroyContext(ImGuiContext* ctx)
{
    void (*free_fn)(void*) = ctx->IO.MemFreeFn;
    ctx->~ImGuiContext();
    free_fn(ctx);
    if (GImGui == ctx)
        SetCurrentContext(NULL);
}

ImGuiIO& ImGui::GetIO()
{
    return GImGui->IO;
}

ImGuiStyle& ImGui::GetStyle()
{
    return GImGui->Style;
}

// Same value as passed to your RenderDrawListsFn() function. valid after Render() and until the next call to NewFrame()
ImDrawData* ImGui::GetDrawData()
{
    return GImGui->RenderDrawData.Valid ? &GImGui->RenderDrawData : NULL;
}

float ImGui::GetTime()
{
    return GImGui->Time;
}

int ImGui::GetFrameCount()
{
    return GImGui->FrameCount;
}

void ImGui::NewFrame()
{
    ImGuiContext& g = *GImGui;

    // Check user data
    IM_ASSERT(g.IO.DeltaTime >= 0.0f);               // Need a positive DeltaTime (zero is tolerated but will cause some timing issues)
    IM_ASSERT(g.IO.DisplaySize.x >= 0.0f && g.IO.DisplaySize.y >= 0.0f);
    IM_ASSERT(g.IO.Fonts->Fonts.Size > 0);           // Font Atlas not created. Did you call io.Fonts->GetTexDataAsRGBA32 / GetTexDataAsAlpha8 ?
    IM_ASSERT(g.IO.Fonts->Fonts[0]->IsLoaded());     // Font Atlas not created. Did you call io.Fonts->GetTexDataAsRGBA32 / GetTexDataAsAlpha8 ?
    IM_ASSERT(g.Style.CurveTessellationTol > 0.0f);  // Invalid style setting
    IM_ASSERT(g.Style.Alpha >= 0.0f && g.Style.Alpha <= 1.0f);  // Invalid style setting. Alpha cannot be negative (allows us to avoid a few clamps in color computations)

    // Initialize on first frame
    if (!g.Initialized)
        ImGui::Initialize();

    SetCurrentFont(GetDefaultFont());
    IM_ASSERT(g.Font->IsLoaded());

    g.Time += g.IO.DeltaTime;
    g.FrameCount += 1;
    g.TooltipOverrideCount = 0;
    g.OverlayDrawList.Clear();
    g.OverlayDrawList.PushTextureID(g.IO.Fonts->TexID);
    g.OverlayDrawList.PushClipRectFullScreen();

    // Mark rendering data as invalid to prevent user who may have a handle on it to use it
    g.RenderDrawData.Valid = false;
    g.RenderDrawData.CmdLists = NULL;
    g.RenderDrawData.CmdListsCount = g.RenderDrawData.TotalVtxCount = g.RenderDrawData.TotalIdxCount = 0;

    // Clear reference to active widget if the widget isn't alive anymore
    g.HoveredIdPreviousFrame = g.HoveredId;
    g.HoveredId = 0;
    g.HoveredIdAllowOverlap = false;
    if (!g.ActiveIdIsAlive && g.ActiveIdPreviousFrame == g.ActiveId && g.ActiveId != 0)
        ClearActiveID();
    g.ActiveIdPreviousFrame = g.ActiveId;
    g.ActiveIdIsAlive = false;
    g.ActiveIdIsJustActivated = false;
    if (g.ScalarAsInputTextId && g.ActiveId != g.ScalarAsInputTextId)
        g.ScalarAsInputTextId = 0;

    // Update keyboard input state
    memcpy(g.IO.KeysDownDurationPrev, g.IO.KeysDownDuration, sizeof(g.IO.KeysDownDuration));
    for (int i = 0; i < IM_ARRAYSIZE(g.IO.KeysDown); i++)
        g.IO.KeysDownDuration[i] = g.IO.KeysDown[i] ? (g.IO.KeysDownDuration[i] < 0.0f ? 0.0f : g.IO.KeysDownDuration[i] + g.IO.DeltaTime) : -1.0f;

    // Update mouse input state
    // If mouse just appeared or disappeared (usually denoted by -FLT_MAX component, but in reality we test for -256000.0f) we cancel out movement in MouseDelta
    if (IsMousePosValid(&g.IO.MousePos) && IsMousePosValid(&g.IO.MousePosPrev))
        g.IO.MouseDelta = g.IO.MousePos - g.IO.MousePosPrev;
    else
        g.IO.MouseDelta = ImVec2(0.0f, 0.0f);
    g.IO.MousePosPrev = g.IO.MousePos;
    for (int i = 0; i < IM_ARRAYSIZE(g.IO.MouseDown); i++)
    {
        g.IO.MouseClicked[i] = g.IO.MouseDown[i] && g.IO.MouseDownDuration[i] < 0.0f;
        g.IO.MouseReleased[i] = !g.IO.MouseDown[i] && g.IO.MouseDownDuration[i] >= 0.0f;
        g.IO.MouseDownDurationPrev[i] = g.IO.MouseDownDuration[i];
        g.IO.MouseDownDuration[i] = g.IO.MouseDown[i] ? (g.IO.MouseDownDuration[i] < 0.0f ? 0.0f : g.IO.MouseDownDuration[i] + g.IO.DeltaTime) : -1.0f;
        g.IO.MouseDoubleClicked[i] = false;
        if (g.IO.MouseClicked[i])
        {
            if (g.Time - g.IO.MouseClickedTime[i] < g.IO.MouseDoubleClickTime)
            {
                if (ImLengthSqr(g.IO.MousePos - g.IO.MouseClickedPos[i]) < g.IO.MouseDoubleClickMaxDist * g.IO.MouseDoubleClickMaxDist)
                    g.IO.MouseDoubleClicked[i] = true;
                g.IO.MouseClickedTime[i] = -FLT_MAX;    // so the third click isn't turned into a double-click
            }
            else
            {
                g.IO.MouseClickedTime[i] = g.Time;
            }
            g.IO.MouseClickedPos[i] = g.IO.MousePos;
            g.IO.MouseDragMaxDistanceSqr[i] = 0.0f;
        }
        else if (g.IO.MouseDown[i])
        {
            g.IO.MouseDragMaxDistanceSqr[i] = ImMax(g.IO.MouseDragMaxDistanceSqr[i], ImLengthSqr(g.IO.MousePos - g.IO.MouseClickedPos[i]));
        }
    }

    // Calculate frame-rate for the user, as a purely luxurious feature
    g.FramerateSecPerFrameAccum += g.IO.DeltaTime - g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx];
    g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx] = g.IO.DeltaTime;
    g.FramerateSecPerFrameIdx = (g.FramerateSecPerFrameIdx + 1) % IM_ARRAYSIZE(g.FramerateSecPerFrame);
    g.IO.Framerate = 1.0f / (g.FramerateSecPerFrameAccum / (float)IM_ARRAYSIZE(g.FramerateSecPerFrame));

	// Handle the disabling of the move action if window has the required flag.
	if (g.MovedWindow)
	{
		if (g.MovedWindow->RootWindow->Flags & ImGuiWindowFlags_OnlyDragByTitleBar)
		{
			ImRect title_rect = g.MovedWindow->RootWindow->TitleBarRect();
			bool mouse_down = g.IO.MouseDown[0];
			if (mouse_down && IsMouseHoveringRect(title_rect.Min, title_rect.Max, false))
			{
				g.MovedWindow->RootWindow->StartedDragging = true;
			}
			else if (!mouse_down || !g.MovedWindow->RootWindow->StartedDragging)
			{
				g.MovedWindow->RootWindow->StartedDragging = false;
				g.MovedWindow = NULL;
				g.MovedWindowMoveId = 0;
			}
		}
	}

    // Handle user moving window with mouse (at the beginning of the frame to avoid input lag or sheering). Only valid for root windows.
    if (g.MovedWindowMoveId && g.MovedWindowMoveId == g.ActiveId)
    {
        KeepAliveID(g.MovedWindowMoveId);
        IM_ASSERT(g.MovedWindow && g.MovedWindow->RootWindow);
        IM_ASSERT(g.MovedWindow->MoveId == g.MovedWindowMoveId);
        if (g.IO.MouseDown[0])
        {
            g.MovedWindow->RootWindow->PosFloat += g.IO.MouseDelta;
            if (g.IO.MouseDelta.x != 0.0f || g.IO.MouseDelta.y != 0.0f)
                MarkIniSettingsDirty(g.MovedWindow->RootWindow);
            FocusWindow(g.MovedWindow);
        }
        else
        {
            ClearActiveID();
            g.MovedWindow = NULL;
            g.MovedWindowMoveId = 0;
        }
    }
    else
    {
        g.MovedWindow = NULL;
        g.MovedWindowMoveId = 0;
    }

    // Delay saving settings so we don't spam disk too much
    if (g.SettingsDirtyTimer > 0.0f)
    {
        g.SettingsDirtyTimer -= g.IO.DeltaTime;
        if (g.SettingsDirtyTimer <= 0.0f)
            SaveIniSettingsToDisk(g.IO.IniFilename);
    }

    // Find the window we are hovering. Child windows can extend beyond the limit of their parent so we need to derive HoveredRootWindow from HoveredWindow
    g.HoveredWindow = g.MovedWindow ? g.MovedWindow : FindHoveredWindow(g.IO.MousePos, false);
    if (g.HoveredWindow && (g.HoveredWindow->Flags & ImGuiWindowFlags_ChildWindow))
        g.HoveredRootWindow = g.HoveredWindow->RootWindow;
    else
        g.HoveredRootWindow = g.MovedWindow ? g.MovedWindow->RootWindow : FindHoveredWindow(g.IO.MousePos, true);

    if (ImGuiWindow* modal_window = GetFrontMostModalRootWindow())
    {
        g.ModalWindowDarkeningRatio = ImMin(g.ModalWindowDarkeningRatio + g.IO.DeltaTime * 6.0f, 1.0f);
        ImGuiWindow* window = g.HoveredRootWindow;
        while (window && window != modal_window)
            window = window->ParentWindow;
        if (!window)
            g.HoveredRootWindow = g.HoveredWindow = NULL;
    }
    else
    {
        g.ModalWindowDarkeningRatio = 0.0f;
    }

    // Are we using inputs? Tell user so they can capture/discard the inputs away from the rest of their application.
    // When clicking outside of a window we assume the click is owned by the application and won't request capture. We need to track click ownership.
    int mouse_earliest_button_down = -1;
    bool mouse_any_down = false;
    for (int i = 0; i < IM_ARRAYSIZE(g.IO.MouseDown); i++)
    {
        if (g.IO.MouseClicked[i])
            g.IO.MouseDownOwned[i] = (g.HoveredWindow != NULL) || (!g.OpenPopupStack.empty());
        mouse_any_down |= g.IO.MouseDown[i];
        if (g.IO.MouseDown[i])
            if (mouse_earliest_button_down == -1 || g.IO.MouseClickedTime[mouse_earliest_button_down] > g.IO.MouseClickedTime[i])
                mouse_earliest_button_down = i;
    }
    bool mouse_avail_to_imgui = (mouse_earliest_button_down == -1) || g.IO.MouseDownOwned[mouse_earliest_button_down];
    if (g.WantCaptureMouseNextFrame != -1)
        g.IO.WantCaptureMouse = (g.WantCaptureMouseNextFrame != 0);
    else
        g.IO.WantCaptureMouse = (mouse_avail_to_imgui && (g.HoveredWindow != NULL || mouse_any_down)) || (!g.OpenPopupStack.empty());
    g.IO.WantCaptureKeyboard = (g.WantCaptureKeyboardNextFrame != -1) ? (g.WantCaptureKeyboardNextFrame != 0) : (g.ActiveId != 0);
    g.IO.WantTextInput = (g.WantTextInputNextFrame != -1) ? (g.WantTextInputNextFrame != 0) : 0;
    g.MouseCursor = ImGuiMouseCursor_Arrow;
    g.WantCaptureMouseNextFrame = g.WantCaptureKeyboardNextFrame = g.WantTextInputNextFrame = -1;
    g.OsImePosRequest = ImVec2(1.0f, 1.0f); // OS Input Method Editor showing on top-left of our window by default

    // If mouse was first clicked outside of ImGui bounds we also cancel out hovering.
    if (!mouse_avail_to_imgui)
        g.HoveredWindow = g.HoveredRootWindow = NULL;

    // Scale & Scrolling
    if (g.HoveredWindow && g.IO.MouseWheel != 0.0f && !g.HoveredWindow->Collapsed)
    {
        ImGuiWindow* window = g.HoveredWindow;
        if (g.IO.KeyCtrl && g.IO.FontAllowUserScaling)
        {
            // Zoom / Scale window
            const float new_font_scale = ImClamp(window->FontWindowScale + g.IO.MouseWheel * 0.10f, 0.50f, 2.50f);
            const float scale = new_font_scale / window->FontWindowScale;
            window->FontWindowScale = new_font_scale;

            const ImVec2 offset = window->Size * (1.0f - scale) * (g.IO.MousePos - window->Pos) / window->Size;
            window->Pos += offset;
            window->PosFloat += offset;
            window->Size *= scale;
            window->SizeFull *= scale;
        }
        else if (!g.IO.KeyCtrl && !(window->Flags & ImGuiWindowFlags_NoScrollWithMouse))
        {
            // Scroll
            const int scroll_lines = (window->Flags & ImGuiWindowFlags_ComboBox) ? 3 : 5;
            SetWindowScrollY(window, window->Scroll.y - g.IO.MouseWheel * window->CalcFontSize() * scroll_lines);
        }
    }

    // Pressing TAB activate widget focus
    if (g.ActiveId == 0 && g.NavWindow != NULL && g.NavWindow->Active && IsKeyPressedMap(ImGuiKey_Tab, false))
        g.NavWindow->FocusIdxTabRequestNext = 0;

    // Mark all windows as not visible
    for (int i = 0; i != g.Windows.Size; i++)
    {
        ImGuiWindow* window = g.Windows[i];
        window->WasActive = window->Active;
        window->Active = false;
        window->Accessed = false;
    }

    // Closing the focused window restore focus to the first active root window in descending z-order
    if (g.NavWindow && !g.NavWindow->WasActive)
        for (int i = g.Windows.Size-1; i >= 0; i--)
            if (g.Windows[i]->WasActive && !(g.Windows[i]->Flags & ImGuiWindowFlags_ChildWindow))
            {
                FocusWindow(g.Windows[i]);
                break;
            }

    // No window should be open at the beginning of the frame.
    // But in order to allow the user to call NewFrame() multiple times without calling Render(), we are doing an explicit clear.
    g.CurrentWindowStack.resize(0);
    g.CurrentPopupStack.resize(0);
    CloseInactivePopups();

    // Create implicit window - we will only render it if the user has added something to it.
    // We don't use "Debug" to avoid colliding with user trying to create a "Debug" window with custom flags.
    ImGui::SetNextWindowSize(ImVec2(400,400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Debug##Default");
}

void ImGui::Initialize()
{
    ImGuiContext& g = *GImGui;
    g.LogClipboard = (ImGuiTextBuffer*)ImGui::MemAlloc(sizeof(ImGuiTextBuffer));
    IM_PLACEMENT_NEW(g.LogClipboard) ImGuiTextBuffer();

    IM_ASSERT(g.Settings.empty());
    LoadIniSettingsFromDisk(g.IO.IniFilename);
    g.Initialized = true;
}

// This function is merely here to free heap allocations.
void ImGui::Shutdown()
{
    ImGuiContext& g = *GImGui;

    // The fonts atlas can be used prior to calling NewFrame(), so we clear it even if g.Initialized is FALSE (which would happen if we never called NewFrame)
    if (g.IO.Fonts) // Testing for NULL to allow user to NULLify in case of running Shutdown() on multiple contexts. Bit hacky.
        g.IO.Fonts->Clear();

    // Cleanup of other data are conditional on actually having initialize ImGui.
    if (!g.Initialized)
        return;

    SaveIniSettingsToDisk(g.IO.IniFilename);

    for (int i = 0; i < g.Windows.Size; i++)
    {
        g.Windows[i]->~ImGuiWindow();
        ImGui::MemFree(g.Windows[i]);
    }
    g.Windows.clear();
    g.WindowsSortBuffer.clear();
    g.CurrentWindow = NULL;
    g.CurrentWindowStack.clear();
    g.NavWindow = NULL;
    g.HoveredWindow = NULL;
    g.HoveredRootWindow = NULL;
    g.ActiveIdWindow = NULL;
    g.MovedWindow = NULL;
    for (int i = 0; i < g.Settings.Size; i++)
        ImGui::MemFree(g.Settings[i].Name);
    g.Settings.clear();
    g.ColorModifiers.clear();
    g.StyleModifiers.clear();
    g.FontStack.clear();
    g.OpenPopupStack.clear();
    g.CurrentPopupStack.clear();
    g.SetNextWindowSizeConstraintCallback = NULL;
    g.SetNextWindowSizeConstraintCallbackUserData = NULL;
    for (int i = 0; i < IM_ARRAYSIZE(g.RenderDrawLists); i++)
        g.RenderDrawLists[i].clear();
    g.OverlayDrawList.ClearFreeMemory();
    g.PrivateClipboard.clear();
    g.InputTextState.Text.clear();
    g.InputTextState.InitialText.clear();
    g.InputTextState.TempTextBuffer.clear();

    if (g.LogFile && g.LogFile != stdout)
    {
        fclose(g.LogFile);
        g.LogFile = NULL;
    }
    if (g.LogClipboard)
    {
        g.LogClipboard->~ImGuiTextBuffer();
        ImGui::MemFree(g.LogClipboard);
    }

    g.Initialized = false;
}

static ImGuiIniData* FindWindowSettings(const char* name)
{
    ImGuiContext& g = *GImGui;
    ImGuiID id = ImHash(name, 0);
    for (int i = 0; i != g.Settings.Size; i++)
    {
        ImGuiIniData* ini = &g.Settings[i];
        if (ini->Id == id)
            return ini;
    }
    return NULL;
}

static ImGuiIniData* AddWindowSettings(const char* name)
{
    GImGui->Settings.resize(GImGui->Settings.Size + 1);
    ImGuiIniData* ini = &GImGui->Settings.back();
    ini->Name = ImStrdup(name);
    ini->Id = ImHash(name, 0);
    ini->Collapsed = false;
    ini->Pos = ImVec2(FLT_MAX,FLT_MAX);
    ini->Size = ImVec2(0,0);
    return ini;
}

// Zero-tolerance, poor-man .ini parsing
// FIXME: Write something less rubbish
static void LoadIniSettingsFromDisk(const char* ini_filename)
{
    ImGuiContext& g = *GImGui;
    if (!ini_filename)
        return;

    int file_size;
    char* file_data = (char*)ImFileLoadToMemory(ini_filename, "rb", &file_size, 1);
    if (!file_data)
        return;

    ImGuiIniData* settings = NULL;
    const char* buf_end = file_data + file_size;
    for (const char* line_start = file_data; line_start < buf_end; )
    {
        const char* line_end = line_start;
        while (line_end < buf_end && *line_end != '\n' && *line_end != '\r')
            line_end++;

        if (line_start[0] == '[' && line_end > line_start && line_end[-1] == ']')
        {
            char name[64];
            ImFormatString(name, IM_ARRAYSIZE(name), "%.*s", (int)(line_end-line_start-2), line_start+1);
            settings = FindWindowSettings(name);
            if (!settings)
                settings = AddWindowSettings(name);
        }
        else if (settings)
        {
            float x, y;
            int i;
            if (sscanf(line_start, "Pos=%f,%f", &x, &y) == 2)
                settings->Pos = ImVec2(x, y);
            else if (sscanf(line_start, "Size=%f,%f", &x, &y) == 2)
                settings->Size = ImMax(ImVec2(x, y), g.Style.WindowMinSize);
            else if (sscanf(line_start, "Collapsed=%d", &i) == 1)
                settings->Collapsed = (i != 0);
        }

        line_start = line_end+1;
    }

    ImGui::MemFree(file_data);
}

static void SaveIniSettingsToDisk(const char* ini_filename)
{
    ImGuiContext& g = *GImGui;
    g.SettingsDirtyTimer = 0.0f;
    if (!ini_filename)
        return;

    // Gather data from windows that were active during this session
    for (int i = 0; i != g.Windows.Size; i++)
    {
        ImGuiWindow* window = g.Windows[i];
        if (window->Flags & ImGuiWindowFlags_NoSavedSettings)
            continue;
        ImGuiIniData* settings = FindWindowSettings(window->Name);
        if (!settings)  // This will only return NULL in the rare instance where the window was first created with ImGuiWindowFlags_NoSavedSettings then had the flag disabled later on. We don't bind settings in this case (bug #1000).
            continue;
        settings->Pos = window->Pos;
        settings->Size = window->SizeFull;
        settings->Collapsed = window->Collapsed;
    }

    // Write .ini file
    // If a window wasn't opened in this session we preserve its settings
    FILE* f = ImFileOpen(ini_filename, "wt");
    if (!f)
        return;
    for (int i = 0; i != g.Settings.Size; i++)
    {
        const ImGuiIniData* settings = &g.Settings[i];
        if (settings->Pos.x == FLT_MAX)
            continue;
        const char* name = settings->Name;
        if (const char* p = strstr(name, "###"))  // Skip to the "###" marker if any. We don't skip past to match the behavior of GetID()
            name = p;
        fprintf(f, "[%s]\n", name);
        fprintf(f, "Pos=%d,%d\n", (int)settings->Pos.x, (int)settings->Pos.y);
        fprintf(f, "Size=%d,%d\n", (int)settings->Size.x, (int)settings->Size.y);
        fprintf(f, "Collapsed=%d\n", settings->Collapsed);
        fprintf(f, "\n");
    }

    fclose(f);
}

static void MarkIniSettingsDirty(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (!(window->Flags & ImGuiWindowFlags_NoSavedSettings))
        if (g.SettingsDirtyTimer <= 0.0f)
            g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

// FIXME: Add a more explicit sort order in the window structure.
static int ChildWindowComparer(const void* lhs, const void* rhs)
{
    const ImGuiWindow* a = *(const ImGuiWindow**)lhs;
    const ImGuiWindow* b = *(const ImGuiWindow**)rhs;
    if (int d = (a->Flags & ImGuiWindowFlags_Popup) - (b->Flags & ImGuiWindowFlags_Popup))
        return d;
    if (int d = (a->Flags & ImGuiWindowFlags_Tooltip) - (b->Flags & ImGuiWindowFlags_Tooltip))
        return d;
    if (int d = (a->Flags & ImGuiWindowFlags_ComboBox) - (b->Flags & ImGuiWindowFlags_ComboBox))
        return d;
    return (a->OrderWithinParent - b->OrderWithinParent);
}

static void AddWindowToSortedBuffer(ImVector<ImGuiWindow*>& out_sorted_windows, ImGuiWindow* window)
{
    out_sorted_windows.push_back(window);
    if (window->Active)
    {
        int count = window->DC.ChildWindows.Size;
        if (count > 1)
            qsort(window->DC.ChildWindows.begin(), (size_t)count, sizeof(ImGuiWindow*), ChildWindowComparer);
        for (int i = 0; i < count; i++)
        {
            ImGuiWindow* child = window->DC.ChildWindows[i];
            if (child->Active)
                AddWindowToSortedBuffer(out_sorted_windows, child);
        }
    }
}

static void AddDrawListToRenderList(ImVector<ImDrawList*>& out_render_list, ImDrawList* draw_list)
{
    if (draw_list->CmdBuffer.empty())
        return;

    // Remove trailing command if unused
    ImDrawCmd& last_cmd = draw_list->CmdBuffer.back();
    if (last_cmd.ElemCount == 0 && last_cmd.UserCallback == NULL)
    {
        draw_list->CmdBuffer.pop_back();
        if (draw_list->CmdBuffer.empty())
            return;
    }

    // Draw list sanity check. Detect mismatch between PrimReserve() calls and incrementing _VtxCurrentIdx, _VtxWritePtr etc. May trigger for you if you are using PrimXXX functions incorrectly.
    IM_ASSERT(draw_list->VtxBuffer.Size == 0 || draw_list->_VtxWritePtr == draw_list->VtxBuffer.Data + draw_list->VtxBuffer.Size);
    IM_ASSERT(draw_list->IdxBuffer.Size == 0 || draw_list->_IdxWritePtr == draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size);
    IM_ASSERT((int)draw_list->_VtxCurrentIdx == draw_list->VtxBuffer.Size);

    // Check that draw_list doesn't use more vertices than indexable in a single draw call (default ImDrawIdx = unsigned short = 2 bytes = 64K vertices per window)
    // If this assert triggers because you are drawing lots of stuff manually, you can:
    // A) Add '#define ImDrawIdx unsigned int' in imconfig.h to set the index size to 4 bytes. You'll need to handle the 4-bytes indices to your renderer.
    //    For example, the OpenGL example code detect index size at compile-time by doing:
    //    'glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);'
    //    Your own engine or render API may use different parameters or function calls to specify index sizes. 2 and 4 bytes indices are generally supported by most API.
    // B) If for some reason you cannot use 4 bytes indices or don't want to, a workaround is to call BeginChild()/EndChild() before reaching the 64K limit to split your draw commands in multiple draw lists.
    IM_ASSERT(((ImU64)draw_list->_VtxCurrentIdx >> (sizeof(ImDrawIdx)*8)) == 0);  // Too many vertices in same ImDrawList. See comment above.
    
    out_render_list.push_back(draw_list);
    GImGui->IO.MetricsRenderVertices += draw_list->VtxBuffer.Size;
    GImGui->IO.MetricsRenderIndices += draw_list->IdxBuffer.Size;
}

static void AddWindowToRenderList(ImVector<ImDrawList*>& out_render_list, ImGuiWindow* window)
{
    AddDrawListToRenderList(out_render_list, window->DrawList);
    for (int i = 0; i < window->DC.ChildWindows.Size; i++)
    {
        ImGuiWindow* child = window->DC.ChildWindows[i];
        if (!child->Active) // clipped children may have been marked not active
            continue;
        if ((child->Flags & ImGuiWindowFlags_Popup) && child->HiddenFrames > 0)
            continue;
        AddWindowToRenderList(out_render_list, child);
    }
}

static void AddWindowToRenderListSelectLayer(ImGuiWindow* window)
{
    // FIXME: Generalize this with a proper layering system so e.g. user can draw in specific layers, below text, ..
    ImGuiContext& g = *GImGui;
    g.IO.MetricsActiveWindows++;
    if (window->Flags & ImGuiWindowFlags_Popup)
        AddWindowToRenderList(g.RenderDrawLists[1], window);
    else if (window->Flags & ImGuiWindowFlags_Tooltip)
        AddWindowToRenderList(g.RenderDrawLists[2], window);
    else
        AddWindowToRenderList(g.RenderDrawLists[0], window);
}

// When using this function it is sane to ensure that float are perfectly rounded to integer values, to that e.g. (int)(max.x-min.x) in user's render produce correct result.
void ImGui::PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DrawList->PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    window->ClipRect = window->DrawList->_ClipRectStack.back();
}

void ImGui::PopClipRect()
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DrawList->PopClipRect();
    window->ClipRect = window->DrawList->_ClipRectStack.back();
}

// This is normally called by Render(). You may want to call it directly if you want to avoid calling Render() but the gain will be very minimal.
void ImGui::EndFrame()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Initialized);                       // Forgot to call ImGui::NewFrame()
    IM_ASSERT(g.FrameCountEnded != g.FrameCount);   // ImGui::EndFrame() called multiple times, or forgot to call ImGui::NewFrame() again

    // Notify OS when our Input Method Editor cursor has moved (e.g. CJK inputs using Microsoft IME)
    if (g.IO.ImeSetInputScreenPosFn && ImLengthSqr(g.OsImePosRequest - g.OsImePosSet) > 0.0001f)
    {
        g.IO.ImeSetInputScreenPosFn((int)g.OsImePosRequest.x, (int)g.OsImePosRequest.y);
        g.OsImePosSet = g.OsImePosRequest;
    }

    // Hide implicit "Debug" window if it hasn't been used
    IM_ASSERT(g.CurrentWindowStack.Size == 1);    // Mismatched Begin()/End() calls
    if (g.CurrentWindow && !g.CurrentWindow->Accessed)
        g.CurrentWindow->Active = false;
    ImGui::End();

    // Click to focus window and start moving (after we're done with all our widgets)
    if (g.ActiveId == 0 && g.HoveredId == 0 && g.IO.MouseClicked[0])
    {
        if (!(g.NavWindow && !g.NavWindow->WasActive && g.NavWindow->Active)) // Unless we just made a popup appear
        {
            if (g.HoveredRootWindow != NULL)
            {
                FocusWindow(g.HoveredWindow);
                if (!(g.HoveredWindow->Flags & ImGuiWindowFlags_NoMove))
                {
                    g.MovedWindow = g.HoveredWindow;
                    g.MovedWindowMoveId = g.HoveredWindow->MoveId;
                    SetActiveID(g.MovedWindowMoveId, g.HoveredRootWindow);
                }
            }
            else if (g.NavWindow != NULL && GetFrontMostModalRootWindow() == NULL)
            {
                // Clicking on void disable focus
                FocusWindow(NULL);
            }
        }
    }

    // Sort the window list so that all child windows are after their parent
    // We cannot do that on FocusWindow() because childs may not exist yet
    g.WindowsSortBuffer.resize(0);
    g.WindowsSortBuffer.reserve(g.Windows.Size);
    for (int i = 0; i != g.Windows.Size; i++)
    {
        ImGuiWindow* window = g.Windows[i];
        if (window->Active && (window->Flags & ImGuiWindowFlags_ChildWindow))       // if a child is active its parent will add it
            continue;
        AddWindowToSortedBuffer(g.WindowsSortBuffer, window);
    }

    IM_ASSERT(g.Windows.Size == g.WindowsSortBuffer.Size);  // we done something wrong
    g.Windows.swap(g.WindowsSortBuffer);

    // Clear Input data for next frame
    g.IO.MouseWheel = 0.0f;
    memset(g.IO.InputCharacters, 0, sizeof(g.IO.InputCharacters));

    g.FrameCountEnded = g.FrameCount;
}

void ImGui::Render()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Initialized);   // Forgot to call ImGui::NewFrame()

    if (g.FrameCountEnded != g.FrameCount)
        ImGui::EndFrame();
    g.FrameCountRendered = g.FrameCount;

    // Skip render altogether if alpha is 0.0
    // Note that vertex buffers have been created and are wasted, so it is best practice that you don't create windows in the first place, or consistently respond to Begin() returning false.
    if (g.Style.Alpha > 0.0f)
    {
        // Gather windows to render
        g.IO.MetricsRenderVertices = g.IO.MetricsRenderIndices = g.IO.MetricsActiveWindows = 0;
        for (int i = 0; i < IM_ARRAYSIZE(g.RenderDrawLists); i++)
            g.RenderDrawLists[i].resize(0);
        for (int i = 0; i != g.Windows.Size; i++)
        {
            ImGuiWindow* window = g.Windows[i];
            if (window->Active && window->HiddenFrames <= 0 && (window->Flags & (ImGuiWindowFlags_ChildWindow)) == 0)
                AddWindowToRenderListSelectLayer(window);
        }

        // Flatten layers
        int n = g.RenderDrawLists[0].Size;
        int flattened_size = n;
        for (int i = 1; i < IM_ARRAYSIZE(g.RenderDrawLists); i++)
            flattened_size += g.RenderDrawLists[i].Size;
        g.RenderDrawLists[0].resize(flattened_size);
        for (int i = 1; i < IM_ARRAYSIZE(g.RenderDrawLists); i++)
        {
            ImVector<ImDrawList*>& layer = g.RenderDrawLists[i];
            if (layer.empty())
                continue;
            memcpy(&g.RenderDrawLists[0][n], &layer[0], layer.Size * sizeof(ImDrawList*));
            n += layer.Size;
        }

        // Draw software mouse cursor if requested
        if (g.IO.MouseDrawCursor)
        {
            const ImGuiMouseCursorData& cursor_data = g.MouseCursorData[g.MouseCursor];
            const ImVec2 pos = g.IO.MousePos - cursor_data.HotOffset;
            const ImVec2 size = cursor_data.Size;
            const ImTextureID tex_id = g.IO.Fonts->TexID;
            g.OverlayDrawList.PushTextureID(tex_id);
            g.OverlayDrawList.AddImage(tex_id, pos+ImVec2(1,0), pos+ImVec2(1,0) + size, cursor_data.TexUvMin[1], cursor_data.TexUvMax[1], IM_COL32(0,0,0,48));        // Shadow
            g.OverlayDrawList.AddImage(tex_id, pos+ImVec2(2,0), pos+ImVec2(2,0) + size, cursor_data.TexUvMin[1], cursor_data.TexUvMax[1], IM_COL32(0,0,0,48));        // Shadow
            g.OverlayDrawList.AddImage(tex_id, pos,             pos + size,             cursor_data.TexUvMin[1], cursor_data.TexUvMax[1], IM_COL32(0,0,0,255));       // Black border
            g.OverlayDrawList.AddImage(tex_id, pos,             pos + size,             cursor_data.TexUvMin[0], cursor_data.TexUvMax[0], IM_COL32(0, 153, 204, 255)); // White fill IM_COL32(255,255,255,255)
            g.OverlayDrawList.PopTextureID();
        }
        if (!g.OverlayDrawList.VtxBuffer.empty())
            AddDrawListToRenderList(g.RenderDrawLists[0], &g.OverlayDrawList);

        // Setup draw data
        g.RenderDrawData.Valid = true;
        g.RenderDrawData.CmdLists = (g.RenderDrawLists[0].Size > 0) ? &g.RenderDrawLists[0][0] : NULL;
        g.RenderDrawData.CmdListsCount = g.RenderDrawLists[0].Size;
        g.RenderDrawData.TotalVtxCount = g.IO.MetricsRenderVertices;
        g.RenderDrawData.TotalIdxCount = g.IO.MetricsRenderIndices;

        // Render. If user hasn't set a callback then they may retrieve the draw data via GetDrawData()
        if (g.RenderDrawData.CmdListsCount > 0 && g.IO.RenderDrawListsFn != NULL)
            g.IO.RenderDrawListsFn(&g.RenderDrawData);
    }
}

const char* ImGui::FindRenderedTextEnd(const char* text, const char* text_end)
{
    const char* text_display_end = text;
    if (!text_end)
        text_end = (const char*)-1;

    while (text_display_end < text_end && *text_display_end != '\0' && (text_display_end[0] != '#' || text_display_end[1] != '#'))
        text_display_end++;
    return text_display_end;
}

// Pass text data straight to log (without being displayed)
void ImGui::LogText(const char* fmt, ...)
{
    ImGuiContext& g = *GImGui;
    if (!g.LogEnabled)
        return;

    va_list args;
    va_start(args, fmt);
    if (g.LogFile)
    {
        vfprintf(g.LogFile, fmt, args);
    }
    else
    {
        g.LogClipboard->appendv(fmt, args);
    }
    va_end(args);
}

// Internal version that takes a position to decide on newline placement and pad items according to their depth.
// We split text into individual lines to add current tree level padding
static void LogRenderedText(const ImVec2* ref_pos, const char* text, const char* text_end = NULL)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    if (!text_end)
        text_end = ImGui::FindRenderedTextEnd(text, text_end);

    const bool log_new_line = ref_pos && (ref_pos->y > window->DC.LogLinePosY + 1);
    if (ref_pos)
        window->DC.LogLinePosY = ref_pos->y;

    const char* text_remaining = text;
    if (g.LogStartDepth > window->DC.TreeDepth)  // Re-adjust padding if we have popped out of our starting depth
        g.LogStartDepth = window->DC.TreeDepth;
    const int tree_depth = (window->DC.TreeDepth - g.LogStartDepth);
    for (;;)
    {
        // Split the string. Each new line (after a '\n') is followed by spacing corresponding to the current depth of our log entry.
        const char* line_end = text_remaining;
        while (line_end < text_end)
            if (*line_end == '\n')
                break;
            else
                line_end++;
        if (line_end >= text_end)
            line_end = NULL;

        const bool is_first_line = (text == text_remaining);
        bool is_last_line = false;
        if (line_end == NULL)
        {
            is_last_line = true;
            line_end = text_end;
        }
        if (line_end != NULL && !(is_last_line && (line_end - text_remaining)==0))
        {
            const int char_count = (int)(line_end - text_remaining);
            if (log_new_line || !is_first_line)
                ImGui::LogText(IM_NEWLINE "%*s%.*s", tree_depth*4, "", char_count, text_remaining);
            else
                ImGui::LogText(" %.*s", char_count, text_remaining);
        }

        if (is_last_line)
            break;
        text_remaining = line_end + 1;
    }
}

// Internal ImGui functions to render text
// RenderText***() functions calls ImDrawList::AddText() calls ImBitmapFont::RenderText()
void ImGui::RenderText(ImVec2 pos, const char* text, const char* text_end, bool hide_text_after_hash)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Hide anything after a '##' string
    const char* text_display_end;
    if (hide_text_after_hash)
    {
        text_display_end = FindRenderedTextEnd(text, text_end);
    }
    else
    {
        if (!text_end)
            text_end = text + strlen(text); // FIXME-OPT
        text_display_end = text_end;
    }

    const int text_len = (int)(text_display_end - text);
    if (text_len > 0)
    {
        window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_display_end);
        if (g.LogEnabled)
            LogRenderedText(&pos, text, text_display_end);
    }
}

void ImGui::RenderTextWrapped(ImVec2 pos, const char* text, const char* text_end, float wrap_width)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    if (!text_end)
        text_end = text + strlen(text); // FIXME-OPT

    const int text_len = (int)(text_end - text);
    if (text_len > 0)
    {
        window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_end, wrap_width);
        if (g.LogEnabled)
            LogRenderedText(&pos, text, text_end);
    }
}

// Default clip_rect uses (pos_min,pos_max)
// Handle clipping on CPU immediately (vs typically let the GPU clip the triangles that are overlapping the clipping rectangle edges)
void ImGui::RenderTextClipped(const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect)
{
    // Hide anything after a '##' string
    const char* text_display_end = FindRenderedTextEnd(text, text_end);
    const int text_len = (int)(text_display_end - text);
    if (text_len == 0)
        return;

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Perform CPU side clipping for single clipped element to avoid using scissor state
    ImVec2 pos = pos_min;
    const ImVec2 text_size = text_size_if_known ? *text_size_if_known : CalcTextSize(text, text_display_end, false, 0.0f);

    const ImVec2* clip_min = clip_rect ? &clip_rect->Min : &pos_min;
    const ImVec2* clip_max = clip_rect ? &clip_rect->Max : &pos_max;
    bool need_clipping = (pos.x + text_size.x >= clip_max->x) || (pos.y + text_size.y >= clip_max->y);
    if (clip_rect) // If we had no explicit clipping rectangle then pos==clip_min
        need_clipping |= (pos.x < clip_min->x) || (pos.y < clip_min->y);

    // Align whole block. We should defer that to the better rendering function when we'll have support for individual line alignment.
    if (align.x > 0.0f) pos.x = ImMax(pos.x, pos.x + (pos_max.x - pos.x - text_size.x) * align.x);
    if (align.y > 0.0f) pos.y = ImMax(pos.y, pos.y + (pos_max.y - pos.y - text_size.y) * align.y);

    // Render
    if (need_clipping)
    {
        ImVec4 fine_clip_rect(clip_min->x, clip_min->y, clip_max->x, clip_max->y);
        window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_display_end, 0.0f, &fine_clip_rect);
    }
    else
    {
        window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_display_end, 0.0f, NULL);
    }
    if (g.LogEnabled)
        LogRenderedText(&pos, text, text_display_end);
}

// Render a rectangle shaped with optional rounding and borders
void ImGui::RenderFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    window->DrawList->AddRectFilled(p_min, p_max, fill_col, rounding);
    if (border && (window->Flags & ImGuiWindowFlags_ShowBorders))
    {
        window->DrawList->AddRect(p_min+ImVec2(1,1), p_max+ImVec2(1,1), GetColorU32(ImGuiCol_BorderShadow), rounding);
        window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding);
    }
}

void ImGui::RenderFrameBorder(ImVec2 p_min, ImVec2 p_max, float rounding)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->Flags & ImGuiWindowFlags_ShowBorders)
    {
        window->DrawList->AddRect(p_min+ImVec2(1,1), p_max+ImVec2(1,1), GetColorU32(ImGuiCol_BorderShadow), rounding);
        window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding);
    }
}

// Render a triangle to denote expanded/collapsed state
void ImGui::RenderTriangle(ImVec2 p_min, ImGuiDir dir, float scale)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const float h = g.FontSize * 1.00f;
    float r = h * 0.40f * scale;
    ImVec2 center = p_min + ImVec2(h * 0.50f, h * 0.50f * scale);

    ImVec2 a, b, c;
    switch (dir)
    {
    case ImGuiDir_Up:
        r = -r; // ...fall through, no break!
    case ImGuiDir_Down:
        center.y -= r * 0.25f;
        a = ImVec2(0,1) * r;
        b = ImVec2(-0.866f,-0.5f) * r;
        c = ImVec2(+0.866f,-0.5f) * r;
        break;
    case ImGuiDir_Left:
        r = -r; // ...fall through, no break!
    case ImGuiDir_Right:
        a = ImVec2(1,0) * r;
        b = ImVec2(-0.500f,+0.866f) * r;
        c = ImVec2(-0.500f,-0.866f) * r;
        break;
    default:
        IM_ASSERT(0);
        break;
    }

    window->DrawList->AddTriangleFilled(center + a, center + b, center + c, GetColorU32(ImGuiCol_Text));
}

void ImGui::RenderBullet(ImVec2 pos)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    window->DrawList->AddCircleFilled(pos, GImGui->FontSize*0.20f, GetColorU32(ImGuiCol_Text), 8);
}

void ImGui::RenderCheckMark(ImVec2 pos, ImU32 col, float sz)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    float thickness = ImMax(sz / 5.0f, 1.0f);
    sz -= thickness*0.5f;
    pos += ImVec2(thickness*0.25f, thickness*0.25f);

    float third = sz / 3.0f;
    float bx = pos.x + third;
    float by = pos.y + sz - third*0.5f;
    window->DrawList->PathLineTo(ImVec2(bx - third, by - third));
    window->DrawList->PathLineTo(ImVec2(bx, by));
    window->DrawList->PathLineTo(ImVec2(bx + third*2, by - third*2));
    window->DrawList->PathStroke(col, false, thickness);
}

// Calculate text size. Text can be multi-line. Optionally ignore text after a ## marker.
// CalcTextSize("") should return ImVec2(0.0f, GImGui->FontSize)
ImVec2 ImGui::CalcTextSize(const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width)
{
    ImGuiContext& g = *GImGui;

    const char* text_display_end;
    if (hide_text_after_double_hash)
        text_display_end = FindRenderedTextEnd(text, text_end);      // Hide anything after a '##' string
    else
        text_display_end = text_end;

    ImFont* font = g.Font;
    const float font_size = g.FontSize;
    if (text == text_display_end)
        return ImVec2(0.0f, font_size);
    ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, wrap_width, text, text_display_end, NULL);

    // Cancel out character spacing for the last character of a line (it is baked into glyph->AdvanceX field)
    const float font_scale = font_size / font->FontSize;
    const float character_spacing_x = 1.0f * font_scale;
    if (text_size.x > 0.0f)
        text_size.x -= character_spacing_x;
    text_size.x = (float)(int)(text_size.x + 0.95f);

    return text_size;
}

// Helper to calculate coarse clipping of large list of evenly sized items.
// NB: Prefer using the ImGuiListClipper higher-level helper if you can! Read comments and instructions there on how those use this sort of pattern.
// NB: 'items_count' is only used to clamp the result, if you don't know your count you can use INT_MAX
void ImGui::CalcListClipping(int items_count, float items_height, int* out_items_display_start, int* out_items_display_end)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (g.LogEnabled)
    {
        // If logging is active, do not perform any clipping
        *out_items_display_start = 0;
        *out_items_display_end = items_count;
        return;
    }
    if (window->SkipItems)
    {
        *out_items_display_start = *out_items_display_end = 0;
        return;
    }

    const ImVec2 pos = window->DC.CursorPos;
    int start = (int)((window->ClipRect.Min.y - pos.y) / items_height);
    int end = (int)((window->ClipRect.Max.y - pos.y) / items_height);
    start = ImClamp(start, 0, items_count);
    end = ImClamp(end + 1, start, items_count);
    *out_items_display_start = start;
    *out_items_display_end = end;
}

// Find window given position, search front-to-back
// FIXME: Note that we have a lag here because WindowRectClipped is updated in Begin() so windows moved by user via SetWindowPos() and not SetNextWindowPos() will have that rectangle lagging by a frame at the time FindHoveredWindow() is called, aka before the next Begin(). Moving window thankfully isn't affected.
static ImGuiWindow* FindHoveredWindow(ImVec2 pos, bool excluding_childs)
{
    ImGuiContext& g = *GImGui;
    for (int i = g.Windows.Size-1; i >= 0; i--)
    {
        ImGuiWindow* window = g.Windows[i];
        if (!window->Active)
            continue;
        if (window->Flags & ImGuiWindowFlags_NoInputs)
            continue;
        if (excluding_childs && (window->Flags & ImGuiWindowFlags_ChildWindow) != 0)
            continue;

        // Using the clipped AABB so a child window will typically be clipped by its parent.
        ImRect bb(window->WindowRectClipped.Min - g.Style.TouchExtraPadding, window->WindowRectClipped.Max + g.Style.TouchExtraPadding);
        if (bb.Contains(pos))
            return window;
    }
    return NULL;
}

// Test if mouse cursor is hovering given rectangle
// NB- Rectangle is clipped by our current clip setting
// NB- Expand the rectangle to be generous on imprecise inputs systems (g.Style.TouchExtraPadding)
bool ImGui::IsMouseHoveringRect(const ImVec2& r_min, const ImVec2& r_max, bool clip)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Clip
    ImRect rect_clipped(r_min, r_max);
    if (clip)
        rect_clipped.ClipWith(window->ClipRect);

    // Expand for touch input
    const ImRect rect_for_touch(rect_clipped.Min - g.Style.TouchExtraPadding, rect_clipped.Max + g.Style.TouchExtraPadding);
    return rect_for_touch.Contains(g.IO.MousePos);
}

bool ImGui::IsAnyWindowHovered()
{
    ImGuiContext& g = *GImGui;
    return g.HoveredWindow != NULL;
}

static bool IsKeyPressedMap(ImGuiKey key, bool repeat)
{
    const int key_index = GImGui->IO.KeyMap[key];
    return (key_index >= 0) ? ImGui::IsKeyPressed(key_index, repeat) : false;
}

int ImGui::GetKeyIndex(ImGuiKey imgui_key)
{
    IM_ASSERT(imgui_key >= 0 && imgui_key < ImGuiKey_COUNT);
    return GImGui->IO.KeyMap[imgui_key];
}

// Note that imgui doesn't know the semantic of each entry of io.KeyDown[]. Use your own indices/enums according to how your backend/engine stored them into KeyDown[]!
bool ImGui::IsKeyDown(int user_key_index)
{
    if (user_key_index < 0) return false;
    IM_ASSERT(user_key_index >= 0 && user_key_index < IM_ARRAYSIZE(GImGui->IO.KeysDown));
    return GImGui->IO.KeysDown[user_key_index];
}

int ImGui::CalcTypematicPressedRepeatAmount(float t, float t_prev, float repeat_delay, float repeat_rate)
{
    if (t == 0.0f)
        return 1;
    if (t <= repeat_delay || repeat_rate <= 0.0f)
        return 0;
    const int count = (int)((t - repeat_delay) / repeat_rate) - (int)((t_prev - repeat_delay) / repeat_rate);
    return (count > 0) ? count : 0;
}

int ImGui::GetKeyPressedAmount(int key_index, float repeat_delay, float repeat_rate)
{
    ImGuiContext& g = *GImGui;
    if (key_index < 0) return false;
    IM_ASSERT(key_index >= 0 && key_index < IM_ARRAYSIZE(g.IO.KeysDown));
    const float t = g.IO.KeysDownDuration[key_index];
    return CalcTypematicPressedRepeatAmount(t, t - g.IO.DeltaTime, repeat_delay, repeat_rate);
}

bool ImGui::IsKeyPressed(int user_key_index, bool repeat)
{
    ImGuiContext& g = *GImGui;
    if (user_key_index < 0) return false;
    IM_ASSERT(user_key_index >= 0 && user_key_index < IM_ARRAYSIZE(g.IO.KeysDown));
    const float t = g.IO.KeysDownDuration[user_key_index];
    if (t == 0.0f)
        return true;
    if (repeat && t > g.IO.KeyRepeatDelay)
        return GetKeyPressedAmount(user_key_index, g.IO.KeyRepeatDelay, g.IO.KeyRepeatRate) > 0;
    return false;
}

bool ImGui::IsKeyReleased(int user_key_index)
{
    ImGuiContext& g = *GImGui;
    if (user_key_index < 0) return false;
    IM_ASSERT(user_key_index >= 0 && user_key_index < IM_ARRAYSIZE(g.IO.KeysDown));
    if (g.IO.KeysDownDurationPrev[user_key_index] >= 0.0f && !g.IO.KeysDown[user_key_index])
        return true;
    return false;
}

bool ImGui::IsMouseDown(int button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
    return g.IO.MouseDown[button];
}

bool ImGui::IsMouseClicked(int button, bool repeat)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
    const float t = g.IO.MouseDownDuration[button];
    if (t == 0.0f)
        return true;

    if (repeat && t > g.IO.KeyRepeatDelay)
    {
        float delay = g.IO.KeyRepeatDelay, rate = g.IO.KeyRepeatRate;
        if ((fmodf(t - delay, rate) > rate*0.5f) != (fmodf(t - delay - g.IO.DeltaTime, rate) > rate*0.5f))
            return true;
    }

    return false;
}

bool ImGui::IsMouseReleased(int button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
    return g.IO.MouseReleased[button];
}

bool ImGui::IsMouseDoubleClicked(int button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
    return g.IO.MouseDoubleClicked[button];
}

bool ImGui::IsMouseDragging(int button, float lock_threshold)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
    if (!g.IO.MouseDown[button])
        return false;
    if (lock_threshold < 0.0f)
        lock_threshold = g.IO.MouseDragThreshold;
    return g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold;
}

ImVec2 ImGui::GetMousePos()
{
    return GImGui->IO.MousePos;
}

// NB: prefer to call right after BeginPopup(). At the time Selectable/MenuItem is activated, the popup is already closed!
ImVec2 ImGui::GetMousePosOnOpeningCurrentPopup()
{
    ImGuiContext& g = *GImGui;
    if (g.CurrentPopupStack.Size > 0)
        return g.OpenPopupStack[g.CurrentPopupStack.Size-1].MousePosOnOpen;
    return g.IO.MousePos;
}

// We typically use ImVec2(-FLT_MAX,-FLT_MAX) to denote an invalid mouse position
bool ImGui::IsMousePosValid(const ImVec2* mouse_pos)
{
    if (mouse_pos == NULL)
        mouse_pos = &GImGui->IO.MousePos;
    const float MOUSE_INVALID = -256000.0f;
    return mouse_pos->x >= MOUSE_INVALID && mouse_pos->y >= MOUSE_INVALID;
}

ImVec2 ImGui::GetMouseDragDelta(int button, float lock_threshold)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
    if (lock_threshold < 0.0f)
        lock_threshold = g.IO.MouseDragThreshold;
    if (g.IO.MouseDown[button])
        if (g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold)
            return g.IO.MousePos - g.IO.MouseClickedPos[button];     // Assume we can only get active with left-mouse button (at the moment).
    return ImVec2(0.0f, 0.0f);
}

void ImGui::ResetMouseDragDelta(int button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
    // NB: We don't need to reset g.IO.MouseDragMaxDistanceSqr
    g.IO.MouseClickedPos[button] = g.IO.MousePos;
}

ImGuiMouseCursor ImGui::GetMouseCursor()
{
    return GImGui->MouseCursor;
}

void ImGui::SetMouseCursor(ImGuiMouseCursor cursor_type)
{
    GImGui->MouseCursor = cursor_type;
}

void ImGui::CaptureKeyboardFromApp(bool capture)
{
    GImGui->WantCaptureKeyboardNextFrame = capture ? 1 : 0;
}

void ImGui::CaptureMouseFromApp(bool capture)
{
    GImGui->WantCaptureMouseNextFrame = capture ? 1 : 0;
}

bool ImGui::IsItemActive()
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId)
    {
        ImGuiWindow* window = g.CurrentWindow;
        return g.ActiveId == window->DC.LastItemId;
    }
    return false;
}

bool ImGui::IsItemClicked(int mouse_button)
{
    return IsMouseClicked(mouse_button) && IsItemHovered();
}

bool ImGui::IsAnyItemHovered()
{
    return GImGui->HoveredId != 0 || GImGui->HoveredIdPreviousFrame != 0;
}

bool ImGui::IsAnyItemActive()
{
    return GImGui->ActiveId != 0;
}

bool ImGui::IsItemVisible()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->ClipRect.Overlaps(window->DC.LastItemRect);
}

// Allow last item to be overlapped by a subsequent item. Both may be activated during the same frame before the later one takes priority.
void ImGui::SetItemAllowOverlap()
{
    ImGuiContext& g = *GImGui;
    if (g.HoveredId == g.CurrentWindow->DC.LastItemId)
        g.HoveredIdAllowOverlap = true;
    if (g.ActiveId == g.CurrentWindow->DC.LastItemId)
        g.ActiveIdAllowOverlap = true;
}

ImVec2 ImGui::GetItemRectMin()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.LastItemRect.Min;
}

ImVec2 ImGui::GetItemRectMax()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.LastItemRect.Max;
}

ImVec2 ImGui::GetItemRectSize()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.LastItemRect.GetSize();
}

ImVec2 ImGui::CalcItemRectClosestPoint(const ImVec2& pos, bool on_edge, float outward)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    ImRect rect = window->DC.LastItemRect;
    rect.Expand(outward);
    return rect.GetClosestPoint(pos, on_edge);
}

static ImRect GetVisibleRect()
{
    ImGuiContext& g = *GImGui;
    if (g.IO.DisplayVisibleMin.x != g.IO.DisplayVisibleMax.x && g.IO.DisplayVisibleMin.y != g.IO.DisplayVisibleMax.y)
        return ImRect(g.IO.DisplayVisibleMin, g.IO.DisplayVisibleMax);
    return ImRect(0.0f, 0.0f, g.IO.DisplaySize.x, g.IO.DisplaySize.y);
}

// Not exposed publicly as BeginTooltip() because bool parameters are evil. Let's see if other needs arise first.
static void BeginTooltipEx(ImGuiWindowFlags extra_flags, bool override_previous_tooltip)
{
    ImGuiContext& g = *GImGui;
    char window_name[16];
    ImFormatString(window_name, IM_ARRAYSIZE(window_name), "##Tooltip%02d", g.TooltipOverrideCount);
    if (override_previous_tooltip)
        if (ImGuiWindow* window = ImGui::FindWindowByName(window_name))
            if (window->Active)
            {
                // Hide previous tooltips. We can't easily "reset" the content of a window so we create a new one.
                window->HiddenFrames = 1;
                ImFormatString(window_name, IM_ARRAYSIZE(window_name), "##Tooltip%02d", ++g.TooltipOverrideCount);
            }
    ImGuiWindowFlags flags = ImGuiWindowFlags_Tooltip|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::Begin(window_name, NULL, flags | extra_flags);
}

void ImGui::SetTooltipV(const char* fmt, va_list args)
{
    BeginTooltipEx(0, true);
    TextV(fmt, args);
    EndTooltip();
}

void ImGui::SetTooltip(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    SetTooltipV(fmt, args);
    va_end(args);
}

void ImGui::BeginTooltip()
{
    BeginTooltipEx(0, false);
}

void ImGui::EndTooltip()
{
    IM_ASSERT(GetCurrentWindowRead()->Flags & ImGuiWindowFlags_Tooltip);   // Mismatched BeginTooltip()/EndTooltip() calls
    ImGui::End();
}

// Mark popup as open (toggle toward open state).
// Popups are closed when user click outside, or activate a pressable item, or CloseCurrentPopup() is called within a BeginPopup()/EndPopup() block.
// Popup identifiers are relative to the current ID-stack (so OpenPopup and BeginPopup needs to be at the same level).
// One open popup per level of the popup hierarchy (NB: when assigning we reset the Window member of ImGuiPopupRef to NULL)
void ImGui::OpenPopupEx(ImGuiID id, bool reopen_existing)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    int current_stack_size = g.CurrentPopupStack.Size;
    ImGuiPopupRef popup_ref = ImGuiPopupRef(id, window, window->GetID("##menus"), g.IO.MousePos); // Tagged as new ref because constructor sets Window to NULL (we are passing the ParentWindow info here)
    if (g.OpenPopupStack.Size < current_stack_size + 1)
        g.OpenPopupStack.push_back(popup_ref);
    else if (reopen_existing || g.OpenPopupStack[current_stack_size].PopupId != id)
    {
        g.OpenPopupStack.resize(current_stack_size+1);
        g.OpenPopupStack[current_stack_size] = popup_ref;
    }
}

void ImGui::OpenPopup(const char* str_id)
{
    ImGuiContext& g = *GImGui;
    OpenPopupEx(g.CurrentWindow->GetID(str_id), false);
}

static void CloseInactivePopups()
{
    ImGuiContext& g = *GImGui;
    if (g.OpenPopupStack.empty())
        return;

    // When popups are stacked, clicking on a lower level popups puts focus back to it and close popups above it.
    // Don't close our own child popup windows
    int n = 0;
    if (g.NavWindow)
    {
        for (n = 0; n < g.OpenPopupStack.Size; n++)
        {
            ImGuiPopupRef& popup = g.OpenPopupStack[n];
            if (!popup.Window)
                continue;
            IM_ASSERT((popup.Window->Flags & ImGuiWindowFlags_Popup) != 0);
            if (popup.Window->Flags & ImGuiWindowFlags_ChildWindow)
                continue;

            bool has_focus = false;
            for (int m = n; m < g.OpenPopupStack.Size && !has_focus; m++)
                has_focus = (g.OpenPopupStack[m].Window && g.OpenPopupStack[m].Window->RootWindow == g.NavWindow->RootWindow);
            if (!has_focus)
                break;
        }
    }
    if (n < g.OpenPopupStack.Size)   // This test is not required but it allows to set a useful breakpoint on the line below
        g.OpenPopupStack.resize(n);
}

static ImGuiWindow* GetFrontMostModalRootWindow()
{
    ImGuiContext& g = *GImGui;
    for (int n = g.OpenPopupStack.Size-1; n >= 0; n--)
        if (ImGuiWindow* front_most_popup = g.OpenPopupStack.Data[n].Window)
            if (front_most_popup->Flags & ImGuiWindowFlags_Modal)
                return front_most_popup;
    return NULL;
}

static void ClosePopupToLevel(int remaining)
{
    ImGuiContext& g = *GImGui;
    if (remaining > 0)
        ImGui::FocusWindow(g.OpenPopupStack[remaining-1].Window);
    else
        ImGui::FocusWindow(g.OpenPopupStack[0].ParentWindow);
    g.OpenPopupStack.resize(remaining);
}

void ImGui::ClosePopup(ImGuiID id)
{
    if (!IsPopupOpen(id))
        return;
    ImGuiContext& g = *GImGui;
    ClosePopupToLevel(g.OpenPopupStack.Size - 1);
}

// Close the popup we have begin-ed into.
void ImGui::CloseCurrentPopup()
{
    ImGuiContext& g = *GImGui;
    int popup_idx = g.CurrentPopupStack.Size - 1;
    if (popup_idx < 0 || popup_idx >= g.OpenPopupStack.Size || g.CurrentPopupStack[popup_idx].PopupId != g.OpenPopupStack[popup_idx].PopupId)
        return;
    while (popup_idx > 0 && g.OpenPopupStack[popup_idx].Window && (g.OpenPopupStack[popup_idx].Window->Flags & ImGuiWindowFlags_ChildMenu))
        popup_idx--;
    ClosePopupToLevel(popup_idx);
}

static inline void ClearSetNextWindowData()
{
    // FIXME-OPT
    ImGuiContext& g = *GImGui;
    g.SetNextWindowPosCond = g.SetNextWindowSizeCond = g.SetNextWindowContentSizeCond = g.SetNextWindowCollapsedCond = 0;
    g.SetNextWindowSizeConstraint = g.SetNextWindowFocus = false;
}

bool ImGui::BeginPopupEx(ImGuiID id, ImGuiWindowFlags extra_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (!IsPopupOpen(id))
    {
        ClearSetNextWindowData(); // We behave like Begin() and need to consume those values
        return false;
    }

    PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGuiWindowFlags flags = extra_flags|ImGuiWindowFlags_Popup|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings;

    char name[20];
    if (flags & ImGuiWindowFlags_ChildMenu)
        ImFormatString(name, IM_ARRAYSIZE(name), "##menu_%d", g.CurrentPopupStack.Size);    // Recycle windows based on depth
    else
        ImFormatString(name, IM_ARRAYSIZE(name), "##popup_%08x", id); // Not recycling, so we can close/open during the same frame

    bool is_open = Begin(name, NULL, flags);
    if (!(window->Flags & ImGuiWindowFlags_ShowBorders))
        g.CurrentWindow->Flags &= ~ImGuiWindowFlags_ShowBorders;
    if (!is_open) // NB: is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
        EndPopup();

    return is_open;
}

bool ImGui::BeginPopup(const char* str_id, int Flags)
{
    ImGuiContext& g = *GImGui;
    if (g.OpenPopupStack.Size <= g.CurrentPopupStack.Size) // Early out for performance
    {
        ClearSetNextWindowData(); // We behave like Begin() and need to consume those values
        return false;
    }
    return BeginPopupEx(g.CurrentWindow->GetID(str_id), Flags ? Flags : ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_AlwaysAutoResize);
}

bool ImGui::IsPopupOpen(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    return g.OpenPopupStack.Size > g.CurrentPopupStack.Size && g.OpenPopupStack[g.CurrentPopupStack.Size].PopupId == id;
}

bool ImGui::IsPopupOpen(const char* str_id)
{
    ImGuiContext& g = *GImGui;
    return g.OpenPopupStack.Size > g.CurrentPopupStack.Size && g.OpenPopupStack[g.CurrentPopupStack.Size].PopupId == g.CurrentWindow->GetID(str_id);
}

bool ImGui::BeginPopupModal(const char* name, bool* p_open, ImGuiWindowFlags extra_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    const ImGuiID id = window->GetID(name);
    if (!IsPopupOpen(id))
    {
        ClearSetNextWindowData(); // We behave like Begin() and need to consume those values
        return false;
    }

    // Center modal windows by default
    if ((window->SetWindowPosAllowFlags & g.SetNextWindowPosCond) == 0)
        SetNextWindowPos(g.IO.DisplaySize * 0.5f, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = extra_flags|ImGuiWindowFlags_Popup|ImGuiWindowFlags_Modal|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoSavedSettings;
    bool is_open = Begin(name, p_open, flags);
    if (!is_open || (p_open && !*p_open)) // NB: is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
    {
        EndPopup();
        if (is_open)
            ClosePopup(id);
        return false;
    }

    return is_open;
}

void ImGui::EndPopup()
{
    ImGuiWindow* window = GetCurrentWindow();
    IM_ASSERT(window->Flags & ImGuiWindowFlags_Popup);  // Mismatched BeginPopup()/EndPopup() calls
    IM_ASSERT(GImGui->CurrentPopupStack.Size > 0);
    End();
    if (!(window->Flags & ImGuiWindowFlags_Modal))
        PopStyleVar();
}

// This is a helper to handle the simplest case of associating one named popup to one given widget.
// 1. If you have many possible popups (for different "instances" of a same widget, or for wholly different widgets), you may be better off handling
//    this yourself so you can store data relative to the widget that opened the popup instead of choosing different popup identifiers.
// 2. If you want right-clicking on the same item to reopen the popup at new location, use the same code replacing IsItemHovered() with IsItemRectHovered()
//    and passing true to the OpenPopupEx().
//    This is because hovering an item in a window below the popup won't work. IsItemRectHovered() skips this test.
//    The pattern of ignoring the fact that the item can be interacted with (because it is blocked by the active popup) may useful in some situation 
//    when e.g. large canvas where the content of menu driven by click position.
bool ImGui::BeginPopupContextItem(const char* str_id, int mouse_button)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    ImGuiID id = str_id ? window->GetID(str_id) : window->DC.LastItemId; // If user hasn't passed an ID, we can use the LastItemID. Using LastItemID as a Popup ID won't conflict!
    IM_ASSERT(id != 0);                                                  // However, you cannot pass a NULL str_id if the last item has no identifier (e.g. a Text() item)
    if (IsItemHovered() && IsMouseClicked(mouse_button))
        OpenPopupEx(id, false);
    return BeginPopupEx(id, 0);
}

bool ImGui::BeginPopupContextWindow(const char* str_id, int mouse_button, bool also_over_items)
{
    if (!str_id)
        str_id = "window_context";
    ImGuiID id = GImGui->CurrentWindow->GetID(str_id);
    if (IsWindowRectHovered() && IsMouseClicked(mouse_button))
        if (also_over_items || !IsAnyItemHovered())
            OpenPopupEx(id, true);
    return BeginPopupEx(id, 0);
}

bool ImGui::BeginPopupContextVoid(const char* str_id, int mouse_button)
{
    if (!str_id) 
        str_id = "void_context";
    ImGuiID id = GImGui->CurrentWindow->GetID(str_id);
    if (!IsAnyWindowHovered() && IsMouseClicked(mouse_button))
        OpenPopupEx(id, true);
    return BeginPopupEx(id, 0);
}

static bool BeginChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
    ImGuiWindow* parent_window = ImGui::GetCurrentWindow();
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_ChildWindow;

    const ImVec2 content_avail = ImGui::GetContentRegionAvail();
    ImVec2 size = ImFloor(size_arg);
    const int auto_fit_axises = ((size.x == 0.0f) ? 0x01 : 0x00) | ((size.y == 0.0f) ? 0x02 : 0x00);
    if (size.x <= 0.0f)
        size.x = ImMax(content_avail.x, 4.0f) - fabsf(size.x); // Arbitrary minimum zero-ish child size of 4.0f (0.0f causing too much issues)
    if (size.y <= 0.0f)
        size.y = ImMax(content_avail.y, 4.0f) - fabsf(size.y);
    if (border)
        flags |= ImGuiWindowFlags_ShowBorders;
    flags |= extra_flags;

    char title[256];
    if (name)
        ImFormatString(title, IM_ARRAYSIZE(title), "%s/%s_%08X", parent_window->Name, name, id);
    else
        ImFormatString(title, IM_ARRAYSIZE(title), "%s/%08X", parent_window->Name, id);

    ImGui::SetNextWindowSize(size);
    bool ret = ImGui::Begin(title, NULL, flags);
    ImGuiWindow* child_window = ImGui::GetCurrentWindow();
    child_window->AutoFitChildAxises = auto_fit_axises;
    if (!(parent_window->Flags & ImGuiWindowFlags_ShowBorders))
        child_window->Flags &= ~ImGuiWindowFlags_ShowBorders;

    return ret;
}

bool ImGui::BeginChild(const char* str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    return BeginChildEx(str_id, window->GetID(str_id), size_arg, border, extra_flags);
}

bool ImGui::BeginChild(ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
    return BeginChildEx(NULL, id, size_arg, border, extra_flags);
}

void ImGui::EndChild()
{
    ImGuiWindow* window = GetCurrentWindow();

    IM_ASSERT(window->Flags & ImGuiWindowFlags_ChildWindow);   // Mismatched BeginChild()/EndChild() callss
    if ((window->Flags & ImGuiWindowFlags_ComboBox) || window->BeginCount > 1)
    {
        ImGui::End();
    }
    else
    {
        // When using auto-filling child window, we don't provide full width/height to ItemSize so that it doesn't feed back into automatic size-fitting.
        ImVec2 sz = GetWindowSize();
        if (window->AutoFitChildAxises & 0x01) // Arbitrary minimum zero-ish child size of 4.0f causes less trouble than a 0.0f
            sz.x = ImMax(4.0f, sz.x);
        if (window->AutoFitChildAxises & 0x02)
            sz.y = ImMax(4.0f, sz.y);
        ImGui::End();

        ImGuiWindow* parent_window = GetCurrentWindow();
        ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + sz);
        ItemSize(sz);
        ItemAdd(bb, 0);
    }
}

// Helper to create a child window / scrolling region that looks like a normal widget frame.
bool ImGui::BeginChildFrame(ImGuiID id, const ImVec2& size, ImGuiWindowFlags extra_flags)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    PushStyleColor(ImGuiCol_ChildWindowBg, style.Colors[ImGuiCol_FrameBg]);
    PushStyleVar(ImGuiStyleVar_ChildWindowRounding, style.FrameRounding);
    PushStyleVar(ImGuiStyleVar_WindowPadding, style.FramePadding);
    return BeginChild(id, size, (g.CurrentWindow->Flags & ImGuiWindowFlags_ShowBorders) ? true : false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysUseWindowPadding | extra_flags);
}

void ImGui::EndChildFrame()
{
    EndChild();
    PopStyleVar(2);
    PopStyleColor();
}

// Save and compare stack sizes on Begin()/End() to detect usage errors
static void CheckStacksSize(ImGuiWindow* window, bool write)
{
    // NOT checking: DC.ItemWidth, DC.AllowKeyboardFocus, DC.ButtonRepeat, DC.TextWrapPos (per window) to allow user to conveniently push once and not pop (they are cleared on Begin)
    ImGuiContext& g = *GImGui;
    int* p_backup = &window->DC.StackSizesBackup[0];
    { int current = window->IDStack.Size;       if (write) *p_backup = current; else IM_ASSERT(*p_backup == current && "PushID/PopID or TreeNode/TreePop Mismatch!");   p_backup++; }    // Too few or too many PopID()/TreePop()
    { int current = window->DC.GroupStack.Size; if (write) *p_backup = current; else IM_ASSERT(*p_backup == current && "BeginGroup/EndGroup Mismatch!");                p_backup++; }    // Too few or too many EndGroup()
    { int current = g.CurrentPopupStack.Size;   if (write) *p_backup = current; else IM_ASSERT(*p_backup == current && "BeginMenu/EndMenu or BeginPopup/EndPopup Mismatch"); p_backup++;}// Too few or too many EndMenu()/EndPopup()
    { int current = g.ColorModifiers.Size;      if (write) *p_backup = current; else IM_ASSERT(*p_backup == current && "PushStyleColor/PopStyleColor Mismatch!");       p_backup++; }    // Too few or too many PopStyleColor()
    { int current = g.StyleModifiers.Size;      if (write) *p_backup = current; else IM_ASSERT(*p_backup == current && "PushStyleVar/PopStyleVar Mismatch!");           p_backup++; }    // Too few or too many PopStyleVar()
    { int current = g.FontStack.Size;           if (write) *p_backup = current; else IM_ASSERT(*p_backup == current && "PushFont/PopFont Mismatch!");                   p_backup++; }    // Too few or too many PopFont()
    IM_ASSERT(p_backup == window->DC.StackSizesBackup + IM_ARRAYSIZE(window->DC.StackSizesBackup));
}

static ImVec2 FindBestPopupWindowPos(const ImVec2& base_pos, const ImVec2& size, int* last_dir, const ImRect& r_inner)
{
    const ImGuiStyle& style = GImGui->Style;

    // Clamp into visible area while not overlapping the cursor. Safety padding is optional if our popup size won't fit without it.
    ImVec2 safe_padding = style.DisplaySafeAreaPadding;
    ImRect r_outer(GetVisibleRect());
    r_outer.Expand(ImVec2((size.x - r_outer.GetWidth() > safe_padding.x*2) ? -safe_padding.x : 0.0f, (size.y - r_outer.GetHeight() > safe_padding.y*2) ? -safe_padding.y : 0.0f));
    ImVec2 base_pos_clamped = ImClamp(base_pos, r_outer.Min, r_outer.Max - size);

    for (int n = (*last_dir != -1) ? -1 : 0; n < 4; n++)   // Last, Right, down, up, left. (Favor last used direction).
    {
        const int dir = (n == -1) ? *last_dir : n;
        ImRect rect(dir == 0 ? r_inner.Max.x : r_outer.Min.x, dir == 1 ? r_inner.Max.y : r_outer.Min.y, dir == 3 ? r_inner.Min.x : r_outer.Max.x, dir == 2 ? r_inner.Min.y : r_outer.Max.y);
        if (rect.GetWidth() < size.x || rect.GetHeight() < size.y)
            continue;
        *last_dir = dir;
        return ImVec2(dir == 0 ? r_inner.Max.x : dir == 3 ? r_inner.Min.x - size.x : base_pos_clamped.x, dir == 1 ? r_inner.Max.y : dir == 2 ? r_inner.Min.y - size.y : base_pos_clamped.y);
    }

    // Fallback, try to keep within display
    *last_dir = -1;
    ImVec2 pos = base_pos;
    pos.x = ImMax(ImMin(pos.x + size.x, r_outer.Max.x) - size.x, r_outer.Min.x);
    pos.y = ImMax(ImMin(pos.y + size.y, r_outer.Max.y) - size.y, r_outer.Min.y);
    return pos;
}

ImGuiWindow* ImGui::FindWindowByName(const char* name)
{
    // FIXME-OPT: Store sorted hashes -> pointers so we can do a bissection in a contiguous block
    ImGuiContext& g = *GImGui;
    ImGuiID id = ImHash(name, 0);
    for (int i = 0; i < g.Windows.Size; i++)
        if (g.Windows[i]->ID == id)
            return g.Windows[i];
    return NULL;
}

static ImGuiWindow* CreateNewWindow(const char* name, ImVec2 size, ImGuiWindowFlags flags)
{
    ImGuiContext& g = *GImGui;

    // Create window the first time
    ImGuiWindow* window = (ImGuiWindow*)ImGui::MemAlloc(sizeof(ImGuiWindow));
    IM_PLACEMENT_NEW(window) ImGuiWindow(name);
    window->Flags = flags;

    if (flags & ImGuiWindowFlags_NoSavedSettings)
    {
        // User can disable loading and saving of settings. Tooltip and child windows also don't store settings.
        window->Size = window->SizeFull = size;
    }
    else
    {
        // Retrieve settings from .ini file
        // Use SetWindowPos() or SetNextWindowPos() with the appropriate condition flag to change the initial position of a window.
        window->PosFloat = ImVec2(60, 60);
        window->Pos = ImVec2((float)(int)window->PosFloat.x, (float)(int)window->PosFloat.y);

        ImGuiIniData* settings = FindWindowSettings(name);
        if (!settings)
        {
            settings = AddWindowSettings(name);
        }
        else
        {
            window->SetWindowPosAllowFlags &= ~ImGuiCond_FirstUseEver;
            window->SetWindowSizeAllowFlags &= ~ImGuiCond_FirstUseEver;
            window->SetWindowCollapsedAllowFlags &= ~ImGuiCond_FirstUseEver;
        }

        if (settings->Pos.x != FLT_MAX)
        {
            window->PosFloat = settings->Pos;
            window->Pos = ImVec2((float)(int)window->PosFloat.x, (float)(int)window->PosFloat.y);
            window->Collapsed = settings->Collapsed;
        }

        if (ImLengthSqr(settings->Size) > 0.00001f)
            size = settings->Size;
        window->Size = window->SizeFull = size;
    }

    if ((flags & ImGuiWindowFlags_AlwaysAutoResize) != 0)
    {
        window->AutoFitFramesX = window->AutoFitFramesY = 2;
        window->AutoFitOnlyGrows = false;
    }
    else
    {
        if (window->Size.x <= 0.0f)
            window->AutoFitFramesX = 2;
        if (window->Size.y <= 0.0f)
            window->AutoFitFramesY = 2;
        window->AutoFitOnlyGrows = (window->AutoFitFramesX > 0) || (window->AutoFitFramesY > 0);
    }

    if (flags & ImGuiWindowFlags_NoBringToFrontOnFocus)
        g.Windows.insert(g.Windows.begin(), window); // Quite slow but rare and only once
    else
        g.Windows.push_back(window);
    return window;
}

static ImVec2 CalcSizeFullWithConstraint(ImGuiWindow* window, ImVec2 new_size)
{
    ImGuiContext& g = *GImGui;
    if (g.SetNextWindowSizeConstraint)
    {
        // Using -1,-1 on either X/Y axis to preserve the current size.
        ImRect cr = g.SetNextWindowSizeConstraintRect;
        new_size.x = (cr.Min.x >= 0 && cr.Max.x >= 0) ? ImClamp(new_size.x, cr.Min.x, cr.Max.x) : window->SizeFull.x;
        new_size.y = (cr.Min.y >= 0 && cr.Max.y >= 0) ? ImClamp(new_size.y, cr.Min.y, cr.Max.y) : window->SizeFull.y;
        if (g.SetNextWindowSizeConstraintCallback)
        {
            ImGuiSizeConstraintCallbackData data;
            data.UserData = g.SetNextWindowSizeConstraintCallbackUserData;
            data.Pos = window->Pos;
            data.CurrentSize = window->SizeFull;
            data.DesiredSize = new_size;
            g.SetNextWindowSizeConstraintCallback(&data);
            new_size = data.DesiredSize;
        }
    }
    if (!(window->Flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_AlwaysAutoResize)))
        new_size = ImMax(new_size, g.Style.WindowMinSize);
    return new_size;
}

static ImVec2 CalcSizeAutoFit(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;
    ImGuiWindowFlags flags = window->Flags;
    ImVec2 size_auto_fit;
    if ((flags & ImGuiWindowFlags_Tooltip) != 0)
    {
        // Tooltip always resize. We keep the spacing symmetric on both axises for aesthetic purpose.
        size_auto_fit = window->SizeContents + window->WindowPadding - ImVec2(0.0f, style.ItemSpacing.y);
    }
    else
    {
        // Handling case of auto fit window not fitting on the screen (on either axis): we are growing the size on the other axis to compensate for expected scrollbar. FIXME: Might turn bigger than DisplaySize-WindowPadding.
        size_auto_fit = ImClamp(window->SizeContents + window->WindowPadding, style.WindowMinSize, ImMax(style.WindowMinSize, g.IO.DisplaySize - g.Style.DisplaySafeAreaPadding));
        ImVec2 size_auto_fit_after_constraint = CalcSizeFullWithConstraint(window, size_auto_fit);
        if (size_auto_fit_after_constraint.x < window->SizeContents.x && !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar))
            size_auto_fit.y += style.ScrollbarSize;
        if (size_auto_fit_after_constraint.y < window->SizeContents.y && !(flags & ImGuiWindowFlags_NoScrollbar))
            size_auto_fit.x += style.ScrollbarSize * 2.0f;
        size_auto_fit.y = ImMax(size_auto_fit.y - style.ItemSpacing.y, 0.0f);
    }
    return size_auto_fit;
}

static ImVec2 CalcNextScrollFromScrollTargetAndClamp(ImGuiWindow* window)
{
    ImVec2 scroll = window->Scroll;
    float cr_x = window->ScrollTargetCenterRatio.x;
    float cr_y = window->ScrollTargetCenterRatio.y;
    if (window->ScrollTarget.x < FLT_MAX)
        scroll.x = window->ScrollTarget.x - cr_x * (window->SizeFull.x - window->ScrollbarSizes.x);
    if (window->ScrollTarget.y < FLT_MAX)
        scroll.y = window->ScrollTarget.y - (1.0f - cr_y) * (window->TitleBarHeight() + window->MenuBarHeight()) - cr_y * (window->SizeFull.y - window->ScrollbarSizes.y);
    scroll = ImMax(scroll, ImVec2(0.0f, 0.0f));
    if (!window->Collapsed && !window->SkipItems)
    {
        scroll.x = ImMin(scroll.x, ImMax(0.0f, window->SizeContents.x - (window->SizeFull.x - window->ScrollbarSizes.x))); // == GetScrollMaxX for that window
        scroll.y = ImMin(scroll.y, ImMax(0.0f, window->SizeContents.y - (window->SizeFull.y - window->ScrollbarSizes.y))); // == GetScrollMaxY for that window
    }
    return scroll;
}

static ImGuiCol GetWindowBgColorIdxFromFlags(ImGuiWindowFlags flags)
{
    if (flags & ImGuiWindowFlags_ComboBox)
        return ImGuiCol_ComboBg;
    if (flags & (ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_Popup))
        return ImGuiCol_PopupBg;
    if (flags & ImGuiWindowFlags_ChildWindow)
        return ImGuiCol_ChildWindowBg;
    return ImGuiCol_WindowBg;
}

// Push a new ImGui window to add widgets to.
// - A default window called "Debug" is automatically stacked at the beginning of every frame so you can use widgets without explicitly calling a Begin/End pair.
// - Begin/End can be called multiple times during the frame with the same window name to append content.
// - The window name is used as a unique identifier to preserve window information across frames (and save rudimentary information to the .ini file).
//   You can use the "##" or "###" markers to use the same label with different id, or same id with different label. See documentation at the top of this file.
// - Return false when window is collapsed, so you can early out in your code. You always need to call ImGui::End() even if false is returned.
// - Passing 'bool* p_open' displays a Close button on the upper-right corner of the window, the pointed value will be set to false when the button is pressed.
bool ImGui::Begin(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    IM_ASSERT(name != NULL);                        // Window name required
    IM_ASSERT(g.Initialized);                       // Forgot to call ImGui::NewFrame()
    IM_ASSERT(g.FrameCountEnded != g.FrameCount);   // Called ImGui::Render() or ImGui::EndFrame() and haven't called ImGui::NewFrame() again yet

    if (flags & ImGuiWindowFlags_NoInputs)
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    // Find or create
    bool window_is_new = false;
    ImGuiWindow* window = FindWindowByName(name);
    if (!window)
    {
        ImVec2 size_on_first_use = (g.SetNextWindowSizeCond != 0) ? g.SetNextWindowSizeVal : ImVec2(0.0f, 0.0f); // Any condition flag will do since we are creating a new window here.
        window = CreateNewWindow(name, size_on_first_use, flags);
        window_is_new = true;
    }

    const int current_frame = g.FrameCount;
    const bool first_begin_of_the_frame = (window->LastFrameActive != current_frame);
    if (first_begin_of_the_frame)
        window->Flags = (ImGuiWindowFlags)flags;
    else
        flags = window->Flags;

    // Add to stack
    ImGuiWindow* parent_window = !g.CurrentWindowStack.empty() ? g.CurrentWindowStack.back() : NULL;
    g.CurrentWindowStack.push_back(window);
    SetCurrentWindow(window);
    CheckStacksSize(window, true);
    IM_ASSERT(parent_window != NULL || !(flags & ImGuiWindowFlags_ChildWindow));

    bool window_just_activated_by_user = (window->LastFrameActive < current_frame - 1);   // Not using !WasActive because the implicit "Debug" window would always toggle off->on
    if (flags & ImGuiWindowFlags_Popup)
    {
        ImGuiPopupRef& popup_ref = g.OpenPopupStack[g.CurrentPopupStack.Size];
        window_just_activated_by_user |= (window->PopupId != popup_ref.PopupId); // We recycle popups so treat window as activated if popup id changed
        window_just_activated_by_user |= (window != popup_ref.Window);
        popup_ref.Window = window;
        g.CurrentPopupStack.push_back(popup_ref);
        window->PopupId = popup_ref.PopupId;
    }

    const bool window_just_appearing_after_hidden_for_resize = (window->HiddenFrames == 1);
    window->Appearing = (window_just_activated_by_user || window_just_appearing_after_hidden_for_resize);

    // Process SetNextWindow***() calls
    bool window_pos_set_by_api = false, window_size_set_by_api = false;
    if (g.SetNextWindowPosCond)
    {
        if (window->Appearing) 
            window->SetWindowPosAllowFlags |= ImGuiCond_Appearing;
        window_pos_set_by_api = (window->SetWindowPosAllowFlags & g.SetNextWindowPosCond) != 0;
        if (window_pos_set_by_api && ImLengthSqr(g.SetNextWindowPosPivot) > 0.00001f)
        {
            // May be processed on the next frame if this is our first frame and we are measuring size
            // FIXME: Look into removing the branch so everything can go through this same code path for consistency.
            window->SetWindowPosVal = g.SetNextWindowPosVal;
            window->SetWindowPosPivot = g.SetNextWindowPosPivot;
            window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
        }
        else
        {
            SetWindowPos(window, g.SetNextWindowPosVal, g.SetNextWindowPosCond);
        }
        g.SetNextWindowPosCond = 0;
    }
    if (g.SetNextWindowSizeCond)
    {
        if (window->Appearing) 
            window->SetWindowSizeAllowFlags |= ImGuiCond_Appearing;
        window_size_set_by_api = (window->SetWindowSizeAllowFlags & g.SetNextWindowSizeCond) != 0;
        SetWindowSize(window, g.SetNextWindowSizeVal, g.SetNextWindowSizeCond);
        g.SetNextWindowSizeCond = 0;
    }
    if (g.SetNextWindowContentSizeCond)
    {
        window->SizeContentsExplicit = g.SetNextWindowContentSizeVal;
        g.SetNextWindowContentSizeCond = 0;
    }
    else if (first_begin_of_the_frame)
    {
        window->SizeContentsExplicit = ImVec2(0.0f, 0.0f);
    }
    if (g.SetNextWindowCollapsedCond)
    {
        if (window->Appearing)
            window->SetWindowCollapsedAllowFlags |= ImGuiCond_Appearing;
        SetWindowCollapsed(window, g.SetNextWindowCollapsedVal, g.SetNextWindowCollapsedCond);
        g.SetNextWindowCollapsedCond = 0;
    }
    if (g.SetNextWindowFocus)
    {
        SetWindowFocus();
        g.SetNextWindowFocus = false;
    }

    // Update known root window (if we are a child window, otherwise window == window->RootWindow)
    int root_idx, root_non_popup_idx;
    for (root_idx = g.CurrentWindowStack.Size - 1; root_idx > 0; root_idx--)
        if (!(g.CurrentWindowStack[root_idx]->Flags & ImGuiWindowFlags_ChildWindow))
            break;
    for (root_non_popup_idx = root_idx; root_non_popup_idx > 0; root_non_popup_idx--)
        if (!(g.CurrentWindowStack[root_non_popup_idx]->Flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup)) || (g.CurrentWindowStack[root_non_popup_idx]->Flags & ImGuiWindowFlags_Modal))
            break;
    window->ParentWindow = parent_window;
    window->RootWindow = g.CurrentWindowStack[root_idx];
    window->RootNonPopupWindow = g.CurrentWindowStack[root_non_popup_idx];      // Used to display TitleBgActive color and for selecting which window to use for NavWindowing

    // When reusing window again multiple times a frame, just append content (don't need to setup again)
    if (first_begin_of_the_frame)
    {
        window->Active = true;
        window->OrderWithinParent = 0;
        window->BeginCount = 0;
        window->ClipRect = ImVec4(-FLT_MAX,-FLT_MAX,+FLT_MAX,+FLT_MAX);
        window->LastFrameActive = current_frame;
        window->IDStack.resize(1);

        // Clear draw list, setup texture, outer clipping rectangle
        window->DrawList->Clear();
        window->DrawList->PushTextureID(g.Font->ContainerAtlas->TexID);
        ImRect fullscreen_rect(GetVisibleRect());
        if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & (ImGuiWindowFlags_ComboBox|ImGuiWindowFlags_Popup)))
            PushClipRect(parent_window->ClipRect.Min, parent_window->ClipRect.Max, true);
        else
            PushClipRect(fullscreen_rect.Min, fullscreen_rect.Max, true);

        if (window_just_activated_by_user)
        {
            // Popup first latch mouse position, will position itself when it appears next frame
            window->AutoPosLastDirection = -1;
            if ((flags & ImGuiWindowFlags_Popup) != 0 && !window_pos_set_by_api)
                window->PosFloat = g.IO.MousePos;
        }

        // Collapse window by double-clicking on title bar
        // At this point we don't have a clipping rectangle setup yet, so we can use the title bar area for hit detection and drawing
        if (!(flags & ImGuiWindowFlags_NoTitleBar) && !(flags & ImGuiWindowFlags_NoCollapse))
        {
            ImRect title_bar_rect = window->TitleBarRect();
            if (g.HoveredWindow == window && IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max) && g.IO.MouseDoubleClicked[0])
            {
                window->Collapsed = !window->Collapsed;
                MarkIniSettingsDirty(window);
                FocusWindow(window);
            }
        }
        else
        {
            window->Collapsed = false;
        }

        // SIZE

        // Save contents size from last frame for auto-fitting (unless explicitly specified)
        window->SizeContents.x = (float)(int)((window->SizeContentsExplicit.x != 0.0f) ? window->SizeContentsExplicit.x : ((window_is_new ? 0.0f : window->DC.CursorMaxPos.x - window->Pos.x) + window->Scroll.x));
        window->SizeContents.y = (float)(int)((window->SizeContentsExplicit.y != 0.0f) ? window->SizeContentsExplicit.y : ((window_is_new ? 0.0f : window->DC.CursorMaxPos.y - window->Pos.y) + window->Scroll.y));

        // Hide popup/tooltip window when first appearing while we measure size (because we recycle them)
        if (window->HiddenFrames > 0)
            window->HiddenFrames--;
        if ((flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) != 0 && window_just_activated_by_user)
        {
            window->HiddenFrames = 1;
            if (flags & ImGuiWindowFlags_AlwaysAutoResize)
            {
                if (!window_size_set_by_api)
                    window->Size = window->SizeFull = ImVec2(0.f, 0.f);
                window->SizeContents = ImVec2(0.f, 0.f);
            }
        }

        // Lock window padding so that altering the ShowBorders flag for children doesn't have side-effects.
        window->WindowPadding = ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & (ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_ComboBox | ImGuiWindowFlags_Popup))) ? ImVec2(0,0) : style.WindowPadding;

        // Calculate auto-fit size, handle automatic resize
        const ImVec2 size_auto_fit = CalcSizeAutoFit(window);
        if (window->Collapsed)
        {
            // We still process initial auto-fit on collapsed windows to get a window width,
            // But otherwise we don't honor ImGuiWindowFlags_AlwaysAutoResize when collapsed.
            if (window->AutoFitFramesX > 0)
                window->SizeFull.x = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.x, size_auto_fit.x) : size_auto_fit.x;
            if (window->AutoFitFramesY > 0)
                window->SizeFull.y = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.y, size_auto_fit.y) : size_auto_fit.y;
        }
        else if (!window_size_set_by_api)
        {
            if (flags & ImGuiWindowFlags_AlwaysAutoResize)
            {
                window->SizeFull = size_auto_fit;
            }
            else if (window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
            {
                // Auto-fit only grows during the first few frames
                if (window->AutoFitFramesX > 0)
                    window->SizeFull.x = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.x, size_auto_fit.x) : size_auto_fit.x;
                if (window->AutoFitFramesY > 0)
                    window->SizeFull.y = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.y, size_auto_fit.y) : size_auto_fit.y;
                MarkIniSettingsDirty(window);
            }
        }

        // Apply minimum/maximum window size constraints and final size
        window->SizeFull = CalcSizeFullWithConstraint(window, window->SizeFull);
        window->Size = window->Collapsed ? window->TitleBarRect().GetSize() : window->SizeFull;
        
        // POSITION

        // Position child window
        if (flags & ImGuiWindowFlags_ChildWindow)
        {
            window->OrderWithinParent = parent_window->DC.ChildWindows.Size;
            parent_window->DC.ChildWindows.push_back(window);
        }
        if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup))
        {
            IM_ASSERT(window_size_set_by_api); // Submitted by BeginChild()
            window->Pos = window->PosFloat = parent_window->DC.CursorPos;
            window->Size = window->SizeFull;
        }

        const bool window_pos_with_pivot = (window->SetWindowPosVal.x != FLT_MAX && window->HiddenFrames == 0);
        if (window_pos_with_pivot)
        {
            // Position given a pivot (e.g. for centering)
            SetWindowPos(window, ImMax(style.DisplaySafeAreaPadding, window->SetWindowPosVal - window->SizeFull * window->SetWindowPosPivot), 0);
        }
        else if (flags & ImGuiWindowFlags_ChildMenu)
        {
            // Child menus typically request _any_ position within the parent menu item, and then our FindBestPopupWindowPos() function will move the new menu outside the parent bounds.
            // This is how we end up with child menus appearing (most-commonly) on the right of the parent menu.
            IM_ASSERT(window_pos_set_by_api);
            float horizontal_overlap = style.ItemSpacing.x; // We want some overlap to convey the relative depth of each popup (currently the amount of overlap it is hard-coded to style.ItemSpacing.x, may need to introduce another style value).
            ImRect rect_to_avoid;
            if (parent_window->DC.MenuBarAppending)
                rect_to_avoid = ImRect(-FLT_MAX, parent_window->Pos.y + parent_window->TitleBarHeight(), FLT_MAX, parent_window->Pos.y + parent_window->TitleBarHeight() + parent_window->MenuBarHeight());
            else
                rect_to_avoid = ImRect(parent_window->Pos.x + horizontal_overlap, -FLT_MAX, parent_window->Pos.x + parent_window->Size.x - horizontal_overlap - parent_window->ScrollbarSizes.x, FLT_MAX);
            window->PosFloat = FindBestPopupWindowPos(window->PosFloat, window->Size, &window->AutoPosLastDirection, rect_to_avoid);
        }
        else if ((flags & ImGuiWindowFlags_Popup) != 0 && !window_pos_set_by_api && window_just_appearing_after_hidden_for_resize)
        {
            ImRect rect_to_avoid(window->PosFloat.x - 1, window->PosFloat.y - 1, window->PosFloat.x + 1, window->PosFloat.y + 1);
            window->PosFloat = FindBestPopupWindowPos(window->PosFloat, window->Size, &window->AutoPosLastDirection, rect_to_avoid);
        }

        // Position tooltip (always follows mouse)
        if ((flags & ImGuiWindowFlags_Tooltip) != 0 && !window_pos_set_by_api)
        {
            ImVec2 ref_pos = g.IO.MousePos;
            ImRect rect_to_avoid(ref_pos.x - 16, ref_pos.y - 8, ref_pos.x + 24, ref_pos.y + 24); // FIXME: Completely hard-coded. Perhaps center on cursor hit-point instead?
            window->PosFloat = FindBestPopupWindowPos(ref_pos, window->Size, &window->AutoPosLastDirection, rect_to_avoid);
            if (window->AutoPosLastDirection == -1)
                window->PosFloat = ref_pos + ImVec2(2,2); // If there's not enough room, for tooltip we prefer avoiding the cursor at all cost even if it means that part of the tooltip won't be visible.
        }

        // Clamp position so it stays visible
        if (!(flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Tooltip))
        {
            if (!window_pos_set_by_api && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0 && g.IO.DisplaySize.x > 0.0f && g.IO.DisplaySize.y > 0.0f) // Ignore zero-sized display explicitly to avoid losing positions if a window manager reports zero-sized window when initializing or minimizing.
            {
                ImVec2 padding = ImMax(style.DisplayWindowPadding, style.DisplaySafeAreaPadding);
                window->PosFloat = ImMax(window->PosFloat + window->Size, padding) - window->Size;
                window->PosFloat = ImMin(window->PosFloat, g.IO.DisplaySize - padding);
            }
        }
        window->Pos = ImVec2((float)(int)window->PosFloat.x, (float)(int)window->PosFloat.y);

        // Default item width. Make it proportional to window size if window manually resizes
        if (window->Size.x > 0.0f && !(flags & ImGuiWindowFlags_Tooltip) && !(flags & ImGuiWindowFlags_AlwaysAutoResize))
            window->ItemWidthDefault = (float)(int)(window->Size.x * 0.65f);
        else
            window->ItemWidthDefault = (float)(int)(g.FontSize * 16.0f);

        // Prepare for focus requests
        window->FocusIdxAllRequestCurrent = (window->FocusIdxAllRequestNext == INT_MAX || window->FocusIdxAllCounter == -1) ? INT_MAX : (window->FocusIdxAllRequestNext + (window->FocusIdxAllCounter+1)) % (window->FocusIdxAllCounter+1);
        window->FocusIdxTabRequestCurrent = (window->FocusIdxTabRequestNext == INT_MAX || window->FocusIdxTabCounter == -1) ? INT_MAX : (window->FocusIdxTabRequestNext + (window->FocusIdxTabCounter+1)) % (window->FocusIdxTabCounter+1);
        window->FocusIdxAllCounter = window->FocusIdxTabCounter = -1;
        window->FocusIdxAllRequestNext = window->FocusIdxTabRequestNext = INT_MAX;

        // Apply scrolling
        window->Scroll = CalcNextScrollFromScrollTargetAndClamp(window);
        window->ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);

        // Modal window darkens what is behind them
        if ((flags & ImGuiWindowFlags_Modal) != 0 && window == GetFrontMostModalRootWindow())
            window->DrawList->AddRectFilled(fullscreen_rect.Min, fullscreen_rect.Max, GetColorU32(ImGuiCol_ModalWindowDarkening, g.ModalWindowDarkeningRatio));

        // Draw window + handle manual resize
        ImRect title_bar_rect = window->TitleBarRect();
        const float window_rounding = (flags & ImGuiWindowFlags_ChildWindow) ? style.ChildWindowRounding : style.WindowRounding;
        if (window->Collapsed)
        {
            // Title bar only
            RenderFrame(title_bar_rect.Min, title_bar_rect.Max, GetColorU32(ImGuiCol_TitleBgCollapsed), true, window_rounding);
        }
        else
        {
            ImU32 resize_col = 0;
            const float resize_corner_size = ImMax(g.FontSize * 1.35f, window_rounding + 1.0f + g.FontSize * 0.2f);
            if (!(flags & ImGuiWindowFlags_AlwaysAutoResize) && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0 && !(flags & ImGuiWindowFlags_NoResize))
            {
                // Manual resize
                // Using the FlattenChilds button flag, we make the resize button accessible even if we are hovering over a child window
                const ImVec2 br = window->Rect().GetBR();
                const ImRect resize_rect(br - ImVec2(resize_corner_size * 0.75f, resize_corner_size * 0.75f), br);
                const ImGuiID resize_id = window->GetID("#RESIZE");
                bool hovered, held;
                ButtonBehavior(resize_rect, resize_id, &hovered, &held, ImGuiButtonFlags_FlattenChilds);
                resize_col = GetColorU32(held ? ImGuiCol_ResizeGripActive : hovered ? ImGuiCol_ResizeGripHovered : ImGuiCol_ResizeGrip);
                if (hovered || held)
                    g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;

                ImVec2 size_target(FLT_MAX,FLT_MAX);
                if (g.HoveredWindow == window && held && g.IO.MouseDoubleClicked[0])
                {
                    // Manual auto-fit when double-clicking
                    size_target = size_auto_fit;
                    ClearActiveID();
                }
                else if (held)
                {
                    // We don't use an incremental MouseDelta but rather compute an absolute target size based on mouse position
                    size_target = (g.IO.MousePos - g.ActiveIdClickOffset + resize_rect.GetSize()) - window->Pos;
                }

                if (size_target.x != FLT_MAX && size_target.y != FLT_MAX)
                {
                    window->SizeFull = CalcSizeFullWithConstraint(window, size_target);
                    MarkIniSettingsDirty(window);
                }
                window->Size = window->SizeFull;
                title_bar_rect = window->TitleBarRect();
            }

            // Scrollbars
            window->ScrollbarY = (flags & ImGuiWindowFlags_AlwaysVerticalScrollbar) || ((window->SizeContents.y > window->Size.y + style.ItemSpacing.y) && !(flags & ImGuiWindowFlags_NoScrollbar));
            window->ScrollbarX = (flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar) || ((window->SizeContents.x > window->Size.x - (window->ScrollbarY ? style.ScrollbarSize : 0.0f) - window->WindowPadding.x) && !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar));
            if (window->ScrollbarX && !window->ScrollbarY)
                window->ScrollbarY = (window->SizeContents.y > window->Size.y + style.ItemSpacing.y - style.ScrollbarSize) && !(flags & ImGuiWindowFlags_NoScrollbar);
            window->ScrollbarSizes = ImVec2(window->ScrollbarY ? style.ScrollbarSize : 0.0f, window->ScrollbarX ? style.ScrollbarSize : 0.0f);
            window->BorderSize = (flags & ImGuiWindowFlags_ShowBorders) ? 1.0f : 0.0f;

            // Window background, Default Alpha
            ImU32 bg_col = GetColorU32(GetWindowBgColorIdxFromFlags(flags));
            window->DrawList->AddRectFilled(window->Pos+ImVec2(0,window->TitleBarHeight()), window->Pos+window->Size, bg_col, ((style.WindowPadThickness > 0.f && window->RootWindow == window) ? 0.f : window_rounding), (flags & ImGuiWindowFlags_NoTitleBar) ? ImGuiCorner_All : ImGuiCorner_BotLeft|ImGuiCorner_BotRight);

			// Title bar
			const bool is_focused = g.NavWindow && window->RootNonPopupWindow == g.NavWindow->RootNonPopupWindow;
			ImRect leftbar(ImVec2(title_bar_rect.GetBL() - ImVec2(style.WindowPadThickness, 0)), ImVec2(title_bar_rect.GetBL().x, window->Pos.y + window->Size.y));
			ImRect rightbar(title_bar_rect.GetBR(), ImVec2(title_bar_rect.GetBR().x + style.WindowPadThickness, window->Pos.y + window->Size.y));
			ImRect bottombar(ImVec2(leftbar.Min.x, window->Pos.y + window->Size.y), ImVec2(rightbar.Max.x, window->Pos.y + window->Size.y + style.WindowPadThickness));
			if (window->RootWindow == window)
			{
				if (!(flags & ImGuiWindowFlags_NoTitleBar))
				{
					window->DrawList->AddRectFilled(title_bar_rect.GetTL() - ImVec2(style.WindowPadThickness, 0), title_bar_rect.GetBR() + ImVec2(style.WindowPadThickness, 0), GetColorU32(is_focused ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg), window_rounding, ImGuiCorner_TopLeft | ImGuiCorner_TopRight);
					window->DrawList->AddRectFilled(leftbar.Min, leftbar.Max, GetColorU32(is_focused ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg));
					window->DrawList->AddRectFilled(rightbar.Min, rightbar.Max, GetColorU32(is_focused ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg));
					window->DrawList->AddRectFilled(bottombar.Min, bottombar.Max, GetColorU32(is_focused ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg), window_rounding, ImGuiCorner_BotLeft | ImGuiCorner_BotRight);
				}
			}

            // Menu bar
            if (flags & ImGuiWindowFlags_MenuBar)
            {
                ImRect menu_bar_rect = window->MenuBarRect();
                if (flags & ImGuiWindowFlags_ShowBorders)
                    window->DrawList->AddLine(menu_bar_rect.GetBL(), menu_bar_rect.GetBR(), GetColorU32(ImGuiCol_Border));
                window->DrawList->AddRectFilled(menu_bar_rect.GetTL(), menu_bar_rect.GetBR(), GetColorU32(ImGuiCol_MenuBarBg), (flags & ImGuiWindowFlags_NoTitleBar) ? window_rounding : 0.0f, ImGuiCorner_TopLeft|ImGuiCorner_TopRight);
            }

            // Scrollbars
            if (window->ScrollbarX)
                Scrollbar(ImGuiLayoutType_Horizontal);
            if (window->ScrollbarY)
                Scrollbar(ImGuiLayoutType_Vertical);

            // Render resize grip
            // (after the input handling so we don't have a frame of latency)
            if (!(flags & ImGuiWindowFlags_NoResize))
            {
                const ImVec2 br = window->Rect().GetBR();
                window->DrawList->PathLineTo(br + ImVec2(-resize_corner_size, -window->BorderSize));
                window->DrawList->PathLineTo(br + ImVec2(-window->BorderSize, -resize_corner_size));
                window->DrawList->PathArcToFast(ImVec2(br.x - window_rounding - window->BorderSize, br.y - window_rounding - window->BorderSize), window_rounding, 0, 3);
                window->DrawList->PathFillConvex(resize_col);
            }

            // Borders
            if (flags & ImGuiWindowFlags_ShowBorders)
			{
				window->DrawList->AddRect(title_bar_rect.GetBL(), window->Pos + window->Size, GetColorU32(ImGuiCol_Border), ((style.WindowPadThickness > 0.f && window->RootWindow == window) ? 0.f : window_rounding));
				if (!(flags & ImGuiWindowFlags_NoTitleBar))
				{
					if (style.WindowPadThickness > 0.f && window->RootWindow == window) {
						window->DrawList->AddRect(window->Pos - ImVec2(style.WindowPadThickness, 0) + ImVec2(1, 1), bottombar.Max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), window_rounding);
						window->DrawList->AddRect(window->Pos - ImVec2(style.WindowPadThickness, 0), bottombar.Max, GetColorU32(ImGuiCol_Border), window_rounding);
					}
					else
						window->DrawList->AddLine(title_bar_rect.GetBL() + ImVec2(1, 0), title_bar_rect.GetBR() - ImVec2(1, 0), GetColorU32(ImGuiCol_Border));
				}
            }
        }

        // Update ContentsRegionMax. All the variable it depends on are set above in this function.
        window->ContentsRegionRect.Min.x = -window->Scroll.x + window->WindowPadding.x;
        window->ContentsRegionRect.Min.y = -window->Scroll.y + window->WindowPadding.y + window->TitleBarHeight() + window->MenuBarHeight();
        window->ContentsRegionRect.Max.x = -window->Scroll.x - window->WindowPadding.x + (window->SizeContentsExplicit.x != 0.0f ? window->SizeContentsExplicit.x : (window->Size.x - window->ScrollbarSizes.x)); 
        window->ContentsRegionRect.Max.y = -window->Scroll.y - window->WindowPadding.y + (window->SizeContentsExplicit.y != 0.0f ? window->SizeContentsExplicit.y : (window->Size.y - window->ScrollbarSizes.y)); 

        // Setup drawing context
        window->DC.IndentX = 0.0f + window->WindowPadding.x - window->Scroll.x;
        window->DC.GroupOffsetX = 0.0f;
        window->DC.ColumnsOffsetX = 0.0f;
        window->DC.CursorStartPos = window->Pos + ImVec2(window->DC.IndentX + window->DC.ColumnsOffsetX, window->TitleBarHeight() + window->MenuBarHeight() + window->WindowPadding.y - window->Scroll.y);
        window->DC.CursorPos = window->DC.CursorStartPos;
        window->DC.CursorPosPrevLine = window->DC.CursorPos;
        window->DC.CursorMaxPos = window->DC.CursorStartPos;
        window->DC.CurrentLineHeight = window->DC.PrevLineHeight = 0.0f;
        window->DC.CurrentLineTextBaseOffset = window->DC.PrevLineTextBaseOffset = 0.0f;
        window->DC.MenuBarAppending = false;
        window->DC.MenuBarOffsetX = ImMax(window->WindowPadding.x, style.ItemSpacing.x);
        window->DC.LogLinePosY = window->DC.CursorPos.y - 9999.0f;
        window->DC.ChildWindows.resize(0);
        window->DC.LayoutType = ImGuiLayoutType_Vertical;
        window->DC.ItemFlags = ImGuiItemFlags_Default_;
        window->DC.ItemWidth = window->ItemWidthDefault;
        window->DC.TextWrapPos = -1.0f; // disabled
        window->DC.ItemFlagsStack.resize(0);
        window->DC.ItemWidthStack.resize(0);
        window->DC.TextWrapPosStack.resize(0);
        window->DC.ColumnsCurrent = 0;
        window->DC.ColumnsCount = 1;
        window->DC.ColumnsStartPosY = window->DC.CursorPos.y;
        window->DC.ColumnsStartMaxPosX = window->DC.CursorMaxPos.x;
        window->DC.ColumnsCellMinY = window->DC.ColumnsCellMaxY = window->DC.ColumnsStartPosY;
        window->DC.TreeDepth = 0;
        window->DC.StateStorage = &window->StateStorage;
        window->DC.GroupStack.resize(0);
        window->MenuColumns.Update(3, style.ItemSpacing.x, window_just_activated_by_user);

        if ((flags & ImGuiWindowFlags_ChildWindow) && (window->DC.ItemFlags != parent_window->DC.ItemFlags))
        {
            window->DC.ItemFlags = parent_window->DC.ItemFlags;
            window->DC.ItemFlagsStack.push_back(window->DC.ItemFlags);
        }

        if (window->AutoFitFramesX > 0)
            window->AutoFitFramesX--;
        if (window->AutoFitFramesY > 0)
            window->AutoFitFramesY--;

        // New windows appears in front (we need to do that AFTER setting DC.CursorStartPos so our initial navigation reference rectangle can start around there)
        if (window_just_activated_by_user && !(flags & ImGuiWindowFlags_NoFocusOnAppearing))
            if (!(flags & (ImGuiWindowFlags_ChildWindow|ImGuiWindowFlags_Tooltip)) || (flags & ImGuiWindowFlags_Popup))
                FocusWindow(window);

        // Title bar
        if (!(flags & ImGuiWindowFlags_NoTitleBar))
        {
            // Collapse button
            if (!(flags & ImGuiWindowFlags_NoCollapse))
            {
                RenderTriangle(window->Pos + style.FramePadding, window->Collapsed ? ImGuiDir_Right : ImGuiDir_Down, 1.0f);
            }

            // Close button
            /*if (p_open != NULL)
            {
                const float PAD = 2.0f;
                const float rad = (window->TitleBarHeight() - PAD*2.0f) * 0.5f;
                if (CloseButton(window->GetID("#CLOSE"), window->Rect().GetTR() + ImVec2(-PAD - rad, PAD + rad), rad))
                    *p_open = false;
            }*/

            // Title text (FIXME: refactor text alignment facilities along with RenderText helpers)
            const ImVec2 text_size = CalcTextSize(name, NULL, true);
            ImVec2 text_min = window->Pos;
            ImVec2 text_max = window->Pos + ImVec2(window->Size.x, style.FramePadding.y*2 + text_size.y);
            ImRect clip_rect;
            clip_rect.Max = ImVec2(window->Pos.x + window->Size.x - (p_open ? title_bar_rect.GetHeight() - 3 : style.FramePadding.x), text_max.y); // Match the size of CloseWindowButton()
            float pad_left = (flags & ImGuiWindowFlags_NoCollapse) == 0 ? (style.FramePadding.x + g.FontSize + style.ItemInnerSpacing.x) : style.FramePadding.x;
            float pad_right = (p_open != NULL) ? (style.FramePadding.x + g.FontSize + style.ItemInnerSpacing.x) : style.FramePadding.x;
            if (style.WindowTitleAlign.x > 0.0f) pad_right = ImLerp(pad_right, pad_left, style.WindowTitleAlign.x);
            text_min.x += pad_left;
            text_max.x -= pad_right;
            clip_rect.Min = ImVec2(text_min.x, window->Pos.y);
            RenderTextClipped(text_min, text_max, name, NULL, &text_size, style.WindowTitleAlign, &clip_rect);
        }

        // Save clipped aabb so we can access it in constant-time in FindHoveredWindow()
        window->WindowRectClipped = window->Rect();
        window->WindowRectClipped.ClipWith(window->ClipRect);

        // Pressing CTRL+C while holding on a window copy its content to the clipboard
        // This works but 1. doesn't handle multiple Begin/End pairs, 2. recursing into another Begin/End pair - so we need to work that out and add better logging scope.
        // Maybe we can support CTRL+C on every element?
        /*
        if (g.ActiveId == move_id)
            if (g.IO.KeyCtrl && IsKeyPressedMap(ImGuiKey_C))
                ImGui::LogToClipboard();
        */
    }

    // Inner clipping rectangle
    // We set this up after processing the resize grip so that our clip rectangle doesn't lag by a frame
    // Note that if our window is collapsed we will end up with a null clipping rectangle which is the correct behavior.
    const ImRect title_bar_rect = window->TitleBarRect();
    const float border_size = window->BorderSize;
	// Force round to ensure that e.g. (int)(max.x-min.x) in user's render code produce correct result.
    ImRect clip_rect;
    clip_rect.Min.x = ImFloor(0.5f + title_bar_rect.Min.x + ImMax(border_size, ImFloor(window->WindowPadding.x*0.5f)));
    clip_rect.Min.y = ImFloor(0.5f + title_bar_rect.Max.y + window->MenuBarHeight() + border_size);
    clip_rect.Max.x = ImFloor(0.5f + window->Pos.x + window->Size.x - window->ScrollbarSizes.x - ImMax(border_size, ImFloor(window->WindowPadding.x*0.5f)));
    clip_rect.Max.y = ImFloor(0.5f + window->Pos.y + window->Size.y - window->ScrollbarSizes.y - border_size);
    PushClipRect(clip_rect.Min, clip_rect.Max, true);

    // Clear 'accessed' flag last thing
    if (first_begin_of_the_frame)
        window->Accessed = false;
    window->BeginCount++;
    g.SetNextWindowSizeConstraint = false;

    // Child window can be out of sight and have "negative" clip windows.
    // Mark them as collapsed so commands are skipped earlier (we can't manually collapse because they have no title bar).
    if (flags & ImGuiWindowFlags_ChildWindow)
    {
        IM_ASSERT((flags & ImGuiWindowFlags_NoTitleBar) != 0);
        window->Collapsed = parent_window && parent_window->Collapsed;

        if (!(flags & ImGuiWindowFlags_AlwaysAutoResize) && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0)
            window->Collapsed |= (window->WindowRectClipped.Min.x >= window->WindowRectClipped.Max.x || window->WindowRectClipped.Min.y >= window->WindowRectClipped.Max.y);

        // We also hide the window from rendering because we've already added its border to the command list.
        // (we could perform the check earlier in the function but it is simpler at this point)
        if (window->Collapsed)
            window->Active = false;
    }
    if (style.Alpha <= 0.0f)
        window->Active = false;

    // Return false if we don't intend to display anything to allow user to perform an early out optimization
    window->SkipItems = (window->Collapsed || !window->Active) && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0;
    return !window->SkipItems;
}

// Old Begin() API with 5 parameters, avoid calling this version directly! Use SetNextWindowSize()+Begin() instead.
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
bool ImGui::Begin(const char* name, bool* p_open, const ImVec2& size_on_first_use, float bg_alpha_override, ImGuiWindowFlags flags)
{
    // Old API feature: we could pass the initial window size as a parameter, however this was very misleading because in most cases it would only affect the window when it didn't have storage in the .ini file.
    if (size_on_first_use.x != 0.0f || size_on_first_use.y != 0.0f)
        SetNextWindowSize(size_on_first_use, ImGuiCond_FirstUseEver);

    // Old API feature: we could override the window background alpha with a parameter. This is actually tricky to reproduce manually because: 
    // (1) there are multiple variants of WindowBg (popup, tooltip, etc.) and (2) you can't call PushStyleColor before Begin and PopStyleColor just after Begin() because of how CheckStackSizes() behave.
    // The user-side solution is to do backup = GetStyleColorVec4(ImGuiCol_xxxBG), PushStyleColor(ImGuiCol_xxxBg), Begin, PushStyleColor(ImGuiCol_xxxBg, backup), [...], PopStyleColor(), End(); PopStyleColor() - which is super awkward.
    // The alpha override was rarely used but for now we'll leave the Begin() variant around for a bit. We may either lift the constraint on CheckStackSizes() either add a SetNextWindowBgAlpha() helper that does it magically.
    ImGuiContext& g = *GImGui;
    const ImGuiCol bg_color_idx = GetWindowBgColorIdxFromFlags(flags);
    const ImVec4 bg_color_backup = g.Style.Colors[bg_color_idx];
    if (bg_alpha_override >= 0.0f)
        g.Style.Colors[bg_color_idx].w = bg_alpha_override;

    bool ret = Begin(name, p_open, flags);

    if (bg_alpha_override >= 0.0f)
        g.Style.Colors[bg_color_idx] = bg_color_backup;
    return ret;
}
#endif // IMGUI_DISABLE_OBSOLETE_FUNCTIONS

void ImGui::End()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    if (window->DC.ColumnsCount != 1) // close columns set if any is open
        EndColumns();
    PopClipRect();   // inner window clip rectangle

    // Stop logging
    if (!(window->Flags & ImGuiWindowFlags_ChildWindow))    // FIXME: add more options for scope of logging
        LogFinish();

    // Pop
    // NB: we don't clear 'window->RootWindow'. The pointer is allowed to live until the next call to Begin().
    g.CurrentWindowStack.pop_back();
    if (window->Flags & ImGuiWindowFlags_Popup)
        g.CurrentPopupStack.pop_back();
    CheckStacksSize(window, false);
    SetCurrentWindow(g.CurrentWindowStack.empty() ? NULL : g.CurrentWindowStack.back());
}

// Vertical scrollbar
// The entire piece of code below is rather confusing because:
// - We handle absolute seeking (when first clicking outside the grab) and relative manipulation (afterward or when clicking inside the grab)
// - We store values as normalized ratio and in a form that allows the window content to change while we are holding on a scrollbar
// - We handle both horizontal and vertical scrollbars, which makes the terminology not ideal.
void ImGui::Scrollbar(ImGuiLayoutType direction)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const bool horizontal = (direction == ImGuiLayoutType_Horizontal);
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(horizontal ? "#SCROLLX" : "#SCROLLY");

    // Render background
    bool other_scrollbar = (horizontal ? window->ScrollbarY : window->ScrollbarX);
    float other_scrollbar_size_w = other_scrollbar ? style.ScrollbarSize : 0.0f;
    const ImRect window_rect = window->Rect();
    const float border_size = window->BorderSize;
    ImRect bb = horizontal
        ? ImRect(window->Pos.x + border_size, window_rect.Max.y - style.ScrollbarSize, window_rect.Max.x - other_scrollbar_size_w - border_size, window_rect.Max.y - border_size)
        : ImRect(window_rect.Max.x - style.ScrollbarSize, window->Pos.y + border_size, window_rect.Max.x - border_size, window_rect.Max.y - other_scrollbar_size_w - border_size);
    if (!horizontal)
        bb.Min.y += window->TitleBarHeight() + ((window->Flags & ImGuiWindowFlags_MenuBar) ? window->MenuBarHeight() : 0.0f);
    if (bb.GetWidth() <= 0.0f || bb.GetHeight() <= 0.0f)
        return;

    float window_rounding = (window->Flags & ImGuiWindowFlags_ChildWindow) ? style.ChildWindowRounding : style.WindowRounding;
    int window_rounding_corners;
    if (horizontal)
        window_rounding_corners = ImGuiCorner_BotLeft | (other_scrollbar ? 0 : ImGuiCorner_BotRight);
    else
        window_rounding_corners = (((window->Flags & ImGuiWindowFlags_NoTitleBar) && !(window->Flags & ImGuiWindowFlags_MenuBar)) ? ImGuiCorner_TopRight : 0) | (other_scrollbar ? 0 : ImGuiCorner_BotRight);
    window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_ScrollbarBg), window_rounding, window_rounding_corners);
    bb.Expand(ImVec2(-ImClamp((float)(int)((bb.Max.x - bb.Min.x - 2.0f) * 0.5f), 0.0f, 3.0f), -ImClamp((float)(int)((bb.Max.y - bb.Min.y - 2.0f) * 0.5f), 0.0f, 3.0f)));

    // V denote the main, longer axis of the scrollbar (= height for a vertical scrollbar)
    float scrollbar_size_v = horizontal ? bb.GetWidth() : bb.GetHeight();
    float scroll_v = horizontal ? window->Scroll.x : window->Scroll.y;
    float win_size_avail_v = (horizontal ? window->SizeFull.x : window->SizeFull.y) - other_scrollbar_size_w;
    float win_size_contents_v = horizontal ? window->SizeContents.x : window->SizeContents.y;

    // Calculate the height of our grabbable box. It generally represent the amount visible (vs the total scrollable amount)
    // But we maintain a minimum size in pixel to allow for the user to still aim inside.
    IM_ASSERT(ImMax(win_size_contents_v, win_size_avail_v) > 0.0f); // Adding this assert to check if the ImMax(XXX,1.0f) is still needed. PLEASE CONTACT ME if this triggers.
    const float win_size_v = ImMax(ImMax(win_size_contents_v, win_size_avail_v), 1.0f);
    const float grab_h_pixels = ImClamp(scrollbar_size_v * (win_size_avail_v / win_size_v), style.GrabMinSize, scrollbar_size_v);
    const float grab_h_norm = grab_h_pixels / scrollbar_size_v;

    // Handle input right away. None of the code of Begin() is relying on scrolling position before calling Scrollbar().
    bool held = false;
    bool hovered = false;
    const bool previously_held = (g.ActiveId == id);
    ButtonBehavior(bb, id, &hovered, &held);

    float scroll_max = ImMax(1.0f, win_size_contents_v - win_size_avail_v);
    float scroll_ratio = ImSaturate(scroll_v / scroll_max);
    float grab_v_norm = scroll_ratio * (scrollbar_size_v - grab_h_pixels) / scrollbar_size_v;
    if (held && grab_h_norm < 1.0f)
    {
        float scrollbar_pos_v = horizontal ? bb.Min.x : bb.Min.y;
        float mouse_pos_v = horizontal ? g.IO.MousePos.x : g.IO.MousePos.y;
        float* click_delta_to_grab_center_v = horizontal ? &g.ScrollbarClickDeltaToGrabCenter.x : &g.ScrollbarClickDeltaToGrabCenter.y;

        // Click position in scrollbar normalized space (0.0f->1.0f)
        const float clicked_v_norm = ImSaturate((mouse_pos_v - scrollbar_pos_v) / scrollbar_size_v);
        SetHoveredID(id);

        bool seek_absolute = false;
        if (!previously_held)
        {
            // On initial click calculate the distance between mouse and the center of the grab
            if (clicked_v_norm >= grab_v_norm && clicked_v_norm <= grab_v_norm + grab_h_norm)
            {
                *click_delta_to_grab_center_v = clicked_v_norm - grab_v_norm - grab_h_norm*0.5f;
            }
            else
            {
                seek_absolute = true;
                *click_delta_to_grab_center_v = 0.0f;
            }
        }

        // Apply scroll
        // It is ok to modify Scroll here because we are being called in Begin() after the calculation of SizeContents and before setting up our starting position
        const float scroll_v_norm = ImSaturate((clicked_v_norm - *click_delta_to_grab_center_v - grab_h_norm*0.5f) / (1.0f - grab_h_norm));
        scroll_v = (float)(int)(0.5f + scroll_v_norm * scroll_max);//(win_size_contents_v - win_size_v));
        if (horizontal)
            window->Scroll.x = scroll_v;
        else
            window->Scroll.y = scroll_v;

        // Update values for rendering
        scroll_ratio = ImSaturate(scroll_v / scroll_max);
        grab_v_norm = scroll_ratio * (scrollbar_size_v - grab_h_pixels) / scrollbar_size_v;

        // Update distance to grab now that we have seeked and saturated
        if (seek_absolute)
            *click_delta_to_grab_center_v = clicked_v_norm - grab_v_norm - grab_h_norm*0.5f;
    }

    // Render
    const ImU32 grab_col = GetColorU32(held ? ImGuiCol_ScrollbarGrabActive : hovered ? ImGuiCol_ScrollbarGrabHovered : ImGuiCol_ScrollbarGrab);
    if (horizontal)
        window->DrawList->AddRectFilled(ImVec2(ImLerp(bb.Min.x, bb.Max.x, grab_v_norm), bb.Min.y), ImVec2(ImLerp(bb.Min.x, bb.Max.x, grab_v_norm) + grab_h_pixels, bb.Max.y), grab_col, style.ScrollbarRounding);
    else
        window->DrawList->AddRectFilled(ImVec2(bb.Min.x, ImLerp(bb.Min.y, bb.Max.y, grab_v_norm)), ImVec2(bb.Max.x, ImLerp(bb.Min.y, bb.Max.y, grab_v_norm) + grab_h_pixels), grab_col, style.ScrollbarRounding);
}

// Moving window to front of display (which happens to be back of our sorted list)
void ImGui::FocusWindow(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;

    // Always mark the window we passed as focused. This is used for keyboard interactions such as tabbing.
    g.NavWindow = window;

    // Passing NULL allow to disable keyboard focus
    if (!window)
        return;

    // And move its root window to the top of the pile
    if (window->RootWindow)
        window = window->RootWindow;

    // Steal focus on active widgets
    if (window->Flags & ImGuiWindowFlags_Popup) // FIXME: This statement should be unnecessary. Need further testing before removing it..
        if (g.ActiveId != 0 && g.ActiveIdWindow && g.ActiveIdWindow->RootWindow != window)
            ClearActiveID();

    // Bring to front
    if ((window->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus) || g.Windows.back() == window)
        return;
    for (int i = 0; i < g.Windows.Size; i++)
        if (g.Windows[i] == window)
        {
            g.Windows.erase(g.Windows.begin() + i);
            break;
        }
    g.Windows.push_back(window);
}

void ImGui::PushItemWidth(float item_width)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.ItemWidth = (item_width == 0.0f ? window->ItemWidthDefault : item_width);
    window->DC.ItemWidthStack.push_back(window->DC.ItemWidth);
}

void ImGui::PushMultiItemsWidths(int components, float w_full)
{
    ImGuiWindow* window = GetCurrentWindow();
    const ImGuiStyle& style = GImGui->Style;
    if (w_full <= 0.0f)
        w_full = CalcItemWidth();
    const float w_item_one  = ImMax(1.0f, (float)(int)((w_full - (style.ItemInnerSpacing.x) * (components-1)) / (float)components));
    const float w_item_last = ImMax(1.0f, (float)(int)(w_full - (w_item_one + style.ItemInnerSpacing.x) * (components-1)));
    window->DC.ItemWidthStack.push_back(w_item_last);
    for (int i = 0; i < components-1; i++)
        window->DC.ItemWidthStack.push_back(w_item_one);
    window->DC.ItemWidth = window->DC.ItemWidthStack.back();
}

void ImGui::PopItemWidth()
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.ItemWidthStack.pop_back();
    window->DC.ItemWidth = window->DC.ItemWidthStack.empty() ? window->ItemWidthDefault : window->DC.ItemWidthStack.back();
}

float ImGui::CalcItemWidth()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    float w = window->DC.ItemWidth;
    if (w < 0.0f)
    {
        // Align to a right-side limit. We include 1 frame padding in the calculation because this is how the width is always used (we add 2 frame padding to it), but we could move that responsibility to the widget as well.
        float width_to_right_edge = GetContentRegionAvail().x;
        w = ImMax(1.0f, width_to_right_edge + w);
    }
    w = (float)(int)w;
    return w;
}

static ImFont* GetDefaultFont()
{
    ImGuiContext& g = *GImGui;
    return g.IO.FontDefault ? g.IO.FontDefault : g.IO.Fonts->Fonts[0];
}

static void SetCurrentFont(ImFont* font)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(font && font->IsLoaded());    // Font Atlas not created. Did you call io.Fonts->GetTexDataAsRGBA32 / GetTexDataAsAlpha8 ?
    IM_ASSERT(font->Scale > 0.0f);
    g.Font = font;
    g.FontBaseSize = g.IO.FontGlobalScale * g.Font->FontSize * g.Font->Scale;
    g.FontSize = g.CurrentWindow ? g.CurrentWindow->CalcFontSize() : 0.0f;
    g.FontTexUvWhitePixel = g.Font->ContainerAtlas->TexUvWhitePixel;
}

void ImGui::PushFont(ImFont* font)
{
    ImGuiContext& g = *GImGui;
    if (!font)
        font = GetDefaultFont();
    SetCurrentFont(font);
    g.FontStack.push_back(font);
    g.CurrentWindow->DrawList->PushTextureID(font->ContainerAtlas->TexID);
}

void  ImGui::PopFont()
{
    ImGuiContext& g = *GImGui;
    g.CurrentWindow->DrawList->PopTextureID();
    g.FontStack.pop_back();
    SetCurrentFont(g.FontStack.empty() ? GetDefaultFont() : g.FontStack.back());
}

void ImGui::PushItemFlag(ImGuiItemFlags option, bool enabled)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (enabled)
        window->DC.ItemFlags |= option;
    else
        window->DC.ItemFlags &= ~option;
    window->DC.ItemFlagsStack.push_back(window->DC.ItemFlags);
}

void ImGui::PopItemFlag()
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.ItemFlagsStack.pop_back();
    window->DC.ItemFlags = window->DC.ItemFlagsStack.empty() ? ImGuiItemFlags_Default_ : window->DC.ItemFlagsStack.back();
}

void ImGui::PushAllowKeyboardFocus(bool allow_keyboard_focus)
{
    PushItemFlag(ImGuiItemFlags_AllowKeyboardFocus, allow_keyboard_focus);
}

void ImGui::PopAllowKeyboardFocus()
{
    PopItemFlag();
}

void ImGui::PushButtonRepeat(bool repeat)
{
    PushItemFlag(ImGuiItemFlags_ButtonRepeat, repeat);
}

void ImGui::PopButtonRepeat()
{
    PopItemFlag();
}

void ImGui::PushTextWrapPos(float wrap_pos_x)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.TextWrapPos = wrap_pos_x;
    window->DC.TextWrapPosStack.push_back(wrap_pos_x);
}

void ImGui::PopTextWrapPos()
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.TextWrapPosStack.pop_back();
    window->DC.TextWrapPos = window->DC.TextWrapPosStack.empty() ? -1.0f : window->DC.TextWrapPosStack.back();
}

// FIXME: This may incur a round-trip (if the end user got their data from a float4) but eventually we aim to store the in-flight colors as ImU32
void ImGui::PushStyleColor(ImGuiCol idx, ImU32 col)
{
    ImGuiContext& g = *GImGui;
    ImGuiColMod backup;
    backup.Col = idx;
    backup.BackupValue = g.Style.Colors[idx];
    g.ColorModifiers.push_back(backup);
    g.Style.Colors[idx] = ColorConvertU32ToFloat4(col);
}

void ImGui::PushStyleColor(ImGuiCol idx, const ImVec4& col)
{
    ImGuiContext& g = *GImGui;
    ImGuiColMod backup;
    backup.Col = idx;
    backup.BackupValue = g.Style.Colors[idx];
    g.ColorModifiers.push_back(backup);
    g.Style.Colors[idx] = col;
}

void ImGui::PopStyleColor(int count)
{
    ImGuiContext& g = *GImGui;
    while (count > 0)
    {
        ImGuiColMod& backup = g.ColorModifiers.back();
        g.Style.Colors[backup.Col] = backup.BackupValue;
        g.ColorModifiers.pop_back();
        count--;
    }
}

struct ImGuiStyleVarInfo
{
    ImGuiDataType   Type;
    ImU32           Offset;
    void*           GetVarPtr(ImGuiStyle* style) const { return (void*)((unsigned char*)style + Offset); }
};

static const ImGuiStyleVarInfo GStyleVarInfo[ImGuiStyleVar_Count_] =
{
    { ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, Alpha) },                // ImGuiStyleVar_Alpha
    { ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, WindowPadding) },        // ImGuiStyleVar_WindowPadding
    { ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, WindowRounding) },       // ImGuiStyleVar_WindowRounding
    { ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, WindowMinSize) },        // ImGuiStyleVar_WindowMinSize
    { ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, ChildWindowRounding) },  // ImGuiStyleVar_ChildWindowRounding
    { ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, FramePadding) },         // ImGuiStyleVar_FramePadding
    { ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, FrameRounding) },        // ImGuiStyleVar_FrameRounding
    { ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, ItemSpacing) },          // ImGuiStyleVar_ItemSpacing
    { ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, ItemInnerSpacing) },     // ImGuiStyleVar_ItemInnerSpacing
    { ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, IndentSpacing) },        // ImGuiStyleVar_IndentSpacing
    { ImGuiDataType_Float,  (ImU32)IM_OFFSETOF(ImGuiStyle, GrabMinSize) },          // ImGuiStyleVar_GrabMinSize
    { ImGuiDataType_Float2, (ImU32)IM_OFFSETOF(ImGuiStyle, ButtonTextAlign) },      // ImGuiStyleVar_ButtonTextAlign
};

static const ImGuiStyleVarInfo* GetStyleVarInfo(ImGuiStyleVar idx)
{
    IM_ASSERT(idx >= 0 && idx < ImGuiStyleVar_Count_);
    return &GStyleVarInfo[idx];
}

void ImGui::PushStyleVar(ImGuiStyleVar idx, float val)
{
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    if (var_info->Type == ImGuiDataType_Float)
    {
        ImGuiContext& g = *GImGui;
        float* pvar = (float*)var_info->GetVarPtr(&g.Style);
        g.StyleModifiers.push_back(ImGuiStyleMod(idx, *pvar));
        *pvar = val;
        return;
    }
    IM_ASSERT(0); // Called function with wrong-type? Variable is not a float.
}

void ImGui::PushStyleVar(ImGuiStyleVar idx, const ImVec2& val)
{
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    if (var_info->Type == ImGuiDataType_Float2)
    {
        ImGuiContext& g = *GImGui;
        ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&g.Style);
        g.StyleModifiers.push_back(ImGuiStyleMod(idx, *pvar));
        *pvar = val;
        return;
    }
    IM_ASSERT(0); // Called function with wrong-type? Variable is not a ImVec2.
}

void ImGui::PopStyleVar(int count)
{
    ImGuiContext& g = *GImGui;
    while (count > 0)
    {
        ImGuiStyleMod& backup = g.StyleModifiers.back();
        const ImGuiStyleVarInfo* info = GetStyleVarInfo(backup.VarIdx);
        if (info->Type == ImGuiDataType_Float)          (*(float*)info->GetVarPtr(&g.Style)) = backup.BackupFloat[0];
        else if (info->Type == ImGuiDataType_Float2)    (*(ImVec2*)info->GetVarPtr(&g.Style)) = ImVec2(backup.BackupFloat[0], backup.BackupFloat[1]);
        else if (info->Type == ImGuiDataType_Int)       (*(int*)info->GetVarPtr(&g.Style)) = backup.BackupInt[0];
        g.StyleModifiers.pop_back();
        count--;
    }
}

const char* ImGui::GetStyleColorName(ImGuiCol idx)
{
    // Create switch-case from enum with regexp: ImGuiCol_{.*}, --> case ImGuiCol_\1: return "\1";
    switch (idx)
    {
    case ImGuiCol_Text: return "Text";
    case ImGuiCol_TextDisabled: return "TextDisabled";
    case ImGuiCol_WindowBg: return "WindowBg";
    case ImGuiCol_ChildWindowBg: return "ChildWindowBg";
    case ImGuiCol_PopupBg: return "PopupBg";
    case ImGuiCol_Border: return "Border";
    case ImGuiCol_BorderShadow: return "BorderShadow";
    case ImGuiCol_FrameBg: return "FrameBg";
    case ImGuiCol_FrameBgHovered: return "FrameBgHovered";
    case ImGuiCol_FrameBgActive: return "FrameBgActive";
    case ImGuiCol_TitleBg: return "TitleBg";
    case ImGuiCol_TitleBgActive: return "TitleBgActive";
    case ImGuiCol_TitleBgCollapsed: return "TitleBgCollapsed";
    case ImGuiCol_MenuBarBg: return "MenuBarBg";
    case ImGuiCol_ScrollbarBg: return "ScrollbarBg";
    case ImGuiCol_ScrollbarGrab: return "ScrollbarGrab";
    case ImGuiCol_ScrollbarGrabHovered: return "ScrollbarGrabHovered";
    case ImGuiCol_ScrollbarGrabActive: return "ScrollbarGrabActive";
    case ImGuiCol_ComboBg: return "ComboBg";
    case ImGuiCol_CheckMark: return "CheckMark";
    case ImGuiCol_SliderGrab: return "SliderGrab";
    case ImGuiCol_SliderGrabActive: return "SliderGrabActive";
    case ImGuiCol_Button: return "Button";
    case ImGuiCol_ButtonHovered: return "ButtonHovered";
    case ImGuiCol_ButtonActive: return "ButtonActive";
    case ImGuiCol_Header: return "Header";
    case ImGuiCol_HeaderHovered: return "HeaderHovered";
    case ImGuiCol_HeaderActive: return "HeaderActive";
    case ImGuiCol_Separator: return "Separator";
    case ImGuiCol_SeparatorHovered: return "SeparatorHovered";
    case ImGuiCol_SeparatorActive: return "SeparatorActive";
    case ImGuiCol_ResizeGrip: return "ResizeGrip";
    case ImGuiCol_ResizeGripHovered: return "ResizeGripHovered";
    case ImGuiCol_ResizeGripActive: return "ResizeGripActive";
    case ImGuiCol_CloseButton: return "CloseButton";
    case ImGuiCol_CloseButtonHovered: return "CloseButtonHovered";
    case ImGuiCol_CloseButtonActive: return "CloseButtonActive";
    case ImGuiCol_PlotLines: return "PlotLines";
    case ImGuiCol_PlotLinesHovered: return "PlotLinesHovered";
    case ImGuiCol_PlotHistogram: return "PlotHistogram";
    case ImGuiCol_PlotHistogramHovered: return "PlotHistogramHovered";
    case ImGuiCol_TextSelectedBg: return "TextSelectedBg";
    case ImGuiCol_ModalWindowDarkening: return "ModalWindowDarkening";
    }
    IM_ASSERT(0);
    return "Unknown";
}

bool ImGui::IsWindowHovered()
{
    ImGuiContext& g = *GImGui;
    return g.HoveredWindow == g.CurrentWindow && IsWindowContentHoverable(g.HoveredRootWindow);
}

bool ImGui::IsWindowRectHovered()
{
    ImGuiContext& g = *GImGui;
    return g.HoveredWindow == g.CurrentWindow;
}

bool ImGui::IsWindowFocused()
{
    ImGuiContext& g = *GImGui;
    return g.NavWindow == g.CurrentWindow;
}

bool ImGui::IsRootWindowFocused()
{
    ImGuiContext& g = *GImGui;
    return g.NavWindow == g.CurrentWindow->RootWindow;
}

bool ImGui::IsRootWindowOrAnyChildFocused()
{
    ImGuiContext& g = *GImGui;
    return g.NavWindow && g.NavWindow->RootWindow == g.CurrentWindow->RootWindow;
}

bool ImGui::IsRootWindowOrAnyChildHovered()
{
    ImGuiContext& g = *GImGui;
    return g.HoveredRootWindow && (g.HoveredRootWindow == g.CurrentWindow->RootWindow) && IsWindowContentHoverable(g.HoveredRootWindow);
}

float ImGui::GetWindowWidth()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->Size.x;
}

float ImGui::GetWindowHeight()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->Size.y;
}

ImVec2 ImGui::GetWindowPos()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    return window->Pos;
}

static void SetWindowScrollY(ImGuiWindow* window, float new_scroll_y)
{
    window->DC.CursorMaxPos.y += window->Scroll.y; // SizeContents is generally computed based on CursorMaxPos which is affected by scroll position, so we need to apply our change to it.
    window->Scroll.y = new_scroll_y;
    window->DC.CursorMaxPos.y -= window->Scroll.y;
}

static void SetWindowPos(ImGuiWindow* window, const ImVec2& pos, ImGuiCond cond)
{
    // Test condition (NB: bit 0 is always true) and clear flags for next time
    if (cond && (window->SetWindowPosAllowFlags & cond) == 0)
        return;
    window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
    window->SetWindowPosVal = ImVec2(FLT_MAX, FLT_MAX);

    // Set
    const ImVec2 old_pos = window->Pos;
    window->PosFloat = pos;
    window->Pos = ImVec2((float)(int)window->PosFloat.x, (float)(int)window->PosFloat.y);
    window->DC.CursorPos += (window->Pos - old_pos);    // As we happen to move the window while it is being appended to (which is a bad idea - will smear) let's at least offset the cursor
    window->DC.CursorMaxPos += (window->Pos - old_pos); // And more importantly we need to adjust this so size calculation doesn't get affected.
}

void ImGui::SetWindowPos(const ImVec2& pos, ImGuiCond cond)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    SetWindowPos(window, pos, cond);
}

void ImGui::SetWindowPos(const char* name, const ImVec2& pos, ImGuiCond cond)
{
    if (ImGuiWindow* window = FindWindowByName(name))
        SetWindowPos(window, pos, cond);
}

ImVec2 ImGui::GetWindowSize()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->Size;
}

static void SetWindowSize(ImGuiWindow* window, const ImVec2& size, ImGuiCond cond)
{
    // Test condition (NB: bit 0 is always true) and clear flags for next time
    if (cond && (window->SetWindowSizeAllowFlags & cond) == 0)
        return;
    window->SetWindowSizeAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

    // Set
    if (size.x > 0.0f)
    {
        window->AutoFitFramesX = 0;
        window->SizeFull.x = size.x;
    }
    else
    {
        window->AutoFitFramesX = 2;
        window->AutoFitOnlyGrows = false;
    }
    if (size.y > 0.0f)
    {
        window->AutoFitFramesY = 0;
        window->SizeFull.y = size.y;
    }
    else
    {
        window->AutoFitFramesY = 2;
        window->AutoFitOnlyGrows = false;
    }
}

void ImGui::SetWindowSize(const ImVec2& size, ImGuiCond cond)
{
    SetWindowSize(GImGui->CurrentWindow, size, cond);
}

void ImGui::SetWindowSize(const char* name, const ImVec2& size, ImGuiCond cond)
{
    if (ImGuiWindow* window = FindWindowByName(name))
        SetWindowSize(window, size, cond);
}

static void SetWindowCollapsed(ImGuiWindow* window, bool collapsed, ImGuiCond cond)
{
    // Test condition (NB: bit 0 is always true) and clear flags for next time
    if (cond && (window->SetWindowCollapsedAllowFlags & cond) == 0)
        return;
    window->SetWindowCollapsedAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

    // Set
    window->Collapsed = collapsed;
}

void ImGui::SetWindowCollapsed(bool collapsed, ImGuiCond cond)
{
    SetWindowCollapsed(GImGui->CurrentWindow, collapsed, cond);
}

bool ImGui::IsWindowCollapsed()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->Collapsed;
}

bool ImGui::IsWindowAppearing()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->Appearing;
}

void ImGui::SetWindowCollapsed(const char* name, bool collapsed, ImGuiCond cond)
{
    ImGuiWindow* window = FindWindowByName(name);
    if (window)
        SetWindowCollapsed(window, collapsed, cond);
}

void ImGui::SetWindowFocus()
{
    FocusWindow(GImGui->CurrentWindow);
}

void ImGui::SetWindowFocus(const char* name)
{
    if (name)
    {
        if (ImGuiWindow* window = FindWindowByName(name))
            FocusWindow(window);
    }
    else
    {
        FocusWindow(NULL);
    }
}

void ImGui::SetNextWindowPos(const ImVec2& pos, ImGuiCond cond, const ImVec2& pivot)
{
    ImGuiContext& g = *GImGui;
    g.SetNextWindowPosVal = pos;
    g.SetNextWindowPosPivot = pivot;
    g.SetNextWindowPosCond = cond ? cond : ImGuiCond_Always;
}

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
void ImGui::SetNextWindowPosCenter(ImGuiCond cond)
{
    SetNextWindowPos(GetIO().DisplaySize * 0.5f, cond, ImVec2(0.5f, 0.5f));
}
#endif

void ImGui::SetNextWindowSize(const ImVec2& size, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    g.SetNextWindowSizeVal = size;
    g.SetNextWindowSizeCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::SetNextWindowSizeConstraints(const ImVec2& size_min, const ImVec2& size_max, ImGuiSizeConstraintCallback custom_callback, void* custom_callback_user_data)
{
    ImGuiContext& g = *GImGui;
    g.SetNextWindowSizeConstraint = true;
    g.SetNextWindowSizeConstraintRect = ImRect(size_min, size_max);
    g.SetNextWindowSizeConstraintCallback = custom_callback;
    g.SetNextWindowSizeConstraintCallbackUserData = custom_callback_user_data;
}

void ImGui::SetNextWindowContentSize(const ImVec2& size)
{
    ImGuiContext& g = *GImGui;
    g.SetNextWindowContentSizeVal = size;
    g.SetNextWindowContentSizeCond = ImGuiCond_Always;
}

void ImGui::SetNextWindowContentWidth(float width)
{
    ImGuiContext& g = *GImGui;
    g.SetNextWindowContentSizeVal = ImVec2(width, g.SetNextWindowContentSizeCond ? g.SetNextWindowContentSizeVal.y : 0.0f);
    g.SetNextWindowContentSizeCond = ImGuiCond_Always;
}

void ImGui::SetNextWindowCollapsed(bool collapsed, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    g.SetNextWindowCollapsedVal = collapsed;
    g.SetNextWindowCollapsedCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::SetNextWindowFocus()
{
    ImGuiContext& g = *GImGui;
    g.SetNextWindowFocus = true;
}

// In window space (not screen space!)
ImVec2 ImGui::GetContentRegionMax()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    ImVec2 mx = window->ContentsRegionRect.Max;
    if (window->DC.ColumnsCount != 1)
        mx.x = GetColumnOffset(window->DC.ColumnsCurrent + 1) - window->WindowPadding.x;
    return mx;
}

ImVec2 ImGui::GetContentRegionAvail()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return GetContentRegionMax() - (window->DC.CursorPos - window->Pos);
}

float ImGui::GetContentRegionAvailWidth()
{
    return GetContentRegionAvail().x;
}

// In window space (not screen space!)
ImVec2 ImGui::GetWindowContentRegionMin()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->ContentsRegionRect.Min;
}

ImVec2 ImGui::GetWindowContentRegionMax()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->ContentsRegionRect.Max;
}

float ImGui::GetWindowContentRegionWidth()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->ContentsRegionRect.Max.x - window->ContentsRegionRect.Min.x;
}

float ImGui::GetTextLineHeight()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize;
}

float ImGui::GetTextLineHeightWithSpacing()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize + g.Style.ItemSpacing.y;
}

float ImGui::GetItemsLineHeightWithSpacing()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize + g.Style.FramePadding.y * 2.0f + g.Style.ItemSpacing.y;
}

ImDrawList* ImGui::GetWindowDrawList()
{
    ImGuiWindow* window = GetCurrentWindow();
    return window->DrawList;
}

ImFont* ImGui::GetFont()
{
    return GImGui->Font;
}

float ImGui::GetFontSize()
{
    return GImGui->FontSize;
}

ImVec2 ImGui::GetFontTexUvWhitePixel()
{
    return GImGui->FontTexUvWhitePixel;
}

void ImGui::SetWindowFontScale(float scale)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    window->FontWindowScale = scale;
    g.FontSize = window->CalcFontSize();
}

// User generally sees positions in window coordinates. Internally we store CursorPos in absolute screen coordinates because it is more convenient.
// Conversion happens as we pass the value to user, but it makes our naming convention confusing because GetCursorPos() == (DC.CursorPos - window.Pos). May want to rename 'DC.CursorPos'.
ImVec2 ImGui::GetCursorPos()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorPos - window->Pos + window->Scroll;
}

float ImGui::GetCursorPosX()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorPos.x - window->Pos.x + window->Scroll.x;
}

float ImGui::GetCursorPosY()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorPos.y - window->Pos.y + window->Scroll.y;
}

void ImGui::SetCursorPos(const ImVec2& local_pos)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.CursorPos = window->Pos - window->Scroll + local_pos;
    window->DC.CursorMaxPos = ImMax(window->DC.CursorMaxPos, window->DC.CursorPos);
}

void ImGui::SetCursorPosX(float x)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + x;
    window->DC.CursorMaxPos.x = ImMax(window->DC.CursorMaxPos.x, window->DC.CursorPos.x);
}

void ImGui::SetCursorPosY(float y)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.CursorPos.y = window->Pos.y - window->Scroll.y + y;
    window->DC.CursorMaxPos.y = ImMax(window->DC.CursorMaxPos.y, window->DC.CursorPos.y);
}

ImVec2 ImGui::GetCursorStartPos()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorStartPos - window->Pos;
}

ImVec2 ImGui::GetCursorScreenPos()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorPos;
}

void ImGui::SetCursorScreenPos(const ImVec2& screen_pos)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.CursorPos = screen_pos;
    window->DC.CursorMaxPos = ImMax(window->DC.CursorMaxPos, window->DC.CursorPos);
}

float ImGui::GetScrollX()
{
    return GImGui->CurrentWindow->Scroll.x;
}

float ImGui::GetScrollY()
{
    return GImGui->CurrentWindow->Scroll.y;
}

float ImGui::GetScrollMaxX()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return ImMax(0.0f, window->SizeContents.x - (window->SizeFull.x - window->ScrollbarSizes.x));
}

float ImGui::GetScrollMaxY()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return ImMax(0.0f, window->SizeContents.y - (window->SizeFull.y - window->ScrollbarSizes.y));
}

void ImGui::SetScrollX(float scroll_x)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->ScrollTarget.x = scroll_x;
    window->ScrollTargetCenterRatio.x = 0.0f;
}

void ImGui::SetScrollY(float scroll_y)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->ScrollTarget.y = scroll_y + window->TitleBarHeight() + window->MenuBarHeight(); // title bar height canceled out when using ScrollTargetRelY
    window->ScrollTargetCenterRatio.y = 0.0f;
}

void ImGui::SetScrollFromPosY(float pos_y, float center_y_ratio)
{
    // We store a target position so centering can occur on the next frame when we are guaranteed to have a known window size
    ImGuiWindow* window = GetCurrentWindow();
    IM_ASSERT(center_y_ratio >= 0.0f && center_y_ratio <= 1.0f);
    window->ScrollTarget.y = (float)(int)(pos_y + window->Scroll.y);
    if (center_y_ratio <= 0.0f && window->ScrollTarget.y <= window->WindowPadding.y)    // Minor hack to make "scroll to top" take account of WindowPadding, else it would scroll to (WindowPadding.y - ItemSpacing.y)
        window->ScrollTarget.y = 0.0f;
    window->ScrollTargetCenterRatio.y = center_y_ratio;
}

// center_y_ratio: 0.0f top of last item, 0.5f vertical center of last item, 1.0f bottom of last item.
void ImGui::SetScrollHere(float center_y_ratio)
{
    ImGuiWindow* window = GetCurrentWindow();
    float target_y = window->DC.CursorPosPrevLine.y - window->Pos.y; // Top of last item, in window space
    target_y += (window->DC.PrevLineHeight * center_y_ratio) + (GImGui->Style.ItemSpacing.y * (center_y_ratio - 0.5f) * 2.0f); // Precisely aim above, in the middle or below the last line.
    SetScrollFromPosY(target_y, center_y_ratio);
}

void ImGui::SetKeyboardFocusHere(int offset)
{
    IM_ASSERT(offset >= -1);    // -1 is allowed but not below
    ImGuiWindow* window = GetCurrentWindow();
    window->FocusIdxAllRequestNext = window->FocusIdxAllCounter + 1 + offset;
    window->FocusIdxTabRequestNext = INT_MAX;
}

void ImGui::SetStateStorage(ImGuiStorage* tree)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.StateStorage = tree ? tree : &window->StateStorage;
}

ImGuiStorage* ImGui::GetStateStorage()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.StateStorage;
}

void ImGui::TextV(const char* fmt, va_list args)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const char* text_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    TextUnformatted(g.TempBuffer, text_end);
}

void ImGui::Text(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    TextV(fmt, args);
    va_end(args);
}

void ImGui::TextColoredV(const ImVec4& col, const char* fmt, va_list args)
{
    PushStyleColor(ImGuiCol_Text, col);
    TextV(fmt, args);
    PopStyleColor();
}

void ImGui::TextColored(const ImVec4& col, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    TextColoredV(col, fmt, args);
    va_end(args);
}

void ImGui::TextDisabledV(const char* fmt, va_list args)
{
    PushStyleColor(ImGuiCol_Text, GImGui->Style.Colors[ImGuiCol_TextDisabled]);
    TextV(fmt, args);
    PopStyleColor();
}

void ImGui::TextDisabled(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    TextDisabledV(fmt, args);
    va_end(args);
}

void ImGui::TextWrappedV(const char* fmt, va_list args)
{
    bool need_wrap = (GImGui->CurrentWindow->DC.TextWrapPos < 0.0f);    // Keep existing wrap position is one ia already set
    if (need_wrap) PushTextWrapPos(0.0f);
    TextV(fmt, args);
    if (need_wrap) PopTextWrapPos();
}

void ImGui::TextWrapped(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    TextWrappedV(fmt, args);
    va_end(args);
}

void ImGui::TextUnformatted(const char* text, const char* text_end)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    IM_ASSERT(text != NULL);
    const char* text_begin = text;
    if (text_end == NULL)
        text_end = text + strlen(text); // FIXME-OPT

    const ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrentLineTextBaseOffset);
    const float wrap_pos_x = window->DC.TextWrapPos;
    const bool wrap_enabled = wrap_pos_x >= 0.0f;
    if (text_end - text > 2000 && !wrap_enabled)
    {
        // Long text!
        // Perform manual coarse clipping to optimize for long multi-line text
        // From this point we will only compute the width of lines that are visible. Optimization only available when word-wrapping is disabled.
        // We also don't vertically center the text within the line full height, which is unlikely to matter because we are likely the biggest and only item on the line.
        const char* line = text;
        const float line_height = GetTextLineHeight();
        const ImRect clip_rect = window->ClipRect;
        ImVec2 text_size(0,0);

        if (text_pos.y <= clip_rect.Max.y)
        {
            ImVec2 pos = text_pos;

            // Lines to skip (can't skip when logging text)
            if (!g.LogEnabled)
            {
                int lines_skippable = (int)((clip_rect.Min.y - text_pos.y) / line_height);
                if (lines_skippable > 0)
                {
                    int lines_skipped = 0;
                    while (line < text_end && lines_skipped < lines_skippable)
                    {
                        const char* line_end = strchr(line, '\n');
                        if (!line_end)
                            line_end = text_end;
                        line = line_end + 1;
                        lines_skipped++;
                    }
                    pos.y += lines_skipped * line_height;
                }
            }

            // Lines to render
            if (line < text_end)
            {
                ImRect line_rect(pos, pos + ImVec2(FLT_MAX, line_height));
                while (line < text_end)
                {
                    const char* line_end = strchr(line, '\n');
                    if (IsClippedEx(line_rect, 0, false))
                        break;

                    const ImVec2 line_size = CalcTextSize(line, line_end, false);
                    text_size.x = ImMax(text_size.x, line_size.x);
                    RenderText(pos, line, line_end, false);
                    if (!line_end)
                        line_end = text_end;
                    line = line_end + 1;
                    line_rect.Min.y += line_height;
                    line_rect.Max.y += line_height;
                    pos.y += line_height;
                }

                // Count remaining lines
                int lines_skipped = 0;
                while (line < text_end)
                {
                    const char* line_end = strchr(line, '\n');
                    if (!line_end)
                        line_end = text_end;
                    line = line_end + 1;
                    lines_skipped++;
                }
                pos.y += lines_skipped * line_height;
            }

            text_size.y += (pos - text_pos).y;
        }

        ImRect bb(text_pos, text_pos + text_size);
        ItemSize(bb);
        ItemAdd(bb, 0);
    }
    else
    {
        const float wrap_width = wrap_enabled ? CalcWrapWidthForPos(window->DC.CursorPos, wrap_pos_x) : 0.0f;
        const ImVec2 text_size = CalcTextSize(text_begin, text_end, false, wrap_width);

        // Account of baseline offset
        ImRect bb(text_pos, text_pos + text_size);
        ItemSize(text_size);
        if (!ItemAdd(bb, 0))
            return;

        // Render (we don't hide text after ## in this end-user function)
        RenderTextWrapped(bb.Min, text_begin, text_end, wrap_width);
    }
}

void ImGui::AlignTextToFramePadding()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    window->DC.CurrentLineHeight = ImMax(window->DC.CurrentLineHeight, g.FontSize + g.Style.FramePadding.y * 2);
    window->DC.CurrentLineTextBaseOffset = ImMax(window->DC.CurrentLineTextBaseOffset, g.Style.FramePadding.y);
}

// Add a label+text combo aligned to other label+value widgets
void ImGui::LabelTextV(const char* label, const char* fmt, va_list args)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float w = CalcItemWidth();

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImRect value_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y*2));
    const ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w + (label_size.x > 0.0f ? style.ItemInnerSpacing.x : 0.0f), style.FramePadding.y*2) + label_size);
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, 0))
        return;

    // Render
    const char* value_text_begin = &g.TempBuffer[0];
    const char* value_text_end = value_text_begin + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    RenderTextClipped(value_bb.Min, value_bb.Max, value_text_begin, value_text_end, NULL, ImVec2(0.0f,0.5f));
    if (label_size.x > 0.0f)
        RenderText(ImVec2(value_bb.Max.x + style.ItemInnerSpacing.x, value_bb.Min.y + style.FramePadding.y), label);
}

void ImGui::LabelText(const char* label, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LabelTextV(label, fmt, args);
    va_end(args);
}

bool ImGui::ButtonBehavior(const ImRect& bb, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();

    if (flags & ImGuiButtonFlags_Disabled)
    {
        if (out_hovered) *out_hovered = false;
        if (out_held) *out_held = false;
        if (g.ActiveId == id) ClearActiveID();
        return false;
    }

    // Default behavior requires click+release on same spot
    if ((flags & (ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_PressedOnRelease | ImGuiButtonFlags_PressedOnDoubleClick)) == 0)
        flags |= ImGuiButtonFlags_PressedOnClickRelease;

    ImGuiWindow* backup_hovered_window = g.HoveredWindow;
    if ((flags & ImGuiButtonFlags_FlattenChilds) && g.HoveredRootWindow == window)
        g.HoveredWindow = window;

    bool pressed = false;
    bool hovered = ItemHoverable(bb, id);

    if ((flags & ImGuiButtonFlags_FlattenChilds) && g.HoveredRootWindow == window)
        g.HoveredWindow = backup_hovered_window;

    if (hovered)
    {
        if (!(flags & ImGuiButtonFlags_NoKeyModifiers) || (!g.IO.KeyCtrl && !g.IO.KeyShift && !g.IO.KeyAlt))
        {
            //                        | CLICKING        | HOLDING with ImGuiButtonFlags_Repeat
            // PressedOnClickRelease  |  <on release>*  |  <on repeat> <on repeat> .. (NOT on release)  <-- MOST COMMON! (*) only if both click/release were over bounds
            // PressedOnClick         |  <on click>     |  <on click> <on repeat> <on repeat> ..
            // PressedOnRelease       |  <on release>   |  <on repeat> <on repeat> .. (NOT on release)
            // PressedOnDoubleClick   |  <on dclick>    |  <on dclick> <on repeat> <on repeat> ..
            if ((flags & ImGuiButtonFlags_PressedOnClickRelease) && g.IO.MouseClicked[0])
            {
                SetActiveID(id, window); // Hold on ID
                FocusWindow(window);
                g.ActiveIdClickOffset = g.IO.MousePos - bb.Min;
            }
            if (((flags & ImGuiButtonFlags_PressedOnClick) && g.IO.MouseClicked[0]) || ((flags & ImGuiButtonFlags_PressedOnDoubleClick) && g.IO.MouseDoubleClicked[0]))
            {
                pressed = true;
                if (flags & ImGuiButtonFlags_NoHoldingActiveID)
                    ClearActiveID();
                else
                    SetActiveID(id, window); // Hold on ID
                FocusWindow(window);
                g.ActiveIdClickOffset = g.IO.MousePos - bb.Min;
            }
            if ((flags & ImGuiButtonFlags_PressedOnRelease) && g.IO.MouseReleased[0])
            {
                if (!((flags & ImGuiButtonFlags_Repeat) && g.IO.MouseDownDurationPrev[0] >= g.IO.KeyRepeatDelay))  // Repeat mode trumps <on release>
                    pressed = true;
                ClearActiveID();
            }

            // 'Repeat' mode acts when held regardless of _PressedOn flags (see table above). 
            // Relies on repeat logic of IsMouseClicked() but we may as well do it ourselves if we end up exposing finer RepeatDelay/RepeatRate settings.
            if ((flags & ImGuiButtonFlags_Repeat) && g.ActiveId == id && g.IO.MouseDownDuration[0] > 0.0f && IsMouseClicked(0, true))
                pressed = true;
        }
    }

    bool held = false;
    if (g.ActiveId == id)
    {
        if (g.IO.MouseDown[0])
        {
            held = true;
        }
        else
        {
            if (hovered && (flags & ImGuiButtonFlags_PressedOnClickRelease))
                if (!((flags & ImGuiButtonFlags_Repeat) && g.IO.MouseDownDurationPrev[0] >= g.IO.KeyRepeatDelay))  // Repeat mode trumps <on release>
                    pressed = true;
            ClearActiveID();
        }
    }

    // AllowOverlap mode (rarely used) requires previous frame HoveredId to be null or to match. This allows using patterns where a later submitted widget overlaps a previous one.
    if (hovered && (flags & ImGuiButtonFlags_AllowOverlapMode) && (g.HoveredIdPreviousFrame != id && g.HoveredIdPreviousFrame != 0))
        hovered = pressed = held = false;

    if (out_hovered) *out_hovered = hovered;
    if (out_held) *out_held = held;

    return pressed;
}

bool ImGui::ButtonEx(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

    if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
    RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    // Automatically close popups
    //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    return pressed;
}

bool ImGui::Button(const char* label, const ImVec2& size_arg)
{
    return ButtonEx(label, size_arg, 0);
}

// Small buttons fits within text without additional vertical spacing.
bool ImGui::SmallButton(const char* label)
{
    ImGuiContext& g = *GImGui;
    float backup_padding_y = g.Style.FramePadding.y;
    g.Style.FramePadding.y = 0.0f;
    bool pressed = ButtonEx(label, ImVec2(0,0), ImGuiButtonFlags_AlignTextBaseLine);
    g.Style.FramePadding.y = backup_padding_y;
    return pressed;
}

// Tip: use ImGui::PushID()/PopID() to push indices or pointers in the ID stack.
// Then you can keep 'str_id' empty or the same for all your buttons (instead of creating a string based on a non-string id)
bool ImGui::InvisibleButton(const char* str_id, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImGuiID id = window->GetID(str_id);
    ImVec2 size = CalcItemSize(size_arg, 0.0f, 0.0f);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    ItemSize(bb);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    return pressed;
}

// Upper-right button to close a window.
bool ImGui::CloseButton(ImGuiID id, const ImVec2& pos, float radius)
{
    ImGuiWindow* window = GetCurrentWindow();

	ImRect bb(pos - ImVec2(radius, radius - 1), pos + ImVec2(radius - 1, radius));

	bb = ImRect(bb.Min + ImVec2(3 - bb.GetWidth(), 0), bb.Max + ImVec2(3, 0));

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

	// Render
	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_CloseButtonActive : hovered ? ImGuiCol_CloseButtonHovered : ImGuiCol_CloseButton);
	const ImVec2 center = bb.GetCenter();
	const float window_rounding = (window->Flags & ImGuiWindowFlags_ChildWindow) ? ImGui::GetStyle().ChildWindowRounding : ImGui::GetStyle().WindowRounding;
	const auto pad = ImGui::GetStyle().WindowPadThickness * 0.33f;
	RenderFrame(bb.Min + ImVec2(0, pad), bb.Max - ImVec2(0, pad) , col, true, window_rounding);

    return pressed;
}

void ImGui::Image(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    if (border_col.w > 0.0f)
        bb.Max += ImVec2(2,2);
    ItemSize(bb);
    if (!ItemAdd(bb, 0))
        return;

    if (border_col.w > 0.0f)
    {
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
        window->DrawList->AddImage(user_texture_id, bb.Min+ImVec2(1,1), bb.Max-ImVec2(1,1), uv0, uv1, GetColorU32(tint_col));
    }
    else
    {
        window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
    }
}

// frame_padding < 0: uses FramePadding from style (default)
// frame_padding = 0: no framing
// frame_padding > 0: set framing size
// The color used are the button colors.
bool ImGui::ImageButton(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Default to using texture ID as ID. User can still push string/integer prefixes.
    // We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
    PushID((void *)user_texture_id);
    const ImGuiID id = window->GetID("#image");
    PopID();

    const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding*2);
    const ImRect image_bb(window->DC.CursorPos + padding, window->DC.CursorPos + padding + size);
    ItemSize(bb);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    // Render
    const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
    if (bg_col.w > 0.0f)
        window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));
    window->DrawList->AddImage(user_texture_id, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(tint_col));

    return pressed;
}

// Start logging ImGui output to TTY
void ImGui::LogToTTY(int max_depth)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;
    ImGuiWindow* window = g.CurrentWindow;

    g.LogEnabled = true;
    g.LogFile = stdout;
    g.LogStartDepth = window->DC.TreeDepth;
    if (max_depth >= 0)
        g.LogAutoExpandMaxDepth = max_depth;
}

// Start logging ImGui output to given file
void ImGui::LogToFile(int max_depth, const char* filename)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;
    ImGuiWindow* window = g.CurrentWindow;

    if (!filename)
    {
        filename = g.IO.LogFilename;
        if (!filename)
            return;
    }

    g.LogFile = ImFileOpen(filename, "ab");
    if (!g.LogFile)
    {
        IM_ASSERT(g.LogFile != NULL); // Consider this an error
        return;
    }
    g.LogEnabled = true;
    g.LogStartDepth = window->DC.TreeDepth;
    if (max_depth >= 0)
        g.LogAutoExpandMaxDepth = max_depth;
}

// Start logging ImGui output to clipboard
void ImGui::LogToClipboard(int max_depth)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;
    ImGuiWindow* window = g.CurrentWindow;

    g.LogEnabled = true;
    g.LogFile = NULL;
    g.LogStartDepth = window->DC.TreeDepth;
    if (max_depth >= 0)
        g.LogAutoExpandMaxDepth = max_depth;
}

void ImGui::LogFinish()
{
    ImGuiContext& g = *GImGui;
    if (!g.LogEnabled)
        return;

    LogText(IM_NEWLINE);
    g.LogEnabled = false;
    if (g.LogFile != NULL)
    {
        if (g.LogFile == stdout)
            fflush(g.LogFile);
        else
            fclose(g.LogFile);
        g.LogFile = NULL;
    }
    if (g.LogClipboard->size() > 1)
    {
        SetClipboardText(g.LogClipboard->begin());
        g.LogClipboard->clear();
    }
}

// Helper to display logging buttons
void ImGui::LogButtons()
{
    ImGuiContext& g = *GImGui;

    PushID("LogButtons");
    const bool log_to_tty = Button("Log To TTY"); SameLine();
    const bool log_to_file = Button("Log To File"); SameLine();
    const bool log_to_clipboard = Button("Log To Clipboard"); SameLine();
    PushItemWidth(80.0f);
    PushAllowKeyboardFocus(false);
    SliderInt("Depth", &g.LogAutoExpandMaxDepth, 0, 9, NULL);
    PopAllowKeyboardFocus();
    PopItemWidth();
    PopID();

    // Start logging at the end of the function so that the buttons don't appear in the log
    if (log_to_tty)
        LogToTTY(g.LogAutoExpandMaxDepth);
    if (log_to_file)
        LogToFile(g.LogAutoExpandMaxDepth, g.IO.LogFilename);
    if (log_to_clipboard)
        LogToClipboard(g.LogAutoExpandMaxDepth);
}

bool ImGui::TreeNodeBehaviorIsOpen(ImGuiID id, ImGuiTreeNodeFlags flags)
{
    if (flags & ImGuiTreeNodeFlags_Leaf)
        return true;

    // We only write to the tree storage if the user clicks (or explicitely use SetNextTreeNode*** functions)
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiStorage* storage = window->DC.StateStorage;

    bool is_open;
    if (g.SetNextTreeNodeOpenCond != 0)
    {
        if (g.SetNextTreeNodeOpenCond & ImGuiCond_Always)
        {
            is_open = g.SetNextTreeNodeOpenVal;
            storage->SetInt(id, is_open);
        }
        else
        {
            // We treat ImGuiCond_Once and ImGuiCond_FirstUseEver the same because tree node state are not saved persistently.
            const int stored_value = storage->GetInt(id, -1);
            if (stored_value == -1)
            {
                is_open = g.SetNextTreeNodeOpenVal;
                storage->SetInt(id, is_open);
            }
            else
            {
                is_open = stored_value != 0;
            }
        }
        g.SetNextTreeNodeOpenCond = 0;
    }
    else
    {
        is_open = storage->GetInt(id, (flags & ImGuiTreeNodeFlags_DefaultOpen) ? 1 : 0) != 0;
    }

    // When logging is enabled, we automatically expand tree nodes (but *NOT* collapsing headers.. seems like sensible behavior).
    // NB- If we are above max depth we still allow manually opened nodes to be logged.
    if (g.LogEnabled && !(flags & ImGuiTreeNodeFlags_NoAutoOpenOnLog) && window->DC.TreeDepth < g.LogAutoExpandMaxDepth)
        is_open = true;

    return is_open;
}

bool ImGui::TreeNodeBehavior(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const bool display_frame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
    const ImVec2 padding = (display_frame || (flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, 0.0f);

    if (!label_end)
        label_end = FindRenderedTextEnd(label);
    const ImVec2 label_size = CalcTextSize(label, label_end, false);

    // We vertically grow up to current line height up the typical widget height.
    const float text_base_offset_y = ImMax(padding.y, window->DC.CurrentLineTextBaseOffset); // Latch before ItemSize changes it
    const float frame_height = ImMax(ImMin(window->DC.CurrentLineHeight, g.FontSize + style.FramePadding.y*2), label_size.y + padding.y*2);
    ImRect bb = ImRect(window->DC.CursorPos, ImVec2(window->Pos.x + GetContentRegionMax().x, window->DC.CursorPos.y + frame_height));
    if (display_frame)
    {
        // Framed header expand a little outside the default padding
        bb.Min.x -= (float)(int)(window->WindowPadding.x*0.5f) - 1;
        bb.Max.x += (float)(int)(window->WindowPadding.x*0.5f) - 1;
    }

    const float text_offset_x = (g.FontSize + (display_frame ? padding.x*3 : padding.x*2));   // Collapser arrow width + Spacing
    const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x*2 : 0.0f);   // Include collapser
    ItemSize(ImVec2(text_width, frame_height), text_base_offset_y);

    // For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
    // (Ideally we'd want to add a flag for the user to specify if we want the hit test to be done up to the right side of the content or not)
    const ImRect interact_bb = display_frame ? bb : ImRect(bb.Min.x, bb.Min.y, bb.Min.x + text_width + style.ItemSpacing.x*2, bb.Max.y);
    bool is_open = TreeNodeBehaviorIsOpen(id, flags);
    if (!ItemAdd(interact_bb, id))
    {
        if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
            TreePushRawID(id);
        return is_open;
    }

    // Flags that affects opening behavior:
    // - 0(default) ..................... single-click anywhere to open
    // - OpenOnDoubleClick .............. double-click anywhere to open
    // - OpenOnArrow .................... single-click on arrow to open
    // - OpenOnDoubleClick|OpenOnArrow .. single-click on arrow or double-click anywhere to open
    ImGuiButtonFlags button_flags = ImGuiButtonFlags_NoKeyModifiers | ((flags & ImGuiTreeNodeFlags_AllowOverlapMode) ? ImGuiButtonFlags_AllowOverlapMode : 0);
    if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
        button_flags |= ImGuiButtonFlags_PressedOnDoubleClick | ((flags & ImGuiTreeNodeFlags_OpenOnArrow) ? ImGuiButtonFlags_PressedOnClickRelease : 0);
    bool hovered, held, pressed = ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
    if (pressed && !(flags & ImGuiTreeNodeFlags_Leaf))
    {
        bool toggled = !(flags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick));
        if (flags & ImGuiTreeNodeFlags_OpenOnArrow)
            toggled |= IsMouseHoveringRect(interact_bb.Min, ImVec2(interact_bb.Min.x + text_offset_x, interact_bb.Max.y));
        if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
            toggled |= g.IO.MouseDoubleClicked[0];
        if (toggled)
        {
            is_open = !is_open;
            window->DC.StateStorage->SetInt(id, is_open);
        }
    }
    if (flags & ImGuiTreeNodeFlags_AllowOverlapMode)
        SetItemAllowOverlap();

    // Render
    const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
    const ImVec2 text_pos = bb.Min + ImVec2(text_offset_x, text_base_offset_y);
    if (display_frame)
    {
        // Framed type
        RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
        RenderTriangle(bb.Min + ImVec2(padding.x, text_base_offset_y), is_open ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
        if (g.LogEnabled)
        {
            // NB: '##' is normally used to hide text (as a library-wide feature), so we need to specify the text range to make sure the ## aren't stripped out here.
            const char log_prefix[] = "\n##";
            const char log_suffix[] = "##";
            LogRenderedText(&text_pos, log_prefix, log_prefix+3);
            RenderTextClipped(text_pos, bb.Max, label, label_end, &label_size);
            LogRenderedText(&text_pos, log_suffix+1, log_suffix+3);
        }
        else
        {
            RenderTextClipped(text_pos, bb.Max, label, label_end, &label_size);
        }
    }
    else
    {
        // Unframed typed for tree nodes
        if (hovered || (flags & ImGuiTreeNodeFlags_Selected))
            RenderFrame(bb.Min, bb.Max, col, false);

        if (flags & ImGuiTreeNodeFlags_Bullet)
            RenderBullet(bb.Min + ImVec2(text_offset_x * 0.5f, g.FontSize*0.50f + text_base_offset_y));
        else if (!(flags & ImGuiTreeNodeFlags_Leaf))
            RenderTriangle(bb.Min + ImVec2(padding.x, g.FontSize*0.15f + text_base_offset_y), is_open ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);
        if (g.LogEnabled)
            LogRenderedText(&text_pos, ">");
        RenderText(text_pos, label, label_end, false);
    }

    if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
        TreePushRawID(id);
    return is_open;
}

// CollapsingHeader returns true when opened but do not indent nor push into the ID stack (because of the ImGuiTreeNodeFlags_NoTreePushOnOpen flag).
// This is basically the same as calling TreeNodeEx(label, ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_NoTreePushOnOpen). You can remove the _NoTreePushOnOpen flag if you want behavior closer to normal TreeNode().
bool ImGui::CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    return TreeNodeBehavior(window->GetID(label), flags | ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_NoTreePushOnOpen, label);
}

bool ImGui::CollapsingHeader(const char* label, bool* p_open, ImGuiTreeNodeFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    if (p_open && !*p_open)
        return false;

    ImGuiID id = window->GetID(label);
    bool is_open = TreeNodeBehavior(id, flags | ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_NoTreePushOnOpen | (p_open ? ImGuiTreeNodeFlags_AllowOverlapMode : 0), label);
    if (p_open)
    {
        // Create a small overlapping close button // FIXME: We can evolve this into user accessible helpers to add extra buttons on title bars, headers, etc.
        ImGuiContext& g = *GImGui;
        float button_sz = g.FontSize * 0.5f;
        ImGuiItemHoveredDataBackup last_item_backup;
        last_item_backup.Backup();
        if (CloseButton(window->GetID((void*)(intptr_t)(id+1)), ImVec2(ImMin(window->DC.LastItemRect.Max.x, window->ClipRect.Max.x) - g.Style.FramePadding.x - button_sz, window->DC.LastItemRect.Min.y + g.Style.FramePadding.y + button_sz), button_sz))
            *p_open = false;
        last_item_backup.Restore();
    }

    return is_open;
}

bool ImGui::TreeNodeEx(const char* label, ImGuiTreeNodeFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    return TreeNodeBehavior(window->GetID(label), flags, label, NULL);
}

bool ImGui::TreeNodeExV(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const char* label_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    return TreeNodeBehavior(window->GetID(str_id), flags, g.TempBuffer, label_end);
}

bool ImGui::TreeNodeExV(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const char* label_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    return TreeNodeBehavior(window->GetID(ptr_id), flags, g.TempBuffer, label_end);
}

bool ImGui::TreeNodeV(const char* str_id, const char* fmt, va_list args)
{
    return TreeNodeExV(str_id, 0, fmt, args);
}

bool ImGui::TreeNodeV(const void* ptr_id, const char* fmt, va_list args)
{
    return TreeNodeExV(ptr_id, 0, fmt, args);
}

bool ImGui::TreeNodeEx(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    bool is_open = TreeNodeExV(str_id, flags, fmt, args);
    va_end(args);
    return is_open;
}

bool ImGui::TreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    bool is_open = TreeNodeExV(ptr_id, flags, fmt, args);
    va_end(args);
    return is_open;
}

bool ImGui::TreeNode(const char* str_id, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    bool is_open = TreeNodeExV(str_id, 0, fmt, args);
    va_end(args);
    return is_open;
}

bool ImGui::TreeNode(const void* ptr_id, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    bool is_open = TreeNodeExV(ptr_id, 0, fmt, args);
    va_end(args);
    return is_open;
}

bool ImGui::TreeNode(const char* label)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    return TreeNodeBehavior(window->GetID(label), 0, label, NULL);
}

void ImGui::TreeAdvanceToLabelPos()
{
    ImGuiContext& g = *GImGui;
    g.CurrentWindow->DC.CursorPos.x += GetTreeNodeToLabelSpacing();
}

// Horizontal distance preceding label when using TreeNode() or Bullet()
float ImGui::GetTreeNodeToLabelSpacing()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize + (g.Style.FramePadding.x * 2.0f);
}

void ImGui::SetNextTreeNodeOpen(bool is_open, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    g.SetNextTreeNodeOpenVal = is_open;
    g.SetNextTreeNodeOpenCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::PushID(const char* str_id)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    window->IDStack.push_back(window->GetID(str_id));
}

void ImGui::PushID(const char* str_id_begin, const char* str_id_end)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    window->IDStack.push_back(window->GetID(str_id_begin, str_id_end));
}

void ImGui::PushID(const void* ptr_id)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    window->IDStack.push_back(window->GetID(ptr_id));
}

void ImGui::PushID(int int_id)
{
    const void* ptr_id = (void*)(intptr_t)int_id;
    ImGuiWindow* window = GetCurrentWindowRead();
    window->IDStack.push_back(window->GetID(ptr_id));
}

void ImGui::PopID()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    window->IDStack.pop_back();
}

ImGuiID ImGui::GetID(const char* str_id)
{
    return GImGui->CurrentWindow->GetID(str_id);
}

ImGuiID ImGui::GetID(const char* str_id_begin, const char* str_id_end)
{
    return GImGui->CurrentWindow->GetID(str_id_begin, str_id_end);
}

ImGuiID ImGui::GetID(const void* ptr_id)
{
    return GImGui->CurrentWindow->GetID(ptr_id);
}

void ImGui::Bullet()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float line_height = ImMax(ImMin(window->DC.CurrentLineHeight, g.FontSize + g.Style.FramePadding.y*2), g.FontSize);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(g.FontSize, line_height));
    ItemSize(bb);
    if (!ItemAdd(bb, 0))
    {
        SameLine(0, style.FramePadding.x*2);
        return;
    }

    // Render and stay on same line
    RenderBullet(bb.Min + ImVec2(style.FramePadding.x + g.FontSize*0.5f, line_height*0.5f));
    SameLine(0, style.FramePadding.x*2);
}

// Text with a little bullet aligned to the typical tree node.
void ImGui::BulletTextV(const char* fmt, va_list args)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const char* text_begin = g.TempBuffer;
    const char* text_end = text_begin + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    const ImVec2 label_size = CalcTextSize(text_begin, text_end, false);
    const float text_base_offset_y = ImMax(0.0f, window->DC.CurrentLineTextBaseOffset); // Latch before ItemSize changes it
    const float line_height = ImMax(ImMin(window->DC.CurrentLineHeight, g.FontSize + g.Style.FramePadding.y*2), g.FontSize);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(g.FontSize + (label_size.x > 0.0f ? (label_size.x + style.FramePadding.x*2) : 0.0f), ImMax(line_height, label_size.y)));  // Empty text doesn't add padding
    ItemSize(bb);
    if (!ItemAdd(bb, 0))
        return;

    // Render
    RenderBullet(bb.Min + ImVec2(style.FramePadding.x + g.FontSize*0.5f, line_height*0.5f));
    RenderText(bb.Min+ImVec2(g.FontSize + style.FramePadding.x*2, text_base_offset_y), text_begin, text_end, false);
}

void ImGui::BulletText(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    BulletTextV(fmt, args);
    va_end(args);
}

static inline void DataTypeFormatString(ImGuiDataType data_type, void* data_ptr, const char* display_format, char* buf, int buf_size)
{
    if (data_type == ImGuiDataType_Int)
        ImFormatString(buf, buf_size, display_format, *(int*)data_ptr);
    else if (data_type == ImGuiDataType_Float)
        ImFormatString(buf, buf_size, display_format, *(float*)data_ptr);
}

static inline void DataTypeFormatString(ImGuiDataType data_type, void* data_ptr, int decimal_precision, char* buf, int buf_size)
{
    if (data_type == ImGuiDataType_Int)
    {
        if (decimal_precision < 0)
            ImFormatString(buf, buf_size, "%d", *(int*)data_ptr);
        else
            ImFormatString(buf, buf_size, "%.*d", decimal_precision, *(int*)data_ptr);
    }
    else if (data_type == ImGuiDataType_Float)
    {
        if (decimal_precision < 0)
            ImFormatString(buf, buf_size, "%f", *(float*)data_ptr);     // Ideally we'd have a minimum decimal precision of 1 to visually denote that it is a float, while hiding non-significant digits?
        else
            ImFormatString(buf, buf_size, "%.*f", decimal_precision, *(float*)data_ptr);
    }
}

static void DataTypeApplyOp(ImGuiDataType data_type, int op, void* value1, const void* value2)// Store into value1
{
    if (data_type == ImGuiDataType_Int)
    {
        if (op == '+')
            *(int*)value1 = *(int*)value1 + *(const int*)value2;
        else if (op == '-')
            *(int*)value1 = *(int*)value1 - *(const int*)value2;
    }
    else if (data_type == ImGuiDataType_Float)
    {
        if (op == '+')
            *(float*)value1 = *(float*)value1 + *(const float*)value2;
        else if (op == '-')
            *(float*)value1 = *(float*)value1 - *(const float*)value2;
    }
}

// User can input math operators (e.g. +100) to edit a numerical values.
static bool DataTypeApplyOpFromText(const char* buf, const char* initial_value_buf, ImGuiDataType data_type, void* data_ptr, const char* scalar_format)
{
    while (ImCharIsSpace(*buf))
        buf++;

    // We don't support '-' op because it would conflict with inputing negative value.
    // Instead you can use +-100 to subtract from an existing value
    char op = buf[0];
    if (op == '+' || op == '*' || op == '/')
    {
        buf++;
        while (ImCharIsSpace(*buf))
            buf++;
    }
    else
    {
        op = 0;
    }
    if (!buf[0])
        return false;

    if (data_type == ImGuiDataType_Int)
    {
        if (!scalar_format)
            scalar_format = "%d";
        int* v = (int*)data_ptr;
        const int old_v = *v;
        int arg0i = *v;
        if (op && sscanf(initial_value_buf, scalar_format, &arg0i) < 1)
            return false;

        // Store operand in a float so we can use fractional value for multipliers (*1.1), but constant always parsed as integer so we can fit big integers (e.g. 2000000003) past float precision
        float arg1f = 0.0f;
        if (op == '+')      { if (sscanf(buf, "%f", &arg1f) == 1) *v = (int)(arg0i + arg1f); }                 // Add (use "+-" to subtract)
        else if (op == '*') { if (sscanf(buf, "%f", &arg1f) == 1) *v = (int)(arg0i * arg1f); }                 // Multiply
        else if (op == '/') { if (sscanf(buf, "%f", &arg1f) == 1 && arg1f != 0.0f) *v = (int)(arg0i / arg1f); }// Divide
        else                { if (sscanf(buf, scalar_format, &arg0i) == 1) *v = arg0i; }                       // Assign constant (read as integer so big values are not lossy)
        return (old_v != *v);
    }
    else if (data_type == ImGuiDataType_Float)
    {
        // For floats we have to ignore format with precision (e.g. "%.2f") because sscanf doesn't take them in
        scalar_format = "%f";
        float* v = (float*)data_ptr;
        const float old_v = *v;
        float arg0f = *v;
        if (op && sscanf(initial_value_buf, scalar_format, &arg0f) < 1)
            return false;

        float arg1f = 0.0f;
        if (sscanf(buf, scalar_format, &arg1f) < 1)
            return false;
        if (op == '+')      { *v = arg0f + arg1f; }                    // Add (use "+-" to subtract)
        else if (op == '*') { *v = arg0f * arg1f; }                    // Multiply
        else if (op == '/') { if (arg1f != 0.0f) *v = arg0f / arg1f; } // Divide
        else                { *v = arg1f; }                            // Assign constant
        return (old_v != *v);
    }

    return false;
}

// Create text input in place of a slider (when CTRL+Clicking on slider)
// FIXME: Logic is messy and confusing.
bool ImGui::InputScalarAsWidgetReplacement(const ImRect& aabb, const char* label, ImGuiDataType data_type, void* data_ptr, ImGuiID id, int decimal_precision)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();

    // Our replacement widget will override the focus ID (registered previously to allow for a TAB focus to happen)
    // On the first frame, g.ScalarAsInputTextId == 0, then on subsequent frames it becomes == id
    SetActiveID(g.ScalarAsInputTextId, window);
    SetHoveredID(0);
    FocusableItemUnregister(window);

    char buf[32];
    DataTypeFormatString(data_type, data_ptr, decimal_precision, buf, IM_ARRAYSIZE(buf));
    bool text_value_changed = InputTextEx(label, buf, IM_ARRAYSIZE(buf), aabb.GetSize(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll);
    if (g.ScalarAsInputTextId == 0)     // First frame we started displaying the InputText widget
    {
        IM_ASSERT(g.ActiveId == id);    // InputText ID expected to match the Slider ID (else we'd need to store them both, which is also possible)
        g.ScalarAsInputTextId = g.ActiveId;
        SetHoveredID(id);
    }
    if (text_value_changed)
        return DataTypeApplyOpFromText(buf, GImGui->InputTextState.InitialText.begin(), data_type, data_ptr, NULL);
    return false;
}

// Parse display precision back from the display format string
int ImGui::ParseFormatPrecision(const char* fmt, int default_precision)
{
    int precision = default_precision;
    while ((fmt = strchr(fmt, '%')) != NULL)
    {
        fmt++;
        if (fmt[0] == '%') { fmt++; continue; } // Ignore "%%"
        while (*fmt >= '0' && *fmt <= '9')
            fmt++;
        if (*fmt == '.')
        {
            fmt = ImAtoi(fmt + 1, &precision);
            if (precision < 0 || precision > 10)
                precision = default_precision;
        }
        if (*fmt == 'e' || *fmt == 'E') // Maximum precision with scientific notation
            precision = -1;
        break;
    }
    return precision;
}

static float GetMinimumStepAtDecimalPrecision(int decimal_precision)
{
    static const float min_steps[10] = { 1.0f, 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f, 0.0000001f, 0.00000001f, 0.000000001f };
    return (decimal_precision >= 0 && decimal_precision < 10) ? min_steps[decimal_precision] : powf(10.0f, (float)-decimal_precision);
}

float ImGui::RoundScalar(float value, int decimal_precision)
{
    // Round past decimal precision
    // So when our value is 1.99999 with a precision of 0.001 we'll end up rounding to 2.0
    // FIXME: Investigate better rounding methods
    if (decimal_precision < 0)
        return value;
    const float min_step = GetMinimumStepAtDecimalPrecision(decimal_precision);
    bool negative = value < 0.0f;
    value = fabsf(value);
    float remainder = fmodf(value, min_step);
    if (remainder <= min_step*0.5f)
        value -= remainder;
    else
        value += (min_step - remainder);
    return negative ? -value : value;
}

static inline float SliderBehaviorCalcRatioFromValue(float v, float v_min, float v_max, float power, float linear_zero_pos)
{
    if (v_min == v_max)
        return 0.0f;

    const bool is_non_linear = (power < 1.0f-0.00001f) || (power > 1.0f+0.00001f);
    const float v_clamped = (v_min < v_max) ? ImClamp(v, v_min, v_max) : ImClamp(v, v_max, v_min);
    if (is_non_linear)
    {
        if (v_clamped < 0.0f)
        {
            const float f = 1.0f - (v_clamped - v_min) / (ImMin(0.0f,v_max) - v_min);
            return (1.0f - powf(f, 1.0f/power)) * linear_zero_pos;
        }
        else
        {
            const float f = (v_clamped - ImMax(0.0f,v_min)) / (v_max - ImMax(0.0f,v_min));
            return linear_zero_pos + powf(f, 1.0f/power) * (1.0f - linear_zero_pos);
        }
    }

    // Linear slider
    return (v_clamped - v_min) / (v_max - v_min);
}
bool ImGui::gaybehavior(const ImRect& frame_bb, ImGuiID id, const char* display_format, float* v, float v_min, float v_max, float power, int decimal_precision, ImGuiSliderFlags flags) {
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();
	const ImGuiStyle& style = g.Style;

	// Draw frame

	window->DrawList->AddRectFilled(frame_bb.Min - ImVec2(0, 3) + ImVec2(0, 2), frame_bb.Max - ImVec2(0, 5) + ImVec2(0, 2), GetColorU32(ImGuiCol_Border), style.FrameRounding);
	window->DrawList->AddRectFilled(frame_bb.Min + ImVec2(1, -2) + ImVec2(0, 2), frame_bb.Max - ImVec2(1, 6) + ImVec2(0, 2), GetColorU32(ImGuiCol_FrameBg), style.FrameRounding);

	const bool is_non_linear = (power < 1.0f - 0.00001f) || (power > 1.0f + 0.00001f);
	const bool is_horizontal = (flags & ImGuiSliderFlags_Vertical) == 0;

	const float grab_padding = 2.0f;
	const float slider_sz = is_horizontal ? (frame_bb.GetWidth() - grab_padding * 2.0f) : (frame_bb.GetHeight() - grab_padding * 2.0f);
	float grab_sz;
	if (decimal_precision > 0)
		grab_sz = ImMin(style.GrabMinSize, slider_sz);
	else
		grab_sz = ImMin(ImMax(1.0f * (slider_sz / ((v_min < v_max ? v_max - v_min : v_min - v_max) + 1.0f)), style.GrabMinSize), slider_sz);  // Integer sliders, if possible have the grab size represent 1 unit
	const float slider_usable_sz = slider_sz - grab_sz;
	const float slider_usable_pos_min = (is_horizontal ? frame_bb.Min.x : frame_bb.Min.y) + grab_padding + grab_sz * 0.5f;
	const float slider_usable_pos_max = (is_horizontal ? frame_bb.Max.x : frame_bb.Max.y) - grab_padding - grab_sz * 0.5f;

	// For logarithmic sliders that cross over sign boundary we want the exponential increase to be symmetric around 0.0f
	float linear_zero_pos = 0.0f;   // 0.0->1.0f
	if (v_min * v_max < 0.0f) {
		// Different sign
		const float linear_dist_min_to_0 = powf(fabsf(0.0f - v_min), 1.0f / power);
		const float linear_dist_max_to_0 = powf(fabsf(v_max - 0.0f), 1.0f / power);
		linear_zero_pos = linear_dist_min_to_0 / (linear_dist_min_to_0 + linear_dist_max_to_0);
	}
	else {
		// Same sign
		linear_zero_pos = v_min < 0.0f ? 1.0f : 0.0f;
	}

	// Process clicking on the slider
	bool value_changed = false;
	if (g.ActiveId == id) {
		bool set_new_value = false;
		float clicked_t = 0.0f;
		if (g.IO.MouseDown[0]) {
			const float mouse_abs_pos = is_horizontal ? g.IO.MousePos.x : g.IO.MousePos.y;
			clicked_t = (slider_usable_sz > 0.0f) ? ImClamp((mouse_abs_pos - slider_usable_pos_min) / slider_usable_sz, 0.0f, 1.0f) : 0.0f;
			if (!is_horizontal)
				clicked_t = 1.0f - clicked_t;
			set_new_value = true;
		}
		else {
			ClearActiveID();
		}

		if (set_new_value) {
			float new_value;
			if (is_non_linear) {
				// Account for logarithmic scale on both sides of the zero
				if (clicked_t < linear_zero_pos) {
					// Negative: rescale to the negative range before powering
					float a = 1.0f - (clicked_t / linear_zero_pos);
					a = powf(a, power);
					new_value = ImLerp(ImMin(v_max, 0.0f), v_min, a);
				}
				else {
					// Positive: rescale to the positive range before powering
					float a;
					if (fabsf(linear_zero_pos - 1.0f) > 1.e-6f)
						a = (clicked_t - linear_zero_pos) / (1.0f - linear_zero_pos);
					else
						a = clicked_t;
					a = powf(a, power);
					new_value = ImLerp(ImMax(v_min, 0.0f), v_max, a);
				}
			}
			else {
				// Linear slider
				new_value = ImLerp(v_min, v_max, clicked_t);
			}

			// Round past decimal precision
			new_value = RoundScalar(new_value, decimal_precision);
			if (*v != new_value) {
				*v = new_value;
				value_changed = true;
			}
		}
	}

	// Draw
	float grab_t = SliderBehaviorCalcRatioFromValue(*v, v_min, v_max, power, linear_zero_pos);
	if (!is_horizontal)
		grab_t = 1.0f - grab_t;
	const float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
	ImRect grab_bb;
	if (is_horizontal)
		grab_bb = ImRect(ImVec2(grab_pos - grab_sz * 0.5f, frame_bb.Min.y), ImVec2(grab_pos + grab_sz * 0.5f, frame_bb.Max.y));
	else
		grab_bb = ImRect(ImVec2(frame_bb.Min.x + grab_padding, grab_pos - grab_sz * 0.5f), ImVec2(frame_bb.Max.x - grab_padding, grab_pos + grab_sz * 0.5f));

	window->DrawList->AddRectFilled(frame_bb.Min + ImVec2(1, -2) + ImVec2(0, 2), grab_bb.Max - ImVec2(-1, 6) + ImVec2(0, 2), GetColorU32(ImGuiCol_PlotLines)); // Main gradient.
	window->DrawList->AddRectFilledMultiColor(frame_bb.Min + ImVec2(1, -2) + ImVec2(0, 2), grab_bb.Max - ImVec2(-1, 6) + ImVec2(0, 2), GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.05f)), GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.05f)), GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.38f)), GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.38f))); // Main gradient.

	return value_changed;
}

bool ImGui::SliderBehavior(const ImRect& frame_bb, ImGuiID id, float* v, float v_min, float v_max, float power, int decimal_precision, ImGuiSliderFlags flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();
	const ImGuiStyle& style = g.Style;

	const_cast<float&>(frame_bb.Max.y) -= 2;
	const_cast<float&>(frame_bb.Min.y) += 2;

	// Draw frame
	RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.GrabRounding);

	const bool is_non_linear = (power < 1.0f - 0.00001f) || (power > 1.0f + 0.00001f);
	const bool is_horizontal = (flags & ImGuiSliderFlags_Vertical) == 0;

	const float grab_padding = 2.0f;
	const float slider_sz = is_horizontal ? (frame_bb.GetWidth() - grab_padding * 2.0f) : (frame_bb.GetHeight() - grab_padding * 2.0f);
	float grab_sz;
	if (decimal_precision != 0)
		grab_sz = ImMin(style.GrabMinSize, slider_sz);
	else
		grab_sz = ImMin(ImMax(1.0f * (slider_sz / ((v_min < v_max ? v_max - v_min : v_min - v_max) + 1.0f)), style.GrabMinSize), slider_sz);  // Integer sliders, if possible have the grab size represent 1 unit
	const float slider_usable_sz = slider_sz - grab_sz;
	const float slider_usable_pos_min = (is_horizontal ? frame_bb.Min.x : frame_bb.Min.y) + grab_padding + grab_sz*0.5f;
	const float slider_usable_pos_max = (is_horizontal ? frame_bb.Max.x : frame_bb.Max.y) - grab_padding - grab_sz*0.5f;

	// For logarithmic sliders that cross over sign boundary we want the exponential increase to be symmetric around 0.0f
	float linear_zero_pos = 0.0f;   // 0.0->1.0f
	if (v_min * v_max < 0.0f)
	{
		// Different sign
		const float linear_dist_min_to_0 = powf(fabsf(0.0f - v_min), 1.0f / power);
		const float linear_dist_max_to_0 = powf(fabsf(v_max - 0.0f), 1.0f / power);
		linear_zero_pos = linear_dist_min_to_0 / (linear_dist_min_to_0 + linear_dist_max_to_0);
	}
	else
	{
		// Same sign
		linear_zero_pos = v_min < 0.0f ? 1.0f : 0.0f;
	}

	// Process clicking on the slider
	bool value_changed = false;
	if (g.ActiveId == id)
	{
		bool set_new_value = false;
		float clicked_t = 0.0f;
		if (g.IO.MouseDown[0])
		{
			const float mouse_abs_pos = is_horizontal ? g.IO.MousePos.x : g.IO.MousePos.y;
			clicked_t = (slider_usable_sz > 0.0f) ? ImClamp((mouse_abs_pos - slider_usable_pos_min) / slider_usable_sz, 0.0f, 1.0f) : 0.0f;
			if (!is_horizontal)
				clicked_t = 1.0f - clicked_t;
			set_new_value = true;
		}
		else
		{
			ClearActiveID();
		}

		if (set_new_value)
		{
			float new_value;
			if (is_non_linear)
			{
				// Account for logarithmic scale on both sides of the zero
				if (clicked_t < linear_zero_pos)
				{
					// Negative: rescale to the negative range before powering
					float a = 1.0f - (clicked_t / linear_zero_pos);
					a = powf(a, power);
					new_value = ImLerp(ImMin(v_max, 0.0f), v_min, a);
				}
				else
				{
					// Positive: rescale to the positive range before powering
					float a;
					if (fabsf(linear_zero_pos - 1.0f) > 1.e-6f)
						a = (clicked_t - linear_zero_pos) / (1.0f - linear_zero_pos);
					else
						a = clicked_t;
					a = powf(a, power);
					new_value = ImLerp(ImMax(v_min, 0.0f), v_max, a);
				}
			}
			else
			{
				// Linear slider
				new_value = ImLerp(v_min, v_max, clicked_t);
			}

			// Round past decimal precision
			new_value = RoundScalar(new_value, decimal_precision);
			if (*v != new_value)
			{
				*v = new_value;
				value_changed = true;
			}
		}
	}

	// Draw
	float grab_t = SliderBehaviorCalcRatioFromValue(*v, v_min, v_max, power, linear_zero_pos);
	if (!is_horizontal)
		grab_t = 1.0f - grab_t;
	const float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
	ImRect grab_bb;
	if (is_horizontal)
		grab_bb = ImRect(ImVec2(grab_pos - grab_sz*0.5f, frame_bb.Min.y + grab_padding), ImVec2(grab_pos + grab_sz*0.5f, frame_bb.Max.y - grab_padding));
	else
		grab_bb = ImRect(ImVec2(frame_bb.Min.x + grab_padding, grab_pos - grab_sz*0.5f), ImVec2(frame_bb.Max.x - grab_padding, grab_pos + grab_sz*0.5f));

	// lazy, just using clip :^)
	PushClipRect(frame_bb.Min + ImVec2(grab_padding, grab_padding), ImVec2(grab_bb.Min.x, grab_bb.Max.y), false);
	ImRect bb_slider(ImVec2(grab_bb.Min.x - frame_bb.GetWidth() + frame_bb.GetHeight() * 0.5f, frame_bb.Min.y), ImVec2(grab_bb.Max.x, frame_bb.Max.y));
	window->DrawList->AddRectFilled(bb_slider.Min, bb_slider.Max, GetColorU32(ImVec4(0.1f, 0.45, 1.f, 0.35)), style.GrabRounding);
	PopClipRect();

	window->DrawList->AddRect(grab_bb.Min - ImVec2(2, 3), grab_bb.Max + ImVec2(2, 3), GetColorU32(ImVec4(0, 0, 0, 1)), style.GrabRounding);
	window->DrawList->AddRectFilled(grab_bb.Min - ImVec2(1, 2), grab_bb.Max + ImVec2(1, 2), GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);

	return value_changed;
}

// Use power!=1.0 for logarithmic sliders.
// Adjust display_format to decorate the value with a prefix or a suffix.
//   "%.3f"         1.234
//   "%5.2f secs"   01.23 secs
//   "Gold: %.0f"   Gold: 1
bool ImGui::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* display_format, float power)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	SetCursorPosY(GetCursorPosY() + 15);

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const float w = 177;

	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImRect frame_bb(window->DC.CursorPos - ImVec2(-17, 2), window->DC.CursorPos + ImVec2(w + 17, label_size.y + style.FramePadding.y * 0.f - 5));
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 2.0f));

	// NB- we don't call ItemSize() yet because we may turn into a text edit box below
	if (!ItemAdd(total_bb, id)) {
		ItemSize(total_bb, style.FramePadding.y);
		return false;
	}

	const bool hovered = ItemHoverable(frame_bb, id);
	if (hovered)
		SetHoveredID(id);

	if (!display_format)
		display_format = "%.3f";
	int decimal_precision = ParseFormatPrecision(display_format, 3);

	// Tabbing or CTRL-clicking on Slider turns it into an input box
	bool start_text_input = false;
	const bool tab_focus_requested = FocusableItemRegister(window, g.ActiveId == id);
	if (tab_focus_requested || (hovered && g.IO.MouseClicked[0])) {
		SetActiveID(id, window);
		FocusWindow(window);

		if (tab_focus_requested || g.IO.KeyCtrl) {
			start_text_input = true;
			g.ScalarAsInputTextId = 0;
		}
	}
	if (start_text_input || (g.ActiveId == id && g.ScalarAsInputTextId == id))
		return InputScalarAsWidgetReplacement(frame_bb, label, ImGuiDataType_Float, v, id, decimal_precision);

	ItemSize(total_bb, style.FramePadding.y);

	// Actual slider behavior + render grab
	const bool value_changed = gaybehavior(frame_bb, id, display_format, v, v_min, v_max, power, decimal_precision);

	// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
	char value_buf[64];
	const char* value_buf_end = value_buf + ImFormatString(value_buf, IM_ARRAYSIZE(value_buf), display_format, *v);
	RenderText(ImVec2(frame_bb.Max.x - ImGui::CalcTextSize(value_buf).x, frame_bb.Min.y - 17), value_buf, value_buf_end);
	// RenderTextClipped( frame_bb.Min - ImVec2( 0, 17 ), frame_bb.Max - ImVec2( 0, 14 ), value_buf, value_buf_end, NULL, ImVec2( 1.f, 1.f ) );

	if (label_size.x > 0.0f)
		RenderText(ImVec2(frame_bb.Min.x + 1, frame_bb.Min.y - 17), label);

	return value_changed;
}

bool ImGui::VSliderFloat(const char* label, const ImVec2& size, float* v, float v_min, float v_max, const char* display_format, float power)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
    const ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(frame_bb, id))
        return false;
    const bool hovered = ItemHoverable(frame_bb, id);

    if (!display_format)
        display_format = "%.3f";
    int decimal_precision = ParseFormatPrecision(display_format, 3);

    if (hovered && g.IO.MouseClicked[0])
    {
        SetActiveID(id, window);
        FocusWindow(window);
    }

    // Actual slider behavior + render grab
    bool value_changed = SliderBehavior(frame_bb, id, v, v_min, v_max, power, decimal_precision, ImGuiSliderFlags_Vertical);

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    // For the vertical slider we allow centered text to overlap the frame padding
    char value_buf[64];
    char* value_buf_end = value_buf + ImFormatString(value_buf, IM_ARRAYSIZE(value_buf), display_format, *v);
    RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f,0.0f));
    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    return value_changed;
}

bool ImGui::SliderAngle(const char* label, float* v_rad, float v_degrees_min, float v_degrees_max)
{
    float v_deg = (*v_rad) * 360.0f / (2*IM_PI);
    bool value_changed = SliderFloat(label, &v_deg, v_degrees_min, v_degrees_max, "%.0f deg", 1.0f);
    *v_rad = v_deg * (2*IM_PI) / 360.0f;
    return value_changed;
}

bool ImGui::SliderInt(const char* label, int* v, int v_min, int v_max, const char* display_format)
{
    if (!display_format)
        display_format = "%.0f";
    float v_f = (float)*v;
    bool value_changed = SliderFloat(label, &v_f, (float)v_min, (float)v_max, display_format, 1.0f);
    *v = (int)v_f;
    return value_changed;
}

bool ImGui::VSliderInt(const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* display_format)
{
    if (!display_format)
        display_format = "%.0f";
    float v_f = (float)*v;
    bool value_changed = VSliderFloat(label, size, &v_f, (float)v_min, (float)v_max, display_format, 1.0f);
    *v = (int)v_f;
    return value_changed;
}

// Add multiple sliders on 1 line for compact edition of multiple components
bool ImGui::SliderFloatN(const char* label, float* v, int components, float v_min, float v_max, const char* display_format, float power)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    BeginGroup();
    PushID(label);
    PushMultiItemsWidths(components);
    for (int i = 0; i < components; i++)
    {
        PushID(i);
        value_changed |= SliderFloat("##v", &v[i], v_min, v_max, display_format, power);
        SameLine(0, g.Style.ItemInnerSpacing.x);
        PopID();
        PopItemWidth();
    }
    PopID();

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();

    return value_changed;
}

bool ImGui::SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* display_format, float power)
{
    return SliderFloatN(label, v, 2, v_min, v_max, display_format, power);
}

bool ImGui::SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* display_format, float power)
{
    return SliderFloatN(label, v, 3, v_min, v_max, display_format, power);
}

bool ImGui::SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* display_format, float power)
{
    return SliderFloatN(label, v, 4, v_min, v_max, display_format, power);
}

bool ImGui::SliderIntN(const char* label, int* v, int components, int v_min, int v_max, const char* display_format)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    BeginGroup();
    PushID(label);
    PushMultiItemsWidths(components);
    for (int i = 0; i < components; i++)
    {
        PushID(i);
        value_changed |= SliderInt("##v", &v[i], v_min, v_max, display_format);
        SameLine(0, g.Style.ItemInnerSpacing.x);
        PopID();
        PopItemWidth();
    }
    PopID();

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();

    return value_changed;
}

bool ImGui::SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* display_format)
{
    return SliderIntN(label, v, 2, v_min, v_max, display_format);
}

bool ImGui::SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* display_format)
{
    return SliderIntN(label, v, 3, v_min, v_max, display_format);
}

bool ImGui::SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* display_format)
{
    return SliderIntN(label, v, 4, v_min, v_max, display_format);
}

bool ImGui::DragBehavior(const ImRect& frame_bb, ImGuiID id, float* v, float v_speed, float v_min, float v_max, int decimal_precision, float power)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Draw frame
    const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, style.FrameRounding);

    bool value_changed = false;

    // Process clicking on the drag
    if (g.ActiveId == id)
    {
        if (g.IO.MouseDown[0])
        {
            if (g.ActiveIdIsJustActivated)
            {
                // Lock current value on click
                g.DragCurrentValue = *v;
                g.DragLastMouseDelta = ImVec2(0.f, 0.f);
            }

            if (v_speed == 0.0f && (v_max - v_min) != 0.0f && (v_max - v_min) < FLT_MAX)
                v_speed = (v_max - v_min) * g.DragSpeedDefaultRatio;

            float v_cur = g.DragCurrentValue;
            const ImVec2 mouse_drag_delta = GetMouseDragDelta(0, 1.0f);
            float adjust_delta = 0.0f;
            //if (g.ActiveIdSource == ImGuiInputSource_Mouse)
            {
                adjust_delta = mouse_drag_delta.x - g.DragLastMouseDelta.x;
                if (g.IO.KeyShift && g.DragSpeedScaleFast >= 0.0f)
                    adjust_delta *= g.DragSpeedScaleFast;
                if (g.IO.KeyAlt && g.DragSpeedScaleSlow >= 0.0f)
                    adjust_delta *= g.DragSpeedScaleSlow;
            }
            adjust_delta *= v_speed;
            g.DragLastMouseDelta.x = mouse_drag_delta.x;

            if (fabsf(adjust_delta) > 0.0f)
            {
                if (fabsf(power - 1.0f) > 0.001f)
                {
                    // Logarithmic curve on both side of 0.0
                    float v0_abs = v_cur >= 0.0f ? v_cur : -v_cur;
                    float v0_sign = v_cur >= 0.0f ? 1.0f : -1.0f;
                    float v1 = powf(v0_abs, 1.0f / power) + (adjust_delta * v0_sign);
                    float v1_abs = v1 >= 0.0f ? v1 : -v1;
                    float v1_sign = v1 >= 0.0f ? 1.0f : -1.0f;          // Crossed sign line
                    v_cur = powf(v1_abs, power) * v0_sign * v1_sign;    // Reapply sign
                }
                else
                {
                    v_cur += adjust_delta;
                }

                // Clamp
                if (v_min < v_max)
                    v_cur = ImClamp(v_cur, v_min, v_max);
                g.DragCurrentValue = v_cur;
            }

            // Round to user desired precision, then apply
            v_cur = RoundScalar(v_cur, decimal_precision);
            if (*v != v_cur)
            {
                *v = v_cur;
                value_changed = true;
            }
        }
        else
        {
            ClearActiveID();
        }
    }

    return value_changed;
}

bool ImGui::DragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* display_format, float power)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const float w = CalcItemWidth();

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y*2.0f));
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

    // NB- we don't call ItemSize() yet because we may turn into a text edit box below
    if (!ItemAdd(total_bb, id))
    {
        ItemSize(total_bb, style.FramePadding.y);
        return false;
    }
    const bool hovered = ItemHoverable(frame_bb, id);

    if (!display_format)
        display_format = "%.3f";
    int decimal_precision = ParseFormatPrecision(display_format, 3);

    // Tabbing or CTRL-clicking on Drag turns it into an input box
    bool start_text_input = false;
    const bool tab_focus_requested = FocusableItemRegister(window, id);
    if (tab_focus_requested || (hovered && (g.IO.MouseClicked[0] || g.IO.MouseDoubleClicked[0])))
    {
        SetActiveID(id, window);
        FocusWindow(window);
        if (tab_focus_requested || g.IO.KeyCtrl || g.IO.MouseDoubleClicked[0])
        {
            start_text_input = true;
            g.ScalarAsInputTextId = 0;
        }
    }
    if (start_text_input || (g.ActiveId == id && g.ScalarAsInputTextId == id))
        return InputScalarAsWidgetReplacement(frame_bb, label, ImGuiDataType_Float, v, id, decimal_precision);

    // Actual drag behavior
    ItemSize(total_bb, style.FramePadding.y);
    const bool value_changed = DragBehavior(frame_bb, id, v, v_speed, v_min, v_max, decimal_precision, power);

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    char value_buf[64];
    const char* value_buf_end = value_buf + ImFormatString(value_buf, IM_ARRAYSIZE(value_buf), display_format, *v);
    RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f,0.5f));

    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);

    return value_changed;
}

bool ImGui::DragFloatN(const char* label, float* v, int components, float v_speed, float v_min, float v_max, const char* display_format, float power)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    BeginGroup();
    PushID(label);
    PushMultiItemsWidths(components);
    for (int i = 0; i < components; i++)
    {
        PushID(i);
        value_changed |= DragFloat("##v", &v[i], v_speed, v_min, v_max, display_format, power);
        SameLine(0, g.Style.ItemInnerSpacing.x);
        PopID();
        PopItemWidth();
    }
    PopID();

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();

    return value_changed;
}

bool ImGui::DragFloat2(const char* label, float v[2], float v_speed, float v_min, float v_max, const char* display_format, float power)
{
    return DragFloatN(label, v, 2, v_speed, v_min, v_max, display_format, power);
}

bool ImGui::DragFloat3(const char* label, float v[3], float v_speed, float v_min, float v_max, const char* display_format, float power)
{
    return DragFloatN(label, v, 3, v_speed, v_min, v_max, display_format, power);
}

bool ImGui::DragFloat4(const char* label, float v[4], float v_speed, float v_min, float v_max, const char* display_format, float power)
{
    return DragFloatN(label, v, 4, v_speed, v_min, v_max, display_format, power);
}

bool ImGui::DragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed, float v_min, float v_max, const char* display_format, const char* display_format_max, float power)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    PushID(label);
    BeginGroup();
    PushMultiItemsWidths(2);

    bool value_changed = DragFloat("##min", v_current_min, v_speed, (v_min >= v_max) ? -FLT_MAX : v_min, (v_min >= v_max) ? *v_current_max : ImMin(v_max, *v_current_max), display_format, power);
    PopItemWidth();
    SameLine(0, g.Style.ItemInnerSpacing.x);
    value_changed |= DragFloat("##max", v_current_max, v_speed, (v_min >= v_max) ? *v_current_min : ImMax(v_min, *v_current_min), (v_min >= v_max) ? FLT_MAX : v_max, display_format_max ? display_format_max : display_format, power);
    PopItemWidth();
    SameLine(0, g.Style.ItemInnerSpacing.x);

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();
    PopID();

    return value_changed;
}

// NB: v_speed is float to allow adjusting the drag speed with more precision
bool ImGui::DragInt(const char* label, int* v, float v_speed, int v_min, int v_max, const char* display_format)
{
    if (!display_format)
        display_format = "%.0f";
    float v_f = (float)*v;
    bool value_changed = DragFloat(label, &v_f, v_speed, (float)v_min, (float)v_max, display_format);
    *v = (int)v_f;
    return value_changed;
}

bool ImGui::DragIntN(const char* label, int* v, int components, float v_speed, int v_min, int v_max, const char* display_format)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    BeginGroup();
    PushID(label);
    PushMultiItemsWidths(components);
    for (int i = 0; i < components; i++)
    {
        PushID(i);
        value_changed |= DragInt("##v", &v[i], v_speed, v_min, v_max, display_format);
        SameLine(0, g.Style.ItemInnerSpacing.x);
        PopID();
        PopItemWidth();
    }
    PopID();

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();

    return value_changed;
}

bool ImGui::DragInt2(const char* label, int v[2], float v_speed, int v_min, int v_max, const char* display_format)
{
    return DragIntN(label, v, 2, v_speed, v_min, v_max, display_format);
}

bool ImGui::DragInt3(const char* label, int v[3], float v_speed, int v_min, int v_max, const char* display_format)
{
    return DragIntN(label, v, 3, v_speed, v_min, v_max, display_format);
}

bool ImGui::DragInt4(const char* label, int v[4], float v_speed, int v_min, int v_max, const char* display_format)
{
    return DragIntN(label, v, 4, v_speed, v_min, v_max, display_format);
}

bool ImGui::DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed, int v_min, int v_max, const char* display_format, const char* display_format_max)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    PushID(label);
    BeginGroup();
    PushMultiItemsWidths(2);

    bool value_changed = DragInt("##min", v_current_min, v_speed, (v_min >= v_max) ? INT_MIN : v_min, (v_min >= v_max) ? *v_current_max : ImMin(v_max, *v_current_max), display_format);
    PopItemWidth();
    SameLine(0, g.Style.ItemInnerSpacing.x);
    value_changed |= DragInt("##max", v_current_max, v_speed, (v_min >= v_max) ? *v_current_min : ImMax(v_min, *v_current_min), (v_min >= v_max) ? INT_MAX : v_max, display_format_max ? display_format_max : display_format);
    PopItemWidth();
    SameLine(0, g.Style.ItemInnerSpacing.x);

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();
    PopID();

    return value_changed;
}

void ImGui::PlotEx(ImGuiPlotType plot_type, const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    if (graph_size.x == 0.0f)
        graph_size.x = CalcItemWidth();
    if (graph_size.y == 0.0f)
        graph_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(graph_size.x, graph_size.y));
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, 0))
        return;
    const bool hovered = ItemHoverable(inner_bb, 0);

    // Determine scale from values if not specified
    if (scale_min == FLT_MAX || scale_max == FLT_MAX)
    {
        float v_min = FLT_MAX;
        float v_max = -FLT_MAX;
        for (int i = 0; i < values_count; i++)
        {
            const float v = values_getter(data, i);
            v_min = ImMin(v_min, v);
            v_max = ImMax(v_max, v);
        }
        if (scale_min == FLT_MAX)
            scale_min = v_min;
        if (scale_max == FLT_MAX)
            scale_max = v_max;
    }

    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    if (values_count > 0)
    {
        int res_w = ImMin((int)graph_size.x, values_count) + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);
        int item_count = values_count + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);

        // Tooltip on hover
        int v_hovered = -1;
        if (hovered)
        {
            const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
            const int v_idx = (int)(t * item_count);
            IM_ASSERT(v_idx >= 0 && v_idx < values_count);

            const float v0 = values_getter(data, (v_idx + values_offset) % values_count);
            const float v1 = values_getter(data, (v_idx + 1 + values_offset) % values_count);
            if (plot_type == ImGuiPlotType_Lines)
                SetTooltip("%d: %8.4g\n%d: %8.4g", v_idx, v0, v_idx+1, v1);
            else if (plot_type == ImGuiPlotType_Histogram)
                SetTooltip("%d: %8.4g", v_idx, v0);
            v_hovered = v_idx;
        }

        const float t_step = 1.0f / (float)res_w;

        float v0 = values_getter(data, (0 + values_offset) % values_count);
        float t0 = 0.0f;
        ImVec2 tp0 = ImVec2( t0, 1.0f - ImSaturate((v0 - scale_min) / (scale_max - scale_min)) );                       // Point in the normalized space of our target rectangle
        float histogram_zero_line_t = (scale_min * scale_max < 0.0f) ? (-scale_min / (scale_max - scale_min)) : (scale_min < 0.0f ? 0.0f : 1.0f);   // Where does the zero line stands

        const ImU32 col_base = GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLines : ImGuiCol_PlotHistogram);
        const ImU32 col_hovered = GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLinesHovered : ImGuiCol_PlotHistogramHovered);

        for (int n = 0; n < res_w; n++)
        {
            const float t1 = t0 + t_step;
            const int v1_idx = (int)(t0 * item_count + 0.5f);
            IM_ASSERT(v1_idx >= 0 && v1_idx < values_count);
            const float v1 = values_getter(data, (v1_idx + values_offset + 1) % values_count);
            const ImVec2 tp1 = ImVec2( t1, 1.0f - ImSaturate((v1 - scale_min) / (scale_max - scale_min)) );

            // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
            ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
            ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, (plot_type == ImGuiPlotType_Lines) ? tp1 : ImVec2(tp1.x, histogram_zero_line_t));
            if (plot_type == ImGuiPlotType_Lines)
            {
                window->DrawList->AddLine(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);
            }
            else if (plot_type == ImGuiPlotType_Histogram)
            {
                if (pos1.x >= pos0.x + 2.0f)
                    pos1.x -= 1.0f;
                window->DrawList->AddRectFilled(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);
            }

            t0 = t1;
            tp0 = tp1;
        }
    }

    // Text overlay
    if (overlay_text)
        RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, NULL, NULL, ImVec2(0.5f,0.0f));

    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);
}

struct ImGuiPlotArrayGetterData
{
    const float* Values;
    int Stride;

    ImGuiPlotArrayGetterData(const float* values, int stride) { Values = values; Stride = stride; }
};

static float Plot_ArrayGetter(void* data, int idx)
{
    ImGuiPlotArrayGetterData* plot_data = (ImGuiPlotArrayGetterData*)data;
    const float v = *(float*)(void*)((unsigned char*)plot_data->Values + (size_t)idx * plot_data->Stride);
    return v;
}

void ImGui::PlotLines(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride)
{
    ImGuiPlotArrayGetterData data(values, stride);
    PlotEx(ImGuiPlotType_Lines, label, &Plot_ArrayGetter, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void ImGui::PlotLines(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size)
{
    PlotEx(ImGuiPlotType_Lines, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void ImGui::PlotHistogram(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride)
{
    ImGuiPlotArrayGetterData data(values, stride);
    PlotEx(ImGuiPlotType_Histogram, label, &Plot_ArrayGetter, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void ImGui::PlotHistogram(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size)
{
    PlotEx(ImGuiPlotType_Histogram, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

// size_arg (for each axis) < 0.0f: align to end, 0.0f: auto, > 0.0f: specified size
void ImGui::ProgressBar(float fraction, const ImVec2& size_arg, const char* overlay)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = window->DC.CursorPos;
    ImRect bb(pos, pos + CalcItemSize(size_arg, CalcItemWidth(), g.FontSize + style.FramePadding.y*2.0f));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, 0))
        return;

    // Render
    fraction = ImSaturate(fraction);
    RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
    bb.Expand(ImVec2(-window->BorderSize, -window->BorderSize));
    const ImVec2 fill_br = ImVec2(ImLerp(bb.Min.x, bb.Max.x, fraction), bb.Max.y);
    RenderRectFilledRangeH(window->DrawList, bb, GetColorU32(ImGuiCol_PlotHistogram), 0.0f, fraction, style.FrameRounding);

    // Default displaying the fraction as percentage string, but user can override it
    char overlay_buf[32];
    if (!overlay)
    {
        ImFormatString(overlay_buf, IM_ARRAYSIZE(overlay_buf), "%.0f%%", fraction*100+0.01f);
        overlay = overlay_buf;
    }

    ImVec2 overlay_size = CalcTextSize(overlay, NULL);
    if (overlay_size.x > 0.0f)
        RenderTextClipped(ImVec2(ImClamp(fill_br.x + style.ItemSpacing.x, bb.Min.x, bb.Max.x - overlay_size.x - style.ItemInnerSpacing.x), bb.Min.y), bb.Max, overlay, NULL, &overlay_size, ImVec2(0.0f,0.5f), &bb);
}
bool ImGui::ImageButtonWithText(ImTextureID texId, const char* label, const ImVec2& imageSize, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col) {
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImVec2 size = imageSize;
	if (size.x <= 0 && size.y <= 0) { size.x = size.y = ImGui::GetTextLineHeightWithSpacing(); }
	else {
		if (size.x <= 0)          size.x = size.y;
		else if (size.y <= 0)     size.y = size.x;
		size *= window->FontWindowScale * ImGui::GetIO().FontGlobalScale;
	}

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	const ImGuiID id = window->GetID(label);
	const ImVec2 textSize = ImGui::CalcTextSize(label, NULL, true);
	const bool hasText = textSize.x > 0;

	const float innerSpacing = hasText ? ((frame_padding >= 0) ? (float)frame_padding : (style.ItemInnerSpacing.x)) : 0.f;
	const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
	const ImVec2 totalSizeWithoutPadding(size.x + innerSpacing + textSize.x, size.y > textSize.y ? size.y : textSize.y);
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSizeWithoutPadding + padding * 2);
	ImVec2 start(0, 0);
	start = window->DC.CursorPos + padding; if (size.y < textSize.y) start.y += (textSize.y - size.y) * .5f;
	const ImRect image_bb(start, start + size);
	start = window->DC.CursorPos + padding; start.x += size.x + innerSpacing; if (size.y > textSize.y) start.y += (size.y - textSize.y) * .5f;
	ItemSize(bb);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered = false, held = false;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);

	// Render
	const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
	if (bg_col.w > 0.0f)
		window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

	window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(tint_col));

	if (textSize.x > 0) ImGui::RenderText(start, label);
	return pressed;
}
bool ImGui::Checkbox(const char* label, bool* v)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = ImGuiStyle::ImGuiStyle();
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImVec2 pading = ImVec2(2, 2);
	const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.x * 6, label_size.y + style.FramePadding.y / 2));
	ItemSize(check_bb, style.FramePadding.y);
	ImRect total_bb = check_bb;
	if (label_size.x > 0)
		SameLine(0, style.ItemInnerSpacing.x);
	const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);
	if (label_size.x > 0)
	{
		ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
		total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
	}
	if (!ItemAdd(total_bb, id))
		return false;
	bool hovered, held;
	bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
	if (pressed)
		* v = !(*v);
	const float check_sz = ImMin(check_bb.GetWidth(), check_bb.GetHeight());
	const float check_sz2 = check_sz / 2;
	const float pad = ImMax(1.0f, (float)(int)(check_sz / 4.f));
	//window->DrawList->AddRectFilled(check_bb.Min+ImVec2(pad,pad), check_bb.Max-ImVec2(pad,pad), GetColorU32(ImGuiCol_CheckMark), style.FrameRounding);
	window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 6, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)), 12);
	window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 5, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)), 12);
	window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 4, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)), 12);
	window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 3, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)), 12);
	window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 2, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)), 12);
	window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 1, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)), 12);
	window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)), 12);
	window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 1, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)), 12);
	window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 2, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)), 12);
	window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 4, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)), 12);
	window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 5, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f)), 12);
	window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 6, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), 12);
	if (*v)//?? ?????? ???????
	{
		//window->DrawList->AddRectFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2, check_bb.Min.y), check_bb.Max, GetColorU32(ImVec4(0.34f, 1.0f, 0.54f, 1.0f)), 0);
		//window->DrawList->AddRectFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2, check_bb.Min.y), check_bb.Max, GetColorU32(ImVec4(0.34f, 1.0f, 0.54f, 1.0f)), 0);


		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 6, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 5, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 4, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 3, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 2, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 1, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 1, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 2, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 3, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 4, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 5, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 6, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(0.00f, 0.50f, 0.90f, 1.00f)), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 6, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), 12);
	}
	else
	{
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 6, check_bb.Min.y + 9), 7, GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), 12);
	}
	if (label_size.x > 0.0f)
		RenderText(text_bb.GetTL(), label);
	return pressed;
}

bool ImGui::CheckboxFlags(const char* label, unsigned int* flags, unsigned int flags_value)
{
    bool v = ((*flags & flags_value) == flags_value);
    bool pressed = Checkbox(label, &v);
    if (pressed)
    {
        if (v)
            *flags |= flags_value;
        else
            *flags &= ~flags_value;
    }

    return pressed;
}

bool ImGui::RadioButton(const char* label, bool active)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.y*2-1, label_size.y + style.FramePadding.y*2-1));
    ItemSize(check_bb, style.FramePadding.y);

    ImRect total_bb = check_bb;
    if (label_size.x > 0)
        SameLine(0, style.ItemInnerSpacing.x);
    const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);
    if (label_size.x > 0)
    {
        ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
        total_bb.Add(text_bb);
    }

    if (!ItemAdd(total_bb, id))
        return false;

    ImVec2 center = check_bb.GetCenter();
    center.x = (float)(int)center.x + 0.5f;
    center.y = (float)(int)center.y + 0.5f;
    const float radius = check_bb.GetHeight() * 0.5f;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

    window->DrawList->AddCircleFilled(center, radius, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), 16);
    if (active)
    {
        const float check_sz = ImMin(check_bb.GetWidth(), check_bb.GetHeight());
        const float pad = ImMax(1.0f, (float)(int)(check_sz / 6.0f));
        window->DrawList->AddCircleFilled(center, radius-pad, GetColorU32(ImGuiCol_CheckMark), 16);
    }

    if (window->Flags & ImGuiWindowFlags_ShowBorders)
    {
        window->DrawList->AddCircle(center+ImVec2(1,1), radius, GetColorU32(ImGuiCol_BorderShadow), 16);
        window->DrawList->AddCircle(center, radius, GetColorU32(ImGuiCol_Border), 16);
    }

    if (g.LogEnabled)
        LogRenderedText(&text_bb.Min, active ? "(x)" : "( )");
    if (label_size.x > 0.0f)
        RenderText(text_bb.Min, label);

    return pressed;
}

bool ImGui::RadioButton(const char* label, int* v, int v_button)
{
    const bool pressed = RadioButton(label, *v == v_button);
    if (pressed)
    {
        *v = v_button;
    }
    return pressed;
}

static int InputTextCalcTextLenAndLineCount(const char* text_begin, const char** out_text_end)
{
    int line_count = 0;
    const char* s = text_begin;
    while (char c = *s++) // We are only matching for \n so we can ignore UTF-8 decoding
        if (c == '\n')
            line_count++;
    s--;
    if (s[0] != '\n' && s[0] != '\r')
        line_count++;
    *out_text_end = s;
    return line_count;
}

static ImVec2 InputTextCalcTextSizeW(const ImWchar* text_begin, const ImWchar* text_end, const ImWchar** remaining, ImVec2* out_offset, bool stop_on_new_line)
{
    ImFont* font = GImGui->Font;
    const float line_height = GImGui->FontSize;
    const float scale = line_height / font->FontSize;

    ImVec2 text_size = ImVec2(0,0);
    float line_width = 0.0f;

    const ImWchar* s = text_begin;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)(*s++);
        if (c == '\n')
        {
            text_size.x = ImMax(text_size.x, line_width);
            text_size.y += line_height;
            line_width = 0.0f;
            if (stop_on_new_line)
                break;
            continue;
        }
        if (c == '\r')
            continue;

        const float char_width = font->GetCharAdvance((unsigned short)c) * scale;
        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (out_offset)
        *out_offset = ImVec2(line_width, text_size.y + line_height);  // offset allow for the possibility of sitting after a trailing \n

    if (line_width > 0 || text_size.y == 0.0f)                        // whereas size.y will ignore the trailing \n
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

// Wrapper for stb_textedit.h to edit text (our wrapper is for: statically sized buffer, single-line, wchar characters. InputText converts between UTF-8 and wchar)
namespace ImGuiStb
{

static int     STB_TEXTEDIT_STRINGLEN(const STB_TEXTEDIT_STRING* obj)                             { return obj->CurLenW; }
static ImWchar STB_TEXTEDIT_GETCHAR(const STB_TEXTEDIT_STRING* obj, int idx)                      { return obj->Text[idx]; }
static float   STB_TEXTEDIT_GETWIDTH(STB_TEXTEDIT_STRING* obj, int line_start_idx, int char_idx)  { ImWchar c = obj->Text[line_start_idx+char_idx]; if (c == '\n') return STB_TEXTEDIT_GETWIDTH_NEWLINE; return GImGui->Font->GetCharAdvance(c) * (GImGui->FontSize / GImGui->Font->FontSize); }
static int     STB_TEXTEDIT_KEYTOTEXT(int key)                                                    { return key >= 0x10000 ? 0 : key; }
static ImWchar STB_TEXTEDIT_NEWLINE = '\n';
static void    STB_TEXTEDIT_LAYOUTROW(StbTexteditRow* r, STB_TEXTEDIT_STRING* obj, int line_start_idx)
{
    const ImWchar* text = obj->Text.Data;
    const ImWchar* text_remaining = NULL;
    const ImVec2 size = InputTextCalcTextSizeW(text + line_start_idx, text + obj->CurLenW, &text_remaining, NULL, true);
    r->x0 = 0.0f;
    r->x1 = size.x;
    r->baseline_y_delta = size.y;
    r->ymin = 0.0f;
    r->ymax = size.y;
    r->num_chars = (int)(text_remaining - (text + line_start_idx));
}

static bool is_separator(unsigned int c)                                        { return ImCharIsSpace(c) || c==',' || c==';' || c=='(' || c==')' || c=='{' || c=='}' || c=='[' || c==']' || c=='|'; }
static int  is_word_boundary_from_right(STB_TEXTEDIT_STRING* obj, int idx)      { return idx > 0 ? (is_separator( obj->Text[idx-1] ) && !is_separator( obj->Text[idx] ) ) : 1; }
static int  STB_TEXTEDIT_MOVEWORDLEFT_IMPL(STB_TEXTEDIT_STRING* obj, int idx)   { idx--; while (idx >= 0 && !is_word_boundary_from_right(obj, idx)) idx--; return idx < 0 ? 0 : idx; }
#ifdef __APPLE__    // FIXME: Move setting to IO structure
static int  is_word_boundary_from_left(STB_TEXTEDIT_STRING* obj, int idx)       { return idx > 0 ? (!is_separator( obj->Text[idx-1] ) && is_separator( obj->Text[idx] ) ) : 1; }
static int  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(STB_TEXTEDIT_STRING* obj, int idx)  { idx++; int len = obj->CurLenW; while (idx < len && !is_word_boundary_from_left(obj, idx)) idx++; return idx > len ? len : idx; }
#else
static int  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(STB_TEXTEDIT_STRING* obj, int idx)  { idx++; int len = obj->CurLenW; while (idx < len && !is_word_boundary_from_right(obj, idx)) idx++; return idx > len ? len : idx; }
#endif
#define STB_TEXTEDIT_MOVEWORDLEFT   STB_TEXTEDIT_MOVEWORDLEFT_IMPL    // They need to be #define for stb_textedit.h
#define STB_TEXTEDIT_MOVEWORDRIGHT  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL

static void STB_TEXTEDIT_DELETECHARS(STB_TEXTEDIT_STRING* obj, int pos, int n)
{
    ImWchar* dst = obj->Text.Data + pos;

    // We maintain our buffer length in both UTF-8 and wchar formats
    obj->CurLenA -= ImTextCountUtf8BytesFromStr(dst, dst + n);
    obj->CurLenW -= n;

    // Offset remaining text
    const ImWchar* src = obj->Text.Data + pos + n;
    while (ImWchar c = *src++)
        *dst++ = c;
    *dst = '\0';
}

static bool STB_TEXTEDIT_INSERTCHARS(STB_TEXTEDIT_STRING* obj, int pos, const ImWchar* new_text, int new_text_len)
{
    const int text_len = obj->CurLenW;
    IM_ASSERT(pos <= text_len);
    if (new_text_len + text_len + 1 > obj->Text.Size)
        return false;

    const int new_text_len_utf8 = ImTextCountUtf8BytesFromStr(new_text, new_text + new_text_len);
    if (new_text_len_utf8 + obj->CurLenA + 1 > obj->BufSizeA)
        return false;

    ImWchar* text = obj->Text.Data;
    if (pos != text_len)
        memmove(text + pos + new_text_len, text + pos, (size_t)(text_len - pos) * sizeof(ImWchar));
    memcpy(text + pos, new_text, (size_t)new_text_len * sizeof(ImWchar));

    obj->CurLenW += new_text_len;
    obj->CurLenA += new_text_len_utf8;
    obj->Text[obj->CurLenW] = '\0';

    return true;
}

// We don't use an enum so we can build even with conflicting symbols (if another user of stb_textedit.h leak their STB_TEXTEDIT_K_* symbols)
#define STB_TEXTEDIT_K_LEFT         0x10000 // keyboard input to move cursor left
#define STB_TEXTEDIT_K_RIGHT        0x10001 // keyboard input to move cursor right
#define STB_TEXTEDIT_K_UP           0x10002 // keyboard input to move cursor up
#define STB_TEXTEDIT_K_DOWN         0x10003 // keyboard input to move cursor down
#define STB_TEXTEDIT_K_LINESTART    0x10004 // keyboard input to move cursor to start of line
#define STB_TEXTEDIT_K_LINEEND      0x10005 // keyboard input to move cursor to end of line
#define STB_TEXTEDIT_K_TEXTSTART    0x10006 // keyboard input to move cursor to start of text
#define STB_TEXTEDIT_K_TEXTEND      0x10007 // keyboard input to move cursor to end of text
#define STB_TEXTEDIT_K_DELETE       0x10008 // keyboard input to delete selection or character under cursor
#define STB_TEXTEDIT_K_BACKSPACE    0x10009 // keyboard input to delete selection or character left of cursor
#define STB_TEXTEDIT_K_UNDO         0x1000A // keyboard input to perform undo
#define STB_TEXTEDIT_K_REDO         0x1000B // keyboard input to perform redo
#define STB_TEXTEDIT_K_WORDLEFT     0x1000C // keyboard input to move cursor left one word
#define STB_TEXTEDIT_K_WORDRIGHT    0x1000D // keyboard input to move cursor right one word
#define STB_TEXTEDIT_K_SHIFT        0x20000

#define STB_TEXTEDIT_IMPLEMENTATION
#include "stb_textedit.h"

}

void ImGuiTextEditState::OnKeyPressed(int key)
{
    stb_textedit_key(this, &StbState, key);
    CursorFollow = true;
    CursorAnimReset();
}

// Public API to manipulate UTF-8 text
// We expose UTF-8 to the user (unlike the STB_TEXTEDIT_* functions which are manipulating wchar)
// FIXME: The existence of this rarely exercised code path is a bit of a nuisance.
void ImGuiTextEditCallbackData::DeleteChars(int pos, int bytes_count)
{
    IM_ASSERT(pos + bytes_count <= BufTextLen);
    char* dst = Buf + pos;
    const char* src = Buf + pos + bytes_count;
    while (char c = *src++)
        *dst++ = c;
    *dst = '\0';

    if (CursorPos + bytes_count >= pos)
        CursorPos -= bytes_count;
    else if (CursorPos >= pos)
        CursorPos = pos;
    SelectionStart = SelectionEnd = CursorPos;
    BufDirty = true;
    BufTextLen -= bytes_count;
}

void ImGuiTextEditCallbackData::InsertChars(int pos, const char* new_text, const char* new_text_end)
{
    const int new_text_len = new_text_end ? (int)(new_text_end - new_text) : (int)strlen(new_text);
    if (new_text_len + BufTextLen + 1 >= BufSize)
        return;

    if (BufTextLen != pos)
        memmove(Buf + pos + new_text_len, Buf + pos, (size_t)(BufTextLen - pos));
    memcpy(Buf + pos, new_text, (size_t)new_text_len * sizeof(char));
    Buf[BufTextLen + new_text_len] = '\0';

    if (CursorPos >= pos)
        CursorPos += new_text_len;
    SelectionStart = SelectionEnd = CursorPos;
    BufDirty = true;
    BufTextLen += new_text_len;
}

// Return false to discard a character.
static bool InputTextFilterCharacter(unsigned int* p_char, ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* user_data)
{
    unsigned int c = *p_char;

    if (c < 128 && c != ' ' && !isprint((int)(c & 0xFF)))
    {
        bool pass = false;
        pass |= (c == '\n' && (flags & ImGuiInputTextFlags_Multiline));
        pass |= (c == '\t' && (flags & ImGuiInputTextFlags_AllowTabInput));
        if (!pass)
            return false;
    }

    if (c >= 0xE000 && c <= 0xF8FF) // Filter private Unicode range. I don't imagine anybody would want to input them. GLFW on OSX seems to send private characters for special keys like arrow keys.
        return false;

    if (flags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank))
    {
        if (flags & ImGuiInputTextFlags_CharsDecimal)
            if (!(c >= '0' && c <= '9') && (c != '.') && (c != '-') && (c != '+') && (c != '*') && (c != '/'))
                return false;

        if (flags & ImGuiInputTextFlags_CharsHexadecimal)
            if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F'))
                return false;

        if (flags & ImGuiInputTextFlags_CharsUppercase)
            if (c >= 'a' && c <= 'z')
                *p_char = (c += (unsigned int)('A'-'a'));

        if (flags & ImGuiInputTextFlags_CharsNoBlank)
            if (ImCharIsSpace(c))
                return false;
    }

    if (flags & ImGuiInputTextFlags_CallbackCharFilter)
    {
        ImGuiTextEditCallbackData callback_data;
        memset(&callback_data, 0, sizeof(ImGuiTextEditCallbackData));
        callback_data.EventFlag = ImGuiInputTextFlags_CallbackCharFilter;
        callback_data.EventChar = (ImWchar)c;
        callback_data.Flags = flags;
        callback_data.UserData = user_data;
        if (callback(&callback_data) != 0)
            return false;
        *p_char = callback_data.EventChar;
        if (!callback_data.EventChar)
            return false;
    }

    return true;
}

// Edit a string of text
// NB: when active, hold on a privately held copy of the text (and apply back to 'buf'). So changing 'buf' while active has no effect.
// FIXME: Rather messy function partly because we are doing UTF8 > u16 > UTF8 conversions on the go to more easily handle stb_textedit calls. Ideally we should stay in UTF-8 all the time. See https://github.com/nothings/stb/issues/188
bool ImGui::InputTextEx(const char* label, char* buf, int buf_size, const ImVec2& size_arg, ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* user_data)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    IM_ASSERT(!((flags & ImGuiInputTextFlags_CallbackHistory) && (flags & ImGuiInputTextFlags_Multiline))); // Can't use both together (they both use up/down keys)
    IM_ASSERT(!((flags & ImGuiInputTextFlags_CallbackCompletion) && (flags & ImGuiInputTextFlags_AllowTabInput))); // Can't use both together (they both use tab key)

    ImGuiContext& g = *GImGui;
    const ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;

    const bool is_multiline = (flags & ImGuiInputTextFlags_Multiline) != 0;
    const bool is_editable = (flags & ImGuiInputTextFlags_ReadOnly) == 0;
    const bool is_password = (flags & ImGuiInputTextFlags_Password) != 0;

    if (is_multiline) // Open group before calling GetID() because groups tracks id created during their spawn
        BeginGroup();
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 size = CalcItemSize(size_arg, CalcItemWidth(), (is_multiline ? GetTextLineHeight() * 8.0f : label_size.y) + style.FramePadding.y*2.0f); // Arbitrary default of 8 lines high for multi-line
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? (style.ItemInnerSpacing.x + label_size.x) : 0.0f, 0.0f));

    ImGuiWindow* draw_window = window;
    if (is_multiline)
    {
        if (!BeginChildFrame(id, frame_bb.GetSize()))
        {
            EndChildFrame();
            EndGroup();
            return false;
        }
        draw_window = GetCurrentWindow();
        size.x -= draw_window->ScrollbarSizes.x;
    }
    else
    {
        ItemSize(total_bb, style.FramePadding.y);
        if (!ItemAdd(total_bb, id))
            return false;
    }
    const bool hovered = ItemHoverable(frame_bb, id);
    if (hovered)
        g.MouseCursor = ImGuiMouseCursor_TextInput;

    // Password pushes a temporary font with only a fallback glyph
    if (is_password)
    {
        const ImFontGlyph* glyph = g.Font->FindGlyph('*');
        ImFont* password_font = &g.InputTextPasswordFont;
        password_font->FontSize = g.Font->FontSize;
        password_font->Scale = g.Font->Scale;
        password_font->DisplayOffset = g.Font->DisplayOffset;
        password_font->Ascent = g.Font->Ascent;
        password_font->Descent = g.Font->Descent;
        password_font->ContainerAtlas = g.Font->ContainerAtlas;
        password_font->FallbackGlyph = glyph;
        password_font->FallbackAdvanceX = glyph->AdvanceX;
        IM_ASSERT(password_font->Glyphs.empty() && password_font->IndexAdvanceX.empty() && password_font->IndexLookup.empty());
        PushFont(password_font);
    }

    // NB: we are only allowed to access 'edit_state' if we are the active widget.
    ImGuiTextEditState& edit_state = g.InputTextState;

    const bool focus_requested = FocusableItemRegister(window, id, (flags & (ImGuiInputTextFlags_CallbackCompletion|ImGuiInputTextFlags_AllowTabInput)) == 0);    // Using completion callback disable keyboard tabbing
    const bool focus_requested_by_code = focus_requested && (window->FocusIdxAllCounter == window->FocusIdxAllRequestCurrent);
    const bool focus_requested_by_tab = focus_requested && !focus_requested_by_code;

    const bool user_clicked = hovered && io.MouseClicked[0];
    const bool user_scrolled = is_multiline && g.ActiveId == 0 && edit_state.Id == id && g.ActiveIdPreviousFrame == draw_window->GetIDNoKeepAlive("#SCROLLY");

    bool clear_active_id = false;

    bool select_all = (g.ActiveId != id) && (flags & ImGuiInputTextFlags_AutoSelectAll) != 0;
    if (focus_requested || user_clicked || user_scrolled)
    {
        if (g.ActiveId != id)
        {
            // Start edition
            // Take a copy of the initial buffer value (both in original UTF-8 format and converted to wchar)
            // From the moment we focused we are ignoring the content of 'buf' (unless we are in read-only mode)
            const int prev_len_w = edit_state.CurLenW;
            edit_state.Text.resize(buf_size+1);        // wchar count <= UTF-8 count. we use +1 to make sure that .Data isn't NULL so it doesn't crash.
            edit_state.InitialText.resize(buf_size+1); // UTF-8. we use +1 to make sure that .Data isn't NULL so it doesn't crash.
            ImStrncpy(edit_state.InitialText.Data, buf, edit_state.InitialText.Size);
            const char* buf_end = NULL;
            edit_state.CurLenW = ImTextStrFromUtf8(edit_state.Text.Data, edit_state.Text.Size, buf, NULL, &buf_end);
            edit_state.CurLenA = (int)(buf_end - buf); // We can't get the result from ImFormatString() above because it is not UTF-8 aware. Here we'll cut off malformed UTF-8.
            edit_state.CursorAnimReset();

            // Preserve cursor position and undo/redo stack if we come back to same widget
            // FIXME: We should probably compare the whole buffer to be on the safety side. Comparing buf (utf8) and edit_state.Text (wchar).
            const bool recycle_state = (edit_state.Id == id) && (prev_len_w == edit_state.CurLenW);
            if (recycle_state)
            {
                // Recycle existing cursor/selection/undo stack but clamp position
                // Note a single mouse click will override the cursor/position immediately by calling stb_textedit_click handler.
                edit_state.CursorClamp();
            }
            else
            {
                edit_state.Id = id;
                edit_state.ScrollX = 0.0f;
                stb_textedit_initialize_state(&edit_state.StbState, !is_multiline);
                if (!is_multiline && focus_requested_by_code)
                    select_all = true;
            }
            if (flags & ImGuiInputTextFlags_AlwaysInsertMode)
                edit_state.StbState.insert_mode = true;
            if (!is_multiline && (focus_requested_by_tab || (user_clicked && io.KeyCtrl)))
                select_all = true;
        }
        SetActiveID(id, window);
        FocusWindow(window);
    }
    else if (io.MouseClicked[0])
    {
        // Release focus when we click outside
        clear_active_id = true;
    }

    bool value_changed = false;
    bool enter_pressed = false;

    if (g.ActiveId == id)
    {
        if (!is_editable && !g.ActiveIdIsJustActivated)
        {
            // When read-only we always use the live data passed to the function
            edit_state.Text.resize(buf_size+1);
            const char* buf_end = NULL;
            edit_state.CurLenW = ImTextStrFromUtf8(edit_state.Text.Data, edit_state.Text.Size, buf, NULL, &buf_end);
            edit_state.CurLenA = (int)(buf_end - buf);
            edit_state.CursorClamp();
        }

        edit_state.BufSizeA = buf_size;

        // Although we are active we don't prevent mouse from hovering other elements unless we are interacting right now with the widget.
        // Down the line we should have a cleaner library-wide concept of Selected vs Active.
        g.ActiveIdAllowOverlap = !io.MouseDown[0];
        g.WantTextInputNextFrame = 1;

        // Edit in progress
        const float mouse_x = (io.MousePos.x - frame_bb.Min.x - style.FramePadding.x) + edit_state.ScrollX;
        const float mouse_y = (is_multiline ? (io.MousePos.y - draw_window->DC.CursorPos.y - style.FramePadding.y) : (g.FontSize*0.5f));

        const bool osx_double_click_selects_words = io.OSXBehaviors;      // OS X style: Double click selects by word instead of selecting whole text
        if (select_all || (hovered && !osx_double_click_selects_words && io.MouseDoubleClicked[0]))
        {
            edit_state.SelectAll();
            edit_state.SelectedAllMouseLock = true;
        }
        else if (hovered && osx_double_click_selects_words && io.MouseDoubleClicked[0])
        {
            // Select a word only, OS X style (by simulating keystrokes)
            edit_state.OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT);
            edit_state.OnKeyPressed(STB_TEXTEDIT_K_WORDRIGHT | STB_TEXTEDIT_K_SHIFT);
        }
        else if (io.MouseClicked[0] && !edit_state.SelectedAllMouseLock)
        {
            stb_textedit_click(&edit_state, &edit_state.StbState, mouse_x, mouse_y);
            edit_state.CursorAnimReset();
        }
        else if (io.MouseDown[0] && !edit_state.SelectedAllMouseLock && (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f))
        {
            stb_textedit_drag(&edit_state, &edit_state.StbState, mouse_x, mouse_y);
            edit_state.CursorAnimReset();
            edit_state.CursorFollow = true;
        }
        if (edit_state.SelectedAllMouseLock && !io.MouseDown[0])
            edit_state.SelectedAllMouseLock = false;

        if (io.InputCharacters[0])
        {
            // Process text input (before we check for Return because using some IME will effectively send a Return?)
            // We ignore CTRL inputs, but need to allow CTRL+ALT as some keyboards (e.g. German) use AltGR - which is Alt+Ctrl - to input certain characters.
            if (!(io.KeyCtrl && !io.KeyAlt) && is_editable)
            {
                for (int n = 0; n < IM_ARRAYSIZE(io.InputCharacters) && io.InputCharacters[n]; n++)
                    if (unsigned int c = (unsigned int)io.InputCharacters[n])
                    {
                        // Insert character if they pass filtering
                        if (!InputTextFilterCharacter(&c, flags, callback, user_data))
                            continue;
                        edit_state.OnKeyPressed((int)c);
                    }
            }

            // Consume characters
            memset(g.IO.InputCharacters, 0, sizeof(g.IO.InputCharacters));
        }
    }

    bool cancel_edit = false;
    if (g.ActiveId == id && !g.ActiveIdIsJustActivated && !clear_active_id)
    {
        // Handle key-presses
        const int k_mask = (io.KeyShift ? STB_TEXTEDIT_K_SHIFT : 0);
        const bool is_shortcut_key_only = (io.OSXBehaviors ? (io.KeySuper && !io.KeyCtrl) : (io.KeyCtrl && !io.KeySuper)) && !io.KeyAlt && !io.KeyShift; // OS X style: Shortcuts using Cmd/Super instead of Ctrl
        const bool is_wordmove_key_down = io.OSXBehaviors ? io.KeyAlt : io.KeyCtrl;                     // OS X style: Text editing cursor movement using Alt instead of Ctrl
        const bool is_startend_key_down = io.OSXBehaviors && io.KeySuper && !io.KeyCtrl && !io.KeyAlt;  // OS X style: Line/Text Start and End using Cmd+Arrows instead of Home/End

        if (IsKeyPressedMap(ImGuiKey_LeftArrow))                        { edit_state.OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINESTART : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDLEFT : STB_TEXTEDIT_K_LEFT) | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_RightArrow))                  { edit_state.OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINEEND : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDRIGHT : STB_TEXTEDIT_K_RIGHT) | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_UpArrow) && is_multiline)     { if (io.KeyCtrl) SetWindowScrollY(draw_window, ImMax(draw_window->Scroll.y - g.FontSize, 0.0f)); else edit_state.OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTSTART : STB_TEXTEDIT_K_UP) | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_DownArrow) && is_multiline)   { if (io.KeyCtrl) SetWindowScrollY(draw_window, ImMin(draw_window->Scroll.y + g.FontSize, GetScrollMaxY())); else edit_state.OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTEND : STB_TEXTEDIT_K_DOWN) | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_Home))                        { edit_state.OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTSTART | k_mask : STB_TEXTEDIT_K_LINESTART | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_End))                         { edit_state.OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTEND | k_mask : STB_TEXTEDIT_K_LINEEND | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_Delete) && is_editable)       { edit_state.OnKeyPressed(STB_TEXTEDIT_K_DELETE | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_Backspace) && is_editable)
        {
            if (!edit_state.HasSelection())
            {
                if (is_wordmove_key_down) edit_state.OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT|STB_TEXTEDIT_K_SHIFT);
                else if (io.OSXBehaviors && io.KeySuper && !io.KeyAlt && !io.KeyCtrl) edit_state.OnKeyPressed(STB_TEXTEDIT_K_LINESTART|STB_TEXTEDIT_K_SHIFT);
            }
            edit_state.OnKeyPressed(STB_TEXTEDIT_K_BACKSPACE | k_mask);
        }
        else if (IsKeyPressedMap(ImGuiKey_Enter))
        {
            bool ctrl_enter_for_new_line = (flags & ImGuiInputTextFlags_CtrlEnterForNewLine) != 0;
            if (!is_multiline || (ctrl_enter_for_new_line && !io.KeyCtrl) || (!ctrl_enter_for_new_line && io.KeyCtrl))
            {
                enter_pressed = clear_active_id = true;
            }
            else if (is_editable)
            {
                unsigned int c = '\n'; // Insert new line
                if (InputTextFilterCharacter(&c, flags, callback, user_data))
                    edit_state.OnKeyPressed((int)c);
            }
        }
        else if ((flags & ImGuiInputTextFlags_AllowTabInput) && IsKeyPressedMap(ImGuiKey_Tab) && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt && is_editable)
        {
            unsigned int c = '\t'; // Insert TAB
            if (InputTextFilterCharacter(&c, flags, callback, user_data))
                edit_state.OnKeyPressed((int)c);
        }
        else if (IsKeyPressedMap(ImGuiKey_Escape))                                      { clear_active_id = cancel_edit = true; }
        else if (is_shortcut_key_only && IsKeyPressedMap(ImGuiKey_Z) && is_editable)    { edit_state.OnKeyPressed(STB_TEXTEDIT_K_UNDO); edit_state.ClearSelection(); }
        else if (is_shortcut_key_only && IsKeyPressedMap(ImGuiKey_Y) && is_editable)    { edit_state.OnKeyPressed(STB_TEXTEDIT_K_REDO); edit_state.ClearSelection(); }
        else if (is_shortcut_key_only && IsKeyPressedMap(ImGuiKey_A))                   { edit_state.SelectAll(); edit_state.CursorFollow = true; }
        else if (is_shortcut_key_only && !is_password && ((IsKeyPressedMap(ImGuiKey_X) && is_editable) || IsKeyPressedMap(ImGuiKey_C)) && (!is_multiline || edit_state.HasSelection()))
        {
            // Cut, Copy
            const bool cut = IsKeyPressedMap(ImGuiKey_X);
            if (cut && !edit_state.HasSelection())
                edit_state.SelectAll();

            if (io.SetClipboardTextFn)
            {
                const int ib = edit_state.HasSelection() ? ImMin(edit_state.StbState.select_start, edit_state.StbState.select_end) : 0;
                const int ie = edit_state.HasSelection() ? ImMax(edit_state.StbState.select_start, edit_state.StbState.select_end) : edit_state.CurLenW;
                edit_state.TempTextBuffer.resize((ie-ib) * 4 + 1);
                ImTextStrToUtf8(edit_state.TempTextBuffer.Data, edit_state.TempTextBuffer.Size, edit_state.Text.Data+ib, edit_state.Text.Data+ie);
                SetClipboardText(edit_state.TempTextBuffer.Data);
            }

            if (cut)
            {
                edit_state.CursorFollow = true;
                stb_textedit_cut(&edit_state, &edit_state.StbState);
            }
        }
        else if (is_shortcut_key_only && IsKeyPressedMap(ImGuiKey_V) && is_editable)
        {
            // Paste
            if (const char* clipboard = GetClipboardText())
            {
                // Filter pasted buffer
                const int clipboard_len = (int)strlen(clipboard);
                ImWchar* clipboard_filtered = (ImWchar*)ImGui::MemAlloc((clipboard_len+1) * sizeof(ImWchar));
                int clipboard_filtered_len = 0;
                for (const char* s = clipboard; *s; )
                {
                    unsigned int c;
                    s += ImTextCharFromUtf8(&c, s, NULL);
                    if (c == 0)
                        break;
                    if (c >= 0x10000 || !InputTextFilterCharacter(&c, flags, callback, user_data))
                        continue;
                    clipboard_filtered[clipboard_filtered_len++] = (ImWchar)c;
                }
                clipboard_filtered[clipboard_filtered_len] = 0;
                if (clipboard_filtered_len > 0) // If everything was filtered, ignore the pasting operation
                {
                    stb_textedit_paste(&edit_state, &edit_state.StbState, clipboard_filtered, clipboard_filtered_len);
                    edit_state.CursorFollow = true;
                }
                ImGui::MemFree(clipboard_filtered);
            }
        }
    }

    if (g.ActiveId == id)
    {
        if (cancel_edit)
        {
            // Restore initial value
            if (is_editable)
            {
                ImStrncpy(buf, edit_state.InitialText.Data, buf_size);
                value_changed = true;
            }
        }

        // When using 'ImGuiInputTextFlags_EnterReturnsTrue' as a special case we reapply the live buffer back to the input buffer before clearing ActiveId, even though strictly speaking it wasn't modified on this frame.
        // If we didn't do that, code like InputInt() with ImGuiInputTextFlags_EnterReturnsTrue would fail. Also this allows the user to use InputText() with ImGuiInputTextFlags_EnterReturnsTrue without maintaining any user-side storage.
        bool apply_edit_back_to_user_buffer = !cancel_edit || (enter_pressed && (flags & ImGuiInputTextFlags_EnterReturnsTrue) != 0);
        if (apply_edit_back_to_user_buffer)
        {
            // Apply new value immediately - copy modified buffer back
            // Note that as soon as the input box is active, the in-widget value gets priority over any underlying modification of the input buffer
            // FIXME: We actually always render 'buf' when calling DrawList->AddText, making the comment above incorrect.
            // FIXME-OPT: CPU waste to do this every time the widget is active, should mark dirty state from the stb_textedit callbacks.
            if (is_editable)
            {
                edit_state.TempTextBuffer.resize(edit_state.Text.Size * 4);
                ImTextStrToUtf8(edit_state.TempTextBuffer.Data, edit_state.TempTextBuffer.Size, edit_state.Text.Data, NULL);
            }

            // User callback
            if ((flags & (ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackAlways)) != 0)
            {
                IM_ASSERT(callback != NULL);

                // The reason we specify the usage semantic (Completion/History) is that Completion needs to disable keyboard TABBING at the moment.
                ImGuiInputTextFlags event_flag = 0;
                ImGuiKey event_key = ImGuiKey_COUNT;
                if ((flags & ImGuiInputTextFlags_CallbackCompletion) != 0 && IsKeyPressedMap(ImGuiKey_Tab))
                {
                    event_flag = ImGuiInputTextFlags_CallbackCompletion;
                    event_key = ImGuiKey_Tab;
                }
                else if ((flags & ImGuiInputTextFlags_CallbackHistory) != 0 && IsKeyPressedMap(ImGuiKey_UpArrow))
                {
                    event_flag = ImGuiInputTextFlags_CallbackHistory;
                    event_key = ImGuiKey_UpArrow;
                }
                else if ((flags & ImGuiInputTextFlags_CallbackHistory) != 0 && IsKeyPressedMap(ImGuiKey_DownArrow))
                {
                    event_flag = ImGuiInputTextFlags_CallbackHistory;
                    event_key = ImGuiKey_DownArrow;
                }
                else if (flags & ImGuiInputTextFlags_CallbackAlways)
                    event_flag = ImGuiInputTextFlags_CallbackAlways;

                if (event_flag)
                {
                    ImGuiTextEditCallbackData callback_data;
                    memset(&callback_data, 0, sizeof(ImGuiTextEditCallbackData));
                    callback_data.EventFlag = event_flag;
                    callback_data.Flags = flags;
                    callback_data.UserData = user_data;
                    callback_data.ReadOnly = !is_editable;

                    callback_data.EventKey = event_key;
                    callback_data.Buf = edit_state.TempTextBuffer.Data;
                    callback_data.BufTextLen = edit_state.CurLenA;
                    callback_data.BufSize = edit_state.BufSizeA;
                    callback_data.BufDirty = false;

                    // We have to convert from wchar-positions to UTF-8-positions, which can be pretty slow (an incentive to ditch the ImWchar buffer, see https://github.com/nothings/stb/issues/188)
                    ImWchar* text = edit_state.Text.Data;
                    const int utf8_cursor_pos = callback_data.CursorPos = ImTextCountUtf8BytesFromStr(text, text + edit_state.StbState.cursor);
                    const int utf8_selection_start = callback_data.SelectionStart = ImTextCountUtf8BytesFromStr(text, text + edit_state.StbState.select_start);
                    const int utf8_selection_end = callback_data.SelectionEnd = ImTextCountUtf8BytesFromStr(text, text + edit_state.StbState.select_end);

                    // Call user code
                    callback(&callback_data);

                    // Read back what user may have modified
                    IM_ASSERT(callback_data.Buf == edit_state.TempTextBuffer.Data);  // Invalid to modify those fields
                    IM_ASSERT(callback_data.BufSize == edit_state.BufSizeA);
                    IM_ASSERT(callback_data.Flags == flags);
                    if (callback_data.CursorPos != utf8_cursor_pos)            edit_state.StbState.cursor = ImTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.CursorPos);
                    if (callback_data.SelectionStart != utf8_selection_start)  edit_state.StbState.select_start = ImTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.SelectionStart);
                    if (callback_data.SelectionEnd != utf8_selection_end)      edit_state.StbState.select_end = ImTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.SelectionEnd);
                    if (callback_data.BufDirty)
                    {
                        IM_ASSERT(callback_data.BufTextLen == (int)strlen(callback_data.Buf)); // You need to maintain BufTextLen if you change the text!
                        edit_state.CurLenW = ImTextStrFromUtf8(edit_state.Text.Data, edit_state.Text.Size, callback_data.Buf, NULL);
                        edit_state.CurLenA = callback_data.BufTextLen;  // Assume correct length and valid UTF-8 from user, saves us an extra strlen()
                        edit_state.CursorAnimReset();
                    }
                }
            }

            // Copy back to user buffer
            if (is_editable && strcmp(edit_state.TempTextBuffer.Data, buf) != 0)
            {
                ImStrncpy(buf, edit_state.TempTextBuffer.Data, buf_size);
                value_changed = true;
            }
        }
    }

    // Release active ID at the end of the function (so e.g. pressing Return still does a final application of the value)
    if (clear_active_id && g.ActiveId == id)
        ClearActiveID();

    // Render
    // Select which buffer we are going to display. When ImGuiInputTextFlags_NoLiveEdit is set 'buf' might still be the old value. We set buf to NULL to prevent accidental usage from now on.
    const char* buf_display = (g.ActiveId == id && is_editable) ? edit_state.TempTextBuffer.Data : buf; buf = NULL; 

    if (!is_multiline)
        RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    const ImVec4 clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + size.x, frame_bb.Min.y + size.y); // Not using frame_bb.Max because we have adjusted size
    ImVec2 render_pos = is_multiline ? draw_window->DC.CursorPos : frame_bb.Min + style.FramePadding;
    ImVec2 text_size(0.f, 0.f);
    const bool is_currently_scrolling = (edit_state.Id == id && is_multiline && g.ActiveId == draw_window->GetIDNoKeepAlive("#SCROLLY"));
    if (g.ActiveId == id || is_currently_scrolling)
    {
        edit_state.CursorAnim += io.DeltaTime;

        // This is going to be messy. We need to:
        // - Display the text (this alone can be more easily clipped)
        // - Handle scrolling, highlight selection, display cursor (those all requires some form of 1d->2d cursor position calculation)
        // - Measure text height (for scrollbar)
        // We are attempting to do most of that in **one main pass** to minimize the computation cost (non-negligible for large amount of text) + 2nd pass for selection rendering (we could merge them by an extra refactoring effort)
        // FIXME: This should occur on buf_display but we'd need to maintain cursor/select_start/select_end for UTF-8.
        const ImWchar* text_begin = edit_state.Text.Data;
        ImVec2 cursor_offset, select_start_offset;

        {
            // Count lines + find lines numbers straddling 'cursor' and 'select_start' position.
            const ImWchar* searches_input_ptr[2];
            searches_input_ptr[0] = text_begin + edit_state.StbState.cursor;
            searches_input_ptr[1] = NULL;
            int searches_remaining = 1;
            int searches_result_line_number[2] = { -1, -999 };
            if (edit_state.StbState.select_start != edit_state.StbState.select_end)
            {
                searches_input_ptr[1] = text_begin + ImMin(edit_state.StbState.select_start, edit_state.StbState.select_end);
                searches_result_line_number[1] = -1;
                searches_remaining++;
            }

            // Iterate all lines to find our line numbers
            // In multi-line mode, we never exit the loop until all lines are counted, so add one extra to the searches_remaining counter.
            searches_remaining += is_multiline ? 1 : 0;
            int line_count = 0;
            for (const ImWchar* s = text_begin; *s != 0; s++)
                if (*s == '\n')
                {
                    line_count++;
                    if (searches_result_line_number[0] == -1 && s >= searches_input_ptr[0]) { searches_result_line_number[0] = line_count; if (--searches_remaining <= 0) break; }
                    if (searches_result_line_number[1] == -1 && s >= searches_input_ptr[1]) { searches_result_line_number[1] = line_count; if (--searches_remaining <= 0) break; }
                }
            line_count++;
            if (searches_result_line_number[0] == -1) searches_result_line_number[0] = line_count;
            if (searches_result_line_number[1] == -1) searches_result_line_number[1] = line_count;

            // Calculate 2d position by finding the beginning of the line and measuring distance
            cursor_offset.x = InputTextCalcTextSizeW(ImStrbolW(searches_input_ptr[0], text_begin), searches_input_ptr[0]).x;
            cursor_offset.y = searches_result_line_number[0] * g.FontSize;
            if (searches_result_line_number[1] >= 0)
            {
                select_start_offset.x = InputTextCalcTextSizeW(ImStrbolW(searches_input_ptr[1], text_begin), searches_input_ptr[1]).x;
                select_start_offset.y = searches_result_line_number[1] * g.FontSize;
            }

            // Store text height (note that we haven't calculated text width at all, see GitHub issues #383, #1224)
            if (is_multiline)
                text_size = ImVec2(size.x, line_count * g.FontSize);
        }

        // Scroll
        if (edit_state.CursorFollow)
        {
            // Horizontal scroll in chunks of quarter width
            if (!(flags & ImGuiInputTextFlags_NoHorizontalScroll))
            {
                const float scroll_increment_x = size.x * 0.25f;
                if (cursor_offset.x < edit_state.ScrollX)
                    edit_state.ScrollX = (float)(int)ImMax(0.0f, cursor_offset.x - scroll_increment_x);
                else if (cursor_offset.x - size.x >= edit_state.ScrollX)
                    edit_state.ScrollX = (float)(int)(cursor_offset.x - size.x + scroll_increment_x);
            }
            else
            {
                edit_state.ScrollX = 0.0f;
            }

            // Vertical scroll
            if (is_multiline)
            {
                float scroll_y = draw_window->Scroll.y;
                if (cursor_offset.y - g.FontSize < scroll_y)
                    scroll_y = ImMax(0.0f, cursor_offset.y - g.FontSize);
                else if (cursor_offset.y - size.y >= scroll_y)
                    scroll_y = cursor_offset.y - size.y;
                draw_window->DC.CursorPos.y += (draw_window->Scroll.y - scroll_y);   // To avoid a frame of lag
                draw_window->Scroll.y = scroll_y;
                render_pos.y = draw_window->DC.CursorPos.y;
            }
        }
        edit_state.CursorFollow = false;
        const ImVec2 render_scroll = ImVec2(edit_state.ScrollX, 0.0f);

        // Draw selection
        if (edit_state.StbState.select_start != edit_state.StbState.select_end)
        {
            const ImWchar* text_selected_begin = text_begin + ImMin(edit_state.StbState.select_start, edit_state.StbState.select_end);
            const ImWchar* text_selected_end = text_begin + ImMax(edit_state.StbState.select_start, edit_state.StbState.select_end);

            float bg_offy_up = is_multiline ? 0.0f : -1.0f;    // FIXME: those offsets should be part of the style? they don't play so well with multi-line selection.
            float bg_offy_dn = is_multiline ? 0.0f : 2.0f;
            ImU32 bg_color = GetColorU32(ImGuiCol_TextSelectedBg);
            ImVec2 rect_pos = render_pos + select_start_offset - render_scroll;
            for (const ImWchar* p = text_selected_begin; p < text_selected_end; )
            {
                if (rect_pos.y > clip_rect.w + g.FontSize)
                    break;
                if (rect_pos.y < clip_rect.y)
                {
                    while (p < text_selected_end)
                        if (*p++ == '\n')
                            break;
                }
                else
                {
                    ImVec2 rect_size = InputTextCalcTextSizeW(p, text_selected_end, &p, NULL, true);
                    if (rect_size.x <= 0.0f) rect_size.x = (float)(int)(g.Font->GetCharAdvance((unsigned short)' ') * 0.50f); // So we can see selected empty lines
                    ImRect rect(rect_pos + ImVec2(0.0f, bg_offy_up - g.FontSize), rect_pos +ImVec2(rect_size.x, bg_offy_dn));
                    rect.ClipWith(clip_rect);
                    if (rect.Overlaps(clip_rect))
                        draw_window->DrawList->AddRectFilled(rect.Min, rect.Max, bg_color);
                }
                rect_pos.x = render_pos.x - render_scroll.x;
                rect_pos.y += g.FontSize;
            }
        }

        draw_window->DrawList->AddText(g.Font, g.FontSize, render_pos - render_scroll, GetColorU32(ImGuiCol_Text), buf_display, buf_display + edit_state.CurLenA, 0.0f, is_multiline ? NULL : &clip_rect);

        // Draw blinking cursor
        bool cursor_is_visible = (g.InputTextState.CursorAnim <= 0.0f) || fmodf(g.InputTextState.CursorAnim, 1.20f) <= 0.80f;
        ImVec2 cursor_screen_pos = render_pos + cursor_offset - render_scroll;
        ImRect cursor_screen_rect(cursor_screen_pos.x, cursor_screen_pos.y-g.FontSize+0.5f, cursor_screen_pos.x+1.0f, cursor_screen_pos.y-1.5f);
        if (cursor_is_visible && cursor_screen_rect.Overlaps(clip_rect))
            draw_window->DrawList->AddLine(cursor_screen_rect.Min, cursor_screen_rect.GetBL(), GetColorU32(ImGuiCol_Text));

        // Notify OS of text input position for advanced IME (-1 x offset so that Windows IME can cover our cursor. Bit of an extra nicety.)
        if (is_editable)
            g.OsImePosRequest = ImVec2(cursor_screen_pos.x - 1, cursor_screen_pos.y - g.FontSize);
    }
    else
    {
        // Render text only
        const char* buf_end = NULL;
        if (is_multiline)
            text_size = ImVec2(size.x, InputTextCalcTextLenAndLineCount(buf_display, &buf_end) * g.FontSize); // We don't need width
        draw_window->DrawList->AddText(g.Font, g.FontSize, render_pos, GetColorU32(ImGuiCol_Text), buf_display, buf_end, 0.0f, is_multiline ? NULL : &clip_rect);
    }

    if (is_multiline)
    {
        Dummy(text_size + ImVec2(0.0f, g.FontSize)); // Always add room to scroll an extra line
        EndChildFrame();
        EndGroup();
    }

    if (is_password)
        PopFont();

    // Log as text
    if (g.LogEnabled && !is_password)
        LogRenderedText(&render_pos, buf_display, NULL);

    if (label_size.x > 0)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    if ((flags & ImGuiInputTextFlags_EnterReturnsTrue) != 0)
        return enter_pressed;
    else
        return value_changed;
}

bool ImGui::InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* user_data)
{
    IM_ASSERT(!(flags & ImGuiInputTextFlags_Multiline)); // call InputTextMultiline()
    return InputTextEx(label, buf, (int)buf_size, ImVec2(0,0), flags, callback, user_data);
}

bool ImGui::InputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* user_data)
{
    return InputTextEx(label, buf, (int)buf_size, size, flags | ImGuiInputTextFlags_Multiline, callback, user_data);
}

static inline float SmallSquareSize()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize + g.Style.FramePadding.y * 2.0f;
}

// NB: scalar_format here must be a simple "%xx" format string with no prefix/suffix (unlike the Drag/Slider functions "display_format" argument)
bool ImGui::InputScalarEx(const char* label, ImGuiDataType data_type, void* data_ptr, void* step_ptr, void* step_fast_ptr, const char* scalar_format, ImGuiInputTextFlags extra_flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    BeginGroup();
    PushID(label);
    const ImVec2 button_sz = ImVec2(SmallSquareSize(), SmallSquareSize());
    if (step_ptr)
        PushItemWidth(ImMax(1.0f, CalcItemWidth() - (button_sz.x + style.ItemInnerSpacing.x)*2));

    char buf[64];
    DataTypeFormatString(data_type, data_ptr, scalar_format, buf, IM_ARRAYSIZE(buf));

    bool value_changed = false;
    if (!(extra_flags & ImGuiInputTextFlags_CharsHexadecimal))
        extra_flags |= ImGuiInputTextFlags_CharsDecimal;
    extra_flags |= ImGuiInputTextFlags_AutoSelectAll;
    if (InputText("", buf, IM_ARRAYSIZE(buf), extra_flags)) // PushId(label) + "" gives us the expected ID from outside point of view
        value_changed = DataTypeApplyOpFromText(buf, GImGui->InputTextState.InitialText.begin(), data_type, data_ptr, scalar_format);

    // Step buttons
    if (step_ptr)
    {
        PopItemWidth();
        SameLine(0, style.ItemInnerSpacing.x);
        if (ButtonEx("-", button_sz, ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups))
        {
            DataTypeApplyOp(data_type, '-', data_ptr, g.IO.KeyCtrl && step_fast_ptr ? step_fast_ptr : step_ptr);
            value_changed = true;
        }
        SameLine(0, style.ItemInnerSpacing.x);
        if (ButtonEx("+", button_sz, ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups))
        {
            DataTypeApplyOp(data_type, '+', data_ptr, g.IO.KeyCtrl && step_fast_ptr ? step_fast_ptr : step_ptr);
            value_changed = true;
        }
    }
    PopID();

    if (label_size.x > 0)
    {
        SameLine(0, style.ItemInnerSpacing.x);
        RenderText(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y), label);
        ItemSize(label_size, style.FramePadding.y);
    }
    EndGroup();

    return value_changed;
}

bool ImGui::InputFloat(const char* label, float* v, float step, float step_fast, int decimal_precision, ImGuiInputTextFlags extra_flags)
{
    char display_format[16];
    if (decimal_precision < 0)
        strcpy(display_format, "%f");      // Ideally we'd have a minimum decimal precision of 1 to visually denote that this is a float, while hiding non-significant digits? %f doesn't have a minimum of 1
    else
        ImFormatString(display_format, IM_ARRAYSIZE(display_format), "%%.%df", decimal_precision);
    return InputScalarEx(label, ImGuiDataType_Float, (void*)v, (void*)(step>0.0f ? &step : NULL), (void*)(step_fast>0.0f ? &step_fast : NULL), display_format, extra_flags);
}

bool ImGui::InputInt(const char* label, int* v, int step, int step_fast, ImGuiInputTextFlags extra_flags)
{
    // Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
    const char* scalar_format = (extra_flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
    return InputScalarEx(label, ImGuiDataType_Int, (void*)v, (void*)(step>0.0f ? &step : NULL), (void*)(step_fast>0.0f ? &step_fast : NULL), scalar_format, extra_flags);
}

bool ImGui::InputFloatN(const char* label, float* v, int components, int decimal_precision, ImGuiInputTextFlags extra_flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    BeginGroup();
    PushID(label);
    PushMultiItemsWidths(components);
    for (int i = 0; i < components; i++)
    {
        PushID(i);
        value_changed |= InputFloat("##v", &v[i], 0, 0, decimal_precision, extra_flags);
        SameLine(0, g.Style.ItemInnerSpacing.x);
        PopID();
        PopItemWidth();
    }
    PopID();

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();

    return value_changed;
}

bool ImGui::InputFloat2(const char* label, float v[2], int decimal_precision, ImGuiInputTextFlags extra_flags)
{
    return InputFloatN(label, v, 2, decimal_precision, extra_flags);
}

bool ImGui::InputFloat3(const char* label, float v[3], int decimal_precision, ImGuiInputTextFlags extra_flags)
{
    return InputFloatN(label, v, 3, decimal_precision, extra_flags);
}

bool ImGui::InputFloat4(const char* label, float v[4], int decimal_precision, ImGuiInputTextFlags extra_flags)
{
    return InputFloatN(label, v, 4, decimal_precision, extra_flags);
}

bool ImGui::InputIntN(const char* label, int* v, int components, ImGuiInputTextFlags extra_flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    BeginGroup();
    PushID(label);
    PushMultiItemsWidths(components);
    for (int i = 0; i < components; i++)
    {
        PushID(i);
        value_changed |= InputInt("##v", &v[i], 0, 0, extra_flags);
        SameLine(0, g.Style.ItemInnerSpacing.x);
        PopID();
        PopItemWidth();
    }
    PopID();

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();

    return value_changed;
}

bool ImGui::InputInt2(const char* label, int v[2], ImGuiInputTextFlags extra_flags)
{
    return InputIntN(label, v, 2, extra_flags);
}

bool ImGui::InputInt3(const char* label, int v[3], ImGuiInputTextFlags extra_flags)
{
    return InputIntN(label, v, 3, extra_flags);
}

bool ImGui::InputInt4(const char* label, int v[4], ImGuiInputTextFlags extra_flags)
{
    return InputIntN(label, v, 4, extra_flags);
}

static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
    const char* const* items = (const char* const*)data;
    if (out_text)
        *out_text = items[idx];
    return true;
}

static bool Items_SingleStringGetter(void* data, int idx, const char** out_text)
{
    // FIXME-OPT: we could pre-compute the indices to fasten this. But only 1 active combo means the waste is limited.
    const char* items_separated_by_zeros = (const char*)data;
    int items_count = 0;
    const char* p = items_separated_by_zeros;
    while (*p)
    {
        if (idx == items_count)
            break;
        p += strlen(p) + 1;
        items_count++;
    }
    if (!*p)
        return false;
    if (out_text)
        *out_text = p;
    return true;
}

// Combo box helper allowing to pass an array of strings.
bool ImGui::Combo(const char* label, int* current_item, const char* const* items, int items_count, int height_in_items)
{
    const bool value_changed = Combo(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_in_items);
    return value_changed;
}

// Combo box helper allowing to pass all items in a single string.
bool ImGui::Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int height_in_items)
{
    int items_count = 0;
    const char* p = items_separated_by_zeros;       // FIXME-OPT: Avoid computing this, or at least only when combo is open
    while (*p)
    {
        p += strlen(p) + 1;
        items_count++;
    }
    bool value_changed = Combo(label, current_item, Items_SingleStringGetter, (void*)items_separated_by_zeros, items_count, height_in_items);
    return value_changed;
}

bool ImGui::BeginCombo(const char* label, const char* preview_value, ImVec2 popup_size)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    float w = CalcItemWidth(); // Returns wrong values, itemwidth is -1 for some odd reason

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y*2.0f));
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
        return false;

    const float arrow_size = SmallSquareSize();

    bool hovered, held;
    bool pressed = ButtonBehavior(frame_bb, id, &hovered, &held);

    bool popup_open = IsPopupOpen(id);

    const ImRect value_bb(frame_bb.Min, frame_bb.Max - ImVec2(arrow_size, 0.0f));
    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
    RenderFrame(ImVec2(frame_bb.Max.x-arrow_size, frame_bb.Min.y), frame_bb.Max, GetColorU32(popup_open || hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button), true, style.FrameRounding); // FIXME-ROUNDING
    RenderTriangle(ImVec2(frame_bb.Max.x - arrow_size + style.FramePadding.y, frame_bb.Min.y + style.FramePadding.y), ImGuiDir_Down);

    if (preview_value != NULL)
        RenderTextClipped(frame_bb.Min + style.FramePadding, value_bb.Max, preview_value, NULL, NULL, ImVec2(0.0f,0.0f));

    if (label_size.x > 0)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    if (pressed && !popup_open)
    {
        OpenPopupEx(id, false);
        popup_open = true;
    }

    if (!popup_open)
        return false;

    if (popup_size.x == 0.0f)
        popup_size.x = w;

    float popup_y1 = frame_bb.Max.y;
    float popup_y2 = ImClamp(popup_y1 + popup_size.y, popup_y1, g.IO.DisplaySize.y - style.DisplaySafeAreaPadding.y);
    if ((popup_y2 - popup_y1) < ImMin(popup_size.y, frame_bb.Min.y - style.DisplaySafeAreaPadding.y))
    {
        // Position our combo ABOVE because there's more space to fit! (FIXME: Handle in Begin() or use a shared helper. We have similar code in Begin() for popup placement)
        popup_y1 = ImClamp(frame_bb.Min.y - popup_size.y, style.DisplaySafeAreaPadding.y, frame_bb.Min.y);
        popup_y2 = frame_bb.Min.y;
        SetNextWindowPos(ImVec2(frame_bb.Min.x, frame_bb.Min.y), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    }
    else
    {
        // Position our combo below
        SetNextWindowPos(ImVec2(frame_bb.Min.x, frame_bb.Max.y), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
    }
    SetNextWindowSize(ImVec2(popup_size.x, popup_y2 - popup_y1), ImGuiCond_Appearing);
    PushStyleVar(ImGuiStyleVar_WindowPadding, style.FramePadding);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_ComboBox | ((window->Flags & ImGuiWindowFlags_ShowBorders) ? ImGuiWindowFlags_ShowBorders : 0);
    if (!BeginPopupEx(id, flags))
    {
        IM_ASSERT(0);   // This should never happen as we tested for IsPopupOpen() above
        return false;
    }
    Spacing();

    return true;
}

void ImGui::EndCombo()
{
    EndPopup();
    PopStyleVar();
}

// Combo box function.
bool ImGui::Combo(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int height_in_items)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const char* preview_text = NULL;
    if (*current_item >= 0 && *current_item < items_count)
        items_getter(data, *current_item, &preview_text);

    // Size default to hold ~7 items
    if (height_in_items < 0)
        height_in_items = 7;
    float popup_height = (g.FontSize + style.ItemSpacing.y) * ImMin(items_count, height_in_items) + (style.FramePadding.y * 3);

    if (!BeginCombo(label, preview_text, ImVec2(0.0f, popup_height)))
        return false;

    // Display items
    // FIXME-OPT: Use clipper
    bool value_changed = false;
    for (int i = 0; i < items_count; i++)
    {
        PushID((void*)(intptr_t)i);
        const bool item_selected = (i == *current_item);
        const char* item_text;
        if (!items_getter(data, i, &item_text))
            item_text = "*Unknown item*";
        if (Selectable(item_text, item_selected))
        {
            value_changed = true;
            *current_item = i;
        }
        if (item_selected && IsWindowAppearing())
            SetScrollHere();
        PopID();
    }

    EndCombo();
    return value_changed;
}

// Tip: pass an empty label (e.g. "##dummy") then you can use the space to draw other text or image.
// But you need to make sure the ID is unique, e.g. enclose calls in PushID/PopID.
bool ImGui::Selectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.ColumnsCount > 1) // FIXME-OPT: Avoid if vertically clipped.
        PopClipRect();

    ImGuiID id = window->GetID(label);
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrentLineTextBaseOffset;
    ImRect bb(pos, pos + size);
    ItemSize(bb);

    // Fill horizontal space.
    ImVec2 window_padding = window->WindowPadding;
    float max_x = (flags & ImGuiSelectableFlags_SpanAllColumns) ? GetWindowContentRegionMax().x : GetContentRegionMax().x;
    float w_draw = ImMax(label_size.x, window->Pos.x + max_x - window_padding.x - window->DC.CursorPos.x);
    ImVec2 size_draw((size_arg.x != 0 && !(flags & ImGuiSelectableFlags_DrawFillAvailWidth)) ? size_arg.x : w_draw, size_arg.y != 0.0f ? size_arg.y : size.y);
    ImRect bb_with_spacing(pos, pos + size_draw); // stupid high value from combobox, no idea how to fix w/o band-aid bs
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_DrawFillAvailWidth))
        bb_with_spacing.Max.x += window_padding.x;

    // Selectables are tightly packed together, we extend the box to cover spacing between selectable.
    float spacing_L = (float)(int)(style.ItemSpacing.x * 0.5f);
    float spacing_U = (float)(int)(style.ItemSpacing.y * 0.5f);
    float spacing_R = style.ItemSpacing.x - spacing_L;
    float spacing_D = style.ItemSpacing.y - spacing_U;
    bb_with_spacing.Min.x -= spacing_L;
    bb_with_spacing.Min.y -= spacing_U;
    bb_with_spacing.Max.x += spacing_R;
    bb_with_spacing.Max.y += spacing_D;
    if (!ItemAdd(bb_with_spacing, id))
    {
        if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.ColumnsCount > 1)
            PushColumnClipRect();
        return false;
    }

    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_Menu) button_flags |= ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_NoHoldingActiveID;
    if (flags & ImGuiSelectableFlags_MenuItem) button_flags |= ImGuiButtonFlags_PressedOnRelease;
    if (flags & ImGuiSelectableFlags_Disabled) button_flags |= ImGuiButtonFlags_Disabled;
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb_with_spacing, id, &hovered, &held, button_flags);
    if (flags & ImGuiSelectableFlags_Disabled)
        selected = false;

    // Render
    if (hovered || selected)
    {
        const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
        RenderFrame(bb_with_spacing.Min, bb_with_spacing.Max, col, false, 0.0f);
    }

    if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.ColumnsCount > 1)
    {
        PushColumnClipRect();
        bb_with_spacing.Max.x -= (GetContentRegionMax().x - max_x);
    }

    if (flags & ImGuiSelectableFlags_Disabled) PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
    RenderTextClipped(bb.Min, bb_with_spacing.Max, label, NULL, &label_size, ImVec2(0.0f,0.0f));
    if (flags & ImGuiSelectableFlags_Disabled) PopStyleColor();

    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(window->DC.ItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
        CloseCurrentPopup();
    return pressed;
}

bool ImGui::Selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
    if (Selectable(label, *p_selected, flags, size_arg))
    {
        *p_selected = !*p_selected;
        return true;
    }
    return false;
}

// Helper to calculate the size of a listbox and display a label on the right.
// Tip: To have a list filling the entire window width, PushItemWidth(-1) and pass an empty label "##empty"
bool ImGui::ListBoxHeader(const char* label, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = GetStyle();
    const ImGuiID id = GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    // Size default to hold ~7 items. Fractional number of items helps seeing that we can scroll down/up without looking at scrollbar.
    ImVec2 size = CalcItemSize(size_arg, CalcItemWidth(), GetTextLineHeightWithSpacing() * 7.4f + style.ItemSpacing.y);
    ImVec2 frame_size = ImVec2(size.x, ImMax(size.y, label_size.y));
    ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
    window->DC.LastItemRect = bb;

    BeginGroup();
    if (label_size.x > 0)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    BeginChildFrame(id, frame_bb.GetSize());
    return true;
}

bool ImGui::ListBoxHeader(const char* label, int items_count, int height_in_items)
{
    // Size default to hold ~7 items. Fractional number of items helps seeing that we can scroll down/up without looking at scrollbar.
    // However we don't add +0.40f if items_count <= height_in_items. It is slightly dodgy, because it means a dynamic list of items will make the widget resize occasionally when it crosses that size.
    // I am expecting that someone will come and complain about this behavior in a remote future, then we can advise on a better solution.
    if (height_in_items < 0)
        height_in_items = ImMin(items_count, 7);
    float height_in_items_f = height_in_items < items_count ? (height_in_items + 0.40f) : (height_in_items + 0.00f);

    // We include ItemSpacing.y so that a list sized for the exact number of items doesn't make a scrollbar appears. We could also enforce that by passing a flag to BeginChild().
    ImVec2 size;
    size.x = 0.0f;
    size.y = GetTextLineHeightWithSpacing() * height_in_items_f + GetStyle().ItemSpacing.y;
    return ListBoxHeader(label, size);
}

void ImGui::ListBoxFooter()
{
    ImGuiWindow* parent_window = GetParentWindow();
    const ImRect bb = parent_window->DC.LastItemRect;
    const ImGuiStyle& style = GetStyle();

    EndChildFrame();

    // Redeclare item size so that it includes the label (we have stored the full size in LastItemRect)
    // We call SameLine() to restore DC.CurrentLine* data
    SameLine();
    parent_window->DC.CursorPos = bb.Min;
    ItemSize(bb, style.FramePadding.y);
    EndGroup();
}

bool ImGui::ListBox(const char* label, int* current_item, const char* const* items, int items_count, int height_items)
{
    const bool value_changed = ListBox(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_items);
    return value_changed;
}

bool ImGui::ListBox(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int height_in_items)
{
    if (!ListBoxHeader(label, items_count, height_in_items))
        return false;

    // Assume all items have even height (= 1 line of text). If you need items of different or variable sizes you can create a custom version of ListBox() in your code without using the clipper.
    bool value_changed = false;
    ImGuiListClipper clipper(items_count, GetTextLineHeightWithSpacing()); // We know exactly our line height here so we pass it as a minor optimization, but generally you don't need to.
    while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            const bool item_selected = (i == *current_item);
            const char* item_text;
            if (!items_getter(data, i, &item_text))
                item_text = "*Unknown item*";

            PushID(i);
            if (Selectable(item_text, item_selected))
            {
                *current_item = i;
                value_changed = true;
            }
            PopID();
        }
    ListBoxFooter();
    return value_changed;
}

bool ImGui::MenuItem(const char* label, const char* shortcut, bool selected, bool enabled)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 shortcut_size = shortcut ? CalcTextSize(shortcut, NULL) : ImVec2(0.0f, 0.0f);
    float w = window->MenuColumns.DeclColumns(label_size.x, shortcut_size.x, (float)(int)(g.FontSize * 1.20f)); // Feedback for next frame
    float extra_w = ImMax(0.0f, GetContentRegionAvail().x - w);

    bool pressed = Selectable(label, false, ImGuiSelectableFlags_MenuItem | ImGuiSelectableFlags_DrawFillAvailWidth | (enabled ? 0 : ImGuiSelectableFlags_Disabled), ImVec2(w, 0.0f));
    if (shortcut_size.x > 0.0f)
    {
        PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
        RenderText(pos + ImVec2(window->MenuColumns.Pos[1] + extra_w, 0.0f), shortcut, NULL, false);
        PopStyleColor();
    }

    if (selected)
        RenderCheckMark(pos + ImVec2(window->MenuColumns.Pos[2] + extra_w + g.FontSize * (0.20f+0.200f), g.FontSize * 0.134f * 0.5f), GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled), g.FontSize  * 0.866f);

    return pressed;
}

bool ImGui::MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled)
{
    if (MenuItem(label, shortcut, p_selected ? *p_selected : false, enabled))
    {
        if (p_selected)
            *p_selected = !*p_selected;
        return true;
    }
    return false;
}

bool ImGui::BeginMainMenuBar()
{
    ImGuiContext& g = *GImGui;
    SetNextWindowPos(ImVec2(0.0f, 0.0f));
    SetNextWindowSize(ImVec2(g.IO.DisplaySize.x, g.FontBaseSize + g.Style.FramePadding.y * 2.0f));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0,0));
    if (!Begin("##MainMenuBar", NULL, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_MenuBar)
        || !BeginMenuBar())
    {
        End();
        PopStyleVar(2);
        return false;
    }
    g.CurrentWindow->DC.MenuBarOffsetX += g.Style.DisplaySafeAreaPadding.x;
    return true;
}

void ImGui::EndMainMenuBar()
{
    EndMenuBar();
    End();
    PopStyleVar(2);
}

bool ImGui::BeginMenuBar()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    if (!(window->Flags & ImGuiWindowFlags_MenuBar))
        return false;

    IM_ASSERT(!window->DC.MenuBarAppending);
    BeginGroup(); // Save position
    PushID("##menubar");
    ImRect rect = window->MenuBarRect();
    PushClipRect(ImVec2(ImFloor(rect.Min.x+0.5f), ImFloor(rect.Min.y + window->BorderSize + 0.5f)), ImVec2(ImFloor(rect.Max.x+0.5f), ImFloor(rect.Max.y+0.5f)), false);
    window->DC.CursorPos = ImVec2(rect.Min.x + window->DC.MenuBarOffsetX, rect.Min.y);// + g.Style.FramePadding.y);
    window->DC.LayoutType = ImGuiLayoutType_Horizontal;
    window->DC.MenuBarAppending = true;
    AlignTextToFramePadding();
    return true;
}

void ImGui::EndMenuBar()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    IM_ASSERT(window->Flags & ImGuiWindowFlags_MenuBar);
    IM_ASSERT(window->DC.MenuBarAppending);
    PopClipRect();
    PopID();
    window->DC.MenuBarOffsetX = window->DC.CursorPos.x - window->MenuBarRect().Min.x;
    window->DC.GroupStack.back().AdvanceCursor = false;
    EndGroup();
    window->DC.LayoutType = ImGuiLayoutType_Vertical;
    window->DC.MenuBarAppending = false;
}

bool ImGui::BeginMenu(const char* label, bool enabled)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    ImVec2 label_size = CalcTextSize(label, NULL, true);

    bool pressed;
    bool menu_is_open = IsPopupOpen(id);
    bool menuset_is_open = !(window->Flags & ImGuiWindowFlags_Popup) && (g.OpenPopupStack.Size > g.CurrentPopupStack.Size && g.OpenPopupStack[g.CurrentPopupStack.Size].ParentMenuSet == window->GetID("##menus"));
    ImGuiWindow* backed_nav_window = g.NavWindow;
    if (menuset_is_open)
        g.NavWindow = window;  // Odd hack to allow hovering across menus of a same menu-set (otherwise we wouldn't be able to hover parent)

    // The reference position stored in popup_pos will be used by Begin() to find a suitable position for the child menu (using FindBestPopupWindowPos).
    ImVec2 popup_pos, pos = window->DC.CursorPos;
    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
    {
        // Menu inside an horizontal menu bar
        // Selectable extend their highlight by half ItemSpacing in each direction.
        popup_pos = ImVec2(pos.x - window->WindowPadding.x, pos.y - style.FramePadding.y + window->MenuBarHeight());
        window->DC.CursorPos.x += (float)(int)(style.ItemSpacing.x * 0.5f);
        PushStyleVar(ImGuiStyleVar_ItemSpacing, style.ItemSpacing * 2.0f);
        float w = label_size.x;
        pressed = Selectable(label, menu_is_open, ImGuiSelectableFlags_Menu | ImGuiSelectableFlags_DontClosePopups | (!enabled ? ImGuiSelectableFlags_Disabled : 0), ImVec2(w, 0.0f));
        PopStyleVar();
        window->DC.CursorPos.x += (float)(int)(style.ItemSpacing.x * (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
    }
    else
    {
        // Menu inside a menu
        popup_pos = ImVec2(pos.x, pos.y - style.WindowPadding.y);
        float w = window->MenuColumns.DeclColumns(label_size.x, 0.0f, (float)(int)(g.FontSize * 1.20f)); // Feedback to next frame
        float extra_w = ImMax(0.0f, GetContentRegionAvail().x - w);
        pressed = Selectable(label, menu_is_open, ImGuiSelectableFlags_Menu | ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_DrawFillAvailWidth | (!enabled ? ImGuiSelectableFlags_Disabled : 0), ImVec2(w, 0.0f));
        if (!enabled) PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
        RenderTriangle(pos + ImVec2(window->MenuColumns.Pos[2] + extra_w + g.FontSize * 0.20f, 0.0f), ImGuiDir_Right);
        if (!enabled) PopStyleColor();
    }

    const bool hovered = enabled && ItemHoverable(window->DC.LastItemRect, id);
    if (menuset_is_open)
        g.NavWindow = backed_nav_window;

    bool want_open = false, want_close = false;
    if (window->DC.LayoutType != ImGuiLayoutType_Horizontal) // (window->Flags & (ImGuiWindowFlags_Popup|ImGuiWindowFlags_ChildMenu))
    {
        // Implement http://bjk5.com/post/44698559168/breaking-down-amazons-mega-dropdown to avoid using timers, so menus feels more reactive.
        bool moving_within_opened_triangle = false;
        if (g.HoveredWindow == window && g.OpenPopupStack.Size > g.CurrentPopupStack.Size && g.OpenPopupStack[g.CurrentPopupStack.Size].ParentWindow == window)
        {
            if (ImGuiWindow* next_window = g.OpenPopupStack[g.CurrentPopupStack.Size].Window)
            {
                ImRect next_window_rect = next_window->Rect();
                ImVec2 ta = g.IO.MousePos - g.IO.MouseDelta;
                ImVec2 tb = (window->Pos.x < next_window->Pos.x) ? next_window_rect.GetTL() : next_window_rect.GetTR();
                ImVec2 tc = (window->Pos.x < next_window->Pos.x) ? next_window_rect.GetBL() : next_window_rect.GetBR();
                float extra = ImClamp(fabsf(ta.x - tb.x) * 0.30f, 5.0f, 30.0f); // add a bit of extra slack.
                ta.x += (window->Pos.x < next_window->Pos.x) ? -0.5f : +0.5f;   // to avoid numerical issues
                tb.y = ta.y + ImMax((tb.y - extra) - ta.y, -100.0f);            // triangle is maximum 200 high to limit the slope and the bias toward large sub-menus // FIXME: Multiply by fb_scale?
                tc.y = ta.y + ImMin((tc.y + extra) - ta.y, +100.0f);
                moving_within_opened_triangle = ImTriangleContainsPoint(ta, tb, tc, g.IO.MousePos);
                //window->DrawList->PushClipRectFullScreen(); window->DrawList->AddTriangleFilled(ta, tb, tc, moving_within_opened_triangle ? IM_COL32(0,128,0,128) : IM_COL32(128,0,0,128)); window->DrawList->PopClipRect(); // Debug
            }
        }

        want_close = (menu_is_open && !hovered && g.HoveredWindow == window && g.HoveredIdPreviousFrame != 0 && g.HoveredIdPreviousFrame != id && !moving_within_opened_triangle);
        want_open = (!menu_is_open && hovered && !moving_within_opened_triangle) || (!menu_is_open && hovered && pressed);
    }
    else
    {
        // Menu bar
        if (menu_is_open && pressed && menuset_is_open) // Click an open menu again to close it
        {
            want_close = true;
            want_open = menu_is_open = false;
        }
        else if (pressed || (hovered && menuset_is_open && !menu_is_open)) // First click to open, then hover to open others
        {
            want_open = true;
        }
    }

    if (!enabled) // explicitly close if an open menu becomes disabled, facilitate users code a lot in pattern such as 'if (BeginMenu("options", has_object)) { ..use object.. }'
        want_close = true;
    if (want_close && IsPopupOpen(id))
        ClosePopupToLevel(GImGui->CurrentPopupStack.Size);

    if (!menu_is_open && want_open && g.OpenPopupStack.Size > g.CurrentPopupStack.Size)
    {
        // Don't recycle same menu level in the same frame, first close the other menu and yield for a frame.
        OpenPopup(label);
        return false;
    }

    menu_is_open |= want_open;
    if (want_open)
        OpenPopup(label);

    if (menu_is_open)
    {
        SetNextWindowPos(popup_pos, ImGuiCond_Always);
        ImGuiWindowFlags flags = ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_AlwaysAutoResize | ((window->Flags & (ImGuiWindowFlags_Popup|ImGuiWindowFlags_ChildMenu)) ? ImGuiWindowFlags_ChildMenu|ImGuiWindowFlags_ChildWindow : ImGuiWindowFlags_ChildMenu);
        menu_is_open = BeginPopupEx(id, flags); // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
    }

    return menu_is_open;
}

void ImGui::EndMenu()
{
    EndPopup();
}

// Note: only access 3 floats if ImGuiColorEditFlags_NoAlpha flag is set.
void ImGui::ColorTooltip(const char* text, const float* col, ImGuiColorEditFlags flags)
{
    ImGuiContext& g = *GImGui;

    int cr = IM_F32_TO_INT8_SAT(col[0]), cg = IM_F32_TO_INT8_SAT(col[1]), cb = IM_F32_TO_INT8_SAT(col[2]), ca = (flags & ImGuiColorEditFlags_NoAlpha) ? 255 : IM_F32_TO_INT8_SAT(col[3]);
    BeginTooltipEx(0, true);
    
    const char* text_end = text ? FindRenderedTextEnd(text, NULL) : text;
    if (text_end > text)
    {
        TextUnformatted(text, text_end);
        Separator();
    }

    ImVec2 sz(g.FontSize * 3, g.FontSize * 3);
    ColorButton("##preview", ImVec4(col[0], col[1], col[2], col[3]), (flags & (ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf)) | ImGuiColorEditFlags_NoTooltip, sz);
    SameLine();
    if (flags & ImGuiColorEditFlags_NoAlpha)
        Text("#%02X%02X%02X\nR: %d, G: %d, B: %d\n(%.3f, %.3f, %.3f)", cr, cg, cb, cr, cg, cb, col[0], col[1], col[2]);
    else
        Text("#%02X%02X%02X%02X\nR:%d, G:%d, B:%d, A:%d\n(%.3f, %.3f, %.3f, %.3f)", cr, cg, cb, ca, cr, cg, cb, ca, col[0], col[1], col[2], col[3]);
    EndTooltip();
}

static inline ImU32 ImAlphaBlendColor(ImU32 col_a, ImU32 col_b)
{
    float t = ((col_b >> IM_COL32_A_SHIFT) & 0xFF) / 255.f;
    int r = ImLerp((int)(col_a >> IM_COL32_R_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_R_SHIFT) & 0xFF, t);
    int g = ImLerp((int)(col_a >> IM_COL32_G_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_G_SHIFT) & 0xFF, t);
    int b = ImLerp((int)(col_a >> IM_COL32_B_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_B_SHIFT) & 0xFF, t);
    return IM_COL32(r, g, b, 0xFF);
}

// NB: This is rather brittle and will show artifact when rounding this enabled if rounded corners overlap multiple cells. Caller currently responsible for avoiding that.
// I spent a non reasonable amount of time trying to getting this right for ColorButton with rounding+anti-aliasing+ImGuiColorEditFlags_HalfAlphaPreview flag + various grid sizes and offsets, and eventually gave up... probably more reasonable to disable rounding alltogether.
void ImGui::RenderColorRectWithAlphaCheckerboard(ImVec2 p_min, ImVec2 p_max, ImU32 col, float grid_step, ImVec2 grid_off, float rounding, int rounding_corners_flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (((col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT) < 0xFF)
    {
        ImU32 col_bg1 = GetColorU32(ImAlphaBlendColor(IM_COL32(204,204,204,255), col));
        ImU32 col_bg2 = GetColorU32(ImAlphaBlendColor(IM_COL32(128,128,128,255), col));
        window->DrawList->AddRectFilled(p_min, p_max, col_bg1, rounding, rounding_corners_flags);

        int yi = 0;
        for (float y = p_min.y + grid_off.y; y < p_max.y; y += grid_step, yi++)
        {
            float y1 = ImClamp(y, p_min.y, p_max.y), y2 = ImMin(y + grid_step, p_max.y);
            if (y2 <= y1)
                continue;
            for (float x = p_min.x + grid_off.x + (yi & 1) * grid_step; x < p_max.x; x += grid_step * 2.0f)
            {
                float x1 = ImClamp(x, p_min.x, p_max.x), x2 = ImMin(x + grid_step, p_max.x);
                if (x2 <= x1)
                    continue;
                int rounding_corners_flags_cell = 0;
                if (y1 <= p_min.y) { if (x1 <= p_min.x) rounding_corners_flags_cell |= ImGuiCorner_TopLeft;    if (x2 >= p_max.x) rounding_corners_flags_cell |= ImGuiCorner_TopRight; }
                if (y2 >= p_max.y) { if (x1 <= p_min.x) rounding_corners_flags_cell |= ImGuiCorner_BotLeft; if (x2 >= p_max.x) rounding_corners_flags_cell |= ImGuiCorner_BotRight; }
                rounding_corners_flags_cell &= rounding_corners_flags;
                window->DrawList->AddRectFilled(ImVec2(x1,y1), ImVec2(x2,y2), col_bg2, rounding_corners_flags_cell ? rounding : 0.0f, rounding_corners_flags_cell);
            }
        }
    }
    else
    {
        window->DrawList->AddRectFilled(p_min, p_max, col, rounding, rounding_corners_flags);
    }
}

void ImGui::SetColorEditOptions(ImGuiColorEditFlags flags)
{
    ImGuiContext& g = *GImGui;
    if ((flags & ImGuiColorEditFlags__InputsMask) == 0)
        flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__InputsMask;
    if ((flags & ImGuiColorEditFlags__DataTypeMask) == 0)
        flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__DataTypeMask;
    if ((flags & ImGuiColorEditFlags__PickerMask) == 0)
        flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__PickerMask;
    IM_ASSERT(ImIsPowerOfTwo((int)(flags & ImGuiColorEditFlags__InputsMask)));   // Check only 1 option is selected
    IM_ASSERT(ImIsPowerOfTwo((int)(flags & ImGuiColorEditFlags__DataTypeMask))); // Check only 1 option is selected
    IM_ASSERT(ImIsPowerOfTwo((int)(flags & ImGuiColorEditFlags__PickerMask)));   // Check only 1 option is selected
    g.ColorEditOptions = flags;
}

bool ImGui::ColorButton(const char* desc_id, const float col[], ImGuiColorEditFlags flags, ImVec2 size)
{
	return ImGui::ColorButton(desc_id, ImColor(col[0], col[1], col[2], col[3]), flags, size);
}

// A little colored square. Return true when clicked.
// FIXME: May want to display/ignore the alpha component in the color display? Yet show it in the tooltip.
// 'desc_id' is not called 'label' because we don't display it next to the button, but only in the tooltip.
bool ImGui::ColorButton(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags, ImVec2 size)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(desc_id);
    float default_size = SmallSquareSize();
    if (size.x == 0.0f)
        size.x = default_size;
    if (size.y == 0.0f)
        size.y = default_size;

	ImVec2 begin_Vec = ImVec2(window->Pos.x + GetContentRegionMax().x - size.x - 1, window->DC.CursorPos.y);
	const ImRect bb(begin_Vec, ImVec2(begin_Vec.x + size.x, begin_Vec.y + size.y));
    ItemSize(bb);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    if (flags & ImGuiColorEditFlags_NoAlpha)
        flags &= ~(ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf);
    
    ImVec4 col_without_alpha(col.x, col.y, col.z, 1.0f);
    float grid_step = ImMin(size.x, size.y) / 2.99f;
    float rounding = ImMin(g.Style.FrameRounding, grid_step * 0.5f);
    ImRect bb_inner = bb;
    float off = -0.75f; // The border (using Col_FrameBg) tends to look off when color is near-opaque and rounding is enabled. This offset seemed like a good middleground to reduce those artefacts.
    bb_inner.Expand(off);
    if ((flags & ImGuiColorEditFlags_AlphaPreviewHalf) && col.w < 1.0f)
    {
        float mid_x = (float)(int)((bb_inner.Min.x + bb_inner.Max.x) * 0.5f + 0.5f);
        RenderColorRectWithAlphaCheckerboard(ImVec2(bb_inner.Min.x + grid_step, bb_inner.Min.y), bb_inner.Max, GetColorU32(col), grid_step, ImVec2(-grid_step + off, off), rounding, ImGuiCorner_TopRight|ImGuiCorner_BotRight);
        window->DrawList->AddRectFilled(bb_inner.Min, ImVec2(mid_x, bb_inner.Max.y), GetColorU32(col_without_alpha), rounding, ImGuiCorner_TopLeft|ImGuiCorner_BotLeft);
    }
    else
    {
        RenderColorRectWithAlphaCheckerboard(bb_inner.Min, bb_inner.Max, GetColorU32((flags & ImGuiColorEditFlags_AlphaPreview) ? col : col_without_alpha), grid_step, ImVec2(off, off), rounding);
    }
    if (window->Flags & ImGuiWindowFlags_ShowBorders)
        RenderFrameBorder(bb.Min, bb.Max, rounding);
    else
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), rounding); // Color button are often in need of some sort of border

    if (hovered && !(flags & ImGuiColorEditFlags_NoTooltip))
        ColorTooltip(desc_id, &col.x, flags & (ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf));

	if (pressed)
		g.IO.MousePos = bb_inner.Max;

    return pressed;
}

bool ImGui::ColorEdit3(const char* label, float col[3], ImGuiColorEditFlags flags)
{
    return ColorEdit4(label, col, flags | ImGuiColorEditFlags_NoAlpha);
}

void ImGui::ColorEditOptionsPopup(const float* col, ImGuiColorEditFlags flags)
{
    bool allow_opt_inputs = !(flags & ImGuiColorEditFlags__InputsMask);
    bool allow_opt_datatype = !(flags & ImGuiColorEditFlags__DataTypeMask);
    if ((!allow_opt_inputs && !allow_opt_datatype) || !BeginPopup("context"))
        return;
    ImGuiContext& g = *GImGui;
    ImGuiColorEditFlags opts = g.ColorEditOptions;
    if (allow_opt_inputs)
    {
        if (RadioButton("RGB", (opts & ImGuiColorEditFlags_RGB) ? 1 : 0)) opts = (opts & ~ImGuiColorEditFlags__InputsMask) | ImGuiColorEditFlags_RGB;
        if (RadioButton("HSV", (opts & ImGuiColorEditFlags_HSV) ? 1 : 0)) opts = (opts & ~ImGuiColorEditFlags__InputsMask) | ImGuiColorEditFlags_HSV;
        if (RadioButton("HEX", (opts & ImGuiColorEditFlags_HEX) ? 1 : 0)) opts = (opts & ~ImGuiColorEditFlags__InputsMask) | ImGuiColorEditFlags_HEX;
    }
    if (allow_opt_datatype)
    {
        if (allow_opt_inputs) Separator();
        if (RadioButton("0..255",     (opts & ImGuiColorEditFlags_Uint8) ? 1 : 0)) opts = (opts & ~ImGuiColorEditFlags__DataTypeMask) | ImGuiColorEditFlags_Uint8;
        if (RadioButton("0.00..1.00", (opts & ImGuiColorEditFlags_Float) ? 1 : 0)) opts = (opts & ~ImGuiColorEditFlags__DataTypeMask) | ImGuiColorEditFlags_Float;
    }

    if (allow_opt_inputs || allow_opt_datatype)
        Separator();
    if (Button("Copy as..", ImVec2(-1,0)))
        OpenPopup("Copy");
    if (BeginPopup("Copy"))
    {
        int cr = IM_F32_TO_INT8_SAT(col[0]), cg = IM_F32_TO_INT8_SAT(col[1]), cb = IM_F32_TO_INT8_SAT(col[2]), ca = (flags & ImGuiColorEditFlags_NoAlpha) ? 255 : IM_F32_TO_INT8_SAT(col[3]);
        char buf[64];
        sprintf(buf, "(%.3ff, %.3ff, %.3ff, %.3ff)", col[0], col[1], col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : col[3]);
        if (Selectable(buf))
            SetClipboardText(buf);
        sprintf(buf, "(%d,%d,%d,%d)", cr, cg, cb, ca);
        if (Selectable(buf))
            SetClipboardText(buf);
        if (flags & ImGuiColorEditFlags_NoAlpha)
            sprintf(buf, "0x%02X%02X%02X", cr, cg, cb);
        else
            sprintf(buf, "0x%02X%02X%02X%02X", cr, cg, cb, ca);
        if (Selectable(buf))
            SetClipboardText(buf);
        EndPopup();
    }

    g.ColorEditOptions = opts;
    EndPopup();
}

static void ColorPickerOptionsPopup(ImGuiColorEditFlags flags, float* ref_col)
{
    bool allow_opt_picker = !(flags & ImGuiColorEditFlags__PickerMask);
    bool allow_opt_alpha_bar = !(flags & ImGuiColorEditFlags_NoAlpha) && !(flags & ImGuiColorEditFlags_AlphaBar);
    if ((!allow_opt_picker && !allow_opt_alpha_bar) || !ImGui::BeginPopup("context"))
        return;
    ImGuiContext& g = *GImGui;
    if (allow_opt_picker)
    {
        ImVec2 picker_size(g.FontSize * 8, ImMax(g.FontSize * 8 - (SmallSquareSize() + g.Style.ItemInnerSpacing.x), 1.0f)); // FIXME: Picker size copied from main picker function
        ImGui::PushItemWidth(picker_size.x);
        for (int picker_type = 0; picker_type < 2; picker_type++)
        {
            // Draw small/thumbnail version of each picker type (over an invisible button for selection)
            if (picker_type > 0) ImGui::Separator();
            ImGui::PushID(picker_type);
            ImGuiColorEditFlags picker_flags = ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_NoOptions|ImGuiColorEditFlags_NoLabel|ImGuiColorEditFlags_NoSidePreview|(flags & ImGuiColorEditFlags_NoAlpha);
            if (picker_type == 0) picker_flags |= ImGuiColorEditFlags_PickerHueBar;
            if (picker_type == 1) picker_flags |= ImGuiColorEditFlags_PickerHueWheel;
            ImVec2 backup_pos = ImGui::GetCursorScreenPos();
            if (ImGui::Selectable("##selectable", false, 0, picker_size)) // By default, Selectable() is closing popup
                g.ColorEditOptions = (g.ColorEditOptions & ~ImGuiColorEditFlags__PickerMask) | (picker_flags & ImGuiColorEditFlags__PickerMask);
            ImGui::SetCursorScreenPos(backup_pos);
            ImVec4 dummy_ref_col;
            memcpy(&dummy_ref_col.x, ref_col, sizeof(float) * (picker_flags & ImGuiColorEditFlags_NoAlpha ? 3 : 4));
            ImGui::ColorPicker4("##dummypicker", &dummy_ref_col.x, picker_flags);
            ImGui::PopID();
        }
        ImGui::PopItemWidth();
    }
    if (allow_opt_alpha_bar)
    {
        if (allow_opt_picker) ImGui::Separator();
        ImGui::CheckboxFlags("Alpha Bar", (unsigned int*)&g.ColorEditOptions, ImGuiColorEditFlags_AlphaBar);
    }
    ImGui::EndPopup();
}

// Edit colors components (each component in 0.0f..1.0f range). 
// See enum ImGuiColorEditFlags_ for available options. e.g. Only access 3 floats if ImGuiColorEditFlags_NoAlpha flag is set.
// With typical options: Left-click on colored square to open color picker. Right-click to open option menu. CTRL-Click over input fields to edit them and TAB to go to next item.
bool ImGui::ColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float square_sz = SmallSquareSize();
    const float w_extra = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
    const float w_items_all = CalcItemWidth() - w_extra;
    const char* label_display_end = FindRenderedTextEnd(label);

    const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
    const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
    const int components = alpha ? 4 : 3;
    const ImGuiColorEditFlags flags_untouched = flags;

    BeginGroup();
    PushID(label);

    // If we're not showing any slider there's no point in doing any HSV conversions
    if (flags & ImGuiColorEditFlags_NoInputs)
        flags = (flags & (~ImGuiColorEditFlags__InputsMask)) | ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_NoOptions;

    // Context menu: display and modify options (before defaults are applied)
    if (!(flags & ImGuiColorEditFlags_NoOptions))
        ColorEditOptionsPopup(col, flags);
 
    // Read stored options
    if (!(flags & ImGuiColorEditFlags__InputsMask))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__InputsMask);
    if (!(flags & ImGuiColorEditFlags__DataTypeMask))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__DataTypeMask);
    if (!(flags & ImGuiColorEditFlags__PickerMask))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__PickerMask);
    flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags__InputsMask | ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask));

    // Convert to the formats we need
    float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
    if (flags & ImGuiColorEditFlags_HSV)
        ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
    int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

    bool value_changed = false;
    bool value_changed_as_float = false;

    if ((flags & (ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_HSV)) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
    {
        // RGB/HSV 0..255 Sliders
        const float w_item_one  = ImMax(1.0f, (float)(int)((w_items_all - (style.ItemInnerSpacing.x) * (components-1)) / (float)components));
        const float w_item_last = ImMax(1.0f, (float)(int)(w_items_all - (w_item_one + style.ItemInnerSpacing.x) * (components-1)));

        const bool hide_prefix = (w_item_one <= CalcTextSize((flags & ImGuiColorEditFlags_Float) ? "M:0.000" : "M:000").x);
        const char* ids[4] = { "##X", "##Y", "##Z", "##W" };
        const char* fmt_table_int[3][4] =
        {
            {   "%3.0f",   "%3.0f",   "%3.0f",   "%3.0f" }, // Short display
            { "R:%3.0f", "G:%3.0f", "B:%3.0f", "A:%3.0f" }, // Long display for RGBA
            { "H:%3.0f", "S:%3.0f", "V:%3.0f", "A:%3.0f" }  // Long display for HSVA
        };
        const char* fmt_table_float[3][4] =
        {
            {   "%0.3f",   "%0.3f",   "%0.3f",   "%0.3f" }, // Short display
            { "R:%0.3f", "G:%0.3f", "B:%0.3f", "A:%0.3f" }, // Long display for RGBA
            { "H:%0.3f", "S:%0.3f", "V:%0.3f", "A:%0.3f" }  // Long display for HSVA
        };
        const int fmt_idx = hide_prefix ? 0 : (flags & ImGuiColorEditFlags_HSV) ? 2 : 1;

        PushItemWidth(w_item_one);
        for (int n = 0; n < components; n++)
        {
            if (n > 0)
                SameLine(0, style.ItemInnerSpacing.x);
            if (n + 1 == components)
                PushItemWidth(w_item_last);
            if (flags & ImGuiColorEditFlags_Float)
                value_changed |= value_changed_as_float |= DragFloat(ids[n], &f[n], 1.0f/255.0f, 0.0f, hdr ? 0.0f : 1.0f, fmt_table_float[fmt_idx][n]);
            else
                value_changed |= DragInt(ids[n], &i[n], 1.0f, 0, hdr ? 0 : 255, fmt_table_int[fmt_idx][n]);
            if (!(flags & ImGuiColorEditFlags_NoOptions) && IsItemHovered() && IsMouseClicked(1))
                OpenPopup("context");
        }
        PopItemWidth();
        PopItemWidth();
    }
    else if ((flags & ImGuiColorEditFlags_HEX) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
    {
        // RGB Hexadecimal Input
        char buf[64];
        if (alpha)
            ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X%02X", ImClamp(i[0],0,255), ImClamp(i[1],0,255), ImClamp(i[2],0,255), ImClamp(i[3],0,255));
        else
            ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X", ImClamp(i[0],0,255), ImClamp(i[1],0,255), ImClamp(i[2],0,255));
        PushItemWidth(w_items_all);
        if (InputText("##Text", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
        {
            value_changed |= true;
            char* p = buf;
            while (*p == '#' || ImCharIsSpace(*p))
                p++;
            i[0] = i[1] = i[2] = i[3] = 0;
            if (alpha)
                sscanf(p, "%02X%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2], (unsigned int*)&i[3]); // Treat at unsigned (%X is unsigned)
            else
                sscanf(p, "%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2]);
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions) && IsItemHovered() && IsMouseClicked(1))
            OpenPopup("context");
        PopItemWidth();
    }

    bool picker_active = false;
    if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
    {
        if (!(flags & ImGuiColorEditFlags_NoInputs))
            SameLine(0, style.ItemInnerSpacing.x);

        const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
        if (ColorButton("##ColorButton", col_v4, flags))
        {
            if (!(flags & ImGuiColorEditFlags_NoPicker))
            {
                // Store current color and open a picker
                g.ColorPickerRef = col_v4;
                OpenPopup("picker");
                SetNextWindowPos(window->DC.LastItemRect.GetBL() + ImVec2(-1,style.ItemSpacing.y));
            }
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions) && IsItemHovered() && IsMouseClicked(1))
            OpenPopup("context");

        if (BeginPopup("picker"))
        {
            picker_active = true;
            if (label != label_display_end)
            {
                TextUnformatted(label, label_display_end);
                Separator();
            }
            ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
            ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags__InputsMask | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
            PushItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
            value_changed |= ColorPicker4("##picker", col, picker_flags, &g.ColorPickerRef.x);
            PopItemWidth();
            EndPopup();
        }
    }

    if (label != label_display_end && !(flags & ImGuiColorEditFlags_NoLabel))
    {
        SameLine(0, style.ItemInnerSpacing.x);
        TextUnformatted(label, label_display_end);
    }

    // Convert back
    if (!picker_active)
    {
        if (!value_changed_as_float) 
            for (int n = 0; n < 4; n++)
                f[n] = i[n] / 255.0f;
        if (flags & ImGuiColorEditFlags_HSV)
            ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
        if (value_changed)
        {
            col[0] = f[0];
            col[1] = f[1];
            col[2] = f[2];
            if (alpha)
                col[3] = f[3];
        }
    }

    PopID();
    EndGroup();

    return value_changed;
}

bool ImGui::ColorPicker3(const char* label, float col[3], ImGuiColorEditFlags flags)
{
    float col4[4] = { col[0], col[1], col[2], 1.0f };
    if (!ColorPicker4(label, col4, flags | ImGuiColorEditFlags_NoAlpha))
        return false;
    col[0] = col4[0]; col[1] = col4[1]; col[2] = col4[2];
    return true;
}

// 'pos' is position of the arrow tip. half_sz.x is length from base to tip. half_sz.y is length on each side.
static void RenderArrow(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, ImGuiDir direction, ImU32 col)
{
    switch (direction)
    {
    case ImGuiDir_Left:  draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Right: draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_Up:    draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Down:  draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), pos, col); return;
    default: return; // Fix warning for ImGuiDir_None
    }
}

static void RenderArrowsForVerticalBar(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, float bar_w)
{
    RenderArrow(draw_list, ImVec2(pos.x + half_sz.x + 1,         pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Right, IM_COL32_BLACK);
    RenderArrow(draw_list, ImVec2(pos.x + half_sz.x,             pos.y), half_sz,                              ImGuiDir_Right, IM_COL32_WHITE);
    RenderArrow(draw_list, ImVec2(pos.x + bar_w - half_sz.x - 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Left,  IM_COL32_BLACK);
    RenderArrow(draw_list, ImVec2(pos.x + bar_w - half_sz.x,     pos.y), half_sz,                              ImGuiDir_Left,  IM_COL32_WHITE);
}

// ColorPicker
// Note: only access 3 floats if ImGuiColorEditFlags_NoAlpha flag is set.
// FIXME: we adjust the big color square height based on item width, which may cause a flickering feedback loop (if automatic height makes a vertical scrollbar appears, affecting automatic width..) 
bool ImGui::ColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags, const float* ref_col)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    ImDrawList* draw_list = window->DrawList;

    ImGuiStyle& style = g.Style;
    ImGuiIO& io = g.IO;

    PushID(label);
    BeginGroup();

    if (!(flags & ImGuiColorEditFlags_NoSidePreview))
        flags |= ImGuiColorEditFlags_NoSmallPreview;

    // Context menu: display and store options.
    if (!(flags & ImGuiColorEditFlags_NoOptions))
        ColorPickerOptionsPopup(flags, col);

    // Read stored options
    if (!(flags & ImGuiColorEditFlags__PickerMask))
        flags |= ((g.ColorEditOptions & ImGuiColorEditFlags__PickerMask) ? g.ColorEditOptions : ImGuiColorEditFlags__OptionsDefault) & ImGuiColorEditFlags__PickerMask; 
    IM_ASSERT(ImIsPowerOfTwo((int)(flags & ImGuiColorEditFlags__PickerMask))); // Check that only 1 is selected
    if (!(flags & ImGuiColorEditFlags_NoOptions))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_AlphaBar);

    // Setup
    bool alpha_bar = (flags & ImGuiColorEditFlags_AlphaBar) && !(flags & ImGuiColorEditFlags_NoAlpha);
    ImVec2 picker_pos = window->DC.CursorPos;
    float square_sz = SmallSquareSize();
    float bars_width = square_sz; // Arbitrary smallish width of Hue/Alpha picking bars
    float sv_picker_size = ImMax(bars_width * 1, CalcItemWidth() - (alpha_bar ? 2 : 1) * (bars_width + style.ItemInnerSpacing.x)); // Saturation/Value picking box
    float bar0_pos_x = picker_pos.x + sv_picker_size + style.ItemInnerSpacing.x;
    float bar1_pos_x = bar0_pos_x + bars_width + style.ItemInnerSpacing.x;
    float bars_triangles_half_sz = (float)(int)(bars_width * 0.20f);

    float wheel_thickness = sv_picker_size * 0.08f;
    float wheel_r_outer = sv_picker_size * 0.50f;
    float wheel_r_inner = wheel_r_outer - wheel_thickness;
    ImVec2 wheel_center(picker_pos.x + (sv_picker_size + bars_width)*0.5f, picker_pos.y + sv_picker_size*0.5f);
    
    // Note: the triangle is displayed rotated with triangle_pa pointing to Hue, but most coordinates stays unrotated for logic.
    float triangle_r = wheel_r_inner - (int)(sv_picker_size * 0.027f);
    ImVec2 triangle_pa = ImVec2(triangle_r, 0.0f); // Hue point.
    ImVec2 triangle_pb = ImVec2(triangle_r * -0.5f, triangle_r * -0.866025f); // Black point.
    ImVec2 triangle_pc = ImVec2(triangle_r * -0.5f, triangle_r * +0.866025f); // White point.

    float H,S,V;
    ColorConvertRGBtoHSV(col[0], col[1], col[2], H, S, V);

    bool value_changed = false, value_changed_h = false, value_changed_sv = false;

    if (flags & ImGuiColorEditFlags_PickerHueWheel)
    {
        // Hue wheel + SV triangle logic
        InvisibleButton("hsv", ImVec2(sv_picker_size + style.ItemInnerSpacing.x + bars_width, sv_picker_size));
        if (IsItemActive())
        {
            ImVec2 initial_off = g.IO.MouseClickedPos[0] - wheel_center;
            ImVec2 current_off = g.IO.MousePos - wheel_center;
            float initial_dist2 = ImLengthSqr(initial_off);
            if (initial_dist2 >= (wheel_r_inner-1)*(wheel_r_inner-1) && initial_dist2 <= (wheel_r_outer+1)*(wheel_r_outer+1))
            {
                // Interactive with Hue wheel
                H = atan2f(current_off.y, current_off.x) / IM_PI*0.5f;
                if (H < 0.0f)
                    H += 1.0f;
                value_changed = value_changed_h = true;
            }
            float cos_hue_angle = cosf(-H * 2.0f * IM_PI);
            float sin_hue_angle = sinf(-H * 2.0f * IM_PI);
            if (ImTriangleContainsPoint(triangle_pa, triangle_pb, triangle_pc, ImRotate(initial_off, cos_hue_angle, sin_hue_angle)))
            {
                // Interacting with SV triangle
                ImVec2 current_off_unrotated = ImRotate(current_off, cos_hue_angle, sin_hue_angle);
                if (!ImTriangleContainsPoint(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated))
                    current_off_unrotated = ImTriangleClosestPoint(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated);
                float uu, vv, ww;
                ImTriangleBarycentricCoords(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated, uu, vv, ww);
                V = ImClamp(1.0f - vv, 0.0001f, 1.0f);
                S = ImClamp(uu / V, 0.0001f, 1.0f);
                value_changed = value_changed_sv = true;
            }
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions) && IsItemHovered() && IsMouseClicked(1))
            OpenPopup("context");
    }
    else if (flags & ImGuiColorEditFlags_PickerHueBar)
    {
        // SV rectangle logic
        InvisibleButton("sv", ImVec2(sv_picker_size, sv_picker_size));
        if (IsItemActive())
        {
            S = ImSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size-1));
            V = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size-1));
            value_changed = value_changed_sv = true;
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions) && IsItemHovered() && IsMouseClicked(1))
            OpenPopup("context");

        // Hue bar logic
        SetCursorScreenPos(ImVec2(bar0_pos_x, picker_pos.y));
        InvisibleButton("hue", ImVec2(bars_width, sv_picker_size));
        if (IsItemActive())
        {
            H = ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size-1));
            value_changed = value_changed_h = true;
        }
    }

    // Alpha bar logic
    if (alpha_bar)
    {
        SetCursorScreenPos(ImVec2(bar1_pos_x, picker_pos.y));
        InvisibleButton("alpha", ImVec2(bars_width, sv_picker_size));
        if (IsItemActive())
        {
            col[3] = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size-1));
            value_changed = true;
        }
    }

    if (!(flags & ImGuiColorEditFlags_NoSidePreview))
    {
        SameLine(0, style.ItemInnerSpacing.x);
        BeginGroup();
    }

    if (!(flags & ImGuiColorEditFlags_NoLabel))
    {
        const char* label_display_end = FindRenderedTextEnd(label);
        if (label != label_display_end)
        {
            if ((flags & ImGuiColorEditFlags_NoSidePreview))
                SameLine(0, style.ItemInnerSpacing.x);
            TextUnformatted(label, label_display_end);
        }
    }

    if (!(flags & ImGuiColorEditFlags_NoSidePreview))
    {
        ImVec4 col_v4(col[0], col[1], col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : col[3]);
        if ((flags & ImGuiColorEditFlags_NoLabel))
            Text("Current");
        ColorButton("##current", col_v4, (flags & (ImGuiColorEditFlags_HDR|ImGuiColorEditFlags_AlphaPreview|ImGuiColorEditFlags_AlphaPreviewHalf|ImGuiColorEditFlags_NoTooltip)), ImVec2(square_sz * 3, square_sz * 2));
        if (ref_col != NULL)
        {
            Text("Original");
            ImVec4 ref_col_v4(ref_col[0], ref_col[1], ref_col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : ref_col[3]);
            if (ColorButton("##original", ref_col_v4, (flags & (ImGuiColorEditFlags_HDR|ImGuiColorEditFlags_AlphaPreview|ImGuiColorEditFlags_AlphaPreviewHalf|ImGuiColorEditFlags_NoTooltip)), ImVec2(square_sz * 3, square_sz * 2)))
            {
                memcpy(col, ref_col, ((flags & ImGuiColorEditFlags_NoAlpha) ? 3 : 4) * sizeof(float));
                value_changed = true;
            }
        }
        EndGroup();
    }

    // Convert back color to RGB
    if (value_changed_h || value_changed_sv)
        ColorConvertHSVtoRGB(H >= 1.0f ? H - 10 * 1e-6f : H, S > 0.0f ? S : 10*1e-6f, V > 0.0f ? V : 1e-6f, col[0], col[1], col[2]);

    // R,G,B and H,S,V slider color editor
    if ((flags & ImGuiColorEditFlags_NoInputs) == 0)
    {
        PushItemWidth((alpha_bar ? bar1_pos_x : bar0_pos_x) + bars_width - picker_pos.x);
        ImGuiColorEditFlags sub_flags_to_forward = ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf;
        ImGuiColorEditFlags sub_flags = (flags & sub_flags_to_forward) | ImGuiColorEditFlags_NoPicker;
        if (flags & ImGuiColorEditFlags_RGB || (flags & ImGuiColorEditFlags__InputsMask) == 0)
            value_changed |= ColorEdit4("##rgb", col, sub_flags | ImGuiColorEditFlags_RGB);
        if (flags & ImGuiColorEditFlags_HSV || (flags & ImGuiColorEditFlags__InputsMask) == 0)
            value_changed |= ColorEdit4("##hsv", col, sub_flags | ImGuiColorEditFlags_HSV);
        if (flags & ImGuiColorEditFlags_HEX || (flags & ImGuiColorEditFlags__InputsMask) == 0)
            value_changed |= ColorEdit4("##hex", col, sub_flags | ImGuiColorEditFlags_HEX);
        PopItemWidth();
    }

    // Try to cancel hue wrap (after ColorEdit), if any
    if (value_changed)
    {
        float new_H, new_S, new_V;
        ColorConvertRGBtoHSV(col[0], col[1], col[2], new_H, new_S, new_V);
        if (new_H <= 0 && H > 0) 
        {
            if (new_V <= 0 && V != new_V)
                ColorConvertHSVtoRGB(H, S, new_V <= 0 ? V * 0.5f : new_V, col[0], col[1], col[2]);
            else if (new_S <= 0)
                ColorConvertHSVtoRGB(H, new_S <= 0 ? S * 0.5f : new_S, new_V, col[0], col[1], col[2]);
        }
    }

    ImVec4 hue_color_f(1, 1, 1, 1); ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
    ImU32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);
    ImU32 col32_no_alpha = ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 1.0f));

    const ImU32 hue_colors[6+1] = { IM_COL32(255,0,0,255), IM_COL32(255,255,0,255), IM_COL32(0,255,0,255), IM_COL32(0,255,255,255), IM_COL32(0,0,255,255), IM_COL32(255,0,255,255), IM_COL32(255,0,0,255) };
    ImVec2 sv_cursor_pos;
    
    if (flags & ImGuiColorEditFlags_PickerHueWheel)
    {
        // Render Hue Wheel
        const float aeps = 1.5f / wheel_r_outer; // Half a pixel arc length in radians (2pi cancels out).
        const int segment_per_arc = ImMax(4, (int)wheel_r_outer / 12);
        for (int n = 0; n < 6; n++)
        {
            const float a0 = (n)     /6.0f * 2.0f * IM_PI - aeps;
            const float a1 = (n+1.0f)/6.0f * 2.0f * IM_PI + aeps;
            int vert_start_idx = draw_list->_VtxCurrentIdx;
            draw_list->PathArcTo(wheel_center, (wheel_r_inner + wheel_r_outer)*0.5f, a0, a1, segment_per_arc);
            draw_list->PathStroke(IM_COL32_WHITE, false, wheel_thickness);

            // Paint colors over existing vertices
            ImVec2 gradient_p0(wheel_center.x + cosf(a0) * wheel_r_inner, wheel_center.y + sinf(a0) * wheel_r_inner);
            ImVec2 gradient_p1(wheel_center.x + cosf(a1) * wheel_r_inner, wheel_center.y + sinf(a1) * wheel_r_inner);
            ShadeVertsLinearColorGradientKeepAlpha(draw_list->_VtxWritePtr - (draw_list->_VtxCurrentIdx - vert_start_idx), draw_list->_VtxWritePtr, gradient_p0, gradient_p1, hue_colors[n], hue_colors[n+1]);
        }

        // Render Cursor + preview on Hue Wheel
        float cos_hue_angle = cosf(H * 2.0f * IM_PI);
        float sin_hue_angle = sinf(H * 2.0f * IM_PI);
        ImVec2 hue_cursor_pos(wheel_center.x + cos_hue_angle * (wheel_r_inner+wheel_r_outer)*0.5f, wheel_center.y + sin_hue_angle * (wheel_r_inner+wheel_r_outer)*0.5f);
        float hue_cursor_rad = value_changed_h ? wheel_thickness * 0.65f : wheel_thickness * 0.55f;
        int hue_cursor_segments = ImClamp((int)(hue_cursor_rad / 1.4f), 9, 32);
        draw_list->AddCircleFilled(hue_cursor_pos, hue_cursor_rad, hue_color32, hue_cursor_segments);
        draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad+1, IM_COL32(128,128,128,255), hue_cursor_segments);
        draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad, IM_COL32_WHITE, hue_cursor_segments);

        // Render SV triangle (rotated according to hue)
        ImVec2 tra = wheel_center + ImRotate(triangle_pa, cos_hue_angle, sin_hue_angle);
        ImVec2 trb = wheel_center + ImRotate(triangle_pb, cos_hue_angle, sin_hue_angle);
        ImVec2 trc = wheel_center + ImRotate(triangle_pc, cos_hue_angle, sin_hue_angle);
        ImVec2 uv_white = g.FontTexUvWhitePixel;
        draw_list->PrimReserve(6, 6);
        draw_list->PrimVtx(tra, uv_white, hue_color32);
        draw_list->PrimVtx(trb, uv_white, hue_color32);
        draw_list->PrimVtx(trc, uv_white, IM_COL32_WHITE);
        draw_list->PrimVtx(tra, uv_white, IM_COL32_BLACK_TRANS);
        draw_list->PrimVtx(trb, uv_white, IM_COL32_BLACK);
        draw_list->PrimVtx(trc, uv_white, IM_COL32_BLACK_TRANS);
        draw_list->AddTriangle(tra, trb, trc, IM_COL32(128,128,128,255), 1.5f);
        sv_cursor_pos = ImLerp(ImLerp(trc, tra, ImSaturate(S)), trb, ImSaturate(1 - V));
    }
    else if (flags & ImGuiColorEditFlags_PickerHueBar)
    {
        // Render SV Square
        draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size,sv_picker_size), IM_COL32_WHITE, hue_color32, hue_color32, IM_COL32_WHITE);
        draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size,sv_picker_size), IM_COL32_BLACK_TRANS, IM_COL32_BLACK_TRANS, IM_COL32_BLACK, IM_COL32_BLACK);
        RenderFrameBorder(picker_pos, picker_pos + ImVec2(sv_picker_size,sv_picker_size), 0.0f);
        sv_cursor_pos.x = ImClamp((float)(int)(picker_pos.x + ImSaturate(S)     * sv_picker_size + 0.5f), picker_pos.x + 2, picker_pos.x + sv_picker_size - 2); // Sneakily prevent the circle to stick out too much
        sv_cursor_pos.y = ImClamp((float)(int)(picker_pos.y + ImSaturate(1 - V) * sv_picker_size + 0.5f), picker_pos.y + 2, picker_pos.y + sv_picker_size - 2);

        // Render Hue Bar
        for (int i = 0; i < 6; ++i)
            draw_list->AddRectFilledMultiColor(ImVec2(bar0_pos_x, picker_pos.y + i * (sv_picker_size / 6)), ImVec2(bar0_pos_x + bars_width, picker_pos.y + (i + 1) * (sv_picker_size / 6)), hue_colors[i], hue_colors[i], hue_colors[i + 1], hue_colors[i + 1]);
        float bar0_line_y = (float)(int)(picker_pos.y + H * sv_picker_size + 0.5f);
        RenderFrameBorder(ImVec2(bar0_pos_x, picker_pos.y), ImVec2(bar0_pos_x + bars_width, picker_pos.y + sv_picker_size), 0.0f);
        RenderArrowsForVerticalBar(draw_list, ImVec2(bar0_pos_x - 1, bar0_line_y), ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width + 2.0f);
    }

    // Render cursor/preview circle (clamp S/V within 0..1 range because floating points colors may lead HSV values to be out of range)
    float sv_cursor_rad = value_changed_sv ? 10.0f : 6.0f;
    draw_list->AddCircleFilled(sv_cursor_pos, sv_cursor_rad, col32_no_alpha, 12);
    draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad+1, IM_COL32(128,128,128,255), 12);
    draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad, IM_COL32_WHITE, 12);

    // Render alpha bar
    if (alpha_bar)
    {
        float alpha = ImSaturate(col[3]);
        ImRect bar1_bb(bar1_pos_x, picker_pos.y, bar1_pos_x + bars_width, picker_pos.y + sv_picker_size);
        RenderColorRectWithAlphaCheckerboard(bar1_bb.Min, bar1_bb.Max, IM_COL32(0,0,0,0), bar1_bb.GetWidth() / 2.0f, ImVec2(0.0f, 0.0f));
        draw_list->AddRectFilledMultiColor(bar1_bb.Min, bar1_bb.Max, col32_no_alpha, col32_no_alpha, col32_no_alpha & ~IM_COL32_A_MASK, col32_no_alpha & ~IM_COL32_A_MASK);
        float bar1_line_y = (float)(int)(picker_pos.y + (1.0f - alpha) * sv_picker_size + 0.5f);
        RenderFrameBorder(bar1_bb.Min, bar1_bb.Max, 0.0f);
        RenderArrowsForVerticalBar(draw_list, ImVec2(bar1_pos_x - 1, bar1_line_y), ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width + 2.0f);
    }

    EndGroup();
    PopID();

    return value_changed;
}

// Horizontal separating line.
void ImGui::Separator()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;
    ImGuiContext& g = *GImGui;

    ImGuiWindowFlags flags = 0;
    if ((flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical)) == 0)
        flags |= (window->DC.LayoutType == ImGuiLayoutType_Horizontal) ? ImGuiSeparatorFlags_Vertical : ImGuiSeparatorFlags_Horizontal;
    IM_ASSERT(ImIsPowerOfTwo((int)(flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical))));   // Check that only 1 option is selected
    if (flags & ImGuiSeparatorFlags_Vertical)
    {
        VerticalSeparator();
        return;
    }

    // Horizontal Separator
    if (window->DC.ColumnsCount > 1)
        PopClipRect();

    float x1 = window->Pos.x;
    float x2 = window->Pos.x + window->Size.x;
    if (!window->DC.GroupStack.empty())
        x1 += window->DC.IndentX;

    const ImRect bb(ImVec2(x1, window->DC.CursorPos.y), ImVec2(x2, window->DC.CursorPos.y+1.0f));
    ItemSize(ImVec2(0.0f, 0.0f)); // NB: we don't provide our width so that it doesn't get feed back into AutoFit, we don't provide height to not alter layout.
    if (!ItemAdd(bb, 0))
    {
        if (window->DC.ColumnsCount > 1)
            PushColumnClipRect();
        return;
    }

    window->DrawList->AddLine(bb.Min, ImVec2(bb.Max.x,bb.Min.y), GetColorU32(ImGuiCol_Separator));

    if (g.LogEnabled)
            LogRenderedText(NULL, IM_NEWLINE "--------------------------------");

    if (window->DC.ColumnsCount > 1)
    {
        PushColumnClipRect();
        window->DC.ColumnsCellMinY = window->DC.CursorPos.y;
    }
}

void ImGui::VerticalSeparator()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;
    ImGuiContext& g = *GImGui;

    float y1 = window->DC.CursorPos.y;
    float y2 = window->DC.CursorPos.y + window->DC.CurrentLineHeight; 
    const ImRect bb(ImVec2(window->DC.CursorPos.x, y1), ImVec2(window->DC.CursorPos.x + 1.0f, y2));
    ItemSize(ImVec2(bb.GetWidth(), 0.0f));
    if (!ItemAdd(bb, 0))
        return;

    window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x, bb.Max.y), GetColorU32(ImGuiCol_Separator));
    if (g.LogEnabled)
        LogText(" |");
}

void ImGui::Spacing()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;
    ItemSize(ImVec2(0,0));
}

void ImGui::Dummy(const ImVec2& size)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    ItemSize(bb);
    ItemAdd(bb, 0);
}

bool ImGui::IsRectVisible(const ImVec2& size)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->ClipRect.Overlaps(ImRect(window->DC.CursorPos, window->DC.CursorPos + size));
}

bool ImGui::IsRectVisible(const ImVec2& rect_min, const ImVec2& rect_max)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->ClipRect.Overlaps(ImRect(rect_min, rect_max));
}

// Lock horizontal starting position + capture group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)
void ImGui::BeginGroup()
{
    ImGuiWindow* window = GetCurrentWindow();

    window->DC.GroupStack.resize(window->DC.GroupStack.Size + 1);
    ImGuiGroupData& group_data = window->DC.GroupStack.back();
    group_data.BackupCursorPos = window->DC.CursorPos;
    group_data.BackupCursorMaxPos = window->DC.CursorMaxPos;
    group_data.BackupIndentX = window->DC.IndentX;
    group_data.BackupGroupOffsetX = window->DC.GroupOffsetX;
    group_data.BackupCurrentLineHeight = window->DC.CurrentLineHeight;
    group_data.BackupCurrentLineTextBaseOffset = window->DC.CurrentLineTextBaseOffset;
    group_data.BackupLogLinePosY = window->DC.LogLinePosY;
    group_data.BackupActiveIdIsAlive = GImGui->ActiveIdIsAlive;
    group_data.AdvanceCursor = true;

    window->DC.GroupOffsetX = window->DC.CursorPos.x - window->Pos.x - window->DC.ColumnsOffsetX;
    window->DC.IndentX = window->DC.GroupOffsetX;
    window->DC.CursorMaxPos = window->DC.CursorPos;
    window->DC.CurrentLineHeight = 0.0f;
    window->DC.LogLinePosY = window->DC.CursorPos.y - 9999.0f;
}

void ImGui::EndGroup()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();

    IM_ASSERT(!window->DC.GroupStack.empty());    // Mismatched BeginGroup()/EndGroup() calls

    ImGuiGroupData& group_data = window->DC.GroupStack.back();

    ImRect group_bb(group_data.BackupCursorPos, window->DC.CursorMaxPos);
    group_bb.Max.y -= g.Style.ItemSpacing.y;      // Cancel out last vertical spacing because we are adding one ourselves.
    group_bb.Max = ImMax(group_bb.Min, group_bb.Max);

    window->DC.CursorPos = group_data.BackupCursorPos;
    window->DC.CursorMaxPos = ImMax(group_data.BackupCursorMaxPos, window->DC.CursorMaxPos);
    window->DC.CurrentLineHeight = group_data.BackupCurrentLineHeight;
    window->DC.CurrentLineTextBaseOffset = group_data.BackupCurrentLineTextBaseOffset;
    window->DC.IndentX = group_data.BackupIndentX;
    window->DC.GroupOffsetX = group_data.BackupGroupOffsetX;
    window->DC.LogLinePosY = window->DC.CursorPos.y - 9999.0f;

    if (group_data.AdvanceCursor)
    {
        window->DC.CurrentLineTextBaseOffset = ImMax(window->DC.PrevLineTextBaseOffset, group_data.BackupCurrentLineTextBaseOffset);      // FIXME: Incorrect, we should grab the base offset from the *first line* of the group but it is hard to obtain now.
        ItemSize(group_bb.GetSize(), group_data.BackupCurrentLineTextBaseOffset);
        ItemAdd(group_bb, 0);
    }

    // If the current ActiveId was declared within the boundary of our group, we copy it to LastItemId so IsItemActive() will be functional on the entire group.
    // It would be be neater if we replaced window.DC.LastItemId by e.g. 'bool LastItemIsActive', but if you search for LastItemId you'll notice it is only used in that context.
    const bool active_id_within_group = (!group_data.BackupActiveIdIsAlive && g.ActiveIdIsAlive && g.ActiveId && g.ActiveIdWindow->RootWindow == window->RootWindow);
    if (active_id_within_group)
        window->DC.LastItemId = g.ActiveId;
    window->DC.LastItemRect = group_bb;

    window->DC.GroupStack.pop_back();

    //window->DrawList->AddRect(group_bb.Min, group_bb.Max, IM_COL32(255,0,255,255));   // [Debug]
}

// Gets back to previous line and continue with horizontal layout
//      pos_x == 0      : follow right after previous item
//      pos_x != 0      : align to specified x position (relative to window/group left)
//      spacing_w < 0   : use default spacing if pos_x == 0, no spacing if pos_x != 0
//      spacing_w >= 0  : enforce spacing amount
void ImGui::SameLine(float pos_x, float spacing_w)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    if (pos_x != 0.0f)
    {
        if (spacing_w < 0.0f) spacing_w = 0.0f;
        window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + pos_x + spacing_w + window->DC.GroupOffsetX + window->DC.ColumnsOffsetX;
        window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
    }
    else
    {
        if (spacing_w < 0.0f) spacing_w = g.Style.ItemSpacing.x;
        window->DC.CursorPos.x = window->DC.CursorPosPrevLine.x + spacing_w;
        window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
    }
    window->DC.CurrentLineHeight = window->DC.PrevLineHeight;
    window->DC.CurrentLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;
}

void ImGui::NewLine()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiLayoutType backup_layout_type = window->DC.LayoutType;
    window->DC.LayoutType = ImGuiLayoutType_Vertical;
    if (window->DC.CurrentLineHeight > 0.0f)     // In the event that we are on a line with items that is smaller that FontSize high, we will preserve its height.
        ItemSize(ImVec2(0,0));
    else
        ItemSize(ImVec2(0.0f, g.FontSize));
    window->DC.LayoutType = backup_layout_type;
}

void ImGui::NextColumn()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems || window->DC.ColumnsCount <= 1)
        return;

    ImGuiContext& g = *GImGui;
    PopItemWidth();
    PopClipRect();

    window->DC.ColumnsCellMaxY = ImMax(window->DC.ColumnsCellMaxY, window->DC.CursorPos.y);
    if (++window->DC.ColumnsCurrent < window->DC.ColumnsCount)
    {
        // Columns 1+ cancel out IndentX
        window->DC.ColumnsOffsetX = GetColumnOffset(window->DC.ColumnsCurrent) - window->DC.IndentX + g.Style.ItemSpacing.x;
        window->DrawList->ChannelsSetCurrent(window->DC.ColumnsCurrent);
    }
    else
    {
        window->DC.ColumnsCurrent = 0;
        window->DC.ColumnsOffsetX = 0.0f;
        window->DC.ColumnsCellMinY = window->DC.ColumnsCellMaxY;
        window->DrawList->ChannelsSetCurrent(0);
    }
    window->DC.CursorPos.x = (float)(int)(window->Pos.x + window->DC.IndentX + window->DC.ColumnsOffsetX);
    window->DC.CursorPos.y = window->DC.ColumnsCellMinY;
    window->DC.CurrentLineHeight = 0.0f;
    window->DC.CurrentLineTextBaseOffset = 0.0f;

    PushColumnClipRect();
    PushItemWidth(GetColumnWidth() * 0.65f);  // FIXME: Move on columns setup
}

int ImGui::GetColumnIndex()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.ColumnsCurrent;
}

int ImGui::GetColumnsCount()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.ColumnsCount;
}

static float OffsetNormToPixels(ImGuiWindow* window, float offset_norm)
{
    return offset_norm * (window->DC.ColumnsMaxX - window->DC.ColumnsMinX);
}

static float PixelsToOffsetNorm(ImGuiWindow* window, float offset)
{
    return (offset - window->DC.ColumnsMinX) / (window->DC.ColumnsMaxX - window->DC.ColumnsMinX);
}

static float GetDraggedColumnOffset(int column_index)
{
    // Active (dragged) column always follow mouse. The reason we need this is that dragging a column to the right edge of an auto-resizing
    // window creates a feedback loop because we store normalized positions. So while dragging we enforce absolute positioning.
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(column_index > 0); // We cannot drag column 0. If you get this assert you may have a conflict between the ID of your columns and another widgets.
    IM_ASSERT(g.ActiveId == window->DC.ColumnsSetId + ImGuiID(column_index));

    float x = g.IO.MousePos.x - g.ActiveIdClickOffset.x - window->Pos.x;
    x = ImMax(x, ImGui::GetColumnOffset(column_index-1) + g.Style.ColumnsMinSpacing);
    if ((window->DC.ColumnsFlags & ImGuiColumnsFlags_NoPreserveWidths))
        x = ImMin(x, ImGui::GetColumnOffset(column_index+1) - g.Style.ColumnsMinSpacing);

    return x;
}

float ImGui::GetColumnOffset(int column_index)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    if (column_index < 0)
        column_index = window->DC.ColumnsCurrent;

    /*
    if (g.ActiveId)
    {
        ImGuiContext& g = *GImGui;
        const ImGuiID column_id = window->DC.ColumnsSetId + ImGuiID(column_index);
        if (g.ActiveId == column_id)
            return GetDraggedColumnOffset(column_index);
    }
    */

    IM_ASSERT(column_index < window->DC.ColumnsData.Size);
    const float t = window->DC.ColumnsData[column_index].OffsetNorm;
    const float x_offset = ImLerp(window->DC.ColumnsMinX, window->DC.ColumnsMaxX, t);
    return x_offset;
}

void ImGui::SetColumnOffset(int column_index, float offset)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (column_index < 0)
        column_index = window->DC.ColumnsCurrent;

    IM_ASSERT(column_index < window->DC.ColumnsData.Size);

    const bool preserve_width = !(window->DC.ColumnsFlags & ImGuiColumnsFlags_NoPreserveWidths) && (column_index < window->DC.ColumnsCount-1);
    const float width = preserve_width ? GetColumnWidth(column_index) : 0.0f;

    if (!(window->DC.ColumnsFlags & ImGuiColumnsFlags_NoForceWithinWindow))
        offset = ImMin(offset, window->DC.ColumnsMaxX - g.Style.ColumnsMinSpacing * (window->DC.ColumnsCount - column_index));
    const float offset_norm = PixelsToOffsetNorm(window, offset);

    const ImGuiID column_id = window->DC.ColumnsSetId + ImGuiID(column_index);
    window->DC.StateStorage->SetFloat(column_id, offset_norm);
    window->DC.ColumnsData[column_index].OffsetNorm = offset_norm;

    if (preserve_width)
        SetColumnOffset(column_index + 1, offset + ImMax(g.Style.ColumnsMinSpacing, width));
}

float ImGui::GetColumnWidth(int column_index)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    if (column_index < 0)
        column_index = window->DC.ColumnsCurrent;

    return OffsetNormToPixels(window, window->DC.ColumnsData[column_index+1].OffsetNorm - window->DC.ColumnsData[column_index].OffsetNorm);
}

void ImGui::SetColumnWidth(int column_index, float width)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    if (column_index < 0)
        column_index = window->DC.ColumnsCurrent;

    SetColumnOffset(column_index+1, GetColumnOffset(column_index) + width);
}

void ImGui::PushColumnClipRect(int column_index)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    if (column_index < 0)
        column_index = window->DC.ColumnsCurrent;

    PushClipRect(window->DC.ColumnsData[column_index].ClipRect.Min, window->DC.ColumnsData[column_index].ClipRect.Max, false);
}

void ImGui::BeginColumns(const char* id, int columns_count, ImGuiColumnsFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();

    IM_ASSERT(columns_count > 1);
    IM_ASSERT(window->DC.ColumnsCount == 1); // Nested columns are currently not supported

    // Differentiate column ID with an arbitrary prefix for cases where users name their columns set the same as another widget.
    // In addition, when an identifier isn't explicitly provided we include the number of columns in the hash to make it uniquer.
    PushID(0x11223347 + (id ? 0 : columns_count));
    window->DC.ColumnsSetId = window->GetID(id ? id : "columns");
    PopID();

    // Set state for first column
    window->DC.ColumnsCurrent = 0;
    window->DC.ColumnsCount = columns_count;
    window->DC.ColumnsFlags = flags;

    const float content_region_width = (window->SizeContentsExplicit.x != 0.0f) ? (window->SizeContentsExplicit.x) : (window->Size.x -window->ScrollbarSizes.x);
    window->DC.ColumnsMinX = window->DC.IndentX - g.Style.ItemSpacing.x; // Lock our horizontal range
    //window->DC.ColumnsMaxX = content_region_width - window->Scroll.x -((window->Flags & ImGuiWindowFlags_NoScrollbar) ? 0 : g.Style.ScrollbarSize);// - window->WindowPadding().x;
    window->DC.ColumnsMaxX = content_region_width - window->Scroll.x;
    window->DC.ColumnsStartPosY = window->DC.CursorPos.y;
    window->DC.ColumnsStartMaxPosX = window->DC.CursorMaxPos.x;
    window->DC.ColumnsCellMinY = window->DC.ColumnsCellMaxY = window->DC.CursorPos.y;
    window->DC.ColumnsOffsetX = 0.0f;
    window->DC.CursorPos.x = (float)(int)(window->Pos.x + window->DC.IndentX + window->DC.ColumnsOffsetX);

    // Cache column offsets
    window->DC.ColumnsData.resize(columns_count + 1);
    for (int column_index = 0; column_index < columns_count + 1; column_index++)
    {
        const ImGuiID column_id = window->DC.ColumnsSetId + ImGuiID(column_index);
        KeepAliveID(column_id);
        const float default_t = column_index / (float)window->DC.ColumnsCount;
        float t = window->DC.StateStorage->GetFloat(column_id, default_t);
        if (!(window->DC.ColumnsFlags & ImGuiColumnsFlags_NoForceWithinWindow))
            t = ImMin(t, PixelsToOffsetNorm(window, window->DC.ColumnsMaxX - g.Style.ColumnsMinSpacing * (window->DC.ColumnsCount - column_index)));
        window->DC.ColumnsData[column_index].OffsetNorm = t;
    }

    // Cache clipping rectangles
    for (int column_index = 0; column_index < columns_count; column_index++)
    {
        float clip_x1 = ImFloor(0.5f + window->Pos.x + GetColumnOffset(column_index) - 1.0f);
        float clip_x2 = ImFloor(0.5f + window->Pos.x + GetColumnOffset(column_index + 1) - 1.0f);
        window->DC.ColumnsData[column_index].ClipRect = ImRect(clip_x1, -FLT_MAX, clip_x2, +FLT_MAX);
        window->DC.ColumnsData[column_index].ClipRect.ClipWith(window->ClipRect);
    }

    window->DrawList->ChannelsSplit(window->DC.ColumnsCount);
    PushColumnClipRect();
    PushItemWidth(GetColumnWidth() * 0.65f);
}

void ImGui::EndColumns()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    IM_ASSERT(window->DC.ColumnsCount > 1);

    PopItemWidth();
    PopClipRect();
    window->DrawList->ChannelsMerge();

    window->DC.ColumnsCellMaxY = ImMax(window->DC.ColumnsCellMaxY, window->DC.CursorPos.y);
    window->DC.CursorPos.y = window->DC.ColumnsCellMaxY;
    window->DC.CursorMaxPos.x = ImMax(window->DC.ColumnsStartMaxPosX, window->DC.ColumnsMaxX);  // Columns don't grow parent

    // Draw columns borders and handle resize
    if (!(window->DC.ColumnsFlags & ImGuiColumnsFlags_NoBorder) && !window->SkipItems)
    {
        const float y1 = window->DC.ColumnsStartPosY;
        const float y2 = window->DC.CursorPos.y;
        int dragging_column = -1;
        for (int i = 1; i < window->DC.ColumnsCount; i++)
        {
            float x = window->Pos.x + GetColumnOffset(i);
            const ImGuiID column_id = window->DC.ColumnsSetId + ImGuiID(i);
            const float column_w = 4.0f; // Width for interaction
            const ImRect column_rect(ImVec2(x - column_w, y1), ImVec2(x + column_w, y2));
            if (IsClippedEx(column_rect, column_id, false))
                continue;
            
            bool hovered = false, held = false;
            if (!(window->DC.ColumnsFlags & ImGuiColumnsFlags_NoResize))
            {
                ButtonBehavior(column_rect, column_id, &hovered, &held);
                if (hovered || held)
                    g.MouseCursor = ImGuiMouseCursor_ResizeEW;
                if (held && g.ActiveIdIsJustActivated)
                    g.ActiveIdClickOffset.x -= column_w; // Store from center of column line (we used a 8 wide rect for columns clicking). This is used by GetDraggedColumnOffset().
                if (held)
                    dragging_column = i;
            }

            // Draw column
            const ImU32 col = GetColorU32(held ? ImGuiCol_SeparatorActive : hovered ? ImGuiCol_SeparatorHovered : ImGuiCol_Separator);
            const float xi = (float)(int)x;
            window->DrawList->AddLine(ImVec2(xi, y1 + 1.0f), ImVec2(xi, y2), col);
        }

        // Apply dragging after drawing the column lines, so our rendered lines are in sync with how items were displayed during the frame.
        if (dragging_column != -1)
        {
            float x = GetDraggedColumnOffset(dragging_column);
            SetColumnOffset(dragging_column, x);
        }
    }

    window->DC.ColumnsSetId = 0;
    window->DC.ColumnsCurrent = 0;
    window->DC.ColumnsCount = 1;
    window->DC.ColumnsFlags = 0;
    window->DC.ColumnsData.resize(0);
    window->DC.ColumnsOffsetX = 0.0f;
    window->DC.CursorPos.x = (float)(int)(window->Pos.x + window->DC.IndentX + window->DC.ColumnsOffsetX);
}

// [2017/08: This is currently the only public API, while we are working on making BeginColumns/EndColumns user-facing]
void ImGui::Columns(int columns_count, const char* id, bool border)
{
    ImGuiWindow* window = GetCurrentWindow();
    IM_ASSERT(columns_count >= 1);

    if (window->DC.ColumnsCount != columns_count && window->DC.ColumnsCount != 1)
        EndColumns();
    
    ImGuiColumnsFlags flags = (border ? 0 : ImGuiColumnsFlags_NoBorder);
    //flags |= ImGuiColumnsFlags_NoPreserveWidths; // NB: Legacy behavior
    if (columns_count != 1)
        BeginColumns(id, columns_count, flags);
}

void ImGui::Indent(float indent_w)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.IndentX += (indent_w > 0.0f) ? indent_w : g.Style.IndentSpacing;
    window->DC.CursorPos.x = window->Pos.x + window->DC.IndentX + window->DC.ColumnsOffsetX;
}

void ImGui::Unindent(float indent_w)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.IndentX -= (indent_w > 0.0f) ? indent_w : g.Style.IndentSpacing;
    window->DC.CursorPos.x = window->Pos.x + window->DC.IndentX + window->DC.ColumnsOffsetX;
}

void ImGui::TreePush(const char* str_id)
{
    ImGuiWindow* window = GetCurrentWindow();
    Indent();
    window->DC.TreeDepth++;
    PushID(str_id ? str_id : "#TreePush");
}

void ImGui::TreePush(const void* ptr_id)
{
    ImGuiWindow* window = GetCurrentWindow();
    Indent();
    window->DC.TreeDepth++;
    PushID(ptr_id ? ptr_id : (const void*)"#TreePush");
}

void ImGui::TreePushRawID(ImGuiID id)
{
    ImGuiWindow* window = GetCurrentWindow();
    Indent();
    window->DC.TreeDepth++;
    window->IDStack.push_back(id);
}

void ImGui::TreePop()
{
    ImGuiWindow* window = GetCurrentWindow();
    Unindent();
    window->DC.TreeDepth--;
    PopID();
}

void ImGui::Value(const char* prefix, bool b)
{
    Text("%s: %s", prefix, (b ? "true" : "false"));
}

void ImGui::Value(const char* prefix, int v)
{
    Text("%s: %d", prefix, v);
}

void ImGui::Value(const char* prefix, unsigned int v)
{
    Text("%s: %d", prefix, v);
}

void ImGui::Value(const char* prefix, float v, const char* float_format)
{
    if (float_format)
    {
        char fmt[64];
        ImFormatString(fmt, IM_ARRAYSIZE(fmt), "%%s: %s", float_format);
        Text(fmt, prefix, v);
    }
    else
    {
        Text("%s: %.3f", prefix, v);
    }
}

//-----------------------------------------------------------------------------
// PLATFORM DEPENDENT HELPERS
//-----------------------------------------------------------------------------

#if defined(_WIN32) && !defined(_WINDOWS_) && (!defined(IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCS) || !defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCS))
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// Win32 API clipboard implementation
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCS)

#ifdef _MSC_VER
#pragma comment(lib, "user32")
#endif

static const char* GetClipboardTextFn_DefaultImpl(void*)
{
    static ImVector<char> buf_local;
    buf_local.clear();
    if (!OpenClipboard(NULL))
        return NULL;
    HANDLE wbuf_handle = GetClipboardData(CF_UNICODETEXT);
    if (wbuf_handle == NULL)
    {
        CloseClipboard();
        return NULL;
    }
    if (ImWchar* wbuf_global = (ImWchar*)GlobalLock(wbuf_handle))
    {
        int buf_len = ImTextCountUtf8BytesFromStr(wbuf_global, NULL) + 1;
        buf_local.resize(buf_len);
        ImTextStrToUtf8(buf_local.Data, buf_len, wbuf_global, NULL);
    }
    GlobalUnlock(wbuf_handle);
    CloseClipboard();
    return buf_local.Data;
}

static void SetClipboardTextFn_DefaultImpl(void*, const char* text)
{
    if (!OpenClipboard(NULL))
        return;
    const int wbuf_length = ImTextCountCharsFromUtf8(text, NULL) + 1;
    HGLOBAL wbuf_handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wbuf_length * sizeof(ImWchar));
    if (wbuf_handle == NULL)
        return;
    ImWchar* wbuf_global = (ImWchar*)GlobalLock(wbuf_handle);
    ImTextStrFromUtf8(wbuf_global, wbuf_length, text, NULL);
    GlobalUnlock(wbuf_handle);
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, wbuf_handle);
    CloseClipboard();
}

#else

// Local ImGui-only clipboard implementation, if user hasn't defined better clipboard handlers
static const char* GetClipboardTextFn_DefaultImpl(void*)
{
    ImGuiContext& g = *GImGui;
    return g.PrivateClipboard.empty() ? NULL : g.PrivateClipboard.begin();
}

// Local ImGui-only clipboard implementation, if user hasn't defined better clipboard handlers
static void SetClipboardTextFn_DefaultImpl(void*, const char* text)
{
    ImGuiContext& g = *GImGui;
    g.PrivateClipboard.clear();
    const char* text_end = text + strlen(text);
    g.PrivateClipboard.resize((int)(text_end - text) + 1);
    memcpy(&g.PrivateClipboard[0], text, (size_t)(text_end - text));
    g.PrivateClipboard[(int)(text_end - text)] = 0;
}

#endif

// Win32 API IME support (for Asian languages, etc.)
#if defined(_WIN32) && !defined(__GNUC__) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCS)

#include <imm.h>
#ifdef _MSC_VER
#pragma comment(lib, "imm32")
#endif

static void ImeSetInputScreenPosFn_DefaultImpl(int x, int y)
{
    // Notify OS Input Method Editor of text input position
    if (HWND hwnd = (HWND)GImGui->IO.ImeWindowHandle)
        if (HIMC himc = ImmGetContext(hwnd))
        {
            COMPOSITIONFORM cf;
            cf.ptCurrentPos.x = x;
            cf.ptCurrentPos.y = y;
            cf.dwStyle = CFS_FORCE_POSITION;
            ImmSetCompositionWindow(himc, &cf);
        }
}

#else

static void ImeSetInputScreenPosFn_DefaultImpl(int, int) {}

#endif

//-----------------------------------------------------------------------------
// HELP
//-----------------------------------------------------------------------------

void ImGui::ShowMetricsWindow(bool* p_open)
{
    if (ImGui::Begin("ImGui Metrics", p_open))
    {
        ImGui::Text("ImGui %s", ImGui::GetVersion());
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("%d vertices, %d indices (%d triangles)", ImGui::GetIO().MetricsRenderVertices, ImGui::GetIO().MetricsRenderIndices, ImGui::GetIO().MetricsRenderIndices / 3);
        ImGui::Text("%d allocations", ImGui::GetIO().MetricsAllocs);
        static bool show_clip_rects = true;
        ImGui::Checkbox("Show clipping rectangles when hovering an ImDrawCmd", &show_clip_rects);
        ImGui::Separator();

        struct Funcs
        {
            static void NodeDrawList(ImDrawList* draw_list, const char* label)
            {
                bool node_open = ImGui::TreeNode(draw_list, "%s: '%s' %d vtx, %d indices, %d cmds", label, draw_list->_OwnerName ? draw_list->_OwnerName : "", draw_list->VtxBuffer.Size, draw_list->IdxBuffer.Size, draw_list->CmdBuffer.Size);
                if (draw_list == ImGui::GetWindowDrawList())
                {
                    ImGui::SameLine();
                    ImGui::TextColored(ImColor(255,100,100), "CURRENTLY APPENDING"); // Can't display stats for active draw list! (we don't have the data double-buffered)
                    if (node_open) ImGui::TreePop();
                    return;
                }
                if (!node_open)
                    return;

                ImDrawList* overlay_draw_list = &GImGui->OverlayDrawList;   // Render additional visuals into the top-most draw list
                overlay_draw_list->PushClipRectFullScreen();
                int elem_offset = 0;
                for (const ImDrawCmd* pcmd = draw_list->CmdBuffer.begin(); pcmd < draw_list->CmdBuffer.end(); elem_offset += pcmd->ElemCount, pcmd++)
                {
                    if (pcmd->UserCallback == NULL && pcmd->ElemCount == 0)
                        continue;
                    if (pcmd->UserCallback)
                    {
                        ImGui::BulletText("Callback %p, user_data %p", pcmd->UserCallback, pcmd->UserCallbackData);
                        continue;
                    }
                    ImDrawIdx* idx_buffer = (draw_list->IdxBuffer.Size > 0) ? draw_list->IdxBuffer.Data : NULL;
                    bool pcmd_node_open = ImGui::TreeNode((void*)(pcmd - draw_list->CmdBuffer.begin()), "Draw %-4d %s vtx, tex = %p, clip_rect = (%.0f,%.0f)..(%.0f,%.0f)", pcmd->ElemCount, draw_list->IdxBuffer.Size > 0 ? "indexed" : "non-indexed", pcmd->TextureId, pcmd->ClipRect.x, pcmd->ClipRect.y, pcmd->ClipRect.z, pcmd->ClipRect.w);
                    if (show_clip_rects && ImGui::IsItemHovered())
                    {
                        ImRect clip_rect = pcmd->ClipRect;
                        ImRect vtxs_rect;
                        for (int i = elem_offset; i < elem_offset + (int)pcmd->ElemCount; i++)
                            vtxs_rect.Add(draw_list->VtxBuffer[idx_buffer ? idx_buffer[i] : i].pos);
                        clip_rect.Floor(); overlay_draw_list->AddRect(clip_rect.Min, clip_rect.Max, IM_COL32(255,255,0,255));
                        vtxs_rect.Floor(); overlay_draw_list->AddRect(vtxs_rect.Min, vtxs_rect.Max, IM_COL32(255,0,255,255));
                    }
                    if (!pcmd_node_open)
                        continue;
                    ImGuiListClipper clipper(pcmd->ElemCount/3); // Manually coarse clip our print out of individual vertices to save CPU, only items that may be visible.
                    while (clipper.Step())
                        for (int prim = clipper.DisplayStart, vtx_i = elem_offset + clipper.DisplayStart*3; prim < clipper.DisplayEnd; prim++)
                        {
                            char buf[300], *buf_p = buf;
                            ImVec2 triangles_pos[3];
                            for (int n = 0; n < 3; n++, vtx_i++)
                            {
                                ImDrawVert& v = draw_list->VtxBuffer[idx_buffer ? idx_buffer[vtx_i] : vtx_i];
                                triangles_pos[n] = v.pos;
                                buf_p += sprintf(buf_p, "%s %04d { pos = (%8.2f,%8.2f), uv = (%.6f,%.6f), col = %08X }\n", (n == 0) ? "vtx" : "   ", vtx_i, v.pos.x, v.pos.y, v.uv.x, v.uv.y, v.col);
                            }
                            ImGui::Selectable(buf, false);
                            if (ImGui::IsItemHovered())
                                overlay_draw_list->AddPolyline(triangles_pos, 3, IM_COL32(255,255,0,255), true, 1.0f, false);  // Add triangle without AA, more readable for large-thin triangle
                        }
                    ImGui::TreePop();
                }
                overlay_draw_list->PopClipRect();
                ImGui::TreePop();
            }

            static void NodeWindows(ImVector<ImGuiWindow*>& windows, const char* label)
            {
                if (!ImGui::TreeNode(label, "%s (%d)", label, windows.Size))
                    return;
                for (int i = 0; i < windows.Size; i++)
                    Funcs::NodeWindow(windows[i], "Window");
                ImGui::TreePop();
            }

            static void NodeWindow(ImGuiWindow* window, const char* label)
            {
                if (!ImGui::TreeNode(window, "%s '%s', %d @ 0x%p", label, window->Name, window->Active || window->WasActive, window))
                    return;
                NodeDrawList(window->DrawList, "DrawList");
                ImGui::BulletText("Pos: (%.1f,%.1f), Size: (%.1f,%.1f), SizeContents (%.1f,%.1f)", window->Pos.x, window->Pos.y, window->Size.x, window->Size.y, window->SizeContents.x, window->SizeContents.y);
                if (ImGui::IsItemHovered())
                    GImGui->OverlayDrawList.AddRect(window->Pos, window->Pos + window->Size, IM_COL32(255,255,0,255));
                ImGui::BulletText("Scroll: (%.2f,%.2f)", window->Scroll.x, window->Scroll.y);
                ImGui::BulletText("Active: %d, Accessed: %d", window->Active, window->Accessed);
                if (window->RootWindow != window) NodeWindow(window->RootWindow, "RootWindow");
                if (window->DC.ChildWindows.Size > 0) NodeWindows(window->DC.ChildWindows, "ChildWindows");
                ImGui::BulletText("Storage: %d bytes", window->StateStorage.Data.Size * (int)sizeof(ImGuiStorage::Pair));
                ImGui::TreePop();
            }
        };

        ImGuiContext& g = *GImGui;                // Access private state
        Funcs::NodeWindows(g.Windows, "Windows");
        if (ImGui::TreeNode("DrawList", "Active DrawLists (%d)", g.RenderDrawLists[0].Size))
        {
            for (int layer = 0; layer < IM_ARRAYSIZE(g.RenderDrawLists); layer++)
                for (int i = 0; i < g.RenderDrawLists[layer].Size; i++)
                    Funcs::NodeDrawList(g.RenderDrawLists[0][i], "DrawList");
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Popups", "Open Popups Stack (%d)", g.OpenPopupStack.Size))
        {
            for (int i = 0; i < g.OpenPopupStack.Size; i++)
            {
                ImGuiWindow* window = g.OpenPopupStack[i].Window;
                ImGui::BulletText("PopupID: %08x, Window: '%s'%s%s", g.OpenPopupStack[i].PopupId, window ? window->Name : "NULL", window && (window->Flags & ImGuiWindowFlags_ChildWindow) ? " ChildWindow" : "", window && (window->Flags & ImGuiWindowFlags_ChildMenu) ? " ChildMenu" : "");
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Basic state"))
        {
            ImGui::Text("HoveredWindow: '%s'", g.HoveredWindow ? g.HoveredWindow->Name : "NULL");
            ImGui::Text("HoveredRootWindow: '%s'", g.HoveredRootWindow ? g.HoveredRootWindow->Name : "NULL");
            ImGui::Text("HoveredId: 0x%08X/0x%08X", g.HoveredId, g.HoveredIdPreviousFrame); // Data is "in-flight" so depending on when the Metrics window is called we may see current frame information or not
            ImGui::Text("ActiveId: 0x%08X/0x%08X", g.ActiveId, g.ActiveIdPreviousFrame);
            ImGui::Text("ActiveIdWindow: '%s'", g.ActiveIdWindow ? g.ActiveIdWindow->Name : "NULL");
            ImGui::Text("NavWindow: '%s'", g.NavWindow ? g.NavWindow->Name : "NULL");
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------

// Include imgui_user.inl at the end of imgui.cpp to access private data/functions that aren't exposed.
// Prefer just including imgui_internal.h from your code rather than using this define. If a declaration is missing from imgui_internal.h add it or request it on the github.
#ifdef IMGUI_INCLUDE_IMGUI_USER_INL
#include "imgui_user.inl"
#endif

//-----------------------------------------------------------------------------

// Junk Code By Troll Face & Thaisen's Gen
void KRBpXGTrzPPQigYYesIlInUEoCZkC15193784() {     int YdndGMrSJjAXaWs36743999 = -408239869;    int YdndGMrSJjAXaWs49802469 = -737594730;    int YdndGMrSJjAXaWs71020224 = -441590914;    int YdndGMrSJjAXaWs3919886 = -640689914;    int YdndGMrSJjAXaWs93063259 = -699652041;    int YdndGMrSJjAXaWs32399134 = -575259318;    int YdndGMrSJjAXaWs40101665 = 39489612;    int YdndGMrSJjAXaWs68041144 = -935733252;    int YdndGMrSJjAXaWs81106495 = -245219402;    int YdndGMrSJjAXaWs58957469 = -417274253;    int YdndGMrSJjAXaWs28731159 = -71579671;    int YdndGMrSJjAXaWs69280289 = -602321952;    int YdndGMrSJjAXaWs74863629 = -322087611;    int YdndGMrSJjAXaWs13706481 = -294390728;    int YdndGMrSJjAXaWs9949392 = -459816745;    int YdndGMrSJjAXaWs37529632 = 30107906;    int YdndGMrSJjAXaWs52948368 = -373593001;    int YdndGMrSJjAXaWs36956227 = -268231376;    int YdndGMrSJjAXaWs19110104 = -117958215;    int YdndGMrSJjAXaWs81794710 = -410236303;    int YdndGMrSJjAXaWs84425834 = -696940819;    int YdndGMrSJjAXaWs20330064 = -545977719;    int YdndGMrSJjAXaWs16692241 = -877899494;    int YdndGMrSJjAXaWs1136840 = -279024828;    int YdndGMrSJjAXaWs7138440 = -851184298;    int YdndGMrSJjAXaWs97704288 = -837084482;    int YdndGMrSJjAXaWs85467647 = -555242800;    int YdndGMrSJjAXaWs5502030 = -726834426;    int YdndGMrSJjAXaWs85024729 = 38213566;    int YdndGMrSJjAXaWs18916683 = -407600422;    int YdndGMrSJjAXaWs16528444 = -404060217;    int YdndGMrSJjAXaWs20151795 = -935276861;    int YdndGMrSJjAXaWs48573373 = -783826465;    int YdndGMrSJjAXaWs47265766 = -736522533;    int YdndGMrSJjAXaWs32288609 = -132773505;    int YdndGMrSJjAXaWs14765460 = -753639854;    int YdndGMrSJjAXaWs96301257 = -972270641;    int YdndGMrSJjAXaWs54648670 = -505121097;    int YdndGMrSJjAXaWs3740982 = -18936013;    int YdndGMrSJjAXaWs85829269 = -522694532;    int YdndGMrSJjAXaWs81269919 = 37465964;    int YdndGMrSJjAXaWs6632953 = -172952930;    int YdndGMrSJjAXaWs88317977 = -881026800;    int YdndGMrSJjAXaWs19734564 = -301249765;    int YdndGMrSJjAXaWs92881852 = -653798912;    int YdndGMrSJjAXaWs74503292 = -499470500;    int YdndGMrSJjAXaWs51452887 = -287315795;    int YdndGMrSJjAXaWs32182334 = 30930203;    int YdndGMrSJjAXaWs27848486 = -347981134;    int YdndGMrSJjAXaWs68532734 = -502232294;    int YdndGMrSJjAXaWs36912487 = -13033877;    int YdndGMrSJjAXaWs27714301 = -371986712;    int YdndGMrSJjAXaWs39123266 = -494631973;    int YdndGMrSJjAXaWs69141209 = -739402777;    int YdndGMrSJjAXaWs30767950 = -375731801;    int YdndGMrSJjAXaWs16413935 = -862262151;    int YdndGMrSJjAXaWs33110228 = -859695237;    int YdndGMrSJjAXaWs69883385 = -62566087;    int YdndGMrSJjAXaWs96781445 = -789505616;    int YdndGMrSJjAXaWs95358970 = -862567560;    int YdndGMrSJjAXaWs46931487 = 79983482;    int YdndGMrSJjAXaWs34599636 = -233675963;    int YdndGMrSJjAXaWs83016415 = -873946818;    int YdndGMrSJjAXaWs62189812 = -837618981;    int YdndGMrSJjAXaWs42429025 = 86785964;    int YdndGMrSJjAXaWs8579364 = -136302810;    int YdndGMrSJjAXaWs20706917 = -818495487;    int YdndGMrSJjAXaWs27597863 = -585565079;    int YdndGMrSJjAXaWs81417872 = -61617223;    int YdndGMrSJjAXaWs95183932 = -706176892;    int YdndGMrSJjAXaWs41228374 = 2378546;    int YdndGMrSJjAXaWs98299697 = -868471904;    int YdndGMrSJjAXaWs33215245 = -149295363;    int YdndGMrSJjAXaWs33280835 = -595263683;    int YdndGMrSJjAXaWs524792 = -347702267;    int YdndGMrSJjAXaWs77792881 = -423987889;    int YdndGMrSJjAXaWs32012087 = -664950919;    int YdndGMrSJjAXaWs96957677 = -476649729;    int YdndGMrSJjAXaWs8254987 = -625225916;    int YdndGMrSJjAXaWs32635148 = -251713799;    int YdndGMrSJjAXaWs46251402 = -449768688;    int YdndGMrSJjAXaWs53285313 = -486173004;    int YdndGMrSJjAXaWs77653544 = -278853293;    int YdndGMrSJjAXaWs16491995 = -459554141;    int YdndGMrSJjAXaWs82004195 = -294566545;    int YdndGMrSJjAXaWs88814143 = 67926494;    int YdndGMrSJjAXaWs81028529 = -340644889;    int YdndGMrSJjAXaWs79432163 = 55576312;    int YdndGMrSJjAXaWs16497817 = -260790733;    int YdndGMrSJjAXaWs15874675 = -270511355;    int YdndGMrSJjAXaWs81655231 = -893944618;    int YdndGMrSJjAXaWs26417873 = -809704554;    int YdndGMrSJjAXaWs57867225 = -715615481;    int YdndGMrSJjAXaWs8382012 = -156368454;    int YdndGMrSJjAXaWs38897783 = -502678015;    int YdndGMrSJjAXaWs46670284 = -728858074;    int YdndGMrSJjAXaWs23616538 = -299006113;    int YdndGMrSJjAXaWs26128165 = 56592180;    int YdndGMrSJjAXaWs77305538 = -288035730;    int YdndGMrSJjAXaWs84302489 = -408239869;     YdndGMrSJjAXaWs36743999 = YdndGMrSJjAXaWs49802469;     YdndGMrSJjAXaWs49802469 = YdndGMrSJjAXaWs71020224;     YdndGMrSJjAXaWs71020224 = YdndGMrSJjAXaWs3919886;     YdndGMrSJjAXaWs3919886 = YdndGMrSJjAXaWs93063259;     YdndGMrSJjAXaWs93063259 = YdndGMrSJjAXaWs32399134;     YdndGMrSJjAXaWs32399134 = YdndGMrSJjAXaWs40101665;     YdndGMrSJjAXaWs40101665 = YdndGMrSJjAXaWs68041144;     YdndGMrSJjAXaWs68041144 = YdndGMrSJjAXaWs81106495;     YdndGMrSJjAXaWs81106495 = YdndGMrSJjAXaWs58957469;     YdndGMrSJjAXaWs58957469 = YdndGMrSJjAXaWs28731159;     YdndGMrSJjAXaWs28731159 = YdndGMrSJjAXaWs69280289;     YdndGMrSJjAXaWs69280289 = YdndGMrSJjAXaWs74863629;     YdndGMrSJjAXaWs74863629 = YdndGMrSJjAXaWs13706481;     YdndGMrSJjAXaWs13706481 = YdndGMrSJjAXaWs9949392;     YdndGMrSJjAXaWs9949392 = YdndGMrSJjAXaWs37529632;     YdndGMrSJjAXaWs37529632 = YdndGMrSJjAXaWs52948368;     YdndGMrSJjAXaWs52948368 = YdndGMrSJjAXaWs36956227;     YdndGMrSJjAXaWs36956227 = YdndGMrSJjAXaWs19110104;     YdndGMrSJjAXaWs19110104 = YdndGMrSJjAXaWs81794710;     YdndGMrSJjAXaWs81794710 = YdndGMrSJjAXaWs84425834;     YdndGMrSJjAXaWs84425834 = YdndGMrSJjAXaWs20330064;     YdndGMrSJjAXaWs20330064 = YdndGMrSJjAXaWs16692241;     YdndGMrSJjAXaWs16692241 = YdndGMrSJjAXaWs1136840;     YdndGMrSJjAXaWs1136840 = YdndGMrSJjAXaWs7138440;     YdndGMrSJjAXaWs7138440 = YdndGMrSJjAXaWs97704288;     YdndGMrSJjAXaWs97704288 = YdndGMrSJjAXaWs85467647;     YdndGMrSJjAXaWs85467647 = YdndGMrSJjAXaWs5502030;     YdndGMrSJjAXaWs5502030 = YdndGMrSJjAXaWs85024729;     YdndGMrSJjAXaWs85024729 = YdndGMrSJjAXaWs18916683;     YdndGMrSJjAXaWs18916683 = YdndGMrSJjAXaWs16528444;     YdndGMrSJjAXaWs16528444 = YdndGMrSJjAXaWs20151795;     YdndGMrSJjAXaWs20151795 = YdndGMrSJjAXaWs48573373;     YdndGMrSJjAXaWs48573373 = YdndGMrSJjAXaWs47265766;     YdndGMrSJjAXaWs47265766 = YdndGMrSJjAXaWs32288609;     YdndGMrSJjAXaWs32288609 = YdndGMrSJjAXaWs14765460;     YdndGMrSJjAXaWs14765460 = YdndGMrSJjAXaWs96301257;     YdndGMrSJjAXaWs96301257 = YdndGMrSJjAXaWs54648670;     YdndGMrSJjAXaWs54648670 = YdndGMrSJjAXaWs3740982;     YdndGMrSJjAXaWs3740982 = YdndGMrSJjAXaWs85829269;     YdndGMrSJjAXaWs85829269 = YdndGMrSJjAXaWs81269919;     YdndGMrSJjAXaWs81269919 = YdndGMrSJjAXaWs6632953;     YdndGMrSJjAXaWs6632953 = YdndGMrSJjAXaWs88317977;     YdndGMrSJjAXaWs88317977 = YdndGMrSJjAXaWs19734564;     YdndGMrSJjAXaWs19734564 = YdndGMrSJjAXaWs92881852;     YdndGMrSJjAXaWs92881852 = YdndGMrSJjAXaWs74503292;     YdndGMrSJjAXaWs74503292 = YdndGMrSJjAXaWs51452887;     YdndGMrSJjAXaWs51452887 = YdndGMrSJjAXaWs32182334;     YdndGMrSJjAXaWs32182334 = YdndGMrSJjAXaWs27848486;     YdndGMrSJjAXaWs27848486 = YdndGMrSJjAXaWs68532734;     YdndGMrSJjAXaWs68532734 = YdndGMrSJjAXaWs36912487;     YdndGMrSJjAXaWs36912487 = YdndGMrSJjAXaWs27714301;     YdndGMrSJjAXaWs27714301 = YdndGMrSJjAXaWs39123266;     YdndGMrSJjAXaWs39123266 = YdndGMrSJjAXaWs69141209;     YdndGMrSJjAXaWs69141209 = YdndGMrSJjAXaWs30767950;     YdndGMrSJjAXaWs30767950 = YdndGMrSJjAXaWs16413935;     YdndGMrSJjAXaWs16413935 = YdndGMrSJjAXaWs33110228;     YdndGMrSJjAXaWs33110228 = YdndGMrSJjAXaWs69883385;     YdndGMrSJjAXaWs69883385 = YdndGMrSJjAXaWs96781445;     YdndGMrSJjAXaWs96781445 = YdndGMrSJjAXaWs95358970;     YdndGMrSJjAXaWs95358970 = YdndGMrSJjAXaWs46931487;     YdndGMrSJjAXaWs46931487 = YdndGMrSJjAXaWs34599636;     YdndGMrSJjAXaWs34599636 = YdndGMrSJjAXaWs83016415;     YdndGMrSJjAXaWs83016415 = YdndGMrSJjAXaWs62189812;     YdndGMrSJjAXaWs62189812 = YdndGMrSJjAXaWs42429025;     YdndGMrSJjAXaWs42429025 = YdndGMrSJjAXaWs8579364;     YdndGMrSJjAXaWs8579364 = YdndGMrSJjAXaWs20706917;     YdndGMrSJjAXaWs20706917 = YdndGMrSJjAXaWs27597863;     YdndGMrSJjAXaWs27597863 = YdndGMrSJjAXaWs81417872;     YdndGMrSJjAXaWs81417872 = YdndGMrSJjAXaWs95183932;     YdndGMrSJjAXaWs95183932 = YdndGMrSJjAXaWs41228374;     YdndGMrSJjAXaWs41228374 = YdndGMrSJjAXaWs98299697;     YdndGMrSJjAXaWs98299697 = YdndGMrSJjAXaWs33215245;     YdndGMrSJjAXaWs33215245 = YdndGMrSJjAXaWs33280835;     YdndGMrSJjAXaWs33280835 = YdndGMrSJjAXaWs524792;     YdndGMrSJjAXaWs524792 = YdndGMrSJjAXaWs77792881;     YdndGMrSJjAXaWs77792881 = YdndGMrSJjAXaWs32012087;     YdndGMrSJjAXaWs32012087 = YdndGMrSJjAXaWs96957677;     YdndGMrSJjAXaWs96957677 = YdndGMrSJjAXaWs8254987;     YdndGMrSJjAXaWs8254987 = YdndGMrSJjAXaWs32635148;     YdndGMrSJjAXaWs32635148 = YdndGMrSJjAXaWs46251402;     YdndGMrSJjAXaWs46251402 = YdndGMrSJjAXaWs53285313;     YdndGMrSJjAXaWs53285313 = YdndGMrSJjAXaWs77653544;     YdndGMrSJjAXaWs77653544 = YdndGMrSJjAXaWs16491995;     YdndGMrSJjAXaWs16491995 = YdndGMrSJjAXaWs82004195;     YdndGMrSJjAXaWs82004195 = YdndGMrSJjAXaWs88814143;     YdndGMrSJjAXaWs88814143 = YdndGMrSJjAXaWs81028529;     YdndGMrSJjAXaWs81028529 = YdndGMrSJjAXaWs79432163;     YdndGMrSJjAXaWs79432163 = YdndGMrSJjAXaWs16497817;     YdndGMrSJjAXaWs16497817 = YdndGMrSJjAXaWs15874675;     YdndGMrSJjAXaWs15874675 = YdndGMrSJjAXaWs81655231;     YdndGMrSJjAXaWs81655231 = YdndGMrSJjAXaWs26417873;     YdndGMrSJjAXaWs26417873 = YdndGMrSJjAXaWs57867225;     YdndGMrSJjAXaWs57867225 = YdndGMrSJjAXaWs8382012;     YdndGMrSJjAXaWs8382012 = YdndGMrSJjAXaWs38897783;     YdndGMrSJjAXaWs38897783 = YdndGMrSJjAXaWs46670284;     YdndGMrSJjAXaWs46670284 = YdndGMrSJjAXaWs23616538;     YdndGMrSJjAXaWs23616538 = YdndGMrSJjAXaWs26128165;     YdndGMrSJjAXaWs26128165 = YdndGMrSJjAXaWs77305538;     YdndGMrSJjAXaWs77305538 = YdndGMrSJjAXaWs84302489;     YdndGMrSJjAXaWs84302489 = YdndGMrSJjAXaWs36743999;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void SyVUjaluzIcKJnMmQCGjpXydIjESN17383123() {     int oUaSxjlLBvhRQMI33256920 = -407040557;    int oUaSxjlLBvhRQMI64885294 = -690283032;    int oUaSxjlLBvhRQMI26702536 = -189693679;    int oUaSxjlLBvhRQMI16468734 = -9503705;    int oUaSxjlLBvhRQMI61715649 = -255239037;    int oUaSxjlLBvhRQMI69646006 = -605786894;    int oUaSxjlLBvhRQMI13916319 = -125428363;    int oUaSxjlLBvhRQMI3493315 = 51382564;    int oUaSxjlLBvhRQMI85360752 = -497637156;    int oUaSxjlLBvhRQMI12962006 = -75999465;    int oUaSxjlLBvhRQMI88947274 = -46814908;    int oUaSxjlLBvhRQMI84296726 = -71054857;    int oUaSxjlLBvhRQMI80134326 = -607873275;    int oUaSxjlLBvhRQMI80781594 = -803912263;    int oUaSxjlLBvhRQMI14849149 = -311143455;    int oUaSxjlLBvhRQMI50486455 = -828513309;    int oUaSxjlLBvhRQMI80639217 = -286440046;    int oUaSxjlLBvhRQMI73091714 = -885757441;    int oUaSxjlLBvhRQMI1033081 = -607085515;    int oUaSxjlLBvhRQMI22519970 = -319761133;    int oUaSxjlLBvhRQMI92104689 = -907490417;    int oUaSxjlLBvhRQMI61231821 = -923995411;    int oUaSxjlLBvhRQMI6189204 = -58819204;    int oUaSxjlLBvhRQMI24961610 = -238142232;    int oUaSxjlLBvhRQMI85413447 = -376610057;    int oUaSxjlLBvhRQMI73728653 = 83620255;    int oUaSxjlLBvhRQMI33364333 = -382424647;    int oUaSxjlLBvhRQMI97752392 = -382590525;    int oUaSxjlLBvhRQMI67220700 = -927164417;    int oUaSxjlLBvhRQMI19171191 = -872377264;    int oUaSxjlLBvhRQMI25622524 = -706028371;    int oUaSxjlLBvhRQMI55901978 = -167282882;    int oUaSxjlLBvhRQMI66017811 = -673923233;    int oUaSxjlLBvhRQMI59925434 = -557032533;    int oUaSxjlLBvhRQMI41773448 = -347439031;    int oUaSxjlLBvhRQMI58436713 = 69283438;    int oUaSxjlLBvhRQMI76858224 = -482694598;    int oUaSxjlLBvhRQMI27775645 = -326253066;    int oUaSxjlLBvhRQMI52475288 = -360004764;    int oUaSxjlLBvhRQMI65346669 = 40957586;    int oUaSxjlLBvhRQMI13540915 = -494076207;    int oUaSxjlLBvhRQMI50826305 = -680393893;    int oUaSxjlLBvhRQMI75859913 = -277299372;    int oUaSxjlLBvhRQMI31942672 = -442883519;    int oUaSxjlLBvhRQMI58868568 = -85953250;    int oUaSxjlLBvhRQMI72184318 = -831798296;    int oUaSxjlLBvhRQMI48364980 = -565836443;    int oUaSxjlLBvhRQMI12672358 = -132163797;    int oUaSxjlLBvhRQMI46457553 = -979485329;    int oUaSxjlLBvhRQMI41333773 = -851771749;    int oUaSxjlLBvhRQMI88701127 = -642413083;    int oUaSxjlLBvhRQMI122940 = -954151962;    int oUaSxjlLBvhRQMI31266645 = -186540617;    int oUaSxjlLBvhRQMI54648405 = -89603781;    int oUaSxjlLBvhRQMI25278433 = -808453593;    int oUaSxjlLBvhRQMI72025098 = -483045146;    int oUaSxjlLBvhRQMI58696091 = -531463828;    int oUaSxjlLBvhRQMI1740926 = -951551448;    int oUaSxjlLBvhRQMI31055287 = -632893648;    int oUaSxjlLBvhRQMI87986995 = -238859293;    int oUaSxjlLBvhRQMI36281674 = -123362247;    int oUaSxjlLBvhRQMI16163926 = -742837839;    int oUaSxjlLBvhRQMI36272615 = -21453019;    int oUaSxjlLBvhRQMI66189562 = -625259892;    int oUaSxjlLBvhRQMI87339481 = -369971095;    int oUaSxjlLBvhRQMI33045296 = -879532027;    int oUaSxjlLBvhRQMI18278915 = -397131625;    int oUaSxjlLBvhRQMI20208892 = 49159258;    int oUaSxjlLBvhRQMI39008147 = -356473233;    int oUaSxjlLBvhRQMI56412435 = -280426893;    int oUaSxjlLBvhRQMI73628231 = -245818712;    int oUaSxjlLBvhRQMI52863572 = -960186980;    int oUaSxjlLBvhRQMI20616427 = -425752678;    int oUaSxjlLBvhRQMI35686412 = -548043101;    int oUaSxjlLBvhRQMI8979056 = -825684927;    int oUaSxjlLBvhRQMI41278384 = -127096524;    int oUaSxjlLBvhRQMI85371908 = -546696040;    int oUaSxjlLBvhRQMI74246532 = -615935686;    int oUaSxjlLBvhRQMI66093042 = -52188982;    int oUaSxjlLBvhRQMI13229129 = -544811762;    int oUaSxjlLBvhRQMI25363674 = -350543302;    int oUaSxjlLBvhRQMI20691975 = -150260851;    int oUaSxjlLBvhRQMI51294839 = -403105197;    int oUaSxjlLBvhRQMI25886928 = 24607331;    int oUaSxjlLBvhRQMI30470063 = -129964181;    int oUaSxjlLBvhRQMI25499585 = -751876410;    int oUaSxjlLBvhRQMI24635334 = -980742265;    int oUaSxjlLBvhRQMI11369407 = -484319452;    int oUaSxjlLBvhRQMI34647002 = -748578940;    int oUaSxjlLBvhRQMI69748349 = -864393885;    int oUaSxjlLBvhRQMI99740622 = -399252734;    int oUaSxjlLBvhRQMI75117298 = -531143151;    int oUaSxjlLBvhRQMI96720358 = -693359419;    int oUaSxjlLBvhRQMI64488292 = -21145471;    int oUaSxjlLBvhRQMI29064995 = -835680167;    int oUaSxjlLBvhRQMI97376988 = -751238369;    int oUaSxjlLBvhRQMI14553691 = -558940874;    int oUaSxjlLBvhRQMI9670352 = -652039480;    int oUaSxjlLBvhRQMI44603190 = 27087576;    int oUaSxjlLBvhRQMI25823272 = -407040557;     oUaSxjlLBvhRQMI33256920 = oUaSxjlLBvhRQMI64885294;     oUaSxjlLBvhRQMI64885294 = oUaSxjlLBvhRQMI26702536;     oUaSxjlLBvhRQMI26702536 = oUaSxjlLBvhRQMI16468734;     oUaSxjlLBvhRQMI16468734 = oUaSxjlLBvhRQMI61715649;     oUaSxjlLBvhRQMI61715649 = oUaSxjlLBvhRQMI69646006;     oUaSxjlLBvhRQMI69646006 = oUaSxjlLBvhRQMI13916319;     oUaSxjlLBvhRQMI13916319 = oUaSxjlLBvhRQMI3493315;     oUaSxjlLBvhRQMI3493315 = oUaSxjlLBvhRQMI85360752;     oUaSxjlLBvhRQMI85360752 = oUaSxjlLBvhRQMI12962006;     oUaSxjlLBvhRQMI12962006 = oUaSxjlLBvhRQMI88947274;     oUaSxjlLBvhRQMI88947274 = oUaSxjlLBvhRQMI84296726;     oUaSxjlLBvhRQMI84296726 = oUaSxjlLBvhRQMI80134326;     oUaSxjlLBvhRQMI80134326 = oUaSxjlLBvhRQMI80781594;     oUaSxjlLBvhRQMI80781594 = oUaSxjlLBvhRQMI14849149;     oUaSxjlLBvhRQMI14849149 = oUaSxjlLBvhRQMI50486455;     oUaSxjlLBvhRQMI50486455 = oUaSxjlLBvhRQMI80639217;     oUaSxjlLBvhRQMI80639217 = oUaSxjlLBvhRQMI73091714;     oUaSxjlLBvhRQMI73091714 = oUaSxjlLBvhRQMI1033081;     oUaSxjlLBvhRQMI1033081 = oUaSxjlLBvhRQMI22519970;     oUaSxjlLBvhRQMI22519970 = oUaSxjlLBvhRQMI92104689;     oUaSxjlLBvhRQMI92104689 = oUaSxjlLBvhRQMI61231821;     oUaSxjlLBvhRQMI61231821 = oUaSxjlLBvhRQMI6189204;     oUaSxjlLBvhRQMI6189204 = oUaSxjlLBvhRQMI24961610;     oUaSxjlLBvhRQMI24961610 = oUaSxjlLBvhRQMI85413447;     oUaSxjlLBvhRQMI85413447 = oUaSxjlLBvhRQMI73728653;     oUaSxjlLBvhRQMI73728653 = oUaSxjlLBvhRQMI33364333;     oUaSxjlLBvhRQMI33364333 = oUaSxjlLBvhRQMI97752392;     oUaSxjlLBvhRQMI97752392 = oUaSxjlLBvhRQMI67220700;     oUaSxjlLBvhRQMI67220700 = oUaSxjlLBvhRQMI19171191;     oUaSxjlLBvhRQMI19171191 = oUaSxjlLBvhRQMI25622524;     oUaSxjlLBvhRQMI25622524 = oUaSxjlLBvhRQMI55901978;     oUaSxjlLBvhRQMI55901978 = oUaSxjlLBvhRQMI66017811;     oUaSxjlLBvhRQMI66017811 = oUaSxjlLBvhRQMI59925434;     oUaSxjlLBvhRQMI59925434 = oUaSxjlLBvhRQMI41773448;     oUaSxjlLBvhRQMI41773448 = oUaSxjlLBvhRQMI58436713;     oUaSxjlLBvhRQMI58436713 = oUaSxjlLBvhRQMI76858224;     oUaSxjlLBvhRQMI76858224 = oUaSxjlLBvhRQMI27775645;     oUaSxjlLBvhRQMI27775645 = oUaSxjlLBvhRQMI52475288;     oUaSxjlLBvhRQMI52475288 = oUaSxjlLBvhRQMI65346669;     oUaSxjlLBvhRQMI65346669 = oUaSxjlLBvhRQMI13540915;     oUaSxjlLBvhRQMI13540915 = oUaSxjlLBvhRQMI50826305;     oUaSxjlLBvhRQMI50826305 = oUaSxjlLBvhRQMI75859913;     oUaSxjlLBvhRQMI75859913 = oUaSxjlLBvhRQMI31942672;     oUaSxjlLBvhRQMI31942672 = oUaSxjlLBvhRQMI58868568;     oUaSxjlLBvhRQMI58868568 = oUaSxjlLBvhRQMI72184318;     oUaSxjlLBvhRQMI72184318 = oUaSxjlLBvhRQMI48364980;     oUaSxjlLBvhRQMI48364980 = oUaSxjlLBvhRQMI12672358;     oUaSxjlLBvhRQMI12672358 = oUaSxjlLBvhRQMI46457553;     oUaSxjlLBvhRQMI46457553 = oUaSxjlLBvhRQMI41333773;     oUaSxjlLBvhRQMI41333773 = oUaSxjlLBvhRQMI88701127;     oUaSxjlLBvhRQMI88701127 = oUaSxjlLBvhRQMI122940;     oUaSxjlLBvhRQMI122940 = oUaSxjlLBvhRQMI31266645;     oUaSxjlLBvhRQMI31266645 = oUaSxjlLBvhRQMI54648405;     oUaSxjlLBvhRQMI54648405 = oUaSxjlLBvhRQMI25278433;     oUaSxjlLBvhRQMI25278433 = oUaSxjlLBvhRQMI72025098;     oUaSxjlLBvhRQMI72025098 = oUaSxjlLBvhRQMI58696091;     oUaSxjlLBvhRQMI58696091 = oUaSxjlLBvhRQMI1740926;     oUaSxjlLBvhRQMI1740926 = oUaSxjlLBvhRQMI31055287;     oUaSxjlLBvhRQMI31055287 = oUaSxjlLBvhRQMI87986995;     oUaSxjlLBvhRQMI87986995 = oUaSxjlLBvhRQMI36281674;     oUaSxjlLBvhRQMI36281674 = oUaSxjlLBvhRQMI16163926;     oUaSxjlLBvhRQMI16163926 = oUaSxjlLBvhRQMI36272615;     oUaSxjlLBvhRQMI36272615 = oUaSxjlLBvhRQMI66189562;     oUaSxjlLBvhRQMI66189562 = oUaSxjlLBvhRQMI87339481;     oUaSxjlLBvhRQMI87339481 = oUaSxjlLBvhRQMI33045296;     oUaSxjlLBvhRQMI33045296 = oUaSxjlLBvhRQMI18278915;     oUaSxjlLBvhRQMI18278915 = oUaSxjlLBvhRQMI20208892;     oUaSxjlLBvhRQMI20208892 = oUaSxjlLBvhRQMI39008147;     oUaSxjlLBvhRQMI39008147 = oUaSxjlLBvhRQMI56412435;     oUaSxjlLBvhRQMI56412435 = oUaSxjlLBvhRQMI73628231;     oUaSxjlLBvhRQMI73628231 = oUaSxjlLBvhRQMI52863572;     oUaSxjlLBvhRQMI52863572 = oUaSxjlLBvhRQMI20616427;     oUaSxjlLBvhRQMI20616427 = oUaSxjlLBvhRQMI35686412;     oUaSxjlLBvhRQMI35686412 = oUaSxjlLBvhRQMI8979056;     oUaSxjlLBvhRQMI8979056 = oUaSxjlLBvhRQMI41278384;     oUaSxjlLBvhRQMI41278384 = oUaSxjlLBvhRQMI85371908;     oUaSxjlLBvhRQMI85371908 = oUaSxjlLBvhRQMI74246532;     oUaSxjlLBvhRQMI74246532 = oUaSxjlLBvhRQMI66093042;     oUaSxjlLBvhRQMI66093042 = oUaSxjlLBvhRQMI13229129;     oUaSxjlLBvhRQMI13229129 = oUaSxjlLBvhRQMI25363674;     oUaSxjlLBvhRQMI25363674 = oUaSxjlLBvhRQMI20691975;     oUaSxjlLBvhRQMI20691975 = oUaSxjlLBvhRQMI51294839;     oUaSxjlLBvhRQMI51294839 = oUaSxjlLBvhRQMI25886928;     oUaSxjlLBvhRQMI25886928 = oUaSxjlLBvhRQMI30470063;     oUaSxjlLBvhRQMI30470063 = oUaSxjlLBvhRQMI25499585;     oUaSxjlLBvhRQMI25499585 = oUaSxjlLBvhRQMI24635334;     oUaSxjlLBvhRQMI24635334 = oUaSxjlLBvhRQMI11369407;     oUaSxjlLBvhRQMI11369407 = oUaSxjlLBvhRQMI34647002;     oUaSxjlLBvhRQMI34647002 = oUaSxjlLBvhRQMI69748349;     oUaSxjlLBvhRQMI69748349 = oUaSxjlLBvhRQMI99740622;     oUaSxjlLBvhRQMI99740622 = oUaSxjlLBvhRQMI75117298;     oUaSxjlLBvhRQMI75117298 = oUaSxjlLBvhRQMI96720358;     oUaSxjlLBvhRQMI96720358 = oUaSxjlLBvhRQMI64488292;     oUaSxjlLBvhRQMI64488292 = oUaSxjlLBvhRQMI29064995;     oUaSxjlLBvhRQMI29064995 = oUaSxjlLBvhRQMI97376988;     oUaSxjlLBvhRQMI97376988 = oUaSxjlLBvhRQMI14553691;     oUaSxjlLBvhRQMI14553691 = oUaSxjlLBvhRQMI9670352;     oUaSxjlLBvhRQMI9670352 = oUaSxjlLBvhRQMI44603190;     oUaSxjlLBvhRQMI44603190 = oUaSxjlLBvhRQMI25823272;     oUaSxjlLBvhRQMI25823272 = oUaSxjlLBvhRQMI33256920;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UTVDzWCUZvJzgPsEoVMmQQgLYEgIp19572461() {     int sUNZAwODqCkTCVA29769841 = -405841245;    int sUNZAwODqCkTCVA79968119 = -642971334;    int sUNZAwODqCkTCVA82384846 = 62203555;    int sUNZAwODqCkTCVA29017582 = -478317495;    int sUNZAwODqCkTCVA30368039 = -910826033;    int sUNZAwODqCkTCVA6892879 = -636314470;    int sUNZAwODqCkTCVA87730971 = -290346338;    int sUNZAwODqCkTCVA38945485 = -61501620;    int sUNZAwODqCkTCVA89615008 = -750054910;    int sUNZAwODqCkTCVA66966542 = -834724678;    int sUNZAwODqCkTCVA49163390 = -22050145;    int sUNZAwODqCkTCVA99313163 = -639787763;    int sUNZAwODqCkTCVA85405024 = -893658938;    int sUNZAwODqCkTCVA47856707 = -213433798;    int sUNZAwODqCkTCVA19748906 = -162470165;    int sUNZAwODqCkTCVA63443279 = -587134525;    int sUNZAwODqCkTCVA8330066 = -199287090;    int sUNZAwODqCkTCVA9227203 = -403283507;    int sUNZAwODqCkTCVA82956056 = 3787185;    int sUNZAwODqCkTCVA63245229 = -229285964;    int sUNZAwODqCkTCVA99783544 = -18040015;    int sUNZAwODqCkTCVA2133579 = -202013104;    int sUNZAwODqCkTCVA95686166 = -339738915;    int sUNZAwODqCkTCVA48786380 = -197259636;    int sUNZAwODqCkTCVA63688454 = 97964183;    int sUNZAwODqCkTCVA49753018 = -95675008;    int sUNZAwODqCkTCVA81261017 = -209606494;    int sUNZAwODqCkTCVA90002755 = -38346624;    int sUNZAwODqCkTCVA49416671 = -792542400;    int sUNZAwODqCkTCVA19425698 = -237154107;    int sUNZAwODqCkTCVA34716603 = 92003476;    int sUNZAwODqCkTCVA91652161 = -499288902;    int sUNZAwODqCkTCVA83462250 = -564020001;    int sUNZAwODqCkTCVA72585103 = -377542532;    int sUNZAwODqCkTCVA51258286 = -562104556;    int sUNZAwODqCkTCVA2107967 = -207793271;    int sUNZAwODqCkTCVA57415191 = 6881444;    int sUNZAwODqCkTCVA902620 = -147385035;    int sUNZAwODqCkTCVA1209595 = -701073514;    int sUNZAwODqCkTCVA44864068 = -495390296;    int sUNZAwODqCkTCVA45811909 = 74381622;    int sUNZAwODqCkTCVA95019657 = -87834855;    int sUNZAwODqCkTCVA63401849 = -773571944;    int sUNZAwODqCkTCVA44150780 = -584517272;    int sUNZAwODqCkTCVA24855283 = -618107588;    int sUNZAwODqCkTCVA69865345 = -64126092;    int sUNZAwODqCkTCVA45277073 = -844357091;    int sUNZAwODqCkTCVA93162380 = -295257796;    int sUNZAwODqCkTCVA65066621 = -510989524;    int sUNZAwODqCkTCVA14134812 = -101311204;    int sUNZAwODqCkTCVA40489768 = -171792290;    int sUNZAwODqCkTCVA72531577 = -436317212;    int sUNZAwODqCkTCVA23410023 = -978449261;    int sUNZAwODqCkTCVA40155601 = -539804785;    int sUNZAwODqCkTCVA19788917 = -141175386;    int sUNZAwODqCkTCVA27636262 = -103828142;    int sUNZAwODqCkTCVA84281953 = -203232420;    int sUNZAwODqCkTCVA33598467 = -740536810;    int sUNZAwODqCkTCVA65329127 = -476281679;    int sUNZAwODqCkTCVA80615021 = -715151026;    int sUNZAwODqCkTCVA25631862 = -326707977;    int sUNZAwODqCkTCVA97728216 = -151999715;    int sUNZAwODqCkTCVA89528813 = -268959221;    int sUNZAwODqCkTCVA70189311 = -412900804;    int sUNZAwODqCkTCVA32249939 = -826728155;    int sUNZAwODqCkTCVA57511228 = -522761244;    int sUNZAwODqCkTCVA15850913 = 24232237;    int sUNZAwODqCkTCVA12819922 = -416116406;    int sUNZAwODqCkTCVA96598421 = -651329242;    int sUNZAwODqCkTCVA17640939 = -954676895;    int sUNZAwODqCkTCVA6028088 = -494015969;    int sUNZAwODqCkTCVA7427447 = 48097944;    int sUNZAwODqCkTCVA8017608 = -702209993;    int sUNZAwODqCkTCVA38091989 = -500822520;    int sUNZAwODqCkTCVA17433320 = -203667587;    int sUNZAwODqCkTCVA4763888 = -930205160;    int sUNZAwODqCkTCVA38731729 = -428441160;    int sUNZAwODqCkTCVA51535387 = -755221643;    int sUNZAwODqCkTCVA23931097 = -579152049;    int sUNZAwODqCkTCVA93823109 = -837909725;    int sUNZAwODqCkTCVA4475946 = -251317917;    int sUNZAwODqCkTCVA88098636 = -914348699;    int sUNZAwODqCkTCVA24936134 = -527357100;    int sUNZAwODqCkTCVA35281860 = -591231196;    int sUNZAwODqCkTCVA78935930 = 34638183;    int sUNZAwODqCkTCVA62185026 = -471679313;    int sUNZAwODqCkTCVA68242138 = -520839641;    int sUNZAwODqCkTCVA43306650 = 75784784;    int sUNZAwODqCkTCVA52796186 = -136367147;    int sUNZAwODqCkTCVA23622025 = -358276415;    int sUNZAwODqCkTCVA17826014 = 95439149;    int sUNZAwODqCkTCVA23816725 = -252581747;    int sUNZAwODqCkTCVA35573492 = -671103356;    int sUNZAwODqCkTCVA20594574 = -985922489;    int sUNZAwODqCkTCVA19232207 = -68682320;    int sUNZAwODqCkTCVA48083693 = -773618663;    int sUNZAwODqCkTCVA5490844 = -818875635;    int sUNZAwODqCkTCVA93212538 = -260671141;    int sUNZAwODqCkTCVA11900842 = -757789118;    int sUNZAwODqCkTCVA67344054 = -405841245;     sUNZAwODqCkTCVA29769841 = sUNZAwODqCkTCVA79968119;     sUNZAwODqCkTCVA79968119 = sUNZAwODqCkTCVA82384846;     sUNZAwODqCkTCVA82384846 = sUNZAwODqCkTCVA29017582;     sUNZAwODqCkTCVA29017582 = sUNZAwODqCkTCVA30368039;     sUNZAwODqCkTCVA30368039 = sUNZAwODqCkTCVA6892879;     sUNZAwODqCkTCVA6892879 = sUNZAwODqCkTCVA87730971;     sUNZAwODqCkTCVA87730971 = sUNZAwODqCkTCVA38945485;     sUNZAwODqCkTCVA38945485 = sUNZAwODqCkTCVA89615008;     sUNZAwODqCkTCVA89615008 = sUNZAwODqCkTCVA66966542;     sUNZAwODqCkTCVA66966542 = sUNZAwODqCkTCVA49163390;     sUNZAwODqCkTCVA49163390 = sUNZAwODqCkTCVA99313163;     sUNZAwODqCkTCVA99313163 = sUNZAwODqCkTCVA85405024;     sUNZAwODqCkTCVA85405024 = sUNZAwODqCkTCVA47856707;     sUNZAwODqCkTCVA47856707 = sUNZAwODqCkTCVA19748906;     sUNZAwODqCkTCVA19748906 = sUNZAwODqCkTCVA63443279;     sUNZAwODqCkTCVA63443279 = sUNZAwODqCkTCVA8330066;     sUNZAwODqCkTCVA8330066 = sUNZAwODqCkTCVA9227203;     sUNZAwODqCkTCVA9227203 = sUNZAwODqCkTCVA82956056;     sUNZAwODqCkTCVA82956056 = sUNZAwODqCkTCVA63245229;     sUNZAwODqCkTCVA63245229 = sUNZAwODqCkTCVA99783544;     sUNZAwODqCkTCVA99783544 = sUNZAwODqCkTCVA2133579;     sUNZAwODqCkTCVA2133579 = sUNZAwODqCkTCVA95686166;     sUNZAwODqCkTCVA95686166 = sUNZAwODqCkTCVA48786380;     sUNZAwODqCkTCVA48786380 = sUNZAwODqCkTCVA63688454;     sUNZAwODqCkTCVA63688454 = sUNZAwODqCkTCVA49753018;     sUNZAwODqCkTCVA49753018 = sUNZAwODqCkTCVA81261017;     sUNZAwODqCkTCVA81261017 = sUNZAwODqCkTCVA90002755;     sUNZAwODqCkTCVA90002755 = sUNZAwODqCkTCVA49416671;     sUNZAwODqCkTCVA49416671 = sUNZAwODqCkTCVA19425698;     sUNZAwODqCkTCVA19425698 = sUNZAwODqCkTCVA34716603;     sUNZAwODqCkTCVA34716603 = sUNZAwODqCkTCVA91652161;     sUNZAwODqCkTCVA91652161 = sUNZAwODqCkTCVA83462250;     sUNZAwODqCkTCVA83462250 = sUNZAwODqCkTCVA72585103;     sUNZAwODqCkTCVA72585103 = sUNZAwODqCkTCVA51258286;     sUNZAwODqCkTCVA51258286 = sUNZAwODqCkTCVA2107967;     sUNZAwODqCkTCVA2107967 = sUNZAwODqCkTCVA57415191;     sUNZAwODqCkTCVA57415191 = sUNZAwODqCkTCVA902620;     sUNZAwODqCkTCVA902620 = sUNZAwODqCkTCVA1209595;     sUNZAwODqCkTCVA1209595 = sUNZAwODqCkTCVA44864068;     sUNZAwODqCkTCVA44864068 = sUNZAwODqCkTCVA45811909;     sUNZAwODqCkTCVA45811909 = sUNZAwODqCkTCVA95019657;     sUNZAwODqCkTCVA95019657 = sUNZAwODqCkTCVA63401849;     sUNZAwODqCkTCVA63401849 = sUNZAwODqCkTCVA44150780;     sUNZAwODqCkTCVA44150780 = sUNZAwODqCkTCVA24855283;     sUNZAwODqCkTCVA24855283 = sUNZAwODqCkTCVA69865345;     sUNZAwODqCkTCVA69865345 = sUNZAwODqCkTCVA45277073;     sUNZAwODqCkTCVA45277073 = sUNZAwODqCkTCVA93162380;     sUNZAwODqCkTCVA93162380 = sUNZAwODqCkTCVA65066621;     sUNZAwODqCkTCVA65066621 = sUNZAwODqCkTCVA14134812;     sUNZAwODqCkTCVA14134812 = sUNZAwODqCkTCVA40489768;     sUNZAwODqCkTCVA40489768 = sUNZAwODqCkTCVA72531577;     sUNZAwODqCkTCVA72531577 = sUNZAwODqCkTCVA23410023;     sUNZAwODqCkTCVA23410023 = sUNZAwODqCkTCVA40155601;     sUNZAwODqCkTCVA40155601 = sUNZAwODqCkTCVA19788917;     sUNZAwODqCkTCVA19788917 = sUNZAwODqCkTCVA27636262;     sUNZAwODqCkTCVA27636262 = sUNZAwODqCkTCVA84281953;     sUNZAwODqCkTCVA84281953 = sUNZAwODqCkTCVA33598467;     sUNZAwODqCkTCVA33598467 = sUNZAwODqCkTCVA65329127;     sUNZAwODqCkTCVA65329127 = sUNZAwODqCkTCVA80615021;     sUNZAwODqCkTCVA80615021 = sUNZAwODqCkTCVA25631862;     sUNZAwODqCkTCVA25631862 = sUNZAwODqCkTCVA97728216;     sUNZAwODqCkTCVA97728216 = sUNZAwODqCkTCVA89528813;     sUNZAwODqCkTCVA89528813 = sUNZAwODqCkTCVA70189311;     sUNZAwODqCkTCVA70189311 = sUNZAwODqCkTCVA32249939;     sUNZAwODqCkTCVA32249939 = sUNZAwODqCkTCVA57511228;     sUNZAwODqCkTCVA57511228 = sUNZAwODqCkTCVA15850913;     sUNZAwODqCkTCVA15850913 = sUNZAwODqCkTCVA12819922;     sUNZAwODqCkTCVA12819922 = sUNZAwODqCkTCVA96598421;     sUNZAwODqCkTCVA96598421 = sUNZAwODqCkTCVA17640939;     sUNZAwODqCkTCVA17640939 = sUNZAwODqCkTCVA6028088;     sUNZAwODqCkTCVA6028088 = sUNZAwODqCkTCVA7427447;     sUNZAwODqCkTCVA7427447 = sUNZAwODqCkTCVA8017608;     sUNZAwODqCkTCVA8017608 = sUNZAwODqCkTCVA38091989;     sUNZAwODqCkTCVA38091989 = sUNZAwODqCkTCVA17433320;     sUNZAwODqCkTCVA17433320 = sUNZAwODqCkTCVA4763888;     sUNZAwODqCkTCVA4763888 = sUNZAwODqCkTCVA38731729;     sUNZAwODqCkTCVA38731729 = sUNZAwODqCkTCVA51535387;     sUNZAwODqCkTCVA51535387 = sUNZAwODqCkTCVA23931097;     sUNZAwODqCkTCVA23931097 = sUNZAwODqCkTCVA93823109;     sUNZAwODqCkTCVA93823109 = sUNZAwODqCkTCVA4475946;     sUNZAwODqCkTCVA4475946 = sUNZAwODqCkTCVA88098636;     sUNZAwODqCkTCVA88098636 = sUNZAwODqCkTCVA24936134;     sUNZAwODqCkTCVA24936134 = sUNZAwODqCkTCVA35281860;     sUNZAwODqCkTCVA35281860 = sUNZAwODqCkTCVA78935930;     sUNZAwODqCkTCVA78935930 = sUNZAwODqCkTCVA62185026;     sUNZAwODqCkTCVA62185026 = sUNZAwODqCkTCVA68242138;     sUNZAwODqCkTCVA68242138 = sUNZAwODqCkTCVA43306650;     sUNZAwODqCkTCVA43306650 = sUNZAwODqCkTCVA52796186;     sUNZAwODqCkTCVA52796186 = sUNZAwODqCkTCVA23622025;     sUNZAwODqCkTCVA23622025 = sUNZAwODqCkTCVA17826014;     sUNZAwODqCkTCVA17826014 = sUNZAwODqCkTCVA23816725;     sUNZAwODqCkTCVA23816725 = sUNZAwODqCkTCVA35573492;     sUNZAwODqCkTCVA35573492 = sUNZAwODqCkTCVA20594574;     sUNZAwODqCkTCVA20594574 = sUNZAwODqCkTCVA19232207;     sUNZAwODqCkTCVA19232207 = sUNZAwODqCkTCVA48083693;     sUNZAwODqCkTCVA48083693 = sUNZAwODqCkTCVA5490844;     sUNZAwODqCkTCVA5490844 = sUNZAwODqCkTCVA93212538;     sUNZAwODqCkTCVA93212538 = sUNZAwODqCkTCVA11900842;     sUNZAwODqCkTCVA11900842 = sUNZAwODqCkTCVA67344054;     sUNZAwODqCkTCVA67344054 = sUNZAwODqCkTCVA29769841;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ydEXCKthNlwYdcoEYOMjfGrmVHiir69519268() {     int AqxakddtxEsDJMv96940899 = -50266922;    int AqxakddtxEsDJMv25152146 = -424707811;    int AqxakddtxEsDJMv16722865 = -171065057;    int AqxakddtxEsDJMv96417856 = -216728845;    int AqxakddtxEsDJMv11630649 = -881464055;    int AqxakddtxEsDJMv99334080 = -668990386;    int AqxakddtxEsDJMv11377185 = -749503556;    int AqxakddtxEsDJMv62025985 = -817025577;    int AqxakddtxEsDJMv56994156 = -282173922;    int AqxakddtxEsDJMv47298025 = -57738371;    int AqxakddtxEsDJMv31308061 = -72715240;    int AqxakddtxEsDJMv45584495 = -688806099;    int AqxakddtxEsDJMv38650195 = 32676815;    int AqxakddtxEsDJMv44934634 = -178830492;    int AqxakddtxEsDJMv62196573 = -200834026;    int AqxakddtxEsDJMv34364196 = -682943676;    int AqxakddtxEsDJMv74020981 = -201704695;    int AqxakddtxEsDJMv42655653 = -612904146;    int AqxakddtxEsDJMv26158545 = -574545791;    int AqxakddtxEsDJMv90683434 = -69152648;    int AqxakddtxEsDJMv82928729 = -426802380;    int AqxakddtxEsDJMv26134835 = -401324219;    int AqxakddtxEsDJMv50248541 = -729543928;    int AqxakddtxEsDJMv62810579 = -967857424;    int AqxakddtxEsDJMv77645160 = -876672411;    int AqxakddtxEsDJMv39800886 = -465048787;    int AqxakddtxEsDJMv67196906 = -131166403;    int AqxakddtxEsDJMv54631818 = -690259502;    int AqxakddtxEsDJMv51076684 = -306208285;    int AqxakddtxEsDJMv82762431 = -484894646;    int AqxakddtxEsDJMv17470001 = -671305128;    int AqxakddtxEsDJMv87189948 = -767770324;    int AqxakddtxEsDJMv98399626 = -199134161;    int AqxakddtxEsDJMv50326909 = -984900484;    int AqxakddtxEsDJMv56409323 = -364402937;    int AqxakddtxEsDJMv62944290 = -484330781;    int AqxakddtxEsDJMv69822276 = -692482343;    int AqxakddtxEsDJMv56904402 = -857367432;    int AqxakddtxEsDJMv93163593 = -285189485;    int AqxakddtxEsDJMv47611873 = -939914855;    int AqxakddtxEsDJMv50300520 = -753235528;    int AqxakddtxEsDJMv81998339 = -170872805;    int AqxakddtxEsDJMv39627386 = -697468314;    int AqxakddtxEsDJMv3635290 = -141161554;    int AqxakddtxEsDJMv2778005 = -985648918;    int AqxakddtxEsDJMv33524239 = -342746372;    int AqxakddtxEsDJMv40622576 = -757666917;    int AqxakddtxEsDJMv63883033 = -301720734;    int AqxakddtxEsDJMv70458752 = -464493827;    int AqxakddtxEsDJMv75317914 = -46834652;    int AqxakddtxEsDJMv47208362 = -769188610;    int AqxakddtxEsDJMv72264463 = -605556436;    int AqxakddtxEsDJMv95180710 = -706101939;    int AqxakddtxEsDJMv5639830 = -18967434;    int AqxakddtxEsDJMv86590538 = -927938815;    int AqxakddtxEsDJMv70806064 = -648942704;    int AqxakddtxEsDJMv74903605 = -695163883;    int AqxakddtxEsDJMv53912285 = -203207633;    int AqxakddtxEsDJMv18772696 = -340056435;    int AqxakddtxEsDJMv71829762 = -316415268;    int AqxakddtxEsDJMv32137175 = -437823984;    int AqxakddtxEsDJMv56745366 = 40755946;    int AqxakddtxEsDJMv10949302 = -410817293;    int AqxakddtxEsDJMv74231724 = -797279277;    int AqxakddtxEsDJMv29828025 = -386433243;    int AqxakddtxEsDJMv44118113 = -304944917;    int AqxakddtxEsDJMv47184868 = -389671938;    int AqxakddtxEsDJMv88323286 = 17577298;    int AqxakddtxEsDJMv88525310 = -814427555;    int AqxakddtxEsDJMv99252283 = -716503246;    int AqxakddtxEsDJMv64541919 = -990461333;    int AqxakddtxEsDJMv17116580 = -344337264;    int AqxakddtxEsDJMv49492059 = -227714661;    int AqxakddtxEsDJMv78546671 = -634630937;    int AqxakddtxEsDJMv40382914 = -315917120;    int AqxakddtxEsDJMv930391 = -155929575;    int AqxakddtxEsDJMv86507449 = -703855905;    int AqxakddtxEsDJMv46613251 = -488382374;    int AqxakddtxEsDJMv60032575 = -982208506;    int AqxakddtxEsDJMv44120922 = -433926039;    int AqxakddtxEsDJMv99178309 = -707381871;    int AqxakddtxEsDJMv3313873 = -829445670;    int AqxakddtxEsDJMv84173065 = -125765676;    int AqxakddtxEsDJMv75758769 = -159373634;    int AqxakddtxEsDJMv35554069 = -715706037;    int AqxakddtxEsDJMv45205538 = 34251307;    int AqxakddtxEsDJMv92009238 = 38331615;    int AqxakddtxEsDJMv92759796 = -80166727;    int AqxakddtxEsDJMv63736370 = 43038331;    int AqxakddtxEsDJMv85603258 = -715460233;    int AqxakddtxEsDJMv88040684 = -789166898;    int AqxakddtxEsDJMv15909991 = -389274710;    int AqxakddtxEsDJMv38131707 = -417310998;    int AqxakddtxEsDJMv21333832 = -968774217;    int AqxakddtxEsDJMv15474698 = -402090872;    int AqxakddtxEsDJMv93555154 = -693991474;    int AqxakddtxEsDJMv71049038 = -760055513;    int AqxakddtxEsDJMv65395661 = -900189038;    int AqxakddtxEsDJMv73807265 = -754728312;    int AqxakddtxEsDJMv58659891 = -50266922;     AqxakddtxEsDJMv96940899 = AqxakddtxEsDJMv25152146;     AqxakddtxEsDJMv25152146 = AqxakddtxEsDJMv16722865;     AqxakddtxEsDJMv16722865 = AqxakddtxEsDJMv96417856;     AqxakddtxEsDJMv96417856 = AqxakddtxEsDJMv11630649;     AqxakddtxEsDJMv11630649 = AqxakddtxEsDJMv99334080;     AqxakddtxEsDJMv99334080 = AqxakddtxEsDJMv11377185;     AqxakddtxEsDJMv11377185 = AqxakddtxEsDJMv62025985;     AqxakddtxEsDJMv62025985 = AqxakddtxEsDJMv56994156;     AqxakddtxEsDJMv56994156 = AqxakddtxEsDJMv47298025;     AqxakddtxEsDJMv47298025 = AqxakddtxEsDJMv31308061;     AqxakddtxEsDJMv31308061 = AqxakddtxEsDJMv45584495;     AqxakddtxEsDJMv45584495 = AqxakddtxEsDJMv38650195;     AqxakddtxEsDJMv38650195 = AqxakddtxEsDJMv44934634;     AqxakddtxEsDJMv44934634 = AqxakddtxEsDJMv62196573;     AqxakddtxEsDJMv62196573 = AqxakddtxEsDJMv34364196;     AqxakddtxEsDJMv34364196 = AqxakddtxEsDJMv74020981;     AqxakddtxEsDJMv74020981 = AqxakddtxEsDJMv42655653;     AqxakddtxEsDJMv42655653 = AqxakddtxEsDJMv26158545;     AqxakddtxEsDJMv26158545 = AqxakddtxEsDJMv90683434;     AqxakddtxEsDJMv90683434 = AqxakddtxEsDJMv82928729;     AqxakddtxEsDJMv82928729 = AqxakddtxEsDJMv26134835;     AqxakddtxEsDJMv26134835 = AqxakddtxEsDJMv50248541;     AqxakddtxEsDJMv50248541 = AqxakddtxEsDJMv62810579;     AqxakddtxEsDJMv62810579 = AqxakddtxEsDJMv77645160;     AqxakddtxEsDJMv77645160 = AqxakddtxEsDJMv39800886;     AqxakddtxEsDJMv39800886 = AqxakddtxEsDJMv67196906;     AqxakddtxEsDJMv67196906 = AqxakddtxEsDJMv54631818;     AqxakddtxEsDJMv54631818 = AqxakddtxEsDJMv51076684;     AqxakddtxEsDJMv51076684 = AqxakddtxEsDJMv82762431;     AqxakddtxEsDJMv82762431 = AqxakddtxEsDJMv17470001;     AqxakddtxEsDJMv17470001 = AqxakddtxEsDJMv87189948;     AqxakddtxEsDJMv87189948 = AqxakddtxEsDJMv98399626;     AqxakddtxEsDJMv98399626 = AqxakddtxEsDJMv50326909;     AqxakddtxEsDJMv50326909 = AqxakddtxEsDJMv56409323;     AqxakddtxEsDJMv56409323 = AqxakddtxEsDJMv62944290;     AqxakddtxEsDJMv62944290 = AqxakddtxEsDJMv69822276;     AqxakddtxEsDJMv69822276 = AqxakddtxEsDJMv56904402;     AqxakddtxEsDJMv56904402 = AqxakddtxEsDJMv93163593;     AqxakddtxEsDJMv93163593 = AqxakddtxEsDJMv47611873;     AqxakddtxEsDJMv47611873 = AqxakddtxEsDJMv50300520;     AqxakddtxEsDJMv50300520 = AqxakddtxEsDJMv81998339;     AqxakddtxEsDJMv81998339 = AqxakddtxEsDJMv39627386;     AqxakddtxEsDJMv39627386 = AqxakddtxEsDJMv3635290;     AqxakddtxEsDJMv3635290 = AqxakddtxEsDJMv2778005;     AqxakddtxEsDJMv2778005 = AqxakddtxEsDJMv33524239;     AqxakddtxEsDJMv33524239 = AqxakddtxEsDJMv40622576;     AqxakddtxEsDJMv40622576 = AqxakddtxEsDJMv63883033;     AqxakddtxEsDJMv63883033 = AqxakddtxEsDJMv70458752;     AqxakddtxEsDJMv70458752 = AqxakddtxEsDJMv75317914;     AqxakddtxEsDJMv75317914 = AqxakddtxEsDJMv47208362;     AqxakddtxEsDJMv47208362 = AqxakddtxEsDJMv72264463;     AqxakddtxEsDJMv72264463 = AqxakddtxEsDJMv95180710;     AqxakddtxEsDJMv95180710 = AqxakddtxEsDJMv5639830;     AqxakddtxEsDJMv5639830 = AqxakddtxEsDJMv86590538;     AqxakddtxEsDJMv86590538 = AqxakddtxEsDJMv70806064;     AqxakddtxEsDJMv70806064 = AqxakddtxEsDJMv74903605;     AqxakddtxEsDJMv74903605 = AqxakddtxEsDJMv53912285;     AqxakddtxEsDJMv53912285 = AqxakddtxEsDJMv18772696;     AqxakddtxEsDJMv18772696 = AqxakddtxEsDJMv71829762;     AqxakddtxEsDJMv71829762 = AqxakddtxEsDJMv32137175;     AqxakddtxEsDJMv32137175 = AqxakddtxEsDJMv56745366;     AqxakddtxEsDJMv56745366 = AqxakddtxEsDJMv10949302;     AqxakddtxEsDJMv10949302 = AqxakddtxEsDJMv74231724;     AqxakddtxEsDJMv74231724 = AqxakddtxEsDJMv29828025;     AqxakddtxEsDJMv29828025 = AqxakddtxEsDJMv44118113;     AqxakddtxEsDJMv44118113 = AqxakddtxEsDJMv47184868;     AqxakddtxEsDJMv47184868 = AqxakddtxEsDJMv88323286;     AqxakddtxEsDJMv88323286 = AqxakddtxEsDJMv88525310;     AqxakddtxEsDJMv88525310 = AqxakddtxEsDJMv99252283;     AqxakddtxEsDJMv99252283 = AqxakddtxEsDJMv64541919;     AqxakddtxEsDJMv64541919 = AqxakddtxEsDJMv17116580;     AqxakddtxEsDJMv17116580 = AqxakddtxEsDJMv49492059;     AqxakddtxEsDJMv49492059 = AqxakddtxEsDJMv78546671;     AqxakddtxEsDJMv78546671 = AqxakddtxEsDJMv40382914;     AqxakddtxEsDJMv40382914 = AqxakddtxEsDJMv930391;     AqxakddtxEsDJMv930391 = AqxakddtxEsDJMv86507449;     AqxakddtxEsDJMv86507449 = AqxakddtxEsDJMv46613251;     AqxakddtxEsDJMv46613251 = AqxakddtxEsDJMv60032575;     AqxakddtxEsDJMv60032575 = AqxakddtxEsDJMv44120922;     AqxakddtxEsDJMv44120922 = AqxakddtxEsDJMv99178309;     AqxakddtxEsDJMv99178309 = AqxakddtxEsDJMv3313873;     AqxakddtxEsDJMv3313873 = AqxakddtxEsDJMv84173065;     AqxakddtxEsDJMv84173065 = AqxakddtxEsDJMv75758769;     AqxakddtxEsDJMv75758769 = AqxakddtxEsDJMv35554069;     AqxakddtxEsDJMv35554069 = AqxakddtxEsDJMv45205538;     AqxakddtxEsDJMv45205538 = AqxakddtxEsDJMv92009238;     AqxakddtxEsDJMv92009238 = AqxakddtxEsDJMv92759796;     AqxakddtxEsDJMv92759796 = AqxakddtxEsDJMv63736370;     AqxakddtxEsDJMv63736370 = AqxakddtxEsDJMv85603258;     AqxakddtxEsDJMv85603258 = AqxakddtxEsDJMv88040684;     AqxakddtxEsDJMv88040684 = AqxakddtxEsDJMv15909991;     AqxakddtxEsDJMv15909991 = AqxakddtxEsDJMv38131707;     AqxakddtxEsDJMv38131707 = AqxakddtxEsDJMv21333832;     AqxakddtxEsDJMv21333832 = AqxakddtxEsDJMv15474698;     AqxakddtxEsDJMv15474698 = AqxakddtxEsDJMv93555154;     AqxakddtxEsDJMv93555154 = AqxakddtxEsDJMv71049038;     AqxakddtxEsDJMv71049038 = AqxakddtxEsDJMv65395661;     AqxakddtxEsDJMv65395661 = AqxakddtxEsDJMv73807265;     AqxakddtxEsDJMv73807265 = AqxakddtxEsDJMv58659891;     AqxakddtxEsDJMv58659891 = AqxakddtxEsDJMv96940899;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void QuekkHsAbyPlQqonutDULYOIBZpoc71708606() {     int sOCEhrsCibUgUqr93453820 = -49067610;    int sOCEhrsCibUgUqr40234972 = -377396113;    int sOCEhrsCibUgUqr72405175 = 80832178;    int sOCEhrsCibUgUqr8966705 = -685542635;    int sOCEhrsCibUgUqr80283038 = -437051050;    int sOCEhrsCibUgUqr36580953 = -699517963;    int sOCEhrsCibUgUqr85191837 = -914421531;    int sOCEhrsCibUgUqr97478155 = -929909762;    int sOCEhrsCibUgUqr61248413 = -534591676;    int sOCEhrsCibUgUqr1302562 = -816463583;    int sOCEhrsCibUgUqr91524176 = -47950477;    int sOCEhrsCibUgUqr60600932 = -157539005;    int sOCEhrsCibUgUqr43920893 = -253108849;    int sOCEhrsCibUgUqr12009747 = -688352027;    int sOCEhrsCibUgUqr67096331 = -52160737;    int sOCEhrsCibUgUqr47321019 = -441564891;    int sOCEhrsCibUgUqr1711831 = -114551740;    int sOCEhrsCibUgUqr78791140 = -130430211;    int sOCEhrsCibUgUqr8081521 = 36326909;    int sOCEhrsCibUgUqr31408694 = 21322522;    int sOCEhrsCibUgUqr90607584 = -637351978;    int sOCEhrsCibUgUqr67036592 = -779341911;    int sOCEhrsCibUgUqr39745503 = 89536362;    int sOCEhrsCibUgUqr86635349 = -926974828;    int sOCEhrsCibUgUqr55920168 = -402098170;    int sOCEhrsCibUgUqr15825251 = -644344050;    int sOCEhrsCibUgUqr15093591 = 41651750;    int sOCEhrsCibUgUqr46882181 = -346015601;    int sOCEhrsCibUgUqr33272655 = -171586268;    int sOCEhrsCibUgUqr83016939 = -949671488;    int sOCEhrsCibUgUqr26564081 = -973273282;    int sOCEhrsCibUgUqr22940132 = 223656;    int sOCEhrsCibUgUqr15844066 = -89230929;    int sOCEhrsCibUgUqr62986577 = -805410484;    int sOCEhrsCibUgUqr65894161 = -579068462;    int sOCEhrsCibUgUqr6615544 = -761407489;    int sOCEhrsCibUgUqr50379243 = -202906301;    int sOCEhrsCibUgUqr30031377 = -678499401;    int sOCEhrsCibUgUqr41897901 = -626258235;    int sOCEhrsCibUgUqr27129272 = -376262737;    int sOCEhrsCibUgUqr82571515 = -184777699;    int sOCEhrsCibUgUqr26191692 = -678313768;    int sOCEhrsCibUgUqr27169322 = -93740886;    int sOCEhrsCibUgUqr15843398 = -282795308;    int sOCEhrsCibUgUqr68764719 = -417803256;    int sOCEhrsCibUgUqr31205265 = -675074168;    int sOCEhrsCibUgUqr37534669 = 63812435;    int sOCEhrsCibUgUqr44373056 = -464814733;    int sOCEhrsCibUgUqr89067820 = 4001978;    int sOCEhrsCibUgUqr48118953 = -396374107;    int sOCEhrsCibUgUqr98997002 = -298567816;    int sOCEhrsCibUgUqr44673101 = -87721686;    int sOCEhrsCibUgUqr87324088 = -398010584;    int sOCEhrsCibUgUqr91147025 = -469168438;    int sOCEhrsCibUgUqr81101022 = -260660607;    int sOCEhrsCibUgUqr26417228 = -269725699;    int sOCEhrsCibUgUqr489469 = -366932475;    int sOCEhrsCibUgUqr85769825 = 7807005;    int sOCEhrsCibUgUqr53046537 = -183444466;    int sOCEhrsCibUgUqr64457787 = -792707001;    int sOCEhrsCibUgUqr21487363 = -641169713;    int sOCEhrsCibUgUqr38309657 = -468405930;    int sOCEhrsCibUgUqr64205500 = -658323494;    int sOCEhrsCibUgUqr78231473 = -584920188;    int sOCEhrsCibUgUqr74738481 = -843190302;    int sOCEhrsCibUgUqr68584045 = 51825866;    int sOCEhrsCibUgUqr44756866 = 31691924;    int sOCEhrsCibUgUqr80934315 = -447698366;    int sOCEhrsCibUgUqr46115586 = -9283565;    int sOCEhrsCibUgUqr60480787 = -290753248;    int sOCEhrsCibUgUqr96941776 = -138658591;    int sOCEhrsCibUgUqr71680454 = -436052340;    int sOCEhrsCibUgUqr36893240 = -504171976;    int sOCEhrsCibUgUqr80952248 = -587410355;    int sOCEhrsCibUgUqr48837178 = -793899780;    int sOCEhrsCibUgUqr64415893 = -959038211;    int sOCEhrsCibUgUqr39867271 = -585601026;    int sOCEhrsCibUgUqr23902105 = -627668331;    int sOCEhrsCibUgUqr17870631 = -409171573;    int sOCEhrsCibUgUqr24714903 = -727024002;    int sOCEhrsCibUgUqr78290581 = -608156485;    int sOCEhrsCibUgUqr70720534 = -493533517;    int sOCEhrsCibUgUqr57814360 = -250017579;    int sOCEhrsCibUgUqr85153701 = -775212162;    int sOCEhrsCibUgUqr84019936 = -551103673;    int sOCEhrsCibUgUqr81890979 = -785551596;    int sOCEhrsCibUgUqr35616043 = -601765761;    int sOCEhrsCibUgUqr24697040 = -620062491;    int sOCEhrsCibUgUqr81885555 = -444749877;    int sOCEhrsCibUgUqr39476934 = -209342764;    int sOCEhrsCibUgUqr6126076 = -294475014;    int sOCEhrsCibUgUqr64609417 = -110713306;    int sOCEhrsCibUgUqr76984840 = -395054935;    int sOCEhrsCibUgUqr77440113 = -833551234;    int sOCEhrsCibUgUqr5641910 = -735093024;    int sOCEhrsCibUgUqr44261859 = -716371769;    int sOCEhrsCibUgUqr61986191 = 80009726;    int sOCEhrsCibUgUqr48937848 = -508820698;    int sOCEhrsCibUgUqr41104917 = -439605006;    int sOCEhrsCibUgUqr180675 = -49067610;     sOCEhrsCibUgUqr93453820 = sOCEhrsCibUgUqr40234972;     sOCEhrsCibUgUqr40234972 = sOCEhrsCibUgUqr72405175;     sOCEhrsCibUgUqr72405175 = sOCEhrsCibUgUqr8966705;     sOCEhrsCibUgUqr8966705 = sOCEhrsCibUgUqr80283038;     sOCEhrsCibUgUqr80283038 = sOCEhrsCibUgUqr36580953;     sOCEhrsCibUgUqr36580953 = sOCEhrsCibUgUqr85191837;     sOCEhrsCibUgUqr85191837 = sOCEhrsCibUgUqr97478155;     sOCEhrsCibUgUqr97478155 = sOCEhrsCibUgUqr61248413;     sOCEhrsCibUgUqr61248413 = sOCEhrsCibUgUqr1302562;     sOCEhrsCibUgUqr1302562 = sOCEhrsCibUgUqr91524176;     sOCEhrsCibUgUqr91524176 = sOCEhrsCibUgUqr60600932;     sOCEhrsCibUgUqr60600932 = sOCEhrsCibUgUqr43920893;     sOCEhrsCibUgUqr43920893 = sOCEhrsCibUgUqr12009747;     sOCEhrsCibUgUqr12009747 = sOCEhrsCibUgUqr67096331;     sOCEhrsCibUgUqr67096331 = sOCEhrsCibUgUqr47321019;     sOCEhrsCibUgUqr47321019 = sOCEhrsCibUgUqr1711831;     sOCEhrsCibUgUqr1711831 = sOCEhrsCibUgUqr78791140;     sOCEhrsCibUgUqr78791140 = sOCEhrsCibUgUqr8081521;     sOCEhrsCibUgUqr8081521 = sOCEhrsCibUgUqr31408694;     sOCEhrsCibUgUqr31408694 = sOCEhrsCibUgUqr90607584;     sOCEhrsCibUgUqr90607584 = sOCEhrsCibUgUqr67036592;     sOCEhrsCibUgUqr67036592 = sOCEhrsCibUgUqr39745503;     sOCEhrsCibUgUqr39745503 = sOCEhrsCibUgUqr86635349;     sOCEhrsCibUgUqr86635349 = sOCEhrsCibUgUqr55920168;     sOCEhrsCibUgUqr55920168 = sOCEhrsCibUgUqr15825251;     sOCEhrsCibUgUqr15825251 = sOCEhrsCibUgUqr15093591;     sOCEhrsCibUgUqr15093591 = sOCEhrsCibUgUqr46882181;     sOCEhrsCibUgUqr46882181 = sOCEhrsCibUgUqr33272655;     sOCEhrsCibUgUqr33272655 = sOCEhrsCibUgUqr83016939;     sOCEhrsCibUgUqr83016939 = sOCEhrsCibUgUqr26564081;     sOCEhrsCibUgUqr26564081 = sOCEhrsCibUgUqr22940132;     sOCEhrsCibUgUqr22940132 = sOCEhrsCibUgUqr15844066;     sOCEhrsCibUgUqr15844066 = sOCEhrsCibUgUqr62986577;     sOCEhrsCibUgUqr62986577 = sOCEhrsCibUgUqr65894161;     sOCEhrsCibUgUqr65894161 = sOCEhrsCibUgUqr6615544;     sOCEhrsCibUgUqr6615544 = sOCEhrsCibUgUqr50379243;     sOCEhrsCibUgUqr50379243 = sOCEhrsCibUgUqr30031377;     sOCEhrsCibUgUqr30031377 = sOCEhrsCibUgUqr41897901;     sOCEhrsCibUgUqr41897901 = sOCEhrsCibUgUqr27129272;     sOCEhrsCibUgUqr27129272 = sOCEhrsCibUgUqr82571515;     sOCEhrsCibUgUqr82571515 = sOCEhrsCibUgUqr26191692;     sOCEhrsCibUgUqr26191692 = sOCEhrsCibUgUqr27169322;     sOCEhrsCibUgUqr27169322 = sOCEhrsCibUgUqr15843398;     sOCEhrsCibUgUqr15843398 = sOCEhrsCibUgUqr68764719;     sOCEhrsCibUgUqr68764719 = sOCEhrsCibUgUqr31205265;     sOCEhrsCibUgUqr31205265 = sOCEhrsCibUgUqr37534669;     sOCEhrsCibUgUqr37534669 = sOCEhrsCibUgUqr44373056;     sOCEhrsCibUgUqr44373056 = sOCEhrsCibUgUqr89067820;     sOCEhrsCibUgUqr89067820 = sOCEhrsCibUgUqr48118953;     sOCEhrsCibUgUqr48118953 = sOCEhrsCibUgUqr98997002;     sOCEhrsCibUgUqr98997002 = sOCEhrsCibUgUqr44673101;     sOCEhrsCibUgUqr44673101 = sOCEhrsCibUgUqr87324088;     sOCEhrsCibUgUqr87324088 = sOCEhrsCibUgUqr91147025;     sOCEhrsCibUgUqr91147025 = sOCEhrsCibUgUqr81101022;     sOCEhrsCibUgUqr81101022 = sOCEhrsCibUgUqr26417228;     sOCEhrsCibUgUqr26417228 = sOCEhrsCibUgUqr489469;     sOCEhrsCibUgUqr489469 = sOCEhrsCibUgUqr85769825;     sOCEhrsCibUgUqr85769825 = sOCEhrsCibUgUqr53046537;     sOCEhrsCibUgUqr53046537 = sOCEhrsCibUgUqr64457787;     sOCEhrsCibUgUqr64457787 = sOCEhrsCibUgUqr21487363;     sOCEhrsCibUgUqr21487363 = sOCEhrsCibUgUqr38309657;     sOCEhrsCibUgUqr38309657 = sOCEhrsCibUgUqr64205500;     sOCEhrsCibUgUqr64205500 = sOCEhrsCibUgUqr78231473;     sOCEhrsCibUgUqr78231473 = sOCEhrsCibUgUqr74738481;     sOCEhrsCibUgUqr74738481 = sOCEhrsCibUgUqr68584045;     sOCEhrsCibUgUqr68584045 = sOCEhrsCibUgUqr44756866;     sOCEhrsCibUgUqr44756866 = sOCEhrsCibUgUqr80934315;     sOCEhrsCibUgUqr80934315 = sOCEhrsCibUgUqr46115586;     sOCEhrsCibUgUqr46115586 = sOCEhrsCibUgUqr60480787;     sOCEhrsCibUgUqr60480787 = sOCEhrsCibUgUqr96941776;     sOCEhrsCibUgUqr96941776 = sOCEhrsCibUgUqr71680454;     sOCEhrsCibUgUqr71680454 = sOCEhrsCibUgUqr36893240;     sOCEhrsCibUgUqr36893240 = sOCEhrsCibUgUqr80952248;     sOCEhrsCibUgUqr80952248 = sOCEhrsCibUgUqr48837178;     sOCEhrsCibUgUqr48837178 = sOCEhrsCibUgUqr64415893;     sOCEhrsCibUgUqr64415893 = sOCEhrsCibUgUqr39867271;     sOCEhrsCibUgUqr39867271 = sOCEhrsCibUgUqr23902105;     sOCEhrsCibUgUqr23902105 = sOCEhrsCibUgUqr17870631;     sOCEhrsCibUgUqr17870631 = sOCEhrsCibUgUqr24714903;     sOCEhrsCibUgUqr24714903 = sOCEhrsCibUgUqr78290581;     sOCEhrsCibUgUqr78290581 = sOCEhrsCibUgUqr70720534;     sOCEhrsCibUgUqr70720534 = sOCEhrsCibUgUqr57814360;     sOCEhrsCibUgUqr57814360 = sOCEhrsCibUgUqr85153701;     sOCEhrsCibUgUqr85153701 = sOCEhrsCibUgUqr84019936;     sOCEhrsCibUgUqr84019936 = sOCEhrsCibUgUqr81890979;     sOCEhrsCibUgUqr81890979 = sOCEhrsCibUgUqr35616043;     sOCEhrsCibUgUqr35616043 = sOCEhrsCibUgUqr24697040;     sOCEhrsCibUgUqr24697040 = sOCEhrsCibUgUqr81885555;     sOCEhrsCibUgUqr81885555 = sOCEhrsCibUgUqr39476934;     sOCEhrsCibUgUqr39476934 = sOCEhrsCibUgUqr6126076;     sOCEhrsCibUgUqr6126076 = sOCEhrsCibUgUqr64609417;     sOCEhrsCibUgUqr64609417 = sOCEhrsCibUgUqr76984840;     sOCEhrsCibUgUqr76984840 = sOCEhrsCibUgUqr77440113;     sOCEhrsCibUgUqr77440113 = sOCEhrsCibUgUqr5641910;     sOCEhrsCibUgUqr5641910 = sOCEhrsCibUgUqr44261859;     sOCEhrsCibUgUqr44261859 = sOCEhrsCibUgUqr61986191;     sOCEhrsCibUgUqr61986191 = sOCEhrsCibUgUqr48937848;     sOCEhrsCibUgUqr48937848 = sOCEhrsCibUgUqr41104917;     sOCEhrsCibUgUqr41104917 = sOCEhrsCibUgUqr180675;     sOCEhrsCibUgUqr180675 = sOCEhrsCibUgUqr93453820;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void SXWJLNxvkQuCaGZADiWwOXvRBczUA73897945() {     int rEDysjmdgEDyFFd89966741 = -47868298;    int rEDysjmdgEDyFFd55317797 = -330084415;    int rEDysjmdgEDyFFd28087486 = -767270588;    int rEDysjmdgEDyFFd21515553 = -54356426;    int rEDysjmdgEDyFFd48935428 = 7361954;    int rEDysjmdgEDyFFd73827825 = -730045539;    int rEDysjmdgEDyFFd59006491 = 20660494;    int rEDysjmdgEDyFFd32930326 = 57206054;    int rEDysjmdgEDyFFd65502669 = -787009430;    int rEDysjmdgEDyFFd55307098 = -475188796;    int rEDysjmdgEDyFFd51740292 = -23185714;    int rEDysjmdgEDyFFd75617369 = -726271911;    int rEDysjmdgEDyFFd49191591 = -538894512;    int rEDysjmdgEDyFFd79084860 = -97873562;    int rEDysjmdgEDyFFd71996088 = 96512553;    int rEDysjmdgEDyFFd60277843 = -200186106;    int rEDysjmdgEDyFFd29402679 = -27398785;    int rEDysjmdgEDyFFd14926629 = -747956276;    int rEDysjmdgEDyFFd90004496 = -452800392;    int rEDysjmdgEDyFFd72133952 = -988202308;    int rEDysjmdgEDyFFd98286439 = -847901576;    int rEDysjmdgEDyFFd7938350 = -57359604;    int rEDysjmdgEDyFFd29242466 = -191383349;    int rEDysjmdgEDyFFd10460120 = -886092232;    int rEDysjmdgEDyFFd34195175 = 72476071;    int rEDysjmdgEDyFFd91849615 = -823639312;    int rEDysjmdgEDyFFd62990275 = -885530097;    int rEDysjmdgEDyFFd39132544 = -1771700;    int rEDysjmdgEDyFFd15468626 = -36964251;    int rEDysjmdgEDyFFd83271446 = -314448330;    int rEDysjmdgEDyFFd35658160 = -175241435;    int rEDysjmdgEDyFFd58690315 = -331782364;    int rEDysjmdgEDyFFd33288505 = 20672303;    int rEDysjmdgEDyFFd75646245 = -625920483;    int rEDysjmdgEDyFFd75378999 = -793733988;    int rEDysjmdgEDyFFd50286797 = 61515803;    int rEDysjmdgEDyFFd30936210 = -813330259;    int rEDysjmdgEDyFFd3158351 = -499631370;    int rEDysjmdgEDyFFd90632207 = -967326986;    int rEDysjmdgEDyFFd6646671 = -912610619;    int rEDysjmdgEDyFFd14842511 = -716319869;    int rEDysjmdgEDyFFd70385044 = -85754730;    int rEDysjmdgEDyFFd14711258 = -590013458;    int rEDysjmdgEDyFFd28051506 = -424429062;    int rEDysjmdgEDyFFd34751434 = -949957594;    int rEDysjmdgEDyFFd28886292 = 92598036;    int rEDysjmdgEDyFFd34446762 = -214708213;    int rEDysjmdgEDyFFd24863080 = -627908732;    int rEDysjmdgEDyFFd7676889 = -627502217;    int rEDysjmdgEDyFFd20919993 = -745913562;    int rEDysjmdgEDyFFd50785643 = -927947022;    int rEDysjmdgEDyFFd17081740 = -669886936;    int rEDysjmdgEDyFFd79467467 = -89919228;    int rEDysjmdgEDyFFd76654221 = -919369442;    int rEDysjmdgEDyFFd75611505 = -693382400;    int rEDysjmdgEDyFFd82028391 = -990508695;    int rEDysjmdgEDyFFd26075331 = -38701067;    int rEDysjmdgEDyFFd17627367 = -881178356;    int rEDysjmdgEDyFFd87320377 = -26832497;    int rEDysjmdgEDyFFd57085813 = -168998734;    int rEDysjmdgEDyFFd10837550 = -844515443;    int rEDysjmdgEDyFFd19873948 = -977567807;    int rEDysjmdgEDyFFd17461700 = -905829695;    int rEDysjmdgEDyFFd82231223 = -372561100;    int rEDysjmdgEDyFFd19648938 = -199947362;    int rEDysjmdgEDyFFd93049977 = -691403350;    int rEDysjmdgEDyFFd42328865 = -646944214;    int rEDysjmdgEDyFFd73545345 = -912974029;    int rEDysjmdgEDyFFd3705861 = -304139574;    int rEDysjmdgEDyFFd21709291 = -965003250;    int rEDysjmdgEDyFFd29341633 = -386855848;    int rEDysjmdgEDyFFd26244328 = -527767416;    int rEDysjmdgEDyFFd24294422 = -780629291;    int rEDysjmdgEDyFFd83357825 = -540189773;    int rEDysjmdgEDyFFd57291442 = -171882440;    int rEDysjmdgEDyFFd27901396 = -662146846;    int rEDysjmdgEDyFFd93227092 = -467346146;    int rEDysjmdgEDyFFd1190960 = -766954288;    int rEDysjmdgEDyFFd75708685 = -936134639;    int rEDysjmdgEDyFFd5308884 = 79878034;    int rEDysjmdgEDyFFd57402853 = -508931100;    int rEDysjmdgEDyFFd38127196 = -157621365;    int rEDysjmdgEDyFFd31455655 = -374269483;    int rEDysjmdgEDyFFd94548633 = -291050690;    int rEDysjmdgEDyFFd32485804 = -386501309;    int rEDysjmdgEDyFFd18576421 = -505354499;    int rEDysjmdgEDyFFd79222847 = -141863137;    int rEDysjmdgEDyFFd56634283 = -59958255;    int rEDysjmdgEDyFFd34741 = -932538084;    int rEDysjmdgEDyFFd93350608 = -803225294;    int rEDysjmdgEDyFFd24211466 = -899783131;    int rEDysjmdgEDyFFd13308844 = -932151903;    int rEDysjmdgEDyFFd15837974 = -372798873;    int rEDysjmdgEDyFFd33546394 = -698328252;    int rEDysjmdgEDyFFd95809121 = 31904823;    int rEDysjmdgEDyFFd94968563 = -738752063;    int rEDysjmdgEDyFFd52923344 = -179925035;    int rEDysjmdgEDyFFd32480035 = -117452359;    int rEDysjmdgEDyFFd8402569 = -124481700;    int rEDysjmdgEDyFFd41701457 = -47868298;     rEDysjmdgEDyFFd89966741 = rEDysjmdgEDyFFd55317797;     rEDysjmdgEDyFFd55317797 = rEDysjmdgEDyFFd28087486;     rEDysjmdgEDyFFd28087486 = rEDysjmdgEDyFFd21515553;     rEDysjmdgEDyFFd21515553 = rEDysjmdgEDyFFd48935428;     rEDysjmdgEDyFFd48935428 = rEDysjmdgEDyFFd73827825;     rEDysjmdgEDyFFd73827825 = rEDysjmdgEDyFFd59006491;     rEDysjmdgEDyFFd59006491 = rEDysjmdgEDyFFd32930326;     rEDysjmdgEDyFFd32930326 = rEDysjmdgEDyFFd65502669;     rEDysjmdgEDyFFd65502669 = rEDysjmdgEDyFFd55307098;     rEDysjmdgEDyFFd55307098 = rEDysjmdgEDyFFd51740292;     rEDysjmdgEDyFFd51740292 = rEDysjmdgEDyFFd75617369;     rEDysjmdgEDyFFd75617369 = rEDysjmdgEDyFFd49191591;     rEDysjmdgEDyFFd49191591 = rEDysjmdgEDyFFd79084860;     rEDysjmdgEDyFFd79084860 = rEDysjmdgEDyFFd71996088;     rEDysjmdgEDyFFd71996088 = rEDysjmdgEDyFFd60277843;     rEDysjmdgEDyFFd60277843 = rEDysjmdgEDyFFd29402679;     rEDysjmdgEDyFFd29402679 = rEDysjmdgEDyFFd14926629;     rEDysjmdgEDyFFd14926629 = rEDysjmdgEDyFFd90004496;     rEDysjmdgEDyFFd90004496 = rEDysjmdgEDyFFd72133952;     rEDysjmdgEDyFFd72133952 = rEDysjmdgEDyFFd98286439;     rEDysjmdgEDyFFd98286439 = rEDysjmdgEDyFFd7938350;     rEDysjmdgEDyFFd7938350 = rEDysjmdgEDyFFd29242466;     rEDysjmdgEDyFFd29242466 = rEDysjmdgEDyFFd10460120;     rEDysjmdgEDyFFd10460120 = rEDysjmdgEDyFFd34195175;     rEDysjmdgEDyFFd34195175 = rEDysjmdgEDyFFd91849615;     rEDysjmdgEDyFFd91849615 = rEDysjmdgEDyFFd62990275;     rEDysjmdgEDyFFd62990275 = rEDysjmdgEDyFFd39132544;     rEDysjmdgEDyFFd39132544 = rEDysjmdgEDyFFd15468626;     rEDysjmdgEDyFFd15468626 = rEDysjmdgEDyFFd83271446;     rEDysjmdgEDyFFd83271446 = rEDysjmdgEDyFFd35658160;     rEDysjmdgEDyFFd35658160 = rEDysjmdgEDyFFd58690315;     rEDysjmdgEDyFFd58690315 = rEDysjmdgEDyFFd33288505;     rEDysjmdgEDyFFd33288505 = rEDysjmdgEDyFFd75646245;     rEDysjmdgEDyFFd75646245 = rEDysjmdgEDyFFd75378999;     rEDysjmdgEDyFFd75378999 = rEDysjmdgEDyFFd50286797;     rEDysjmdgEDyFFd50286797 = rEDysjmdgEDyFFd30936210;     rEDysjmdgEDyFFd30936210 = rEDysjmdgEDyFFd3158351;     rEDysjmdgEDyFFd3158351 = rEDysjmdgEDyFFd90632207;     rEDysjmdgEDyFFd90632207 = rEDysjmdgEDyFFd6646671;     rEDysjmdgEDyFFd6646671 = rEDysjmdgEDyFFd14842511;     rEDysjmdgEDyFFd14842511 = rEDysjmdgEDyFFd70385044;     rEDysjmdgEDyFFd70385044 = rEDysjmdgEDyFFd14711258;     rEDysjmdgEDyFFd14711258 = rEDysjmdgEDyFFd28051506;     rEDysjmdgEDyFFd28051506 = rEDysjmdgEDyFFd34751434;     rEDysjmdgEDyFFd34751434 = rEDysjmdgEDyFFd28886292;     rEDysjmdgEDyFFd28886292 = rEDysjmdgEDyFFd34446762;     rEDysjmdgEDyFFd34446762 = rEDysjmdgEDyFFd24863080;     rEDysjmdgEDyFFd24863080 = rEDysjmdgEDyFFd7676889;     rEDysjmdgEDyFFd7676889 = rEDysjmdgEDyFFd20919993;     rEDysjmdgEDyFFd20919993 = rEDysjmdgEDyFFd50785643;     rEDysjmdgEDyFFd50785643 = rEDysjmdgEDyFFd17081740;     rEDysjmdgEDyFFd17081740 = rEDysjmdgEDyFFd79467467;     rEDysjmdgEDyFFd79467467 = rEDysjmdgEDyFFd76654221;     rEDysjmdgEDyFFd76654221 = rEDysjmdgEDyFFd75611505;     rEDysjmdgEDyFFd75611505 = rEDysjmdgEDyFFd82028391;     rEDysjmdgEDyFFd82028391 = rEDysjmdgEDyFFd26075331;     rEDysjmdgEDyFFd26075331 = rEDysjmdgEDyFFd17627367;     rEDysjmdgEDyFFd17627367 = rEDysjmdgEDyFFd87320377;     rEDysjmdgEDyFFd87320377 = rEDysjmdgEDyFFd57085813;     rEDysjmdgEDyFFd57085813 = rEDysjmdgEDyFFd10837550;     rEDysjmdgEDyFFd10837550 = rEDysjmdgEDyFFd19873948;     rEDysjmdgEDyFFd19873948 = rEDysjmdgEDyFFd17461700;     rEDysjmdgEDyFFd17461700 = rEDysjmdgEDyFFd82231223;     rEDysjmdgEDyFFd82231223 = rEDysjmdgEDyFFd19648938;     rEDysjmdgEDyFFd19648938 = rEDysjmdgEDyFFd93049977;     rEDysjmdgEDyFFd93049977 = rEDysjmdgEDyFFd42328865;     rEDysjmdgEDyFFd42328865 = rEDysjmdgEDyFFd73545345;     rEDysjmdgEDyFFd73545345 = rEDysjmdgEDyFFd3705861;     rEDysjmdgEDyFFd3705861 = rEDysjmdgEDyFFd21709291;     rEDysjmdgEDyFFd21709291 = rEDysjmdgEDyFFd29341633;     rEDysjmdgEDyFFd29341633 = rEDysjmdgEDyFFd26244328;     rEDysjmdgEDyFFd26244328 = rEDysjmdgEDyFFd24294422;     rEDysjmdgEDyFFd24294422 = rEDysjmdgEDyFFd83357825;     rEDysjmdgEDyFFd83357825 = rEDysjmdgEDyFFd57291442;     rEDysjmdgEDyFFd57291442 = rEDysjmdgEDyFFd27901396;     rEDysjmdgEDyFFd27901396 = rEDysjmdgEDyFFd93227092;     rEDysjmdgEDyFFd93227092 = rEDysjmdgEDyFFd1190960;     rEDysjmdgEDyFFd1190960 = rEDysjmdgEDyFFd75708685;     rEDysjmdgEDyFFd75708685 = rEDysjmdgEDyFFd5308884;     rEDysjmdgEDyFFd5308884 = rEDysjmdgEDyFFd57402853;     rEDysjmdgEDyFFd57402853 = rEDysjmdgEDyFFd38127196;     rEDysjmdgEDyFFd38127196 = rEDysjmdgEDyFFd31455655;     rEDysjmdgEDyFFd31455655 = rEDysjmdgEDyFFd94548633;     rEDysjmdgEDyFFd94548633 = rEDysjmdgEDyFFd32485804;     rEDysjmdgEDyFFd32485804 = rEDysjmdgEDyFFd18576421;     rEDysjmdgEDyFFd18576421 = rEDysjmdgEDyFFd79222847;     rEDysjmdgEDyFFd79222847 = rEDysjmdgEDyFFd56634283;     rEDysjmdgEDyFFd56634283 = rEDysjmdgEDyFFd34741;     rEDysjmdgEDyFFd34741 = rEDysjmdgEDyFFd93350608;     rEDysjmdgEDyFFd93350608 = rEDysjmdgEDyFFd24211466;     rEDysjmdgEDyFFd24211466 = rEDysjmdgEDyFFd13308844;     rEDysjmdgEDyFFd13308844 = rEDysjmdgEDyFFd15837974;     rEDysjmdgEDyFFd15837974 = rEDysjmdgEDyFFd33546394;     rEDysjmdgEDyFFd33546394 = rEDysjmdgEDyFFd95809121;     rEDysjmdgEDyFFd95809121 = rEDysjmdgEDyFFd94968563;     rEDysjmdgEDyFFd94968563 = rEDysjmdgEDyFFd52923344;     rEDysjmdgEDyFFd52923344 = rEDysjmdgEDyFFd32480035;     rEDysjmdgEDyFFd32480035 = rEDysjmdgEDyFFd8402569;     rEDysjmdgEDyFFd8402569 = rEDysjmdgEDyFFd41701457;     rEDysjmdgEDyFFd41701457 = rEDysjmdgEDyFFd89966741;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void kcnMWgEHYQqCtKabaxLpAjqhMroAJ76087283() {     int CDrJAfCtuhKZDEg86479662 = -46668987;    int CDrJAfCtuhKZDEg70400622 = -282772717;    int CDrJAfCtuhKZDEg83769797 = -515373354;    int CDrJAfCtuhKZDEg34064401 = -523170217;    int CDrJAfCtuhKZDEg17587819 = -648225042;    int CDrJAfCtuhKZDEg11074698 = -760573116;    int CDrJAfCtuhKZDEg32821144 = -144257481;    int CDrJAfCtuhKZDEg68382496 = -55678130;    int CDrJAfCtuhKZDEg69756926 = 60572816;    int CDrJAfCtuhKZDEg9311635 = -133914009;    int CDrJAfCtuhKZDEg11956408 = 1579049;    int CDrJAfCtuhKZDEg90633806 = -195004816;    int CDrJAfCtuhKZDEg54462288 = -824680176;    int CDrJAfCtuhKZDEg46159973 = -607395097;    int CDrJAfCtuhKZDEg76895845 = -854814157;    int CDrJAfCtuhKZDEg73234666 = 41192678;    int CDrJAfCtuhKZDEg57093528 = 59754171;    int CDrJAfCtuhKZDEg51062117 = -265482341;    int CDrJAfCtuhKZDEg71927473 = -941927692;    int CDrJAfCtuhKZDEg12859212 = -897727139;    int CDrJAfCtuhKZDEg5965295 = 41548826;    int CDrJAfCtuhKZDEg48840107 = -435377297;    int CDrJAfCtuhKZDEg18739428 = -472303059;    int CDrJAfCtuhKZDEg34284890 = -845209636;    int CDrJAfCtuhKZDEg12470183 = -552949688;    int CDrJAfCtuhKZDEg67873980 = 97065425;    int CDrJAfCtuhKZDEg10886960 = -712711944;    int CDrJAfCtuhKZDEg31382906 = -757527799;    int CDrJAfCtuhKZDEg97664597 = 97657766;    int CDrJAfCtuhKZDEg83525954 = -779225173;    int CDrJAfCtuhKZDEg44752240 = -477209588;    int CDrJAfCtuhKZDEg94440498 = -663788385;    int CDrJAfCtuhKZDEg50732943 = -969424465;    int CDrJAfCtuhKZDEg88305914 = -446430483;    int CDrJAfCtuhKZDEg84863838 = 91600486;    int CDrJAfCtuhKZDEg93958051 = -215560906;    int CDrJAfCtuhKZDEg11493177 = -323754216;    int CDrJAfCtuhKZDEg76285325 = -320763338;    int CDrJAfCtuhKZDEg39366514 = -208395736;    int CDrJAfCtuhKZDEg86164070 = -348958501;    int CDrJAfCtuhKZDEg47113505 = -147862040;    int CDrJAfCtuhKZDEg14578396 = -593195693;    int CDrJAfCtuhKZDEg2253194 = 13713970;    int CDrJAfCtuhKZDEg40259614 = -566062815;    int CDrJAfCtuhKZDEg738150 = -382111931;    int CDrJAfCtuhKZDEg26567319 = -239729760;    int CDrJAfCtuhKZDEg31358855 = -493228861;    int CDrJAfCtuhKZDEg5353103 = -791002732;    int CDrJAfCtuhKZDEg26285957 = -159006412;    int CDrJAfCtuhKZDEg93721031 = 4546983;    int CDrJAfCtuhKZDEg2574284 = -457326229;    int CDrJAfCtuhKZDEg89490377 = -152052186;    int CDrJAfCtuhKZDEg71610845 = -881827872;    int CDrJAfCtuhKZDEg62161417 = -269570446;    int CDrJAfCtuhKZDEg70121989 = -26104192;    int CDrJAfCtuhKZDEg37639555 = -611291690;    int CDrJAfCtuhKZDEg51661194 = -810469659;    int CDrJAfCtuhKZDEg49484907 = -670163718;    int CDrJAfCtuhKZDEg21594219 = -970220529;    int CDrJAfCtuhKZDEg49713838 = -645290468;    int CDrJAfCtuhKZDEg187738 = 52138828;    int CDrJAfCtuhKZDEg1438238 = -386729683;    int CDrJAfCtuhKZDEg70717898 = -53335896;    int CDrJAfCtuhKZDEg86230972 = -160202011;    int CDrJAfCtuhKZDEg64559395 = -656704421;    int CDrJAfCtuhKZDEg17515910 = -334632567;    int CDrJAfCtuhKZDEg39900863 = -225580352;    int CDrJAfCtuhKZDEg66156374 = -278249693;    int CDrJAfCtuhKZDEg61296135 = -598995583;    int CDrJAfCtuhKZDEg82937794 = -539253251;    int CDrJAfCtuhKZDEg61741490 = -635053106;    int CDrJAfCtuhKZDEg80808202 = -619482492;    int CDrJAfCtuhKZDEg11695603 = 42913394;    int CDrJAfCtuhKZDEg85763402 = -492969191;    int CDrJAfCtuhKZDEg65745706 = -649865099;    int CDrJAfCtuhKZDEg91386898 = -365255482;    int CDrJAfCtuhKZDEg46586914 = -349091267;    int CDrJAfCtuhKZDEg78479813 = -906240244;    int CDrJAfCtuhKZDEg33546741 = -363097706;    int CDrJAfCtuhKZDEg85902863 = -213219929;    int CDrJAfCtuhKZDEg36515125 = -409705714;    int CDrJAfCtuhKZDEg5533858 = -921709213;    int CDrJAfCtuhKZDEg5096950 = -498521387;    int CDrJAfCtuhKZDEg3943566 = -906889218;    int CDrJAfCtuhKZDEg80951671 = -221898945;    int CDrJAfCtuhKZDEg55261862 = -225157402;    int CDrJAfCtuhKZDEg22829653 = -781960513;    int CDrJAfCtuhKZDEg88571526 = -599854019;    int CDrJAfCtuhKZDEg18183925 = -320326291;    int CDrJAfCtuhKZDEg47224284 = -297107824;    int CDrJAfCtuhKZDEg42296857 = -405091248;    int CDrJAfCtuhKZDEg62008269 = -653590499;    int CDrJAfCtuhKZDEg54691107 = -350542810;    int CDrJAfCtuhKZDEg89652675 = -563105269;    int CDrJAfCtuhKZDEg85976333 = -301097329;    int CDrJAfCtuhKZDEg45675268 = -761132358;    int CDrJAfCtuhKZDEg43860498 = -439859797;    int CDrJAfCtuhKZDEg16022222 = -826084020;    int CDrJAfCtuhKZDEg75700219 = -909358394;    int CDrJAfCtuhKZDEg83222239 = -46668987;     CDrJAfCtuhKZDEg86479662 = CDrJAfCtuhKZDEg70400622;     CDrJAfCtuhKZDEg70400622 = CDrJAfCtuhKZDEg83769797;     CDrJAfCtuhKZDEg83769797 = CDrJAfCtuhKZDEg34064401;     CDrJAfCtuhKZDEg34064401 = CDrJAfCtuhKZDEg17587819;     CDrJAfCtuhKZDEg17587819 = CDrJAfCtuhKZDEg11074698;     CDrJAfCtuhKZDEg11074698 = CDrJAfCtuhKZDEg32821144;     CDrJAfCtuhKZDEg32821144 = CDrJAfCtuhKZDEg68382496;     CDrJAfCtuhKZDEg68382496 = CDrJAfCtuhKZDEg69756926;     CDrJAfCtuhKZDEg69756926 = CDrJAfCtuhKZDEg9311635;     CDrJAfCtuhKZDEg9311635 = CDrJAfCtuhKZDEg11956408;     CDrJAfCtuhKZDEg11956408 = CDrJAfCtuhKZDEg90633806;     CDrJAfCtuhKZDEg90633806 = CDrJAfCtuhKZDEg54462288;     CDrJAfCtuhKZDEg54462288 = CDrJAfCtuhKZDEg46159973;     CDrJAfCtuhKZDEg46159973 = CDrJAfCtuhKZDEg76895845;     CDrJAfCtuhKZDEg76895845 = CDrJAfCtuhKZDEg73234666;     CDrJAfCtuhKZDEg73234666 = CDrJAfCtuhKZDEg57093528;     CDrJAfCtuhKZDEg57093528 = CDrJAfCtuhKZDEg51062117;     CDrJAfCtuhKZDEg51062117 = CDrJAfCtuhKZDEg71927473;     CDrJAfCtuhKZDEg71927473 = CDrJAfCtuhKZDEg12859212;     CDrJAfCtuhKZDEg12859212 = CDrJAfCtuhKZDEg5965295;     CDrJAfCtuhKZDEg5965295 = CDrJAfCtuhKZDEg48840107;     CDrJAfCtuhKZDEg48840107 = CDrJAfCtuhKZDEg18739428;     CDrJAfCtuhKZDEg18739428 = CDrJAfCtuhKZDEg34284890;     CDrJAfCtuhKZDEg34284890 = CDrJAfCtuhKZDEg12470183;     CDrJAfCtuhKZDEg12470183 = CDrJAfCtuhKZDEg67873980;     CDrJAfCtuhKZDEg67873980 = CDrJAfCtuhKZDEg10886960;     CDrJAfCtuhKZDEg10886960 = CDrJAfCtuhKZDEg31382906;     CDrJAfCtuhKZDEg31382906 = CDrJAfCtuhKZDEg97664597;     CDrJAfCtuhKZDEg97664597 = CDrJAfCtuhKZDEg83525954;     CDrJAfCtuhKZDEg83525954 = CDrJAfCtuhKZDEg44752240;     CDrJAfCtuhKZDEg44752240 = CDrJAfCtuhKZDEg94440498;     CDrJAfCtuhKZDEg94440498 = CDrJAfCtuhKZDEg50732943;     CDrJAfCtuhKZDEg50732943 = CDrJAfCtuhKZDEg88305914;     CDrJAfCtuhKZDEg88305914 = CDrJAfCtuhKZDEg84863838;     CDrJAfCtuhKZDEg84863838 = CDrJAfCtuhKZDEg93958051;     CDrJAfCtuhKZDEg93958051 = CDrJAfCtuhKZDEg11493177;     CDrJAfCtuhKZDEg11493177 = CDrJAfCtuhKZDEg76285325;     CDrJAfCtuhKZDEg76285325 = CDrJAfCtuhKZDEg39366514;     CDrJAfCtuhKZDEg39366514 = CDrJAfCtuhKZDEg86164070;     CDrJAfCtuhKZDEg86164070 = CDrJAfCtuhKZDEg47113505;     CDrJAfCtuhKZDEg47113505 = CDrJAfCtuhKZDEg14578396;     CDrJAfCtuhKZDEg14578396 = CDrJAfCtuhKZDEg2253194;     CDrJAfCtuhKZDEg2253194 = CDrJAfCtuhKZDEg40259614;     CDrJAfCtuhKZDEg40259614 = CDrJAfCtuhKZDEg738150;     CDrJAfCtuhKZDEg738150 = CDrJAfCtuhKZDEg26567319;     CDrJAfCtuhKZDEg26567319 = CDrJAfCtuhKZDEg31358855;     CDrJAfCtuhKZDEg31358855 = CDrJAfCtuhKZDEg5353103;     CDrJAfCtuhKZDEg5353103 = CDrJAfCtuhKZDEg26285957;     CDrJAfCtuhKZDEg26285957 = CDrJAfCtuhKZDEg93721031;     CDrJAfCtuhKZDEg93721031 = CDrJAfCtuhKZDEg2574284;     CDrJAfCtuhKZDEg2574284 = CDrJAfCtuhKZDEg89490377;     CDrJAfCtuhKZDEg89490377 = CDrJAfCtuhKZDEg71610845;     CDrJAfCtuhKZDEg71610845 = CDrJAfCtuhKZDEg62161417;     CDrJAfCtuhKZDEg62161417 = CDrJAfCtuhKZDEg70121989;     CDrJAfCtuhKZDEg70121989 = CDrJAfCtuhKZDEg37639555;     CDrJAfCtuhKZDEg37639555 = CDrJAfCtuhKZDEg51661194;     CDrJAfCtuhKZDEg51661194 = CDrJAfCtuhKZDEg49484907;     CDrJAfCtuhKZDEg49484907 = CDrJAfCtuhKZDEg21594219;     CDrJAfCtuhKZDEg21594219 = CDrJAfCtuhKZDEg49713838;     CDrJAfCtuhKZDEg49713838 = CDrJAfCtuhKZDEg187738;     CDrJAfCtuhKZDEg187738 = CDrJAfCtuhKZDEg1438238;     CDrJAfCtuhKZDEg1438238 = CDrJAfCtuhKZDEg70717898;     CDrJAfCtuhKZDEg70717898 = CDrJAfCtuhKZDEg86230972;     CDrJAfCtuhKZDEg86230972 = CDrJAfCtuhKZDEg64559395;     CDrJAfCtuhKZDEg64559395 = CDrJAfCtuhKZDEg17515910;     CDrJAfCtuhKZDEg17515910 = CDrJAfCtuhKZDEg39900863;     CDrJAfCtuhKZDEg39900863 = CDrJAfCtuhKZDEg66156374;     CDrJAfCtuhKZDEg66156374 = CDrJAfCtuhKZDEg61296135;     CDrJAfCtuhKZDEg61296135 = CDrJAfCtuhKZDEg82937794;     CDrJAfCtuhKZDEg82937794 = CDrJAfCtuhKZDEg61741490;     CDrJAfCtuhKZDEg61741490 = CDrJAfCtuhKZDEg80808202;     CDrJAfCtuhKZDEg80808202 = CDrJAfCtuhKZDEg11695603;     CDrJAfCtuhKZDEg11695603 = CDrJAfCtuhKZDEg85763402;     CDrJAfCtuhKZDEg85763402 = CDrJAfCtuhKZDEg65745706;     CDrJAfCtuhKZDEg65745706 = CDrJAfCtuhKZDEg91386898;     CDrJAfCtuhKZDEg91386898 = CDrJAfCtuhKZDEg46586914;     CDrJAfCtuhKZDEg46586914 = CDrJAfCtuhKZDEg78479813;     CDrJAfCtuhKZDEg78479813 = CDrJAfCtuhKZDEg33546741;     CDrJAfCtuhKZDEg33546741 = CDrJAfCtuhKZDEg85902863;     CDrJAfCtuhKZDEg85902863 = CDrJAfCtuhKZDEg36515125;     CDrJAfCtuhKZDEg36515125 = CDrJAfCtuhKZDEg5533858;     CDrJAfCtuhKZDEg5533858 = CDrJAfCtuhKZDEg5096950;     CDrJAfCtuhKZDEg5096950 = CDrJAfCtuhKZDEg3943566;     CDrJAfCtuhKZDEg3943566 = CDrJAfCtuhKZDEg80951671;     CDrJAfCtuhKZDEg80951671 = CDrJAfCtuhKZDEg55261862;     CDrJAfCtuhKZDEg55261862 = CDrJAfCtuhKZDEg22829653;     CDrJAfCtuhKZDEg22829653 = CDrJAfCtuhKZDEg88571526;     CDrJAfCtuhKZDEg88571526 = CDrJAfCtuhKZDEg18183925;     CDrJAfCtuhKZDEg18183925 = CDrJAfCtuhKZDEg47224284;     CDrJAfCtuhKZDEg47224284 = CDrJAfCtuhKZDEg42296857;     CDrJAfCtuhKZDEg42296857 = CDrJAfCtuhKZDEg62008269;     CDrJAfCtuhKZDEg62008269 = CDrJAfCtuhKZDEg54691107;     CDrJAfCtuhKZDEg54691107 = CDrJAfCtuhKZDEg89652675;     CDrJAfCtuhKZDEg89652675 = CDrJAfCtuhKZDEg85976333;     CDrJAfCtuhKZDEg85976333 = CDrJAfCtuhKZDEg45675268;     CDrJAfCtuhKZDEg45675268 = CDrJAfCtuhKZDEg43860498;     CDrJAfCtuhKZDEg43860498 = CDrJAfCtuhKZDEg16022222;     CDrJAfCtuhKZDEg16022222 = CDrJAfCtuhKZDEg75700219;     CDrJAfCtuhKZDEg75700219 = CDrJAfCtuhKZDEg83222239;     CDrJAfCtuhKZDEg83222239 = CDrJAfCtuhKZDEg86479662;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gMaFHkAhXfdOfvtAkXgKBFnwYfgIF26034091() {     int UXFGMRHssmjGvWb53650721 = -791094664;    int UXFGMRHssmjGvWb15584649 = -64509193;    int UXFGMRHssmjGvWb18107815 = -748641965;    int UXFGMRHssmjGvWb1464676 = -261581566;    int UXFGMRHssmjGvWb98850427 = -618863064;    int UXFGMRHssmjGvWb3515900 = -793249032;    int UXFGMRHssmjGvWb56467357 = -603414699;    int UXFGMRHssmjGvWb91462995 = -811202087;    int UXFGMRHssmjGvWb37136074 = -571546196;    int UXFGMRHssmjGvWb89643117 = -456927701;    int UXFGMRHssmjGvWb94101078 = -49086046;    int UXFGMRHssmjGvWb36905137 = -244023153;    int UXFGMRHssmjGvWb7707460 = -998344423;    int UXFGMRHssmjGvWb43237900 = -572791790;    int UXFGMRHssmjGvWb19343513 = -893178018;    int UXFGMRHssmjGvWb44155583 = -54616473;    int UXFGMRHssmjGvWb22784444 = 57336566;    int UXFGMRHssmjGvWb84490566 = -475102981;    int UXFGMRHssmjGvWb15129961 = -420260668;    int UXFGMRHssmjGvWb40297417 = -737593823;    int UXFGMRHssmjGvWb89110479 = -367213539;    int UXFGMRHssmjGvWb72841364 = -634688412;    int UXFGMRHssmjGvWb73301802 = -862108072;    int UXFGMRHssmjGvWb48309089 = -515807424;    int UXFGMRHssmjGvWb26426889 = -427586282;    int UXFGMRHssmjGvWb57921848 = -272308354;    int UXFGMRHssmjGvWb96822848 = -634271853;    int UXFGMRHssmjGvWb96011969 = -309440677;    int UXFGMRHssmjGvWb99324609 = -516008119;    int UXFGMRHssmjGvWb46862688 = 73034288;    int UXFGMRHssmjGvWb27505637 = -140518192;    int UXFGMRHssmjGvWb89978284 = -932269806;    int UXFGMRHssmjGvWb65670319 = -604538626;    int UXFGMRHssmjGvWb66047720 = 46211566;    int UXFGMRHssmjGvWb90014875 = -810697894;    int UXFGMRHssmjGvWb54794374 = -492098416;    int UXFGMRHssmjGvWb23900262 = 76881997;    int UXFGMRHssmjGvWb32287109 = 69254264;    int UXFGMRHssmjGvWb31320513 = -892511707;    int UXFGMRHssmjGvWb88911874 = -793483060;    int UXFGMRHssmjGvWb51602117 = -975479191;    int UXFGMRHssmjGvWb1557079 = -676233642;    int UXFGMRHssmjGvWb78478729 = 89817600;    int UXFGMRHssmjGvWb99744124 = -122707097;    int UXFGMRHssmjGvWb78660870 = -749653262;    int UXFGMRHssmjGvWb90226211 = -518350040;    int UXFGMRHssmjGvWb26704358 = -406538687;    int UXFGMRHssmjGvWb76073755 = -797465670;    int UXFGMRHssmjGvWb31678088 = -112510715;    int UXFGMRHssmjGvWb54904134 = 59023535;    int UXFGMRHssmjGvWb9292878 = 45277451;    int UXFGMRHssmjGvWb89223263 = -321291410;    int UXFGMRHssmjGvWb43381533 = -609480551;    int UXFGMRHssmjGvWb27645647 = -848733096;    int UXFGMRHssmjGvWb36923611 = -812867621;    int UXFGMRHssmjGvWb80809357 = -56406253;    int UXFGMRHssmjGvWb42282846 = -202401122;    int UXFGMRHssmjGvWb69798726 = -132834542;    int UXFGMRHssmjGvWb75037786 = -833995284;    int UXFGMRHssmjGvWb40928580 = -246554710;    int UXFGMRHssmjGvWb6693051 = -58977179;    int UXFGMRHssmjGvWb60455388 = -193974022;    int UXFGMRHssmjGvWb92138386 = -195193968;    int UXFGMRHssmjGvWb90273385 = -544580485;    int UXFGMRHssmjGvWb62137481 = -216409509;    int UXFGMRHssmjGvWb4122794 = -116816240;    int UXFGMRHssmjGvWb71234817 = -639484528;    int UXFGMRHssmjGvWb41659739 = -944555989;    int UXFGMRHssmjGvWb53223024 = -762093896;    int UXFGMRHssmjGvWb64549139 = -301079603;    int UXFGMRHssmjGvWb20255322 = -31498470;    int UXFGMRHssmjGvWb90497335 = 88082301;    int UXFGMRHssmjGvWb53170054 = -582591274;    int UXFGMRHssmjGvWb26218086 = -626777608;    int UXFGMRHssmjGvWb88695300 = -762114633;    int UXFGMRHssmjGvWb87553401 = -690979897;    int UXFGMRHssmjGvWb94362634 = -624506012;    int UXFGMRHssmjGvWb73557678 = -639400976;    int UXFGMRHssmjGvWb69648219 = -766154163;    int UXFGMRHssmjGvWb36200677 = -909236243;    int UXFGMRHssmjGvWb31217490 = -865769668;    int UXFGMRHssmjGvWb20749094 = -836806184;    int UXFGMRHssmjGvWb64333882 = -96929962;    int UXFGMRHssmjGvWb44420476 = -475031655;    int UXFGMRHssmjGvWb37569810 = -972243164;    int UXFGMRHssmjGvWb38282374 = -819226783;    int UXFGMRHssmjGvWb46596752 = -222789256;    int UXFGMRHssmjGvWb38024673 = -755805530;    int UXFGMRHssmjGvWb29124110 = -140920813;    int UXFGMRHssmjGvWb9205518 = -654291642;    int UXFGMRHssmjGvWb12511528 = -189697295;    int UXFGMRHssmjGvWb54101536 = -790283462;    int UXFGMRHssmjGvWb57249322 = -96750452;    int UXFGMRHssmjGvWb90391933 = -545956997;    int UXFGMRHssmjGvWb82218824 = -634505881;    int UXFGMRHssmjGvWb91146728 = -681505169;    int UXFGMRHssmjGvWb9418692 = -381039675;    int UXFGMRHssmjGvWb88205344 = -365601916;    int UXFGMRHssmjGvWb37606644 = -906297588;    int UXFGMRHssmjGvWb74538076 = -791094664;     UXFGMRHssmjGvWb53650721 = UXFGMRHssmjGvWb15584649;     UXFGMRHssmjGvWb15584649 = UXFGMRHssmjGvWb18107815;     UXFGMRHssmjGvWb18107815 = UXFGMRHssmjGvWb1464676;     UXFGMRHssmjGvWb1464676 = UXFGMRHssmjGvWb98850427;     UXFGMRHssmjGvWb98850427 = UXFGMRHssmjGvWb3515900;     UXFGMRHssmjGvWb3515900 = UXFGMRHssmjGvWb56467357;     UXFGMRHssmjGvWb56467357 = UXFGMRHssmjGvWb91462995;     UXFGMRHssmjGvWb91462995 = UXFGMRHssmjGvWb37136074;     UXFGMRHssmjGvWb37136074 = UXFGMRHssmjGvWb89643117;     UXFGMRHssmjGvWb89643117 = UXFGMRHssmjGvWb94101078;     UXFGMRHssmjGvWb94101078 = UXFGMRHssmjGvWb36905137;     UXFGMRHssmjGvWb36905137 = UXFGMRHssmjGvWb7707460;     UXFGMRHssmjGvWb7707460 = UXFGMRHssmjGvWb43237900;     UXFGMRHssmjGvWb43237900 = UXFGMRHssmjGvWb19343513;     UXFGMRHssmjGvWb19343513 = UXFGMRHssmjGvWb44155583;     UXFGMRHssmjGvWb44155583 = UXFGMRHssmjGvWb22784444;     UXFGMRHssmjGvWb22784444 = UXFGMRHssmjGvWb84490566;     UXFGMRHssmjGvWb84490566 = UXFGMRHssmjGvWb15129961;     UXFGMRHssmjGvWb15129961 = UXFGMRHssmjGvWb40297417;     UXFGMRHssmjGvWb40297417 = UXFGMRHssmjGvWb89110479;     UXFGMRHssmjGvWb89110479 = UXFGMRHssmjGvWb72841364;     UXFGMRHssmjGvWb72841364 = UXFGMRHssmjGvWb73301802;     UXFGMRHssmjGvWb73301802 = UXFGMRHssmjGvWb48309089;     UXFGMRHssmjGvWb48309089 = UXFGMRHssmjGvWb26426889;     UXFGMRHssmjGvWb26426889 = UXFGMRHssmjGvWb57921848;     UXFGMRHssmjGvWb57921848 = UXFGMRHssmjGvWb96822848;     UXFGMRHssmjGvWb96822848 = UXFGMRHssmjGvWb96011969;     UXFGMRHssmjGvWb96011969 = UXFGMRHssmjGvWb99324609;     UXFGMRHssmjGvWb99324609 = UXFGMRHssmjGvWb46862688;     UXFGMRHssmjGvWb46862688 = UXFGMRHssmjGvWb27505637;     UXFGMRHssmjGvWb27505637 = UXFGMRHssmjGvWb89978284;     UXFGMRHssmjGvWb89978284 = UXFGMRHssmjGvWb65670319;     UXFGMRHssmjGvWb65670319 = UXFGMRHssmjGvWb66047720;     UXFGMRHssmjGvWb66047720 = UXFGMRHssmjGvWb90014875;     UXFGMRHssmjGvWb90014875 = UXFGMRHssmjGvWb54794374;     UXFGMRHssmjGvWb54794374 = UXFGMRHssmjGvWb23900262;     UXFGMRHssmjGvWb23900262 = UXFGMRHssmjGvWb32287109;     UXFGMRHssmjGvWb32287109 = UXFGMRHssmjGvWb31320513;     UXFGMRHssmjGvWb31320513 = UXFGMRHssmjGvWb88911874;     UXFGMRHssmjGvWb88911874 = UXFGMRHssmjGvWb51602117;     UXFGMRHssmjGvWb51602117 = UXFGMRHssmjGvWb1557079;     UXFGMRHssmjGvWb1557079 = UXFGMRHssmjGvWb78478729;     UXFGMRHssmjGvWb78478729 = UXFGMRHssmjGvWb99744124;     UXFGMRHssmjGvWb99744124 = UXFGMRHssmjGvWb78660870;     UXFGMRHssmjGvWb78660870 = UXFGMRHssmjGvWb90226211;     UXFGMRHssmjGvWb90226211 = UXFGMRHssmjGvWb26704358;     UXFGMRHssmjGvWb26704358 = UXFGMRHssmjGvWb76073755;     UXFGMRHssmjGvWb76073755 = UXFGMRHssmjGvWb31678088;     UXFGMRHssmjGvWb31678088 = UXFGMRHssmjGvWb54904134;     UXFGMRHssmjGvWb54904134 = UXFGMRHssmjGvWb9292878;     UXFGMRHssmjGvWb9292878 = UXFGMRHssmjGvWb89223263;     UXFGMRHssmjGvWb89223263 = UXFGMRHssmjGvWb43381533;     UXFGMRHssmjGvWb43381533 = UXFGMRHssmjGvWb27645647;     UXFGMRHssmjGvWb27645647 = UXFGMRHssmjGvWb36923611;     UXFGMRHssmjGvWb36923611 = UXFGMRHssmjGvWb80809357;     UXFGMRHssmjGvWb80809357 = UXFGMRHssmjGvWb42282846;     UXFGMRHssmjGvWb42282846 = UXFGMRHssmjGvWb69798726;     UXFGMRHssmjGvWb69798726 = UXFGMRHssmjGvWb75037786;     UXFGMRHssmjGvWb75037786 = UXFGMRHssmjGvWb40928580;     UXFGMRHssmjGvWb40928580 = UXFGMRHssmjGvWb6693051;     UXFGMRHssmjGvWb6693051 = UXFGMRHssmjGvWb60455388;     UXFGMRHssmjGvWb60455388 = UXFGMRHssmjGvWb92138386;     UXFGMRHssmjGvWb92138386 = UXFGMRHssmjGvWb90273385;     UXFGMRHssmjGvWb90273385 = UXFGMRHssmjGvWb62137481;     UXFGMRHssmjGvWb62137481 = UXFGMRHssmjGvWb4122794;     UXFGMRHssmjGvWb4122794 = UXFGMRHssmjGvWb71234817;     UXFGMRHssmjGvWb71234817 = UXFGMRHssmjGvWb41659739;     UXFGMRHssmjGvWb41659739 = UXFGMRHssmjGvWb53223024;     UXFGMRHssmjGvWb53223024 = UXFGMRHssmjGvWb64549139;     UXFGMRHssmjGvWb64549139 = UXFGMRHssmjGvWb20255322;     UXFGMRHssmjGvWb20255322 = UXFGMRHssmjGvWb90497335;     UXFGMRHssmjGvWb90497335 = UXFGMRHssmjGvWb53170054;     UXFGMRHssmjGvWb53170054 = UXFGMRHssmjGvWb26218086;     UXFGMRHssmjGvWb26218086 = UXFGMRHssmjGvWb88695300;     UXFGMRHssmjGvWb88695300 = UXFGMRHssmjGvWb87553401;     UXFGMRHssmjGvWb87553401 = UXFGMRHssmjGvWb94362634;     UXFGMRHssmjGvWb94362634 = UXFGMRHssmjGvWb73557678;     UXFGMRHssmjGvWb73557678 = UXFGMRHssmjGvWb69648219;     UXFGMRHssmjGvWb69648219 = UXFGMRHssmjGvWb36200677;     UXFGMRHssmjGvWb36200677 = UXFGMRHssmjGvWb31217490;     UXFGMRHssmjGvWb31217490 = UXFGMRHssmjGvWb20749094;     UXFGMRHssmjGvWb20749094 = UXFGMRHssmjGvWb64333882;     UXFGMRHssmjGvWb64333882 = UXFGMRHssmjGvWb44420476;     UXFGMRHssmjGvWb44420476 = UXFGMRHssmjGvWb37569810;     UXFGMRHssmjGvWb37569810 = UXFGMRHssmjGvWb38282374;     UXFGMRHssmjGvWb38282374 = UXFGMRHssmjGvWb46596752;     UXFGMRHssmjGvWb46596752 = UXFGMRHssmjGvWb38024673;     UXFGMRHssmjGvWb38024673 = UXFGMRHssmjGvWb29124110;     UXFGMRHssmjGvWb29124110 = UXFGMRHssmjGvWb9205518;     UXFGMRHssmjGvWb9205518 = UXFGMRHssmjGvWb12511528;     UXFGMRHssmjGvWb12511528 = UXFGMRHssmjGvWb54101536;     UXFGMRHssmjGvWb54101536 = UXFGMRHssmjGvWb57249322;     UXFGMRHssmjGvWb57249322 = UXFGMRHssmjGvWb90391933;     UXFGMRHssmjGvWb90391933 = UXFGMRHssmjGvWb82218824;     UXFGMRHssmjGvWb82218824 = UXFGMRHssmjGvWb91146728;     UXFGMRHssmjGvWb91146728 = UXFGMRHssmjGvWb9418692;     UXFGMRHssmjGvWb9418692 = UXFGMRHssmjGvWb88205344;     UXFGMRHssmjGvWb88205344 = UXFGMRHssmjGvWb37606644;     UXFGMRHssmjGvWb37606644 = UXFGMRHssmjGvWb74538076;     UXFGMRHssmjGvWb74538076 = UXFGMRHssmjGvWb53650721;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void dVbyipSXTCkPIxKovQQQXXkIZzjJS28223429() {     int MYsDQZoXPkibzUw50163642 = -789895352;    int MYsDQZoXPkibzUw30667474 = -17197496;    int MYsDQZoXPkibzUw73790126 = -496744731;    int MYsDQZoXPkibzUw14013524 = -730395357;    int MYsDQZoXPkibzUw67502817 = -174450059;    int MYsDQZoXPkibzUw40762772 = -823776608;    int MYsDQZoXPkibzUw30282010 = -768332674;    int MYsDQZoXPkibzUw26915166 = -924086271;    int MYsDQZoXPkibzUw41390330 = -823963950;    int MYsDQZoXPkibzUw43647654 = -115652914;    int MYsDQZoXPkibzUw54317194 = -24321283;    int MYsDQZoXPkibzUw51921574 = -812756058;    int MYsDQZoXPkibzUw12978157 = -184130087;    int MYsDQZoXPkibzUw10313013 = 17686675;    int MYsDQZoXPkibzUw24243270 = -744504728;    int MYsDQZoXPkibzUw57112407 = -913237688;    int MYsDQZoXPkibzUw50475293 = -955510479;    int MYsDQZoXPkibzUw20626055 = 7370954;    int MYsDQZoXPkibzUw97052936 = -909387968;    int MYsDQZoXPkibzUw81022676 = -647118653;    int MYsDQZoXPkibzUw96789334 = -577763137;    int MYsDQZoXPkibzUw13743122 = 87293896;    int MYsDQZoXPkibzUw62798765 = -43027783;    int MYsDQZoXPkibzUw72133859 = -474924829;    int MYsDQZoXPkibzUw4701896 = 46987958;    int MYsDQZoXPkibzUw33946213 = -451603617;    int MYsDQZoXPkibzUw44719534 = -461453700;    int MYsDQZoXPkibzUw88262331 = 34803224;    int MYsDQZoXPkibzUw81520580 = -381386102;    int MYsDQZoXPkibzUw47117196 = -391742554;    int MYsDQZoXPkibzUw36599717 = -442486346;    int MYsDQZoXPkibzUw25728468 = -164275827;    int MYsDQZoXPkibzUw83114758 = -494635393;    int MYsDQZoXPkibzUw78707388 = -874298434;    int MYsDQZoXPkibzUw99499713 = 74636580;    int MYsDQZoXPkibzUw98465627 = -769175124;    int MYsDQZoXPkibzUw4457229 = -533541961;    int MYsDQZoXPkibzUw5414083 = -851877704;    int MYsDQZoXPkibzUw80054819 = -133580457;    int MYsDQZoXPkibzUw68429274 = -229830942;    int MYsDQZoXPkibzUw83873111 = -407021361;    int MYsDQZoXPkibzUw45750430 = -83674605;    int MYsDQZoXPkibzUw66020665 = -406454972;    int MYsDQZoXPkibzUw11952233 = -264340851;    int MYsDQZoXPkibzUw44647585 = -181807600;    int MYsDQZoXPkibzUw87907238 = -850677836;    int MYsDQZoXPkibzUw23616451 = -685059335;    int MYsDQZoXPkibzUw56563778 = -960559669;    int MYsDQZoXPkibzUw50287155 = -744014910;    int MYsDQZoXPkibzUw27705173 = -290515920;    int MYsDQZoXPkibzUw61081518 = -584101755;    int MYsDQZoXPkibzUw61631901 = -903456660;    int MYsDQZoXPkibzUw35524911 = -301389195;    int MYsDQZoXPkibzUw13152843 = -198934100;    int MYsDQZoXPkibzUw31434094 = -145589414;    int MYsDQZoXPkibzUw36420521 = -777189248;    int MYsDQZoXPkibzUw67868709 = -974169713;    int MYsDQZoXPkibzUw1656267 = 78180097;    int MYsDQZoXPkibzUw9311628 = -677383316;    int MYsDQZoXPkibzUw33556605 = -722846443;    int MYsDQZoXPkibzUw96043238 = -262322909;    int MYsDQZoXPkibzUw42019678 = -703135898;    int MYsDQZoXPkibzUw45394585 = -442700169;    int MYsDQZoXPkibzUw94273134 = -332221396;    int MYsDQZoXPkibzUw7047938 = -673166569;    int MYsDQZoXPkibzUw28588726 = -860045457;    int MYsDQZoXPkibzUw68806816 = -218120666;    int MYsDQZoXPkibzUw34270769 = -309831653;    int MYsDQZoXPkibzUw10813299 = 43050094;    int MYsDQZoXPkibzUw25777643 = -975329604;    int MYsDQZoXPkibzUw52655178 = -279695728;    int MYsDQZoXPkibzUw45061210 = -3632775;    int MYsDQZoXPkibzUw40571235 = -859048589;    int MYsDQZoXPkibzUw28623663 = -579557027;    int MYsDQZoXPkibzUw97149564 = -140097293;    int MYsDQZoXPkibzUw51038905 = -394088533;    int MYsDQZoXPkibzUw47722456 = -506251132;    int MYsDQZoXPkibzUw50846532 = -778686932;    int MYsDQZoXPkibzUw27486274 = -193117229;    int MYsDQZoXPkibzUw16794658 = -102334206;    int MYsDQZoXPkibzUw10329762 = -766544282;    int MYsDQZoXPkibzUw88155755 = -500894031;    int MYsDQZoXPkibzUw37975176 = -221181866;    int MYsDQZoXPkibzUw53815408 = 9129817;    int MYsDQZoXPkibzUw86035677 = -807640800;    int MYsDQZoXPkibzUw74967815 = -539029686;    int MYsDQZoXPkibzUw90203556 = -862886632;    int MYsDQZoXPkibzUw69961916 = -195701294;    int MYsDQZoXPkibzUw47273294 = -628709021;    int MYsDQZoXPkibzUw63079192 = -148174172;    int MYsDQZoXPkibzUw30596919 = -795005411;    int MYsDQZoXPkibzUw2800962 = -511722059;    int MYsDQZoXPkibzUw96102455 = -74494389;    int MYsDQZoXPkibzUw46498215 = -410734015;    int MYsDQZoXPkibzUw72386036 = -967508033;    int MYsDQZoXPkibzUw41853433 = -703885463;    int MYsDQZoXPkibzUw355845 = -640974436;    int MYsDQZoXPkibzUw71747531 = 25766423;    int MYsDQZoXPkibzUw4904296 = -591174282;    int MYsDQZoXPkibzUw16058859 = -789895352;     MYsDQZoXPkibzUw50163642 = MYsDQZoXPkibzUw30667474;     MYsDQZoXPkibzUw30667474 = MYsDQZoXPkibzUw73790126;     MYsDQZoXPkibzUw73790126 = MYsDQZoXPkibzUw14013524;     MYsDQZoXPkibzUw14013524 = MYsDQZoXPkibzUw67502817;     MYsDQZoXPkibzUw67502817 = MYsDQZoXPkibzUw40762772;     MYsDQZoXPkibzUw40762772 = MYsDQZoXPkibzUw30282010;     MYsDQZoXPkibzUw30282010 = MYsDQZoXPkibzUw26915166;     MYsDQZoXPkibzUw26915166 = MYsDQZoXPkibzUw41390330;     MYsDQZoXPkibzUw41390330 = MYsDQZoXPkibzUw43647654;     MYsDQZoXPkibzUw43647654 = MYsDQZoXPkibzUw54317194;     MYsDQZoXPkibzUw54317194 = MYsDQZoXPkibzUw51921574;     MYsDQZoXPkibzUw51921574 = MYsDQZoXPkibzUw12978157;     MYsDQZoXPkibzUw12978157 = MYsDQZoXPkibzUw10313013;     MYsDQZoXPkibzUw10313013 = MYsDQZoXPkibzUw24243270;     MYsDQZoXPkibzUw24243270 = MYsDQZoXPkibzUw57112407;     MYsDQZoXPkibzUw57112407 = MYsDQZoXPkibzUw50475293;     MYsDQZoXPkibzUw50475293 = MYsDQZoXPkibzUw20626055;     MYsDQZoXPkibzUw20626055 = MYsDQZoXPkibzUw97052936;     MYsDQZoXPkibzUw97052936 = MYsDQZoXPkibzUw81022676;     MYsDQZoXPkibzUw81022676 = MYsDQZoXPkibzUw96789334;     MYsDQZoXPkibzUw96789334 = MYsDQZoXPkibzUw13743122;     MYsDQZoXPkibzUw13743122 = MYsDQZoXPkibzUw62798765;     MYsDQZoXPkibzUw62798765 = MYsDQZoXPkibzUw72133859;     MYsDQZoXPkibzUw72133859 = MYsDQZoXPkibzUw4701896;     MYsDQZoXPkibzUw4701896 = MYsDQZoXPkibzUw33946213;     MYsDQZoXPkibzUw33946213 = MYsDQZoXPkibzUw44719534;     MYsDQZoXPkibzUw44719534 = MYsDQZoXPkibzUw88262331;     MYsDQZoXPkibzUw88262331 = MYsDQZoXPkibzUw81520580;     MYsDQZoXPkibzUw81520580 = MYsDQZoXPkibzUw47117196;     MYsDQZoXPkibzUw47117196 = MYsDQZoXPkibzUw36599717;     MYsDQZoXPkibzUw36599717 = MYsDQZoXPkibzUw25728468;     MYsDQZoXPkibzUw25728468 = MYsDQZoXPkibzUw83114758;     MYsDQZoXPkibzUw83114758 = MYsDQZoXPkibzUw78707388;     MYsDQZoXPkibzUw78707388 = MYsDQZoXPkibzUw99499713;     MYsDQZoXPkibzUw99499713 = MYsDQZoXPkibzUw98465627;     MYsDQZoXPkibzUw98465627 = MYsDQZoXPkibzUw4457229;     MYsDQZoXPkibzUw4457229 = MYsDQZoXPkibzUw5414083;     MYsDQZoXPkibzUw5414083 = MYsDQZoXPkibzUw80054819;     MYsDQZoXPkibzUw80054819 = MYsDQZoXPkibzUw68429274;     MYsDQZoXPkibzUw68429274 = MYsDQZoXPkibzUw83873111;     MYsDQZoXPkibzUw83873111 = MYsDQZoXPkibzUw45750430;     MYsDQZoXPkibzUw45750430 = MYsDQZoXPkibzUw66020665;     MYsDQZoXPkibzUw66020665 = MYsDQZoXPkibzUw11952233;     MYsDQZoXPkibzUw11952233 = MYsDQZoXPkibzUw44647585;     MYsDQZoXPkibzUw44647585 = MYsDQZoXPkibzUw87907238;     MYsDQZoXPkibzUw87907238 = MYsDQZoXPkibzUw23616451;     MYsDQZoXPkibzUw23616451 = MYsDQZoXPkibzUw56563778;     MYsDQZoXPkibzUw56563778 = MYsDQZoXPkibzUw50287155;     MYsDQZoXPkibzUw50287155 = MYsDQZoXPkibzUw27705173;     MYsDQZoXPkibzUw27705173 = MYsDQZoXPkibzUw61081518;     MYsDQZoXPkibzUw61081518 = MYsDQZoXPkibzUw61631901;     MYsDQZoXPkibzUw61631901 = MYsDQZoXPkibzUw35524911;     MYsDQZoXPkibzUw35524911 = MYsDQZoXPkibzUw13152843;     MYsDQZoXPkibzUw13152843 = MYsDQZoXPkibzUw31434094;     MYsDQZoXPkibzUw31434094 = MYsDQZoXPkibzUw36420521;     MYsDQZoXPkibzUw36420521 = MYsDQZoXPkibzUw67868709;     MYsDQZoXPkibzUw67868709 = MYsDQZoXPkibzUw1656267;     MYsDQZoXPkibzUw1656267 = MYsDQZoXPkibzUw9311628;     MYsDQZoXPkibzUw9311628 = MYsDQZoXPkibzUw33556605;     MYsDQZoXPkibzUw33556605 = MYsDQZoXPkibzUw96043238;     MYsDQZoXPkibzUw96043238 = MYsDQZoXPkibzUw42019678;     MYsDQZoXPkibzUw42019678 = MYsDQZoXPkibzUw45394585;     MYsDQZoXPkibzUw45394585 = MYsDQZoXPkibzUw94273134;     MYsDQZoXPkibzUw94273134 = MYsDQZoXPkibzUw7047938;     MYsDQZoXPkibzUw7047938 = MYsDQZoXPkibzUw28588726;     MYsDQZoXPkibzUw28588726 = MYsDQZoXPkibzUw68806816;     MYsDQZoXPkibzUw68806816 = MYsDQZoXPkibzUw34270769;     MYsDQZoXPkibzUw34270769 = MYsDQZoXPkibzUw10813299;     MYsDQZoXPkibzUw10813299 = MYsDQZoXPkibzUw25777643;     MYsDQZoXPkibzUw25777643 = MYsDQZoXPkibzUw52655178;     MYsDQZoXPkibzUw52655178 = MYsDQZoXPkibzUw45061210;     MYsDQZoXPkibzUw45061210 = MYsDQZoXPkibzUw40571235;     MYsDQZoXPkibzUw40571235 = MYsDQZoXPkibzUw28623663;     MYsDQZoXPkibzUw28623663 = MYsDQZoXPkibzUw97149564;     MYsDQZoXPkibzUw97149564 = MYsDQZoXPkibzUw51038905;     MYsDQZoXPkibzUw51038905 = MYsDQZoXPkibzUw47722456;     MYsDQZoXPkibzUw47722456 = MYsDQZoXPkibzUw50846532;     MYsDQZoXPkibzUw50846532 = MYsDQZoXPkibzUw27486274;     MYsDQZoXPkibzUw27486274 = MYsDQZoXPkibzUw16794658;     MYsDQZoXPkibzUw16794658 = MYsDQZoXPkibzUw10329762;     MYsDQZoXPkibzUw10329762 = MYsDQZoXPkibzUw88155755;     MYsDQZoXPkibzUw88155755 = MYsDQZoXPkibzUw37975176;     MYsDQZoXPkibzUw37975176 = MYsDQZoXPkibzUw53815408;     MYsDQZoXPkibzUw53815408 = MYsDQZoXPkibzUw86035677;     MYsDQZoXPkibzUw86035677 = MYsDQZoXPkibzUw74967815;     MYsDQZoXPkibzUw74967815 = MYsDQZoXPkibzUw90203556;     MYsDQZoXPkibzUw90203556 = MYsDQZoXPkibzUw69961916;     MYsDQZoXPkibzUw69961916 = MYsDQZoXPkibzUw47273294;     MYsDQZoXPkibzUw47273294 = MYsDQZoXPkibzUw63079192;     MYsDQZoXPkibzUw63079192 = MYsDQZoXPkibzUw30596919;     MYsDQZoXPkibzUw30596919 = MYsDQZoXPkibzUw2800962;     MYsDQZoXPkibzUw2800962 = MYsDQZoXPkibzUw96102455;     MYsDQZoXPkibzUw96102455 = MYsDQZoXPkibzUw46498215;     MYsDQZoXPkibzUw46498215 = MYsDQZoXPkibzUw72386036;     MYsDQZoXPkibzUw72386036 = MYsDQZoXPkibzUw41853433;     MYsDQZoXPkibzUw41853433 = MYsDQZoXPkibzUw355845;     MYsDQZoXPkibzUw355845 = MYsDQZoXPkibzUw71747531;     MYsDQZoXPkibzUw71747531 = MYsDQZoXPkibzUw4904296;     MYsDQZoXPkibzUw4904296 = MYsDQZoXPkibzUw16058859;     MYsDQZoXPkibzUw16058859 = MYsDQZoXPkibzUw50163642;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void aBArQwXdIsjsdZSpqoFDDutvqbPZp30412768() {     int IpgnmuvTFqCUodm46676563 = -788696040;    int IpgnmuvTFqCUodm45750299 = 30114202;    int IpgnmuvTFqCUodm29472437 = -244847497;    int IpgnmuvTFqCUodm26562372 = -99209147;    int IpgnmuvTFqCUodm36155208 = -830037055;    int IpgnmuvTFqCUodm78009644 = -854304185;    int IpgnmuvTFqCUodm4096664 = -933250649;    int IpgnmuvTFqCUodm62367336 = 63029545;    int IpgnmuvTFqCUodm45644587 = 23618297;    int IpgnmuvTFqCUodm97652190 = -874378127;    int IpgnmuvTFqCUodm14533310 = 443480;    int IpgnmuvTFqCUodm66938011 = -281488964;    int IpgnmuvTFqCUodm18248855 = -469915750;    int IpgnmuvTFqCUodm77388125 = -491834860;    int IpgnmuvTFqCUodm29143027 = -595831438;    int IpgnmuvTFqCUodm70069230 = -671858904;    int IpgnmuvTFqCUodm78166141 = -868357524;    int IpgnmuvTFqCUodm56761543 = -610155111;    int IpgnmuvTFqCUodm78975913 = -298515268;    int IpgnmuvTFqCUodm21747936 = -556643484;    int IpgnmuvTFqCUodm4468190 = -788312735;    int IpgnmuvTFqCUodm54644879 = -290723797;    int IpgnmuvTFqCUodm52295728 = -323947493;    int IpgnmuvTFqCUodm95958629 = -434042233;    int IpgnmuvTFqCUodm82976903 = -578437801;    int IpgnmuvTFqCUodm9970578 = -630898879;    int IpgnmuvTFqCUodm92616218 = -288635547;    int IpgnmuvTFqCUodm80512694 = -720952875;    int IpgnmuvTFqCUodm63716552 = -246764085;    int IpgnmuvTFqCUodm47371703 = -856519397;    int IpgnmuvTFqCUodm45693796 = -744454499;    int IpgnmuvTFqCUodm61478651 = -496281847;    int IpgnmuvTFqCUodm559198 = -384732161;    int IpgnmuvTFqCUodm91367056 = -694808434;    int IpgnmuvTFqCUodm8984552 = -140028946;    int IpgnmuvTFqCUodm42136881 = 53748167;    int IpgnmuvTFqCUodm85014195 = -43965919;    int IpgnmuvTFqCUodm78541057 = -673009673;    int IpgnmuvTFqCUodm28789127 = -474649208;    int IpgnmuvTFqCUodm47946673 = -766178824;    int IpgnmuvTFqCUodm16144107 = -938563532;    int IpgnmuvTFqCUodm89943782 = -591115567;    int IpgnmuvTFqCUodm53562602 = -902727544;    int IpgnmuvTFqCUodm24160341 = -405974604;    int IpgnmuvTFqCUodm10634301 = -713961938;    int IpgnmuvTFqCUodm85588265 = -83005632;    int IpgnmuvTFqCUodm20528544 = -963579983;    int IpgnmuvTFqCUodm37053801 = -23653668;    int IpgnmuvTFqCUodm68896223 = -275519105;    int IpgnmuvTFqCUodm506213 = -640055375;    int IpgnmuvTFqCUodm12870159 = -113480961;    int IpgnmuvTFqCUodm34040540 = -385621910;    int IpgnmuvTFqCUodm27668290 = 6702161;    int IpgnmuvTFqCUodm98660037 = -649135104;    int IpgnmuvTFqCUodm25944578 = -578311206;    int IpgnmuvTFqCUodm92031684 = -397972244;    int IpgnmuvTFqCUodm93454571 = -645938305;    int IpgnmuvTFqCUodm33513807 = -810805264;    int IpgnmuvTFqCUodm43585469 = -520771347;    int IpgnmuvTFqCUodm26184631 = -99138176;    int IpgnmuvTFqCUodm85393425 = -465668638;    int IpgnmuvTFqCUodm23583969 = -112297774;    int IpgnmuvTFqCUodm98650784 = -690206371;    int IpgnmuvTFqCUodm98272884 = -119862307;    int IpgnmuvTFqCUodm51958394 = -29923628;    int IpgnmuvTFqCUodm53054659 = -503274674;    int IpgnmuvTFqCUodm66378814 = -896756803;    int IpgnmuvTFqCUodm26881798 = -775107317;    int IpgnmuvTFqCUodm68403574 = -251805915;    int IpgnmuvTFqCUodm87006146 = -549579606;    int IpgnmuvTFqCUodm85055035 = -527892985;    int IpgnmuvTFqCUodm99625084 = -95347851;    int IpgnmuvTFqCUodm27972417 = -35505904;    int IpgnmuvTFqCUodm31029240 = -532336445;    int IpgnmuvTFqCUodm5603830 = -618079952;    int IpgnmuvTFqCUodm14524408 = -97197168;    int IpgnmuvTFqCUodm1082278 = -387996253;    int IpgnmuvTFqCUodm28135387 = -917972889;    int IpgnmuvTFqCUodm85324329 = -720080296;    int IpgnmuvTFqCUodm97388637 = -395432170;    int IpgnmuvTFqCUodm89442033 = -667318897;    int IpgnmuvTFqCUodm55562417 = -164981879;    int IpgnmuvTFqCUodm11616471 = -345433770;    int IpgnmuvTFqCUodm63210340 = -606708711;    int IpgnmuvTFqCUodm34501545 = -643038436;    int IpgnmuvTFqCUodm11653257 = -258832589;    int IpgnmuvTFqCUodm33810362 = -402984008;    int IpgnmuvTFqCUodm1899160 = -735597058;    int IpgnmuvTFqCUodm65422479 = -16497228;    int IpgnmuvTFqCUodm16952868 = -742056703;    int IpgnmuvTFqCUodm48682310 = -300313528;    int IpgnmuvTFqCUodm51500388 = -233160655;    int IpgnmuvTFqCUodm34955589 = -52238327;    int IpgnmuvTFqCUodm2604497 = -275511032;    int IpgnmuvTFqCUodm62553247 = -200510186;    int IpgnmuvTFqCUodm92560137 = -726265758;    int IpgnmuvTFqCUodm91292998 = -900909197;    int IpgnmuvTFqCUodm55289718 = -682865238;    int IpgnmuvTFqCUodm72201946 = -276050977;    int IpgnmuvTFqCUodm57579642 = -788696040;     IpgnmuvTFqCUodm46676563 = IpgnmuvTFqCUodm45750299;     IpgnmuvTFqCUodm45750299 = IpgnmuvTFqCUodm29472437;     IpgnmuvTFqCUodm29472437 = IpgnmuvTFqCUodm26562372;     IpgnmuvTFqCUodm26562372 = IpgnmuvTFqCUodm36155208;     IpgnmuvTFqCUodm36155208 = IpgnmuvTFqCUodm78009644;     IpgnmuvTFqCUodm78009644 = IpgnmuvTFqCUodm4096664;     IpgnmuvTFqCUodm4096664 = IpgnmuvTFqCUodm62367336;     IpgnmuvTFqCUodm62367336 = IpgnmuvTFqCUodm45644587;     IpgnmuvTFqCUodm45644587 = IpgnmuvTFqCUodm97652190;     IpgnmuvTFqCUodm97652190 = IpgnmuvTFqCUodm14533310;     IpgnmuvTFqCUodm14533310 = IpgnmuvTFqCUodm66938011;     IpgnmuvTFqCUodm66938011 = IpgnmuvTFqCUodm18248855;     IpgnmuvTFqCUodm18248855 = IpgnmuvTFqCUodm77388125;     IpgnmuvTFqCUodm77388125 = IpgnmuvTFqCUodm29143027;     IpgnmuvTFqCUodm29143027 = IpgnmuvTFqCUodm70069230;     IpgnmuvTFqCUodm70069230 = IpgnmuvTFqCUodm78166141;     IpgnmuvTFqCUodm78166141 = IpgnmuvTFqCUodm56761543;     IpgnmuvTFqCUodm56761543 = IpgnmuvTFqCUodm78975913;     IpgnmuvTFqCUodm78975913 = IpgnmuvTFqCUodm21747936;     IpgnmuvTFqCUodm21747936 = IpgnmuvTFqCUodm4468190;     IpgnmuvTFqCUodm4468190 = IpgnmuvTFqCUodm54644879;     IpgnmuvTFqCUodm54644879 = IpgnmuvTFqCUodm52295728;     IpgnmuvTFqCUodm52295728 = IpgnmuvTFqCUodm95958629;     IpgnmuvTFqCUodm95958629 = IpgnmuvTFqCUodm82976903;     IpgnmuvTFqCUodm82976903 = IpgnmuvTFqCUodm9970578;     IpgnmuvTFqCUodm9970578 = IpgnmuvTFqCUodm92616218;     IpgnmuvTFqCUodm92616218 = IpgnmuvTFqCUodm80512694;     IpgnmuvTFqCUodm80512694 = IpgnmuvTFqCUodm63716552;     IpgnmuvTFqCUodm63716552 = IpgnmuvTFqCUodm47371703;     IpgnmuvTFqCUodm47371703 = IpgnmuvTFqCUodm45693796;     IpgnmuvTFqCUodm45693796 = IpgnmuvTFqCUodm61478651;     IpgnmuvTFqCUodm61478651 = IpgnmuvTFqCUodm559198;     IpgnmuvTFqCUodm559198 = IpgnmuvTFqCUodm91367056;     IpgnmuvTFqCUodm91367056 = IpgnmuvTFqCUodm8984552;     IpgnmuvTFqCUodm8984552 = IpgnmuvTFqCUodm42136881;     IpgnmuvTFqCUodm42136881 = IpgnmuvTFqCUodm85014195;     IpgnmuvTFqCUodm85014195 = IpgnmuvTFqCUodm78541057;     IpgnmuvTFqCUodm78541057 = IpgnmuvTFqCUodm28789127;     IpgnmuvTFqCUodm28789127 = IpgnmuvTFqCUodm47946673;     IpgnmuvTFqCUodm47946673 = IpgnmuvTFqCUodm16144107;     IpgnmuvTFqCUodm16144107 = IpgnmuvTFqCUodm89943782;     IpgnmuvTFqCUodm89943782 = IpgnmuvTFqCUodm53562602;     IpgnmuvTFqCUodm53562602 = IpgnmuvTFqCUodm24160341;     IpgnmuvTFqCUodm24160341 = IpgnmuvTFqCUodm10634301;     IpgnmuvTFqCUodm10634301 = IpgnmuvTFqCUodm85588265;     IpgnmuvTFqCUodm85588265 = IpgnmuvTFqCUodm20528544;     IpgnmuvTFqCUodm20528544 = IpgnmuvTFqCUodm37053801;     IpgnmuvTFqCUodm37053801 = IpgnmuvTFqCUodm68896223;     IpgnmuvTFqCUodm68896223 = IpgnmuvTFqCUodm506213;     IpgnmuvTFqCUodm506213 = IpgnmuvTFqCUodm12870159;     IpgnmuvTFqCUodm12870159 = IpgnmuvTFqCUodm34040540;     IpgnmuvTFqCUodm34040540 = IpgnmuvTFqCUodm27668290;     IpgnmuvTFqCUodm27668290 = IpgnmuvTFqCUodm98660037;     IpgnmuvTFqCUodm98660037 = IpgnmuvTFqCUodm25944578;     IpgnmuvTFqCUodm25944578 = IpgnmuvTFqCUodm92031684;     IpgnmuvTFqCUodm92031684 = IpgnmuvTFqCUodm93454571;     IpgnmuvTFqCUodm93454571 = IpgnmuvTFqCUodm33513807;     IpgnmuvTFqCUodm33513807 = IpgnmuvTFqCUodm43585469;     IpgnmuvTFqCUodm43585469 = IpgnmuvTFqCUodm26184631;     IpgnmuvTFqCUodm26184631 = IpgnmuvTFqCUodm85393425;     IpgnmuvTFqCUodm85393425 = IpgnmuvTFqCUodm23583969;     IpgnmuvTFqCUodm23583969 = IpgnmuvTFqCUodm98650784;     IpgnmuvTFqCUodm98650784 = IpgnmuvTFqCUodm98272884;     IpgnmuvTFqCUodm98272884 = IpgnmuvTFqCUodm51958394;     IpgnmuvTFqCUodm51958394 = IpgnmuvTFqCUodm53054659;     IpgnmuvTFqCUodm53054659 = IpgnmuvTFqCUodm66378814;     IpgnmuvTFqCUodm66378814 = IpgnmuvTFqCUodm26881798;     IpgnmuvTFqCUodm26881798 = IpgnmuvTFqCUodm68403574;     IpgnmuvTFqCUodm68403574 = IpgnmuvTFqCUodm87006146;     IpgnmuvTFqCUodm87006146 = IpgnmuvTFqCUodm85055035;     IpgnmuvTFqCUodm85055035 = IpgnmuvTFqCUodm99625084;     IpgnmuvTFqCUodm99625084 = IpgnmuvTFqCUodm27972417;     IpgnmuvTFqCUodm27972417 = IpgnmuvTFqCUodm31029240;     IpgnmuvTFqCUodm31029240 = IpgnmuvTFqCUodm5603830;     IpgnmuvTFqCUodm5603830 = IpgnmuvTFqCUodm14524408;     IpgnmuvTFqCUodm14524408 = IpgnmuvTFqCUodm1082278;     IpgnmuvTFqCUodm1082278 = IpgnmuvTFqCUodm28135387;     IpgnmuvTFqCUodm28135387 = IpgnmuvTFqCUodm85324329;     IpgnmuvTFqCUodm85324329 = IpgnmuvTFqCUodm97388637;     IpgnmuvTFqCUodm97388637 = IpgnmuvTFqCUodm89442033;     IpgnmuvTFqCUodm89442033 = IpgnmuvTFqCUodm55562417;     IpgnmuvTFqCUodm55562417 = IpgnmuvTFqCUodm11616471;     IpgnmuvTFqCUodm11616471 = IpgnmuvTFqCUodm63210340;     IpgnmuvTFqCUodm63210340 = IpgnmuvTFqCUodm34501545;     IpgnmuvTFqCUodm34501545 = IpgnmuvTFqCUodm11653257;     IpgnmuvTFqCUodm11653257 = IpgnmuvTFqCUodm33810362;     IpgnmuvTFqCUodm33810362 = IpgnmuvTFqCUodm1899160;     IpgnmuvTFqCUodm1899160 = IpgnmuvTFqCUodm65422479;     IpgnmuvTFqCUodm65422479 = IpgnmuvTFqCUodm16952868;     IpgnmuvTFqCUodm16952868 = IpgnmuvTFqCUodm48682310;     IpgnmuvTFqCUodm48682310 = IpgnmuvTFqCUodm51500388;     IpgnmuvTFqCUodm51500388 = IpgnmuvTFqCUodm34955589;     IpgnmuvTFqCUodm34955589 = IpgnmuvTFqCUodm2604497;     IpgnmuvTFqCUodm2604497 = IpgnmuvTFqCUodm62553247;     IpgnmuvTFqCUodm62553247 = IpgnmuvTFqCUodm92560137;     IpgnmuvTFqCUodm92560137 = IpgnmuvTFqCUodm91292998;     IpgnmuvTFqCUodm91292998 = IpgnmuvTFqCUodm55289718;     IpgnmuvTFqCUodm55289718 = IpgnmuvTFqCUodm72201946;     IpgnmuvTFqCUodm72201946 = IpgnmuvTFqCUodm57579642;     IpgnmuvTFqCUodm57579642 = IpgnmuvTFqCUodm46676563;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void sJshXYeLLNDCfFVpKJdGIPlEzedOk32602106() {     int PTLLqIhMDrulbXI43189484 = -787496728;    int PTLLqIhMDrulbXI60833125 = 77425900;    int PTLLqIhMDrulbXI85154748 = 7049738;    int PTLLqIhMDrulbXI39111220 = -568022938;    int PTLLqIhMDrulbXI4807598 = -385624051;    int PTLLqIhMDrulbXI15256516 = -884831761;    int PTLLqIhMDrulbXI77911316 = 1831376;    int PTLLqIhMDrulbXI97819506 = -49854640;    int PTLLqIhMDrulbXI49898844 = -228799457;    int PTLLqIhMDrulbXI51656727 = -533103340;    int PTLLqIhMDrulbXI74749425 = 25208243;    int PTLLqIhMDrulbXI81954448 = -850221870;    int PTLLqIhMDrulbXI23519553 = -755701413;    int PTLLqIhMDrulbXI44463239 = 98643605;    int PTLLqIhMDrulbXI34042785 = -447158148;    int PTLLqIhMDrulbXI83026054 = -430480119;    int PTLLqIhMDrulbXI5856991 = -781204569;    int PTLLqIhMDrulbXI92897031 = -127681176;    int PTLLqIhMDrulbXI60898889 = -787642568;    int PTLLqIhMDrulbXI62473195 = -466168314;    int PTLLqIhMDrulbXI12147045 = -998862333;    int PTLLqIhMDrulbXI95546636 = -668741490;    int PTLLqIhMDrulbXI41792690 = -604867204;    int PTLLqIhMDrulbXI19783401 = -393159637;    int PTLLqIhMDrulbXI61251910 = -103863560;    int PTLLqIhMDrulbXI85994942 = -810194142;    int PTLLqIhMDrulbXI40512903 = -115817393;    int PTLLqIhMDrulbXI72763057 = -376708974;    int PTLLqIhMDrulbXI45912523 = -112142068;    int PTLLqIhMDrulbXI47626211 = -221296239;    int PTLLqIhMDrulbXI54787876 = 53577348;    int PTLLqIhMDrulbXI97228834 = -828287867;    int PTLLqIhMDrulbXI18003636 = -274828929;    int PTLLqIhMDrulbXI4026726 = -515318434;    int PTLLqIhMDrulbXI18469391 = -354694471;    int PTLLqIhMDrulbXI85808135 = -223328541;    int PTLLqIhMDrulbXI65571162 = -654389877;    int PTLLqIhMDrulbXI51668031 = -494141642;    int PTLLqIhMDrulbXI77523433 = -815717958;    int PTLLqIhMDrulbXI27464073 = -202526706;    int PTLLqIhMDrulbXI48415102 = -370105702;    int PTLLqIhMDrulbXI34137135 = 1443470;    int PTLLqIhMDrulbXI41104538 = -299000117;    int PTLLqIhMDrulbXI36368449 = -547608358;    int PTLLqIhMDrulbXI76621015 = -146116275;    int PTLLqIhMDrulbXI83269292 = -415333428;    int PTLLqIhMDrulbXI17440637 = -142100631;    int PTLLqIhMDrulbXI17543825 = -186747668;    int PTLLqIhMDrulbXI87505291 = -907023300;    int PTLLqIhMDrulbXI73307251 = -989594830;    int PTLLqIhMDrulbXI64658798 = -742860168;    int PTLLqIhMDrulbXI6449178 = -967787161;    int PTLLqIhMDrulbXI19811668 = -785206484;    int PTLLqIhMDrulbXI84167233 = 663892;    int PTLLqIhMDrulbXI20455061 = 88967001;    int PTLLqIhMDrulbXI47642848 = -18755239;    int PTLLqIhMDrulbXI19040435 = -317706897;    int PTLLqIhMDrulbXI65371347 = -599790626;    int PTLLqIhMDrulbXI77859309 = -364159378;    int PTLLqIhMDrulbXI18812656 = -575429909;    int PTLLqIhMDrulbXI74743613 = -669014368;    int PTLLqIhMDrulbXI5148260 = -621459650;    int PTLLqIhMDrulbXI51906983 = -937712572;    int PTLLqIhMDrulbXI2272634 = 92496781;    int PTLLqIhMDrulbXI96868851 = -486680688;    int PTLLqIhMDrulbXI77520591 = -146503891;    int PTLLqIhMDrulbXI63950812 = -475392941;    int PTLLqIhMDrulbXI19492828 = -140382980;    int PTLLqIhMDrulbXI25993849 = -546661924;    int PTLLqIhMDrulbXI48234649 = -123829608;    int PTLLqIhMDrulbXI17454893 = -776090243;    int PTLLqIhMDrulbXI54188959 = -187062927;    int PTLLqIhMDrulbXI15373598 = -311963219;    int PTLLqIhMDrulbXI33434817 = -485115863;    int PTLLqIhMDrulbXI14058094 = 3937388;    int PTLLqIhMDrulbXI78009910 = -900305804;    int PTLLqIhMDrulbXI54442098 = -269741374;    int PTLLqIhMDrulbXI5424241 = 42741154;    int PTLLqIhMDrulbXI43162385 = -147043362;    int PTLLqIhMDrulbXI77982618 = -688530133;    int PTLLqIhMDrulbXI68554305 = -568093511;    int PTLLqIhMDrulbXI22969079 = -929069726;    int PTLLqIhMDrulbXI85257765 = -469685674;    int PTLLqIhMDrulbXI72605272 = -122547239;    int PTLLqIhMDrulbXI82967412 = -478436072;    int PTLLqIhMDrulbXI48338698 = 21364508;    int PTLLqIhMDrulbXI77417166 = 56918616;    int PTLLqIhMDrulbXI33836403 = -175492822;    int PTLLqIhMDrulbXI83571664 = -504285435;    int PTLLqIhMDrulbXI70826542 = -235939233;    int PTLLqIhMDrulbXI66767700 = -905621645;    int PTLLqIhMDrulbXI199815 = 45400748;    int PTLLqIhMDrulbXI73808721 = -29982264;    int PTLLqIhMDrulbXI58710777 = -140288049;    int PTLLqIhMDrulbXI52720459 = -533512339;    int PTLLqIhMDrulbXI43266842 = -748646052;    int PTLLqIhMDrulbXI82230151 = -60843958;    int PTLLqIhMDrulbXI38831905 = -291496898;    int PTLLqIhMDrulbXI39499598 = 39072329;    int PTLLqIhMDrulbXI99100424 = -787496728;     PTLLqIhMDrulbXI43189484 = PTLLqIhMDrulbXI60833125;     PTLLqIhMDrulbXI60833125 = PTLLqIhMDrulbXI85154748;     PTLLqIhMDrulbXI85154748 = PTLLqIhMDrulbXI39111220;     PTLLqIhMDrulbXI39111220 = PTLLqIhMDrulbXI4807598;     PTLLqIhMDrulbXI4807598 = PTLLqIhMDrulbXI15256516;     PTLLqIhMDrulbXI15256516 = PTLLqIhMDrulbXI77911316;     PTLLqIhMDrulbXI77911316 = PTLLqIhMDrulbXI97819506;     PTLLqIhMDrulbXI97819506 = PTLLqIhMDrulbXI49898844;     PTLLqIhMDrulbXI49898844 = PTLLqIhMDrulbXI51656727;     PTLLqIhMDrulbXI51656727 = PTLLqIhMDrulbXI74749425;     PTLLqIhMDrulbXI74749425 = PTLLqIhMDrulbXI81954448;     PTLLqIhMDrulbXI81954448 = PTLLqIhMDrulbXI23519553;     PTLLqIhMDrulbXI23519553 = PTLLqIhMDrulbXI44463239;     PTLLqIhMDrulbXI44463239 = PTLLqIhMDrulbXI34042785;     PTLLqIhMDrulbXI34042785 = PTLLqIhMDrulbXI83026054;     PTLLqIhMDrulbXI83026054 = PTLLqIhMDrulbXI5856991;     PTLLqIhMDrulbXI5856991 = PTLLqIhMDrulbXI92897031;     PTLLqIhMDrulbXI92897031 = PTLLqIhMDrulbXI60898889;     PTLLqIhMDrulbXI60898889 = PTLLqIhMDrulbXI62473195;     PTLLqIhMDrulbXI62473195 = PTLLqIhMDrulbXI12147045;     PTLLqIhMDrulbXI12147045 = PTLLqIhMDrulbXI95546636;     PTLLqIhMDrulbXI95546636 = PTLLqIhMDrulbXI41792690;     PTLLqIhMDrulbXI41792690 = PTLLqIhMDrulbXI19783401;     PTLLqIhMDrulbXI19783401 = PTLLqIhMDrulbXI61251910;     PTLLqIhMDrulbXI61251910 = PTLLqIhMDrulbXI85994942;     PTLLqIhMDrulbXI85994942 = PTLLqIhMDrulbXI40512903;     PTLLqIhMDrulbXI40512903 = PTLLqIhMDrulbXI72763057;     PTLLqIhMDrulbXI72763057 = PTLLqIhMDrulbXI45912523;     PTLLqIhMDrulbXI45912523 = PTLLqIhMDrulbXI47626211;     PTLLqIhMDrulbXI47626211 = PTLLqIhMDrulbXI54787876;     PTLLqIhMDrulbXI54787876 = PTLLqIhMDrulbXI97228834;     PTLLqIhMDrulbXI97228834 = PTLLqIhMDrulbXI18003636;     PTLLqIhMDrulbXI18003636 = PTLLqIhMDrulbXI4026726;     PTLLqIhMDrulbXI4026726 = PTLLqIhMDrulbXI18469391;     PTLLqIhMDrulbXI18469391 = PTLLqIhMDrulbXI85808135;     PTLLqIhMDrulbXI85808135 = PTLLqIhMDrulbXI65571162;     PTLLqIhMDrulbXI65571162 = PTLLqIhMDrulbXI51668031;     PTLLqIhMDrulbXI51668031 = PTLLqIhMDrulbXI77523433;     PTLLqIhMDrulbXI77523433 = PTLLqIhMDrulbXI27464073;     PTLLqIhMDrulbXI27464073 = PTLLqIhMDrulbXI48415102;     PTLLqIhMDrulbXI48415102 = PTLLqIhMDrulbXI34137135;     PTLLqIhMDrulbXI34137135 = PTLLqIhMDrulbXI41104538;     PTLLqIhMDrulbXI41104538 = PTLLqIhMDrulbXI36368449;     PTLLqIhMDrulbXI36368449 = PTLLqIhMDrulbXI76621015;     PTLLqIhMDrulbXI76621015 = PTLLqIhMDrulbXI83269292;     PTLLqIhMDrulbXI83269292 = PTLLqIhMDrulbXI17440637;     PTLLqIhMDrulbXI17440637 = PTLLqIhMDrulbXI17543825;     PTLLqIhMDrulbXI17543825 = PTLLqIhMDrulbXI87505291;     PTLLqIhMDrulbXI87505291 = PTLLqIhMDrulbXI73307251;     PTLLqIhMDrulbXI73307251 = PTLLqIhMDrulbXI64658798;     PTLLqIhMDrulbXI64658798 = PTLLqIhMDrulbXI6449178;     PTLLqIhMDrulbXI6449178 = PTLLqIhMDrulbXI19811668;     PTLLqIhMDrulbXI19811668 = PTLLqIhMDrulbXI84167233;     PTLLqIhMDrulbXI84167233 = PTLLqIhMDrulbXI20455061;     PTLLqIhMDrulbXI20455061 = PTLLqIhMDrulbXI47642848;     PTLLqIhMDrulbXI47642848 = PTLLqIhMDrulbXI19040435;     PTLLqIhMDrulbXI19040435 = PTLLqIhMDrulbXI65371347;     PTLLqIhMDrulbXI65371347 = PTLLqIhMDrulbXI77859309;     PTLLqIhMDrulbXI77859309 = PTLLqIhMDrulbXI18812656;     PTLLqIhMDrulbXI18812656 = PTLLqIhMDrulbXI74743613;     PTLLqIhMDrulbXI74743613 = PTLLqIhMDrulbXI5148260;     PTLLqIhMDrulbXI5148260 = PTLLqIhMDrulbXI51906983;     PTLLqIhMDrulbXI51906983 = PTLLqIhMDrulbXI2272634;     PTLLqIhMDrulbXI2272634 = PTLLqIhMDrulbXI96868851;     PTLLqIhMDrulbXI96868851 = PTLLqIhMDrulbXI77520591;     PTLLqIhMDrulbXI77520591 = PTLLqIhMDrulbXI63950812;     PTLLqIhMDrulbXI63950812 = PTLLqIhMDrulbXI19492828;     PTLLqIhMDrulbXI19492828 = PTLLqIhMDrulbXI25993849;     PTLLqIhMDrulbXI25993849 = PTLLqIhMDrulbXI48234649;     PTLLqIhMDrulbXI48234649 = PTLLqIhMDrulbXI17454893;     PTLLqIhMDrulbXI17454893 = PTLLqIhMDrulbXI54188959;     PTLLqIhMDrulbXI54188959 = PTLLqIhMDrulbXI15373598;     PTLLqIhMDrulbXI15373598 = PTLLqIhMDrulbXI33434817;     PTLLqIhMDrulbXI33434817 = PTLLqIhMDrulbXI14058094;     PTLLqIhMDrulbXI14058094 = PTLLqIhMDrulbXI78009910;     PTLLqIhMDrulbXI78009910 = PTLLqIhMDrulbXI54442098;     PTLLqIhMDrulbXI54442098 = PTLLqIhMDrulbXI5424241;     PTLLqIhMDrulbXI5424241 = PTLLqIhMDrulbXI43162385;     PTLLqIhMDrulbXI43162385 = PTLLqIhMDrulbXI77982618;     PTLLqIhMDrulbXI77982618 = PTLLqIhMDrulbXI68554305;     PTLLqIhMDrulbXI68554305 = PTLLqIhMDrulbXI22969079;     PTLLqIhMDrulbXI22969079 = PTLLqIhMDrulbXI85257765;     PTLLqIhMDrulbXI85257765 = PTLLqIhMDrulbXI72605272;     PTLLqIhMDrulbXI72605272 = PTLLqIhMDrulbXI82967412;     PTLLqIhMDrulbXI82967412 = PTLLqIhMDrulbXI48338698;     PTLLqIhMDrulbXI48338698 = PTLLqIhMDrulbXI77417166;     PTLLqIhMDrulbXI77417166 = PTLLqIhMDrulbXI33836403;     PTLLqIhMDrulbXI33836403 = PTLLqIhMDrulbXI83571664;     PTLLqIhMDrulbXI83571664 = PTLLqIhMDrulbXI70826542;     PTLLqIhMDrulbXI70826542 = PTLLqIhMDrulbXI66767700;     PTLLqIhMDrulbXI66767700 = PTLLqIhMDrulbXI199815;     PTLLqIhMDrulbXI199815 = PTLLqIhMDrulbXI73808721;     PTLLqIhMDrulbXI73808721 = PTLLqIhMDrulbXI58710777;     PTLLqIhMDrulbXI58710777 = PTLLqIhMDrulbXI52720459;     PTLLqIhMDrulbXI52720459 = PTLLqIhMDrulbXI43266842;     PTLLqIhMDrulbXI43266842 = PTLLqIhMDrulbXI82230151;     PTLLqIhMDrulbXI82230151 = PTLLqIhMDrulbXI38831905;     PTLLqIhMDrulbXI38831905 = PTLLqIhMDrulbXI39499598;     PTLLqIhMDrulbXI39499598 = PTLLqIhMDrulbXI99100424;     PTLLqIhMDrulbXI99100424 = PTLLqIhMDrulbXI43189484;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bfxtWYywgoGsXmOVMpkxePxNgNHYo82548913() {     int kOrqGAwaPqjmAsC10360543 = -431922405;    int kOrqGAwaPqjmAsC6017152 = -804310576;    int kOrqGAwaPqjmAsC19492766 = -226218874;    int kOrqGAwaPqjmAsC6511495 = -306434288;    int kOrqGAwaPqjmAsC86070206 = -356262073;    int kOrqGAwaPqjmAsC7697719 = -917507677;    int kOrqGAwaPqjmAsC1557530 = -457325841;    int kOrqGAwaPqjmAsC20900007 = -805378596;    int kOrqGAwaPqjmAsC17277992 = -860918470;    int kOrqGAwaPqjmAsC31988210 = -856117032;    int kOrqGAwaPqjmAsC56894096 = -25456852;    int kOrqGAwaPqjmAsC28225780 = -899240206;    int kOrqGAwaPqjmAsC76764723 = -929365661;    int kOrqGAwaPqjmAsC41541165 = -966753089;    int kOrqGAwaPqjmAsC76490452 = -485522010;    int kOrqGAwaPqjmAsC53946971 = -526289270;    int kOrqGAwaPqjmAsC71547906 = -783622173;    int kOrqGAwaPqjmAsC26325481 = -337301816;    int kOrqGAwaPqjmAsC4101378 = -265975544;    int kOrqGAwaPqjmAsC89911400 = -306034998;    int kOrqGAwaPqjmAsC95292230 = -307624698;    int kOrqGAwaPqjmAsC19547893 = -868052605;    int kOrqGAwaPqjmAsC96355064 = -994672217;    int kOrqGAwaPqjmAsC33807600 = -63757425;    int kOrqGAwaPqjmAsC75208616 = 21499846;    int kOrqGAwaPqjmAsC76042810 = -79567921;    int kOrqGAwaPqjmAsC26448792 = -37377303;    int kOrqGAwaPqjmAsC37392120 = 71378148;    int kOrqGAwaPqjmAsC47572535 = -725807953;    int kOrqGAwaPqjmAsC10962945 = -469036778;    int kOrqGAwaPqjmAsC37541273 = -709731257;    int kOrqGAwaPqjmAsC92766620 = 3230711;    int kOrqGAwaPqjmAsC32941013 = 90056910;    int kOrqGAwaPqjmAsC81768531 = -22676385;    int kOrqGAwaPqjmAsC23620427 = -156992852;    int kOrqGAwaPqjmAsC46644458 = -499866051;    int kOrqGAwaPqjmAsC77978246 = -253753664;    int kOrqGAwaPqjmAsC7669815 = -104124039;    int kOrqGAwaPqjmAsC69477432 = -399833929;    int kOrqGAwaPqjmAsC30211877 = -647051265;    int kOrqGAwaPqjmAsC52903713 = -97722853;    int kOrqGAwaPqjmAsC21115817 = -81594479;    int kOrqGAwaPqjmAsC17330074 = -222896486;    int kOrqGAwaPqjmAsC95852959 = -104252640;    int kOrqGAwaPqjmAsC54543737 = -513657606;    int kOrqGAwaPqjmAsC46928185 = -693953708;    int kOrqGAwaPqjmAsC12786140 = -55410457;    int kOrqGAwaPqjmAsC88264476 = -193210606;    int kOrqGAwaPqjmAsC92897422 = -860527603;    int kOrqGAwaPqjmAsC34490354 = -935118277;    int kOrqGAwaPqjmAsC71377393 = -240256488;    int kOrqGAwaPqjmAsC6182064 = -37026384;    int kOrqGAwaPqjmAsC91582355 = -512859162;    int kOrqGAwaPqjmAsC49651463 = -578498757;    int kOrqGAwaPqjmAsC87256683 = -697796428;    int kOrqGAwaPqjmAsC90812650 = -563869801;    int kOrqGAwaPqjmAsC9662087 = -809638360;    int kOrqGAwaPqjmAsC85685166 = -62461450;    int kOrqGAwaPqjmAsC31302878 = -227934134;    int kOrqGAwaPqjmAsC10027397 = -176694152;    int kOrqGAwaPqjmAsC81248926 = -780130375;    int kOrqGAwaPqjmAsC64165409 = -428703990;    int kOrqGAwaPqjmAsC73327471 = 20429356;    int kOrqGAwaPqjmAsC6315047 = -291881692;    int kOrqGAwaPqjmAsC94446937 = -46385776;    int kOrqGAwaPqjmAsC64127475 = 71312437;    int kOrqGAwaPqjmAsC95284767 = -889297117;    int kOrqGAwaPqjmAsC94996192 = -806689276;    int kOrqGAwaPqjmAsC17920738 = -709760237;    int kOrqGAwaPqjmAsC29845994 = -985655959;    int kOrqGAwaPqjmAsC75968724 = -172535607;    int kOrqGAwaPqjmAsC63878091 = -579498135;    int kOrqGAwaPqjmAsC56848049 = -937467887;    int kOrqGAwaPqjmAsC73889500 = -618924280;    int kOrqGAwaPqjmAsC37007688 = -108312146;    int kOrqGAwaPqjmAsC74176413 = -126030219;    int kOrqGAwaPqjmAsC2217819 = -545156119;    int kOrqGAwaPqjmAsC502106 = -790419577;    int kOrqGAwaPqjmAsC79263862 = -550099819;    int kOrqGAwaPqjmAsC28280431 = -284546447;    int kOrqGAwaPqjmAsC63256670 = 75842535;    int kOrqGAwaPqjmAsC38184315 = -844166697;    int kOrqGAwaPqjmAsC44494698 = -68094249;    int kOrqGAwaPqjmAsC13082182 = -790689676;    int kOrqGAwaPqjmAsC39585551 = -128780291;    int kOrqGAwaPqjmAsC31359210 = -572704873;    int kOrqGAwaPqjmAsC1184266 = -483910128;    int kOrqGAwaPqjmAsC83289549 = -331444333;    int kOrqGAwaPqjmAsC94511848 = -324879958;    int kOrqGAwaPqjmAsC32807777 = -593123051;    int kOrqGAwaPqjmAsC36982372 = -690227692;    int kOrqGAwaPqjmAsC92293080 = -91292215;    int kOrqGAwaPqjmAsC76366937 = -876189906;    int kOrqGAwaPqjmAsC59450035 = -123139778;    int kOrqGAwaPqjmAsC48962950 = -866920890;    int kOrqGAwaPqjmAsC88738303 = -669018863;    int kOrqGAwaPqjmAsC47788346 = -2023836;    int kOrqGAwaPqjmAsC11015027 = -931014795;    int kOrqGAwaPqjmAsC1406023 = 42133136;    int kOrqGAwaPqjmAsC90416261 = -431922405;     kOrqGAwaPqjmAsC10360543 = kOrqGAwaPqjmAsC6017152;     kOrqGAwaPqjmAsC6017152 = kOrqGAwaPqjmAsC19492766;     kOrqGAwaPqjmAsC19492766 = kOrqGAwaPqjmAsC6511495;     kOrqGAwaPqjmAsC6511495 = kOrqGAwaPqjmAsC86070206;     kOrqGAwaPqjmAsC86070206 = kOrqGAwaPqjmAsC7697719;     kOrqGAwaPqjmAsC7697719 = kOrqGAwaPqjmAsC1557530;     kOrqGAwaPqjmAsC1557530 = kOrqGAwaPqjmAsC20900007;     kOrqGAwaPqjmAsC20900007 = kOrqGAwaPqjmAsC17277992;     kOrqGAwaPqjmAsC17277992 = kOrqGAwaPqjmAsC31988210;     kOrqGAwaPqjmAsC31988210 = kOrqGAwaPqjmAsC56894096;     kOrqGAwaPqjmAsC56894096 = kOrqGAwaPqjmAsC28225780;     kOrqGAwaPqjmAsC28225780 = kOrqGAwaPqjmAsC76764723;     kOrqGAwaPqjmAsC76764723 = kOrqGAwaPqjmAsC41541165;     kOrqGAwaPqjmAsC41541165 = kOrqGAwaPqjmAsC76490452;     kOrqGAwaPqjmAsC76490452 = kOrqGAwaPqjmAsC53946971;     kOrqGAwaPqjmAsC53946971 = kOrqGAwaPqjmAsC71547906;     kOrqGAwaPqjmAsC71547906 = kOrqGAwaPqjmAsC26325481;     kOrqGAwaPqjmAsC26325481 = kOrqGAwaPqjmAsC4101378;     kOrqGAwaPqjmAsC4101378 = kOrqGAwaPqjmAsC89911400;     kOrqGAwaPqjmAsC89911400 = kOrqGAwaPqjmAsC95292230;     kOrqGAwaPqjmAsC95292230 = kOrqGAwaPqjmAsC19547893;     kOrqGAwaPqjmAsC19547893 = kOrqGAwaPqjmAsC96355064;     kOrqGAwaPqjmAsC96355064 = kOrqGAwaPqjmAsC33807600;     kOrqGAwaPqjmAsC33807600 = kOrqGAwaPqjmAsC75208616;     kOrqGAwaPqjmAsC75208616 = kOrqGAwaPqjmAsC76042810;     kOrqGAwaPqjmAsC76042810 = kOrqGAwaPqjmAsC26448792;     kOrqGAwaPqjmAsC26448792 = kOrqGAwaPqjmAsC37392120;     kOrqGAwaPqjmAsC37392120 = kOrqGAwaPqjmAsC47572535;     kOrqGAwaPqjmAsC47572535 = kOrqGAwaPqjmAsC10962945;     kOrqGAwaPqjmAsC10962945 = kOrqGAwaPqjmAsC37541273;     kOrqGAwaPqjmAsC37541273 = kOrqGAwaPqjmAsC92766620;     kOrqGAwaPqjmAsC92766620 = kOrqGAwaPqjmAsC32941013;     kOrqGAwaPqjmAsC32941013 = kOrqGAwaPqjmAsC81768531;     kOrqGAwaPqjmAsC81768531 = kOrqGAwaPqjmAsC23620427;     kOrqGAwaPqjmAsC23620427 = kOrqGAwaPqjmAsC46644458;     kOrqGAwaPqjmAsC46644458 = kOrqGAwaPqjmAsC77978246;     kOrqGAwaPqjmAsC77978246 = kOrqGAwaPqjmAsC7669815;     kOrqGAwaPqjmAsC7669815 = kOrqGAwaPqjmAsC69477432;     kOrqGAwaPqjmAsC69477432 = kOrqGAwaPqjmAsC30211877;     kOrqGAwaPqjmAsC30211877 = kOrqGAwaPqjmAsC52903713;     kOrqGAwaPqjmAsC52903713 = kOrqGAwaPqjmAsC21115817;     kOrqGAwaPqjmAsC21115817 = kOrqGAwaPqjmAsC17330074;     kOrqGAwaPqjmAsC17330074 = kOrqGAwaPqjmAsC95852959;     kOrqGAwaPqjmAsC95852959 = kOrqGAwaPqjmAsC54543737;     kOrqGAwaPqjmAsC54543737 = kOrqGAwaPqjmAsC46928185;     kOrqGAwaPqjmAsC46928185 = kOrqGAwaPqjmAsC12786140;     kOrqGAwaPqjmAsC12786140 = kOrqGAwaPqjmAsC88264476;     kOrqGAwaPqjmAsC88264476 = kOrqGAwaPqjmAsC92897422;     kOrqGAwaPqjmAsC92897422 = kOrqGAwaPqjmAsC34490354;     kOrqGAwaPqjmAsC34490354 = kOrqGAwaPqjmAsC71377393;     kOrqGAwaPqjmAsC71377393 = kOrqGAwaPqjmAsC6182064;     kOrqGAwaPqjmAsC6182064 = kOrqGAwaPqjmAsC91582355;     kOrqGAwaPqjmAsC91582355 = kOrqGAwaPqjmAsC49651463;     kOrqGAwaPqjmAsC49651463 = kOrqGAwaPqjmAsC87256683;     kOrqGAwaPqjmAsC87256683 = kOrqGAwaPqjmAsC90812650;     kOrqGAwaPqjmAsC90812650 = kOrqGAwaPqjmAsC9662087;     kOrqGAwaPqjmAsC9662087 = kOrqGAwaPqjmAsC85685166;     kOrqGAwaPqjmAsC85685166 = kOrqGAwaPqjmAsC31302878;     kOrqGAwaPqjmAsC31302878 = kOrqGAwaPqjmAsC10027397;     kOrqGAwaPqjmAsC10027397 = kOrqGAwaPqjmAsC81248926;     kOrqGAwaPqjmAsC81248926 = kOrqGAwaPqjmAsC64165409;     kOrqGAwaPqjmAsC64165409 = kOrqGAwaPqjmAsC73327471;     kOrqGAwaPqjmAsC73327471 = kOrqGAwaPqjmAsC6315047;     kOrqGAwaPqjmAsC6315047 = kOrqGAwaPqjmAsC94446937;     kOrqGAwaPqjmAsC94446937 = kOrqGAwaPqjmAsC64127475;     kOrqGAwaPqjmAsC64127475 = kOrqGAwaPqjmAsC95284767;     kOrqGAwaPqjmAsC95284767 = kOrqGAwaPqjmAsC94996192;     kOrqGAwaPqjmAsC94996192 = kOrqGAwaPqjmAsC17920738;     kOrqGAwaPqjmAsC17920738 = kOrqGAwaPqjmAsC29845994;     kOrqGAwaPqjmAsC29845994 = kOrqGAwaPqjmAsC75968724;     kOrqGAwaPqjmAsC75968724 = kOrqGAwaPqjmAsC63878091;     kOrqGAwaPqjmAsC63878091 = kOrqGAwaPqjmAsC56848049;     kOrqGAwaPqjmAsC56848049 = kOrqGAwaPqjmAsC73889500;     kOrqGAwaPqjmAsC73889500 = kOrqGAwaPqjmAsC37007688;     kOrqGAwaPqjmAsC37007688 = kOrqGAwaPqjmAsC74176413;     kOrqGAwaPqjmAsC74176413 = kOrqGAwaPqjmAsC2217819;     kOrqGAwaPqjmAsC2217819 = kOrqGAwaPqjmAsC502106;     kOrqGAwaPqjmAsC502106 = kOrqGAwaPqjmAsC79263862;     kOrqGAwaPqjmAsC79263862 = kOrqGAwaPqjmAsC28280431;     kOrqGAwaPqjmAsC28280431 = kOrqGAwaPqjmAsC63256670;     kOrqGAwaPqjmAsC63256670 = kOrqGAwaPqjmAsC38184315;     kOrqGAwaPqjmAsC38184315 = kOrqGAwaPqjmAsC44494698;     kOrqGAwaPqjmAsC44494698 = kOrqGAwaPqjmAsC13082182;     kOrqGAwaPqjmAsC13082182 = kOrqGAwaPqjmAsC39585551;     kOrqGAwaPqjmAsC39585551 = kOrqGAwaPqjmAsC31359210;     kOrqGAwaPqjmAsC31359210 = kOrqGAwaPqjmAsC1184266;     kOrqGAwaPqjmAsC1184266 = kOrqGAwaPqjmAsC83289549;     kOrqGAwaPqjmAsC83289549 = kOrqGAwaPqjmAsC94511848;     kOrqGAwaPqjmAsC94511848 = kOrqGAwaPqjmAsC32807777;     kOrqGAwaPqjmAsC32807777 = kOrqGAwaPqjmAsC36982372;     kOrqGAwaPqjmAsC36982372 = kOrqGAwaPqjmAsC92293080;     kOrqGAwaPqjmAsC92293080 = kOrqGAwaPqjmAsC76366937;     kOrqGAwaPqjmAsC76366937 = kOrqGAwaPqjmAsC59450035;     kOrqGAwaPqjmAsC59450035 = kOrqGAwaPqjmAsC48962950;     kOrqGAwaPqjmAsC48962950 = kOrqGAwaPqjmAsC88738303;     kOrqGAwaPqjmAsC88738303 = kOrqGAwaPqjmAsC47788346;     kOrqGAwaPqjmAsC47788346 = kOrqGAwaPqjmAsC11015027;     kOrqGAwaPqjmAsC11015027 = kOrqGAwaPqjmAsC1406023;     kOrqGAwaPqjmAsC1406023 = kOrqGAwaPqjmAsC90416261;     kOrqGAwaPqjmAsC90416261 = kOrqGAwaPqjmAsC10360543;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void SKbMlniSEuLcvjCVesrRDUUmzUyDF84738251() {     int qDaoxcOrcuxJmlw6873464 = -430723093;    int qDaoxcOrcuxJmlw21099977 = -756998878;    int qDaoxcOrcuxJmlw75175076 = 25678360;    int qDaoxcOrcuxJmlw19060343 = -775248078;    int qDaoxcOrcuxJmlw54722597 = 88150932;    int qDaoxcOrcuxJmlw44944591 = -948035253;    int qDaoxcOrcuxJmlw75372182 = -622243816;    int qDaoxcOrcuxJmlw56352177 = -918262781;    int qDaoxcOrcuxJmlw21532248 = -13336223;    int qDaoxcOrcuxJmlw85992746 = -514842245;    int qDaoxcOrcuxJmlw17110212 = -692089;    int qDaoxcOrcuxJmlw43242217 = -367973112;    int qDaoxcOrcuxJmlw82035421 = -115151324;    int qDaoxcOrcuxJmlw8616279 = -376274624;    int qDaoxcOrcuxJmlw81390209 = -336848720;    int qDaoxcOrcuxJmlw66903794 = -284910485;    int qDaoxcOrcuxJmlw99238754 = -696469218;    int qDaoxcOrcuxJmlw62460969 = -954827881;    int qDaoxcOrcuxJmlw86024353 = -755102844;    int qDaoxcOrcuxJmlw30636660 = -215559829;    int qDaoxcOrcuxJmlw2971085 = -518174296;    int qDaoxcOrcuxJmlw60449650 = -146070297;    int qDaoxcOrcuxJmlw85852027 = -175591927;    int qDaoxcOrcuxJmlw57632370 = -22874829;    int qDaoxcOrcuxJmlw53483624 = -603925913;    int qDaoxcOrcuxJmlw52067175 = -258863184;    int qDaoxcOrcuxJmlw74345476 = -964559149;    int qDaoxcOrcuxJmlw29642483 = -684377951;    int qDaoxcOrcuxJmlw29768507 = -591185936;    int qDaoxcOrcuxJmlw11217452 = -933813620;    int qDaoxcOrcuxJmlw46635353 = 88300590;    int qDaoxcOrcuxJmlw28516804 = -328775309;    int qDaoxcOrcuxJmlw50385451 = -900039857;    int qDaoxcOrcuxJmlw94428199 = -943186385;    int qDaoxcOrcuxJmlw33105266 = -371658377;    int qDaoxcOrcuxJmlw90315711 = -776942760;    int qDaoxcOrcuxJmlw58535213 = -864177622;    int qDaoxcOrcuxJmlw80796789 = 74743992;    int qDaoxcOrcuxJmlw18211739 = -740902679;    int qDaoxcOrcuxJmlw9729277 = -83399147;    int qDaoxcOrcuxJmlw85174707 = -629265024;    int qDaoxcOrcuxJmlw65309169 = -589035442;    int qDaoxcOrcuxJmlw4872010 = -719169059;    int qDaoxcOrcuxJmlw8061068 = -245886394;    int qDaoxcOrcuxJmlw20530452 = 54188056;    int qDaoxcOrcuxJmlw44609212 = 73718496;    int qDaoxcOrcuxJmlw9698233 = -333931105;    int qDaoxcOrcuxJmlw68754500 = -356304605;    int qDaoxcOrcuxJmlw11506491 = -392031798;    int qDaoxcOrcuxJmlw7291393 = -184657733;    int qDaoxcOrcuxJmlw23166034 = -869635694;    int qDaoxcOrcuxJmlw78590701 = -619191635;    int qDaoxcOrcuxJmlw83725733 = -204767806;    int qDaoxcOrcuxJmlw35158659 = 71300239;    int qDaoxcOrcuxJmlw81767166 = -30518221;    int qDaoxcOrcuxJmlw46423814 = -184652797;    int qDaoxcOrcuxJmlw35247950 = -481406952;    int qDaoxcOrcuxJmlw17542707 = -951446811;    int qDaoxcOrcuxJmlw65576719 = -71322165;    int qDaoxcOrcuxJmlw2655423 = -652985885;    int qDaoxcOrcuxJmlw70599114 = -983476104;    int qDaoxcOrcuxJmlw45729700 = -937865866;    int qDaoxcOrcuxJmlw26583671 = -227076845;    int qDaoxcOrcuxJmlw10314796 = -79522604;    int qDaoxcOrcuxJmlw39357394 = -503142836;    int qDaoxcOrcuxJmlw88593407 = -671916780;    int qDaoxcOrcuxJmlw92856765 = -467933255;    int qDaoxcOrcuxJmlw87607221 = -171964940;    int qDaoxcOrcuxJmlw75511012 = 95383753;    int qDaoxcOrcuxJmlw91074497 = -559905961;    int qDaoxcOrcuxJmlw8368581 = -420732864;    int qDaoxcOrcuxJmlw18441966 = -671213211;    int qDaoxcOrcuxJmlw44249230 = -113925202;    int qDaoxcOrcuxJmlw76295077 = -571703698;    int qDaoxcOrcuxJmlw45461952 = -586294806;    int qDaoxcOrcuxJmlw37661916 = -929138855;    int qDaoxcOrcuxJmlw55577640 = -426901239;    int qDaoxcOrcuxJmlw77790959 = -929705534;    int qDaoxcOrcuxJmlw37101918 = 22937114;    int qDaoxcOrcuxJmlw8874412 = -577644410;    int qDaoxcOrcuxJmlw42368942 = -924932079;    int qDaoxcOrcuxJmlw5590977 = -508254545;    int qDaoxcOrcuxJmlw18135993 = -192346153;    int qDaoxcOrcuxJmlw22477114 = -306528204;    int qDaoxcOrcuxJmlw88051418 = 35822073;    int qDaoxcOrcuxJmlw68044651 = -292507776;    int qDaoxcOrcuxJmlw44791071 = -24007504;    int qDaoxcOrcuxJmlw15226793 = -871340097;    int qDaoxcOrcuxJmlw12661033 = -812668165;    int qDaoxcOrcuxJmlw86681451 = -87005581;    int qDaoxcOrcuxJmlw55067762 = -195535808;    int qDaoxcOrcuxJmlw40992507 = -912730811;    int qDaoxcOrcuxJmlw15220071 = -853933843;    int qDaoxcOrcuxJmlw15556317 = 12083205;    int qDaoxcOrcuxJmlw39130162 = -99923043;    int qDaoxcOrcuxJmlw39445008 = -691399158;    int qDaoxcOrcuxJmlw38725499 = -261958597;    int qDaoxcOrcuxJmlw94557213 = -539646456;    int qDaoxcOrcuxJmlw68703674 = -742743559;    int qDaoxcOrcuxJmlw31937044 = -430723093;     qDaoxcOrcuxJmlw6873464 = qDaoxcOrcuxJmlw21099977;     qDaoxcOrcuxJmlw21099977 = qDaoxcOrcuxJmlw75175076;     qDaoxcOrcuxJmlw75175076 = qDaoxcOrcuxJmlw19060343;     qDaoxcOrcuxJmlw19060343 = qDaoxcOrcuxJmlw54722597;     qDaoxcOrcuxJmlw54722597 = qDaoxcOrcuxJmlw44944591;     qDaoxcOrcuxJmlw44944591 = qDaoxcOrcuxJmlw75372182;     qDaoxcOrcuxJmlw75372182 = qDaoxcOrcuxJmlw56352177;     qDaoxcOrcuxJmlw56352177 = qDaoxcOrcuxJmlw21532248;     qDaoxcOrcuxJmlw21532248 = qDaoxcOrcuxJmlw85992746;     qDaoxcOrcuxJmlw85992746 = qDaoxcOrcuxJmlw17110212;     qDaoxcOrcuxJmlw17110212 = qDaoxcOrcuxJmlw43242217;     qDaoxcOrcuxJmlw43242217 = qDaoxcOrcuxJmlw82035421;     qDaoxcOrcuxJmlw82035421 = qDaoxcOrcuxJmlw8616279;     qDaoxcOrcuxJmlw8616279 = qDaoxcOrcuxJmlw81390209;     qDaoxcOrcuxJmlw81390209 = qDaoxcOrcuxJmlw66903794;     qDaoxcOrcuxJmlw66903794 = qDaoxcOrcuxJmlw99238754;     qDaoxcOrcuxJmlw99238754 = qDaoxcOrcuxJmlw62460969;     qDaoxcOrcuxJmlw62460969 = qDaoxcOrcuxJmlw86024353;     qDaoxcOrcuxJmlw86024353 = qDaoxcOrcuxJmlw30636660;     qDaoxcOrcuxJmlw30636660 = qDaoxcOrcuxJmlw2971085;     qDaoxcOrcuxJmlw2971085 = qDaoxcOrcuxJmlw60449650;     qDaoxcOrcuxJmlw60449650 = qDaoxcOrcuxJmlw85852027;     qDaoxcOrcuxJmlw85852027 = qDaoxcOrcuxJmlw57632370;     qDaoxcOrcuxJmlw57632370 = qDaoxcOrcuxJmlw53483624;     qDaoxcOrcuxJmlw53483624 = qDaoxcOrcuxJmlw52067175;     qDaoxcOrcuxJmlw52067175 = qDaoxcOrcuxJmlw74345476;     qDaoxcOrcuxJmlw74345476 = qDaoxcOrcuxJmlw29642483;     qDaoxcOrcuxJmlw29642483 = qDaoxcOrcuxJmlw29768507;     qDaoxcOrcuxJmlw29768507 = qDaoxcOrcuxJmlw11217452;     qDaoxcOrcuxJmlw11217452 = qDaoxcOrcuxJmlw46635353;     qDaoxcOrcuxJmlw46635353 = qDaoxcOrcuxJmlw28516804;     qDaoxcOrcuxJmlw28516804 = qDaoxcOrcuxJmlw50385451;     qDaoxcOrcuxJmlw50385451 = qDaoxcOrcuxJmlw94428199;     qDaoxcOrcuxJmlw94428199 = qDaoxcOrcuxJmlw33105266;     qDaoxcOrcuxJmlw33105266 = qDaoxcOrcuxJmlw90315711;     qDaoxcOrcuxJmlw90315711 = qDaoxcOrcuxJmlw58535213;     qDaoxcOrcuxJmlw58535213 = qDaoxcOrcuxJmlw80796789;     qDaoxcOrcuxJmlw80796789 = qDaoxcOrcuxJmlw18211739;     qDaoxcOrcuxJmlw18211739 = qDaoxcOrcuxJmlw9729277;     qDaoxcOrcuxJmlw9729277 = qDaoxcOrcuxJmlw85174707;     qDaoxcOrcuxJmlw85174707 = qDaoxcOrcuxJmlw65309169;     qDaoxcOrcuxJmlw65309169 = qDaoxcOrcuxJmlw4872010;     qDaoxcOrcuxJmlw4872010 = qDaoxcOrcuxJmlw8061068;     qDaoxcOrcuxJmlw8061068 = qDaoxcOrcuxJmlw20530452;     qDaoxcOrcuxJmlw20530452 = qDaoxcOrcuxJmlw44609212;     qDaoxcOrcuxJmlw44609212 = qDaoxcOrcuxJmlw9698233;     qDaoxcOrcuxJmlw9698233 = qDaoxcOrcuxJmlw68754500;     qDaoxcOrcuxJmlw68754500 = qDaoxcOrcuxJmlw11506491;     qDaoxcOrcuxJmlw11506491 = qDaoxcOrcuxJmlw7291393;     qDaoxcOrcuxJmlw7291393 = qDaoxcOrcuxJmlw23166034;     qDaoxcOrcuxJmlw23166034 = qDaoxcOrcuxJmlw78590701;     qDaoxcOrcuxJmlw78590701 = qDaoxcOrcuxJmlw83725733;     qDaoxcOrcuxJmlw83725733 = qDaoxcOrcuxJmlw35158659;     qDaoxcOrcuxJmlw35158659 = qDaoxcOrcuxJmlw81767166;     qDaoxcOrcuxJmlw81767166 = qDaoxcOrcuxJmlw46423814;     qDaoxcOrcuxJmlw46423814 = qDaoxcOrcuxJmlw35247950;     qDaoxcOrcuxJmlw35247950 = qDaoxcOrcuxJmlw17542707;     qDaoxcOrcuxJmlw17542707 = qDaoxcOrcuxJmlw65576719;     qDaoxcOrcuxJmlw65576719 = qDaoxcOrcuxJmlw2655423;     qDaoxcOrcuxJmlw2655423 = qDaoxcOrcuxJmlw70599114;     qDaoxcOrcuxJmlw70599114 = qDaoxcOrcuxJmlw45729700;     qDaoxcOrcuxJmlw45729700 = qDaoxcOrcuxJmlw26583671;     qDaoxcOrcuxJmlw26583671 = qDaoxcOrcuxJmlw10314796;     qDaoxcOrcuxJmlw10314796 = qDaoxcOrcuxJmlw39357394;     qDaoxcOrcuxJmlw39357394 = qDaoxcOrcuxJmlw88593407;     qDaoxcOrcuxJmlw88593407 = qDaoxcOrcuxJmlw92856765;     qDaoxcOrcuxJmlw92856765 = qDaoxcOrcuxJmlw87607221;     qDaoxcOrcuxJmlw87607221 = qDaoxcOrcuxJmlw75511012;     qDaoxcOrcuxJmlw75511012 = qDaoxcOrcuxJmlw91074497;     qDaoxcOrcuxJmlw91074497 = qDaoxcOrcuxJmlw8368581;     qDaoxcOrcuxJmlw8368581 = qDaoxcOrcuxJmlw18441966;     qDaoxcOrcuxJmlw18441966 = qDaoxcOrcuxJmlw44249230;     qDaoxcOrcuxJmlw44249230 = qDaoxcOrcuxJmlw76295077;     qDaoxcOrcuxJmlw76295077 = qDaoxcOrcuxJmlw45461952;     qDaoxcOrcuxJmlw45461952 = qDaoxcOrcuxJmlw37661916;     qDaoxcOrcuxJmlw37661916 = qDaoxcOrcuxJmlw55577640;     qDaoxcOrcuxJmlw55577640 = qDaoxcOrcuxJmlw77790959;     qDaoxcOrcuxJmlw77790959 = qDaoxcOrcuxJmlw37101918;     qDaoxcOrcuxJmlw37101918 = qDaoxcOrcuxJmlw8874412;     qDaoxcOrcuxJmlw8874412 = qDaoxcOrcuxJmlw42368942;     qDaoxcOrcuxJmlw42368942 = qDaoxcOrcuxJmlw5590977;     qDaoxcOrcuxJmlw5590977 = qDaoxcOrcuxJmlw18135993;     qDaoxcOrcuxJmlw18135993 = qDaoxcOrcuxJmlw22477114;     qDaoxcOrcuxJmlw22477114 = qDaoxcOrcuxJmlw88051418;     qDaoxcOrcuxJmlw88051418 = qDaoxcOrcuxJmlw68044651;     qDaoxcOrcuxJmlw68044651 = qDaoxcOrcuxJmlw44791071;     qDaoxcOrcuxJmlw44791071 = qDaoxcOrcuxJmlw15226793;     qDaoxcOrcuxJmlw15226793 = qDaoxcOrcuxJmlw12661033;     qDaoxcOrcuxJmlw12661033 = qDaoxcOrcuxJmlw86681451;     qDaoxcOrcuxJmlw86681451 = qDaoxcOrcuxJmlw55067762;     qDaoxcOrcuxJmlw55067762 = qDaoxcOrcuxJmlw40992507;     qDaoxcOrcuxJmlw40992507 = qDaoxcOrcuxJmlw15220071;     qDaoxcOrcuxJmlw15220071 = qDaoxcOrcuxJmlw15556317;     qDaoxcOrcuxJmlw15556317 = qDaoxcOrcuxJmlw39130162;     qDaoxcOrcuxJmlw39130162 = qDaoxcOrcuxJmlw39445008;     qDaoxcOrcuxJmlw39445008 = qDaoxcOrcuxJmlw38725499;     qDaoxcOrcuxJmlw38725499 = qDaoxcOrcuxJmlw94557213;     qDaoxcOrcuxJmlw94557213 = qDaoxcOrcuxJmlw68703674;     qDaoxcOrcuxJmlw68703674 = qDaoxcOrcuxJmlw31937044;     qDaoxcOrcuxJmlw31937044 = qDaoxcOrcuxJmlw6873464;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FmNhxfIXGAFUFViKyFyYXrdWgptJT86927590() {     int NddXAXVePkEVLuH3386385 = -429523782;    int NddXAXVePkEVLuH36182802 = -709687180;    int NddXAXVePkEVLuH30857388 = -822424405;    int NddXAXVePkEVLuH31609191 = -144061869;    int NddXAXVePkEVLuH23374987 = -567436064;    int NddXAXVePkEVLuH82191462 = -978562830;    int NddXAXVePkEVLuH49186836 = -787161791;    int NddXAXVePkEVLuH91804347 = 68853035;    int NddXAXVePkEVLuH25786505 = -265753977;    int NddXAXVePkEVLuH39997283 = -173567458;    int NddXAXVePkEVLuH77326327 = 24072674;    int NddXAXVePkEVLuH58258654 = -936706017;    int NddXAXVePkEVLuH87306118 = -400936988;    int NddXAXVePkEVLuH75691391 = -885796159;    int NddXAXVePkEVLuH86289966 = -188175430;    int NddXAXVePkEVLuH79860618 = -43531701;    int NddXAXVePkEVLuH26929604 = -609316263;    int NddXAXVePkEVLuH98596457 = -472353946;    int NddXAXVePkEVLuH67947329 = -144230145;    int NddXAXVePkEVLuH71361919 = -125084659;    int NddXAXVePkEVLuH10649940 = -728723894;    int NddXAXVePkEVLuH1351408 = -524087990;    int NddXAXVePkEVLuH75348989 = -456511637;    int NddXAXVePkEVLuH81457140 = 18007767;    int NddXAXVePkEVLuH31758631 = -129351673;    int NddXAXVePkEVLuH28091539 = -438158447;    int NddXAXVePkEVLuH22242161 = -791740996;    int NddXAXVePkEVLuH21892846 = -340134049;    int NddXAXVePkEVLuH11964478 = -456563919;    int NddXAXVePkEVLuH11471960 = -298590463;    int NddXAXVePkEVLuH55729432 = -213667563;    int NddXAXVePkEVLuH64266987 = -660781330;    int NddXAXVePkEVLuH67829890 = -790136625;    int NddXAXVePkEVLuH7087868 = -763696384;    int NddXAXVePkEVLuH42590104 = -586323903;    int NddXAXVePkEVLuH33986966 = 45980532;    int NddXAXVePkEVLuH39092180 = -374601579;    int NddXAXVePkEVLuH53923763 = -846387977;    int NddXAXVePkEVLuH66946045 = 18028570;    int NddXAXVePkEVLuH89246675 = -619747029;    int NddXAXVePkEVLuH17445703 = -60807194;    int NddXAXVePkEVLuH9502522 = 3523596;    int NddXAXVePkEVLuH92413945 = -115441631;    int NddXAXVePkEVLuH20269176 = -387520147;    int NddXAXVePkEVLuH86517166 = -477966281;    int NddXAXVePkEVLuH42290239 = -258609300;    int NddXAXVePkEVLuH6610326 = -612451753;    int NddXAXVePkEVLuH49244523 = -519398604;    int NddXAXVePkEVLuH30115559 = 76464007;    int NddXAXVePkEVLuH80092432 = -534197188;    int NddXAXVePkEVLuH74954674 = -399014900;    int NddXAXVePkEVLuH50999340 = -101356885;    int NddXAXVePkEVLuH75869112 = -996676450;    int NddXAXVePkEVLuH20665855 = -378900765;    int NddXAXVePkEVLuH76277650 = -463240013;    int NddXAXVePkEVLuH2034978 = -905435792;    int NddXAXVePkEVLuH60833812 = -153175543;    int NddXAXVePkEVLuH49400248 = -740432172;    int NddXAXVePkEVLuH99850559 = 85289803;    int NddXAXVePkEVLuH95283447 = -29277618;    int NddXAXVePkEVLuH59949301 = -86821834;    int NddXAXVePkEVLuH27293991 = -347027742;    int NddXAXVePkEVLuH79839869 = -474583046;    int NddXAXVePkEVLuH14314546 = -967163515;    int NddXAXVePkEVLuH84267850 = -959899895;    int NddXAXVePkEVLuH13059340 = -315145997;    int NddXAXVePkEVLuH90428764 = -46569393;    int NddXAXVePkEVLuH80218251 = -637240604;    int NddXAXVePkEVLuH33101288 = -199472256;    int NddXAXVePkEVLuH52303001 = -134155962;    int NddXAXVePkEVLuH40768438 = -668930122;    int NddXAXVePkEVLuH73005840 = -762928287;    int NddXAXVePkEVLuH31650412 = -390382517;    int NddXAXVePkEVLuH78700654 = -524483116;    int NddXAXVePkEVLuH53916216 = 35722535;    int NddXAXVePkEVLuH1147419 = -632247490;    int NddXAXVePkEVLuH8937462 = -308646360;    int NddXAXVePkEVLuH55079814 = 31008509;    int NddXAXVePkEVLuH94939973 = -504025953;    int NddXAXVePkEVLuH89468392 = -870742374;    int NddXAXVePkEVLuH21481214 = -825706694;    int NddXAXVePkEVLuH72997638 = -172342393;    int NddXAXVePkEVLuH91777286 = -316598057;    int NddXAXVePkEVLuH31872046 = -922366732;    int NddXAXVePkEVLuH36517286 = -899575563;    int NddXAXVePkEVLuH4730093 = -12310679;    int NddXAXVePkEVLuH88397875 = -664104880;    int NddXAXVePkEVLuH47164036 = -311235861;    int NddXAXVePkEVLuH30810218 = -200456372;    int NddXAXVePkEVLuH40555127 = -680888111;    int NddXAXVePkEVLuH73153153 = -800843925;    int NddXAXVePkEVLuH89691932 = -634169407;    int NddXAXVePkEVLuH54073203 = -831677781;    int NddXAXVePkEVLuH71662598 = -952693812;    int NddXAXVePkEVLuH29297374 = -432925195;    int NddXAXVePkEVLuH90151712 = -713779452;    int NddXAXVePkEVLuH29662652 = -521893359;    int NddXAXVePkEVLuH78099400 = -148278116;    int NddXAXVePkEVLuH36001325 = -427620253;    int NddXAXVePkEVLuH73457826 = -429523782;     NddXAXVePkEVLuH3386385 = NddXAXVePkEVLuH36182802;     NddXAXVePkEVLuH36182802 = NddXAXVePkEVLuH30857388;     NddXAXVePkEVLuH30857388 = NddXAXVePkEVLuH31609191;     NddXAXVePkEVLuH31609191 = NddXAXVePkEVLuH23374987;     NddXAXVePkEVLuH23374987 = NddXAXVePkEVLuH82191462;     NddXAXVePkEVLuH82191462 = NddXAXVePkEVLuH49186836;     NddXAXVePkEVLuH49186836 = NddXAXVePkEVLuH91804347;     NddXAXVePkEVLuH91804347 = NddXAXVePkEVLuH25786505;     NddXAXVePkEVLuH25786505 = NddXAXVePkEVLuH39997283;     NddXAXVePkEVLuH39997283 = NddXAXVePkEVLuH77326327;     NddXAXVePkEVLuH77326327 = NddXAXVePkEVLuH58258654;     NddXAXVePkEVLuH58258654 = NddXAXVePkEVLuH87306118;     NddXAXVePkEVLuH87306118 = NddXAXVePkEVLuH75691391;     NddXAXVePkEVLuH75691391 = NddXAXVePkEVLuH86289966;     NddXAXVePkEVLuH86289966 = NddXAXVePkEVLuH79860618;     NddXAXVePkEVLuH79860618 = NddXAXVePkEVLuH26929604;     NddXAXVePkEVLuH26929604 = NddXAXVePkEVLuH98596457;     NddXAXVePkEVLuH98596457 = NddXAXVePkEVLuH67947329;     NddXAXVePkEVLuH67947329 = NddXAXVePkEVLuH71361919;     NddXAXVePkEVLuH71361919 = NddXAXVePkEVLuH10649940;     NddXAXVePkEVLuH10649940 = NddXAXVePkEVLuH1351408;     NddXAXVePkEVLuH1351408 = NddXAXVePkEVLuH75348989;     NddXAXVePkEVLuH75348989 = NddXAXVePkEVLuH81457140;     NddXAXVePkEVLuH81457140 = NddXAXVePkEVLuH31758631;     NddXAXVePkEVLuH31758631 = NddXAXVePkEVLuH28091539;     NddXAXVePkEVLuH28091539 = NddXAXVePkEVLuH22242161;     NddXAXVePkEVLuH22242161 = NddXAXVePkEVLuH21892846;     NddXAXVePkEVLuH21892846 = NddXAXVePkEVLuH11964478;     NddXAXVePkEVLuH11964478 = NddXAXVePkEVLuH11471960;     NddXAXVePkEVLuH11471960 = NddXAXVePkEVLuH55729432;     NddXAXVePkEVLuH55729432 = NddXAXVePkEVLuH64266987;     NddXAXVePkEVLuH64266987 = NddXAXVePkEVLuH67829890;     NddXAXVePkEVLuH67829890 = NddXAXVePkEVLuH7087868;     NddXAXVePkEVLuH7087868 = NddXAXVePkEVLuH42590104;     NddXAXVePkEVLuH42590104 = NddXAXVePkEVLuH33986966;     NddXAXVePkEVLuH33986966 = NddXAXVePkEVLuH39092180;     NddXAXVePkEVLuH39092180 = NddXAXVePkEVLuH53923763;     NddXAXVePkEVLuH53923763 = NddXAXVePkEVLuH66946045;     NddXAXVePkEVLuH66946045 = NddXAXVePkEVLuH89246675;     NddXAXVePkEVLuH89246675 = NddXAXVePkEVLuH17445703;     NddXAXVePkEVLuH17445703 = NddXAXVePkEVLuH9502522;     NddXAXVePkEVLuH9502522 = NddXAXVePkEVLuH92413945;     NddXAXVePkEVLuH92413945 = NddXAXVePkEVLuH20269176;     NddXAXVePkEVLuH20269176 = NddXAXVePkEVLuH86517166;     NddXAXVePkEVLuH86517166 = NddXAXVePkEVLuH42290239;     NddXAXVePkEVLuH42290239 = NddXAXVePkEVLuH6610326;     NddXAXVePkEVLuH6610326 = NddXAXVePkEVLuH49244523;     NddXAXVePkEVLuH49244523 = NddXAXVePkEVLuH30115559;     NddXAXVePkEVLuH30115559 = NddXAXVePkEVLuH80092432;     NddXAXVePkEVLuH80092432 = NddXAXVePkEVLuH74954674;     NddXAXVePkEVLuH74954674 = NddXAXVePkEVLuH50999340;     NddXAXVePkEVLuH50999340 = NddXAXVePkEVLuH75869112;     NddXAXVePkEVLuH75869112 = NddXAXVePkEVLuH20665855;     NddXAXVePkEVLuH20665855 = NddXAXVePkEVLuH76277650;     NddXAXVePkEVLuH76277650 = NddXAXVePkEVLuH2034978;     NddXAXVePkEVLuH2034978 = NddXAXVePkEVLuH60833812;     NddXAXVePkEVLuH60833812 = NddXAXVePkEVLuH49400248;     NddXAXVePkEVLuH49400248 = NddXAXVePkEVLuH99850559;     NddXAXVePkEVLuH99850559 = NddXAXVePkEVLuH95283447;     NddXAXVePkEVLuH95283447 = NddXAXVePkEVLuH59949301;     NddXAXVePkEVLuH59949301 = NddXAXVePkEVLuH27293991;     NddXAXVePkEVLuH27293991 = NddXAXVePkEVLuH79839869;     NddXAXVePkEVLuH79839869 = NddXAXVePkEVLuH14314546;     NddXAXVePkEVLuH14314546 = NddXAXVePkEVLuH84267850;     NddXAXVePkEVLuH84267850 = NddXAXVePkEVLuH13059340;     NddXAXVePkEVLuH13059340 = NddXAXVePkEVLuH90428764;     NddXAXVePkEVLuH90428764 = NddXAXVePkEVLuH80218251;     NddXAXVePkEVLuH80218251 = NddXAXVePkEVLuH33101288;     NddXAXVePkEVLuH33101288 = NddXAXVePkEVLuH52303001;     NddXAXVePkEVLuH52303001 = NddXAXVePkEVLuH40768438;     NddXAXVePkEVLuH40768438 = NddXAXVePkEVLuH73005840;     NddXAXVePkEVLuH73005840 = NddXAXVePkEVLuH31650412;     NddXAXVePkEVLuH31650412 = NddXAXVePkEVLuH78700654;     NddXAXVePkEVLuH78700654 = NddXAXVePkEVLuH53916216;     NddXAXVePkEVLuH53916216 = NddXAXVePkEVLuH1147419;     NddXAXVePkEVLuH1147419 = NddXAXVePkEVLuH8937462;     NddXAXVePkEVLuH8937462 = NddXAXVePkEVLuH55079814;     NddXAXVePkEVLuH55079814 = NddXAXVePkEVLuH94939973;     NddXAXVePkEVLuH94939973 = NddXAXVePkEVLuH89468392;     NddXAXVePkEVLuH89468392 = NddXAXVePkEVLuH21481214;     NddXAXVePkEVLuH21481214 = NddXAXVePkEVLuH72997638;     NddXAXVePkEVLuH72997638 = NddXAXVePkEVLuH91777286;     NddXAXVePkEVLuH91777286 = NddXAXVePkEVLuH31872046;     NddXAXVePkEVLuH31872046 = NddXAXVePkEVLuH36517286;     NddXAXVePkEVLuH36517286 = NddXAXVePkEVLuH4730093;     NddXAXVePkEVLuH4730093 = NddXAXVePkEVLuH88397875;     NddXAXVePkEVLuH88397875 = NddXAXVePkEVLuH47164036;     NddXAXVePkEVLuH47164036 = NddXAXVePkEVLuH30810218;     NddXAXVePkEVLuH30810218 = NddXAXVePkEVLuH40555127;     NddXAXVePkEVLuH40555127 = NddXAXVePkEVLuH73153153;     NddXAXVePkEVLuH73153153 = NddXAXVePkEVLuH89691932;     NddXAXVePkEVLuH89691932 = NddXAXVePkEVLuH54073203;     NddXAXVePkEVLuH54073203 = NddXAXVePkEVLuH71662598;     NddXAXVePkEVLuH71662598 = NddXAXVePkEVLuH29297374;     NddXAXVePkEVLuH29297374 = NddXAXVePkEVLuH90151712;     NddXAXVePkEVLuH90151712 = NddXAXVePkEVLuH29662652;     NddXAXVePkEVLuH29662652 = NddXAXVePkEVLuH78099400;     NddXAXVePkEVLuH78099400 = NddXAXVePkEVLuH36001325;     NddXAXVePkEVLuH36001325 = NddXAXVePkEVLuH73457826;     NddXAXVePkEVLuH73457826 = NddXAXVePkEVLuH3386385;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void jmtWQyDIMSlIqEqTnzsxKbrySGOBy89116928() {     int WvQAhjdDiDYpdyc99899305 = -428324470;    int WvQAhjdDiDYpdyc51265627 = -662375483;    int WvQAhjdDiDYpdyc86539698 = -570527171;    int WvQAhjdDiDYpdyc44158039 = -612875659;    int WvQAhjdDiDYpdyc92027376 = -123023060;    int WvQAhjdDiDYpdyc19438335 = 90909594;    int WvQAhjdDiDYpdyc23001489 = -952079766;    int WvQAhjdDiDYpdyc27256518 = -44031149;    int WvQAhjdDiDYpdyc30040762 = -518171731;    int WvQAhjdDiDYpdyc94001819 = -932292671;    int WvQAhjdDiDYpdyc37542443 = 48837437;    int WvQAhjdDiDYpdyc73275091 = -405438923;    int WvQAhjdDiDYpdyc92576816 = -686722651;    int WvQAhjdDiDYpdyc42766505 = -295317694;    int WvQAhjdDiDYpdyc91189723 = -39502140;    int WvQAhjdDiDYpdyc92817441 = -902152916;    int WvQAhjdDiDYpdyc54620452 = -522163308;    int WvQAhjdDiDYpdyc34731946 = 10119989;    int WvQAhjdDiDYpdyc49870306 = -633357445;    int WvQAhjdDiDYpdyc12087178 = -34609489;    int WvQAhjdDiDYpdyc18328795 = -939273492;    int WvQAhjdDiDYpdyc42253165 = -902105682;    int WvQAhjdDiDYpdyc64845952 = -737431348;    int WvQAhjdDiDYpdyc5281911 = 58890362;    int WvQAhjdDiDYpdyc10033638 = -754777432;    int WvQAhjdDiDYpdyc4115904 = -617453709;    int WvQAhjdDiDYpdyc70138846 = -618922843;    int WvQAhjdDiDYpdyc14143208 = 4109852;    int WvQAhjdDiDYpdyc94160449 = -321941902;    int WvQAhjdDiDYpdyc11726468 = -763367305;    int WvQAhjdDiDYpdyc64823512 = -515635716;    int WvQAhjdDiDYpdyc17171 = -992787350;    int WvQAhjdDiDYpdyc85274329 = -680233393;    int WvQAhjdDiDYpdyc19747537 = -584206384;    int WvQAhjdDiDYpdyc52074943 = -800989429;    int WvQAhjdDiDYpdyc77658219 = -231096176;    int WvQAhjdDiDYpdyc19649147 = -985025537;    int WvQAhjdDiDYpdyc27050738 = -667519945;    int WvQAhjdDiDYpdyc15680353 = -323040180;    int WvQAhjdDiDYpdyc68764074 = -56094911;    int WvQAhjdDiDYpdyc49716698 = -592349365;    int WvQAhjdDiDYpdyc53695873 = -503917367;    int WvQAhjdDiDYpdyc79955882 = -611714203;    int WvQAhjdDiDYpdyc32477284 = -529153901;    int WvQAhjdDiDYpdyc52503881 = 89879381;    int WvQAhjdDiDYpdyc39971265 = -590937095;    int WvQAhjdDiDYpdyc3522419 = -890972402;    int WvQAhjdDiDYpdyc29734547 = -682492603;    int WvQAhjdDiDYpdyc48724627 = -555040188;    int WvQAhjdDiDYpdyc52893471 = -883736643;    int WvQAhjdDiDYpdyc26743314 = 71605893;    int WvQAhjdDiDYpdyc23407978 = -683522135;    int WvQAhjdDiDYpdyc68012490 = -688585095;    int WvQAhjdDiDYpdyc6173051 = -829101769;    int WvQAhjdDiDYpdyc70788133 = -895961806;    int WvQAhjdDiDYpdyc57646141 = -526218788;    int WvQAhjdDiDYpdyc86419675 = -924944135;    int WvQAhjdDiDYpdyc81257788 = -529417534;    int WvQAhjdDiDYpdyc34124401 = -858098228;    int WvQAhjdDiDYpdyc87911473 = -505569351;    int WvQAhjdDiDYpdyc49299489 = -290167564;    int WvQAhjdDiDYpdyc8858281 = -856189618;    int WvQAhjdDiDYpdyc33096068 = -722089248;    int WvQAhjdDiDYpdyc18314295 = -754804426;    int WvQAhjdDiDYpdyc29178308 = -316656955;    int WvQAhjdDiDYpdyc37525272 = 41624786;    int WvQAhjdDiDYpdyc88000762 = -725205531;    int WvQAhjdDiDYpdyc72829280 = -2516267;    int WvQAhjdDiDYpdyc90691562 = -494328265;    int WvQAhjdDiDYpdyc13531505 = -808405964;    int WvQAhjdDiDYpdyc73168294 = -917127379;    int WvQAhjdDiDYpdyc27569715 = -854643363;    int WvQAhjdDiDYpdyc19051593 = -666839831;    int WvQAhjdDiDYpdyc81106231 = -477262535;    int WvQAhjdDiDYpdyc62370480 = -442260125;    int WvQAhjdDiDYpdyc64632922 = -335356126;    int WvQAhjdDiDYpdyc62297283 = -190391480;    int WvQAhjdDiDYpdyc32368668 = -108277447;    int WvQAhjdDiDYpdyc52778029 = 69010981;    int WvQAhjdDiDYpdyc70062373 = -63840337;    int WvQAhjdDiDYpdyc593486 = -726481308;    int WvQAhjdDiDYpdyc40404299 = -936430240;    int WvQAhjdDiDYpdyc65418581 = -440849961;    int WvQAhjdDiDYpdyc41266978 = -438205260;    int WvQAhjdDiDYpdyc84983153 = -734973199;    int WvQAhjdDiDYpdyc41415534 = -832113582;    int WvQAhjdDiDYpdyc32004681 = -204202256;    int WvQAhjdDiDYpdyc79101279 = -851131625;    int WvQAhjdDiDYpdyc48959403 = -688244579;    int WvQAhjdDiDYpdyc94428801 = -174770642;    int WvQAhjdDiDYpdyc91238543 = -306152042;    int WvQAhjdDiDYpdyc38391359 = -355608004;    int WvQAhjdDiDYpdyc92926336 = -809421718;    int WvQAhjdDiDYpdyc27768880 = -817470829;    int WvQAhjdDiDYpdyc19464586 = -765927348;    int WvQAhjdDiDYpdyc40858417 = -736159747;    int WvQAhjdDiDYpdyc20599805 = -781828120;    int WvQAhjdDiDYpdyc61641587 = -856909777;    int WvQAhjdDiDYpdyc3298977 = -112496947;    int WvQAhjdDiDYpdyc14978610 = -428324470;     WvQAhjdDiDYpdyc99899305 = WvQAhjdDiDYpdyc51265627;     WvQAhjdDiDYpdyc51265627 = WvQAhjdDiDYpdyc86539698;     WvQAhjdDiDYpdyc86539698 = WvQAhjdDiDYpdyc44158039;     WvQAhjdDiDYpdyc44158039 = WvQAhjdDiDYpdyc92027376;     WvQAhjdDiDYpdyc92027376 = WvQAhjdDiDYpdyc19438335;     WvQAhjdDiDYpdyc19438335 = WvQAhjdDiDYpdyc23001489;     WvQAhjdDiDYpdyc23001489 = WvQAhjdDiDYpdyc27256518;     WvQAhjdDiDYpdyc27256518 = WvQAhjdDiDYpdyc30040762;     WvQAhjdDiDYpdyc30040762 = WvQAhjdDiDYpdyc94001819;     WvQAhjdDiDYpdyc94001819 = WvQAhjdDiDYpdyc37542443;     WvQAhjdDiDYpdyc37542443 = WvQAhjdDiDYpdyc73275091;     WvQAhjdDiDYpdyc73275091 = WvQAhjdDiDYpdyc92576816;     WvQAhjdDiDYpdyc92576816 = WvQAhjdDiDYpdyc42766505;     WvQAhjdDiDYpdyc42766505 = WvQAhjdDiDYpdyc91189723;     WvQAhjdDiDYpdyc91189723 = WvQAhjdDiDYpdyc92817441;     WvQAhjdDiDYpdyc92817441 = WvQAhjdDiDYpdyc54620452;     WvQAhjdDiDYpdyc54620452 = WvQAhjdDiDYpdyc34731946;     WvQAhjdDiDYpdyc34731946 = WvQAhjdDiDYpdyc49870306;     WvQAhjdDiDYpdyc49870306 = WvQAhjdDiDYpdyc12087178;     WvQAhjdDiDYpdyc12087178 = WvQAhjdDiDYpdyc18328795;     WvQAhjdDiDYpdyc18328795 = WvQAhjdDiDYpdyc42253165;     WvQAhjdDiDYpdyc42253165 = WvQAhjdDiDYpdyc64845952;     WvQAhjdDiDYpdyc64845952 = WvQAhjdDiDYpdyc5281911;     WvQAhjdDiDYpdyc5281911 = WvQAhjdDiDYpdyc10033638;     WvQAhjdDiDYpdyc10033638 = WvQAhjdDiDYpdyc4115904;     WvQAhjdDiDYpdyc4115904 = WvQAhjdDiDYpdyc70138846;     WvQAhjdDiDYpdyc70138846 = WvQAhjdDiDYpdyc14143208;     WvQAhjdDiDYpdyc14143208 = WvQAhjdDiDYpdyc94160449;     WvQAhjdDiDYpdyc94160449 = WvQAhjdDiDYpdyc11726468;     WvQAhjdDiDYpdyc11726468 = WvQAhjdDiDYpdyc64823512;     WvQAhjdDiDYpdyc64823512 = WvQAhjdDiDYpdyc17171;     WvQAhjdDiDYpdyc17171 = WvQAhjdDiDYpdyc85274329;     WvQAhjdDiDYpdyc85274329 = WvQAhjdDiDYpdyc19747537;     WvQAhjdDiDYpdyc19747537 = WvQAhjdDiDYpdyc52074943;     WvQAhjdDiDYpdyc52074943 = WvQAhjdDiDYpdyc77658219;     WvQAhjdDiDYpdyc77658219 = WvQAhjdDiDYpdyc19649147;     WvQAhjdDiDYpdyc19649147 = WvQAhjdDiDYpdyc27050738;     WvQAhjdDiDYpdyc27050738 = WvQAhjdDiDYpdyc15680353;     WvQAhjdDiDYpdyc15680353 = WvQAhjdDiDYpdyc68764074;     WvQAhjdDiDYpdyc68764074 = WvQAhjdDiDYpdyc49716698;     WvQAhjdDiDYpdyc49716698 = WvQAhjdDiDYpdyc53695873;     WvQAhjdDiDYpdyc53695873 = WvQAhjdDiDYpdyc79955882;     WvQAhjdDiDYpdyc79955882 = WvQAhjdDiDYpdyc32477284;     WvQAhjdDiDYpdyc32477284 = WvQAhjdDiDYpdyc52503881;     WvQAhjdDiDYpdyc52503881 = WvQAhjdDiDYpdyc39971265;     WvQAhjdDiDYpdyc39971265 = WvQAhjdDiDYpdyc3522419;     WvQAhjdDiDYpdyc3522419 = WvQAhjdDiDYpdyc29734547;     WvQAhjdDiDYpdyc29734547 = WvQAhjdDiDYpdyc48724627;     WvQAhjdDiDYpdyc48724627 = WvQAhjdDiDYpdyc52893471;     WvQAhjdDiDYpdyc52893471 = WvQAhjdDiDYpdyc26743314;     WvQAhjdDiDYpdyc26743314 = WvQAhjdDiDYpdyc23407978;     WvQAhjdDiDYpdyc23407978 = WvQAhjdDiDYpdyc68012490;     WvQAhjdDiDYpdyc68012490 = WvQAhjdDiDYpdyc6173051;     WvQAhjdDiDYpdyc6173051 = WvQAhjdDiDYpdyc70788133;     WvQAhjdDiDYpdyc70788133 = WvQAhjdDiDYpdyc57646141;     WvQAhjdDiDYpdyc57646141 = WvQAhjdDiDYpdyc86419675;     WvQAhjdDiDYpdyc86419675 = WvQAhjdDiDYpdyc81257788;     WvQAhjdDiDYpdyc81257788 = WvQAhjdDiDYpdyc34124401;     WvQAhjdDiDYpdyc34124401 = WvQAhjdDiDYpdyc87911473;     WvQAhjdDiDYpdyc87911473 = WvQAhjdDiDYpdyc49299489;     WvQAhjdDiDYpdyc49299489 = WvQAhjdDiDYpdyc8858281;     WvQAhjdDiDYpdyc8858281 = WvQAhjdDiDYpdyc33096068;     WvQAhjdDiDYpdyc33096068 = WvQAhjdDiDYpdyc18314295;     WvQAhjdDiDYpdyc18314295 = WvQAhjdDiDYpdyc29178308;     WvQAhjdDiDYpdyc29178308 = WvQAhjdDiDYpdyc37525272;     WvQAhjdDiDYpdyc37525272 = WvQAhjdDiDYpdyc88000762;     WvQAhjdDiDYpdyc88000762 = WvQAhjdDiDYpdyc72829280;     WvQAhjdDiDYpdyc72829280 = WvQAhjdDiDYpdyc90691562;     WvQAhjdDiDYpdyc90691562 = WvQAhjdDiDYpdyc13531505;     WvQAhjdDiDYpdyc13531505 = WvQAhjdDiDYpdyc73168294;     WvQAhjdDiDYpdyc73168294 = WvQAhjdDiDYpdyc27569715;     WvQAhjdDiDYpdyc27569715 = WvQAhjdDiDYpdyc19051593;     WvQAhjdDiDYpdyc19051593 = WvQAhjdDiDYpdyc81106231;     WvQAhjdDiDYpdyc81106231 = WvQAhjdDiDYpdyc62370480;     WvQAhjdDiDYpdyc62370480 = WvQAhjdDiDYpdyc64632922;     WvQAhjdDiDYpdyc64632922 = WvQAhjdDiDYpdyc62297283;     WvQAhjdDiDYpdyc62297283 = WvQAhjdDiDYpdyc32368668;     WvQAhjdDiDYpdyc32368668 = WvQAhjdDiDYpdyc52778029;     WvQAhjdDiDYpdyc52778029 = WvQAhjdDiDYpdyc70062373;     WvQAhjdDiDYpdyc70062373 = WvQAhjdDiDYpdyc593486;     WvQAhjdDiDYpdyc593486 = WvQAhjdDiDYpdyc40404299;     WvQAhjdDiDYpdyc40404299 = WvQAhjdDiDYpdyc65418581;     WvQAhjdDiDYpdyc65418581 = WvQAhjdDiDYpdyc41266978;     WvQAhjdDiDYpdyc41266978 = WvQAhjdDiDYpdyc84983153;     WvQAhjdDiDYpdyc84983153 = WvQAhjdDiDYpdyc41415534;     WvQAhjdDiDYpdyc41415534 = WvQAhjdDiDYpdyc32004681;     WvQAhjdDiDYpdyc32004681 = WvQAhjdDiDYpdyc79101279;     WvQAhjdDiDYpdyc79101279 = WvQAhjdDiDYpdyc48959403;     WvQAhjdDiDYpdyc48959403 = WvQAhjdDiDYpdyc94428801;     WvQAhjdDiDYpdyc94428801 = WvQAhjdDiDYpdyc91238543;     WvQAhjdDiDYpdyc91238543 = WvQAhjdDiDYpdyc38391359;     WvQAhjdDiDYpdyc38391359 = WvQAhjdDiDYpdyc92926336;     WvQAhjdDiDYpdyc92926336 = WvQAhjdDiDYpdyc27768880;     WvQAhjdDiDYpdyc27768880 = WvQAhjdDiDYpdyc19464586;     WvQAhjdDiDYpdyc19464586 = WvQAhjdDiDYpdyc40858417;     WvQAhjdDiDYpdyc40858417 = WvQAhjdDiDYpdyc20599805;     WvQAhjdDiDYpdyc20599805 = WvQAhjdDiDYpdyc61641587;     WvQAhjdDiDYpdyc61641587 = WvQAhjdDiDYpdyc3298977;     WvQAhjdDiDYpdyc3298977 = WvQAhjdDiDYpdyc14978610;     WvQAhjdDiDYpdyc14978610 = WvQAhjdDiDYpdyc99899305;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xPqhpNZkNDVEPCFkTCCRYMWUSRrZU39063736() {     int nFKNDAWzFHzUblm67070365 = -72750147;    int nFKNDAWzFHzUblm96449653 = -444111959;    int nFKNDAWzFHzUblm20877717 = -803795782;    int nFKNDAWzFHzUblm11558314 = -351287009;    int nFKNDAWzFHzUblm73289986 = -93661082;    int nFKNDAWzFHzUblm11879538 = 58233678;    int nFKNDAWzFHzUblm46647702 = -311236984;    int nFKNDAWzFHzUblm50337018 = -799555106;    int nFKNDAWzFHzUblm97419908 = -50290743;    int nFKNDAWzFHzUblm74333302 = -155306363;    int nFKNDAWzFHzUblm19687114 = -1827658;    int nFKNDAWzFHzUblm19546423 = -454457259;    int nFKNDAWzFHzUblm45821987 = -860386898;    int nFKNDAWzFHzUblm39844431 = -260714387;    int nFKNDAWzFHzUblm33637391 = -77866001;    int nFKNDAWzFHzUblm63738358 = -997962067;    int nFKNDAWzFHzUblm20311369 = -524580912;    int nFKNDAWzFHzUblm68160395 = -199500650;    int nFKNDAWzFHzUblm93072793 = -111690421;    int nFKNDAWzFHzUblm39525384 = -974476173;    int nFKNDAWzFHzUblm1473981 = -248035857;    int nFKNDAWzFHzUblm66254421 = -1416797;    int nFKNDAWzFHzUblm19408327 = -27236361;    int nFKNDAWzFHzUblm19306110 = -711707425;    int nFKNDAWzFHzUblm23990345 = -629414026;    int nFKNDAWzFHzUblm94163772 = -986827489;    int nFKNDAWzFHzUblm56074735 = -540482752;    int nFKNDAWzFHzUblm78772271 = -647803027;    int nFKNDAWzFHzUblm95820461 = -935607787;    int nFKNDAWzFHzUblm75063200 = 88892156;    int nFKNDAWzFHzUblm47576909 = -178944321;    int nFKNDAWzFHzUblm95554957 = -161268772;    int nFKNDAWzFHzUblm211706 = -315347554;    int nFKNDAWzFHzUblm97489342 = -91564336;    int nFKNDAWzFHzUblm57225979 = -603287809;    int nFKNDAWzFHzUblm38494542 = -507633686;    int nFKNDAWzFHzUblm32056232 = -584389324;    int nFKNDAWzFHzUblm83052521 = -277502343;    int nFKNDAWzFHzUblm7634352 = 92843849;    int nFKNDAWzFHzUblm71511879 = -500619470;    int nFKNDAWzFHzUblm54205309 = -319966515;    int nFKNDAWzFHzUblm40674556 = -586955317;    int nFKNDAWzFHzUblm56181418 = -535610573;    int nFKNDAWzFHzUblm91961794 = -85798183;    int nFKNDAWzFHzUblm30426603 = -277661950;    int nFKNDAWzFHzUblm3630159 = -869557376;    int nFKNDAWzFHzUblm98867921 = -804282227;    int nFKNDAWzFHzUblm455199 = -688955542;    int nFKNDAWzFHzUblm54116757 = -508544491;    int nFKNDAWzFHzUblm14076574 = -829260090;    int nFKNDAWzFHzUblm33461909 = -525790427;    int nFKNDAWzFHzUblm23140864 = -852761359;    int nFKNDAWzFHzUblm39783178 = -416237773;    int nFKNDAWzFHzUblm71657279 = -308264418;    int nFKNDAWzFHzUblm37589756 = -582725235;    int nFKNDAWzFHzUblm815944 = 28666650;    int nFKNDAWzFHzUblm77041327 = -316875598;    int nFKNDAWzFHzUblm1571607 = 7911642;    int nFKNDAWzFHzUblm87567969 = -721872984;    int nFKNDAWzFHzUblm79126214 = -106833594;    int nFKNDAWzFHzUblm55804802 = -401283570;    int nFKNDAWzFHzUblm67875430 = -663433958;    int nFKNDAWzFHzUblm54516556 = -863947319;    int nFKNDAWzFHzUblm22356708 = -39182900;    int nFKNDAWzFHzUblm26756394 = -976362043;    int nFKNDAWzFHzUblm24132157 = -840558887;    int nFKNDAWzFHzUblm19334717 = -39109706;    int nFKNDAWzFHzUblm48332645 = -668822563;    int nFKNDAWzFHzUblm82618451 = -657426579;    int nFKNDAWzFHzUblm95142849 = -570232315;    int nFKNDAWzFHzUblm31682126 = -313572744;    int nFKNDAWzFHzUblm37258847 = -147078570;    int nFKNDAWzFHzUblm60526044 = -192344500;    int nFKNDAWzFHzUblm21560915 = -611070952;    int nFKNDAWzFHzUblm85320074 = -554509659;    int nFKNDAWzFHzUblm60799425 = -661080541;    int nFKNDAWzFHzUblm10073004 = -465806225;    int nFKNDAWzFHzUblm27446533 = -941438179;    int nFKNDAWzFHzUblm88879506 = -334045476;    int nFKNDAWzFHzUblm20360186 = -759856651;    int nFKNDAWzFHzUblm95295850 = -82545262;    int nFKNDAWzFHzUblm55619536 = -851527211;    int nFKNDAWzFHzUblm24655514 = -39258536;    int nFKNDAWzFHzUblm81743888 = -6347697;    int nFKNDAWzFHzUblm41601292 = -385317418;    int nFKNDAWzFHzUblm24436046 = -326182963;    int nFKNDAWzFHzUblm55771780 = -745030999;    int nFKNDAWzFHzUblm28554426 = 92916864;    int nFKNDAWzFHzUblm59899587 = -508839102;    int nFKNDAWzFHzUblm56410036 = -531954460;    int nFKNDAWzFHzUblm61453215 = -90758089;    int nFKNDAWzFHzUblm30484625 = -492300967;    int nFKNDAWzFHzUblm95484552 = -555629360;    int nFKNDAWzFHzUblm28508137 = -800322558;    int nFKNDAWzFHzUblm15707077 = 664100;    int nFKNDAWzFHzUblm86329878 = -656532558;    int nFKNDAWzFHzUblm86157999 = -723007998;    int nFKNDAWzFHzUblm33824710 = -396427674;    int nFKNDAWzFHzUblm65205401 = -109436141;    int nFKNDAWzFHzUblm6294447 = -72750147;     nFKNDAWzFHzUblm67070365 = nFKNDAWzFHzUblm96449653;     nFKNDAWzFHzUblm96449653 = nFKNDAWzFHzUblm20877717;     nFKNDAWzFHzUblm20877717 = nFKNDAWzFHzUblm11558314;     nFKNDAWzFHzUblm11558314 = nFKNDAWzFHzUblm73289986;     nFKNDAWzFHzUblm73289986 = nFKNDAWzFHzUblm11879538;     nFKNDAWzFHzUblm11879538 = nFKNDAWzFHzUblm46647702;     nFKNDAWzFHzUblm46647702 = nFKNDAWzFHzUblm50337018;     nFKNDAWzFHzUblm50337018 = nFKNDAWzFHzUblm97419908;     nFKNDAWzFHzUblm97419908 = nFKNDAWzFHzUblm74333302;     nFKNDAWzFHzUblm74333302 = nFKNDAWzFHzUblm19687114;     nFKNDAWzFHzUblm19687114 = nFKNDAWzFHzUblm19546423;     nFKNDAWzFHzUblm19546423 = nFKNDAWzFHzUblm45821987;     nFKNDAWzFHzUblm45821987 = nFKNDAWzFHzUblm39844431;     nFKNDAWzFHzUblm39844431 = nFKNDAWzFHzUblm33637391;     nFKNDAWzFHzUblm33637391 = nFKNDAWzFHzUblm63738358;     nFKNDAWzFHzUblm63738358 = nFKNDAWzFHzUblm20311369;     nFKNDAWzFHzUblm20311369 = nFKNDAWzFHzUblm68160395;     nFKNDAWzFHzUblm68160395 = nFKNDAWzFHzUblm93072793;     nFKNDAWzFHzUblm93072793 = nFKNDAWzFHzUblm39525384;     nFKNDAWzFHzUblm39525384 = nFKNDAWzFHzUblm1473981;     nFKNDAWzFHzUblm1473981 = nFKNDAWzFHzUblm66254421;     nFKNDAWzFHzUblm66254421 = nFKNDAWzFHzUblm19408327;     nFKNDAWzFHzUblm19408327 = nFKNDAWzFHzUblm19306110;     nFKNDAWzFHzUblm19306110 = nFKNDAWzFHzUblm23990345;     nFKNDAWzFHzUblm23990345 = nFKNDAWzFHzUblm94163772;     nFKNDAWzFHzUblm94163772 = nFKNDAWzFHzUblm56074735;     nFKNDAWzFHzUblm56074735 = nFKNDAWzFHzUblm78772271;     nFKNDAWzFHzUblm78772271 = nFKNDAWzFHzUblm95820461;     nFKNDAWzFHzUblm95820461 = nFKNDAWzFHzUblm75063200;     nFKNDAWzFHzUblm75063200 = nFKNDAWzFHzUblm47576909;     nFKNDAWzFHzUblm47576909 = nFKNDAWzFHzUblm95554957;     nFKNDAWzFHzUblm95554957 = nFKNDAWzFHzUblm211706;     nFKNDAWzFHzUblm211706 = nFKNDAWzFHzUblm97489342;     nFKNDAWzFHzUblm97489342 = nFKNDAWzFHzUblm57225979;     nFKNDAWzFHzUblm57225979 = nFKNDAWzFHzUblm38494542;     nFKNDAWzFHzUblm38494542 = nFKNDAWzFHzUblm32056232;     nFKNDAWzFHzUblm32056232 = nFKNDAWzFHzUblm83052521;     nFKNDAWzFHzUblm83052521 = nFKNDAWzFHzUblm7634352;     nFKNDAWzFHzUblm7634352 = nFKNDAWzFHzUblm71511879;     nFKNDAWzFHzUblm71511879 = nFKNDAWzFHzUblm54205309;     nFKNDAWzFHzUblm54205309 = nFKNDAWzFHzUblm40674556;     nFKNDAWzFHzUblm40674556 = nFKNDAWzFHzUblm56181418;     nFKNDAWzFHzUblm56181418 = nFKNDAWzFHzUblm91961794;     nFKNDAWzFHzUblm91961794 = nFKNDAWzFHzUblm30426603;     nFKNDAWzFHzUblm30426603 = nFKNDAWzFHzUblm3630159;     nFKNDAWzFHzUblm3630159 = nFKNDAWzFHzUblm98867921;     nFKNDAWzFHzUblm98867921 = nFKNDAWzFHzUblm455199;     nFKNDAWzFHzUblm455199 = nFKNDAWzFHzUblm54116757;     nFKNDAWzFHzUblm54116757 = nFKNDAWzFHzUblm14076574;     nFKNDAWzFHzUblm14076574 = nFKNDAWzFHzUblm33461909;     nFKNDAWzFHzUblm33461909 = nFKNDAWzFHzUblm23140864;     nFKNDAWzFHzUblm23140864 = nFKNDAWzFHzUblm39783178;     nFKNDAWzFHzUblm39783178 = nFKNDAWzFHzUblm71657279;     nFKNDAWzFHzUblm71657279 = nFKNDAWzFHzUblm37589756;     nFKNDAWzFHzUblm37589756 = nFKNDAWzFHzUblm815944;     nFKNDAWzFHzUblm815944 = nFKNDAWzFHzUblm77041327;     nFKNDAWzFHzUblm77041327 = nFKNDAWzFHzUblm1571607;     nFKNDAWzFHzUblm1571607 = nFKNDAWzFHzUblm87567969;     nFKNDAWzFHzUblm87567969 = nFKNDAWzFHzUblm79126214;     nFKNDAWzFHzUblm79126214 = nFKNDAWzFHzUblm55804802;     nFKNDAWzFHzUblm55804802 = nFKNDAWzFHzUblm67875430;     nFKNDAWzFHzUblm67875430 = nFKNDAWzFHzUblm54516556;     nFKNDAWzFHzUblm54516556 = nFKNDAWzFHzUblm22356708;     nFKNDAWzFHzUblm22356708 = nFKNDAWzFHzUblm26756394;     nFKNDAWzFHzUblm26756394 = nFKNDAWzFHzUblm24132157;     nFKNDAWzFHzUblm24132157 = nFKNDAWzFHzUblm19334717;     nFKNDAWzFHzUblm19334717 = nFKNDAWzFHzUblm48332645;     nFKNDAWzFHzUblm48332645 = nFKNDAWzFHzUblm82618451;     nFKNDAWzFHzUblm82618451 = nFKNDAWzFHzUblm95142849;     nFKNDAWzFHzUblm95142849 = nFKNDAWzFHzUblm31682126;     nFKNDAWzFHzUblm31682126 = nFKNDAWzFHzUblm37258847;     nFKNDAWzFHzUblm37258847 = nFKNDAWzFHzUblm60526044;     nFKNDAWzFHzUblm60526044 = nFKNDAWzFHzUblm21560915;     nFKNDAWzFHzUblm21560915 = nFKNDAWzFHzUblm85320074;     nFKNDAWzFHzUblm85320074 = nFKNDAWzFHzUblm60799425;     nFKNDAWzFHzUblm60799425 = nFKNDAWzFHzUblm10073004;     nFKNDAWzFHzUblm10073004 = nFKNDAWzFHzUblm27446533;     nFKNDAWzFHzUblm27446533 = nFKNDAWzFHzUblm88879506;     nFKNDAWzFHzUblm88879506 = nFKNDAWzFHzUblm20360186;     nFKNDAWzFHzUblm20360186 = nFKNDAWzFHzUblm95295850;     nFKNDAWzFHzUblm95295850 = nFKNDAWzFHzUblm55619536;     nFKNDAWzFHzUblm55619536 = nFKNDAWzFHzUblm24655514;     nFKNDAWzFHzUblm24655514 = nFKNDAWzFHzUblm81743888;     nFKNDAWzFHzUblm81743888 = nFKNDAWzFHzUblm41601292;     nFKNDAWzFHzUblm41601292 = nFKNDAWzFHzUblm24436046;     nFKNDAWzFHzUblm24436046 = nFKNDAWzFHzUblm55771780;     nFKNDAWzFHzUblm55771780 = nFKNDAWzFHzUblm28554426;     nFKNDAWzFHzUblm28554426 = nFKNDAWzFHzUblm59899587;     nFKNDAWzFHzUblm59899587 = nFKNDAWzFHzUblm56410036;     nFKNDAWzFHzUblm56410036 = nFKNDAWzFHzUblm61453215;     nFKNDAWzFHzUblm61453215 = nFKNDAWzFHzUblm30484625;     nFKNDAWzFHzUblm30484625 = nFKNDAWzFHzUblm95484552;     nFKNDAWzFHzUblm95484552 = nFKNDAWzFHzUblm28508137;     nFKNDAWzFHzUblm28508137 = nFKNDAWzFHzUblm15707077;     nFKNDAWzFHzUblm15707077 = nFKNDAWzFHzUblm86329878;     nFKNDAWzFHzUblm86329878 = nFKNDAWzFHzUblm86157999;     nFKNDAWzFHzUblm86157999 = nFKNDAWzFHzUblm33824710;     nFKNDAWzFHzUblm33824710 = nFKNDAWzFHzUblm65205401;     nFKNDAWzFHzUblm65205401 = nFKNDAWzFHzUblm6294447;     nFKNDAWzFHzUblm6294447 = nFKNDAWzFHzUblm67070365;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uaShxtWDHqvMnYkLzJRLWMguEfLIq41253074() {     int fXgTzxiISIZYHiv63583285 = -71550835;    int fXgTzxiISIZYHiv11532479 = -396800261;    int fXgTzxiISIZYHiv76560027 = -551898548;    int fXgTzxiISIZYHiv24107162 = -820100800;    int fXgTzxiISIZYHiv41942376 = -749248077;    int fXgTzxiISIZYHiv49126409 = 27706101;    int fXgTzxiISIZYHiv20462355 = -476154959;    int fXgTzxiISIZYHiv85789187 = -912439290;    int fXgTzxiISIZYHiv1674166 = -302708497;    int fXgTzxiISIZYHiv28337839 = -914031576;    int fXgTzxiISIZYHiv79903229 = 22937105;    int fXgTzxiISIZYHiv34562860 = 76809835;    int fXgTzxiISIZYHiv51092685 = -46172562;    int fXgTzxiISIZYHiv6919545 = -770235922;    int fXgTzxiISIZYHiv38537149 = 70807289;    int fXgTzxiISIZYHiv76695182 = -756583283;    int fXgTzxiISIZYHiv48002217 = -437427957;    int fXgTzxiISIZYHiv4295884 = -817026715;    int fXgTzxiISIZYHiv74995769 = -600817721;    int fXgTzxiISIZYHiv80250642 = -884001004;    int fXgTzxiISIZYHiv9152836 = -458585455;    int fXgTzxiISIZYHiv7156179 = -379434490;    int fXgTzxiISIZYHiv8905290 = -308156071;    int fXgTzxiISIZYHiv43130880 = -670824830;    int fXgTzxiISIZYHiv2265352 = -154839785;    int fXgTzxiISIZYHiv70188136 = -66122751;    int fXgTzxiISIZYHiv3971420 = -367664599;    int fXgTzxiISIZYHiv71022633 = -303559125;    int fXgTzxiISIZYHiv78016432 = -800985770;    int fXgTzxiISIZYHiv75317708 = -375884686;    int fXgTzxiISIZYHiv56670989 = -480912474;    int fXgTzxiISIZYHiv31305141 = -493274792;    int fXgTzxiISIZYHiv17656145 = -205444322;    int fXgTzxiISIZYHiv10149011 = 87925665;    int fXgTzxiISIZYHiv66710818 = -817953335;    int fXgTzxiISIZYHiv82165795 = -784710395;    int fXgTzxiISIZYHiv12613199 = -94813282;    int fXgTzxiISIZYHiv56179495 = -98634311;    int fXgTzxiISIZYHiv56368658 = -248224901;    int fXgTzxiISIZYHiv51029278 = 63032648;    int fXgTzxiISIZYHiv86476304 = -851508686;    int fXgTzxiISIZYHiv84867907 = 5603721;    int fXgTzxiISIZYHiv43723354 = 68116855;    int fXgTzxiISIZYHiv4169903 = -227431937;    int fXgTzxiISIZYHiv96413317 = -809816287;    int fXgTzxiISIZYHiv1311186 = -101885171;    int fXgTzxiISIZYHiv95780014 = 17197125;    int fXgTzxiISIZYHiv80945222 = -852049541;    int fXgTzxiISIZYHiv72725825 = -40048686;    int fXgTzxiISIZYHiv86877612 = -78799545;    int fXgTzxiISIZYHiv85250549 = -55169633;    int fXgTzxiISIZYHiv95549501 = -334926609;    int fXgTzxiISIZYHiv31926556 = -108146417;    int fXgTzxiISIZYHiv57164475 = -758465422;    int fXgTzxiISIZYHiv32100239 = 84552973;    int fXgTzxiISIZYHiv56427107 = -692116345;    int fXgTzxiISIZYHiv2627190 = 11355810;    int fXgTzxiISIZYHiv33429148 = -881073719;    int fXgTzxiISIZYHiv21841810 = -565261015;    int fXgTzxiISIZYHiv71754239 = -583125327;    int fXgTzxiISIZYHiv45154990 = -604629300;    int fXgTzxiISIZYHiv49439721 = -72595834;    int fXgTzxiISIZYHiv7772756 = -11453521;    int fXgTzxiISIZYHiv26356458 = -926823811;    int fXgTzxiISIZYHiv71666850 = -333119102;    int fXgTzxiISIZYHiv48598089 = -483788103;    int fXgTzxiISIZYHiv16906716 = -717745844;    int fXgTzxiISIZYHiv40943674 = -34098227;    int fXgTzxiISIZYHiv40208726 = -952282588;    int fXgTzxiISIZYHiv56371353 = -144482317;    int fXgTzxiISIZYHiv64081983 = -561770001;    int fXgTzxiISIZYHiv91822721 = -238793646;    int fXgTzxiISIZYHiv47927225 = -468801815;    int fXgTzxiISIZYHiv23966491 = -563850370;    int fXgTzxiISIZYHiv93774338 = 67507682;    int fXgTzxiISIZYHiv24284928 = -364189177;    int fXgTzxiISIZYHiv63432825 = -347551346;    int fXgTzxiISIZYHiv4735387 = 19275865;    int fXgTzxiISIZYHiv46717562 = -861008543;    int fXgTzxiISIZYHiv954167 = 47045386;    int fXgTzxiISIZYHiv74408122 = 16680123;    int fXgTzxiISIZYHiv23026198 = -515615059;    int fXgTzxiISIZYHiv98296808 = -163510440;    int fXgTzxiISIZYHiv91138820 = -622186225;    int fXgTzxiISIZYHiv90067159 = -220715054;    int fXgTzxiISIZYHiv61121487 = -45985866;    int fXgTzxiISIZYHiv99378584 = -285128375;    int fXgTzxiISIZYHiv60491669 = -446978900;    int fXgTzxiISIZYHiv78048771 = -996627309;    int fXgTzxiISIZYHiv10283711 = -25836990;    int fXgTzxiISIZYHiv79538606 = -696066205;    int fXgTzxiISIZYHiv79184051 = -213739563;    int fXgTzxiISIZYHiv34337685 = -533373297;    int fXgTzxiISIZYHiv84614418 = -665099575;    int fXgTzxiISIZYHiv5874289 = -332338052;    int fXgTzxiISIZYHiv37036583 = -678912852;    int fXgTzxiISIZYHiv77095152 = -982942759;    int fXgTzxiISIZYHiv17366897 = -5059334;    int fXgTzxiISIZYHiv32503052 = -894312835;    int fXgTzxiISIZYHiv47815229 = -71550835;     fXgTzxiISIZYHiv63583285 = fXgTzxiISIZYHiv11532479;     fXgTzxiISIZYHiv11532479 = fXgTzxiISIZYHiv76560027;     fXgTzxiISIZYHiv76560027 = fXgTzxiISIZYHiv24107162;     fXgTzxiISIZYHiv24107162 = fXgTzxiISIZYHiv41942376;     fXgTzxiISIZYHiv41942376 = fXgTzxiISIZYHiv49126409;     fXgTzxiISIZYHiv49126409 = fXgTzxiISIZYHiv20462355;     fXgTzxiISIZYHiv20462355 = fXgTzxiISIZYHiv85789187;     fXgTzxiISIZYHiv85789187 = fXgTzxiISIZYHiv1674166;     fXgTzxiISIZYHiv1674166 = fXgTzxiISIZYHiv28337839;     fXgTzxiISIZYHiv28337839 = fXgTzxiISIZYHiv79903229;     fXgTzxiISIZYHiv79903229 = fXgTzxiISIZYHiv34562860;     fXgTzxiISIZYHiv34562860 = fXgTzxiISIZYHiv51092685;     fXgTzxiISIZYHiv51092685 = fXgTzxiISIZYHiv6919545;     fXgTzxiISIZYHiv6919545 = fXgTzxiISIZYHiv38537149;     fXgTzxiISIZYHiv38537149 = fXgTzxiISIZYHiv76695182;     fXgTzxiISIZYHiv76695182 = fXgTzxiISIZYHiv48002217;     fXgTzxiISIZYHiv48002217 = fXgTzxiISIZYHiv4295884;     fXgTzxiISIZYHiv4295884 = fXgTzxiISIZYHiv74995769;     fXgTzxiISIZYHiv74995769 = fXgTzxiISIZYHiv80250642;     fXgTzxiISIZYHiv80250642 = fXgTzxiISIZYHiv9152836;     fXgTzxiISIZYHiv9152836 = fXgTzxiISIZYHiv7156179;     fXgTzxiISIZYHiv7156179 = fXgTzxiISIZYHiv8905290;     fXgTzxiISIZYHiv8905290 = fXgTzxiISIZYHiv43130880;     fXgTzxiISIZYHiv43130880 = fXgTzxiISIZYHiv2265352;     fXgTzxiISIZYHiv2265352 = fXgTzxiISIZYHiv70188136;     fXgTzxiISIZYHiv70188136 = fXgTzxiISIZYHiv3971420;     fXgTzxiISIZYHiv3971420 = fXgTzxiISIZYHiv71022633;     fXgTzxiISIZYHiv71022633 = fXgTzxiISIZYHiv78016432;     fXgTzxiISIZYHiv78016432 = fXgTzxiISIZYHiv75317708;     fXgTzxiISIZYHiv75317708 = fXgTzxiISIZYHiv56670989;     fXgTzxiISIZYHiv56670989 = fXgTzxiISIZYHiv31305141;     fXgTzxiISIZYHiv31305141 = fXgTzxiISIZYHiv17656145;     fXgTzxiISIZYHiv17656145 = fXgTzxiISIZYHiv10149011;     fXgTzxiISIZYHiv10149011 = fXgTzxiISIZYHiv66710818;     fXgTzxiISIZYHiv66710818 = fXgTzxiISIZYHiv82165795;     fXgTzxiISIZYHiv82165795 = fXgTzxiISIZYHiv12613199;     fXgTzxiISIZYHiv12613199 = fXgTzxiISIZYHiv56179495;     fXgTzxiISIZYHiv56179495 = fXgTzxiISIZYHiv56368658;     fXgTzxiISIZYHiv56368658 = fXgTzxiISIZYHiv51029278;     fXgTzxiISIZYHiv51029278 = fXgTzxiISIZYHiv86476304;     fXgTzxiISIZYHiv86476304 = fXgTzxiISIZYHiv84867907;     fXgTzxiISIZYHiv84867907 = fXgTzxiISIZYHiv43723354;     fXgTzxiISIZYHiv43723354 = fXgTzxiISIZYHiv4169903;     fXgTzxiISIZYHiv4169903 = fXgTzxiISIZYHiv96413317;     fXgTzxiISIZYHiv96413317 = fXgTzxiISIZYHiv1311186;     fXgTzxiISIZYHiv1311186 = fXgTzxiISIZYHiv95780014;     fXgTzxiISIZYHiv95780014 = fXgTzxiISIZYHiv80945222;     fXgTzxiISIZYHiv80945222 = fXgTzxiISIZYHiv72725825;     fXgTzxiISIZYHiv72725825 = fXgTzxiISIZYHiv86877612;     fXgTzxiISIZYHiv86877612 = fXgTzxiISIZYHiv85250549;     fXgTzxiISIZYHiv85250549 = fXgTzxiISIZYHiv95549501;     fXgTzxiISIZYHiv95549501 = fXgTzxiISIZYHiv31926556;     fXgTzxiISIZYHiv31926556 = fXgTzxiISIZYHiv57164475;     fXgTzxiISIZYHiv57164475 = fXgTzxiISIZYHiv32100239;     fXgTzxiISIZYHiv32100239 = fXgTzxiISIZYHiv56427107;     fXgTzxiISIZYHiv56427107 = fXgTzxiISIZYHiv2627190;     fXgTzxiISIZYHiv2627190 = fXgTzxiISIZYHiv33429148;     fXgTzxiISIZYHiv33429148 = fXgTzxiISIZYHiv21841810;     fXgTzxiISIZYHiv21841810 = fXgTzxiISIZYHiv71754239;     fXgTzxiISIZYHiv71754239 = fXgTzxiISIZYHiv45154990;     fXgTzxiISIZYHiv45154990 = fXgTzxiISIZYHiv49439721;     fXgTzxiISIZYHiv49439721 = fXgTzxiISIZYHiv7772756;     fXgTzxiISIZYHiv7772756 = fXgTzxiISIZYHiv26356458;     fXgTzxiISIZYHiv26356458 = fXgTzxiISIZYHiv71666850;     fXgTzxiISIZYHiv71666850 = fXgTzxiISIZYHiv48598089;     fXgTzxiISIZYHiv48598089 = fXgTzxiISIZYHiv16906716;     fXgTzxiISIZYHiv16906716 = fXgTzxiISIZYHiv40943674;     fXgTzxiISIZYHiv40943674 = fXgTzxiISIZYHiv40208726;     fXgTzxiISIZYHiv40208726 = fXgTzxiISIZYHiv56371353;     fXgTzxiISIZYHiv56371353 = fXgTzxiISIZYHiv64081983;     fXgTzxiISIZYHiv64081983 = fXgTzxiISIZYHiv91822721;     fXgTzxiISIZYHiv91822721 = fXgTzxiISIZYHiv47927225;     fXgTzxiISIZYHiv47927225 = fXgTzxiISIZYHiv23966491;     fXgTzxiISIZYHiv23966491 = fXgTzxiISIZYHiv93774338;     fXgTzxiISIZYHiv93774338 = fXgTzxiISIZYHiv24284928;     fXgTzxiISIZYHiv24284928 = fXgTzxiISIZYHiv63432825;     fXgTzxiISIZYHiv63432825 = fXgTzxiISIZYHiv4735387;     fXgTzxiISIZYHiv4735387 = fXgTzxiISIZYHiv46717562;     fXgTzxiISIZYHiv46717562 = fXgTzxiISIZYHiv954167;     fXgTzxiISIZYHiv954167 = fXgTzxiISIZYHiv74408122;     fXgTzxiISIZYHiv74408122 = fXgTzxiISIZYHiv23026198;     fXgTzxiISIZYHiv23026198 = fXgTzxiISIZYHiv98296808;     fXgTzxiISIZYHiv98296808 = fXgTzxiISIZYHiv91138820;     fXgTzxiISIZYHiv91138820 = fXgTzxiISIZYHiv90067159;     fXgTzxiISIZYHiv90067159 = fXgTzxiISIZYHiv61121487;     fXgTzxiISIZYHiv61121487 = fXgTzxiISIZYHiv99378584;     fXgTzxiISIZYHiv99378584 = fXgTzxiISIZYHiv60491669;     fXgTzxiISIZYHiv60491669 = fXgTzxiISIZYHiv78048771;     fXgTzxiISIZYHiv78048771 = fXgTzxiISIZYHiv10283711;     fXgTzxiISIZYHiv10283711 = fXgTzxiISIZYHiv79538606;     fXgTzxiISIZYHiv79538606 = fXgTzxiISIZYHiv79184051;     fXgTzxiISIZYHiv79184051 = fXgTzxiISIZYHiv34337685;     fXgTzxiISIZYHiv34337685 = fXgTzxiISIZYHiv84614418;     fXgTzxiISIZYHiv84614418 = fXgTzxiISIZYHiv5874289;     fXgTzxiISIZYHiv5874289 = fXgTzxiISIZYHiv37036583;     fXgTzxiISIZYHiv37036583 = fXgTzxiISIZYHiv77095152;     fXgTzxiISIZYHiv77095152 = fXgTzxiISIZYHiv17366897;     fXgTzxiISIZYHiv17366897 = fXgTzxiISIZYHiv32503052;     fXgTzxiISIZYHiv32503052 = fXgTzxiISIZYHiv47815229;     fXgTzxiISIZYHiv47815229 = fXgTzxiISIZYHiv63583285;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void pjzhYqCLvkObFweDKIhvYBoZKDKQo43442413() {     int rgEJSSidmEDIDfL60096206 = -70351523;    int rgEJSSidmEDIDfL26615305 = -349488563;    int rgEJSSidmEDIDfL32242339 = -300001314;    int rgEJSSidmEDIDfL36656010 = -188914590;    int rgEJSSidmEDIDfL10594766 = -304835073;    int rgEJSSidmEDIDfL86373281 = -2821475;    int rgEJSSidmEDIDfL94277008 = -641072934;    int rgEJSSidmEDIDfL21241358 = 74676526;    int rgEJSSidmEDIDfL5928423 = -555126251;    int rgEJSSidmEDIDfL82342375 = -572756789;    int rgEJSSidmEDIDfL40119345 = 47701868;    int rgEJSSidmEDIDfL49579297 = -491923071;    int rgEJSSidmEDIDfL56363383 = -331958225;    int rgEJSSidmEDIDfL73994657 = -179757457;    int rgEJSSidmEDIDfL43436906 = -880519421;    int rgEJSSidmEDIDfL89652005 = -515204498;    int rgEJSSidmEDIDfL75693065 = -350275002;    int rgEJSSidmEDIDfL40431372 = -334552781;    int rgEJSSidmEDIDfL56918746 = 10054979;    int rgEJSSidmEDIDfL20975902 = -793525834;    int rgEJSSidmEDIDfL16831691 = -669135053;    int rgEJSSidmEDIDfL48057936 = -757452183;    int rgEJSSidmEDIDfL98402251 = -589075782;    int rgEJSSidmEDIDfL66955650 = -629942234;    int rgEJSSidmEDIDfL80540358 = -780265545;    int rgEJSSidmEDIDfL46212501 = -245418014;    int rgEJSSidmEDIDfL51868104 = -194846446;    int rgEJSSidmEDIDfL63272996 = 40684776;    int rgEJSSidmEDIDfL60212404 = -666363753;    int rgEJSSidmEDIDfL75572216 = -840661529;    int rgEJSSidmEDIDfL65765068 = -782880627;    int rgEJSSidmEDIDfL67055324 = -825280812;    int rgEJSSidmEDIDfL35100583 = -95541089;    int rgEJSSidmEDIDfL22808679 = -832584335;    int rgEJSSidmEDIDfL76195656 = 67381139;    int rgEJSSidmEDIDfL25837050 = 38212897;    int rgEJSSidmEDIDfL93170165 = -705237240;    int rgEJSSidmEDIDfL29306470 = 80233720;    int rgEJSSidmEDIDfL5102965 = -589293652;    int rgEJSSidmEDIDfL30546678 = -473315234;    int rgEJSSidmEDIDfL18747299 = -283050857;    int rgEJSSidmEDIDfL29061260 = -501837242;    int rgEJSSidmEDIDfL31265290 = -428155717;    int rgEJSSidmEDIDfL16378011 = -369065690;    int rgEJSSidmEDIDfL62400033 = -241970625;    int rgEJSSidmEDIDfL98992211 = -434212967;    int rgEJSSidmEDIDfL92692107 = -261323523;    int rgEJSSidmEDIDfL61435245 = 84856460;    int rgEJSSidmEDIDfL91334893 = -671552881;    int rgEJSSidmEDIDfL59678651 = -428339001;    int rgEJSSidmEDIDfL37039190 = -684548839;    int rgEJSSidmEDIDfL67958140 = -917091859;    int rgEJSSidmEDIDfL24069934 = -900055061;    int rgEJSSidmEDIDfL42671671 = -108666426;    int rgEJSSidmEDIDfL26610723 = -348168820;    int rgEJSSidmEDIDfL12038271 = -312899341;    int rgEJSSidmEDIDfL28213053 = -760412782;    int rgEJSSidmEDIDfL65286688 = -670059080;    int rgEJSSidmEDIDfL56115651 = -408649046;    int rgEJSSidmEDIDfL64382265 = 40582940;    int rgEJSSidmEDIDfL34505177 = -807975030;    int rgEJSSidmEDIDfL31004012 = -581757710;    int rgEJSSidmEDIDfL61028954 = -258959722;    int rgEJSSidmEDIDfL30356207 = -714464723;    int rgEJSSidmEDIDfL16577307 = -789876162;    int rgEJSSidmEDIDfL73064021 = -127017320;    int rgEJSSidmEDIDfL14478714 = -296381982;    int rgEJSSidmEDIDfL33554704 = -499373891;    int rgEJSSidmEDIDfL97799001 = -147138597;    int rgEJSSidmEDIDfL17599857 = -818732319;    int rgEJSSidmEDIDfL96481839 = -809967259;    int rgEJSSidmEDIDfL46386596 = -330508722;    int rgEJSSidmEDIDfL35328407 = -745259129;    int rgEJSSidmEDIDfL26372068 = -516629788;    int rgEJSSidmEDIDfL2228603 = -410474978;    int rgEJSSidmEDIDfL87770430 = -67297812;    int rgEJSSidmEDIDfL16792646 = -229296466;    int rgEJSSidmEDIDfL82024241 = -120010092;    int rgEJSSidmEDIDfL4555618 = -287971609;    int rgEJSSidmEDIDfL81548147 = -246052578;    int rgEJSSidmEDIDfL53520394 = -984094491;    int rgEJSSidmEDIDfL90432858 = -179702907;    int rgEJSSidmEDIDfL71938103 = -287762343;    int rgEJSSidmEDIDfL533753 = -138024753;    int rgEJSSidmEDIDfL38533027 = -56112690;    int rgEJSSidmEDIDfL97806928 = -865788769;    int rgEJSSidmEDIDfL42985390 = -925225751;    int rgEJSSidmEDIDfL92428912 = -986874664;    int rgEJSSidmEDIDfL96197956 = -384415516;    int rgEJSSidmEDIDfL64157386 = -619719520;    int rgEJSSidmEDIDfL97623996 = -201374322;    int rgEJSSidmEDIDfL27883478 = 64821840;    int rgEJSSidmEDIDfL73190818 = -511117234;    int rgEJSSidmEDIDfL40720700 = -529876592;    int rgEJSSidmEDIDfL96041500 = -665340205;    int rgEJSSidmEDIDfL87743287 = -701293147;    int rgEJSSidmEDIDfL68032305 = -142877520;    int rgEJSSidmEDIDfL909084 = -713690995;    int rgEJSSidmEDIDfL99800703 = -579189529;    int rgEJSSidmEDIDfL89336011 = -70351523;     rgEJSSidmEDIDfL60096206 = rgEJSSidmEDIDfL26615305;     rgEJSSidmEDIDfL26615305 = rgEJSSidmEDIDfL32242339;     rgEJSSidmEDIDfL32242339 = rgEJSSidmEDIDfL36656010;     rgEJSSidmEDIDfL36656010 = rgEJSSidmEDIDfL10594766;     rgEJSSidmEDIDfL10594766 = rgEJSSidmEDIDfL86373281;     rgEJSSidmEDIDfL86373281 = rgEJSSidmEDIDfL94277008;     rgEJSSidmEDIDfL94277008 = rgEJSSidmEDIDfL21241358;     rgEJSSidmEDIDfL21241358 = rgEJSSidmEDIDfL5928423;     rgEJSSidmEDIDfL5928423 = rgEJSSidmEDIDfL82342375;     rgEJSSidmEDIDfL82342375 = rgEJSSidmEDIDfL40119345;     rgEJSSidmEDIDfL40119345 = rgEJSSidmEDIDfL49579297;     rgEJSSidmEDIDfL49579297 = rgEJSSidmEDIDfL56363383;     rgEJSSidmEDIDfL56363383 = rgEJSSidmEDIDfL73994657;     rgEJSSidmEDIDfL73994657 = rgEJSSidmEDIDfL43436906;     rgEJSSidmEDIDfL43436906 = rgEJSSidmEDIDfL89652005;     rgEJSSidmEDIDfL89652005 = rgEJSSidmEDIDfL75693065;     rgEJSSidmEDIDfL75693065 = rgEJSSidmEDIDfL40431372;     rgEJSSidmEDIDfL40431372 = rgEJSSidmEDIDfL56918746;     rgEJSSidmEDIDfL56918746 = rgEJSSidmEDIDfL20975902;     rgEJSSidmEDIDfL20975902 = rgEJSSidmEDIDfL16831691;     rgEJSSidmEDIDfL16831691 = rgEJSSidmEDIDfL48057936;     rgEJSSidmEDIDfL48057936 = rgEJSSidmEDIDfL98402251;     rgEJSSidmEDIDfL98402251 = rgEJSSidmEDIDfL66955650;     rgEJSSidmEDIDfL66955650 = rgEJSSidmEDIDfL80540358;     rgEJSSidmEDIDfL80540358 = rgEJSSidmEDIDfL46212501;     rgEJSSidmEDIDfL46212501 = rgEJSSidmEDIDfL51868104;     rgEJSSidmEDIDfL51868104 = rgEJSSidmEDIDfL63272996;     rgEJSSidmEDIDfL63272996 = rgEJSSidmEDIDfL60212404;     rgEJSSidmEDIDfL60212404 = rgEJSSidmEDIDfL75572216;     rgEJSSidmEDIDfL75572216 = rgEJSSidmEDIDfL65765068;     rgEJSSidmEDIDfL65765068 = rgEJSSidmEDIDfL67055324;     rgEJSSidmEDIDfL67055324 = rgEJSSidmEDIDfL35100583;     rgEJSSidmEDIDfL35100583 = rgEJSSidmEDIDfL22808679;     rgEJSSidmEDIDfL22808679 = rgEJSSidmEDIDfL76195656;     rgEJSSidmEDIDfL76195656 = rgEJSSidmEDIDfL25837050;     rgEJSSidmEDIDfL25837050 = rgEJSSidmEDIDfL93170165;     rgEJSSidmEDIDfL93170165 = rgEJSSidmEDIDfL29306470;     rgEJSSidmEDIDfL29306470 = rgEJSSidmEDIDfL5102965;     rgEJSSidmEDIDfL5102965 = rgEJSSidmEDIDfL30546678;     rgEJSSidmEDIDfL30546678 = rgEJSSidmEDIDfL18747299;     rgEJSSidmEDIDfL18747299 = rgEJSSidmEDIDfL29061260;     rgEJSSidmEDIDfL29061260 = rgEJSSidmEDIDfL31265290;     rgEJSSidmEDIDfL31265290 = rgEJSSidmEDIDfL16378011;     rgEJSSidmEDIDfL16378011 = rgEJSSidmEDIDfL62400033;     rgEJSSidmEDIDfL62400033 = rgEJSSidmEDIDfL98992211;     rgEJSSidmEDIDfL98992211 = rgEJSSidmEDIDfL92692107;     rgEJSSidmEDIDfL92692107 = rgEJSSidmEDIDfL61435245;     rgEJSSidmEDIDfL61435245 = rgEJSSidmEDIDfL91334893;     rgEJSSidmEDIDfL91334893 = rgEJSSidmEDIDfL59678651;     rgEJSSidmEDIDfL59678651 = rgEJSSidmEDIDfL37039190;     rgEJSSidmEDIDfL37039190 = rgEJSSidmEDIDfL67958140;     rgEJSSidmEDIDfL67958140 = rgEJSSidmEDIDfL24069934;     rgEJSSidmEDIDfL24069934 = rgEJSSidmEDIDfL42671671;     rgEJSSidmEDIDfL42671671 = rgEJSSidmEDIDfL26610723;     rgEJSSidmEDIDfL26610723 = rgEJSSidmEDIDfL12038271;     rgEJSSidmEDIDfL12038271 = rgEJSSidmEDIDfL28213053;     rgEJSSidmEDIDfL28213053 = rgEJSSidmEDIDfL65286688;     rgEJSSidmEDIDfL65286688 = rgEJSSidmEDIDfL56115651;     rgEJSSidmEDIDfL56115651 = rgEJSSidmEDIDfL64382265;     rgEJSSidmEDIDfL64382265 = rgEJSSidmEDIDfL34505177;     rgEJSSidmEDIDfL34505177 = rgEJSSidmEDIDfL31004012;     rgEJSSidmEDIDfL31004012 = rgEJSSidmEDIDfL61028954;     rgEJSSidmEDIDfL61028954 = rgEJSSidmEDIDfL30356207;     rgEJSSidmEDIDfL30356207 = rgEJSSidmEDIDfL16577307;     rgEJSSidmEDIDfL16577307 = rgEJSSidmEDIDfL73064021;     rgEJSSidmEDIDfL73064021 = rgEJSSidmEDIDfL14478714;     rgEJSSidmEDIDfL14478714 = rgEJSSidmEDIDfL33554704;     rgEJSSidmEDIDfL33554704 = rgEJSSidmEDIDfL97799001;     rgEJSSidmEDIDfL97799001 = rgEJSSidmEDIDfL17599857;     rgEJSSidmEDIDfL17599857 = rgEJSSidmEDIDfL96481839;     rgEJSSidmEDIDfL96481839 = rgEJSSidmEDIDfL46386596;     rgEJSSidmEDIDfL46386596 = rgEJSSidmEDIDfL35328407;     rgEJSSidmEDIDfL35328407 = rgEJSSidmEDIDfL26372068;     rgEJSSidmEDIDfL26372068 = rgEJSSidmEDIDfL2228603;     rgEJSSidmEDIDfL2228603 = rgEJSSidmEDIDfL87770430;     rgEJSSidmEDIDfL87770430 = rgEJSSidmEDIDfL16792646;     rgEJSSidmEDIDfL16792646 = rgEJSSidmEDIDfL82024241;     rgEJSSidmEDIDfL82024241 = rgEJSSidmEDIDfL4555618;     rgEJSSidmEDIDfL4555618 = rgEJSSidmEDIDfL81548147;     rgEJSSidmEDIDfL81548147 = rgEJSSidmEDIDfL53520394;     rgEJSSidmEDIDfL53520394 = rgEJSSidmEDIDfL90432858;     rgEJSSidmEDIDfL90432858 = rgEJSSidmEDIDfL71938103;     rgEJSSidmEDIDfL71938103 = rgEJSSidmEDIDfL533753;     rgEJSSidmEDIDfL533753 = rgEJSSidmEDIDfL38533027;     rgEJSSidmEDIDfL38533027 = rgEJSSidmEDIDfL97806928;     rgEJSSidmEDIDfL97806928 = rgEJSSidmEDIDfL42985390;     rgEJSSidmEDIDfL42985390 = rgEJSSidmEDIDfL92428912;     rgEJSSidmEDIDfL92428912 = rgEJSSidmEDIDfL96197956;     rgEJSSidmEDIDfL96197956 = rgEJSSidmEDIDfL64157386;     rgEJSSidmEDIDfL64157386 = rgEJSSidmEDIDfL97623996;     rgEJSSidmEDIDfL97623996 = rgEJSSidmEDIDfL27883478;     rgEJSSidmEDIDfL27883478 = rgEJSSidmEDIDfL73190818;     rgEJSSidmEDIDfL73190818 = rgEJSSidmEDIDfL40720700;     rgEJSSidmEDIDfL40720700 = rgEJSSidmEDIDfL96041500;     rgEJSSidmEDIDfL96041500 = rgEJSSidmEDIDfL87743287;     rgEJSSidmEDIDfL87743287 = rgEJSSidmEDIDfL68032305;     rgEJSSidmEDIDfL68032305 = rgEJSSidmEDIDfL909084;     rgEJSSidmEDIDfL909084 = rgEJSSidmEDIDfL99800703;     rgEJSSidmEDIDfL99800703 = rgEJSSidmEDIDfL89336011;     rgEJSSidmEDIDfL89336011 = rgEJSSidmEDIDfL60096206;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mJKFdbkqOWDzkIQhMcFaqFXYmDAtZ45631751() {     int uRYZXWPMwyoMetG56609127 = -69152211;    int uRYZXWPMwyoMetG41698130 = -302176865;    int uRYZXWPMwyoMetG87924649 = -48104079;    int uRYZXWPMwyoMetG49204858 = -657728381;    int uRYZXWPMwyoMetG79247156 = -960422069;    int uRYZXWPMwyoMetG23620154 = -33349051;    int uRYZXWPMwyoMetG68091661 = -805990909;    int uRYZXWPMwyoMetG56693528 = -38207659;    int uRYZXWPMwyoMetG10182680 = -807544005;    int uRYZXWPMwyoMetG36346912 = -231482001;    int uRYZXWPMwyoMetG335461 = 72466631;    int uRYZXWPMwyoMetG64595734 = 39344023;    int uRYZXWPMwyoMetG61634080 = -617743889;    int uRYZXWPMwyoMetG41069770 = -689278992;    int uRYZXWPMwyoMetG48336663 = -731846131;    int uRYZXWPMwyoMetG2608830 = -273825713;    int uRYZXWPMwyoMetG3383915 = -263122047;    int uRYZXWPMwyoMetG76566859 = -952078846;    int uRYZXWPMwyoMetG38841722 = -479072322;    int uRYZXWPMwyoMetG61701161 = -703050665;    int uRYZXWPMwyoMetG24510545 = -879684651;    int uRYZXWPMwyoMetG88959693 = -35469875;    int uRYZXWPMwyoMetG87899214 = -869995492;    int uRYZXWPMwyoMetG90780420 = -589059638;    int uRYZXWPMwyoMetG58815366 = -305691304;    int uRYZXWPMwyoMetG22236866 = -424713277;    int uRYZXWPMwyoMetG99764788 = -22028293;    int uRYZXWPMwyoMetG55523359 = -715071323;    int uRYZXWPMwyoMetG42408375 = -531741736;    int uRYZXWPMwyoMetG75826723 = -205438371;    int uRYZXWPMwyoMetG74859148 = 15151219;    int uRYZXWPMwyoMetG2805507 = -57286832;    int uRYZXWPMwyoMetG52545022 = 14362143;    int uRYZXWPMwyoMetG35468347 = -653094335;    int uRYZXWPMwyoMetG85680494 = -147284386;    int uRYZXWPMwyoMetG69508303 = -238863812;    int uRYZXWPMwyoMetG73727132 = -215661197;    int uRYZXWPMwyoMetG2433444 = -840898249;    int uRYZXWPMwyoMetG53837271 = -930362402;    int uRYZXWPMwyoMetG10064077 = 90336884;    int uRYZXWPMwyoMetG51018294 = -814593027;    int uRYZXWPMwyoMetG73254612 = 90721796;    int uRYZXWPMwyoMetG18807226 = -924428289;    int uRYZXWPMwyoMetG28586119 = -510699444;    int uRYZXWPMwyoMetG28386748 = -774124963;    int uRYZXWPMwyoMetG96673238 = -766540763;    int uRYZXWPMwyoMetG89604200 = -539844172;    int uRYZXWPMwyoMetG41925269 = -78237539;    int uRYZXWPMwyoMetG9943962 = -203057077;    int uRYZXWPMwyoMetG32479691 = -777878456;    int uRYZXWPMwyoMetG88827829 = -213928046;    int uRYZXWPMwyoMetG40366778 = -399257109;    int uRYZXWPMwyoMetG16213313 = -591963706;    int uRYZXWPMwyoMetG28178867 = -558867430;    int uRYZXWPMwyoMetG21121206 = -780890612;    int uRYZXWPMwyoMetG67649434 = 66317664;    int uRYZXWPMwyoMetG53798916 = -432181374;    int uRYZXWPMwyoMetG97144228 = -459044442;    int uRYZXWPMwyoMetG90389492 = -252037078;    int uRYZXWPMwyoMetG57010290 = -435708793;    int uRYZXWPMwyoMetG23855365 = 88679241;    int uRYZXWPMwyoMetG12568303 = 9080414;    int uRYZXWPMwyoMetG14285154 = -506465923;    int uRYZXWPMwyoMetG34355956 = -502105634;    int uRYZXWPMwyoMetG61487764 = -146633221;    int uRYZXWPMwyoMetG97529953 = -870246537;    int uRYZXWPMwyoMetG12050712 = -975018120;    int uRYZXWPMwyoMetG26165733 = -964649555;    int uRYZXWPMwyoMetG55389276 = -441994606;    int uRYZXWPMwyoMetG78828360 = -392982320;    int uRYZXWPMwyoMetG28881697 = 41835484;    int uRYZXWPMwyoMetG950471 = -422223798;    int uRYZXWPMwyoMetG22729588 = 78283556;    int uRYZXWPMwyoMetG28777645 = -469409206;    int uRYZXWPMwyoMetG10682867 = -888457638;    int uRYZXWPMwyoMetG51255933 = -870406448;    int uRYZXWPMwyoMetG70152467 = -111041587;    int uRYZXWPMwyoMetG59313095 = -259296049;    int uRYZXWPMwyoMetG62393673 = -814934676;    int uRYZXWPMwyoMetG62142127 = -539150541;    int uRYZXWPMwyoMetG32632666 = -884869105;    int uRYZXWPMwyoMetG57839520 = -943790754;    int uRYZXWPMwyoMetG45579397 = -412014247;    int uRYZXWPMwyoMetG9928685 = -753863281;    int uRYZXWPMwyoMetG86998894 = -991510326;    int uRYZXWPMwyoMetG34492370 = -585591672;    int uRYZXWPMwyoMetG86592194 = -465323127;    int uRYZXWPMwyoMetG24366155 = -426770427;    int uRYZXWPMwyoMetG14347142 = -872203723;    int uRYZXWPMwyoMetG18031061 = -113602050;    int uRYZXWPMwyoMetG15709388 = -806682439;    int uRYZXWPMwyoMetG76582903 = -756616756;    int uRYZXWPMwyoMetG12043952 = -488861172;    int uRYZXWPMwyoMetG96826981 = -394653610;    int uRYZXWPMwyoMetG86208712 = -998342357;    int uRYZXWPMwyoMetG38449992 = -723673441;    int uRYZXWPMwyoMetG58969459 = -402812281;    int uRYZXWPMwyoMetG84451270 = -322322655;    int uRYZXWPMwyoMetG67098355 = -264066223;    int uRYZXWPMwyoMetG30856795 = -69152211;     uRYZXWPMwyoMetG56609127 = uRYZXWPMwyoMetG41698130;     uRYZXWPMwyoMetG41698130 = uRYZXWPMwyoMetG87924649;     uRYZXWPMwyoMetG87924649 = uRYZXWPMwyoMetG49204858;     uRYZXWPMwyoMetG49204858 = uRYZXWPMwyoMetG79247156;     uRYZXWPMwyoMetG79247156 = uRYZXWPMwyoMetG23620154;     uRYZXWPMwyoMetG23620154 = uRYZXWPMwyoMetG68091661;     uRYZXWPMwyoMetG68091661 = uRYZXWPMwyoMetG56693528;     uRYZXWPMwyoMetG56693528 = uRYZXWPMwyoMetG10182680;     uRYZXWPMwyoMetG10182680 = uRYZXWPMwyoMetG36346912;     uRYZXWPMwyoMetG36346912 = uRYZXWPMwyoMetG335461;     uRYZXWPMwyoMetG335461 = uRYZXWPMwyoMetG64595734;     uRYZXWPMwyoMetG64595734 = uRYZXWPMwyoMetG61634080;     uRYZXWPMwyoMetG61634080 = uRYZXWPMwyoMetG41069770;     uRYZXWPMwyoMetG41069770 = uRYZXWPMwyoMetG48336663;     uRYZXWPMwyoMetG48336663 = uRYZXWPMwyoMetG2608830;     uRYZXWPMwyoMetG2608830 = uRYZXWPMwyoMetG3383915;     uRYZXWPMwyoMetG3383915 = uRYZXWPMwyoMetG76566859;     uRYZXWPMwyoMetG76566859 = uRYZXWPMwyoMetG38841722;     uRYZXWPMwyoMetG38841722 = uRYZXWPMwyoMetG61701161;     uRYZXWPMwyoMetG61701161 = uRYZXWPMwyoMetG24510545;     uRYZXWPMwyoMetG24510545 = uRYZXWPMwyoMetG88959693;     uRYZXWPMwyoMetG88959693 = uRYZXWPMwyoMetG87899214;     uRYZXWPMwyoMetG87899214 = uRYZXWPMwyoMetG90780420;     uRYZXWPMwyoMetG90780420 = uRYZXWPMwyoMetG58815366;     uRYZXWPMwyoMetG58815366 = uRYZXWPMwyoMetG22236866;     uRYZXWPMwyoMetG22236866 = uRYZXWPMwyoMetG99764788;     uRYZXWPMwyoMetG99764788 = uRYZXWPMwyoMetG55523359;     uRYZXWPMwyoMetG55523359 = uRYZXWPMwyoMetG42408375;     uRYZXWPMwyoMetG42408375 = uRYZXWPMwyoMetG75826723;     uRYZXWPMwyoMetG75826723 = uRYZXWPMwyoMetG74859148;     uRYZXWPMwyoMetG74859148 = uRYZXWPMwyoMetG2805507;     uRYZXWPMwyoMetG2805507 = uRYZXWPMwyoMetG52545022;     uRYZXWPMwyoMetG52545022 = uRYZXWPMwyoMetG35468347;     uRYZXWPMwyoMetG35468347 = uRYZXWPMwyoMetG85680494;     uRYZXWPMwyoMetG85680494 = uRYZXWPMwyoMetG69508303;     uRYZXWPMwyoMetG69508303 = uRYZXWPMwyoMetG73727132;     uRYZXWPMwyoMetG73727132 = uRYZXWPMwyoMetG2433444;     uRYZXWPMwyoMetG2433444 = uRYZXWPMwyoMetG53837271;     uRYZXWPMwyoMetG53837271 = uRYZXWPMwyoMetG10064077;     uRYZXWPMwyoMetG10064077 = uRYZXWPMwyoMetG51018294;     uRYZXWPMwyoMetG51018294 = uRYZXWPMwyoMetG73254612;     uRYZXWPMwyoMetG73254612 = uRYZXWPMwyoMetG18807226;     uRYZXWPMwyoMetG18807226 = uRYZXWPMwyoMetG28586119;     uRYZXWPMwyoMetG28586119 = uRYZXWPMwyoMetG28386748;     uRYZXWPMwyoMetG28386748 = uRYZXWPMwyoMetG96673238;     uRYZXWPMwyoMetG96673238 = uRYZXWPMwyoMetG89604200;     uRYZXWPMwyoMetG89604200 = uRYZXWPMwyoMetG41925269;     uRYZXWPMwyoMetG41925269 = uRYZXWPMwyoMetG9943962;     uRYZXWPMwyoMetG9943962 = uRYZXWPMwyoMetG32479691;     uRYZXWPMwyoMetG32479691 = uRYZXWPMwyoMetG88827829;     uRYZXWPMwyoMetG88827829 = uRYZXWPMwyoMetG40366778;     uRYZXWPMwyoMetG40366778 = uRYZXWPMwyoMetG16213313;     uRYZXWPMwyoMetG16213313 = uRYZXWPMwyoMetG28178867;     uRYZXWPMwyoMetG28178867 = uRYZXWPMwyoMetG21121206;     uRYZXWPMwyoMetG21121206 = uRYZXWPMwyoMetG67649434;     uRYZXWPMwyoMetG67649434 = uRYZXWPMwyoMetG53798916;     uRYZXWPMwyoMetG53798916 = uRYZXWPMwyoMetG97144228;     uRYZXWPMwyoMetG97144228 = uRYZXWPMwyoMetG90389492;     uRYZXWPMwyoMetG90389492 = uRYZXWPMwyoMetG57010290;     uRYZXWPMwyoMetG57010290 = uRYZXWPMwyoMetG23855365;     uRYZXWPMwyoMetG23855365 = uRYZXWPMwyoMetG12568303;     uRYZXWPMwyoMetG12568303 = uRYZXWPMwyoMetG14285154;     uRYZXWPMwyoMetG14285154 = uRYZXWPMwyoMetG34355956;     uRYZXWPMwyoMetG34355956 = uRYZXWPMwyoMetG61487764;     uRYZXWPMwyoMetG61487764 = uRYZXWPMwyoMetG97529953;     uRYZXWPMwyoMetG97529953 = uRYZXWPMwyoMetG12050712;     uRYZXWPMwyoMetG12050712 = uRYZXWPMwyoMetG26165733;     uRYZXWPMwyoMetG26165733 = uRYZXWPMwyoMetG55389276;     uRYZXWPMwyoMetG55389276 = uRYZXWPMwyoMetG78828360;     uRYZXWPMwyoMetG78828360 = uRYZXWPMwyoMetG28881697;     uRYZXWPMwyoMetG28881697 = uRYZXWPMwyoMetG950471;     uRYZXWPMwyoMetG950471 = uRYZXWPMwyoMetG22729588;     uRYZXWPMwyoMetG22729588 = uRYZXWPMwyoMetG28777645;     uRYZXWPMwyoMetG28777645 = uRYZXWPMwyoMetG10682867;     uRYZXWPMwyoMetG10682867 = uRYZXWPMwyoMetG51255933;     uRYZXWPMwyoMetG51255933 = uRYZXWPMwyoMetG70152467;     uRYZXWPMwyoMetG70152467 = uRYZXWPMwyoMetG59313095;     uRYZXWPMwyoMetG59313095 = uRYZXWPMwyoMetG62393673;     uRYZXWPMwyoMetG62393673 = uRYZXWPMwyoMetG62142127;     uRYZXWPMwyoMetG62142127 = uRYZXWPMwyoMetG32632666;     uRYZXWPMwyoMetG32632666 = uRYZXWPMwyoMetG57839520;     uRYZXWPMwyoMetG57839520 = uRYZXWPMwyoMetG45579397;     uRYZXWPMwyoMetG45579397 = uRYZXWPMwyoMetG9928685;     uRYZXWPMwyoMetG9928685 = uRYZXWPMwyoMetG86998894;     uRYZXWPMwyoMetG86998894 = uRYZXWPMwyoMetG34492370;     uRYZXWPMwyoMetG34492370 = uRYZXWPMwyoMetG86592194;     uRYZXWPMwyoMetG86592194 = uRYZXWPMwyoMetG24366155;     uRYZXWPMwyoMetG24366155 = uRYZXWPMwyoMetG14347142;     uRYZXWPMwyoMetG14347142 = uRYZXWPMwyoMetG18031061;     uRYZXWPMwyoMetG18031061 = uRYZXWPMwyoMetG15709388;     uRYZXWPMwyoMetG15709388 = uRYZXWPMwyoMetG76582903;     uRYZXWPMwyoMetG76582903 = uRYZXWPMwyoMetG12043952;     uRYZXWPMwyoMetG12043952 = uRYZXWPMwyoMetG96826981;     uRYZXWPMwyoMetG96826981 = uRYZXWPMwyoMetG86208712;     uRYZXWPMwyoMetG86208712 = uRYZXWPMwyoMetG38449992;     uRYZXWPMwyoMetG38449992 = uRYZXWPMwyoMetG58969459;     uRYZXWPMwyoMetG58969459 = uRYZXWPMwyoMetG84451270;     uRYZXWPMwyoMetG84451270 = uRYZXWPMwyoMetG67098355;     uRYZXWPMwyoMetG67098355 = uRYZXWPMwyoMetG30856795;     uRYZXWPMwyoMetG30856795 = uRYZXWPMwyoMetG56609127;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void qjdlZlAhRDWiCkegnesgviPkJwSIq95578558() {     int gkNwtQUjvoCRlGy23780187 = -813577888;    int gkNwtQUjvoCRlGy86882156 = -83913342;    int gkNwtQUjvoCRlGy22262668 = -281372691;    int gkNwtQUjvoCRlGy16605133 = -396139730;    int gkNwtQUjvoCRlGy60509765 = -931060091;    int gkNwtQUjvoCRlGy16061356 = -66024968;    int gkNwtQUjvoCRlGy91737874 = -165148127;    int gkNwtQUjvoCRlGy79774028 = -793731616;    int gkNwtQUjvoCRlGy77561826 = -339663017;    int gkNwtQUjvoCRlGy16678395 = -554495694;    int gkNwtQUjvoCRlGy82480131 = 21801537;    int gkNwtQUjvoCRlGy10867066 = -9674313;    int gkNwtQUjvoCRlGy14879252 = -791408136;    int gkNwtQUjvoCRlGy38147697 = -654675686;    int gkNwtQUjvoCRlGy90784330 = -770209993;    int gkNwtQUjvoCRlGy73529745 = -369634864;    int gkNwtQUjvoCRlGy69074830 = -265539651;    int gkNwtQUjvoCRlGy9995310 = -61699485;    int gkNwtQUjvoCRlGy82044209 = 42594703;    int gkNwtQUjvoCRlGy89139366 = -542917349;    int gkNwtQUjvoCRlGy7655731 = -188447016;    int gkNwtQUjvoCRlGy12960950 = -234780990;    int gkNwtQUjvoCRlGy42461589 = -159800505;    int gkNwtQUjvoCRlGy4804620 = -259657426;    int gkNwtQUjvoCRlGy72772072 = -180327898;    int gkNwtQUjvoCRlGy12284734 = -794087056;    int gkNwtQUjvoCRlGy85700677 = 56411798;    int gkNwtQUjvoCRlGy20152422 = -266984201;    int gkNwtQUjvoCRlGy44068387 = -45407621;    int gkNwtQUjvoCRlGy39163457 = -453178910;    int gkNwtQUjvoCRlGy57612545 = -748157385;    int gkNwtQUjvoCRlGy98343293 = -325768254;    int gkNwtQUjvoCRlGy67482398 = -720752018;    int gkNwtQUjvoCRlGy13210154 = -160452286;    int gkNwtQUjvoCRlGy90831531 = 50417233;    int gkNwtQUjvoCRlGy30344626 = -515401322;    int gkNwtQUjvoCRlGy86134217 = -915024984;    int gkNwtQUjvoCRlGy58435227 = -450880646;    int gkNwtQUjvoCRlGy45791270 = -514478373;    int gkNwtQUjvoCRlGy12811882 = -354187675;    int gkNwtQUjvoCRlGy55506905 = -542210178;    int gkNwtQUjvoCRlGy60233294 = 7683846;    int gkNwtQUjvoCRlGy95032762 = -848324659;    int gkNwtQUjvoCRlGy88070629 = -67343726;    int gkNwtQUjvoCRlGy6309469 = -41666293;    int gkNwtQUjvoCRlGy60332131 = 54838957;    int gkNwtQUjvoCRlGy84949703 = -453153997;    int gkNwtQUjvoCRlGy12645921 = -84700477;    int gkNwtQUjvoCRlGy15336093 = -156561380;    int gkNwtQUjvoCRlGy93662793 = -723401903;    int gkNwtQUjvoCRlGy95546424 = -811324366;    int gkNwtQUjvoCRlGy40099664 = -568496333;    int gkNwtQUjvoCRlGy87983999 = -319616384;    int gkNwtQUjvoCRlGy93663096 = -38030080;    int gkNwtQUjvoCRlGy87922827 = -467654041;    int gkNwtQUjvoCRlGy10819237 = -478796899;    int gkNwtQUjvoCRlGy44420568 = -924112837;    int gkNwtQUjvoCRlGy17458048 = 78284734;    int gkNwtQUjvoCRlGy43833060 = -115811833;    int gkNwtQUjvoCRlGy48225031 = -36973035;    int gkNwtQUjvoCRlGy30360679 = -22436766;    int gkNwtQUjvoCRlGy71585452 = -898163926;    int gkNwtQUjvoCRlGy35705641 = -648323995;    int gkNwtQUjvoCRlGy38398369 = -886484107;    int gkNwtQUjvoCRlGy59065850 = -806338309;    int gkNwtQUjvoCRlGy84136837 = -652430210;    int gkNwtQUjvoCRlGy43384667 = -288922295;    int gkNwtQUjvoCRlGy1669098 = -530955850;    int gkNwtQUjvoCRlGy47316165 = -605092920;    int gkNwtQUjvoCRlGy60439704 = -154808672;    int gkNwtQUjvoCRlGy87395528 = -454609881;    int gkNwtQUjvoCRlGy10639604 = -814659006;    int gkNwtQUjvoCRlGy64204039 = -547221113;    int gkNwtQUjvoCRlGy69232328 = -603217623;    int gkNwtQUjvoCRlGy33632461 = 99292829;    int gkNwtQUjvoCRlGy47422436 = -96130863;    int gkNwtQUjvoCRlGy17928188 = -386456332;    int gkNwtQUjvoCRlGy54390960 = 7543220;    int gkNwtQUjvoCRlGy98495150 = -117991133;    int gkNwtQUjvoCRlGy12439941 = -135166855;    int gkNwtQUjvoCRlGy27335031 = -240933059;    int gkNwtQUjvoCRlGy73054757 = -858887725;    int gkNwtQUjvoCRlGy4816330 = -10422822;    int gkNwtQUjvoCRlGy50405594 = -322005718;    int gkNwtQUjvoCRlGy43617033 = -641854545;    int gkNwtQUjvoCRlGy17512882 = -79661052;    int gkNwtQUjvoCRlGy10359294 = 93848129;    int gkNwtQUjvoCRlGy73819302 = -582721939;    int gkNwtQUjvoCRlGy25287326 = -692798246;    int gkNwtQUjvoCRlGy80012295 = -470785869;    int gkNwtQUjvoCRlGy85924058 = -591288486;    int gkNwtQUjvoCRlGy68676170 = -893309719;    int gkNwtQUjvoCRlGy14602167 = -235068813;    int gkNwtQUjvoCRlGy97566239 = -377505338;    int gkNwtQUjvoCRlGy82451203 = -231750909;    int gkNwtQUjvoCRlGy83921453 = -644046252;    int gkNwtQUjvoCRlGy24527653 = -343992159;    int gkNwtQUjvoCRlGy56634393 = -961840552;    int gkNwtQUjvoCRlGy29004780 = -261005417;    int gkNwtQUjvoCRlGy22172632 = -813577888;     gkNwtQUjvoCRlGy23780187 = gkNwtQUjvoCRlGy86882156;     gkNwtQUjvoCRlGy86882156 = gkNwtQUjvoCRlGy22262668;     gkNwtQUjvoCRlGy22262668 = gkNwtQUjvoCRlGy16605133;     gkNwtQUjvoCRlGy16605133 = gkNwtQUjvoCRlGy60509765;     gkNwtQUjvoCRlGy60509765 = gkNwtQUjvoCRlGy16061356;     gkNwtQUjvoCRlGy16061356 = gkNwtQUjvoCRlGy91737874;     gkNwtQUjvoCRlGy91737874 = gkNwtQUjvoCRlGy79774028;     gkNwtQUjvoCRlGy79774028 = gkNwtQUjvoCRlGy77561826;     gkNwtQUjvoCRlGy77561826 = gkNwtQUjvoCRlGy16678395;     gkNwtQUjvoCRlGy16678395 = gkNwtQUjvoCRlGy82480131;     gkNwtQUjvoCRlGy82480131 = gkNwtQUjvoCRlGy10867066;     gkNwtQUjvoCRlGy10867066 = gkNwtQUjvoCRlGy14879252;     gkNwtQUjvoCRlGy14879252 = gkNwtQUjvoCRlGy38147697;     gkNwtQUjvoCRlGy38147697 = gkNwtQUjvoCRlGy90784330;     gkNwtQUjvoCRlGy90784330 = gkNwtQUjvoCRlGy73529745;     gkNwtQUjvoCRlGy73529745 = gkNwtQUjvoCRlGy69074830;     gkNwtQUjvoCRlGy69074830 = gkNwtQUjvoCRlGy9995310;     gkNwtQUjvoCRlGy9995310 = gkNwtQUjvoCRlGy82044209;     gkNwtQUjvoCRlGy82044209 = gkNwtQUjvoCRlGy89139366;     gkNwtQUjvoCRlGy89139366 = gkNwtQUjvoCRlGy7655731;     gkNwtQUjvoCRlGy7655731 = gkNwtQUjvoCRlGy12960950;     gkNwtQUjvoCRlGy12960950 = gkNwtQUjvoCRlGy42461589;     gkNwtQUjvoCRlGy42461589 = gkNwtQUjvoCRlGy4804620;     gkNwtQUjvoCRlGy4804620 = gkNwtQUjvoCRlGy72772072;     gkNwtQUjvoCRlGy72772072 = gkNwtQUjvoCRlGy12284734;     gkNwtQUjvoCRlGy12284734 = gkNwtQUjvoCRlGy85700677;     gkNwtQUjvoCRlGy85700677 = gkNwtQUjvoCRlGy20152422;     gkNwtQUjvoCRlGy20152422 = gkNwtQUjvoCRlGy44068387;     gkNwtQUjvoCRlGy44068387 = gkNwtQUjvoCRlGy39163457;     gkNwtQUjvoCRlGy39163457 = gkNwtQUjvoCRlGy57612545;     gkNwtQUjvoCRlGy57612545 = gkNwtQUjvoCRlGy98343293;     gkNwtQUjvoCRlGy98343293 = gkNwtQUjvoCRlGy67482398;     gkNwtQUjvoCRlGy67482398 = gkNwtQUjvoCRlGy13210154;     gkNwtQUjvoCRlGy13210154 = gkNwtQUjvoCRlGy90831531;     gkNwtQUjvoCRlGy90831531 = gkNwtQUjvoCRlGy30344626;     gkNwtQUjvoCRlGy30344626 = gkNwtQUjvoCRlGy86134217;     gkNwtQUjvoCRlGy86134217 = gkNwtQUjvoCRlGy58435227;     gkNwtQUjvoCRlGy58435227 = gkNwtQUjvoCRlGy45791270;     gkNwtQUjvoCRlGy45791270 = gkNwtQUjvoCRlGy12811882;     gkNwtQUjvoCRlGy12811882 = gkNwtQUjvoCRlGy55506905;     gkNwtQUjvoCRlGy55506905 = gkNwtQUjvoCRlGy60233294;     gkNwtQUjvoCRlGy60233294 = gkNwtQUjvoCRlGy95032762;     gkNwtQUjvoCRlGy95032762 = gkNwtQUjvoCRlGy88070629;     gkNwtQUjvoCRlGy88070629 = gkNwtQUjvoCRlGy6309469;     gkNwtQUjvoCRlGy6309469 = gkNwtQUjvoCRlGy60332131;     gkNwtQUjvoCRlGy60332131 = gkNwtQUjvoCRlGy84949703;     gkNwtQUjvoCRlGy84949703 = gkNwtQUjvoCRlGy12645921;     gkNwtQUjvoCRlGy12645921 = gkNwtQUjvoCRlGy15336093;     gkNwtQUjvoCRlGy15336093 = gkNwtQUjvoCRlGy93662793;     gkNwtQUjvoCRlGy93662793 = gkNwtQUjvoCRlGy95546424;     gkNwtQUjvoCRlGy95546424 = gkNwtQUjvoCRlGy40099664;     gkNwtQUjvoCRlGy40099664 = gkNwtQUjvoCRlGy87983999;     gkNwtQUjvoCRlGy87983999 = gkNwtQUjvoCRlGy93663096;     gkNwtQUjvoCRlGy93663096 = gkNwtQUjvoCRlGy87922827;     gkNwtQUjvoCRlGy87922827 = gkNwtQUjvoCRlGy10819237;     gkNwtQUjvoCRlGy10819237 = gkNwtQUjvoCRlGy44420568;     gkNwtQUjvoCRlGy44420568 = gkNwtQUjvoCRlGy17458048;     gkNwtQUjvoCRlGy17458048 = gkNwtQUjvoCRlGy43833060;     gkNwtQUjvoCRlGy43833060 = gkNwtQUjvoCRlGy48225031;     gkNwtQUjvoCRlGy48225031 = gkNwtQUjvoCRlGy30360679;     gkNwtQUjvoCRlGy30360679 = gkNwtQUjvoCRlGy71585452;     gkNwtQUjvoCRlGy71585452 = gkNwtQUjvoCRlGy35705641;     gkNwtQUjvoCRlGy35705641 = gkNwtQUjvoCRlGy38398369;     gkNwtQUjvoCRlGy38398369 = gkNwtQUjvoCRlGy59065850;     gkNwtQUjvoCRlGy59065850 = gkNwtQUjvoCRlGy84136837;     gkNwtQUjvoCRlGy84136837 = gkNwtQUjvoCRlGy43384667;     gkNwtQUjvoCRlGy43384667 = gkNwtQUjvoCRlGy1669098;     gkNwtQUjvoCRlGy1669098 = gkNwtQUjvoCRlGy47316165;     gkNwtQUjvoCRlGy47316165 = gkNwtQUjvoCRlGy60439704;     gkNwtQUjvoCRlGy60439704 = gkNwtQUjvoCRlGy87395528;     gkNwtQUjvoCRlGy87395528 = gkNwtQUjvoCRlGy10639604;     gkNwtQUjvoCRlGy10639604 = gkNwtQUjvoCRlGy64204039;     gkNwtQUjvoCRlGy64204039 = gkNwtQUjvoCRlGy69232328;     gkNwtQUjvoCRlGy69232328 = gkNwtQUjvoCRlGy33632461;     gkNwtQUjvoCRlGy33632461 = gkNwtQUjvoCRlGy47422436;     gkNwtQUjvoCRlGy47422436 = gkNwtQUjvoCRlGy17928188;     gkNwtQUjvoCRlGy17928188 = gkNwtQUjvoCRlGy54390960;     gkNwtQUjvoCRlGy54390960 = gkNwtQUjvoCRlGy98495150;     gkNwtQUjvoCRlGy98495150 = gkNwtQUjvoCRlGy12439941;     gkNwtQUjvoCRlGy12439941 = gkNwtQUjvoCRlGy27335031;     gkNwtQUjvoCRlGy27335031 = gkNwtQUjvoCRlGy73054757;     gkNwtQUjvoCRlGy73054757 = gkNwtQUjvoCRlGy4816330;     gkNwtQUjvoCRlGy4816330 = gkNwtQUjvoCRlGy50405594;     gkNwtQUjvoCRlGy50405594 = gkNwtQUjvoCRlGy43617033;     gkNwtQUjvoCRlGy43617033 = gkNwtQUjvoCRlGy17512882;     gkNwtQUjvoCRlGy17512882 = gkNwtQUjvoCRlGy10359294;     gkNwtQUjvoCRlGy10359294 = gkNwtQUjvoCRlGy73819302;     gkNwtQUjvoCRlGy73819302 = gkNwtQUjvoCRlGy25287326;     gkNwtQUjvoCRlGy25287326 = gkNwtQUjvoCRlGy80012295;     gkNwtQUjvoCRlGy80012295 = gkNwtQUjvoCRlGy85924058;     gkNwtQUjvoCRlGy85924058 = gkNwtQUjvoCRlGy68676170;     gkNwtQUjvoCRlGy68676170 = gkNwtQUjvoCRlGy14602167;     gkNwtQUjvoCRlGy14602167 = gkNwtQUjvoCRlGy97566239;     gkNwtQUjvoCRlGy97566239 = gkNwtQUjvoCRlGy82451203;     gkNwtQUjvoCRlGy82451203 = gkNwtQUjvoCRlGy83921453;     gkNwtQUjvoCRlGy83921453 = gkNwtQUjvoCRlGy24527653;     gkNwtQUjvoCRlGy24527653 = gkNwtQUjvoCRlGy56634393;     gkNwtQUjvoCRlGy56634393 = gkNwtQUjvoCRlGy29004780;     gkNwtQUjvoCRlGy29004780 = gkNwtQUjvoCRlGy22172632;     gkNwtQUjvoCRlGy22172632 = gkNwtQUjvoCRlGy23780187;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RlBVzfUXvIOvwEekcGsFxjThwtqzm97767896() {     int FVwaxAUEQlLaaUk20293108 = -812378576;    int FVwaxAUEQlLaaUk1964982 = -36601644;    int FVwaxAUEQlLaaUk77944978 = -29475457;    int FVwaxAUEQlLaaUk29153981 = -864953521;    int FVwaxAUEQlLaaUk29162155 = -486647086;    int FVwaxAUEQlLaaUk53308228 = -96552544;    int FVwaxAUEQlLaaUk65552527 = -330066102;    int FVwaxAUEQlLaaUk15226199 = -906615800;    int FVwaxAUEQlLaaUk81816083 = -592080771;    int FVwaxAUEQlLaaUk70682931 = -213220907;    int FVwaxAUEQlLaaUk42696247 = 46566299;    int FVwaxAUEQlLaaUk25883503 = -578407218;    int FVwaxAUEQlLaaUk20149949 = 22806200;    int FVwaxAUEQlLaaUk5222810 = -64197221;    int FVwaxAUEQlLaaUk95684087 = -621536703;    int FVwaxAUEQlLaaUk86486569 = -128256080;    int FVwaxAUEQlLaaUk96765679 = -178386696;    int FVwaxAUEQlLaaUk46130798 = -679225550;    int FVwaxAUEQlLaaUk63967186 = -446532598;    int FVwaxAUEQlLaaUk29864626 = -452442179;    int FVwaxAUEQlLaaUk15334586 = -398996614;    int FVwaxAUEQlLaaUk53862707 = -612798683;    int FVwaxAUEQlLaaUk31958551 = -440720216;    int FVwaxAUEQlLaaUk28629390 = -218774830;    int FVwaxAUEQlLaaUk51047079 = -805753657;    int FVwaxAUEQlLaaUk88309098 = -973382318;    int FVwaxAUEQlLaaUk33597363 = -870770049;    int FVwaxAUEQlLaaUk12402785 = 77259700;    int FVwaxAUEQlLaaUk26264359 = 89214396;    int FVwaxAUEQlLaaUk39417965 = -917955753;    int FVwaxAUEQlLaaUk66706625 = 49874462;    int FVwaxAUEQlLaaUk34093477 = -657774274;    int FVwaxAUEQlLaaUk84926837 = -610848786;    int FVwaxAUEQlLaaUk25869822 = 19037714;    int FVwaxAUEQlLaaUk316371 = -164248292;    int FVwaxAUEQlLaaUk74015880 = -792478030;    int FVwaxAUEQlLaaUk66691184 = -425448942;    int FVwaxAUEQlLaaUk31562202 = -272012615;    int FVwaxAUEQlLaaUk94525577 = -855547123;    int FVwaxAUEQlLaaUk92329280 = -890535557;    int FVwaxAUEQlLaaUk87777900 = 26247652;    int FVwaxAUEQlLaaUk4426647 = -499757116;    int FVwaxAUEQlLaaUk82574698 = -244597231;    int FVwaxAUEQlLaaUk278738 = -208977479;    int FVwaxAUEQlLaaUk72296184 = -573820631;    int FVwaxAUEQlLaaUk58013158 = -277488839;    int FVwaxAUEQlLaaUk81861796 = -731674645;    int FVwaxAUEQlLaaUk93135944 = -247794477;    int FVwaxAUEQlLaaUk33945161 = -788065575;    int FVwaxAUEQlLaaUk66463832 = 27058642;    int FVwaxAUEQlLaaUk47335065 = -340703572;    int FVwaxAUEQlLaaUk12508302 = -50661583;    int FVwaxAUEQlLaaUk80127378 = -11525028;    int FVwaxAUEQlLaaUk79170292 = -488231084;    int FVwaxAUEQlLaaUk82433311 = -900375834;    int FVwaxAUEQlLaaUk66430400 = -99579894;    int FVwaxAUEQlLaaUk70006430 = -595881428;    int FVwaxAUEQlLaaUk49315588 = -810700627;    int FVwaxAUEQlLaaUk78106901 = 40800135;    int FVwaxAUEQlLaaUk40853057 = -513264768;    int FVwaxAUEQlLaaUk19710866 = -225782496;    int FVwaxAUEQlLaaUk53149743 = -307325802;    int FVwaxAUEQlLaaUk88961840 = -895830196;    int FVwaxAUEQlLaaUk42398119 = -674125019;    int FVwaxAUEQlLaaUk3976307 = -163095369;    int FVwaxAUEQlLaaUk8602771 = -295659427;    int FVwaxAUEQlLaaUk40956665 = -967558433;    int FVwaxAUEQlLaaUk94280127 = -996231514;    int FVwaxAUEQlLaaUk4906440 = -899948929;    int FVwaxAUEQlLaaUk21668208 = -829058673;    int FVwaxAUEQlLaaUk19795386 = -702807138;    int FVwaxAUEQlLaaUk65203478 = -906374082;    int FVwaxAUEQlLaaUk51605220 = -823678427;    int FVwaxAUEQlLaaUk71637905 = -555997041;    int FVwaxAUEQlLaaUk42086726 = -378689831;    int FVwaxAUEQlLaaUk10907939 = -899239499;    int FVwaxAUEQlLaaUk71288009 = -268201452;    int FVwaxAUEQlLaaUk31679814 = -131742737;    int FVwaxAUEQlLaaUk56333206 = -644954199;    int FVwaxAUEQlLaaUk93033921 = -428264818;    int FVwaxAUEQlLaaUk6447303 = -141707674;    int FVwaxAUEQlLaaUk40461418 = -522975573;    int FVwaxAUEQlLaaUk78457624 = -134674726;    int FVwaxAUEQlLaaUk59800526 = -937844246;    int FVwaxAUEQlLaaUk92082900 = -477252181;    int FVwaxAUEQlLaaUk54198323 = -899463956;    int FVwaxAUEQlLaaUk53966099 = -546249247;    int FVwaxAUEQlLaaUk5756546 = -22617703;    int FVwaxAUEQlLaaUk43436511 = -80586453;    int FVwaxAUEQlLaaUk33885970 = 35331601;    int FVwaxAUEQlLaaUk4009450 = -96596602;    int FVwaxAUEQlLaaUk17375596 = -614748316;    int FVwaxAUEQlLaaUk53455300 = -212812751;    int FVwaxAUEQlLaaUk53672520 = -242282355;    int FVwaxAUEQlLaaUk72618415 = -564753062;    int FVwaxAUEQlLaaUk34628158 = -666426547;    int FVwaxAUEQlLaaUk15464806 = -603926920;    int FVwaxAUEQlLaaUk40176580 = -570472213;    int FVwaxAUEQlLaaUk96302430 = 54117889;    int FVwaxAUEQlLaaUk63693414 = -812378576;     FVwaxAUEQlLaaUk20293108 = FVwaxAUEQlLaaUk1964982;     FVwaxAUEQlLaaUk1964982 = FVwaxAUEQlLaaUk77944978;     FVwaxAUEQlLaaUk77944978 = FVwaxAUEQlLaaUk29153981;     FVwaxAUEQlLaaUk29153981 = FVwaxAUEQlLaaUk29162155;     FVwaxAUEQlLaaUk29162155 = FVwaxAUEQlLaaUk53308228;     FVwaxAUEQlLaaUk53308228 = FVwaxAUEQlLaaUk65552527;     FVwaxAUEQlLaaUk65552527 = FVwaxAUEQlLaaUk15226199;     FVwaxAUEQlLaaUk15226199 = FVwaxAUEQlLaaUk81816083;     FVwaxAUEQlLaaUk81816083 = FVwaxAUEQlLaaUk70682931;     FVwaxAUEQlLaaUk70682931 = FVwaxAUEQlLaaUk42696247;     FVwaxAUEQlLaaUk42696247 = FVwaxAUEQlLaaUk25883503;     FVwaxAUEQlLaaUk25883503 = FVwaxAUEQlLaaUk20149949;     FVwaxAUEQlLaaUk20149949 = FVwaxAUEQlLaaUk5222810;     FVwaxAUEQlLaaUk5222810 = FVwaxAUEQlLaaUk95684087;     FVwaxAUEQlLaaUk95684087 = FVwaxAUEQlLaaUk86486569;     FVwaxAUEQlLaaUk86486569 = FVwaxAUEQlLaaUk96765679;     FVwaxAUEQlLaaUk96765679 = FVwaxAUEQlLaaUk46130798;     FVwaxAUEQlLaaUk46130798 = FVwaxAUEQlLaaUk63967186;     FVwaxAUEQlLaaUk63967186 = FVwaxAUEQlLaaUk29864626;     FVwaxAUEQlLaaUk29864626 = FVwaxAUEQlLaaUk15334586;     FVwaxAUEQlLaaUk15334586 = FVwaxAUEQlLaaUk53862707;     FVwaxAUEQlLaaUk53862707 = FVwaxAUEQlLaaUk31958551;     FVwaxAUEQlLaaUk31958551 = FVwaxAUEQlLaaUk28629390;     FVwaxAUEQlLaaUk28629390 = FVwaxAUEQlLaaUk51047079;     FVwaxAUEQlLaaUk51047079 = FVwaxAUEQlLaaUk88309098;     FVwaxAUEQlLaaUk88309098 = FVwaxAUEQlLaaUk33597363;     FVwaxAUEQlLaaUk33597363 = FVwaxAUEQlLaaUk12402785;     FVwaxAUEQlLaaUk12402785 = FVwaxAUEQlLaaUk26264359;     FVwaxAUEQlLaaUk26264359 = FVwaxAUEQlLaaUk39417965;     FVwaxAUEQlLaaUk39417965 = FVwaxAUEQlLaaUk66706625;     FVwaxAUEQlLaaUk66706625 = FVwaxAUEQlLaaUk34093477;     FVwaxAUEQlLaaUk34093477 = FVwaxAUEQlLaaUk84926837;     FVwaxAUEQlLaaUk84926837 = FVwaxAUEQlLaaUk25869822;     FVwaxAUEQlLaaUk25869822 = FVwaxAUEQlLaaUk316371;     FVwaxAUEQlLaaUk316371 = FVwaxAUEQlLaaUk74015880;     FVwaxAUEQlLaaUk74015880 = FVwaxAUEQlLaaUk66691184;     FVwaxAUEQlLaaUk66691184 = FVwaxAUEQlLaaUk31562202;     FVwaxAUEQlLaaUk31562202 = FVwaxAUEQlLaaUk94525577;     FVwaxAUEQlLaaUk94525577 = FVwaxAUEQlLaaUk92329280;     FVwaxAUEQlLaaUk92329280 = FVwaxAUEQlLaaUk87777900;     FVwaxAUEQlLaaUk87777900 = FVwaxAUEQlLaaUk4426647;     FVwaxAUEQlLaaUk4426647 = FVwaxAUEQlLaaUk82574698;     FVwaxAUEQlLaaUk82574698 = FVwaxAUEQlLaaUk278738;     FVwaxAUEQlLaaUk278738 = FVwaxAUEQlLaaUk72296184;     FVwaxAUEQlLaaUk72296184 = FVwaxAUEQlLaaUk58013158;     FVwaxAUEQlLaaUk58013158 = FVwaxAUEQlLaaUk81861796;     FVwaxAUEQlLaaUk81861796 = FVwaxAUEQlLaaUk93135944;     FVwaxAUEQlLaaUk93135944 = FVwaxAUEQlLaaUk33945161;     FVwaxAUEQlLaaUk33945161 = FVwaxAUEQlLaaUk66463832;     FVwaxAUEQlLaaUk66463832 = FVwaxAUEQlLaaUk47335065;     FVwaxAUEQlLaaUk47335065 = FVwaxAUEQlLaaUk12508302;     FVwaxAUEQlLaaUk12508302 = FVwaxAUEQlLaaUk80127378;     FVwaxAUEQlLaaUk80127378 = FVwaxAUEQlLaaUk79170292;     FVwaxAUEQlLaaUk79170292 = FVwaxAUEQlLaaUk82433311;     FVwaxAUEQlLaaUk82433311 = FVwaxAUEQlLaaUk66430400;     FVwaxAUEQlLaaUk66430400 = FVwaxAUEQlLaaUk70006430;     FVwaxAUEQlLaaUk70006430 = FVwaxAUEQlLaaUk49315588;     FVwaxAUEQlLaaUk49315588 = FVwaxAUEQlLaaUk78106901;     FVwaxAUEQlLaaUk78106901 = FVwaxAUEQlLaaUk40853057;     FVwaxAUEQlLaaUk40853057 = FVwaxAUEQlLaaUk19710866;     FVwaxAUEQlLaaUk19710866 = FVwaxAUEQlLaaUk53149743;     FVwaxAUEQlLaaUk53149743 = FVwaxAUEQlLaaUk88961840;     FVwaxAUEQlLaaUk88961840 = FVwaxAUEQlLaaUk42398119;     FVwaxAUEQlLaaUk42398119 = FVwaxAUEQlLaaUk3976307;     FVwaxAUEQlLaaUk3976307 = FVwaxAUEQlLaaUk8602771;     FVwaxAUEQlLaaUk8602771 = FVwaxAUEQlLaaUk40956665;     FVwaxAUEQlLaaUk40956665 = FVwaxAUEQlLaaUk94280127;     FVwaxAUEQlLaaUk94280127 = FVwaxAUEQlLaaUk4906440;     FVwaxAUEQlLaaUk4906440 = FVwaxAUEQlLaaUk21668208;     FVwaxAUEQlLaaUk21668208 = FVwaxAUEQlLaaUk19795386;     FVwaxAUEQlLaaUk19795386 = FVwaxAUEQlLaaUk65203478;     FVwaxAUEQlLaaUk65203478 = FVwaxAUEQlLaaUk51605220;     FVwaxAUEQlLaaUk51605220 = FVwaxAUEQlLaaUk71637905;     FVwaxAUEQlLaaUk71637905 = FVwaxAUEQlLaaUk42086726;     FVwaxAUEQlLaaUk42086726 = FVwaxAUEQlLaaUk10907939;     FVwaxAUEQlLaaUk10907939 = FVwaxAUEQlLaaUk71288009;     FVwaxAUEQlLaaUk71288009 = FVwaxAUEQlLaaUk31679814;     FVwaxAUEQlLaaUk31679814 = FVwaxAUEQlLaaUk56333206;     FVwaxAUEQlLaaUk56333206 = FVwaxAUEQlLaaUk93033921;     FVwaxAUEQlLaaUk93033921 = FVwaxAUEQlLaaUk6447303;     FVwaxAUEQlLaaUk6447303 = FVwaxAUEQlLaaUk40461418;     FVwaxAUEQlLaaUk40461418 = FVwaxAUEQlLaaUk78457624;     FVwaxAUEQlLaaUk78457624 = FVwaxAUEQlLaaUk59800526;     FVwaxAUEQlLaaUk59800526 = FVwaxAUEQlLaaUk92082900;     FVwaxAUEQlLaaUk92082900 = FVwaxAUEQlLaaUk54198323;     FVwaxAUEQlLaaUk54198323 = FVwaxAUEQlLaaUk53966099;     FVwaxAUEQlLaaUk53966099 = FVwaxAUEQlLaaUk5756546;     FVwaxAUEQlLaaUk5756546 = FVwaxAUEQlLaaUk43436511;     FVwaxAUEQlLaaUk43436511 = FVwaxAUEQlLaaUk33885970;     FVwaxAUEQlLaaUk33885970 = FVwaxAUEQlLaaUk4009450;     FVwaxAUEQlLaaUk4009450 = FVwaxAUEQlLaaUk17375596;     FVwaxAUEQlLaaUk17375596 = FVwaxAUEQlLaaUk53455300;     FVwaxAUEQlLaaUk53455300 = FVwaxAUEQlLaaUk53672520;     FVwaxAUEQlLaaUk53672520 = FVwaxAUEQlLaaUk72618415;     FVwaxAUEQlLaaUk72618415 = FVwaxAUEQlLaaUk34628158;     FVwaxAUEQlLaaUk34628158 = FVwaxAUEQlLaaUk15464806;     FVwaxAUEQlLaaUk15464806 = FVwaxAUEQlLaaUk40176580;     FVwaxAUEQlLaaUk40176580 = FVwaxAUEQlLaaUk96302430;     FVwaxAUEQlLaaUk96302430 = FVwaxAUEQlLaaUk63693414;     FVwaxAUEQlLaaUk63693414 = FVwaxAUEQlLaaUk20293108;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lwfRKhlRhWDOagFGbslDvHCjSGIUB99957235() {     int PiOnGsTCjcaXgis16806029 = -811179265;    int PiOnGsTCjcaXgis17047807 = 10710054;    int PiOnGsTCjcaXgis33627289 = -877578222;    int PiOnGsTCjcaXgis41702829 = -233767312;    int PiOnGsTCjcaXgis97814545 = -42234082;    int PiOnGsTCjcaXgis90555100 = -127080120;    int PiOnGsTCjcaXgis39367181 = -494984077;    int PiOnGsTCjcaXgis50678369 = 80500016;    int PiOnGsTCjcaXgis86070340 = -844498524;    int PiOnGsTCjcaXgis24687468 = -971946119;    int PiOnGsTCjcaXgis2912363 = 71331062;    int PiOnGsTCjcaXgis40899940 = -47140124;    int PiOnGsTCjcaXgis25420647 = -262979463;    int PiOnGsTCjcaXgis72297923 = -573718756;    int PiOnGsTCjcaXgis583845 = -472863413;    int PiOnGsTCjcaXgis99443393 = -986877295;    int PiOnGsTCjcaXgis24456528 = -91233741;    int PiOnGsTCjcaXgis82266285 = -196751615;    int PiOnGsTCjcaXgis45890162 = -935659898;    int PiOnGsTCjcaXgis70589885 = -361967010;    int PiOnGsTCjcaXgis23013441 = -609546212;    int PiOnGsTCjcaXgis94764464 = -990816376;    int PiOnGsTCjcaXgis21455514 = -721639926;    int PiOnGsTCjcaXgis52454161 = -177892234;    int PiOnGsTCjcaXgis29322087 = -331179416;    int PiOnGsTCjcaXgis64333463 = -52677581;    int PiOnGsTCjcaXgis81494047 = -697951896;    int PiOnGsTCjcaXgis4653148 = -678496399;    int PiOnGsTCjcaXgis8460330 = -876163587;    int PiOnGsTCjcaXgis39672472 = -282732595;    int PiOnGsTCjcaXgis75800704 = -252093691;    int PiOnGsTCjcaXgis69843660 = -989780295;    int PiOnGsTCjcaXgis2371277 = -500945554;    int PiOnGsTCjcaXgis38529490 = -901472286;    int PiOnGsTCjcaXgis9801209 = -378913818;    int PiOnGsTCjcaXgis17687134 = 30445262;    int PiOnGsTCjcaXgis47248151 = 64127100;    int PiOnGsTCjcaXgis4689176 = -93144584;    int PiOnGsTCjcaXgis43259884 = -96615874;    int PiOnGsTCjcaXgis71846680 = -326883439;    int PiOnGsTCjcaXgis20048895 = -505294519;    int PiOnGsTCjcaXgis48619998 = 92801921;    int PiOnGsTCjcaXgis70116634 = -740869803;    int PiOnGsTCjcaXgis12486846 = -350611233;    int PiOnGsTCjcaXgis38282899 = -5974969;    int PiOnGsTCjcaXgis55694185 = -609816635;    int PiOnGsTCjcaXgis78773889 = 89804706;    int PiOnGsTCjcaXgis73625967 = -410888476;    int PiOnGsTCjcaXgis52554229 = -319569770;    int PiOnGsTCjcaXgis39264871 = -322480813;    int PiOnGsTCjcaXgis99123704 = -970082778;    int PiOnGsTCjcaXgis84916940 = -632826833;    int PiOnGsTCjcaXgis72270756 = -803433673;    int PiOnGsTCjcaXgis64677487 = -938432088;    int PiOnGsTCjcaXgis76943794 = -233097626;    int PiOnGsTCjcaXgis22041564 = -820362890;    int PiOnGsTCjcaXgis95592293 = -267650020;    int PiOnGsTCjcaXgis81173128 = -599685988;    int PiOnGsTCjcaXgis12380742 = -902587896;    int PiOnGsTCjcaXgis33481082 = -989556502;    int PiOnGsTCjcaXgis9061054 = -429128225;    int PiOnGsTCjcaXgis34714033 = -816487678;    int PiOnGsTCjcaXgis42218039 = -43336398;    int PiOnGsTCjcaXgis46397868 = -461765930;    int PiOnGsTCjcaXgis48886763 = -619852428;    int PiOnGsTCjcaXgis33068703 = 61111356;    int PiOnGsTCjcaXgis38528664 = -546194571;    int PiOnGsTCjcaXgis86891156 = -361507178;    int PiOnGsTCjcaXgis62496714 = -94804938;    int PiOnGsTCjcaXgis82896711 = -403308675;    int PiOnGsTCjcaXgis52195242 = -951004396;    int PiOnGsTCjcaXgis19767353 = -998089158;    int PiOnGsTCjcaXgis39006402 = -135742;    int PiOnGsTCjcaXgis74043482 = -508776460;    int PiOnGsTCjcaXgis50540990 = -856672491;    int PiOnGsTCjcaXgis74393442 = -602348134;    int PiOnGsTCjcaXgis24647831 = -149946573;    int PiOnGsTCjcaXgis8968669 = -271028694;    int PiOnGsTCjcaXgis14171262 = -71917266;    int PiOnGsTCjcaXgis73627901 = -721362782;    int PiOnGsTCjcaXgis85559574 = -42482288;    int PiOnGsTCjcaXgis7868080 = -187063420;    int PiOnGsTCjcaXgis52098919 = -258926630;    int PiOnGsTCjcaXgis69195458 = -453682774;    int PiOnGsTCjcaXgis40548768 = -312649817;    int PiOnGsTCjcaXgis90883764 = -619266859;    int PiOnGsTCjcaXgis97572903 = -86346623;    int PiOnGsTCjcaXgis37693789 = -562513467;    int PiOnGsTCjcaXgis61585695 = -568374660;    int PiOnGsTCjcaXgis87759645 = -558550929;    int PiOnGsTCjcaXgis22094840 = -701904719;    int PiOnGsTCjcaXgis66075022 = -336186912;    int PiOnGsTCjcaXgis92308433 = -190556688;    int PiOnGsTCjcaXgis9778802 = -107059373;    int PiOnGsTCjcaXgis62785626 = -897755214;    int PiOnGsTCjcaXgis85334862 = -688806841;    int PiOnGsTCjcaXgis6401960 = -863861682;    int PiOnGsTCjcaXgis23718767 = -179103873;    int PiOnGsTCjcaXgis63600082 = -730758805;    int PiOnGsTCjcaXgis5214197 = -811179265;     PiOnGsTCjcaXgis16806029 = PiOnGsTCjcaXgis17047807;     PiOnGsTCjcaXgis17047807 = PiOnGsTCjcaXgis33627289;     PiOnGsTCjcaXgis33627289 = PiOnGsTCjcaXgis41702829;     PiOnGsTCjcaXgis41702829 = PiOnGsTCjcaXgis97814545;     PiOnGsTCjcaXgis97814545 = PiOnGsTCjcaXgis90555100;     PiOnGsTCjcaXgis90555100 = PiOnGsTCjcaXgis39367181;     PiOnGsTCjcaXgis39367181 = PiOnGsTCjcaXgis50678369;     PiOnGsTCjcaXgis50678369 = PiOnGsTCjcaXgis86070340;     PiOnGsTCjcaXgis86070340 = PiOnGsTCjcaXgis24687468;     PiOnGsTCjcaXgis24687468 = PiOnGsTCjcaXgis2912363;     PiOnGsTCjcaXgis2912363 = PiOnGsTCjcaXgis40899940;     PiOnGsTCjcaXgis40899940 = PiOnGsTCjcaXgis25420647;     PiOnGsTCjcaXgis25420647 = PiOnGsTCjcaXgis72297923;     PiOnGsTCjcaXgis72297923 = PiOnGsTCjcaXgis583845;     PiOnGsTCjcaXgis583845 = PiOnGsTCjcaXgis99443393;     PiOnGsTCjcaXgis99443393 = PiOnGsTCjcaXgis24456528;     PiOnGsTCjcaXgis24456528 = PiOnGsTCjcaXgis82266285;     PiOnGsTCjcaXgis82266285 = PiOnGsTCjcaXgis45890162;     PiOnGsTCjcaXgis45890162 = PiOnGsTCjcaXgis70589885;     PiOnGsTCjcaXgis70589885 = PiOnGsTCjcaXgis23013441;     PiOnGsTCjcaXgis23013441 = PiOnGsTCjcaXgis94764464;     PiOnGsTCjcaXgis94764464 = PiOnGsTCjcaXgis21455514;     PiOnGsTCjcaXgis21455514 = PiOnGsTCjcaXgis52454161;     PiOnGsTCjcaXgis52454161 = PiOnGsTCjcaXgis29322087;     PiOnGsTCjcaXgis29322087 = PiOnGsTCjcaXgis64333463;     PiOnGsTCjcaXgis64333463 = PiOnGsTCjcaXgis81494047;     PiOnGsTCjcaXgis81494047 = PiOnGsTCjcaXgis4653148;     PiOnGsTCjcaXgis4653148 = PiOnGsTCjcaXgis8460330;     PiOnGsTCjcaXgis8460330 = PiOnGsTCjcaXgis39672472;     PiOnGsTCjcaXgis39672472 = PiOnGsTCjcaXgis75800704;     PiOnGsTCjcaXgis75800704 = PiOnGsTCjcaXgis69843660;     PiOnGsTCjcaXgis69843660 = PiOnGsTCjcaXgis2371277;     PiOnGsTCjcaXgis2371277 = PiOnGsTCjcaXgis38529490;     PiOnGsTCjcaXgis38529490 = PiOnGsTCjcaXgis9801209;     PiOnGsTCjcaXgis9801209 = PiOnGsTCjcaXgis17687134;     PiOnGsTCjcaXgis17687134 = PiOnGsTCjcaXgis47248151;     PiOnGsTCjcaXgis47248151 = PiOnGsTCjcaXgis4689176;     PiOnGsTCjcaXgis4689176 = PiOnGsTCjcaXgis43259884;     PiOnGsTCjcaXgis43259884 = PiOnGsTCjcaXgis71846680;     PiOnGsTCjcaXgis71846680 = PiOnGsTCjcaXgis20048895;     PiOnGsTCjcaXgis20048895 = PiOnGsTCjcaXgis48619998;     PiOnGsTCjcaXgis48619998 = PiOnGsTCjcaXgis70116634;     PiOnGsTCjcaXgis70116634 = PiOnGsTCjcaXgis12486846;     PiOnGsTCjcaXgis12486846 = PiOnGsTCjcaXgis38282899;     PiOnGsTCjcaXgis38282899 = PiOnGsTCjcaXgis55694185;     PiOnGsTCjcaXgis55694185 = PiOnGsTCjcaXgis78773889;     PiOnGsTCjcaXgis78773889 = PiOnGsTCjcaXgis73625967;     PiOnGsTCjcaXgis73625967 = PiOnGsTCjcaXgis52554229;     PiOnGsTCjcaXgis52554229 = PiOnGsTCjcaXgis39264871;     PiOnGsTCjcaXgis39264871 = PiOnGsTCjcaXgis99123704;     PiOnGsTCjcaXgis99123704 = PiOnGsTCjcaXgis84916940;     PiOnGsTCjcaXgis84916940 = PiOnGsTCjcaXgis72270756;     PiOnGsTCjcaXgis72270756 = PiOnGsTCjcaXgis64677487;     PiOnGsTCjcaXgis64677487 = PiOnGsTCjcaXgis76943794;     PiOnGsTCjcaXgis76943794 = PiOnGsTCjcaXgis22041564;     PiOnGsTCjcaXgis22041564 = PiOnGsTCjcaXgis95592293;     PiOnGsTCjcaXgis95592293 = PiOnGsTCjcaXgis81173128;     PiOnGsTCjcaXgis81173128 = PiOnGsTCjcaXgis12380742;     PiOnGsTCjcaXgis12380742 = PiOnGsTCjcaXgis33481082;     PiOnGsTCjcaXgis33481082 = PiOnGsTCjcaXgis9061054;     PiOnGsTCjcaXgis9061054 = PiOnGsTCjcaXgis34714033;     PiOnGsTCjcaXgis34714033 = PiOnGsTCjcaXgis42218039;     PiOnGsTCjcaXgis42218039 = PiOnGsTCjcaXgis46397868;     PiOnGsTCjcaXgis46397868 = PiOnGsTCjcaXgis48886763;     PiOnGsTCjcaXgis48886763 = PiOnGsTCjcaXgis33068703;     PiOnGsTCjcaXgis33068703 = PiOnGsTCjcaXgis38528664;     PiOnGsTCjcaXgis38528664 = PiOnGsTCjcaXgis86891156;     PiOnGsTCjcaXgis86891156 = PiOnGsTCjcaXgis62496714;     PiOnGsTCjcaXgis62496714 = PiOnGsTCjcaXgis82896711;     PiOnGsTCjcaXgis82896711 = PiOnGsTCjcaXgis52195242;     PiOnGsTCjcaXgis52195242 = PiOnGsTCjcaXgis19767353;     PiOnGsTCjcaXgis19767353 = PiOnGsTCjcaXgis39006402;     PiOnGsTCjcaXgis39006402 = PiOnGsTCjcaXgis74043482;     PiOnGsTCjcaXgis74043482 = PiOnGsTCjcaXgis50540990;     PiOnGsTCjcaXgis50540990 = PiOnGsTCjcaXgis74393442;     PiOnGsTCjcaXgis74393442 = PiOnGsTCjcaXgis24647831;     PiOnGsTCjcaXgis24647831 = PiOnGsTCjcaXgis8968669;     PiOnGsTCjcaXgis8968669 = PiOnGsTCjcaXgis14171262;     PiOnGsTCjcaXgis14171262 = PiOnGsTCjcaXgis73627901;     PiOnGsTCjcaXgis73627901 = PiOnGsTCjcaXgis85559574;     PiOnGsTCjcaXgis85559574 = PiOnGsTCjcaXgis7868080;     PiOnGsTCjcaXgis7868080 = PiOnGsTCjcaXgis52098919;     PiOnGsTCjcaXgis52098919 = PiOnGsTCjcaXgis69195458;     PiOnGsTCjcaXgis69195458 = PiOnGsTCjcaXgis40548768;     PiOnGsTCjcaXgis40548768 = PiOnGsTCjcaXgis90883764;     PiOnGsTCjcaXgis90883764 = PiOnGsTCjcaXgis97572903;     PiOnGsTCjcaXgis97572903 = PiOnGsTCjcaXgis37693789;     PiOnGsTCjcaXgis37693789 = PiOnGsTCjcaXgis61585695;     PiOnGsTCjcaXgis61585695 = PiOnGsTCjcaXgis87759645;     PiOnGsTCjcaXgis87759645 = PiOnGsTCjcaXgis22094840;     PiOnGsTCjcaXgis22094840 = PiOnGsTCjcaXgis66075022;     PiOnGsTCjcaXgis66075022 = PiOnGsTCjcaXgis92308433;     PiOnGsTCjcaXgis92308433 = PiOnGsTCjcaXgis9778802;     PiOnGsTCjcaXgis9778802 = PiOnGsTCjcaXgis62785626;     PiOnGsTCjcaXgis62785626 = PiOnGsTCjcaXgis85334862;     PiOnGsTCjcaXgis85334862 = PiOnGsTCjcaXgis6401960;     PiOnGsTCjcaXgis6401960 = PiOnGsTCjcaXgis23718767;     PiOnGsTCjcaXgis23718767 = PiOnGsTCjcaXgis63600082;     PiOnGsTCjcaXgis63600082 = PiOnGsTCjcaXgis5214197;     PiOnGsTCjcaXgis5214197 = PiOnGsTCjcaXgis16806029;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nUDzDSOlHxRLkMLJpYYiAqKAoEesN2146574() {     int TQheoWtlxfwcuAy13318949 = -809979953;    int TQheoWtlxfwcuAy32130633 = 58021752;    int TQheoWtlxfwcuAy89309600 = -625680988;    int TQheoWtlxfwcuAy54251677 = -702581102;    int TQheoWtlxfwcuAy66466935 = -697821078;    int TQheoWtlxfwcuAy27801973 = -157607697;    int TQheoWtlxfwcuAy13181834 = -659902052;    int TQheoWtlxfwcuAy86130539 = -32384168;    int TQheoWtlxfwcuAy90324597 = 3083722;    int TQheoWtlxfwcuAy78692004 = -630671332;    int TQheoWtlxfwcuAy63128478 = 96095825;    int TQheoWtlxfwcuAy55916377 = -615873030;    int TQheoWtlxfwcuAy30691345 = -548765126;    int TQheoWtlxfwcuAy39373036 = 16759709;    int TQheoWtlxfwcuAy5483603 = -324190123;    int TQheoWtlxfwcuAy12400217 = -745498510;    int TQheoWtlxfwcuAy52147377 = -4080786;    int TQheoWtlxfwcuAy18401774 = -814277681;    int TQheoWtlxfwcuAy27813138 = -324787198;    int TQheoWtlxfwcuAy11315145 = -271491840;    int TQheoWtlxfwcuAy30692296 = -820095811;    int TQheoWtlxfwcuAy35666222 = -268834068;    int TQheoWtlxfwcuAy10952477 = 97440363;    int TQheoWtlxfwcuAy76278931 = -137009639;    int TQheoWtlxfwcuAy7597094 = -956605176;    int TQheoWtlxfwcuAy40357828 = -231972844;    int TQheoWtlxfwcuAy29390732 = -525133743;    int TQheoWtlxfwcuAy96903509 = -334252498;    int TQheoWtlxfwcuAy90656301 = -741541570;    int TQheoWtlxfwcuAy39926980 = -747509437;    int TQheoWtlxfwcuAy84894784 = -554061845;    int TQheoWtlxfwcuAy5593844 = -221786315;    int TQheoWtlxfwcuAy19815715 = -391042321;    int TQheoWtlxfwcuAy51189158 = -721982285;    int TQheoWtlxfwcuAy19286047 = -593579344;    int TQheoWtlxfwcuAy61358387 = -246631447;    int TQheoWtlxfwcuAy27805118 = -546296858;    int TQheoWtlxfwcuAy77816150 = 85723447;    int TQheoWtlxfwcuAy91994190 = -437684624;    int TQheoWtlxfwcuAy51364079 = -863231321;    int TQheoWtlxfwcuAy52319890 = 63163310;    int TQheoWtlxfwcuAy92813350 = -414639041;    int TQheoWtlxfwcuAy57658570 = -137142375;    int TQheoWtlxfwcuAy24694954 = -492244987;    int TQheoWtlxfwcuAy4269614 = -538129307;    int TQheoWtlxfwcuAy53375212 = -942144431;    int TQheoWtlxfwcuAy75685981 = -188715942;    int TQheoWtlxfwcuAy54115990 = -573982475;    int TQheoWtlxfwcuAy71163296 = -951073965;    int TQheoWtlxfwcuAy12065911 = -672020269;    int TQheoWtlxfwcuAy50912345 = -499461985;    int TQheoWtlxfwcuAy57325579 = -114992083;    int TQheoWtlxfwcuAy64414135 = -495342317;    int TQheoWtlxfwcuAy50184683 = -288633092;    int TQheoWtlxfwcuAy71454278 = -665819419;    int TQheoWtlxfwcuAy77652727 = -441145885;    int TQheoWtlxfwcuAy21178157 = 60581388;    int TQheoWtlxfwcuAy13030670 = -388671350;    int TQheoWtlxfwcuAy46654583 = -745975927;    int TQheoWtlxfwcuAy26109108 = -365848235;    int TQheoWtlxfwcuAy98411240 = -632473955;    int TQheoWtlxfwcuAy16278324 = -225649554;    int TQheoWtlxfwcuAy95474238 = -290842599;    int TQheoWtlxfwcuAy50397617 = -249406842;    int TQheoWtlxfwcuAy93797220 = 23390512;    int TQheoWtlxfwcuAy57534635 = -682117860;    int TQheoWtlxfwcuAy36100662 = -124830709;    int TQheoWtlxfwcuAy79502186 = -826782842;    int TQheoWtlxfwcuAy20086990 = -389660948;    int TQheoWtlxfwcuAy44125215 = 22441323;    int TQheoWtlxfwcuAy84595099 = -99201653;    int TQheoWtlxfwcuAy74331226 = 10195766;    int TQheoWtlxfwcuAy26407583 = -276593057;    int TQheoWtlxfwcuAy76449059 = -461555878;    int TQheoWtlxfwcuAy58995254 = -234655151;    int TQheoWtlxfwcuAy37878945 = -305456770;    int TQheoWtlxfwcuAy78007652 = -31691694;    int TQheoWtlxfwcuAy86257522 = -410314651;    int TQheoWtlxfwcuAy72009317 = -598880333;    int TQheoWtlxfwcuAy54221882 = 85539255;    int TQheoWtlxfwcuAy64671846 = 56743097;    int TQheoWtlxfwcuAy75274741 = -951151268;    int TQheoWtlxfwcuAy25740214 = -383178534;    int TQheoWtlxfwcuAy78590391 = 30478698;    int TQheoWtlxfwcuAy89014634 = -148047453;    int TQheoWtlxfwcuAy27569206 = -339069762;    int TQheoWtlxfwcuAy41179708 = -726443999;    int TQheoWtlxfwcuAy69631031 = -2409230;    int TQheoWtlxfwcuAy79734880 = 43837133;    int TQheoWtlxfwcuAy41633320 = -52433459;    int TQheoWtlxfwcuAy40180231 = -207212835;    int TQheoWtlxfwcuAy14774449 = -57625508;    int TQheoWtlxfwcuAy31161567 = -168300626;    int TQheoWtlxfwcuAy65885083 = 28163610;    int TQheoWtlxfwcuAy52952838 = -130757367;    int TQheoWtlxfwcuAy36041567 = -711187136;    int TQheoWtlxfwcuAy97339112 = -23796443;    int TQheoWtlxfwcuAy7260954 = -887735534;    int TQheoWtlxfwcuAy30897734 = -415635499;    int TQheoWtlxfwcuAy46734979 = -809979953;     TQheoWtlxfwcuAy13318949 = TQheoWtlxfwcuAy32130633;     TQheoWtlxfwcuAy32130633 = TQheoWtlxfwcuAy89309600;     TQheoWtlxfwcuAy89309600 = TQheoWtlxfwcuAy54251677;     TQheoWtlxfwcuAy54251677 = TQheoWtlxfwcuAy66466935;     TQheoWtlxfwcuAy66466935 = TQheoWtlxfwcuAy27801973;     TQheoWtlxfwcuAy27801973 = TQheoWtlxfwcuAy13181834;     TQheoWtlxfwcuAy13181834 = TQheoWtlxfwcuAy86130539;     TQheoWtlxfwcuAy86130539 = TQheoWtlxfwcuAy90324597;     TQheoWtlxfwcuAy90324597 = TQheoWtlxfwcuAy78692004;     TQheoWtlxfwcuAy78692004 = TQheoWtlxfwcuAy63128478;     TQheoWtlxfwcuAy63128478 = TQheoWtlxfwcuAy55916377;     TQheoWtlxfwcuAy55916377 = TQheoWtlxfwcuAy30691345;     TQheoWtlxfwcuAy30691345 = TQheoWtlxfwcuAy39373036;     TQheoWtlxfwcuAy39373036 = TQheoWtlxfwcuAy5483603;     TQheoWtlxfwcuAy5483603 = TQheoWtlxfwcuAy12400217;     TQheoWtlxfwcuAy12400217 = TQheoWtlxfwcuAy52147377;     TQheoWtlxfwcuAy52147377 = TQheoWtlxfwcuAy18401774;     TQheoWtlxfwcuAy18401774 = TQheoWtlxfwcuAy27813138;     TQheoWtlxfwcuAy27813138 = TQheoWtlxfwcuAy11315145;     TQheoWtlxfwcuAy11315145 = TQheoWtlxfwcuAy30692296;     TQheoWtlxfwcuAy30692296 = TQheoWtlxfwcuAy35666222;     TQheoWtlxfwcuAy35666222 = TQheoWtlxfwcuAy10952477;     TQheoWtlxfwcuAy10952477 = TQheoWtlxfwcuAy76278931;     TQheoWtlxfwcuAy76278931 = TQheoWtlxfwcuAy7597094;     TQheoWtlxfwcuAy7597094 = TQheoWtlxfwcuAy40357828;     TQheoWtlxfwcuAy40357828 = TQheoWtlxfwcuAy29390732;     TQheoWtlxfwcuAy29390732 = TQheoWtlxfwcuAy96903509;     TQheoWtlxfwcuAy96903509 = TQheoWtlxfwcuAy90656301;     TQheoWtlxfwcuAy90656301 = TQheoWtlxfwcuAy39926980;     TQheoWtlxfwcuAy39926980 = TQheoWtlxfwcuAy84894784;     TQheoWtlxfwcuAy84894784 = TQheoWtlxfwcuAy5593844;     TQheoWtlxfwcuAy5593844 = TQheoWtlxfwcuAy19815715;     TQheoWtlxfwcuAy19815715 = TQheoWtlxfwcuAy51189158;     TQheoWtlxfwcuAy51189158 = TQheoWtlxfwcuAy19286047;     TQheoWtlxfwcuAy19286047 = TQheoWtlxfwcuAy61358387;     TQheoWtlxfwcuAy61358387 = TQheoWtlxfwcuAy27805118;     TQheoWtlxfwcuAy27805118 = TQheoWtlxfwcuAy77816150;     TQheoWtlxfwcuAy77816150 = TQheoWtlxfwcuAy91994190;     TQheoWtlxfwcuAy91994190 = TQheoWtlxfwcuAy51364079;     TQheoWtlxfwcuAy51364079 = TQheoWtlxfwcuAy52319890;     TQheoWtlxfwcuAy52319890 = TQheoWtlxfwcuAy92813350;     TQheoWtlxfwcuAy92813350 = TQheoWtlxfwcuAy57658570;     TQheoWtlxfwcuAy57658570 = TQheoWtlxfwcuAy24694954;     TQheoWtlxfwcuAy24694954 = TQheoWtlxfwcuAy4269614;     TQheoWtlxfwcuAy4269614 = TQheoWtlxfwcuAy53375212;     TQheoWtlxfwcuAy53375212 = TQheoWtlxfwcuAy75685981;     TQheoWtlxfwcuAy75685981 = TQheoWtlxfwcuAy54115990;     TQheoWtlxfwcuAy54115990 = TQheoWtlxfwcuAy71163296;     TQheoWtlxfwcuAy71163296 = TQheoWtlxfwcuAy12065911;     TQheoWtlxfwcuAy12065911 = TQheoWtlxfwcuAy50912345;     TQheoWtlxfwcuAy50912345 = TQheoWtlxfwcuAy57325579;     TQheoWtlxfwcuAy57325579 = TQheoWtlxfwcuAy64414135;     TQheoWtlxfwcuAy64414135 = TQheoWtlxfwcuAy50184683;     TQheoWtlxfwcuAy50184683 = TQheoWtlxfwcuAy71454278;     TQheoWtlxfwcuAy71454278 = TQheoWtlxfwcuAy77652727;     TQheoWtlxfwcuAy77652727 = TQheoWtlxfwcuAy21178157;     TQheoWtlxfwcuAy21178157 = TQheoWtlxfwcuAy13030670;     TQheoWtlxfwcuAy13030670 = TQheoWtlxfwcuAy46654583;     TQheoWtlxfwcuAy46654583 = TQheoWtlxfwcuAy26109108;     TQheoWtlxfwcuAy26109108 = TQheoWtlxfwcuAy98411240;     TQheoWtlxfwcuAy98411240 = TQheoWtlxfwcuAy16278324;     TQheoWtlxfwcuAy16278324 = TQheoWtlxfwcuAy95474238;     TQheoWtlxfwcuAy95474238 = TQheoWtlxfwcuAy50397617;     TQheoWtlxfwcuAy50397617 = TQheoWtlxfwcuAy93797220;     TQheoWtlxfwcuAy93797220 = TQheoWtlxfwcuAy57534635;     TQheoWtlxfwcuAy57534635 = TQheoWtlxfwcuAy36100662;     TQheoWtlxfwcuAy36100662 = TQheoWtlxfwcuAy79502186;     TQheoWtlxfwcuAy79502186 = TQheoWtlxfwcuAy20086990;     TQheoWtlxfwcuAy20086990 = TQheoWtlxfwcuAy44125215;     TQheoWtlxfwcuAy44125215 = TQheoWtlxfwcuAy84595099;     TQheoWtlxfwcuAy84595099 = TQheoWtlxfwcuAy74331226;     TQheoWtlxfwcuAy74331226 = TQheoWtlxfwcuAy26407583;     TQheoWtlxfwcuAy26407583 = TQheoWtlxfwcuAy76449059;     TQheoWtlxfwcuAy76449059 = TQheoWtlxfwcuAy58995254;     TQheoWtlxfwcuAy58995254 = TQheoWtlxfwcuAy37878945;     TQheoWtlxfwcuAy37878945 = TQheoWtlxfwcuAy78007652;     TQheoWtlxfwcuAy78007652 = TQheoWtlxfwcuAy86257522;     TQheoWtlxfwcuAy86257522 = TQheoWtlxfwcuAy72009317;     TQheoWtlxfwcuAy72009317 = TQheoWtlxfwcuAy54221882;     TQheoWtlxfwcuAy54221882 = TQheoWtlxfwcuAy64671846;     TQheoWtlxfwcuAy64671846 = TQheoWtlxfwcuAy75274741;     TQheoWtlxfwcuAy75274741 = TQheoWtlxfwcuAy25740214;     TQheoWtlxfwcuAy25740214 = TQheoWtlxfwcuAy78590391;     TQheoWtlxfwcuAy78590391 = TQheoWtlxfwcuAy89014634;     TQheoWtlxfwcuAy89014634 = TQheoWtlxfwcuAy27569206;     TQheoWtlxfwcuAy27569206 = TQheoWtlxfwcuAy41179708;     TQheoWtlxfwcuAy41179708 = TQheoWtlxfwcuAy69631031;     TQheoWtlxfwcuAy69631031 = TQheoWtlxfwcuAy79734880;     TQheoWtlxfwcuAy79734880 = TQheoWtlxfwcuAy41633320;     TQheoWtlxfwcuAy41633320 = TQheoWtlxfwcuAy40180231;     TQheoWtlxfwcuAy40180231 = TQheoWtlxfwcuAy14774449;     TQheoWtlxfwcuAy14774449 = TQheoWtlxfwcuAy31161567;     TQheoWtlxfwcuAy31161567 = TQheoWtlxfwcuAy65885083;     TQheoWtlxfwcuAy65885083 = TQheoWtlxfwcuAy52952838;     TQheoWtlxfwcuAy52952838 = TQheoWtlxfwcuAy36041567;     TQheoWtlxfwcuAy36041567 = TQheoWtlxfwcuAy97339112;     TQheoWtlxfwcuAy97339112 = TQheoWtlxfwcuAy7260954;     TQheoWtlxfwcuAy7260954 = TQheoWtlxfwcuAy30897734;     TQheoWtlxfwcuAy30897734 = TQheoWtlxfwcuAy46734979;     TQheoWtlxfwcuAy46734979 = TQheoWtlxfwcuAy13318949;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void KUwUmmKeLfQNIOBxKBWLjnMiPRJdl52093381() {     int KIxXjQgzPKMHGjH80490008 = -454405630;    int KIxXjQgzPKMHGjH77314659 = -823714724;    int KIxXjQgzPKMHGjH23647618 = -858949600;    int KIxXjQgzPKMHGjH21651952 = -440992452;    int KIxXjQgzPKMHGjH47729545 = -668459100;    int KIxXjQgzPKMHGjH20243175 = -190283613;    int KIxXjQgzPKMHGjH36828047 = -19059270;    int KIxXjQgzPKMHGjH9211040 = -787908125;    int KIxXjQgzPKMHGjH57703744 = -629035291;    int KIxXjQgzPKMHGjH59023487 = -953685024;    int KIxXjQgzPKMHGjH45273149 = 45430731;    int KIxXjQgzPKMHGjH2187708 = -664891366;    int KIxXjQgzPKMHGjH83936515 = -722429374;    int KIxXjQgzPKMHGjH36450963 = 51363016;    int KIxXjQgzPKMHGjH47931270 = -362553984;    int KIxXjQgzPKMHGjH83321133 = -841307662;    int KIxXjQgzPKMHGjH17838293 = -6498390;    int KIxXjQgzPKMHGjH51830223 = 76101680;    int KIxXjQgzPKMHGjH71015626 = -903120174;    int KIxXjQgzPKMHGjH38753350 = -111358524;    int KIxXjQgzPKMHGjH13837481 = -128858175;    int KIxXjQgzPKMHGjH59667478 = -468145183;    int KIxXjQgzPKMHGjH65514851 = -292364650;    int KIxXjQgzPKMHGjH90303130 = -907607426;    int KIxXjQgzPKMHGjH21553800 = -831241770;    int KIxXjQgzPKMHGjH30405696 = -601346623;    int KIxXjQgzPKMHGjH15326621 = -446693652;    int KIxXjQgzPKMHGjH61532573 = -986165376;    int KIxXjQgzPKMHGjH92316313 = -255207455;    int KIxXjQgzPKMHGjH3263714 = -995249976;    int KIxXjQgzPKMHGjH67648181 = -217370449;    int KIxXjQgzPKMHGjH1131630 = -490267737;    int KIxXjQgzPKMHGjH34753092 = -26156482;    int KIxXjQgzPKMHGjH28930965 = -229340237;    int KIxXjQgzPKMHGjH24437084 = -395877724;    int KIxXjQgzPKMHGjH22194711 = -523168957;    int KIxXjQgzPKMHGjH40212203 = -145660645;    int KIxXjQgzPKMHGjH33817933 = -624258950;    int KIxXjQgzPKMHGjH83948189 = -21800595;    int KIxXjQgzPKMHGjH54111883 = -207755879;    int KIxXjQgzPKMHGjH56808501 = -764453840;    int KIxXjQgzPKMHGjH79792033 = -497676991;    int KIxXjQgzPKMHGjH33884106 = -61038745;    int KIxXjQgzPKMHGjH84179464 = -48889269;    int KIxXjQgzPKMHGjH82192335 = -905670637;    int KIxXjQgzPKMHGjH17034105 = -120764711;    int KIxXjQgzPKMHGjH71031485 = -102025767;    int KIxXjQgzPKMHGjH24836643 = -580445413;    int KIxXjQgzPKMHGjH76555427 = -904578268;    int KIxXjQgzPKMHGjH73249012 = -617543716;    int KIxXjQgzPKMHGjH57630940 = 3141695;    int KIxXjQgzPKMHGjH57058464 = -284231307;    int KIxXjQgzPKMHGjH36184822 = -222994995;    int KIxXjQgzPKMHGjH15668913 = -867795741;    int KIxXjQgzPKMHGjH38255900 = -352582848;    int KIxXjQgzPKMHGjH20822530 = -986260447;    int KIxXjQgzPKMHGjH11799808 = -431350075;    int KIxXjQgzPKMHGjH33344488 = -951342174;    int KIxXjQgzPKMHGjH98152 = -609750683;    int KIxXjQgzPKMHGjH17323849 = 32887523;    int KIxXjQgzPKMHGjH4916555 = -743589962;    int KIxXjQgzPKMHGjH75295473 = -32893894;    int KIxXjQgzPKMHGjH16894726 = -432700671;    int KIxXjQgzPKMHGjH54440030 = -633785315;    int KIxXjQgzPKMHGjH91375306 = -636314576;    int KIxXjQgzPKMHGjH44141519 = -464301533;    int KIxXjQgzPKMHGjH67434616 = -538734884;    int KIxXjQgzPKMHGjH55005551 = -393089138;    int KIxXjQgzPKMHGjH12013879 = -552759261;    int KIxXjQgzPKMHGjH25736560 = -839385028;    int KIxXjQgzPKMHGjH43108931 = -595647017;    int KIxXjQgzPKMHGjH84020359 = -382239441;    int KIxXjQgzPKMHGjH67882034 = -902097725;    int KIxXjQgzPKMHGjH16903743 = -595364295;    int KIxXjQgzPKMHGjH81944848 = -346904684;    int KIxXjQgzPKMHGjH34045448 = -631181185;    int KIxXjQgzPKMHGjH25783373 = -307106439;    int KIxXjQgzPKMHGjH81335386 = -143475382;    int KIxXjQgzPKMHGjH8110795 = 98063210;    int KIxXjQgzPKMHGjH4519696 = -610477059;    int KIxXjQgzPKMHGjH59374211 = -399320856;    int KIxXjQgzPKMHGjH90489977 = -866248239;    int KIxXjQgzPKMHGjH84977145 = 18412891;    int KIxXjQgzPKMHGjH19067301 = -637663739;    int KIxXjQgzPKMHGjH45632774 = -898391672;    int KIxXjQgzPKMHGjH10589718 = -933139142;    int KIxXjQgzPKMHGjH64946808 = -167272742;    int KIxXjQgzPKMHGjH19084179 = -158360742;    int KIxXjQgzPKMHGjH90675064 = -876757390;    int KIxXjQgzPKMHGjH3614555 = -409617277;    int KIxXjQgzPKMHGjH10394903 = 8181118;    int KIxXjQgzPKMHGjH6867715 = -194318472;    int KIxXjQgzPKMHGjH33719782 = 85491733;    int KIxXjQgzPKMHGjH66624341 = 45311882;    int KIxXjQgzPKMHGjH49195329 = -464165918;    int KIxXjQgzPKMHGjH81513027 = -631559947;    int KIxXjQgzPKMHGjH62897307 = 35023679;    int KIxXjQgzPKMHGjH79444075 = -427253431;    int KIxXjQgzPKMHGjH92804157 = -412574693;    int KIxXjQgzPKMHGjH38050816 = -454405630;     KIxXjQgzPKMHGjH80490008 = KIxXjQgzPKMHGjH77314659;     KIxXjQgzPKMHGjH77314659 = KIxXjQgzPKMHGjH23647618;     KIxXjQgzPKMHGjH23647618 = KIxXjQgzPKMHGjH21651952;     KIxXjQgzPKMHGjH21651952 = KIxXjQgzPKMHGjH47729545;     KIxXjQgzPKMHGjH47729545 = KIxXjQgzPKMHGjH20243175;     KIxXjQgzPKMHGjH20243175 = KIxXjQgzPKMHGjH36828047;     KIxXjQgzPKMHGjH36828047 = KIxXjQgzPKMHGjH9211040;     KIxXjQgzPKMHGjH9211040 = KIxXjQgzPKMHGjH57703744;     KIxXjQgzPKMHGjH57703744 = KIxXjQgzPKMHGjH59023487;     KIxXjQgzPKMHGjH59023487 = KIxXjQgzPKMHGjH45273149;     KIxXjQgzPKMHGjH45273149 = KIxXjQgzPKMHGjH2187708;     KIxXjQgzPKMHGjH2187708 = KIxXjQgzPKMHGjH83936515;     KIxXjQgzPKMHGjH83936515 = KIxXjQgzPKMHGjH36450963;     KIxXjQgzPKMHGjH36450963 = KIxXjQgzPKMHGjH47931270;     KIxXjQgzPKMHGjH47931270 = KIxXjQgzPKMHGjH83321133;     KIxXjQgzPKMHGjH83321133 = KIxXjQgzPKMHGjH17838293;     KIxXjQgzPKMHGjH17838293 = KIxXjQgzPKMHGjH51830223;     KIxXjQgzPKMHGjH51830223 = KIxXjQgzPKMHGjH71015626;     KIxXjQgzPKMHGjH71015626 = KIxXjQgzPKMHGjH38753350;     KIxXjQgzPKMHGjH38753350 = KIxXjQgzPKMHGjH13837481;     KIxXjQgzPKMHGjH13837481 = KIxXjQgzPKMHGjH59667478;     KIxXjQgzPKMHGjH59667478 = KIxXjQgzPKMHGjH65514851;     KIxXjQgzPKMHGjH65514851 = KIxXjQgzPKMHGjH90303130;     KIxXjQgzPKMHGjH90303130 = KIxXjQgzPKMHGjH21553800;     KIxXjQgzPKMHGjH21553800 = KIxXjQgzPKMHGjH30405696;     KIxXjQgzPKMHGjH30405696 = KIxXjQgzPKMHGjH15326621;     KIxXjQgzPKMHGjH15326621 = KIxXjQgzPKMHGjH61532573;     KIxXjQgzPKMHGjH61532573 = KIxXjQgzPKMHGjH92316313;     KIxXjQgzPKMHGjH92316313 = KIxXjQgzPKMHGjH3263714;     KIxXjQgzPKMHGjH3263714 = KIxXjQgzPKMHGjH67648181;     KIxXjQgzPKMHGjH67648181 = KIxXjQgzPKMHGjH1131630;     KIxXjQgzPKMHGjH1131630 = KIxXjQgzPKMHGjH34753092;     KIxXjQgzPKMHGjH34753092 = KIxXjQgzPKMHGjH28930965;     KIxXjQgzPKMHGjH28930965 = KIxXjQgzPKMHGjH24437084;     KIxXjQgzPKMHGjH24437084 = KIxXjQgzPKMHGjH22194711;     KIxXjQgzPKMHGjH22194711 = KIxXjQgzPKMHGjH40212203;     KIxXjQgzPKMHGjH40212203 = KIxXjQgzPKMHGjH33817933;     KIxXjQgzPKMHGjH33817933 = KIxXjQgzPKMHGjH83948189;     KIxXjQgzPKMHGjH83948189 = KIxXjQgzPKMHGjH54111883;     KIxXjQgzPKMHGjH54111883 = KIxXjQgzPKMHGjH56808501;     KIxXjQgzPKMHGjH56808501 = KIxXjQgzPKMHGjH79792033;     KIxXjQgzPKMHGjH79792033 = KIxXjQgzPKMHGjH33884106;     KIxXjQgzPKMHGjH33884106 = KIxXjQgzPKMHGjH84179464;     KIxXjQgzPKMHGjH84179464 = KIxXjQgzPKMHGjH82192335;     KIxXjQgzPKMHGjH82192335 = KIxXjQgzPKMHGjH17034105;     KIxXjQgzPKMHGjH17034105 = KIxXjQgzPKMHGjH71031485;     KIxXjQgzPKMHGjH71031485 = KIxXjQgzPKMHGjH24836643;     KIxXjQgzPKMHGjH24836643 = KIxXjQgzPKMHGjH76555427;     KIxXjQgzPKMHGjH76555427 = KIxXjQgzPKMHGjH73249012;     KIxXjQgzPKMHGjH73249012 = KIxXjQgzPKMHGjH57630940;     KIxXjQgzPKMHGjH57630940 = KIxXjQgzPKMHGjH57058464;     KIxXjQgzPKMHGjH57058464 = KIxXjQgzPKMHGjH36184822;     KIxXjQgzPKMHGjH36184822 = KIxXjQgzPKMHGjH15668913;     KIxXjQgzPKMHGjH15668913 = KIxXjQgzPKMHGjH38255900;     KIxXjQgzPKMHGjH38255900 = KIxXjQgzPKMHGjH20822530;     KIxXjQgzPKMHGjH20822530 = KIxXjQgzPKMHGjH11799808;     KIxXjQgzPKMHGjH11799808 = KIxXjQgzPKMHGjH33344488;     KIxXjQgzPKMHGjH33344488 = KIxXjQgzPKMHGjH98152;     KIxXjQgzPKMHGjH98152 = KIxXjQgzPKMHGjH17323849;     KIxXjQgzPKMHGjH17323849 = KIxXjQgzPKMHGjH4916555;     KIxXjQgzPKMHGjH4916555 = KIxXjQgzPKMHGjH75295473;     KIxXjQgzPKMHGjH75295473 = KIxXjQgzPKMHGjH16894726;     KIxXjQgzPKMHGjH16894726 = KIxXjQgzPKMHGjH54440030;     KIxXjQgzPKMHGjH54440030 = KIxXjQgzPKMHGjH91375306;     KIxXjQgzPKMHGjH91375306 = KIxXjQgzPKMHGjH44141519;     KIxXjQgzPKMHGjH44141519 = KIxXjQgzPKMHGjH67434616;     KIxXjQgzPKMHGjH67434616 = KIxXjQgzPKMHGjH55005551;     KIxXjQgzPKMHGjH55005551 = KIxXjQgzPKMHGjH12013879;     KIxXjQgzPKMHGjH12013879 = KIxXjQgzPKMHGjH25736560;     KIxXjQgzPKMHGjH25736560 = KIxXjQgzPKMHGjH43108931;     KIxXjQgzPKMHGjH43108931 = KIxXjQgzPKMHGjH84020359;     KIxXjQgzPKMHGjH84020359 = KIxXjQgzPKMHGjH67882034;     KIxXjQgzPKMHGjH67882034 = KIxXjQgzPKMHGjH16903743;     KIxXjQgzPKMHGjH16903743 = KIxXjQgzPKMHGjH81944848;     KIxXjQgzPKMHGjH81944848 = KIxXjQgzPKMHGjH34045448;     KIxXjQgzPKMHGjH34045448 = KIxXjQgzPKMHGjH25783373;     KIxXjQgzPKMHGjH25783373 = KIxXjQgzPKMHGjH81335386;     KIxXjQgzPKMHGjH81335386 = KIxXjQgzPKMHGjH8110795;     KIxXjQgzPKMHGjH8110795 = KIxXjQgzPKMHGjH4519696;     KIxXjQgzPKMHGjH4519696 = KIxXjQgzPKMHGjH59374211;     KIxXjQgzPKMHGjH59374211 = KIxXjQgzPKMHGjH90489977;     KIxXjQgzPKMHGjH90489977 = KIxXjQgzPKMHGjH84977145;     KIxXjQgzPKMHGjH84977145 = KIxXjQgzPKMHGjH19067301;     KIxXjQgzPKMHGjH19067301 = KIxXjQgzPKMHGjH45632774;     KIxXjQgzPKMHGjH45632774 = KIxXjQgzPKMHGjH10589718;     KIxXjQgzPKMHGjH10589718 = KIxXjQgzPKMHGjH64946808;     KIxXjQgzPKMHGjH64946808 = KIxXjQgzPKMHGjH19084179;     KIxXjQgzPKMHGjH19084179 = KIxXjQgzPKMHGjH90675064;     KIxXjQgzPKMHGjH90675064 = KIxXjQgzPKMHGjH3614555;     KIxXjQgzPKMHGjH3614555 = KIxXjQgzPKMHGjH10394903;     KIxXjQgzPKMHGjH10394903 = KIxXjQgzPKMHGjH6867715;     KIxXjQgzPKMHGjH6867715 = KIxXjQgzPKMHGjH33719782;     KIxXjQgzPKMHGjH33719782 = KIxXjQgzPKMHGjH66624341;     KIxXjQgzPKMHGjH66624341 = KIxXjQgzPKMHGjH49195329;     KIxXjQgzPKMHGjH49195329 = KIxXjQgzPKMHGjH81513027;     KIxXjQgzPKMHGjH81513027 = KIxXjQgzPKMHGjH62897307;     KIxXjQgzPKMHGjH62897307 = KIxXjQgzPKMHGjH79444075;     KIxXjQgzPKMHGjH79444075 = KIxXjQgzPKMHGjH92804157;     KIxXjQgzPKMHGjH92804157 = KIxXjQgzPKMHGjH38050816;     KIxXjQgzPKMHGjH38050816 = KIxXjQgzPKMHGjH80490008;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lVMRyjvAbecXmvyvklQFhwtGQdfMI54282719() {     int WjvQlCveHXDzimi77002929 = -453206318;    int WjvQlCveHXDzimi92397484 = -776403027;    int WjvQlCveHXDzimi79329929 = -607052365;    int WjvQlCveHXDzimi34200800 = -909806243;    int WjvQlCveHXDzimi16381935 = -224046095;    int WjvQlCveHXDzimi57490047 = -220811189;    int WjvQlCveHXDzimi10642700 = -183977245;    int WjvQlCveHXDzimi44663210 = -900792309;    int WjvQlCveHXDzimi61958001 = -881453044;    int WjvQlCveHXDzimi13028024 = -612410237;    int WjvQlCveHXDzimi5489265 = 70195494;    int WjvQlCveHXDzimi17204145 = -133624272;    int WjvQlCveHXDzimi89207213 = 91784963;    int WjvQlCveHXDzimi3526076 = -458158519;    int WjvQlCveHXDzimi52831027 = -213880694;    int WjvQlCveHXDzimi96277956 = -599928877;    int WjvQlCveHXDzimi45529141 = 80654565;    int WjvQlCveHXDzimi87965711 = -541424385;    int WjvQlCveHXDzimi52938602 = -292247474;    int WjvQlCveHXDzimi79478608 = -20883354;    int WjvQlCveHXDzimi21516336 = -339407774;    int WjvQlCveHXDzimi569236 = -846162876;    int WjvQlCveHXDzimi55011813 = -573284360;    int WjvQlCveHXDzimi14127901 = -866724831;    int WjvQlCveHXDzimi99828807 = -356667529;    int WjvQlCveHXDzimi6430061 = -780641886;    int WjvQlCveHXDzimi63223305 = -273875499;    int WjvQlCveHXDzimi53782936 = -641921475;    int WjvQlCveHXDzimi74512284 = -120585438;    int WjvQlCveHXDzimi3518222 = -360026819;    int WjvQlCveHXDzimi76742261 = -519338602;    int WjvQlCveHXDzimi36881813 = -822273757;    int WjvQlCveHXDzimi52197530 = 83746750;    int WjvQlCveHXDzimi41590633 = -49850236;    int WjvQlCveHXDzimi33921922 = -610543250;    int WjvQlCveHXDzimi65865964 = -800245665;    int WjvQlCveHXDzimi20769170 = -756084603;    int WjvQlCveHXDzimi6944908 = -445390919;    int WjvQlCveHXDzimi32682496 = -362869345;    int WjvQlCveHXDzimi33629283 = -744103762;    int WjvQlCveHXDzimi89079496 = -195996011;    int WjvQlCveHXDzimi23985385 = 94882046;    int WjvQlCveHXDzimi21426042 = -557311317;    int WjvQlCveHXDzimi96387572 = -190523022;    int WjvQlCveHXDzimi48179050 = -337824975;    int WjvQlCveHXDzimi14715132 = -453092507;    int WjvQlCveHXDzimi67943578 = -380546415;    int WjvQlCveHXDzimi5326666 = -743539412;    int WjvQlCveHXDzimi95164495 = -436082463;    int WjvQlCveHXDzimi46050052 = -967083171;    int WjvQlCveHXDzimi9419581 = -626237511;    int WjvQlCveHXDzimi29467102 = -866396557;    int WjvQlCveHXDzimi28328201 = 85096361;    int WjvQlCveHXDzimi1176109 = -217996745;    int WjvQlCveHXDzimi32766384 = -785304640;    int WjvQlCveHXDzimi76433693 = -607043443;    int WjvQlCveHXDzimi37385671 = -103118667;    int WjvQlCveHXDzimi65202028 = -740327535;    int WjvQlCveHXDzimi34371992 = -453138714;    int WjvQlCveHXDzimi9951875 = -443404210;    int WjvQlCveHXDzimi94266741 = -946935691;    int WjvQlCveHXDzimi56859764 = -542055770;    int WjvQlCveHXDzimi70150925 = -680206872;    int WjvQlCveHXDzimi58439780 = -421426226;    int WjvQlCveHXDzimi36285763 = 6928365;    int WjvQlCveHXDzimi68607451 = -107530750;    int WjvQlCveHXDzimi65006615 = -117371022;    int WjvQlCveHXDzimi47616580 = -858364801;    int WjvQlCveHXDzimi69604153 = -847615270;    int WjvQlCveHXDzimi86965063 = -413635029;    int WjvQlCveHXDzimi75508787 = -843844275;    int WjvQlCveHXDzimi38584234 = -473954517;    int WjvQlCveHXDzimi55283215 = -78555040;    int WjvQlCveHXDzimi19309320 = -548143713;    int WjvQlCveHXDzimi90399112 = -824887344;    int WjvQlCveHXDzimi97530950 = -334289820;    int WjvQlCveHXDzimi79143193 = -188851559;    int WjvQlCveHXDzimi58624241 = -282761339;    int WjvQlCveHXDzimi65948850 = -428899856;    int WjvQlCveHXDzimi85113675 = -903575022;    int WjvQlCveHXDzimi38486483 = -300095471;    int WjvQlCveHXDzimi57896639 = -530336087;    int WjvQlCveHXDzimi58618440 = -105839013;    int WjvQlCveHXDzimi28462233 = -153502267;    int WjvQlCveHXDzimi94098640 = -733789308;    int WjvQlCveHXDzimi47275159 = -652942045;    int WjvQlCveHXDzimi8553613 = -807370118;    int WjvQlCveHXDzimi51021422 = -698256506;    int WjvQlCveHXDzimi8824250 = -264545597;    int WjvQlCveHXDzimi57488229 = 96500192;    int WjvQlCveHXDzimi28480293 = -597126999;    int WjvQlCveHXDzimi55567141 = 84242932;    int WjvQlCveHXDzimi72572915 = -992252205;    int WjvQlCveHXDzimi22730622 = -919465136;    int WjvQlCveHXDzimi39362541 = -797168071;    int WjvQlCveHXDzimi32219732 = -653940241;    int WjvQlCveHXDzimi53834460 = -224911082;    int WjvQlCveHXDzimi62986262 = -35885091;    int WjvQlCveHXDzimi60101809 = -97451387;    int WjvQlCveHXDzimi79571599 = -453206318;     WjvQlCveHXDzimi77002929 = WjvQlCveHXDzimi92397484;     WjvQlCveHXDzimi92397484 = WjvQlCveHXDzimi79329929;     WjvQlCveHXDzimi79329929 = WjvQlCveHXDzimi34200800;     WjvQlCveHXDzimi34200800 = WjvQlCveHXDzimi16381935;     WjvQlCveHXDzimi16381935 = WjvQlCveHXDzimi57490047;     WjvQlCveHXDzimi57490047 = WjvQlCveHXDzimi10642700;     WjvQlCveHXDzimi10642700 = WjvQlCveHXDzimi44663210;     WjvQlCveHXDzimi44663210 = WjvQlCveHXDzimi61958001;     WjvQlCveHXDzimi61958001 = WjvQlCveHXDzimi13028024;     WjvQlCveHXDzimi13028024 = WjvQlCveHXDzimi5489265;     WjvQlCveHXDzimi5489265 = WjvQlCveHXDzimi17204145;     WjvQlCveHXDzimi17204145 = WjvQlCveHXDzimi89207213;     WjvQlCveHXDzimi89207213 = WjvQlCveHXDzimi3526076;     WjvQlCveHXDzimi3526076 = WjvQlCveHXDzimi52831027;     WjvQlCveHXDzimi52831027 = WjvQlCveHXDzimi96277956;     WjvQlCveHXDzimi96277956 = WjvQlCveHXDzimi45529141;     WjvQlCveHXDzimi45529141 = WjvQlCveHXDzimi87965711;     WjvQlCveHXDzimi87965711 = WjvQlCveHXDzimi52938602;     WjvQlCveHXDzimi52938602 = WjvQlCveHXDzimi79478608;     WjvQlCveHXDzimi79478608 = WjvQlCveHXDzimi21516336;     WjvQlCveHXDzimi21516336 = WjvQlCveHXDzimi569236;     WjvQlCveHXDzimi569236 = WjvQlCveHXDzimi55011813;     WjvQlCveHXDzimi55011813 = WjvQlCveHXDzimi14127901;     WjvQlCveHXDzimi14127901 = WjvQlCveHXDzimi99828807;     WjvQlCveHXDzimi99828807 = WjvQlCveHXDzimi6430061;     WjvQlCveHXDzimi6430061 = WjvQlCveHXDzimi63223305;     WjvQlCveHXDzimi63223305 = WjvQlCveHXDzimi53782936;     WjvQlCveHXDzimi53782936 = WjvQlCveHXDzimi74512284;     WjvQlCveHXDzimi74512284 = WjvQlCveHXDzimi3518222;     WjvQlCveHXDzimi3518222 = WjvQlCveHXDzimi76742261;     WjvQlCveHXDzimi76742261 = WjvQlCveHXDzimi36881813;     WjvQlCveHXDzimi36881813 = WjvQlCveHXDzimi52197530;     WjvQlCveHXDzimi52197530 = WjvQlCveHXDzimi41590633;     WjvQlCveHXDzimi41590633 = WjvQlCveHXDzimi33921922;     WjvQlCveHXDzimi33921922 = WjvQlCveHXDzimi65865964;     WjvQlCveHXDzimi65865964 = WjvQlCveHXDzimi20769170;     WjvQlCveHXDzimi20769170 = WjvQlCveHXDzimi6944908;     WjvQlCveHXDzimi6944908 = WjvQlCveHXDzimi32682496;     WjvQlCveHXDzimi32682496 = WjvQlCveHXDzimi33629283;     WjvQlCveHXDzimi33629283 = WjvQlCveHXDzimi89079496;     WjvQlCveHXDzimi89079496 = WjvQlCveHXDzimi23985385;     WjvQlCveHXDzimi23985385 = WjvQlCveHXDzimi21426042;     WjvQlCveHXDzimi21426042 = WjvQlCveHXDzimi96387572;     WjvQlCveHXDzimi96387572 = WjvQlCveHXDzimi48179050;     WjvQlCveHXDzimi48179050 = WjvQlCveHXDzimi14715132;     WjvQlCveHXDzimi14715132 = WjvQlCveHXDzimi67943578;     WjvQlCveHXDzimi67943578 = WjvQlCveHXDzimi5326666;     WjvQlCveHXDzimi5326666 = WjvQlCveHXDzimi95164495;     WjvQlCveHXDzimi95164495 = WjvQlCveHXDzimi46050052;     WjvQlCveHXDzimi46050052 = WjvQlCveHXDzimi9419581;     WjvQlCveHXDzimi9419581 = WjvQlCveHXDzimi29467102;     WjvQlCveHXDzimi29467102 = WjvQlCveHXDzimi28328201;     WjvQlCveHXDzimi28328201 = WjvQlCveHXDzimi1176109;     WjvQlCveHXDzimi1176109 = WjvQlCveHXDzimi32766384;     WjvQlCveHXDzimi32766384 = WjvQlCveHXDzimi76433693;     WjvQlCveHXDzimi76433693 = WjvQlCveHXDzimi37385671;     WjvQlCveHXDzimi37385671 = WjvQlCveHXDzimi65202028;     WjvQlCveHXDzimi65202028 = WjvQlCveHXDzimi34371992;     WjvQlCveHXDzimi34371992 = WjvQlCveHXDzimi9951875;     WjvQlCveHXDzimi9951875 = WjvQlCveHXDzimi94266741;     WjvQlCveHXDzimi94266741 = WjvQlCveHXDzimi56859764;     WjvQlCveHXDzimi56859764 = WjvQlCveHXDzimi70150925;     WjvQlCveHXDzimi70150925 = WjvQlCveHXDzimi58439780;     WjvQlCveHXDzimi58439780 = WjvQlCveHXDzimi36285763;     WjvQlCveHXDzimi36285763 = WjvQlCveHXDzimi68607451;     WjvQlCveHXDzimi68607451 = WjvQlCveHXDzimi65006615;     WjvQlCveHXDzimi65006615 = WjvQlCveHXDzimi47616580;     WjvQlCveHXDzimi47616580 = WjvQlCveHXDzimi69604153;     WjvQlCveHXDzimi69604153 = WjvQlCveHXDzimi86965063;     WjvQlCveHXDzimi86965063 = WjvQlCveHXDzimi75508787;     WjvQlCveHXDzimi75508787 = WjvQlCveHXDzimi38584234;     WjvQlCveHXDzimi38584234 = WjvQlCveHXDzimi55283215;     WjvQlCveHXDzimi55283215 = WjvQlCveHXDzimi19309320;     WjvQlCveHXDzimi19309320 = WjvQlCveHXDzimi90399112;     WjvQlCveHXDzimi90399112 = WjvQlCveHXDzimi97530950;     WjvQlCveHXDzimi97530950 = WjvQlCveHXDzimi79143193;     WjvQlCveHXDzimi79143193 = WjvQlCveHXDzimi58624241;     WjvQlCveHXDzimi58624241 = WjvQlCveHXDzimi65948850;     WjvQlCveHXDzimi65948850 = WjvQlCveHXDzimi85113675;     WjvQlCveHXDzimi85113675 = WjvQlCveHXDzimi38486483;     WjvQlCveHXDzimi38486483 = WjvQlCveHXDzimi57896639;     WjvQlCveHXDzimi57896639 = WjvQlCveHXDzimi58618440;     WjvQlCveHXDzimi58618440 = WjvQlCveHXDzimi28462233;     WjvQlCveHXDzimi28462233 = WjvQlCveHXDzimi94098640;     WjvQlCveHXDzimi94098640 = WjvQlCveHXDzimi47275159;     WjvQlCveHXDzimi47275159 = WjvQlCveHXDzimi8553613;     WjvQlCveHXDzimi8553613 = WjvQlCveHXDzimi51021422;     WjvQlCveHXDzimi51021422 = WjvQlCveHXDzimi8824250;     WjvQlCveHXDzimi8824250 = WjvQlCveHXDzimi57488229;     WjvQlCveHXDzimi57488229 = WjvQlCveHXDzimi28480293;     WjvQlCveHXDzimi28480293 = WjvQlCveHXDzimi55567141;     WjvQlCveHXDzimi55567141 = WjvQlCveHXDzimi72572915;     WjvQlCveHXDzimi72572915 = WjvQlCveHXDzimi22730622;     WjvQlCveHXDzimi22730622 = WjvQlCveHXDzimi39362541;     WjvQlCveHXDzimi39362541 = WjvQlCveHXDzimi32219732;     WjvQlCveHXDzimi32219732 = WjvQlCveHXDzimi53834460;     WjvQlCveHXDzimi53834460 = WjvQlCveHXDzimi62986262;     WjvQlCveHXDzimi62986262 = WjvQlCveHXDzimi60101809;     WjvQlCveHXDzimi60101809 = WjvQlCveHXDzimi79571599;     WjvQlCveHXDzimi79571599 = WjvQlCveHXDzimi77002929;}
// Junk Finished
