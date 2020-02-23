#include "Menu.hpp"

#include "Options.hpp"

#include "Structs.hpp"

#include "features/Miscellaneous.hpp"
#include "features/KitParser.hpp"
#include "features/Skinchanger.hpp"

#include "imgui/imgui_internal.h"

#include <functional>
#include <experimental/filesystem> // hack
#include "features/AntiAim.hpp"
#include "orosbucocu.h" // s 
#include "Install.hpp"
#include "skechtew.h"
#include <d3dx9.h>


static const char* subtabsl[] = {
	"Aimbot" ,
	"Triggerbot"
};
static int legittab = 0;


#define UNLEN 256


namespace ImGui
{
	static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	bool Combo(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}

	bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values, int height_in_items = -1)
	{
		if (values.empty()) { return false; }
		return ListBox(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size(), height_in_items);
	}

	static bool ListBox(const char* label, int* current_item, std::function<const char* (int)> lambda, int items_count, int height_in_items)
	{
		return ImGui::ListBox(label, current_item, [](void* data, int idx, const char** out_text)
			{
				*out_text = (*reinterpret_cast<std::function<const char* (int)>*>(data))(idx);
				return true;
			}, &lambda, items_count, height_in_items);
	}

	bool LabelClick(const char* concatoff, const char* concaton, const char* label, bool* v, const char* unique_id)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		// The concatoff/on thingies were for my weapon config system so if we're going to make that, we still need this aids.
		char Buf[64];
		_snprintf(Buf, 62, "%s%s", ((*v) ? concatoff : concaton), label);

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(unique_id);
		const ImVec2 label_size = CalcTextSize(label, NULL, true);

		const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.y * 2, label_size.y + style.FramePadding.y * 2));
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

		if (label_size.x > 0.0f)
			RenderText(check_bb.GetTL(), Buf);

		return pressed;
	}

	void KeyBindButton(ButtonCode_t* key)
	{
		bool clicked = false;

		std::string text = g_InputSystem->ButtonCodeToString(*key);
		std::string unique_id = std::to_string((int)key);

		if (*key <= BUTTON_CODE_NONE)
			text = "Button";

		if (input_shouldListen && input_receivedKeyval == key) {
			clicked = true;
			text = "...";
		}
		text += "]";

		ImGui::SameLine();
		ImGui::LabelClick("[", "[", text.c_str(), &clicked, unique_id.c_str());

		if (clicked)
		{
			input_shouldListen = true;
			input_receivedKeyval = key;
		}

		if (*key == KEY_DELETE)
		{
			*key = BUTTON_CODE_NONE;
		}

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Press Del to Remove Bind");
	}

	ImGuiID Colorpicker_Close = 0;
	__inline void CloseLeftoverPicker() { if (Colorpicker_Close) ImGui::ClosePopup(Colorpicker_Close); }
	void ColorPickerBox(const char* picker_idname, float col_ct[], float col_t[], float col_ct_invis[], float col_t_invis[], bool alpha = true)
	{
		ImGui::SameLine();
		static bool switch_entity_teams = false;
		static bool switch_color_vis = false;
		bool open_popup = ImGui::ColorButton(picker_idname, switch_entity_teams ? (switch_color_vis ? col_t : col_t_invis) : (switch_color_vis ? col_ct : col_ct_invis), ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip, ImVec2(36, 0));
		if (open_popup)
		{
			ImGui::OpenPopup(picker_idname);
			Colorpicker_Close = ImGui::GetID(picker_idname);
		}

		if (ImGui::BeginPopup(picker_idname))
		{
			const char* button_name0 = switch_entity_teams ? "Terrorists" : "Counter-Terrorists";
			if (ImGui::Button(button_name0, ImVec2(-1, 0)))
				switch_entity_teams = !switch_entity_teams;

			const char* button_name1 = switch_color_vis ? "Invisible" : "Visible";
			if (ImGui::Button(button_name1, ImVec2(-1, 0)))
				switch_color_vis = !switch_color_vis;

			std::string id_new = picker_idname;
			id_new += "##pickeritself_";

			ImGui::ColorPicker4(id_new.c_str(), switch_entity_teams ? (switch_color_vis ? col_t : col_t_invis) : (switch_color_vis ? col_ct : col_ct_invis), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_PickerHueBar | (alpha ? ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar : 0));
			ImGui::EndPopup();
		}
	}

	void LocalPlayerColorPickerBox(const char* picker_idname, float col_ct[], float col_t[], bool alpha = false)
	{
		ImGui::SameLine();
		static bool switch_entity_teams = false;
		static bool switch_color_vis = false;
		bool open_popup = ImGui::ColorButton(picker_idname, switch_entity_teams ? (col_t) : (col_ct), ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip, ImVec2(36, 0));
		if (open_popup)
		{
			ImGui::OpenPopup(picker_idname);
			Colorpicker_Close = ImGui::GetID(picker_idname);
		}

		if (ImGui::BeginPopup(picker_idname))
		{
			const char* button_name1 = switch_color_vis ? "Invisible" : "Visible";
			if (ImGui::Button(button_name1, ImVec2(-1, 0)))
				switch_color_vis = !switch_color_vis;

			std::string id_new = picker_idname;
			id_new += "##pickeritself_";

			ImGui::ColorPicker4(id_new.c_str(), switch_color_vis ? (col_t) : (col_ct), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_PickerHueBar | (alpha ? ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar : 0));
			ImGui::EndPopup();
		}
	}

	void CustomColorPickerBox(const char* picker_idname, float col_ct[], float col_t[], bool alpha = false)
	{
		ImGui::SameLine();
		static bool switch_entity_teams = false;
		static bool switch_color_vis = false;
		bool open_popup = ImGui::ColorButton(picker_idname, switch_entity_teams ? (col_t) : (col_ct), ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip, ImVec2(36, 0));
		if (open_popup)
		{
			ImGui::OpenPopup(picker_idname);
			Colorpicker_Close = ImGui::GetID(picker_idname);
		}

		if (ImGui::BeginPopup(picker_idname))
		{
			const char* button_name1 = switch_color_vis ? "Sky" : "World";
			if (ImGui::Button(button_name1, ImVec2(-1, 0)))
				switch_color_vis = !switch_color_vis;

			std::string id_new = picker_idname;
			id_new += "##pickeritself_";

			ImGui::ColorPicker4(id_new.c_str(), switch_color_vis ? (col_t) : (col_ct), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_PickerHueBar | (alpha ? ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar : 0));
			ImGui::EndPopup();
		}
	}

	void ColorPickerBox(const char* picker_idname, float col_n[], bool alpha = true)
	{
		ImGui::SameLine();
		bool open_popup = ImGui::ColorButton(picker_idname, col_n, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip, ImVec2(36, 0));
		if (open_popup)
		{
			ImGui::OpenPopup(picker_idname);
			Colorpicker_Close = ImGui::GetID(picker_idname);
		}

		if (ImGui::BeginPopup(picker_idname))
		{
			std::string id_new = picker_idname;
			id_new += "##pickeritself_";

			ImGui::ColorPicker4(id_new.c_str(), col_n, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_PickerHueBar | (alpha ? ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar : 0));
			ImGui::EndPopup();
		}



	}

	// This can be used anywhere, in group boxes etc.
	void SelectTabs(int* selected, const char* items[], int item_count, ImVec2 size = ImVec2(0, 0))
	{
		auto color_grayblue = GetColorU32(ImVec4(0.05, 0.15, 0.45, 0.30));
		auto color_deepblue = GetColorU32(ImVec4(0, 0.25, 0.50, 0.25));
		auto color_shade_hover = GetColorU32(ImVec4(1, 1, 1, 0.05));
		auto color_shade_clicked = GetColorU32(ImVec4(1, 1, 1, 0.1));
		auto color_black_outlines = GetColorU32(ImVec4(0, 0, 0, 1));

		ImGuiStyle& style = GetStyle();
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		std::string names;
		for (int32_t i = 0; i < item_count; i++)
			names += items[i];

		ImGuiContext* g = GImGui;
		const ImGuiID id = window->GetID(names.c_str());
		const ImVec2 label_size = CalcTextSize(names.c_str(), NULL, true);

		ImVec2 Min = window->DC.CursorPos;
		ImVec2 Max = ((size.x <= 0 || size.y <= 0) ? ImVec2(Min.x + GetContentRegionMax().x - style.WindowPadding.x, Min.y + label_size.y * 2) : Min + size);

		ImRect bb(Min, Max);
		ItemSize(bb, style.FramePadding.y);
		if (!ItemAdd(bb, id))
			return;

		PushClipRect(ImVec2(Min.x, Min.y - 1), ImVec2(Max.x, Max.y + 1), false);

		window->DrawList->AddRectFilledMultiColor(Min, Max, color_grayblue, color_grayblue, color_deepblue, color_deepblue); // Main gradient.

		ImVec2 mouse_pos = GetMousePos();
		bool mouse_click = g->IO.MouseClicked[0];

		float TabSize = ceil((Max.x - Min.x) / item_count);

		for (int32_t i = 0; i < item_count; i++)
		{
			ImVec2 Min_cur_label = ImVec2(Min.x + (int)TabSize * i, Min.y);
			ImVec2 Max_cur_label = ImVec2(Min.x + (int)TabSize * i + (int)TabSize, Max.y);

			// Imprecision clamping. gay but works :^)
			Max_cur_label.x = (Max_cur_label.x >= Max.x ? Max.x : Max_cur_label.x);

			if (mouse_pos.x > Min_cur_label.x && mouse_pos.x < Max_cur_label.x &&
				mouse_pos.y > Min_cur_label.y && mouse_pos.y < Max_cur_label.y)
			{
				if (mouse_click)
					* selected = i;
				else if (i != *selected)
					window->DrawList->AddRectFilled(Min_cur_label, Max_cur_label, color_shade_hover);
			}


			if (i == *selected) {
				window->DrawList->AddRectFilled(Min_cur_label, Max_cur_label, color_shade_clicked);
				window->DrawList->AddRectFilledMultiColor(Min_cur_label, Max_cur_label, color_deepblue, color_deepblue, color_grayblue, color_grayblue);
				window->DrawList->AddLine(ImVec2(Min_cur_label.x - 1.5f, Min_cur_label.y - 1), ImVec2(Max_cur_label.x - 0.5f, Min_cur_label.y - 1), color_black_outlines);
			}
			else
				window->DrawList->AddLine(ImVec2(Min_cur_label.x - 1, Min_cur_label.y), ImVec2(Max_cur_label.x, Min_cur_label.y), color_black_outlines);
			window->DrawList->AddLine(ImVec2(Max_cur_label.x - 1, Max_cur_label.y), ImVec2(Max_cur_label.x - 1, Min_cur_label.y - 0.5f), color_black_outlines);

			const ImVec2 text_size = CalcTextSize(items[i], NULL, true);
			float pad_ = style.FramePadding.x + g->FontSize + style.ItemInnerSpacing.x;
			ImRect tab_rect(Min_cur_label, Max_cur_label);
			RenderTextClipped(Min_cur_label, Max_cur_label, items[i], NULL, &text_size, style.WindowTitleAlign, &tab_rect);
		}

		window->DrawList->AddLine(ImVec2(Min.x, Min.y - 0.5f), ImVec2(Min.x, Max.y), color_black_outlines);
		window->DrawList->AddLine(ImVec2(Min.x, Max.y), Max, color_black_outlines);
		PopClipRect();
	}
}
#include "background.h"
auto temadegistir = 0;
namespace GladiatorMenu
{
	ImFont* cheat_font;
	ImFont* title_font;
	IDirect3DTexture9* brand_img = nullptr;

	ImFont* buton_font;
	ImFont* tab_font;
	ImFont* annen;
	ImFont* baban;
	ImFont* trix;

	IDirect3DTexture9* ragebtn = nullptr;
	IDirect3DTexture9* legitbtn = nullptr;
	IDirect3DTexture9* visualbtn = nullptr;
	IDirect3DTexture9* miscbtn = nullptr;
	IDirect3DTexture9* renkbtn = nullptr;
	IDirect3DTexture9* ayarbtn = nullptr;
	IDirect3DTexture9* sknchangerbtn = nullptr;
	IDirect3DTexture9* korumabtn = nullptr;

	IDirect3DTexture9* trixbaba = nullptr; //background
	IDirect3DTexture9* trixbg = nullptr; //sol ust logo

	void GUI_Init(HWND window, IDirect3DDevice9* pDevice)
	{

		static int hue = 190; // 140

		ImGui_ImplDX9_Init(window, pDevice);

		ImGuiStyle& style = ImGui::GetStyle();

		ImVec4 col_text = ImColor::HSV(hue / 255.f, 20.f / 255.f, 235.f / 255.f);
		ImVec4 col_main = ImColor(31, 29, 50); //ImColor(9, 82, 128);
		ImVec4 col_back = ImColor(31, 29, 50);
		ImVec4 col_area = ImColor(31, 29, 50);
		ImVec4 col_theme = ImColor(31, 29, 50);

		ImVec4 col_title = ImColor(49, 69, 101);




		static float r = 1.0f;
		static float g = 0.f;
		static float b = 0.f;

		if (r == 1.f && g >= 0.f && b <= 0.f)
		{
			g += 0.005f;
			b = 0.f;
		}
		if (r <= 1.f && g >= 1.f && b == 0.f)
		{
			g = 1.f;
			r -= 0.005f;
		}
		if (r <= 0.f && g == 1.f && b >= 0.f)
		{
			r = 0.f;
			b += 0.005f;
		}
		if (r == 0.f && g <= 1.f && b >= 1.f)
		{
			b = 1.f;
			g -= 0.005f;
		}
		if (r >= 0.f && g <= 0.f && b == 1.f)
		{
			g = 0.f;
			r += 0.005f;
		}




		if (r >= 1.f && g >= 0.f && b <= 1.f)
		{
			r = 1.f;
			b -= 0.005f;
		}
		if (trixbaba == nullptr)
		{
			D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &trixfln, sizeof(trixfln), 864, 540, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &trixbaba);
		}



	if (trixbg == nullptr)
	{
		D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &trixlogo, sizeof(trixlogo), 870, 600, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &trixbg);
	}

	if (ragebtn == nullptr)
	{
		D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &ragelogo, sizeof(ragelogo), 40, 40, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &ragebtn);
	}
	if (legitbtn == nullptr)
	{
		D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &legitlogo, sizeof(legitlogo), 40, 40, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &legitbtn);
	}
	if (visualbtn == nullptr)
	{
		D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &visuallogo, sizeof(visuallogo), 40, 40, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &visualbtn);
	}
	if (miscbtn == nullptr)
	{
		D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &digerlogo, sizeof(digerlogo), 40, 40, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &miscbtn);
	}
	if (renkbtn == nullptr)
	{
		D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &renklogo, sizeof(renklogo), 40, 40, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &renkbtn);
	}
	if (sknchangerbtn == nullptr)
	{
		D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &skinlogo, sizeof(skinlogo), 40, 40, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &sknchangerbtn);
	}
	if (ayarbtn == nullptr)
	{
		D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &ayarlogo, sizeof(ayarlogo), 40, 40, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &ayarbtn);
	}
	if (korumabtn == nullptr)
	{
		D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &korumalogo, sizeof(korumalogo), 40, 40, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &korumabtn);
	}
	style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.39f, 0.59f, 0.00f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.00f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.30f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.49f, 0.67f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.49f, 0.67f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.49f, 0.67f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.07f, 0.49f, 0.67f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.46f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.08f, 0.46f, 0.69f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.07f, 0.46f, 0.64f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.04f, 0.39f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_ComboBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.99f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.18f, 0.60f, 0.64f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.72f, 0.76f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.05f, 0.49f, 0.53f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.07f, 0.46f, 0.88f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.07f, 0.46f, 0.88f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.07f, 0.46f, 0.88f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.50f, 0.50f, 0.90f, 0.50f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.70f, 0.70f, 0.90f, 0.60f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 0.50f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);



		style.Alpha = 0.8f;
		style.WindowPadding = ImVec2(8, 8);
		style.WindowRounding = 9.0f;
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.ChildWindowRounding = 9.0f;
		style.FramePadding = ImVec2(4, 3);
		style.FrameRounding = 0.0f;
		style.ItemSpacing = ImVec2(8, 4);
		style.ItemInnerSpacing = ImVec2(4, 4);
		style.TouchExtraPadding = ImVec2(0, 0);
		style.IndentSpacing = 21.0f;
		style.ColumnsMinSpacing = 6.0f;
		style.ScrollbarSize = 10.0f;
		style.ScrollbarRounding = 3.0f;
		style.GrabMinSize = 10.0f;
		style.GrabRounding = 0.0f;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.DisplayWindowPadding = ImVec2(22, 22);
		style.DisplaySafeAreaPadding = ImVec2(4, 4);
		style.AntiAliasedLines = true;
		style.AntiAliasedShapes = true;
		style.CurveTessellationTol = 1.25f;
		style.AntiAliasedLines = true;
		style.AntiAliasedShapes = true;
		style.CurveTessellationTol = 1.25f;
		style.WindowPadThickness = 4.f;

		d3dinit = true;
		cheat_font = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf",
			12.f, 0, ImGui::GetIO().Fonts->GetGlyphRangesDefault());
		title_font = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Impact.ttf", 80);
		tab_font = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf",
			12.f, 0, ImGui::GetIO().Fonts->GetGlyphRangesDefault());
		buton_font = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arialbd.ttf",
			21.f, 0, ImGui::GetIO().Fonts->GetGlyphRangesDefault());
		annen= ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(MyFont_compressed_data, MyFont_compressed_size, 35.f);
		baban = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(tabs_compressed_data, tabs_compressed_size, 35.f);

	}

	void openMenu()
	{
		static bool bDown = false;
		static bool bClicked = false;
		static bool bPrevMenuState = menuOpen;

		if (pressedKey[VK_INSERT])
		{
			bClicked = false;
			bDown = true;
		}
		else if (!pressedKey[VK_INSERT] && bDown)
		{
			bClicked = true;
			bDown = false;
		}
		else
		{
			bClicked = false;
			bDown = false;
		}

		if (bClicked)
		{
			menuOpen = !menuOpen;
			ImGui::CloseLeftoverPicker();
		}

		bPrevMenuState = menuOpen;
	}







	void mainWindow()
	{

		ImGui::SetNextWindowSize(ImVec2(870, 600), ImGuiSetCond_FirstUseEver); // 860 540

		ImGui::PushFont(title_font);
	
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
		if (ImGui::Begin(" TriX Ware Reborn", &menuOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_ShowBorders))
		{
			ImGuiStyle& style = ImGui::GetStyle();
			ImGui::GetWindowDrawList()->AddImage(trixbg, ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y), ImVec2(ImGui::GetWindowPos().x + 870, ImGui::GetWindowPos().y + 600), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));
				ImGui::BeginGroup();
				ImGui::Text(""); ImGui::SameLine();  ImGui::BeginChild("##Butonlar", ImVec2(210, 580), true);
				


				ImGui::Text("   "); ImGui::SameLine(); ImGui::Image(trixbaba, ImVec2(100.f, 80.f));
               	ImGui::Separator();

				ImGui::PushFont(buton_font);
				static int selected_Tab = 0;
				//	ImGui::SelectTabs(&selected_Tab, items, 8);
				style.Colors[ImGuiCol_Button] = ImVec4(0.35f, 0.40f, 0.61f, 0.f);
				style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.48f, 0.71f, 0.f);
				style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.46f, 0.54f, 0.80f, 0.00f);

				if(selected_Tab == 0)
				style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.59f, 0.89f, 1.00f);
				if (!selected_Tab == 0)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

				if (ImGui::ImageButtonWithText(ragebtn,u8"Ragebot", ImVec2(40, 40)))
					selected_Tab = 0;

				if (selected_Tab == 0)
				style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);


				if (selected_Tab == 1)
					style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.59f, 0.89f, 1.00f);
				if (!selected_Tab == 1)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
				if (ImGui::ImageButtonWithText(legitbtn,u8"Legitbot", ImVec2(40, 40)))
					selected_Tab = 1;

				if (selected_Tab == 1)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

				if (selected_Tab == 2)
					style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.59f, 0.89f, 1.00f);
				if (!selected_Tab == 2)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

				if (ImGui::ImageButtonWithText(visualbtn,u8"Visuals", ImVec2(40, 40)))
					selected_Tab = 2;

				if (selected_Tab == 2)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

				if (selected_Tab == 3)
					style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.59f, 0.89f, 1.00f);
				if (!selected_Tab == 3)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);


				if (ImGui::ImageButtonWithText(miscbtn,u8"Others", ImVec2(40, 40)))
					selected_Tab = 3;	
				if (selected_Tab == 3)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

				if (selected_Tab == 6)
					style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.59f, 0.89f, 1.00f);
				if (!selected_Tab == 6)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);


				if (ImGui::ImageButtonWithText(sknchangerbtn,u8"Skins", ImVec2(40, 40)))
					selected_Tab = 6;
				if (selected_Tab == 6)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

				if (selected_Tab == 5)
					style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.59f, 0.89f, 1.00f);
				if (!selected_Tab == 5)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

				if (ImGui::ImageButtonWithText(ayarbtn,u8"Configs", ImVec2(40, 40)))
					selected_Tab = 5;

				if (selected_Tab == 5)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);


				if (selected_Tab == 4)
					style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.59f, 0.89f, 1.00f);
				if (!selected_Tab == 4)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

				if (ImGui::ImageButtonWithText(renkbtn,u8"Colors", ImVec2(40, 40)))
					selected_Tab = 4;
				if (selected_Tab == 4)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
				if (selected_Tab == 7)
					style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.59f, 0.89f, 1.00f);
				if (!selected_Tab == 7)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

				if (ImGui::ImageButtonWithText(korumabtn,u8"Protection", ImVec2(40, 40)))
					selected_Tab = 7;
				if (selected_Tab == 7)
					style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
				style.Colors[ImGuiCol_Button] = ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
				style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
				style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
				style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
				ImGui::EndChild();
				ImGui::SameLine();
				ImGui::PushFont(tab_font);
			ImGui::EndGroup();
			ImGui::SameLine(); // SOL SİKİŞ BUTONLARI
			ImGui::BeginGroup();

			ImGui::Text(" "); ImGui::SameLine();		ImGui::PushID(selected_Tab);

			ImGui::PushFont(tab_font);
			
			switch (selected_Tab)
			{
			case 0:
				ragebotTab();
				break;
			case 1:
				legitTab();
				break;
			case 2:
				visualTab();
				break;
			case 3:
				miscTab();
				break;
			case 4:
				hvhTab();
				break;
			case 5:
				resolverTab();
				break;
			case 6:
				skinchangerTab();
				break;
			case 7:
				protectTab();
				break;
			}
			
			ImGui::PopFont();

			ImGui::PopID();
			ImGui::EndGroup();

			ImGui::End();
		}
		ImGui::PopFont();
	}

	void aimbotTab()
	{

		{
			ImGui::Separator();
			static int selected_Tab = 0;
			static const char* items[9] = { "Pistol", "Deagle/R8" ,"Submachine-Gun", "Shotgun" ,"Machine-Gun", "Assault-Rifle", "Scout", "AWP" ,"Auto-Sniper" };
			ImGui::SelectTabs(&selected_Tab, items, 9);

			ImGui::EndGroup();

			ImGui::BeginGroup();
			ImGui::PushID(selected_Tab);

			ImGui::PushFont(cheat_font);
			ImGui::Separator();
			switch (selected_Tab)
			{
			case 0: // Pistol
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Text(u8"Auto Wall");
					ImGui::SliderFloat("Pistol ##RagePistol", &g_Options.rage_mindmg_amount_pistol, 0.f, 120.f, "%.2f");
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitchance");
					ImGui::SliderFloat("Pistol %##RagePistol", &g_Options.rage_hitchance_amount_pistol, 0.f, 100.0f, "%.2f");
				}
				ImGui::EndColumns();
				ImGui::Separator();
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Checkbox("Prioritize Selected Hitbox##RagePistol", &g_Options.bRage_prioritize_pistol);
					ImGui::Combo("Select Hitbox##RagePistol", &g_Options.iRage_hitbox_pistol, opt_AimHitboxSpot, 4);
					ImGui::Checkbox(u8"MultiPoint##RagePistol", &g_Options.bRage_multipoint_pistol);
					ImGui::SliderFloat("Pointscale##RagePistol", &g_Options.rage_pointscale_amount_pistol, 0.0f, 1.0f);
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitscan");
					ImGui::BeginChild("#MULTIPOINTPISTOL", ImVec2(0, 120), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
					for (int i = 0; i < ARRAYSIZE(opt_MultiHitboxes); ++i)
					{
						ImGui::Selectable(opt_MultiHitboxes[i], &g_Options.rage_multiHitboxesPistol[i]);
					}
					ImGui::EndChild();
				}
				break;
			case 1: // Deagle
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Text(u8"Auto Wall");
					ImGui::SliderFloat("Deagle/R8 ##RageDeagR8", &g_Options.rage_mindmg_amount_deagr8, 0.f, 120.f, "%.2f");
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitchance");
					ImGui::SliderFloat("Deagle/R8 %##RageDeagR8", &g_Options.rage_hitchance_amount_deagr8, 0.f, 100.0f, "%.2f");
				}
				ImGui::EndColumns();
				ImGui::Separator();
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Checkbox("Prioritize Selected Hitbox##RageDeagR8", &g_Options.bRage_prioritize_deagr8);
					ImGui::Combo("Select Hitbox##RageDeagR8", &g_Options.iRage_hitbox_deagr8, opt_AimHitboxSpot, 4);
					ImGui::Checkbox(u8"MultiPoint##RageDeagR8", &g_Options.bRage_multipoint_deagr8);
					ImGui::SliderFloat("Pointscale##RageDeagR8", &g_Options.rage_pointscale_amount_deagr8, 0.0f, 1.0f);
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitscan");
					ImGui::BeginChild("#MULTIPOINTDEAGLER8", ImVec2(0, 120), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
					for (int i = 0; i < ARRAYSIZE(opt_MultiHitboxes); ++i)
					{
						ImGui::Selectable(opt_MultiHitboxes[i], &g_Options.rage_multiHitboxesDeagR8[i]);
					}
					ImGui::EndChild();
				}
				break;

			case 2: // SMG
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Text(u8"Auto Wall");
					ImGui::SliderFloat("SMG ##RageSMG", &g_Options.rage_mindmg_amount_smg, 0.f, 120.f, "%.2f");
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitchance");
					ImGui::SliderFloat("SMG %##RageSMG", &g_Options.rage_hitchance_amount_smg, 0.f, 100.0f, "%.2f");
				}
				ImGui::EndColumns();
				ImGui::Separator();
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Checkbox("Prioritize Selected Hitbox##RageSMG", &g_Options.bRage_prioritize_smg);
					ImGui::Combo("Select Hitbox##RageSMG", &g_Options.iRage_hitbox_smg, opt_AimHitboxSpot, 4);
					ImGui::Checkbox(u8"MultiPoint##RageSMG", &g_Options.bRage_multipoint_smg);
					ImGui::SliderFloat("Pointscale##RageSMG", &g_Options.rage_pointscale_amount_smg, 0.0f, 1.0f);
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitscan");
					ImGui::BeginChild("#MULTIPOINTSMG", ImVec2(0, 120), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
					for (int i = 0; i < ARRAYSIZE(opt_MultiHitboxes); ++i)
					{
						ImGui::Selectable(opt_MultiHitboxes[i], &g_Options.rage_multiHitboxesSMG[i]);
					}
					ImGui::EndChild();
				}
				break;

			case 3: // Shotgun
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Text(u8"Auto Wall");
					ImGui::SliderFloat("Shotgun ##RageSHOTGUN", &g_Options.rage_mindmg_amount_shotgun, 0.f, 120.f, "%.2f");
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitchance");
					ImGui::SliderFloat("Shotgun %##RageSHOTGUN", &g_Options.rage_hitchance_amount_shotgun, 0.f, 100.0f, "%.2f");
				}
				ImGui::EndColumns();
				ImGui::Separator();
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Checkbox(u8"Prioritize Selected Hitbox##RageSHOTGUN", &g_Options.bRage_prioritize_shotgun);
					ImGui::Combo(u8"Select Hitbox##RageSHOTGUN", &g_Options.iRage_hitbox_shotgun, opt_AimHitboxSpot, 4);
					ImGui::Checkbox(u8"MultiPoint##RageSHOTGUN", &g_Options.bRage_multipoint_shotgun);
					ImGui::SliderFloat("Pointscale##RageSHOTGUN", &g_Options.rage_pointscale_amount_shotgun, 0.0f, 1.0f);
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitscan");
					ImGui::BeginChild("#MULTIPOINTSHOTGUN", ImVec2(0, 120), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
					for (int i = 0; i < ARRAYSIZE(opt_MultiHitboxes); ++i)
					{
						ImGui::Selectable(opt_MultiHitboxes[i], &g_Options.rage_multiHitboxesShotgun[i]);
					}
					ImGui::EndChild();
				}
				break;

			case 4: // MG
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Text(u8"Auto Wall");
					ImGui::SliderFloat("MG ##RageMG", &g_Options.rage_mindmg_amount_mg, 0.f, 120.f, "%.2f");
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitchance");
					ImGui::SliderFloat("MG %##RageMG", &g_Options.rage_hitchance_amount_mg, 0.f, 100.0f, "%.2f");
				}
				ImGui::EndColumns();
				ImGui::Separator();
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Checkbox("Prioritize Selected Hitbox##RageMG", &g_Options.bRage_prioritize_mg);
					ImGui::Combo("Select Hitbox##RageMG", &g_Options.iRage_hitbox_mg, opt_AimHitboxSpot, 4);
					ImGui::Checkbox(u8"MultiPoint##RageMG", &g_Options.bRage_multipoint_mg);
					ImGui::SliderFloat(u8"PointScale##RageMG", &g_Options.rage_pointscale_amount_mg, 0.0f, 1.0f);
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitscan");
					ImGui::BeginChild("#MULTIPOINTMG", ImVec2(0, 120), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
					for (int i = 0; i < ARRAYSIZE(opt_MultiHitboxes); ++i)
					{
						ImGui::Selectable(opt_MultiHitboxes[i], &g_Options.rage_multiHitboxesMG[i]);
					}
					ImGui::EndChild();
				}
				break;

			case 5: // ASSAULT RIFLE
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Text(u8"Auto Wall");
					ImGui::SliderFloat("Assault Rifle ##RageAssaultRifle", &g_Options.rage_mindmg_amount_assaultrifle, 0.f, 120.f, "%.2f");
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitchance");
					ImGui::SliderFloat("Assault Rifle %##RageAssaultRifle", &g_Options.rage_hitchance_amount_assaultrifle, 0.f, 100.0f, "%.2f");
				}
				ImGui::EndColumns();
				ImGui::Separator();
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Checkbox("Prioritize Selected Hitbox##RageAssaultRifle", &g_Options.bRage_prioritize_assaultrifle);
					ImGui::Combo("Select Hitbox##RageAssaultRifle", &g_Options.iRage_hitbox_assaultrifle, opt_AimHitboxSpot, 4);
					ImGui::Checkbox(u8"MultiPoint##RageAssaultRifle", &g_Options.bRage_multipoint_assaultrifle);
					ImGui::SliderFloat("Pointscale##RageAssaultRifle", &g_Options.rage_pointscale_amount_assaultrifle, 0.0f, 1.0f);
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitscan");
					ImGui::BeginChild("#MULTIPOINTAssaultRifle", ImVec2(0, 120), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
					for (int i = 0; i < ARRAYSIZE(opt_MultiHitboxes); ++i)
					{
						ImGui::Selectable(opt_MultiHitboxes[i], &g_Options.rage_multiHitboxesAssaultRifle[i]);
					}
					ImGui::EndChild();
				}
				break;
			case 6: // SCOUT
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Text(u8"Auto Wall");
					ImGui::SliderFloat("Scout ##RageScout", &g_Options.rage_mindmg_amount_scout, 0.f, 120.f, "%.2f");
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitchance");
					ImGui::SliderFloat("Scout %##RageScout", &g_Options.rage_hitchance_amount_scout, 0.f, 100.0f, "%.2f");
				}
				ImGui::EndColumns();
				ImGui::Separator();
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Checkbox("Prioritize Selected Hitbox##RageScout", &g_Options.bRage_prioritize_scout);
					ImGui::Combo("Select Hitbox##RageScout", &g_Options.iRage_hitbox_scout, opt_AimHitboxSpot, 4);
					ImGui::Checkbox(u8"MultiPoint##RageScout", &g_Options.bRage_multipoint_scout);
					ImGui::SliderFloat("Pointscale##RageScout", &g_Options.rage_pointscale_amount_scout, 0.0f, 1.0f);
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitscan");
					ImGui::BeginChild("#MULTIPOINTScout", ImVec2(0, 120), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
					for (int i = 0; i < ARRAYSIZE(opt_MultiHitboxes); ++i)
					{
						ImGui::Selectable(opt_MultiHitboxes[i], &g_Options.rage_multiHitboxesScout[i]);
					}
					ImGui::EndChild();
				}
				break;
			case 7: // AWP
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Text(u8"Auto Wall");
					ImGui::SliderFloat("AWP ##RageAWP", &g_Options.rage_mindmg_amount_awp, 0.f, 120.f, "%.2f");
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitchance");
					ImGui::SliderFloat("AWP %##RageAWP", &g_Options.rage_hitchance_amount_awp, 0.f, 100.0f, "%.2f");
				}
				ImGui::EndColumns();
				ImGui::Separator();
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Checkbox("Prioritize Selected Hitbox##RageAWP", &g_Options.bRage_prioritize_awp);
					ImGui::Combo("Select Hitbox##RageAWP", &g_Options.iRage_hitbox_awp, opt_AimHitboxSpot, 4);
					ImGui::Checkbox(u8"MultiPoint##RageAWP", &g_Options.bRage_multipoint_awp);
					ImGui::SliderFloat("Pointscale##RageAWP", &g_Options.rage_pointscale_amount_awp, 0.0f, 1.0f);
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitscan");
					ImGui::BeginChild("#MULTIPOINTAWP", ImVec2(0, 120), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
					for (int i = 0; i < ARRAYSIZE(opt_MultiHitboxes); ++i)
					{
						ImGui::Selectable(opt_MultiHitboxes[i], &g_Options.rage_multiHitboxesAWP[i]);
					}
					ImGui::EndChild();
				}
				break;
			case 8: // AUTO SNIPER
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Text(u8"Auto Wall");
					ImGui::SliderFloat("Auto ##RageAuto", &g_Options.rage_mindmg_amount_auto, 0.f, 120.f, "%.2f");
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitchance");
					ImGui::SliderFloat("Auto %##RageAuto", &g_Options.rage_hitchance_amount_auto, 0.f, 100.0f, "%.2f");
				}
				ImGui::EndColumns();
				ImGui::Separator();
				ImGui::Columns(2, NULL, true);
				{
					ImGui::Checkbox("Prioritize Selected Hitbox##RageAuto", &g_Options.bRage_prioritize_auto);
					ImGui::Combo("Select Hitbox##RageAuto", &g_Options.iRage_hitbox_auto, opt_AimHitboxSpot, 4);
					ImGui::Checkbox(u8"MultiPoint##RageAuto", &g_Options.bRage_multipoint_auto);
					ImGui::SliderFloat("Pointscale##RageAuto", &g_Options.rage_pointscale_amount_auto, 0.0f, 1.0f);
				}
				ImGui::NextColumn();
				{
					ImGui::Text(u8"Hitscan");
					ImGui::BeginChild("#MULTIPOINTAuto", ImVec2(0, 120), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
					for (int i = 0; i < ARRAYSIZE(opt_MultiHitboxes); ++i)
					{
						ImGui::Selectable(opt_MultiHitboxes[i], &g_Options.rage_multiHitboxesAuto[i]);
					}
					ImGui::EndChild();
				}
				break;
			}
			ImGui::PopFont();
			ImGui::PopID();
			ImGui::EndGroup();
		}
	}
	void legitTab()
	{
		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 1;

		bool placeholder_true = true;



		const char* weapon_tabs[] = {
			u8"Rifles", //Rifles
			u8"Pistols", //Pistols
			u8"Scoped", //AWP / Teco
			u8"SMG", //SMG
			u8"Others", // Others
			u8"Shotguns" // Shotguns
		};

		const char* Hitboxx[] =
		{
			"",
			u8"Head",
			u8"Neck",
			u8"Body",
			u8"Stomach",
			u8"Nearest"

		};
		static int subtab1 = 0;
		static int subtab2 = 0;
		ImGui::SameLine();
		ImGui::BeginChild("Main Tab", ImVec2(0, 0), true);
		{
			style.Colors[ImGuiCol_Button] =			ImVec4(0.f, 0.f, 0.f, 0.f);
			style.Colors[ImGuiCol_ButtonHovered] =	ImVec4(0.f, 0.f, 0.f, 0.f);
			style.Colors[ImGuiCol_ButtonActive] =	ImVec4(0.f, 0.f, 0.f, 0.f);

			style.Colors[ImGuiCol_Button] =			ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
			style.Colors[ImGuiCol_ButtonHovered] =	ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
			style.Colors[ImGuiCol_ButtonActive] =	ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
			ImGui::Checkbox(u8"Enable Legitbot", &g_Options.enable_legitbot);
			ImGui::Checkbox(u8"Enable Backtrack ", &g_Options.misc_backtrack);
			ImGui::Text("");
			ImGui::SelectTabs(&legittab, subtabsl, ARRAYSIZE(subtabsl));
			switch (legittab)
			{
			case 0:
				ImGui::Text("");
				ImGui::Text(u8"Aimkey: "); 
				ImGui::KeyBindButton(&g_Options.aimkey);
				
				ImGui::SelectTabs(&subtab2, weapon_tabs, ARRAYSIZE(weapon_tabs));
				switch (subtab2)
				{
				case 0:

					ImGui::BeginChild("Legit_1", ImVec2(0, 0), true);
					{
						ImGui::Checkbox(u8"Enable Aimbot", &g_Options.aim_LegitBotRifles);
						ImGui::Combo(u8"Hitbox", &g_Options.hitbox_rifles, Hitboxx, ARRAYSIZE(Hitboxx));
						ImGui::SliderFloat(u8"Smooth##0", &g_Options.legit_smooth_rifles, 1.00f, 100.00f, "%.2f");
						ImGui::SliderFloat(u8"FOV##0", &g_Options.legit_fov_rifles, 0.00f, 30.00f, "%.2f");
						ImGui::SliderFloat(u8"Min RCS##0", &g_Options.legit_rcsmin_rifles, 1.00f, 100.00f, "%.2f");
						ImGui::SliderFloat(u8"Max RCS##0", &g_Options.legit_rcsmax_rifles, 1.00f, 100.00f, "%.2f");
					}
					ImGui::EndChild();
					break;
				case 1:
					ImGui::BeginChild("Legit_2", ImVec2(0, 0), true);
					{
						//DrawSpecialText("LegitBot", "", false, false);
						ImGui::Checkbox(u8"Enable Aimbot", &g_Options.aim_LegitBotPistols);
						//ImGui::Hotkey("Key##0", &g_Options.aimkey, ImVec2(0, 0));
						ImGui::Combo(u8"Hitbox", &g_Options.hitbox_pistols, Hitboxx, ARRAYSIZE(Hitboxx));
						ImGui::SliderFloat(u8"Smooth##0", &g_Options.legit_smooth_pistols, 1.00f, 100.00f, "%.2f");
						ImGui::SliderFloat(u8"FOV##0", &g_Options.legit_fov_pistols, 0.00f, 30.00f, "%.2f");
						ImGui::SliderFloat(u8"Min RCS##0", &g_Options.legit_rcsmin_pistols, 1.00f, 100.00f, "%.2f");
						ImGui::SliderFloat(u8"Max RCS##0", &g_Options.legit_rcsmax_pistols, 1.00f, 100.00f, "%.2f");
					}
					ImGui::EndChild();
					break;
				case 2:


					ImGui::BeginChild("Legit_3", ImVec2(0, 0), true);
					{
						//DrawSpecialText("LegitBot", "", false, false);
						ImGui::Checkbox(u8"Enable Aimbot", &g_Options.aim_LegitBotSnipers);
						//ImGui::Hotkey("Key##0", &g_Options.aimkey, ImVec2(0, 0));
						ImGui::Combo(u8"Hitbox", &g_Options.hitbox_snipers, Hitboxx, ARRAYSIZE(Hitboxx));
						ImGui::SliderFloat(u8"Smooth##0", &g_Options.legit_smooth_Snipers, 1.00f, 100.00f, "%.2f");
						ImGui::SliderFloat(u8"FOV##0", &g_Options.legit_fov_Snipers, 0.00f, 30.00f, "%.2f");
						ImGui::SliderFloat(u8"Min RCS##0", &g_Options.legit_rcsmin_Snipers, 1.00f, 100.00f, "%.2f");
						ImGui::SliderFloat(u8"Max RCS##0", &g_Options.legit_rcsmax_Snipers, 1.00f, 100.00f, "%.2f");
					}
					ImGui::EndChild();
					break;
				case 3:
					ImGui::BeginChild("Legit_4", ImVec2(0, 0), true);
					{
						//DrawSpecialText("LegitBot", "", false, false);
						ImGui::Checkbox(u8"Enable Aimbot", &g_Options.aim_LegitBotsmg);
						//ImGui::Hotkey("Key##0", &g_Options.aimkey, ImVec2(0, 0));
						ImGui::Combo(u8"Hitbox", &g_Options.hitbox_smg, Hitboxx, ARRAYSIZE(Hitboxx));
						ImGui::SliderFloat(u8"Smooth##0", &g_Options.legit_smooth_smg, 1.00f, 100.00f, "%.2f");
						ImGui::SliderFloat(u8"FOV##0", &g_Options.legit_fov_smg, 0.00f, 30.00f, "%.2f");
						ImGui::SliderFloat(u8"Min RCS##0", &g_Options.legit_rcsmin_smg, 1.00f, 100.00f, "%.2f");
						ImGui::SliderFloat(u8"Max RCS##0", &g_Options.legit_rcsmax_smg, 1.00f, 100.00f, "%.2f");
					}
					ImGui::EndChild();
					break;
				case 4:
					ImGui::BeginChild("Legit_5", ImVec2(0, 0), true);
					{
						//DrawSpecialText("LegitBot", "", false, false);
						ImGui::Checkbox(u8"Enable Aimbot", &g_Options.aim_LegitBotmg);
						//ImGui::Hotkey("Key##0", &g_Options.aimkey, ImVec2(0, 0));//
						ImGui::Combo(u8"Hitbox", &g_Options.hitbox_mg, Hitboxx, ARRAYSIZE(Hitboxx));
						ImGui::SliderFloat(u8"Smooth##0", &g_Options.legit_smooth_mg, 1.00f, 100.00f, "%.2f");
						ImGui::SliderFloat(u8"FOV##0", &g_Options.legit_fov_mg, 0.00f, 30.00f, "%.2f");
						ImGui::SliderFloat(u8"Min RCS##0", &g_Options.legit_rcsmin_mg, 1.00f, 100.00f, "%.2f");
						ImGui::SliderFloat(u8"Max RCS##0", &g_Options.legit_rcsmax_mg, 1.00f, 100.00f, "%.2f");
					}
					ImGui::EndChild();
					break;
				case 5:
					ImGui::BeginChild("Legit_6", ImVec2(0, 0), true);
					{
						//DrawSpecialText("LegitBot", "", false, false);
						ImGui::Checkbox(u8"Enable Aimbot", &g_Options.aim_LegitBotshotgun);
						//ImGui::Hotkey("Key##0", &g_Options.aimkey, ImVec2(0, 0));//
						ImGui::Combo(u8"Hitbox", &g_Options.hitbox_shotgun, Hitboxx, ARRAYSIZE(Hitboxx));
						ImGui::SliderFloat(u8"Smooth##0", &g_Options.legit_smooth_shotgun, 1.00f, 100.00f, "%.2f");
						ImGui::SliderFloat(u8"FOV##0", &g_Options.legit_fov_shotgun, 0.00f, 30.00f, "%.2f");
						ImGui::SliderFloat(u8"Min RCS##0", &g_Options.legit_rcsmin_shotgun, 1.00f, 100.00f, "%.2f");
						ImGui::SliderFloat(u8"Max RCS##0", &g_Options.legit_rcsmax_shotgun, 1.00f, 100.00f, "%.2f");
					}
					ImGui::EndChild();
					break;
				}
				break;
				///
				/// trigger 
				///

			case 1:
				ImGui::Text(u8"Not have xddd");
				
				break;
			}
			ImGui::Separator();
		}
		ImGui::EndChild();

	}


	typedef unsigned int uint;
	std::string skechprotect(uint l = 15, std::string charIndex = "abcdefghijklmnaoqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890")
	{
		//
		// u (uzunluk) ve alfabe ayarlanabilir, ama unutma onları başlatma gücüne sahip.
		//

		uint length = rand() % l + 1; // Değerlerin uzunluğunu üst satırdaki uzunluk değeri kodu belirler.

		uint ri[15]; // Rastgele yenilemek için kullanılcak rastgele değerler dizisi "alfabe" den alınır 

		for (uint i = 0; i < length; ++i)
			ri[i] = rand() % charIndex.length();
		// ri değişkeni her satıra rastgele bir sayı koyar.

		std::string rs = "SkechProtect- " ""; // Bu işlev artık ekrana yazdırılacak yazıyı belirler.


		for (uint i = 0; i < length; ++i)
			rs += charIndex[ri[i]];
		// "rs" ye rastgele karakter ekler.

		if (rs.empty()) skechprotect(l, charIndex); // Eğer sonuç boş işe kod oluşturmayı yeniden başlatır.


		else return rs;

	}
	void protectTab()
	{
		ImGui::Columns(1, NULL, false);
		{
			ImGui::BeginChild("RAGEBOTCdsaHILD", ImVec2(0, 0), true);
			{
				ImGui::Columns(1, NULL, true);
				{
					srand(time(NULL));
					ImGui::Text(u8"SkechProtect has been added specifically for TrixWare.");
					ImGui::Separator();
					ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str());
					ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str());
					ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str());
					ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str());
					ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str());
					ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str()); ImGui::Text(skechprotect().c_str());
				}
			}
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::EndChild();
		}
	}


	void ragebotTab()
	{

		ImGui::Columns(1, NULL, false);
		{
			ImGui::BeginChild("RAGEBOTCHILD", ImVec2(0, 0), true);
			{
				ImGui::Columns(1, NULL, true);
				{
					
					ImGui::Text(u8"Hitchance settings are done automatically, just choose your settings");
					ImGui::Separator();
					char title[UNLEN];
					char buffer[UNLEN + 1];
					DWORD size;
					size = sizeof(buffer);
					GetUserName(buffer, &size);
					char ch1[25] = u8"Welcome, ";
					char* ch = strcat(ch1, buffer);
					ImGui::Text(ch);
					ImGui::Separator();
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.f);
					ImGui::Checkbox(u8"Enable Ragebot##Rage", &g_Options.rage_enabled);
				}


				{
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.f);
					ImGui::Checkbox(u8"SilentAim##Rage", &g_Options.rage_silent);
					ImGui::Checkbox(u8"No Recoil##Rage", &g_Options.rage_norecoil);
					ImGui::Checkbox(u8"Auto Shoot##Rage", &g_Options.rage_autoshoot);
					ImGui::Checkbox(u8"Auto Scope##Rage", &g_Options.rage_autoscope);
					ImGui::Checkbox(u8"Auto Crouch##Rage", &g_Options.rage_autocrouch);
					ImGui::Checkbox(u8"Auto Stop##Rage", &g_Options.rage_autostop);
					ImGui::Checkbox(u8"Auto Revolver##Rage", &g_Options.rage_autocockrevolver);
				}

				ImGui::Columns(1, NULL, true);
				{
					ImGui::BeginChild("COL1", ImVec2(0, 0), true);
					{
						ImGui::Separator();
						ImGui::Text(u8"Anti-Aim");
						ImGui::Separator();
						{
							ImGui::PushItemWidth(0);
							ImGui::Text(u8"Pitch");
							ImGui::Combo(u8"Pitch", &g_Options.hvh_antiaim_x, opt_AApitch, 5);
							ImGui::Separator();
							ImGui::NewLine();
							ImGui::Separator();
							ImGui::Text(u8"Yaw");
							ImGui::Combo(u8"Yaw", &g_Options.hvh_antiaim_y, opt_AAyaw, 9);
							if (g_Options.hvh_antiaim_y == 5 || g_Options.hvh_antiaim_y == 6) {
								ImGui::Text(u8"Custom Yaw Left");
								ImGui::SliderFloat("##AAY Custom Yaw Left", &g_Options.hvh_antiaim_y_custom_left, -180.0f, 180.0f, "%1.f");
								ImGui::Text(u8"Custom Yaw Right");
								ImGui::SliderFloat("##AAY Custom Yaw Right", &g_Options.hvh_antiaim_y_custom_right, -180.0f, 180.0f, "%1.f");
								ImGui::Text(u8"Custom Yaw Back");
								ImGui::SliderFloat("##AAY Custom Yaw Back", &g_Options.hvh_antiaim_y_custom_back, -180.0f, 180.0f, "%1.f");
							}
							if (g_Options.hvh_antiaim_y == 7 || g_Options.hvh_antiaim_y == 8) {
								ImGui::Text(u8"Angle Left");
								ImGui::SliderFloat("##AAY Angle Left", &g_Options.hvh_antiaim_y_desync_start_left, -180, 180, "%1.f");
								ImGui::Text(u8"Angle Right");
								ImGui::SliderFloat("##AAY Angle Right", &g_Options.hvh_antiaim_y_desync_start_right, -180, 180, "%1.f");
								ImGui::Text(u8"Angle Back");
								ImGui::SliderFloat("##AAY Angle Back", &g_Options.hvh_antiaim_y_desync_start_back, -180, 180, "%1.f");
								ImGui::Text(u8"Desync Angle");
								ImGui::SliderFloat("##AAY Desync", &g_Options.hvh_antiaim_y_desync, -180, 180, "%1.f");
							}
							ImGui::Separator();
							ImGui::NewLine();
							ImGui::Separator();
							ImGui::Text(u8"Yaw Move");
							ImGui::Combo(u8"##AAY Move", &g_Options.hvh_antiaim_y_move, opt_AAyaw_move, 7);
							ImGui::Text(u8"do when velocity over");
							ImGui::SliderFloat("##MOVEAATRIGGERSPEED", &g_Options.hvh_antiaim_y_move_trigger_speed, 0.1, 130);
							if (g_Options.hvh_antiaim_y_move == 5 || g_Options.hvh_antiaim_y_move == 6) {
								ImGui::Text(u8"Custom Yaw Move Left");
								ImGui::SliderFloat("##AAY Custom Yaw Move Left", &g_Options.hvh_antiaim_y_custom_realmove_left, -180.0f, 180.0f, "%1.f");
								ImGui::Text(u8"Custom Yaw Move Right");
								ImGui::SliderFloat("##AAY Custom Yaw Move Right", &g_Options.hvh_antiaim_y_custom_realmove_right, -180.0f, 180.0f, "%1.f");
								ImGui::Text(u8"Custom Yaw Move Back");
								ImGui::SliderFloat("##AAY Custom Yaw Move Back", &g_Options.hvh_antiaim_y_custom_realmove_back, -180.0f, 180.0f, "%1.f");
							}


						//	ImGui::PopItemWidth();
						}

						ImGui::EndChild();
					}
				}
				// ImGui::Columns(1);
				// ImGui::Columns(1, NULL, false);
				// {
				//	ImGui::PushItemWidth(235);
				//	ImGui::PopItemWidth();
			//	}
				ImGui::BeginGroup();
				aimbotTab();
				ImGui::Columns(1);
				ImGui::Separator();
				ImGui::EndChild();
			}
		}
	}

	void visualTab()
	{
		ImGui::Columns(2, NULL, false);
		{
			ImGui::BeginChild("COL1", ImVec2(0, 0), true);
			{
				ImGui::Text(u8"ESP");
				ImGui::Separator();
				ImGui::Columns(1);
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.f);
					ImGui::Combo(u8"Box Type##BOXTYPE", &g_Options.esp_player_boxtype, opt_EspType, 3);
					ImGui::Combo(u8"Bounds Type##BOUNDSTYPE", &g_Options.esp_player_boundstype, opt_BoundsType, 2);
					ImGui::SliderFloat(u8"Fill ESP##ESP", &g_Options.esp_fill_amount, 0.f, 255.f);
					ImGui::Checkbox(u8"Farther ESP##ESP", &g_Options.esp_farther);
					ImGui::Checkbox(u8"Show Team", &g_Options.takimarkadas);
					ImGui::Checkbox(u8"Name##ESP", &g_Options.esp_player_name);
					ImGui::Checkbox(u8"Health##ESP", &g_Options.esp_player_health);
					ImGui::Checkbox(u8"Weapons##ESP", &g_Options.esp_player_weapons);
					ImGui::Checkbox(u8"Skeleton##ESP", &g_Options.esp_player_skelet);
					ImGui::Checkbox(u8"Night Mode##ESP", &g_Options.visuals_nightmode);
					ImGui::Checkbox(u8"AsusWalls##ESP", &g_Options.visuals_asuswalls);
					if (g_Options.visuals_draw_xhair)
						ImGui::Separator();
					ImGui::Checkbox(u8"Draw Crosshair", &g_Options.visuals_draw_xhair);
					if (g_Options.visuals_draw_xhair)
					{
						ImGui::Separator();
						ImGui::SliderInt("Crosshair X", &g_Options.visuals_xhair_x, 0, 15);
						ImGui::SliderInt("Crosshair Y", &g_Options.visuals_xhair_y, 0, 15);

						ImGui::Separator();
					}
					ImGui::Checkbox(u8"Backtrack Skeleton##ESP", &g_Options.esp_backtracked_player_skelet);
					ImGui::Checkbox(u8"Chams##ESP", &g_Options.esp_player_chams);
					ImGui::Combo(u8"Chams Type##ESP", &g_Options.esp_player_chams_type, opt_Chams, 10);
					{
						if (g_Options.fake_chams)
						{
						}
						{
						}
					}
				}

				ImGui::Columns(1);
				ImGui::Separator();
				ImGui::Text(u8"Glow");
				ImGui::Separator();
				ImGui::Columns(1, NULL, true);
				{
					ImGui::Checkbox(u8"Enable Glow##Glow", &g_Options.glow_enabled);
					ImGui::Checkbox(u8"Players##Glow", &g_Options.glow_players);
					ImGui::Combo(u8"Players Type##ESP_player", &g_Options.glow_players_style, opt_GlowStyles, 3);
					ImGui::Checkbox(u8"Others##Glow", &g_Options.glow_others);
					ImGui::Combo(u8"Others Type##ESP_other", &g_Options.glow_others_style, opt_GlowStyles, 3);
					ImGui::Separator();
					ImGui::Checkbox(u8"Planted C4##ESP", &g_Options.esp_planted_c4);
					ImGui::Checkbox(u8"Grenade ESP##ESP", &g_Options.esp_grenades);
					ImGui::Combo(u8"Grenade ESP Type##ESP", &g_Options.esp_grenades_type, opt_GrenadeESPType, 4);	
				}

				ImGui::EndChild();
			}
		}
		ImGui::NextColumn();
		{
			ImGui::BeginChild("COL2", ImVec2(0, 0), true);
			{
				ImGui::Text(u8"Others");
				ImGui::Separator();
				ImGui::Columns(1, NULL, true);
				{
					ImGui::Checkbox(u8"HitMarker##Others", &g_Options.visuals_others_hitmarker);
					ImGui::Checkbox(u8"Bullet Impacts##Others", &g_Options.visuals_others_bulletimpacts);
					ImGui::Separator();
					ImGui::Checkbox(u8"Show Manual AA", &g_Options.visuals_manual_aa);
					ImGui::SliderFloat(u8"Indicator Opacity", &g_Options.visuals_manual_aa_opacity, 0, 255);
					ImGui::Separator();
					ImGui::Checkbox(u8"Grenade Prediction##Others", &g_Options.visuals_others_grenade_pred);
					ImGui::Checkbox(u8"Watermark##Others", &g_Options.visuals_others_watermark);
					ImGui::NewLine();
					ImGui::SliderFloat("FOV##Others", &g_Options.visuals_others_player_fov, 0, 80);
					ImGui::SliderFloat("Viewmodel FOV##Others", &g_Options.visuals_others_player_fov_viewmodel, 0, 80);
					ImGui::Separator();
					ImGui::Checkbox(u8"Viewmodel Offset", &g_Options.change_viewmodel_offset);
					ImGui::NewLine();
					ImGui::SliderFloat("Viewmodel Offset X##ViewmodelX", &g_Options.viewmodel_offset_x, -30, 30);
					ImGui::SliderFloat("Viewmodel Offset Y##ViewmodelY", &g_Options.viewmodel_offset_y, -30, 30);
					ImGui::SliderFloat("Viewmodel Offset Z##ViewmodelZ", &g_Options.viewmodel_offset_z, -30, 30);

				}

				ImGui::Columns(1);
				ImGui::Separator();
				ImGui::Text(u8"Removals");
				ImGui::Separator();
				ImGui::Columns(1, NULL, true);
				{
					ImGui::Checkbox(u8"Flash##Removals", &g_Options.removals_flash);
					ImGui::Checkbox(u8"Smoke##Removals", &g_Options.removals_smoke);
					if (g_Options.removals_smoke)
					{
						ImGui::Combo(u8"Smoke Type##Removals", &g_Options.removals_smoke_type, opt_nosmoketype, 2);
					}
					ImGui::Checkbox(u8"Scope##Removals", &g_Options.removals_scope);
					ImGui::Checkbox(u8"Zoom##Removals", &g_Options.removals_zoom);
					ImGui::Checkbox(u8"Visual Recoil##Removals", &g_Options.removals_novisualrecoil);
					ImGui::Checkbox(u8"Crosshair##Removals", &g_Options.removals_crosshair);
					ImGui::Checkbox(u8"FOG##Removals", &g_Options.fog_override);
					ImGui::Checkbox(u8"PosProcessing##Removals", &g_Options.removals_postprocessing);
				}

				ImGui::Separator();
				ImGui::Combo(u8"Dropped Weapons (RGB)##ESP", &g_Options.esp_dropped_weapons, opt_WeaponBoxType, 4);
				if (g_Options.esp_dropped_weapons > 0)
				{
					ImGui::SliderInt("Red##ESP", &g_Options.dropped_weapons_color[0], 0, 255);
					ImGui::SliderInt("Green##ESP", &g_Options.dropped_weapons_color[1], 0, 255);
					ImGui::SliderInt("Blue##ESP", &g_Options.dropped_weapons_color[2], 0, 255);
				}

				ImGui::EndChild();
			}
		}
	}

	void miscTab()
	{
		ImGui::Columns(2, NULL, false);
		{
			ImGui::BeginChild("COL1", ImVec2(0, 0), true);
			{
				ImGui::Text(u8"Movement");
				ImGui::Separator();
				ImGui::Columns(1, NULL, true);
				{
					ImGui::Checkbox(u8"Bunny-hop##Movement", &g_Options.misc_bhop);
					ImGui::Checkbox(u8"AutoStrafe##Movement", &g_Options.misc_autostrafe);
				}

				static char nName[127] = "";
				ImGui::Columns(1);
				ImGui::Separator();
				ImGui::Text(u8"Name");
				ImGui::Separator();
				ImGui::Columns(2, NULL, true);
				{
					ImGui::PushItemWidth(-1);
					ImGui::InputText("##NNAMEINPUT", nName, 127);
					ImGui::PopItemWidth();
				}
				ImGui::NextColumn();
				{
					if (ImGui::Button(u8"Set Name##Nichnamechanger"))
						Miscellaneous::Get().ChangeName(nName);

					ImGui::SameLine();
					if (ImGui::Button(u8"No Name##Nichnamechanger", ImVec2(-1, 0)))
						Miscellaneous::Get().ChangeName("\n");
				}

				static char ClanChanger[127] = "";
				ImGui::Columns(1);
				ImGui::Separator();
				ImGui::Text(u8"Clantag");
				ImGui::Separator();
				ImGui::Columns(2, NULL, true);
				{
					ImGui::PushItemWidth(-1);
					ImGui::InputText("##CLANINPUT", ClanChanger, 127);
					ImGui::PopItemWidth();
				}
				ImGui::NextColumn();
				{
					if (ImGui::Button(u8"Set Clantag"))
						Utils::SetClantag(ClanChanger);

				}
				ImGui::Columns(1);
				ImGui::Separator();
				ImGui::Text(u8"Other");
				ImGui::Separator();
				ImGui::Columns(1, NULL, true);
				{
					ImGui::Checkbox(u8"Gravity Reduction", &g_Options.silahdondurme);
					ImGui::NewLine();
					ImGui::SliderInt(u8"Value", &g_Options.silahdeger, 0, 1500);
					ImGui::Checkbox("Thirty one", &g_Options.otuzbiresp);
					ImGui::Checkbox("Aspect ratio", &g_Options.aspectratioenable);
					if (g_Options.aspectratioenable)
					{
						ImGui::SliderInt("X", &g_Options.aspectratioen_x, 0, 2000);
						ImGui::SliderInt("Y", &g_Options.aspectratioen_y, 0, 2000);
					}
					ImGui::Checkbox(u8"Auto Pistol##Other", &g_Options.misc_auto_pistol);
					ImGui::Checkbox(u8"Log Events##Other", &g_Options.misc_logevents);
					ImGui::Checkbox(u8"Fantail Clantag##Other", &g_Options.misc_animated_clantag);
					ImGui::Checkbox(u8"Sniper Crosshair", &g_Options.force_crosshair);
					ImGui::Checkbox(u8"Reveal Ranks##Other", &g_Options.misc_revealAllRanks);
					ImGui::Checkbox(u8"Auto Accept##Other", &g_Options.misc_autoaccept);
					ImGui::Checkbox(u8"Asus Walls##Other", &g_Options.visuals_asuswalls);
					ImGui::Checkbox(u8"Spectator List##Other", &g_Options.misc_spectatorlist);
					ImGui::Checkbox(u8"ThirdPerson##Other", &g_Options.misc_thirdperson);
					ImGui::KeyBindButton(&g_Options.misc_thirdperson_bind);
					ImGui::Checkbox(u8"Infinite Duck", &g_Options.misc_infinite_duck);
					ImGui::Separator();
					ImGui::Text(u8"Changers");
					static char nName[127] = "";

					{
						ImGui::PushItemWidth(-1);
						ImGui::InputText("##NNAMEINPUT", nName, 127);
						ImGui::PopItemWidth();
					}
					ImGui::NextColumn();
					{
						if (ImGui::Button(u8"Fake Voting##Nichnamechanger"))
							Miscellaneous::Get().FakeVote(nName);

				
						if (ImGui::Button(u8"Hide Your Name##Nichnamechanger"))
							Miscellaneous::Get().ChangeName("\n\xAD\xAD\xAD");

						
						if (ImGui::Button(u8"Fake VAC##Nichnamechanger"))
							Miscellaneous::Get().FakeVac(nName);

					
						if (ImGui::Button(u8"Fantail##Nichnamechanger"))

							Miscellaneous::Get().ChangeName("Fantail User");

					
						if (ImGui::Button(u8"Fake Unbox##Nichnamechanger"))
							Miscellaneous::Get().FakeUnbox(nName);
						ImGui::Separator();


					}

				}

				ImGui::EndChild();
			}
		}
		ImGui::NextColumn();
		{
			ImGui::BeginChild("COL2", ImVec2(0, 0), true);
			{
				ImGui::Text(u8"FakeLag");
				ImGui::Separator();
				ImGui::Columns(1, NULL, true);
				{
					ImGui::Checkbox(u8"Enable##Fakelag", &g_Options.misc_fakelag_enabled);
					ImGui::KeyBindButton(&g_Options.misc_fakeduck);
					ImGui::Checkbox(u8"Adaptive##Fakelag", &g_Options.misc_fakelag_adaptive);
					ImGui::Combo(u8"Activation Type##Fakelag", &g_Options.misc_fakelag_activation_type, std::vector<std::string>{ "Always", "While Moving", "In Air" });
					ImGui::SliderInt("Choke##Fakelag", &g_Options.misc_fakelag_value, 0, 14);
					ImGui::Separator();
					ImGui::Text(u8"Appearance And Motion");
					ImGui::Separator();
					ImGui::Combo(u8"View Mode##AntiAim", &g_Options.hvh_show_real_angles, std::vector<std::string>{ "Real Angles", "Fake Angles" });
					ImGui::Checkbox(u8"Fake Walk##Other", &g_Options.misc_fakewalk);
					ImGui::KeyBindButton(&g_Options.misc_fakewalk_bind);
					ImGui::SliderInt("Fake Walk Speed##Other", &g_Options.misc_fakewalk_speed, 0, 130);
					ImGui::Separator();
					ImGui::Combo(u8"Sky Changer##NightMode", &g_Options.visuals_others_sky, opt_Skynames, 12);
					ImGui::Separator();
					ImGui::Checkbox(u8"Rank Changer", &g_Options.RankChanger.enabled); // Added For Sikiş
					ImGui::Spacing();

					ImGui::Combo(u8"##RankCombo", &g_Options.RankChanger.rank_id, u8"Rank Selection\0Silver 1\0Silver 2\0Silver 3\0Silver 4\0Silver Elite\0Silver Elite Master\0Gold Nova 1\0Gold Nova 2\0Gold Nova 3\0Gold Nova Master\0Master Guardian 1\0Master Guardian 2\0Master Guardian Elite\0Distinguished Master Guardian\0Legendary Eagle\0Legendary Eagle Master\0Supreme Master First Class\0The Global Elite");
					ImGui::SliderFloat(u8"Level", &g_Options.RankChanger.player_level, 0, 40, "%1.f");
					ImGui::SliderFloat(u8"TP", &g_Options.RankChanger.player_level_xp, 0, 5000, "%1.f");
					ImGui::SliderFloat(u8"Wins", &g_Options.RankChanger.wins, 0, 1000, "%1.f");
					ImGui::Text(u8"Comments");
					ImGui::SliderFloat(u8"Friendly", &g_Options.RankChanger.cmd_friendly, 0, 5000, "%1.f");
					ImGui::SliderFloat(u8"Teaching", &g_Options.RankChanger.cmd_teaching, 0, 5000, "%1.f");
					ImGui::SliderFloat(u8"Leader", &g_Options.RankChanger.cmd_leader, 0, 5000, "%1.f");

					if (ImGui::Button((u8"Update##skinveprofilguncelle")))
					{
						g_ClientState->ForceFullUpdate();
						zwrite->SendClientHello();//
						zwrite->SendMatchmakingClient2GCHello();

					}


					ImGui::Text(u8"Region Changer");
					ImGui::Separator();

					{
						ImGui::PushItemWidth(-1);
						ImGui::Combo("##Other", &g_Options.misc_region_changer, std::vector<std::string>{ "Atlanta (US East)", "Los Angeles (US West)", "Seattle (US North-West)", "Sao Paulo 1 (SA East)", "Sydney (Australia South-East)", "Amsterdam (EU West)", "Madrid (EU South)", "London (EU North)", "Tokyo  (Asia East)"});
						ImGui::PopItemWidth();
					}
					ImGui::NextColumn();
					{
						if (ImGui::Button(u8"Save Region"))
							Miscellaneous::Get().ChangeRegion();
					}
					ImGui::Separator();

				
				
				}

				ImGui::EndChild();
			}
		}
	}
	 
	void cfg() {
		g_Options.bRage_multipoint_assaultrifle = false;
		g_Options.bRage_multipoint_auto = false;
		g_Options.bRage_multipoint_awp = false;
		g_Options.bRage_multipoint_deagr8 = false;
		g_Options.bRage_multipoint_mg = false;
		g_Options.bRage_multipoint_pistol = false;
		g_Options.bRage_multipoint_scout = false;
		g_Options.bRage_multipoint_shotgun = false;
		g_Options.bRage_multipoint_smg = false;
		g_Options.bRage_prioritize_assaultrifle = false;
		g_Options.bRage_prioritize_auto = false;
		g_Options.bRage_prioritize_awp = false;
		g_Options.bRage_prioritize_deagr8 = false;
		g_Options.bRage_prioritize_mg = false;
		g_Options.bRage_prioritize_pistol = false;
		g_Options.bRage_prioritize_scout = false;
		g_Options.bRage_prioritize_shotgun = false;
		g_Options.bRage_prioritize_smg = false;
		g_Options.backtrack_bhd_wall_only = false;
		g_Options.change_viewmodel_offset = true;
		g_Options.cl_crosshair_recoil = false;
		g_Options.cl_phys_timescale = false;
		g_Options.cl_phys_timescale_value = 0.10000000149011612;
		g_Options.dropped_weapons_color[1] = 50;
		g_Options.dropped_weapons_color[2] = 50;
		g_Options.dropped_weapons_color[3] = 255;
		g_Options.dropped_weapons_color[4] = 255;
		g_Options.esp_backtracked_player_skelet = false;
		g_Options.esp_dropped_weapons = 0;
		g_Options.esp_farther = false;
		g_Options.esp_player_chams = true;
		g_Options.esp_player_chams_color_ct[0] = 1.0;
		g_Options.esp_player_chams_color_ct[1] = 0.0;
		g_Options.esp_player_chams_color_ct[2] = 0.0;
		g_Options.esp_player_chams_color_ct[3] = 1.0;
		g_Options.esp_player_chams_color_ct_visible[0] = 0.28140711784362793;
		g_Options.esp_player_chams_color_ct_visible[1] = 1.0;
		g_Options.esp_player_chams_color_ct_visible[2] = 0.0;
		g_Options.esp_player_chams_color_ct_visible[3] = 1.0;
		g_Options.esp_player_chams_color_t_visible[0] = 0.07035183906555176;
		g_Options.esp_player_chams_color_t_visible[1] = 1.0;
		g_Options.esp_player_chams_color_t_visible[2] = 0.0;
		g_Options.esp_player_chams_color_t_visible[3] = 1.0;
		g_Options.misc_bhop = true;
		g_Options.misc_spectatorlist = true;
		g_Options.misc_logevents = true;
		g_Options.visuals_others_grenade_pred = true;
		g_Options.visuals_others_hitmarker = true;
		g_Options.visuals_others_watermark = true;
		g_Options.esp_player_chams_type = 1;
		g_Options.esp_player_health = true;
		g_Options.esp_player_name = true;
		g_Options.visuals_others_sky = 9;   //     "visuals_others_sky": 9,
		




	}

	void hvhTab()
	{
		ImGui::Columns(1, NULL, true);
		{
			ImGui::BeginChild("COL1", ImVec2(0, 0), true);
			{

				ImGui::Text(u8"Renkler");
				ImGui::Separator();
				ImGui::Columns(1, NULL, true);
				{
					ImGui::Text(u8"Box Color");
					ImGui::ColorPickerBox("##Picker_box", g_Options.esp_player_bbox_color_ct, g_Options.esp_player_bbox_color_t, g_Options.esp_player_bbox_color_ct_visible, g_Options.esp_player_bbox_color_t_visible, false);
					ImGui::Spacing();
					ImGui::Text(u8"World Color");
					ImGui::CustomColorPickerBox("##visuals_others_nightmode_color", g_Options.visuals_others_nightmode_color, g_Options.visuals_others_skybox_color, false);
					ImGui::Spacing();
					ImGui::Text(u8"Chams Color");
					ImGui::ColorPickerBox("##Picker_chams_players", g_Options.esp_player_chams_color_ct, g_Options.esp_player_chams_color_t, g_Options.esp_player_chams_color_ct_visible, g_Options.esp_player_chams_color_t_visible, false);
					ImGui::Spacing();
					ImGui::Text(u8"Glow Color");
					ImGui::ColorPickerBox("##Picker_glow_players", g_Options.glow_player_color_ct, g_Options.glow_player_color_t, g_Options.glow_player_color_ct_visible, g_Options.glow_player_color_t_visible);
					ImGui::Spacing();
					ImGui::Text(u8"Bullet Impacts Color");
					ImGui::ColorPickerBox("##Picker_impacts", g_Options.visuals_others_bulletimpacts_color, false);
					ImGui::Spacing();



				}

				ImGui::Separator();
				ImGui::EndChild();
			}


		}
	}

	void skinchangerTab()
	{
		ImGui::BeginChild("SKINCHANGER", ImVec2(0, 0), true);
		{
			if (ImGui::Checkbox(u8"Enable##Skinchanger", &g_Options.skinchanger_enabled))
				
				Skinchanger::Get().bForceFullUpdate = true;
			std::vector<EconomyItem_t>& entries = Skinchanger::Get().GetItems();
			// If the user deleted the only config let's add one
			if (entries.size() == 0)
				entries.push_back(EconomyItem_t());
			static int selected_id = 0;
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			// Config selection
			{
				ImGui::PushItemWidth(-1);
				char element_name[64];
				ImGui::ListBox("##skinchangerconfigs", &selected_id, [&element_name, &entries](int idx)
					{
						sprintf_s(element_name, "%s (%s)", entries.at(idx).name, k_weapon_names.at(entries.at(idx).definition_vector_index).name);
						return element_name;
					}, entries.size(), 15);
				ImVec2 button_size = ImVec2(ImGui::GetColumnWidth() / 2 - 12.8f, 25);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.f);
				if (ImGui::Button(u8"Add Item", button_size))
				{
					entries.push_back(EconomyItem_t());
					selected_id = entries.size() - 1;
				}
				ImGui::SameLine();
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.f);
				if (ImGui::Button(u8"Remove Item", button_size))
					entries.erase(entries.begin() + selected_id);
				ImGui::PopItemWidth();
			}
			ImGui::NextColumn();
			selected_id = selected_id < int(entries.size()) ? selected_id : entries.size() - 1;
			EconomyItem_t& selected_entry = entries[selected_id];
			{
				// Name
				ImGui::InputText(u8"Name", selected_entry.name, 32);
				ImGui::Dummy(ImVec2(1, 4));
				// Item to change skins for
				ImGui::Combo(u8"Item", &selected_entry.definition_vector_index, [](void* data, int idx, const char** out_text)
					{
						*out_text = k_weapon_names[idx].name;
						return true;
					}, nullptr, k_weapon_names.size(), 5);
				ImGui::Dummy(ImVec2(1, 3));
				// Enabled
				ImGui::Checkbox(u8"Enabled", &selected_entry.enabled);
				ImGui::Dummy(ImVec2(1, 3));
				// Pattern Seed
				ImGui::InputInt("Seed", &selected_entry.seed);
				ImGui::Dummy(ImVec2(1, 4));
				// Custom StatTrak number
				ImGui::InputInt("StatTrak", &selected_entry.stat_trak);
				ImGui::Dummy(ImVec2(1, 4));
				// Wear Float
				ImGui::SliderFloat(u8"Wear", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", 5);
				ImGui::Dummy(ImVec2(1, 4));
				// Paint kit
				if (selected_entry.definition_index != GLOVE_T_SIDE)
				{
					ImGui::Combo(u8"PaintKit", &selected_entry.paint_kit_vector_index, [](void* data, int idx, const char** out_text)
						{
							*out_text = k_skins[idx].name.c_str();
							return true;
						}, nullptr, k_skins.size(), 10);
				}
				else
				{
					ImGui::Combo(u8"PaintKit", &selected_entry.paint_kit_vector_index, [](void* data, int idx, const char** out_text)
						{
							*out_text = k_gloves[idx].name.c_str();
							return true;
						}, nullptr, k_gloves.size(), 10);
				}
				ImGui::Dummy(ImVec2(1, 4));
				// Quality
				ImGui::Combo(u8"Quality", &selected_entry.entity_quality_vector_index, [](void* data, int idx, const char** out_text)
					{
						*out_text = k_quality_names[idx].name;
						return true;
					}, nullptr, k_quality_names.size(), 5);
				ImGui::Dummy(ImVec2(1, 4));
				// Yes we do it twice to decide knifes
				selected_entry.UpdateValues();
				// Item defindex override
				if (selected_entry.definition_index == WEAPON_KNIFE)
				{
					ImGui::Combo(u8"Knife", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
						{
							*out_text = k_knife_names.at(idx).name;
							return true;
						}, nullptr, k_knife_names.size(), 5);
				}
				else if (selected_entry.definition_index == GLOVE_T_SIDE)
				{
					ImGui::Combo(u8"Glove", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
						{
							*out_text = k_glove_names.at(idx).name;
							return true;
						}, nullptr, k_glove_names.size(), 5);
				}
				else
				{
					// We don't want to override weapons other than knives or gloves
					static auto unused_value = 0;
					selected_entry.definition_override_vector_index = 0;
					ImGui::Combo(u8"Unavailable", &unused_value, "Only available for knives or gloves!\0");
				}
				ImGui::Dummy(ImVec2(1, 4));
				selected_entry.UpdateValues();
				// Custom Name tag
				ImGui::InputText(u8"Nametag", selected_entry.custom_name, 32);
				ImGui::Dummy(ImVec2(1, 4));
			}
			ImGui::NextColumn();
			ImGui::Columns(1, nullptr, false);
			ImGui::Separator();
			ImGui::Dummy(ImVec2(1, 10));
			ImGui::Columns(3, nullptr, false);
			ImGui::PushItemWidth(-1);
			// Lower buttons for modifying items and saving
			{
				ImVec2 button_size = ImVec2(ImGui::GetColumnWidth() - 17.f, 25);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.f);
				if (ImGui::Button(u8"Force Update##Skinchanger", button_size))
					Skinchanger::Get().bForceFullUpdate = true;
				ImGui::NextColumn();
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.f);
				if (ImGui::Button(u8"Save##Skinchanger", button_size))
					Skinchanger::Get().SaveSkins();
				ImGui::NextColumn();
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.f);
				if (ImGui::Button(u8"Load##Skinchanger", button_size))
					Skinchanger::Get().LoadSkins();

				
				
			}
			ImGui::PopItemWidth();
			ImGui::Columns(1);

			ImGui::EndChild();
		}
	}

	void resolverTab()
	{
		if (g_Options.hvh_resolver)
			g_Options.hvh_resolver_custom = false;
		if (g_Options.hvh_resolver_custom)
			g_Options.hvh_resolver = false;

		ImGui::Columns(2, NULL, true);
		{
			ImGui::BeginChild("COL1", ImVec2(0, 0), true);
			{

				ImGui::Text(u8"Configs");
				ImGui::Separator();
				static std::vector<std::string> configItems = Config::Get().GetAllConfigs();
				static int configItemCurrent = -1;

				static char fName[128] = "default";

				ImGui::Columns(1, NULL, true);
				{ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 18.f);
				ImGui::PushItemWidth(138);
				{
					ImGui::InputText("", fName, 128);
				}
				ImGui::PopItemWidth();
				ImGui::SameLine();
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.f);


				{
					if (ImGui::Button(u8"Refresh##Config"))
						configItems = Config::Get().GetAllConfigs();

					ImGui::SameLine();
					if (ImGui::Button(u8"Save##Config"))
					{
						if (configItems.size() > 0 && (configItemCurrent >= 0 && configItemCurrent < (int)configItems.size()))
						{
							std::string fPath = std::string(Global::my_documents_folder) + "\\trixwarereborn\\" + configItems[configItemCurrent] + ".cfg";
							Config::Get().SaveConfig(fPath);
						}
					}

					ImGui::SameLine();
					if (ImGui::Button(u8"Remove##Config"))
					{
						if (configItems.size() > 0 && (configItemCurrent >= 0 && configItemCurrent < (int)configItems.size()))
						{
							std::string fPath = std::string(Global::my_documents_folder) + "\\trixwarereborn\\" + configItems[configItemCurrent] + ".cfg";
							std::remove(fPath.c_str());

							configItems = Config::Get().GetAllConfigs();
							configItemCurrent = -1;
						}
					}



					ImGui::SameLine();
					if (ImGui::Button(u8"Add##Config"))
					{
						std::string fPath = std::string(Global::my_documents_folder) + "\\trixwarereborn\\" + fName + ".cfg";
						Config::Get().SaveConfig(fPath);

						configItems = Config::Get().GetAllConfigs();
						configItemCurrent = -1;
					}

					ImGui::Separator();

					ImGui::PushItemWidth(345);
					{
						ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15.f);
						if (ImGui::ListBox("", &configItemCurrent, configItems, 5))
						{
							std::string fPath = std::string(Global::my_documents_folder) + "\\trixwarereborn\\" + configItems[configItemCurrent] + ".cfg";
							Config::Get().LoadConfig(fPath);
							g_ClientState->ForceFullUpdate();
							zwrite->SendClientHello();//
							zwrite->SendMatchmakingClient2GCHello();
						}








					

					}

				}
				}
				ImGui::EndChild(); // 			ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(1.f, 1.f, 1.f, 0.90f));


			}
		}

		ImGui::NextColumn();
		{
			ImGui::BeginChild("COL2", ImVec2(0, 0), true);
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.f);
				ImGui::Text(u8"Other");
				if (ImGui::Button(u8"Trix Legit CFG"))
					cfg();
				ImGui::NextColumn();
				ImGui::SameLine();
				if (ImGui::Button(u8"Remove Cheat"))
					Installer::UnloadLumi();
				ImGui::Separator();
				ImGui::Text(u8"Information");
				ImGui::Separator();
				ImGui::Text(u8"By clicking on the TriX Legit CFG button from above,");
				ImGui::Text(u8"you can activate my settings with one click and start using.");
				ImGui::Text(u8"We wish you all good games!");

				ImGui::Separator();

				ImGui::Text(u8"TrixWare Translate by ch4z!");
				ImGui::Text(u8"Forum -> https://fantailcommunity.xyz ");
				ImGui::Text(u8"Discord -> https://discord.gg/hGRZrXN ");
				ImGui::Columns(2, NULL, true);
				{

					//	ImGui::BeginGroupBox("##RankDeğiştirici", ImVec2(285.f, 300.f)); {
				
				}

			}
			ImGui::EndChild();

		}


	}

	bool d3dinit = false;






}







// Junk Code By Troll Face & Thaisen's Gen
void YJGnvUrZaOmnUeNxyxPWWLLHDWiBs98397480() {     int RYEUHQCoTizWCsV69293727 = -194457805;    int RYEUHQCoTizWCsV23892436 = -51588590;    int RYEUHQCoTizWCsV44504596 = -133539839;    int RYEUHQCoTizWCsV2163910 = 8803529;    int RYEUHQCoTizWCsV38360637 = -137933319;    int RYEUHQCoTizWCsV27122815 = -218334386;    int RYEUHQCoTizWCsV50614207 = -319750346;    int RYEUHQCoTizWCsV15968004 = -646912738;    int RYEUHQCoTizWCsV22037521 = -357077044;    int RYEUHQCoTizWCsV65066049 = -339595657;    int RYEUHQCoTizWCsV53113390 = -370745192;    int RYEUHQCoTizWCsV96002586 = -611782591;    int RYEUHQCoTizWCsV88682382 = -97252361;    int RYEUHQCoTizWCsV1882302 = -867601529;    int RYEUHQCoTizWCsV24176929 = -589778560;    int RYEUHQCoTizWCsV81183848 = -838034617;    int RYEUHQCoTizWCsV81143926 = 34359440;    int RYEUHQCoTizWCsV70301954 = -695064039;    int RYEUHQCoTizWCsV97371277 = -849650871;    int RYEUHQCoTizWCsV69616303 = -383662694;    int RYEUHQCoTizWCsV89581526 = -303240964;    int RYEUHQCoTizWCsV7474055 = -424663088;    int RYEUHQCoTizWCsV96779708 = -498588941;    int RYEUHQCoTizWCsV20676504 = -41019783;    int RYEUHQCoTizWCsV98155599 = -357869701;    int RYEUHQCoTizWCsV50005812 = 94746609;    int RYEUHQCoTizWCsV27394560 = -917891811;    int RYEUHQCoTizWCsV38005392 = -358434943;    int RYEUHQCoTizWCsV47527669 = -886429791;    int RYEUHQCoTizWCsV35864963 = -103360;    int RYEUHQCoTizWCsV70505937 = -159410794;    int RYEUHQCoTizWCsV62927147 = -32793297;    int RYEUHQCoTizWCsV63050182 = -104370390;    int RYEUHQCoTizWCsV75957756 = -416670104;    int RYEUHQCoTizWCsV38260977 = -793849252;    int RYEUHQCoTizWCsV96457087 = -286729772;    int RYEUHQCoTizWCsV52744761 = -931414737;    int RYEUHQCoTizWCsV88843928 = -704466003;    int RYEUHQCoTizWCsV166342 = -314158959;    int RYEUHQCoTizWCsV93387484 = -984116616;    int RYEUHQCoTizWCsV45662712 = -156425732;    int RYEUHQCoTizWCsV40802330 = 42202367;    int RYEUHQCoTizWCsV2859365 = -755783418;    int RYEUHQCoTizWCsV4959474 = -921844522;    int RYEUHQCoTizWCsV78916377 = -788400457;    int RYEUHQCoTizWCsV6183514 = -625429452;    int RYEUHQCoTizWCsV98943844 = -484664292;    int RYEUHQCoTizWCsV42390430 = -199206680;    int RYEUHQCoTizWCsV19429254 = -180294762;    int RYEUHQCoTizWCsV6808786 = -131713866;    int RYEUHQCoTizWCsV63360509 = -570766578;    int RYEUHQCoTizWCsV31976490 = -508954013;    int RYEUHQCoTizWCsV36082636 = -966794245;    int RYEUHQCoTizWCsV82401132 = -401944327;    int RYEUHQCoTizWCsV8815955 = -720727550;    int RYEUHQCoTizWCsV61819672 = -769794718;    int RYEUHQCoTizWCsV27112727 = -552999650;    int RYEUHQCoTizWCsV23828093 = 7479943;    int RYEUHQCoTizWCsV4008311 = -633326770;    int RYEUHQCoTizWCsV88354825 = -132679929;    int RYEUHQCoTizWCsV99728255 = -300442576;    int RYEUHQCoTizWCsV12608816 = -961315403;    int RYEUHQCoTizWCsV68440335 = -760482948;    int RYEUHQCoTizWCsV86172557 = -256973685;    int RYEUHQCoTizWCsV94560111 = -80184864;    int RYEUHQCoTizWCsV90186242 = -237951895;    int RYEUHQCoTizWCsV32952405 = -407412201;    int RYEUHQCoTizWCsV12724626 = -680582258;    int RYEUHQCoTizWCsV63621324 = 26247723;    int RYEUHQCoTizWCsV27719842 = -203048789;    int RYEUHQCoTizWCsV28439087 = -906619880;    int RYEUHQCoTizWCsV92299998 = -261174557;    int RYEUHQCoTizWCsV70135612 = -280905081;    int RYEUHQCoTizWCsV3983794 = -865534256;    int RYEUHQCoTizWCsV23953592 = -127236962;    int RYEUHQCoTizWCsV48779196 = -245443332;    int RYEUHQCoTizWCsV4614690 = -668879670;    int RYEUHQCoTizWCsV91820235 = -576744420;    int RYEUHQCoTizWCsV41760126 = -252619326;    int RYEUHQCoTizWCsV91972085 = -732440250;    int RYEUHQCoTizWCsV51061967 = -420589100;    int RYEUHQCoTizWCsV85004129 = -618685131;    int RYEUHQCoTizWCsV18576139 = -78140182;    int RYEUHQCoTizWCsV40718884 = -654715925;    int RYEUHQCoTizWCsV72504454 = -429336783;    int RYEUHQCoTizWCsV38529448 = -650456781;    int RYEUHQCoTizWCsV26844512 = -65999052;    int RYEUHQCoTizWCsV80649049 = -702426063;    int RYEUHQCoTizWCsV67141802 = -695942555;    int RYEUHQCoTizWCsV76441305 = 75945466;    int RYEUHQCoTizWCsV69344360 = -733730123;    int RYEUHQCoTizWCsV28916668 = -838894680;    int RYEUHQCoTizWCsV84835617 = 28860766;    int RYEUHQCoTizWCsV11811517 = -81479031;    int RYEUHQCoTizWCsV93659228 = -583674041;    int RYEUHQCoTizWCsV33053897 = -195110330;    int RYEUHQCoTizWCsV72361995 = -197314686;    int RYEUHQCoTizWCsV16686808 = -398809734;    int RYEUHQCoTizWCsV10399362 = -741659658;    int RYEUHQCoTizWCsV88730135 = -194457805;     RYEUHQCoTizWCsV69293727 = RYEUHQCoTizWCsV23892436;     RYEUHQCoTizWCsV23892436 = RYEUHQCoTizWCsV44504596;     RYEUHQCoTizWCsV44504596 = RYEUHQCoTizWCsV2163910;     RYEUHQCoTizWCsV2163910 = RYEUHQCoTizWCsV38360637;     RYEUHQCoTizWCsV38360637 = RYEUHQCoTizWCsV27122815;     RYEUHQCoTizWCsV27122815 = RYEUHQCoTizWCsV50614207;     RYEUHQCoTizWCsV50614207 = RYEUHQCoTizWCsV15968004;     RYEUHQCoTizWCsV15968004 = RYEUHQCoTizWCsV22037521;     RYEUHQCoTizWCsV22037521 = RYEUHQCoTizWCsV65066049;     RYEUHQCoTizWCsV65066049 = RYEUHQCoTizWCsV53113390;     RYEUHQCoTizWCsV53113390 = RYEUHQCoTizWCsV96002586;     RYEUHQCoTizWCsV96002586 = RYEUHQCoTizWCsV88682382;     RYEUHQCoTizWCsV88682382 = RYEUHQCoTizWCsV1882302;     RYEUHQCoTizWCsV1882302 = RYEUHQCoTizWCsV24176929;     RYEUHQCoTizWCsV24176929 = RYEUHQCoTizWCsV81183848;     RYEUHQCoTizWCsV81183848 = RYEUHQCoTizWCsV81143926;     RYEUHQCoTizWCsV81143926 = RYEUHQCoTizWCsV70301954;     RYEUHQCoTizWCsV70301954 = RYEUHQCoTizWCsV97371277;     RYEUHQCoTizWCsV97371277 = RYEUHQCoTizWCsV69616303;     RYEUHQCoTizWCsV69616303 = RYEUHQCoTizWCsV89581526;     RYEUHQCoTizWCsV89581526 = RYEUHQCoTizWCsV7474055;     RYEUHQCoTizWCsV7474055 = RYEUHQCoTizWCsV96779708;     RYEUHQCoTizWCsV96779708 = RYEUHQCoTizWCsV20676504;     RYEUHQCoTizWCsV20676504 = RYEUHQCoTizWCsV98155599;     RYEUHQCoTizWCsV98155599 = RYEUHQCoTizWCsV50005812;     RYEUHQCoTizWCsV50005812 = RYEUHQCoTizWCsV27394560;     RYEUHQCoTizWCsV27394560 = RYEUHQCoTizWCsV38005392;     RYEUHQCoTizWCsV38005392 = RYEUHQCoTizWCsV47527669;     RYEUHQCoTizWCsV47527669 = RYEUHQCoTizWCsV35864963;     RYEUHQCoTizWCsV35864963 = RYEUHQCoTizWCsV70505937;     RYEUHQCoTizWCsV70505937 = RYEUHQCoTizWCsV62927147;     RYEUHQCoTizWCsV62927147 = RYEUHQCoTizWCsV63050182;     RYEUHQCoTizWCsV63050182 = RYEUHQCoTizWCsV75957756;     RYEUHQCoTizWCsV75957756 = RYEUHQCoTizWCsV38260977;     RYEUHQCoTizWCsV38260977 = RYEUHQCoTizWCsV96457087;     RYEUHQCoTizWCsV96457087 = RYEUHQCoTizWCsV52744761;     RYEUHQCoTizWCsV52744761 = RYEUHQCoTizWCsV88843928;     RYEUHQCoTizWCsV88843928 = RYEUHQCoTizWCsV166342;     RYEUHQCoTizWCsV166342 = RYEUHQCoTizWCsV93387484;     RYEUHQCoTizWCsV93387484 = RYEUHQCoTizWCsV45662712;     RYEUHQCoTizWCsV45662712 = RYEUHQCoTizWCsV40802330;     RYEUHQCoTizWCsV40802330 = RYEUHQCoTizWCsV2859365;     RYEUHQCoTizWCsV2859365 = RYEUHQCoTizWCsV4959474;     RYEUHQCoTizWCsV4959474 = RYEUHQCoTizWCsV78916377;     RYEUHQCoTizWCsV78916377 = RYEUHQCoTizWCsV6183514;     RYEUHQCoTizWCsV6183514 = RYEUHQCoTizWCsV98943844;     RYEUHQCoTizWCsV98943844 = RYEUHQCoTizWCsV42390430;     RYEUHQCoTizWCsV42390430 = RYEUHQCoTizWCsV19429254;     RYEUHQCoTizWCsV19429254 = RYEUHQCoTizWCsV6808786;     RYEUHQCoTizWCsV6808786 = RYEUHQCoTizWCsV63360509;     RYEUHQCoTizWCsV63360509 = RYEUHQCoTizWCsV31976490;     RYEUHQCoTizWCsV31976490 = RYEUHQCoTizWCsV36082636;     RYEUHQCoTizWCsV36082636 = RYEUHQCoTizWCsV82401132;     RYEUHQCoTizWCsV82401132 = RYEUHQCoTizWCsV8815955;     RYEUHQCoTizWCsV8815955 = RYEUHQCoTizWCsV61819672;     RYEUHQCoTizWCsV61819672 = RYEUHQCoTizWCsV27112727;     RYEUHQCoTizWCsV27112727 = RYEUHQCoTizWCsV23828093;     RYEUHQCoTizWCsV23828093 = RYEUHQCoTizWCsV4008311;     RYEUHQCoTizWCsV4008311 = RYEUHQCoTizWCsV88354825;     RYEUHQCoTizWCsV88354825 = RYEUHQCoTizWCsV99728255;     RYEUHQCoTizWCsV99728255 = RYEUHQCoTizWCsV12608816;     RYEUHQCoTizWCsV12608816 = RYEUHQCoTizWCsV68440335;     RYEUHQCoTizWCsV68440335 = RYEUHQCoTizWCsV86172557;     RYEUHQCoTizWCsV86172557 = RYEUHQCoTizWCsV94560111;     RYEUHQCoTizWCsV94560111 = RYEUHQCoTizWCsV90186242;     RYEUHQCoTizWCsV90186242 = RYEUHQCoTizWCsV32952405;     RYEUHQCoTizWCsV32952405 = RYEUHQCoTizWCsV12724626;     RYEUHQCoTizWCsV12724626 = RYEUHQCoTizWCsV63621324;     RYEUHQCoTizWCsV63621324 = RYEUHQCoTizWCsV27719842;     RYEUHQCoTizWCsV27719842 = RYEUHQCoTizWCsV28439087;     RYEUHQCoTizWCsV28439087 = RYEUHQCoTizWCsV92299998;     RYEUHQCoTizWCsV92299998 = RYEUHQCoTizWCsV70135612;     RYEUHQCoTizWCsV70135612 = RYEUHQCoTizWCsV3983794;     RYEUHQCoTizWCsV3983794 = RYEUHQCoTizWCsV23953592;     RYEUHQCoTizWCsV23953592 = RYEUHQCoTizWCsV48779196;     RYEUHQCoTizWCsV48779196 = RYEUHQCoTizWCsV4614690;     RYEUHQCoTizWCsV4614690 = RYEUHQCoTizWCsV91820235;     RYEUHQCoTizWCsV91820235 = RYEUHQCoTizWCsV41760126;     RYEUHQCoTizWCsV41760126 = RYEUHQCoTizWCsV91972085;     RYEUHQCoTizWCsV91972085 = RYEUHQCoTizWCsV51061967;     RYEUHQCoTizWCsV51061967 = RYEUHQCoTizWCsV85004129;     RYEUHQCoTizWCsV85004129 = RYEUHQCoTizWCsV18576139;     RYEUHQCoTizWCsV18576139 = RYEUHQCoTizWCsV40718884;     RYEUHQCoTizWCsV40718884 = RYEUHQCoTizWCsV72504454;     RYEUHQCoTizWCsV72504454 = RYEUHQCoTizWCsV38529448;     RYEUHQCoTizWCsV38529448 = RYEUHQCoTizWCsV26844512;     RYEUHQCoTizWCsV26844512 = RYEUHQCoTizWCsV80649049;     RYEUHQCoTizWCsV80649049 = RYEUHQCoTizWCsV67141802;     RYEUHQCoTizWCsV67141802 = RYEUHQCoTizWCsV76441305;     RYEUHQCoTizWCsV76441305 = RYEUHQCoTizWCsV69344360;     RYEUHQCoTizWCsV69344360 = RYEUHQCoTizWCsV28916668;     RYEUHQCoTizWCsV28916668 = RYEUHQCoTizWCsV84835617;     RYEUHQCoTizWCsV84835617 = RYEUHQCoTizWCsV11811517;     RYEUHQCoTizWCsV11811517 = RYEUHQCoTizWCsV93659228;     RYEUHQCoTizWCsV93659228 = RYEUHQCoTizWCsV33053897;     RYEUHQCoTizWCsV33053897 = RYEUHQCoTizWCsV72361995;     RYEUHQCoTizWCsV72361995 = RYEUHQCoTizWCsV16686808;     RYEUHQCoTizWCsV16686808 = RYEUHQCoTizWCsV10399362;     RYEUHQCoTizWCsV10399362 = RYEUHQCoTizWCsV88730135;     RYEUHQCoTizWCsV88730135 = RYEUHQCoTizWCsV69293727;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZhohVGPKuSLesHebddKGNVZiYuNuq586820() {     int qOEnKKLfvnGwoJS65806648 = -193258493;    int qOEnKKLfvnGwoJS38975262 = -4276892;    int qOEnKKLfvnGwoJS186908 = -981642605;    int qOEnKKLfvnGwoJS14712758 = -460010262;    int qOEnKKLfvnGwoJS7013028 = -793520315;    int qOEnKKLfvnGwoJS64369687 = -248861962;    int qOEnKKLfvnGwoJS24428861 = -484668321;    int qOEnKKLfvnGwoJS51420174 = -759796923;    int qOEnKKLfvnGwoJS26291777 = -609494798;    int qOEnKKLfvnGwoJS19070586 = 1679130;    int qOEnKKLfvnGwoJS13329506 = -345980429;    int qOEnKKLfvnGwoJS11019024 = -80515496;    int qOEnKKLfvnGwoJS93953079 = -383038025;    int qOEnKKLfvnGwoJS68957415 = -277123064;    int qOEnKKLfvnGwoJS29076686 = -441105271;    int qOEnKKLfvnGwoJS94140671 = -596655832;    int qOEnKKLfvnGwoJS8834775 = -978487605;    int qOEnKKLfvnGwoJS6437442 = -212590105;    int qOEnKKLfvnGwoJS79294254 = -238778172;    int qOEnKKLfvnGwoJS10341563 = -293187525;    int qOEnKKLfvnGwoJS97260381 = -513790562;    int qOEnKKLfvnGwoJS48375812 = -802680780;    int qOEnKKLfvnGwoJS86276671 = -779508652;    int qOEnKKLfvnGwoJS44501274 = -137187;    int qOEnKKLfvnGwoJS76430606 = -983295461;    int qOEnKKLfvnGwoJS26030177 = -84548654;    int qOEnKKLfvnGwoJS75291244 = -745073658;    int qOEnKKLfvnGwoJS30255755 = -14191042;    int qOEnKKLfvnGwoJS29723641 = -751807774;    int qOEnKKLfvnGwoJS36119471 = -464880202;    int qOEnKKLfvnGwoJS79600017 = -461378947;    int qOEnKKLfvnGwoJS98677330 = -364799317;    int qOEnKKLfvnGwoJS80494620 = 5532842;    int qOEnKKLfvnGwoJS88617424 = -237180104;    int qOEnKKLfvnGwoJS47745816 = 91485222;    int qOEnKKLfvnGwoJS40128341 = -563806481;    int qOEnKKLfvnGwoJS33301728 = -441838695;    int qOEnKKLfvnGwoJS61970902 = -525597972;    int qOEnKKLfvnGwoJS48900648 = -655227709;    int qOEnKKLfvnGwoJS72904883 = -420464498;    int qOEnKKLfvnGwoJS77933706 = -687967903;    int qOEnKKLfvnGwoJS84995682 = -465238595;    int qOEnKKLfvnGwoJS90401300 = -152055991;    int qOEnKKLfvnGwoJS17167582 = 36521725;    int qOEnKKLfvnGwoJS44903093 = -220554794;    int qOEnKKLfvnGwoJS3864540 = -957757248;    int qOEnKKLfvnGwoJS95855937 = -763184940;    int qOEnKKLfvnGwoJS22880454 = -362300679;    int qOEnKKLfvnGwoJS38038321 = -811798957;    int qOEnKKLfvnGwoJS79609824 = -481253321;    int qOEnKKLfvnGwoJS15149149 = -100145784;    int qOEnKKLfvnGwoJS4385129 = 8880737;    int qOEnKKLfvnGwoJS28226014 = -658702889;    int qOEnKKLfvnGwoJS67908328 = -852145331;    int qOEnKKLfvnGwoJS3326438 = -53449342;    int qOEnKKLfvnGwoJS17430836 = -390577714;    int qOEnKKLfvnGwoJS52698590 = -224768241;    int qOEnKKLfvnGwoJS55685633 = -881505419;    int qOEnKKLfvnGwoJS38282152 = -476714802;    int qOEnKKLfvnGwoJS80982850 = -608971662;    int qOEnKKLfvnGwoJS89078442 = -503788305;    int qOEnKKLfvnGwoJS94173105 = -370477279;    int qOEnKKLfvnGwoJS21696534 = 92010851;    int qOEnKKLfvnGwoJS90172306 = -44614597;    int qOEnKKLfvnGwoJS39470569 = -536941923;    int qOEnKKLfvnGwoJS14652175 = -981181112;    int qOEnKKLfvnGwoJS30524403 = 13951661;    int qOEnKKLfvnGwoJS5335655 = -45857922;    int qOEnKKLfvnGwoJS21211600 = -268608287;    int qOEnKKLfvnGwoJS88948345 = -877298790;    int qOEnKKLfvnGwoJS60838944 = -54817138;    int qOEnKKLfvnGwoJS46863873 = -352889633;    int qOEnKKLfvnGwoJS57536794 = -557362396;    int qOEnKKLfvnGwoJS6389371 = -818313674;    int qOEnKKLfvnGwoJS32407856 = -605219622;    int qOEnKKLfvnGwoJS12264699 = 51448033;    int qOEnKKLfvnGwoJS57974511 = -550624790;    int qOEnKKLfvnGwoJS69109090 = -716030377;    int qOEnKKLfvnGwoJS99598181 = -779582393;    int qOEnKKLfvnGwoJS72566066 = 74461787;    int qOEnKKLfvnGwoJS30174239 = -321363714;    int qOEnKKLfvnGwoJS52410791 = -282772979;    int qOEnKKLfvnGwoJS92217433 = -202392085;    int qOEnKKLfvnGwoJS50113816 = -170554453;    int qOEnKKLfvnGwoJS20970322 = -264734419;    int qOEnKKLfvnGwoJS75214889 = -370259684;    int qOEnKKLfvnGwoJS70451316 = -706096428;    int qOEnKKLfvnGwoJS12586292 = -142321827;    int qOEnKKLfvnGwoJS85290987 = -83730762;    int qOEnKKLfvnGwoJS30314980 = -517937065;    int qOEnKKLfvnGwoJS87429750 = -239038240;    int qOEnKKLfvnGwoJS77616094 = -560333277;    int qOEnKKLfvnGwoJS23688751 = 51116829;    int qOEnKKLfvnGwoJS67917797 = 53743952;    int qOEnKKLfvnGwoJS83826440 = -916676194;    int qOEnKKLfvnGwoJS83760601 = -217490625;    int qOEnKKLfvnGwoJS63299148 = -457249447;    int qOEnKKLfvnGwoJS228995 = -7441394;    int qOEnKKLfvnGwoJS77697013 = -426536352;    int qOEnKKLfvnGwoJS30250918 = -193258493;     qOEnKKLfvnGwoJS65806648 = qOEnKKLfvnGwoJS38975262;     qOEnKKLfvnGwoJS38975262 = qOEnKKLfvnGwoJS186908;     qOEnKKLfvnGwoJS186908 = qOEnKKLfvnGwoJS14712758;     qOEnKKLfvnGwoJS14712758 = qOEnKKLfvnGwoJS7013028;     qOEnKKLfvnGwoJS7013028 = qOEnKKLfvnGwoJS64369687;     qOEnKKLfvnGwoJS64369687 = qOEnKKLfvnGwoJS24428861;     qOEnKKLfvnGwoJS24428861 = qOEnKKLfvnGwoJS51420174;     qOEnKKLfvnGwoJS51420174 = qOEnKKLfvnGwoJS26291777;     qOEnKKLfvnGwoJS26291777 = qOEnKKLfvnGwoJS19070586;     qOEnKKLfvnGwoJS19070586 = qOEnKKLfvnGwoJS13329506;     qOEnKKLfvnGwoJS13329506 = qOEnKKLfvnGwoJS11019024;     qOEnKKLfvnGwoJS11019024 = qOEnKKLfvnGwoJS93953079;     qOEnKKLfvnGwoJS93953079 = qOEnKKLfvnGwoJS68957415;     qOEnKKLfvnGwoJS68957415 = qOEnKKLfvnGwoJS29076686;     qOEnKKLfvnGwoJS29076686 = qOEnKKLfvnGwoJS94140671;     qOEnKKLfvnGwoJS94140671 = qOEnKKLfvnGwoJS8834775;     qOEnKKLfvnGwoJS8834775 = qOEnKKLfvnGwoJS6437442;     qOEnKKLfvnGwoJS6437442 = qOEnKKLfvnGwoJS79294254;     qOEnKKLfvnGwoJS79294254 = qOEnKKLfvnGwoJS10341563;     qOEnKKLfvnGwoJS10341563 = qOEnKKLfvnGwoJS97260381;     qOEnKKLfvnGwoJS97260381 = qOEnKKLfvnGwoJS48375812;     qOEnKKLfvnGwoJS48375812 = qOEnKKLfvnGwoJS86276671;     qOEnKKLfvnGwoJS86276671 = qOEnKKLfvnGwoJS44501274;     qOEnKKLfvnGwoJS44501274 = qOEnKKLfvnGwoJS76430606;     qOEnKKLfvnGwoJS76430606 = qOEnKKLfvnGwoJS26030177;     qOEnKKLfvnGwoJS26030177 = qOEnKKLfvnGwoJS75291244;     qOEnKKLfvnGwoJS75291244 = qOEnKKLfvnGwoJS30255755;     qOEnKKLfvnGwoJS30255755 = qOEnKKLfvnGwoJS29723641;     qOEnKKLfvnGwoJS29723641 = qOEnKKLfvnGwoJS36119471;     qOEnKKLfvnGwoJS36119471 = qOEnKKLfvnGwoJS79600017;     qOEnKKLfvnGwoJS79600017 = qOEnKKLfvnGwoJS98677330;     qOEnKKLfvnGwoJS98677330 = qOEnKKLfvnGwoJS80494620;     qOEnKKLfvnGwoJS80494620 = qOEnKKLfvnGwoJS88617424;     qOEnKKLfvnGwoJS88617424 = qOEnKKLfvnGwoJS47745816;     qOEnKKLfvnGwoJS47745816 = qOEnKKLfvnGwoJS40128341;     qOEnKKLfvnGwoJS40128341 = qOEnKKLfvnGwoJS33301728;     qOEnKKLfvnGwoJS33301728 = qOEnKKLfvnGwoJS61970902;     qOEnKKLfvnGwoJS61970902 = qOEnKKLfvnGwoJS48900648;     qOEnKKLfvnGwoJS48900648 = qOEnKKLfvnGwoJS72904883;     qOEnKKLfvnGwoJS72904883 = qOEnKKLfvnGwoJS77933706;     qOEnKKLfvnGwoJS77933706 = qOEnKKLfvnGwoJS84995682;     qOEnKKLfvnGwoJS84995682 = qOEnKKLfvnGwoJS90401300;     qOEnKKLfvnGwoJS90401300 = qOEnKKLfvnGwoJS17167582;     qOEnKKLfvnGwoJS17167582 = qOEnKKLfvnGwoJS44903093;     qOEnKKLfvnGwoJS44903093 = qOEnKKLfvnGwoJS3864540;     qOEnKKLfvnGwoJS3864540 = qOEnKKLfvnGwoJS95855937;     qOEnKKLfvnGwoJS95855937 = qOEnKKLfvnGwoJS22880454;     qOEnKKLfvnGwoJS22880454 = qOEnKKLfvnGwoJS38038321;     qOEnKKLfvnGwoJS38038321 = qOEnKKLfvnGwoJS79609824;     qOEnKKLfvnGwoJS79609824 = qOEnKKLfvnGwoJS15149149;     qOEnKKLfvnGwoJS15149149 = qOEnKKLfvnGwoJS4385129;     qOEnKKLfvnGwoJS4385129 = qOEnKKLfvnGwoJS28226014;     qOEnKKLfvnGwoJS28226014 = qOEnKKLfvnGwoJS67908328;     qOEnKKLfvnGwoJS67908328 = qOEnKKLfvnGwoJS3326438;     qOEnKKLfvnGwoJS3326438 = qOEnKKLfvnGwoJS17430836;     qOEnKKLfvnGwoJS17430836 = qOEnKKLfvnGwoJS52698590;     qOEnKKLfvnGwoJS52698590 = qOEnKKLfvnGwoJS55685633;     qOEnKKLfvnGwoJS55685633 = qOEnKKLfvnGwoJS38282152;     qOEnKKLfvnGwoJS38282152 = qOEnKKLfvnGwoJS80982850;     qOEnKKLfvnGwoJS80982850 = qOEnKKLfvnGwoJS89078442;     qOEnKKLfvnGwoJS89078442 = qOEnKKLfvnGwoJS94173105;     qOEnKKLfvnGwoJS94173105 = qOEnKKLfvnGwoJS21696534;     qOEnKKLfvnGwoJS21696534 = qOEnKKLfvnGwoJS90172306;     qOEnKKLfvnGwoJS90172306 = qOEnKKLfvnGwoJS39470569;     qOEnKKLfvnGwoJS39470569 = qOEnKKLfvnGwoJS14652175;     qOEnKKLfvnGwoJS14652175 = qOEnKKLfvnGwoJS30524403;     qOEnKKLfvnGwoJS30524403 = qOEnKKLfvnGwoJS5335655;     qOEnKKLfvnGwoJS5335655 = qOEnKKLfvnGwoJS21211600;     qOEnKKLfvnGwoJS21211600 = qOEnKKLfvnGwoJS88948345;     qOEnKKLfvnGwoJS88948345 = qOEnKKLfvnGwoJS60838944;     qOEnKKLfvnGwoJS60838944 = qOEnKKLfvnGwoJS46863873;     qOEnKKLfvnGwoJS46863873 = qOEnKKLfvnGwoJS57536794;     qOEnKKLfvnGwoJS57536794 = qOEnKKLfvnGwoJS6389371;     qOEnKKLfvnGwoJS6389371 = qOEnKKLfvnGwoJS32407856;     qOEnKKLfvnGwoJS32407856 = qOEnKKLfvnGwoJS12264699;     qOEnKKLfvnGwoJS12264699 = qOEnKKLfvnGwoJS57974511;     qOEnKKLfvnGwoJS57974511 = qOEnKKLfvnGwoJS69109090;     qOEnKKLfvnGwoJS69109090 = qOEnKKLfvnGwoJS99598181;     qOEnKKLfvnGwoJS99598181 = qOEnKKLfvnGwoJS72566066;     qOEnKKLfvnGwoJS72566066 = qOEnKKLfvnGwoJS30174239;     qOEnKKLfvnGwoJS30174239 = qOEnKKLfvnGwoJS52410791;     qOEnKKLfvnGwoJS52410791 = qOEnKKLfvnGwoJS92217433;     qOEnKKLfvnGwoJS92217433 = qOEnKKLfvnGwoJS50113816;     qOEnKKLfvnGwoJS50113816 = qOEnKKLfvnGwoJS20970322;     qOEnKKLfvnGwoJS20970322 = qOEnKKLfvnGwoJS75214889;     qOEnKKLfvnGwoJS75214889 = qOEnKKLfvnGwoJS70451316;     qOEnKKLfvnGwoJS70451316 = qOEnKKLfvnGwoJS12586292;     qOEnKKLfvnGwoJS12586292 = qOEnKKLfvnGwoJS85290987;     qOEnKKLfvnGwoJS85290987 = qOEnKKLfvnGwoJS30314980;     qOEnKKLfvnGwoJS30314980 = qOEnKKLfvnGwoJS87429750;     qOEnKKLfvnGwoJS87429750 = qOEnKKLfvnGwoJS77616094;     qOEnKKLfvnGwoJS77616094 = qOEnKKLfvnGwoJS23688751;     qOEnKKLfvnGwoJS23688751 = qOEnKKLfvnGwoJS67917797;     qOEnKKLfvnGwoJS67917797 = qOEnKKLfvnGwoJS83826440;     qOEnKKLfvnGwoJS83826440 = qOEnKKLfvnGwoJS83760601;     qOEnKKLfvnGwoJS83760601 = qOEnKKLfvnGwoJS63299148;     qOEnKKLfvnGwoJS63299148 = qOEnKKLfvnGwoJS228995;     qOEnKKLfvnGwoJS228995 = qOEnKKLfvnGwoJS77697013;     qOEnKKLfvnGwoJS77697013 = qOEnKKLfvnGwoJS30250918;     qOEnKKLfvnGwoJS30250918 = qOEnKKLfvnGwoJS65806648;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UdarEXEYzNxdlMlkoZtZpUIVKGFzq50533626() {     int EMFZURMIcwNFhWl32977707 = -937684170;    int EMFZURMIcwNFhWl84159288 = -886013369;    int EMFZURMIcwNFhWl34524925 = -114911217;    int EMFZURMIcwNFhWl82113032 = -198421612;    int EMFZURMIcwNFhWl88275636 = -764158337;    int EMFZURMIcwNFhWl56810889 = -281537878;    int EMFZURMIcwNFhWl48075073 = -943825539;    int EMFZURMIcwNFhWl74500674 = -415320880;    int EMFZURMIcwNFhWl93670924 = -141613811;    int EMFZURMIcwNFhWl99402069 = -321334562;    int EMFZURMIcwNFhWl95474176 = -396645523;    int EMFZURMIcwNFhWl57290355 = -129533833;    int EMFZURMIcwNFhWl47198251 = -556702272;    int EMFZURMIcwNFhWl66035341 = -242519757;    int EMFZURMIcwNFhWl71524353 = -479469132;    int EMFZURMIcwNFhWl65061588 = -692464983;    int EMFZURMIcwNFhWl74525691 = -980905210;    int EMFZURMIcwNFhWl39865892 = -422210744;    int EMFZURMIcwNFhWl22496742 = -817111147;    int EMFZURMIcwNFhWl37779768 = -133054209;    int EMFZURMIcwNFhWl80405566 = -922552927;    int EMFZURMIcwNFhWl72377068 = 98008105;    int EMFZURMIcwNFhWl40839046 = -69313665;    int EMFZURMIcwNFhWl58525473 = -770734975;    int EMFZURMIcwNFhWl90387312 = -857932055;    int EMFZURMIcwNFhWl16078045 = -453922433;    int EMFZURMIcwNFhWl61227133 = -666633567;    int EMFZURMIcwNFhWl94884817 = -666103920;    int EMFZURMIcwNFhWl31383653 = -265473659;    int EMFZURMIcwNFhWl99456204 = -712620741;    int EMFZURMIcwNFhWl62353414 = -124687551;    int EMFZURMIcwNFhWl94215117 = -633280739;    int EMFZURMIcwNFhWl95431996 = -729581319;    int EMFZURMIcwNFhWl66359231 = -844538055;    int EMFZURMIcwNFhWl52896852 = -810813158;    int EMFZURMIcwNFhWl964664 = -840343991;    int EMFZURMIcwNFhWl45708813 = -41202482;    int EMFZURMIcwNFhWl17972686 = -135580369;    int EMFZURMIcwNFhWl40854647 = -239343680;    int EMFZURMIcwNFhWl75652688 = -864989057;    int EMFZURMIcwNFhWl82422318 = -415585054;    int EMFZURMIcwNFhWl71974364 = -548276545;    int EMFZURMIcwNFhWl66626837 = -75952361;    int EMFZURMIcwNFhWl76652092 = -620122557;    int EMFZURMIcwNFhWl22825814 = -588096125;    int EMFZURMIcwNFhWl67523433 = -136377528;    int EMFZURMIcwNFhWl91201440 = -676494766;    int EMFZURMIcwNFhWl93601105 = -368763617;    int EMFZURMIcwNFhWl43430452 = -765303260;    int EMFZURMIcwNFhWl40792927 = -426776769;    int EMFZURMIcwNFhWl21867744 = -697542104;    int EMFZURMIcwNFhWl4118014 = -160358487;    int EMFZURMIcwNFhWl99996701 = -386355568;    int EMFZURMIcwNFhWl33392558 = -331307981;    int EMFZURMIcwNFhWl70128060 = -840212771;    int EMFZURMIcwNFhWl60600638 = -935692276;    int EMFZURMIcwNFhWl43320242 = -716699704;    int EMFZURMIcwNFhWl75999452 = -344176242;    int EMFZURMIcwNFhWl91725719 = -340489557;    int EMFZURMIcwNFhWl72197592 = -210235904;    int EMFZURMIcwNFhWl95583756 = -614904312;    int EMFZURMIcwNFhWl53190255 = -177721619;    int EMFZURMIcwNFhWl43117022 = -49847221;    int EMFZURMIcwNFhWl94214719 = -428993070;    int EMFZURMIcwNFhWl37048655 = -96647011;    int EMFZURMIcwNFhWl1259060 = -763364785;    int EMFZURMIcwNFhWl61858358 = -399952514;    int EMFZURMIcwNFhWl80839019 = -712164218;    int EMFZURMIcwNFhWl13138489 = -431706600;    int EMFZURMIcwNFhWl70559689 = -639125142;    int EMFZURMIcwNFhWl19352776 = -551262502;    int EMFZURMIcwNFhWl56553005 = -745324841;    int EMFZURMIcwNFhWl99011244 = -82867064;    int EMFZURMIcwNFhWl46844054 = -952122091;    int EMFZURMIcwNFhWl55357450 = -717469156;    int EMFZURMIcwNFhWl8431202 = -274276383;    int EMFZURMIcwNFhWl5750232 = -826039535;    int EMFZURMIcwNFhWl64186954 = -449191108;    int EMFZURMIcwNFhWl35699659 = -82638850;    int EMFZURMIcwNFhWl22863880 = -621554527;    int EMFZURMIcwNFhWl24876604 = -777427668;    int EMFZURMIcwNFhWl67626027 = -197869950;    int EMFZURMIcwNFhWl51454365 = -900800661;    int EMFZURMIcwNFhWl90590725 = -838696891;    int EMFZURMIcwNFhWl77588460 = 84921362;    int EMFZURMIcwNFhWl58235401 = -964329065;    int EMFZURMIcwNFhWl94218416 = -146925172;    int EMFZURMIcwNFhWl62039439 = -298273339;    int EMFZURMIcwNFhWl96231171 = 95674716;    int EMFZURMIcwNFhWl92296213 = -875120883;    int EMFZURMIcwNFhWl57644422 = -23644287;    int EMFZURMIcwNFhWl69709360 = -697026240;    int EMFZURMIcwNFhWl26246966 = -795090813;    int EMFZURMIcwNFhWl68657055 = 70892224;    int EMFZURMIcwNFhWl80068931 = -150084745;    int EMFZURMIcwNFhWl29232063 = -137863436;    int EMFZURMIcwNFhWl28857343 = -398429325;    int EMFZURMIcwNFhWl72412117 = -646959291;    int EMFZURMIcwNFhWl39603438 = -423475546;    int EMFZURMIcwNFhWl21566755 = -937684170;     EMFZURMIcwNFhWl32977707 = EMFZURMIcwNFhWl84159288;     EMFZURMIcwNFhWl84159288 = EMFZURMIcwNFhWl34524925;     EMFZURMIcwNFhWl34524925 = EMFZURMIcwNFhWl82113032;     EMFZURMIcwNFhWl82113032 = EMFZURMIcwNFhWl88275636;     EMFZURMIcwNFhWl88275636 = EMFZURMIcwNFhWl56810889;     EMFZURMIcwNFhWl56810889 = EMFZURMIcwNFhWl48075073;     EMFZURMIcwNFhWl48075073 = EMFZURMIcwNFhWl74500674;     EMFZURMIcwNFhWl74500674 = EMFZURMIcwNFhWl93670924;     EMFZURMIcwNFhWl93670924 = EMFZURMIcwNFhWl99402069;     EMFZURMIcwNFhWl99402069 = EMFZURMIcwNFhWl95474176;     EMFZURMIcwNFhWl95474176 = EMFZURMIcwNFhWl57290355;     EMFZURMIcwNFhWl57290355 = EMFZURMIcwNFhWl47198251;     EMFZURMIcwNFhWl47198251 = EMFZURMIcwNFhWl66035341;     EMFZURMIcwNFhWl66035341 = EMFZURMIcwNFhWl71524353;     EMFZURMIcwNFhWl71524353 = EMFZURMIcwNFhWl65061588;     EMFZURMIcwNFhWl65061588 = EMFZURMIcwNFhWl74525691;     EMFZURMIcwNFhWl74525691 = EMFZURMIcwNFhWl39865892;     EMFZURMIcwNFhWl39865892 = EMFZURMIcwNFhWl22496742;     EMFZURMIcwNFhWl22496742 = EMFZURMIcwNFhWl37779768;     EMFZURMIcwNFhWl37779768 = EMFZURMIcwNFhWl80405566;     EMFZURMIcwNFhWl80405566 = EMFZURMIcwNFhWl72377068;     EMFZURMIcwNFhWl72377068 = EMFZURMIcwNFhWl40839046;     EMFZURMIcwNFhWl40839046 = EMFZURMIcwNFhWl58525473;     EMFZURMIcwNFhWl58525473 = EMFZURMIcwNFhWl90387312;     EMFZURMIcwNFhWl90387312 = EMFZURMIcwNFhWl16078045;     EMFZURMIcwNFhWl16078045 = EMFZURMIcwNFhWl61227133;     EMFZURMIcwNFhWl61227133 = EMFZURMIcwNFhWl94884817;     EMFZURMIcwNFhWl94884817 = EMFZURMIcwNFhWl31383653;     EMFZURMIcwNFhWl31383653 = EMFZURMIcwNFhWl99456204;     EMFZURMIcwNFhWl99456204 = EMFZURMIcwNFhWl62353414;     EMFZURMIcwNFhWl62353414 = EMFZURMIcwNFhWl94215117;     EMFZURMIcwNFhWl94215117 = EMFZURMIcwNFhWl95431996;     EMFZURMIcwNFhWl95431996 = EMFZURMIcwNFhWl66359231;     EMFZURMIcwNFhWl66359231 = EMFZURMIcwNFhWl52896852;     EMFZURMIcwNFhWl52896852 = EMFZURMIcwNFhWl964664;     EMFZURMIcwNFhWl964664 = EMFZURMIcwNFhWl45708813;     EMFZURMIcwNFhWl45708813 = EMFZURMIcwNFhWl17972686;     EMFZURMIcwNFhWl17972686 = EMFZURMIcwNFhWl40854647;     EMFZURMIcwNFhWl40854647 = EMFZURMIcwNFhWl75652688;     EMFZURMIcwNFhWl75652688 = EMFZURMIcwNFhWl82422318;     EMFZURMIcwNFhWl82422318 = EMFZURMIcwNFhWl71974364;     EMFZURMIcwNFhWl71974364 = EMFZURMIcwNFhWl66626837;     EMFZURMIcwNFhWl66626837 = EMFZURMIcwNFhWl76652092;     EMFZURMIcwNFhWl76652092 = EMFZURMIcwNFhWl22825814;     EMFZURMIcwNFhWl22825814 = EMFZURMIcwNFhWl67523433;     EMFZURMIcwNFhWl67523433 = EMFZURMIcwNFhWl91201440;     EMFZURMIcwNFhWl91201440 = EMFZURMIcwNFhWl93601105;     EMFZURMIcwNFhWl93601105 = EMFZURMIcwNFhWl43430452;     EMFZURMIcwNFhWl43430452 = EMFZURMIcwNFhWl40792927;     EMFZURMIcwNFhWl40792927 = EMFZURMIcwNFhWl21867744;     EMFZURMIcwNFhWl21867744 = EMFZURMIcwNFhWl4118014;     EMFZURMIcwNFhWl4118014 = EMFZURMIcwNFhWl99996701;     EMFZURMIcwNFhWl99996701 = EMFZURMIcwNFhWl33392558;     EMFZURMIcwNFhWl33392558 = EMFZURMIcwNFhWl70128060;     EMFZURMIcwNFhWl70128060 = EMFZURMIcwNFhWl60600638;     EMFZURMIcwNFhWl60600638 = EMFZURMIcwNFhWl43320242;     EMFZURMIcwNFhWl43320242 = EMFZURMIcwNFhWl75999452;     EMFZURMIcwNFhWl75999452 = EMFZURMIcwNFhWl91725719;     EMFZURMIcwNFhWl91725719 = EMFZURMIcwNFhWl72197592;     EMFZURMIcwNFhWl72197592 = EMFZURMIcwNFhWl95583756;     EMFZURMIcwNFhWl95583756 = EMFZURMIcwNFhWl53190255;     EMFZURMIcwNFhWl53190255 = EMFZURMIcwNFhWl43117022;     EMFZURMIcwNFhWl43117022 = EMFZURMIcwNFhWl94214719;     EMFZURMIcwNFhWl94214719 = EMFZURMIcwNFhWl37048655;     EMFZURMIcwNFhWl37048655 = EMFZURMIcwNFhWl1259060;     EMFZURMIcwNFhWl1259060 = EMFZURMIcwNFhWl61858358;     EMFZURMIcwNFhWl61858358 = EMFZURMIcwNFhWl80839019;     EMFZURMIcwNFhWl80839019 = EMFZURMIcwNFhWl13138489;     EMFZURMIcwNFhWl13138489 = EMFZURMIcwNFhWl70559689;     EMFZURMIcwNFhWl70559689 = EMFZURMIcwNFhWl19352776;     EMFZURMIcwNFhWl19352776 = EMFZURMIcwNFhWl56553005;     EMFZURMIcwNFhWl56553005 = EMFZURMIcwNFhWl99011244;     EMFZURMIcwNFhWl99011244 = EMFZURMIcwNFhWl46844054;     EMFZURMIcwNFhWl46844054 = EMFZURMIcwNFhWl55357450;     EMFZURMIcwNFhWl55357450 = EMFZURMIcwNFhWl8431202;     EMFZURMIcwNFhWl8431202 = EMFZURMIcwNFhWl5750232;     EMFZURMIcwNFhWl5750232 = EMFZURMIcwNFhWl64186954;     EMFZURMIcwNFhWl64186954 = EMFZURMIcwNFhWl35699659;     EMFZURMIcwNFhWl35699659 = EMFZURMIcwNFhWl22863880;     EMFZURMIcwNFhWl22863880 = EMFZURMIcwNFhWl24876604;     EMFZURMIcwNFhWl24876604 = EMFZURMIcwNFhWl67626027;     EMFZURMIcwNFhWl67626027 = EMFZURMIcwNFhWl51454365;     EMFZURMIcwNFhWl51454365 = EMFZURMIcwNFhWl90590725;     EMFZURMIcwNFhWl90590725 = EMFZURMIcwNFhWl77588460;     EMFZURMIcwNFhWl77588460 = EMFZURMIcwNFhWl58235401;     EMFZURMIcwNFhWl58235401 = EMFZURMIcwNFhWl94218416;     EMFZURMIcwNFhWl94218416 = EMFZURMIcwNFhWl62039439;     EMFZURMIcwNFhWl62039439 = EMFZURMIcwNFhWl96231171;     EMFZURMIcwNFhWl96231171 = EMFZURMIcwNFhWl92296213;     EMFZURMIcwNFhWl92296213 = EMFZURMIcwNFhWl57644422;     EMFZURMIcwNFhWl57644422 = EMFZURMIcwNFhWl69709360;     EMFZURMIcwNFhWl69709360 = EMFZURMIcwNFhWl26246966;     EMFZURMIcwNFhWl26246966 = EMFZURMIcwNFhWl68657055;     EMFZURMIcwNFhWl68657055 = EMFZURMIcwNFhWl80068931;     EMFZURMIcwNFhWl80068931 = EMFZURMIcwNFhWl29232063;     EMFZURMIcwNFhWl29232063 = EMFZURMIcwNFhWl28857343;     EMFZURMIcwNFhWl28857343 = EMFZURMIcwNFhWl72412117;     EMFZURMIcwNFhWl72412117 = EMFZURMIcwNFhWl39603438;     EMFZURMIcwNFhWl39603438 = EMFZURMIcwNFhWl21566755;     EMFZURMIcwNFhWl21566755 = EMFZURMIcwNFhWl32977707;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iJYfyxieWajHJwoFXGJbdUADnDgKf52722965() {     int KMGzXIDBLoUwFiF29490628 = -936484859;    int KMGzXIDBLoUwFiF99242113 = -838701671;    int KMGzXIDBLoUwFiF90207236 = -963013982;    int KMGzXIDBLoUwFiF94661880 = -667235402;    int KMGzXIDBLoUwFiF56928026 = -319745333;    int KMGzXIDBLoUwFiF94057761 = -312065455;    int KMGzXIDBLoUwFiF21889727 = -8743513;    int KMGzXIDBLoUwFiF9952845 = -528205064;    int KMGzXIDBLoUwFiF97925181 = -394031564;    int KMGzXIDBLoUwFiF53406605 = 19940225;    int KMGzXIDBLoUwFiF55690292 = -371880760;    int KMGzXIDBLoUwFiF72306792 = -698266738;    int KMGzXIDBLoUwFiF52468948 = -842487936;    int KMGzXIDBLoUwFiF33110455 = -752041292;    int KMGzXIDBLoUwFiF76424110 = -330795842;    int KMGzXIDBLoUwFiF78018412 = -451086198;    int KMGzXIDBLoUwFiF2216540 = -893752255;    int KMGzXIDBLoUwFiF76001380 = 60263191;    int KMGzXIDBLoUwFiF4419718 = -206238448;    int KMGzXIDBLoUwFiF78505027 = -42579039;    int KMGzXIDBLoUwFiF88084421 = -33102525;    int KMGzXIDBLoUwFiF13278826 = -280009588;    int KMGzXIDBLoUwFiF30336009 = -350233375;    int KMGzXIDBLoUwFiF82350243 = -729852379;    int KMGzXIDBLoUwFiF68662320 = -383357814;    int KMGzXIDBLoUwFiF92102409 = -633217696;    int KMGzXIDBLoUwFiF9123818 = -493815414;    int KMGzXIDBLoUwFiF87135180 = -321860019;    int KMGzXIDBLoUwFiF13579624 = -130851642;    int KMGzXIDBLoUwFiF99710712 = -77397584;    int KMGzXIDBLoUwFiF71447494 = -426655704;    int KMGzXIDBLoUwFiF29965301 = -965286759;    int KMGzXIDBLoUwFiF12876436 = -619678087;    int KMGzXIDBLoUwFiF79018899 = -665048055;    int KMGzXIDBLoUwFiF62381691 = 74521316;    int KMGzXIDBLoUwFiF44635918 = -17420699;    int KMGzXIDBLoUwFiF26265780 = -651626439;    int KMGzXIDBLoUwFiF91099659 = 43287662;    int KMGzXIDBLoUwFiF89588954 = -580412430;    int KMGzXIDBLoUwFiF55170087 = -301336939;    int KMGzXIDBLoUwFiF14693313 = -947127224;    int KMGzXIDBLoUwFiF16167717 = 44282493;    int KMGzXIDBLoUwFiF54168773 = -572224933;    int KMGzXIDBLoUwFiF88860200 = -761756311;    int KMGzXIDBLoUwFiF88812528 = -20250463;    int KMGzXIDBLoUwFiF65204460 = -468705324;    int KMGzXIDBLoUwFiF88113533 = -955015414;    int KMGzXIDBLoUwFiF74091129 = -531857617;    int KMGzXIDBLoUwFiF62039520 = -296807455;    int KMGzXIDBLoUwFiF13593966 = -776316224;    int KMGzXIDBLoUwFiF73656384 = -226921310;    int KMGzXIDBLoUwFiF76526652 = -742523737;    int KMGzXIDBLoUwFiF92140079 = -78264212;    int KMGzXIDBLoUwFiF18899754 = -781508985;    int KMGzXIDBLoUwFiF64638543 = -172934564;    int KMGzXIDBLoUwFiF16211802 = -556475271;    int KMGzXIDBLoUwFiF68906105 = -388468296;    int KMGzXIDBLoUwFiF7856993 = -133161604;    int KMGzXIDBLoUwFiF25999561 = -183877589;    int KMGzXIDBLoUwFiF64825617 = -686527637;    int KMGzXIDBLoUwFiF84933943 = -818250042;    int KMGzXIDBLoUwFiF34754546 = -686883495;    int KMGzXIDBLoUwFiF96373220 = -297353422;    int KMGzXIDBLoUwFiF98214469 = -216633981;    int KMGzXIDBLoUwFiF81959111 = -553404071;    int KMGzXIDBLoUwFiF25724992 = -406594002;    int KMGzXIDBLoUwFiF59430356 = 21411348;    int KMGzXIDBLoUwFiF73450049 = -77439881;    int KMGzXIDBLoUwFiF70728763 = -726562609;    int KMGzXIDBLoUwFiF31788193 = -213375143;    int KMGzXIDBLoUwFiF51752632 = -799459759;    int KMGzXIDBLoUwFiF11116880 = -837039917;    int KMGzXIDBLoUwFiF86412426 = -359324379;    int KMGzXIDBLoUwFiF49249631 = -904901509;    int KMGzXIDBLoUwFiF63811714 = -95451815;    int KMGzXIDBLoUwFiF71916704 = 22614982;    int KMGzXIDBLoUwFiF59110053 = -707784656;    int KMGzXIDBLoUwFiF41475808 = -588477065;    int KMGzXIDBLoUwFiF93537714 = -609601917;    int KMGzXIDBLoUwFiF3457860 = -914652491;    int KMGzXIDBLoUwFiF3988876 = -678202282;    int KMGzXIDBLoUwFiF35032689 = -961957797;    int KMGzXIDBLoUwFiF25095660 = 74947436;    int KMGzXIDBLoUwFiF99985658 = -354535419;    int KMGzXIDBLoUwFiF26054328 = -850476274;    int KMGzXIDBLoUwFiF94920842 = -684131968;    int KMGzXIDBLoUwFiF37825221 = -787022548;    int KMGzXIDBLoUwFiF93976682 = -838169102;    int KMGzXIDBLoUwFiF14380356 = -392113491;    int KMGzXIDBLoUwFiF46169889 = -369003413;    int KMGzXIDBLoUwFiF75729812 = -628952404;    int KMGzXIDBLoUwFiF18408787 = -418464836;    int KMGzXIDBLoUwFiF65100099 = -772834750;    int KMGzXIDBLoUwFiF24763337 = -893884794;    int KMGzXIDBLoUwFiF70236143 = -483086898;    int KMGzXIDBLoUwFiF79938767 = -160243730;    int KMGzXIDBLoUwFiF19794496 = -658364086;    int KMGzXIDBLoUwFiF55954304 = -255590952;    int KMGzXIDBLoUwFiF6901089 = -108352240;    int KMGzXIDBLoUwFiF63087537 = -936484859;     KMGzXIDBLoUwFiF29490628 = KMGzXIDBLoUwFiF99242113;     KMGzXIDBLoUwFiF99242113 = KMGzXIDBLoUwFiF90207236;     KMGzXIDBLoUwFiF90207236 = KMGzXIDBLoUwFiF94661880;     KMGzXIDBLoUwFiF94661880 = KMGzXIDBLoUwFiF56928026;     KMGzXIDBLoUwFiF56928026 = KMGzXIDBLoUwFiF94057761;     KMGzXIDBLoUwFiF94057761 = KMGzXIDBLoUwFiF21889727;     KMGzXIDBLoUwFiF21889727 = KMGzXIDBLoUwFiF9952845;     KMGzXIDBLoUwFiF9952845 = KMGzXIDBLoUwFiF97925181;     KMGzXIDBLoUwFiF97925181 = KMGzXIDBLoUwFiF53406605;     KMGzXIDBLoUwFiF53406605 = KMGzXIDBLoUwFiF55690292;     KMGzXIDBLoUwFiF55690292 = KMGzXIDBLoUwFiF72306792;     KMGzXIDBLoUwFiF72306792 = KMGzXIDBLoUwFiF52468948;     KMGzXIDBLoUwFiF52468948 = KMGzXIDBLoUwFiF33110455;     KMGzXIDBLoUwFiF33110455 = KMGzXIDBLoUwFiF76424110;     KMGzXIDBLoUwFiF76424110 = KMGzXIDBLoUwFiF78018412;     KMGzXIDBLoUwFiF78018412 = KMGzXIDBLoUwFiF2216540;     KMGzXIDBLoUwFiF2216540 = KMGzXIDBLoUwFiF76001380;     KMGzXIDBLoUwFiF76001380 = KMGzXIDBLoUwFiF4419718;     KMGzXIDBLoUwFiF4419718 = KMGzXIDBLoUwFiF78505027;     KMGzXIDBLoUwFiF78505027 = KMGzXIDBLoUwFiF88084421;     KMGzXIDBLoUwFiF88084421 = KMGzXIDBLoUwFiF13278826;     KMGzXIDBLoUwFiF13278826 = KMGzXIDBLoUwFiF30336009;     KMGzXIDBLoUwFiF30336009 = KMGzXIDBLoUwFiF82350243;     KMGzXIDBLoUwFiF82350243 = KMGzXIDBLoUwFiF68662320;     KMGzXIDBLoUwFiF68662320 = KMGzXIDBLoUwFiF92102409;     KMGzXIDBLoUwFiF92102409 = KMGzXIDBLoUwFiF9123818;     KMGzXIDBLoUwFiF9123818 = KMGzXIDBLoUwFiF87135180;     KMGzXIDBLoUwFiF87135180 = KMGzXIDBLoUwFiF13579624;     KMGzXIDBLoUwFiF13579624 = KMGzXIDBLoUwFiF99710712;     KMGzXIDBLoUwFiF99710712 = KMGzXIDBLoUwFiF71447494;     KMGzXIDBLoUwFiF71447494 = KMGzXIDBLoUwFiF29965301;     KMGzXIDBLoUwFiF29965301 = KMGzXIDBLoUwFiF12876436;     KMGzXIDBLoUwFiF12876436 = KMGzXIDBLoUwFiF79018899;     KMGzXIDBLoUwFiF79018899 = KMGzXIDBLoUwFiF62381691;     KMGzXIDBLoUwFiF62381691 = KMGzXIDBLoUwFiF44635918;     KMGzXIDBLoUwFiF44635918 = KMGzXIDBLoUwFiF26265780;     KMGzXIDBLoUwFiF26265780 = KMGzXIDBLoUwFiF91099659;     KMGzXIDBLoUwFiF91099659 = KMGzXIDBLoUwFiF89588954;     KMGzXIDBLoUwFiF89588954 = KMGzXIDBLoUwFiF55170087;     KMGzXIDBLoUwFiF55170087 = KMGzXIDBLoUwFiF14693313;     KMGzXIDBLoUwFiF14693313 = KMGzXIDBLoUwFiF16167717;     KMGzXIDBLoUwFiF16167717 = KMGzXIDBLoUwFiF54168773;     KMGzXIDBLoUwFiF54168773 = KMGzXIDBLoUwFiF88860200;     KMGzXIDBLoUwFiF88860200 = KMGzXIDBLoUwFiF88812528;     KMGzXIDBLoUwFiF88812528 = KMGzXIDBLoUwFiF65204460;     KMGzXIDBLoUwFiF65204460 = KMGzXIDBLoUwFiF88113533;     KMGzXIDBLoUwFiF88113533 = KMGzXIDBLoUwFiF74091129;     KMGzXIDBLoUwFiF74091129 = KMGzXIDBLoUwFiF62039520;     KMGzXIDBLoUwFiF62039520 = KMGzXIDBLoUwFiF13593966;     KMGzXIDBLoUwFiF13593966 = KMGzXIDBLoUwFiF73656384;     KMGzXIDBLoUwFiF73656384 = KMGzXIDBLoUwFiF76526652;     KMGzXIDBLoUwFiF76526652 = KMGzXIDBLoUwFiF92140079;     KMGzXIDBLoUwFiF92140079 = KMGzXIDBLoUwFiF18899754;     KMGzXIDBLoUwFiF18899754 = KMGzXIDBLoUwFiF64638543;     KMGzXIDBLoUwFiF64638543 = KMGzXIDBLoUwFiF16211802;     KMGzXIDBLoUwFiF16211802 = KMGzXIDBLoUwFiF68906105;     KMGzXIDBLoUwFiF68906105 = KMGzXIDBLoUwFiF7856993;     KMGzXIDBLoUwFiF7856993 = KMGzXIDBLoUwFiF25999561;     KMGzXIDBLoUwFiF25999561 = KMGzXIDBLoUwFiF64825617;     KMGzXIDBLoUwFiF64825617 = KMGzXIDBLoUwFiF84933943;     KMGzXIDBLoUwFiF84933943 = KMGzXIDBLoUwFiF34754546;     KMGzXIDBLoUwFiF34754546 = KMGzXIDBLoUwFiF96373220;     KMGzXIDBLoUwFiF96373220 = KMGzXIDBLoUwFiF98214469;     KMGzXIDBLoUwFiF98214469 = KMGzXIDBLoUwFiF81959111;     KMGzXIDBLoUwFiF81959111 = KMGzXIDBLoUwFiF25724992;     KMGzXIDBLoUwFiF25724992 = KMGzXIDBLoUwFiF59430356;     KMGzXIDBLoUwFiF59430356 = KMGzXIDBLoUwFiF73450049;     KMGzXIDBLoUwFiF73450049 = KMGzXIDBLoUwFiF70728763;     KMGzXIDBLoUwFiF70728763 = KMGzXIDBLoUwFiF31788193;     KMGzXIDBLoUwFiF31788193 = KMGzXIDBLoUwFiF51752632;     KMGzXIDBLoUwFiF51752632 = KMGzXIDBLoUwFiF11116880;     KMGzXIDBLoUwFiF11116880 = KMGzXIDBLoUwFiF86412426;     KMGzXIDBLoUwFiF86412426 = KMGzXIDBLoUwFiF49249631;     KMGzXIDBLoUwFiF49249631 = KMGzXIDBLoUwFiF63811714;     KMGzXIDBLoUwFiF63811714 = KMGzXIDBLoUwFiF71916704;     KMGzXIDBLoUwFiF71916704 = KMGzXIDBLoUwFiF59110053;     KMGzXIDBLoUwFiF59110053 = KMGzXIDBLoUwFiF41475808;     KMGzXIDBLoUwFiF41475808 = KMGzXIDBLoUwFiF93537714;     KMGzXIDBLoUwFiF93537714 = KMGzXIDBLoUwFiF3457860;     KMGzXIDBLoUwFiF3457860 = KMGzXIDBLoUwFiF3988876;     KMGzXIDBLoUwFiF3988876 = KMGzXIDBLoUwFiF35032689;     KMGzXIDBLoUwFiF35032689 = KMGzXIDBLoUwFiF25095660;     KMGzXIDBLoUwFiF25095660 = KMGzXIDBLoUwFiF99985658;     KMGzXIDBLoUwFiF99985658 = KMGzXIDBLoUwFiF26054328;     KMGzXIDBLoUwFiF26054328 = KMGzXIDBLoUwFiF94920842;     KMGzXIDBLoUwFiF94920842 = KMGzXIDBLoUwFiF37825221;     KMGzXIDBLoUwFiF37825221 = KMGzXIDBLoUwFiF93976682;     KMGzXIDBLoUwFiF93976682 = KMGzXIDBLoUwFiF14380356;     KMGzXIDBLoUwFiF14380356 = KMGzXIDBLoUwFiF46169889;     KMGzXIDBLoUwFiF46169889 = KMGzXIDBLoUwFiF75729812;     KMGzXIDBLoUwFiF75729812 = KMGzXIDBLoUwFiF18408787;     KMGzXIDBLoUwFiF18408787 = KMGzXIDBLoUwFiF65100099;     KMGzXIDBLoUwFiF65100099 = KMGzXIDBLoUwFiF24763337;     KMGzXIDBLoUwFiF24763337 = KMGzXIDBLoUwFiF70236143;     KMGzXIDBLoUwFiF70236143 = KMGzXIDBLoUwFiF79938767;     KMGzXIDBLoUwFiF79938767 = KMGzXIDBLoUwFiF19794496;     KMGzXIDBLoUwFiF19794496 = KMGzXIDBLoUwFiF55954304;     KMGzXIDBLoUwFiF55954304 = KMGzXIDBLoUwFiF6901089;     KMGzXIDBLoUwFiF6901089 = KMGzXIDBLoUwFiF63087537;     KMGzXIDBLoUwFiF63087537 = KMGzXIDBLoUwFiF29490628;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bxzFTGpUsIKFGewdvwTGhMFWFZUcR54912303() {     int CNPrDJVIHazfAGN26003549 = -935285547;    int CNPrDJVIHazfAGN14324939 = -791389973;    int CNPrDJVIHazfAGN45889547 = -711116748;    int CNPrDJVIHazfAGN7210729 = -36049193;    int CNPrDJVIHazfAGN25580417 = -975332328;    int CNPrDJVIHazfAGN31304634 = -342593031;    int CNPrDJVIHazfAGN95704379 = -173661488;    int CNPrDJVIHazfAGN45405015 = -641089248;    int CNPrDJVIHazfAGN2179438 = -646449318;    int CNPrDJVIHazfAGN7411142 = -738784988;    int CNPrDJVIHazfAGN15906408 = -347115998;    int CNPrDJVIHazfAGN87323229 = -166999644;    int CNPrDJVIHazfAGN57739646 = -28273599;    int CNPrDJVIHazfAGN185568 = -161562827;    int CNPrDJVIHazfAGN81323868 = -182122552;    int CNPrDJVIHazfAGN90975235 = -209707414;    int CNPrDJVIHazfAGN29907389 = -806599299;    int CNPrDJVIHazfAGN12136868 = -557262874;    int CNPrDJVIHazfAGN86342694 = -695365748;    int CNPrDJVIHazfAGN19230286 = 47896130;    int CNPrDJVIHazfAGN95763276 = -243652123;    int CNPrDJVIHazfAGN54180583 = -658027280;    int CNPrDJVIHazfAGN19832971 = -631153086;    int CNPrDJVIHazfAGN6175014 = -688969783;    int CNPrDJVIHazfAGN46937327 = 91216427;    int CNPrDJVIHazfAGN68126774 = -812512958;    int CNPrDJVIHazfAGN57020503 = -320997260;    int CNPrDJVIHazfAGN79385543 = 22383882;    int CNPrDJVIHazfAGN95775595 = 3770375;    int CNPrDJVIHazfAGN99965219 = -542174426;    int CNPrDJVIHazfAGN80541573 = -728623858;    int CNPrDJVIHazfAGN65715483 = -197292780;    int CNPrDJVIHazfAGN30320875 = -509774854;    int CNPrDJVIHazfAGN91678567 = -485558055;    int CNPrDJVIHazfAGN71866529 = -140144210;    int CNPrDJVIHazfAGN88307171 = -294497408;    int CNPrDJVIHazfAGN6822747 = -162050397;    int CNPrDJVIHazfAGN64226634 = -877844307;    int CNPrDJVIHazfAGN38323261 = -921481181;    int CNPrDJVIHazfAGN34687487 = -837684821;    int CNPrDJVIHazfAGN46964308 = -378669395;    int CNPrDJVIHazfAGN60361069 = -463158470;    int CNPrDJVIHazfAGN41710709 = 31502495;    int CNPrDJVIHazfAGN1068309 = -903390064;    int CNPrDJVIHazfAGN54799244 = -552404800;    int CNPrDJVIHazfAGN62885486 = -801033120;    int CNPrDJVIHazfAGN85025626 = -133536062;    int CNPrDJVIHazfAGN54581152 = -694951616;    int CNPrDJVIHazfAGN80648588 = -928311650;    int CNPrDJVIHazfAGN86395005 = -25855679;    int CNPrDJVIHazfAGN25445025 = -856300517;    int CNPrDJVIHazfAGN48935290 = -224688987;    int CNPrDJVIHazfAGN84283457 = -870172856;    int CNPrDJVIHazfAGN4406950 = -131709989;    int CNPrDJVIHazfAGN59149027 = -605656356;    int CNPrDJVIHazfAGN71822965 = -177258267;    int CNPrDJVIHazfAGN94491967 = -60236888;    int CNPrDJVIHazfAGN39714534 = 77853035;    int CNPrDJVIHazfAGN60273402 = -27265620;    int CNPrDJVIHazfAGN57453643 = -62819371;    int CNPrDJVIHazfAGN74284131 = 78404229;    int CNPrDJVIHazfAGN16318837 = -96045371;    int CNPrDJVIHazfAGN49629420 = -544859623;    int CNPrDJVIHazfAGN2214219 = -4274893;    int CNPrDJVIHazfAGN26869568 = 89838869;    int CNPrDJVIHazfAGN50190924 = -49823218;    int CNPrDJVIHazfAGN57002354 = -657224790;    int CNPrDJVIHazfAGN66061078 = -542715545;    int CNPrDJVIHazfAGN28319038 = 78581382;    int CNPrDJVIHazfAGN93016696 = -887625145;    int CNPrDJVIHazfAGN84152489 = 52342983;    int CNPrDJVIHazfAGN65680754 = -928754993;    int CNPrDJVIHazfAGN73813607 = -635781694;    int CNPrDJVIHazfAGN51655208 = -857680928;    int CNPrDJVIHazfAGN72265978 = -573434475;    int CNPrDJVIHazfAGN35402208 = -780493654;    int CNPrDJVIHazfAGN12469875 = -589529776;    int CNPrDJVIHazfAGN18764663 = -727763022;    int CNPrDJVIHazfAGN51375770 = -36564983;    int CNPrDJVIHazfAGN84051840 = -107750454;    int CNPrDJVIHazfAGN83101147 = -578976897;    int CNPrDJVIHazfAGN2439351 = -626045645;    int CNPrDJVIHazfAGN98736954 = -49304468;    int CNPrDJVIHazfAGN9380591 = -970373946;    int CNPrDJVIHazfAGN74520195 = -685873910;    int CNPrDJVIHazfAGN31606284 = -403934871;    int CNPrDJVIHazfAGN81432025 = -327119924;    int CNPrDJVIHazfAGN25913926 = -278064866;    int CNPrDJVIHazfAGN32529541 = -879901699;    int CNPrDJVIHazfAGN43564 = -962885943;    int CNPrDJVIHazfAGN93815203 = -134260520;    int CNPrDJVIHazfAGN67108213 = -139903433;    int CNPrDJVIHazfAGN3953233 = -750578687;    int CNPrDJVIHazfAGN80869618 = -758661811;    int CNPrDJVIHazfAGN60403355 = -816089050;    int CNPrDJVIHazfAGN30645472 = -182624025;    int CNPrDJVIHazfAGN10731649 = -918298847;    int CNPrDJVIHazfAGN39496491 = -964222612;    int CNPrDJVIHazfAGN74198740 = -893228934;    int CNPrDJVIHazfAGN4608321 = -935285547;     CNPrDJVIHazfAGN26003549 = CNPrDJVIHazfAGN14324939;     CNPrDJVIHazfAGN14324939 = CNPrDJVIHazfAGN45889547;     CNPrDJVIHazfAGN45889547 = CNPrDJVIHazfAGN7210729;     CNPrDJVIHazfAGN7210729 = CNPrDJVIHazfAGN25580417;     CNPrDJVIHazfAGN25580417 = CNPrDJVIHazfAGN31304634;     CNPrDJVIHazfAGN31304634 = CNPrDJVIHazfAGN95704379;     CNPrDJVIHazfAGN95704379 = CNPrDJVIHazfAGN45405015;     CNPrDJVIHazfAGN45405015 = CNPrDJVIHazfAGN2179438;     CNPrDJVIHazfAGN2179438 = CNPrDJVIHazfAGN7411142;     CNPrDJVIHazfAGN7411142 = CNPrDJVIHazfAGN15906408;     CNPrDJVIHazfAGN15906408 = CNPrDJVIHazfAGN87323229;     CNPrDJVIHazfAGN87323229 = CNPrDJVIHazfAGN57739646;     CNPrDJVIHazfAGN57739646 = CNPrDJVIHazfAGN185568;     CNPrDJVIHazfAGN185568 = CNPrDJVIHazfAGN81323868;     CNPrDJVIHazfAGN81323868 = CNPrDJVIHazfAGN90975235;     CNPrDJVIHazfAGN90975235 = CNPrDJVIHazfAGN29907389;     CNPrDJVIHazfAGN29907389 = CNPrDJVIHazfAGN12136868;     CNPrDJVIHazfAGN12136868 = CNPrDJVIHazfAGN86342694;     CNPrDJVIHazfAGN86342694 = CNPrDJVIHazfAGN19230286;     CNPrDJVIHazfAGN19230286 = CNPrDJVIHazfAGN95763276;     CNPrDJVIHazfAGN95763276 = CNPrDJVIHazfAGN54180583;     CNPrDJVIHazfAGN54180583 = CNPrDJVIHazfAGN19832971;     CNPrDJVIHazfAGN19832971 = CNPrDJVIHazfAGN6175014;     CNPrDJVIHazfAGN6175014 = CNPrDJVIHazfAGN46937327;     CNPrDJVIHazfAGN46937327 = CNPrDJVIHazfAGN68126774;     CNPrDJVIHazfAGN68126774 = CNPrDJVIHazfAGN57020503;     CNPrDJVIHazfAGN57020503 = CNPrDJVIHazfAGN79385543;     CNPrDJVIHazfAGN79385543 = CNPrDJVIHazfAGN95775595;     CNPrDJVIHazfAGN95775595 = CNPrDJVIHazfAGN99965219;     CNPrDJVIHazfAGN99965219 = CNPrDJVIHazfAGN80541573;     CNPrDJVIHazfAGN80541573 = CNPrDJVIHazfAGN65715483;     CNPrDJVIHazfAGN65715483 = CNPrDJVIHazfAGN30320875;     CNPrDJVIHazfAGN30320875 = CNPrDJVIHazfAGN91678567;     CNPrDJVIHazfAGN91678567 = CNPrDJVIHazfAGN71866529;     CNPrDJVIHazfAGN71866529 = CNPrDJVIHazfAGN88307171;     CNPrDJVIHazfAGN88307171 = CNPrDJVIHazfAGN6822747;     CNPrDJVIHazfAGN6822747 = CNPrDJVIHazfAGN64226634;     CNPrDJVIHazfAGN64226634 = CNPrDJVIHazfAGN38323261;     CNPrDJVIHazfAGN38323261 = CNPrDJVIHazfAGN34687487;     CNPrDJVIHazfAGN34687487 = CNPrDJVIHazfAGN46964308;     CNPrDJVIHazfAGN46964308 = CNPrDJVIHazfAGN60361069;     CNPrDJVIHazfAGN60361069 = CNPrDJVIHazfAGN41710709;     CNPrDJVIHazfAGN41710709 = CNPrDJVIHazfAGN1068309;     CNPrDJVIHazfAGN1068309 = CNPrDJVIHazfAGN54799244;     CNPrDJVIHazfAGN54799244 = CNPrDJVIHazfAGN62885486;     CNPrDJVIHazfAGN62885486 = CNPrDJVIHazfAGN85025626;     CNPrDJVIHazfAGN85025626 = CNPrDJVIHazfAGN54581152;     CNPrDJVIHazfAGN54581152 = CNPrDJVIHazfAGN80648588;     CNPrDJVIHazfAGN80648588 = CNPrDJVIHazfAGN86395005;     CNPrDJVIHazfAGN86395005 = CNPrDJVIHazfAGN25445025;     CNPrDJVIHazfAGN25445025 = CNPrDJVIHazfAGN48935290;     CNPrDJVIHazfAGN48935290 = CNPrDJVIHazfAGN84283457;     CNPrDJVIHazfAGN84283457 = CNPrDJVIHazfAGN4406950;     CNPrDJVIHazfAGN4406950 = CNPrDJVIHazfAGN59149027;     CNPrDJVIHazfAGN59149027 = CNPrDJVIHazfAGN71822965;     CNPrDJVIHazfAGN71822965 = CNPrDJVIHazfAGN94491967;     CNPrDJVIHazfAGN94491967 = CNPrDJVIHazfAGN39714534;     CNPrDJVIHazfAGN39714534 = CNPrDJVIHazfAGN60273402;     CNPrDJVIHazfAGN60273402 = CNPrDJVIHazfAGN57453643;     CNPrDJVIHazfAGN57453643 = CNPrDJVIHazfAGN74284131;     CNPrDJVIHazfAGN74284131 = CNPrDJVIHazfAGN16318837;     CNPrDJVIHazfAGN16318837 = CNPrDJVIHazfAGN49629420;     CNPrDJVIHazfAGN49629420 = CNPrDJVIHazfAGN2214219;     CNPrDJVIHazfAGN2214219 = CNPrDJVIHazfAGN26869568;     CNPrDJVIHazfAGN26869568 = CNPrDJVIHazfAGN50190924;     CNPrDJVIHazfAGN50190924 = CNPrDJVIHazfAGN57002354;     CNPrDJVIHazfAGN57002354 = CNPrDJVIHazfAGN66061078;     CNPrDJVIHazfAGN66061078 = CNPrDJVIHazfAGN28319038;     CNPrDJVIHazfAGN28319038 = CNPrDJVIHazfAGN93016696;     CNPrDJVIHazfAGN93016696 = CNPrDJVIHazfAGN84152489;     CNPrDJVIHazfAGN84152489 = CNPrDJVIHazfAGN65680754;     CNPrDJVIHazfAGN65680754 = CNPrDJVIHazfAGN73813607;     CNPrDJVIHazfAGN73813607 = CNPrDJVIHazfAGN51655208;     CNPrDJVIHazfAGN51655208 = CNPrDJVIHazfAGN72265978;     CNPrDJVIHazfAGN72265978 = CNPrDJVIHazfAGN35402208;     CNPrDJVIHazfAGN35402208 = CNPrDJVIHazfAGN12469875;     CNPrDJVIHazfAGN12469875 = CNPrDJVIHazfAGN18764663;     CNPrDJVIHazfAGN18764663 = CNPrDJVIHazfAGN51375770;     CNPrDJVIHazfAGN51375770 = CNPrDJVIHazfAGN84051840;     CNPrDJVIHazfAGN84051840 = CNPrDJVIHazfAGN83101147;     CNPrDJVIHazfAGN83101147 = CNPrDJVIHazfAGN2439351;     CNPrDJVIHazfAGN2439351 = CNPrDJVIHazfAGN98736954;     CNPrDJVIHazfAGN98736954 = CNPrDJVIHazfAGN9380591;     CNPrDJVIHazfAGN9380591 = CNPrDJVIHazfAGN74520195;     CNPrDJVIHazfAGN74520195 = CNPrDJVIHazfAGN31606284;     CNPrDJVIHazfAGN31606284 = CNPrDJVIHazfAGN81432025;     CNPrDJVIHazfAGN81432025 = CNPrDJVIHazfAGN25913926;     CNPrDJVIHazfAGN25913926 = CNPrDJVIHazfAGN32529541;     CNPrDJVIHazfAGN32529541 = CNPrDJVIHazfAGN43564;     CNPrDJVIHazfAGN43564 = CNPrDJVIHazfAGN93815203;     CNPrDJVIHazfAGN93815203 = CNPrDJVIHazfAGN67108213;     CNPrDJVIHazfAGN67108213 = CNPrDJVIHazfAGN3953233;     CNPrDJVIHazfAGN3953233 = CNPrDJVIHazfAGN80869618;     CNPrDJVIHazfAGN80869618 = CNPrDJVIHazfAGN60403355;     CNPrDJVIHazfAGN60403355 = CNPrDJVIHazfAGN30645472;     CNPrDJVIHazfAGN30645472 = CNPrDJVIHazfAGN10731649;     CNPrDJVIHazfAGN10731649 = CNPrDJVIHazfAGN39496491;     CNPrDJVIHazfAGN39496491 = CNPrDJVIHazfAGN74198740;     CNPrDJVIHazfAGN74198740 = CNPrDJVIHazfAGN4608321;     CNPrDJVIHazfAGN4608321 = CNPrDJVIHazfAGN26003549;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void AIYYRxKdVWzAKygtuBjPouTTCvfiO57101642() {     int bcMZkXJpPMUFcOM22516470 = -934086235;    int bcMZkXJpPMUFcOM29407764 = -744078275;    int bcMZkXJpPMUFcOM1571859 = -459219513;    int bcMZkXJpPMUFcOM19759577 = -504862983;    int bcMZkXJpPMUFcOM94232806 = -530919324;    int bcMZkXJpPMUFcOM68551506 = -373120607;    int bcMZkXJpPMUFcOM69519033 = -338579463;    int bcMZkXJpPMUFcOM80857185 = -753973432;    int bcMZkXJpPMUFcOM6433695 = -898867072;    int bcMZkXJpPMUFcOM61415678 = -397510201;    int bcMZkXJpPMUFcOM76122523 = -322351235;    int bcMZkXJpPMUFcOM2339667 = -735732550;    int bcMZkXJpPMUFcOM63010344 = -314059262;    int bcMZkXJpPMUFcOM67260681 = -671084362;    int bcMZkXJpPMUFcOM86223625 = -33449262;    int bcMZkXJpPMUFcOM3932060 = 31671371;    int bcMZkXJpPMUFcOM57598237 = -719446344;    int bcMZkXJpPMUFcOM48272356 = -74788939;    int bcMZkXJpPMUFcOM68265670 = -84493048;    int bcMZkXJpPMUFcOM59955545 = -961628700;    int bcMZkXJpPMUFcOM3442132 = -454201721;    int bcMZkXJpPMUFcOM95082340 = 63955027;    int bcMZkXJpPMUFcOM9329934 = -912072796;    int bcMZkXJpPMUFcOM29999784 = -648087187;    int bcMZkXJpPMUFcOM25212334 = -534209333;    int bcMZkXJpPMUFcOM44151138 = -991808221;    int bcMZkXJpPMUFcOM4917188 = -148179107;    int bcMZkXJpPMUFcOM71635905 = -733372217;    int bcMZkXJpPMUFcOM77971566 = -961607608;    int bcMZkXJpPMUFcOM219728 = 93048732;    int bcMZkXJpPMUFcOM89635653 = 69407989;    int bcMZkXJpPMUFcOM1465667 = -529298800;    int bcMZkXJpPMUFcOM47765313 = -399871622;    int bcMZkXJpPMUFcOM4338236 = -306068054;    int bcMZkXJpPMUFcOM81351368 = -354809735;    int bcMZkXJpPMUFcOM31978425 = -571574116;    int bcMZkXJpPMUFcOM87379713 = -772474355;    int bcMZkXJpPMUFcOM37353609 = -698976276;    int bcMZkXJpPMUFcOM87057567 = -162549931;    int bcMZkXJpPMUFcOM14204886 = -274032703;    int bcMZkXJpPMUFcOM79235303 = -910211566;    int bcMZkXJpPMUFcOM4554421 = -970599432;    int bcMZkXJpPMUFcOM29252645 = -464770077;    int bcMZkXJpPMUFcOM13276417 = 54976182;    int bcMZkXJpPMUFcOM20785959 = 15440862;    int bcMZkXJpPMUFcOM60566513 = -33360916;    int bcMZkXJpPMUFcOM81937719 = -412056710;    int bcMZkXJpPMUFcOM35071175 = -858045615;    int bcMZkXJpPMUFcOM99257656 = -459815845;    int bcMZkXJpPMUFcOM59196044 = -375395134;    int bcMZkXJpPMUFcOM77233664 = -385679723;    int bcMZkXJpPMUFcOM21343929 = -806854238;    int bcMZkXJpPMUFcOM76426836 = -562081501;    int bcMZkXJpPMUFcOM89914145 = -581910993;    int bcMZkXJpPMUFcOM53659510 = 61621851;    int bcMZkXJpPMUFcOM27434129 = -898041262;    int bcMZkXJpPMUFcOM20077831 = -832005480;    int bcMZkXJpPMUFcOM71572074 = -811132327;    int bcMZkXJpPMUFcOM94547242 = -970653651;    int bcMZkXJpPMUFcOM50081668 = -539111104;    int bcMZkXJpPMUFcOM63634318 = -124941501;    int bcMZkXJpPMUFcOM97883127 = -605207247;    int bcMZkXJpPMUFcOM2885619 = -792365825;    int bcMZkXJpPMUFcOM6213968 = -891915804;    int bcMZkXJpPMUFcOM71780025 = -366918190;    int bcMZkXJpPMUFcOM74656856 = -793052435;    int bcMZkXJpPMUFcOM54574353 = -235860928;    int bcMZkXJpPMUFcOM58672108 = 92008791;    int bcMZkXJpPMUFcOM85909313 = -216274628;    int bcMZkXJpPMUFcOM54245200 = -461875147;    int bcMZkXJpPMUFcOM16552346 = -195854274;    int bcMZkXJpPMUFcOM20244629 = 79529931;    int bcMZkXJpPMUFcOM61214789 = -912239009;    int bcMZkXJpPMUFcOM54060785 = -810460346;    int bcMZkXJpPMUFcOM80720242 = 48582865;    int bcMZkXJpPMUFcOM98887710 = -483602289;    int bcMZkXJpPMUFcOM65829696 = -471274897;    int bcMZkXJpPMUFcOM96053516 = -867048978;    int bcMZkXJpPMUFcOM9213826 = -563528050;    int bcMZkXJpPMUFcOM64645821 = -400848417;    int bcMZkXJpPMUFcOM62213419 = -479751511;    int bcMZkXJpPMUFcOM69846012 = -290133493;    int bcMZkXJpPMUFcOM72378249 = -173556372;    int bcMZkXJpPMUFcOM18775523 = -486212474;    int bcMZkXJpPMUFcOM22986063 = -521271546;    int bcMZkXJpPMUFcOM68291725 = -123737774;    int bcMZkXJpPMUFcOM25038831 = -967217300;    int bcMZkXJpPMUFcOM57851168 = -817960630;    int bcMZkXJpPMUFcOM50678726 = -267689906;    int bcMZkXJpPMUFcOM53917239 = -456768473;    int bcMZkXJpPMUFcOM11900595 = -739568637;    int bcMZkXJpPMUFcOM15807639 = -961342029;    int bcMZkXJpPMUFcOM42806366 = -728322625;    int bcMZkXJpPMUFcOM36975900 = -623438828;    int bcMZkXJpPMUFcOM50570567 = -49091203;    int bcMZkXJpPMUFcOM81352175 = -205004319;    int bcMZkXJpPMUFcOM1668803 = -78233608;    int bcMZkXJpPMUFcOM23038678 = -572854273;    int bcMZkXJpPMUFcOM41496392 = -578105629;    int bcMZkXJpPMUFcOM46129103 = -934086235;     bcMZkXJpPMUFcOM22516470 = bcMZkXJpPMUFcOM29407764;     bcMZkXJpPMUFcOM29407764 = bcMZkXJpPMUFcOM1571859;     bcMZkXJpPMUFcOM1571859 = bcMZkXJpPMUFcOM19759577;     bcMZkXJpPMUFcOM19759577 = bcMZkXJpPMUFcOM94232806;     bcMZkXJpPMUFcOM94232806 = bcMZkXJpPMUFcOM68551506;     bcMZkXJpPMUFcOM68551506 = bcMZkXJpPMUFcOM69519033;     bcMZkXJpPMUFcOM69519033 = bcMZkXJpPMUFcOM80857185;     bcMZkXJpPMUFcOM80857185 = bcMZkXJpPMUFcOM6433695;     bcMZkXJpPMUFcOM6433695 = bcMZkXJpPMUFcOM61415678;     bcMZkXJpPMUFcOM61415678 = bcMZkXJpPMUFcOM76122523;     bcMZkXJpPMUFcOM76122523 = bcMZkXJpPMUFcOM2339667;     bcMZkXJpPMUFcOM2339667 = bcMZkXJpPMUFcOM63010344;     bcMZkXJpPMUFcOM63010344 = bcMZkXJpPMUFcOM67260681;     bcMZkXJpPMUFcOM67260681 = bcMZkXJpPMUFcOM86223625;     bcMZkXJpPMUFcOM86223625 = bcMZkXJpPMUFcOM3932060;     bcMZkXJpPMUFcOM3932060 = bcMZkXJpPMUFcOM57598237;     bcMZkXJpPMUFcOM57598237 = bcMZkXJpPMUFcOM48272356;     bcMZkXJpPMUFcOM48272356 = bcMZkXJpPMUFcOM68265670;     bcMZkXJpPMUFcOM68265670 = bcMZkXJpPMUFcOM59955545;     bcMZkXJpPMUFcOM59955545 = bcMZkXJpPMUFcOM3442132;     bcMZkXJpPMUFcOM3442132 = bcMZkXJpPMUFcOM95082340;     bcMZkXJpPMUFcOM95082340 = bcMZkXJpPMUFcOM9329934;     bcMZkXJpPMUFcOM9329934 = bcMZkXJpPMUFcOM29999784;     bcMZkXJpPMUFcOM29999784 = bcMZkXJpPMUFcOM25212334;     bcMZkXJpPMUFcOM25212334 = bcMZkXJpPMUFcOM44151138;     bcMZkXJpPMUFcOM44151138 = bcMZkXJpPMUFcOM4917188;     bcMZkXJpPMUFcOM4917188 = bcMZkXJpPMUFcOM71635905;     bcMZkXJpPMUFcOM71635905 = bcMZkXJpPMUFcOM77971566;     bcMZkXJpPMUFcOM77971566 = bcMZkXJpPMUFcOM219728;     bcMZkXJpPMUFcOM219728 = bcMZkXJpPMUFcOM89635653;     bcMZkXJpPMUFcOM89635653 = bcMZkXJpPMUFcOM1465667;     bcMZkXJpPMUFcOM1465667 = bcMZkXJpPMUFcOM47765313;     bcMZkXJpPMUFcOM47765313 = bcMZkXJpPMUFcOM4338236;     bcMZkXJpPMUFcOM4338236 = bcMZkXJpPMUFcOM81351368;     bcMZkXJpPMUFcOM81351368 = bcMZkXJpPMUFcOM31978425;     bcMZkXJpPMUFcOM31978425 = bcMZkXJpPMUFcOM87379713;     bcMZkXJpPMUFcOM87379713 = bcMZkXJpPMUFcOM37353609;     bcMZkXJpPMUFcOM37353609 = bcMZkXJpPMUFcOM87057567;     bcMZkXJpPMUFcOM87057567 = bcMZkXJpPMUFcOM14204886;     bcMZkXJpPMUFcOM14204886 = bcMZkXJpPMUFcOM79235303;     bcMZkXJpPMUFcOM79235303 = bcMZkXJpPMUFcOM4554421;     bcMZkXJpPMUFcOM4554421 = bcMZkXJpPMUFcOM29252645;     bcMZkXJpPMUFcOM29252645 = bcMZkXJpPMUFcOM13276417;     bcMZkXJpPMUFcOM13276417 = bcMZkXJpPMUFcOM20785959;     bcMZkXJpPMUFcOM20785959 = bcMZkXJpPMUFcOM60566513;     bcMZkXJpPMUFcOM60566513 = bcMZkXJpPMUFcOM81937719;     bcMZkXJpPMUFcOM81937719 = bcMZkXJpPMUFcOM35071175;     bcMZkXJpPMUFcOM35071175 = bcMZkXJpPMUFcOM99257656;     bcMZkXJpPMUFcOM99257656 = bcMZkXJpPMUFcOM59196044;     bcMZkXJpPMUFcOM59196044 = bcMZkXJpPMUFcOM77233664;     bcMZkXJpPMUFcOM77233664 = bcMZkXJpPMUFcOM21343929;     bcMZkXJpPMUFcOM21343929 = bcMZkXJpPMUFcOM76426836;     bcMZkXJpPMUFcOM76426836 = bcMZkXJpPMUFcOM89914145;     bcMZkXJpPMUFcOM89914145 = bcMZkXJpPMUFcOM53659510;     bcMZkXJpPMUFcOM53659510 = bcMZkXJpPMUFcOM27434129;     bcMZkXJpPMUFcOM27434129 = bcMZkXJpPMUFcOM20077831;     bcMZkXJpPMUFcOM20077831 = bcMZkXJpPMUFcOM71572074;     bcMZkXJpPMUFcOM71572074 = bcMZkXJpPMUFcOM94547242;     bcMZkXJpPMUFcOM94547242 = bcMZkXJpPMUFcOM50081668;     bcMZkXJpPMUFcOM50081668 = bcMZkXJpPMUFcOM63634318;     bcMZkXJpPMUFcOM63634318 = bcMZkXJpPMUFcOM97883127;     bcMZkXJpPMUFcOM97883127 = bcMZkXJpPMUFcOM2885619;     bcMZkXJpPMUFcOM2885619 = bcMZkXJpPMUFcOM6213968;     bcMZkXJpPMUFcOM6213968 = bcMZkXJpPMUFcOM71780025;     bcMZkXJpPMUFcOM71780025 = bcMZkXJpPMUFcOM74656856;     bcMZkXJpPMUFcOM74656856 = bcMZkXJpPMUFcOM54574353;     bcMZkXJpPMUFcOM54574353 = bcMZkXJpPMUFcOM58672108;     bcMZkXJpPMUFcOM58672108 = bcMZkXJpPMUFcOM85909313;     bcMZkXJpPMUFcOM85909313 = bcMZkXJpPMUFcOM54245200;     bcMZkXJpPMUFcOM54245200 = bcMZkXJpPMUFcOM16552346;     bcMZkXJpPMUFcOM16552346 = bcMZkXJpPMUFcOM20244629;     bcMZkXJpPMUFcOM20244629 = bcMZkXJpPMUFcOM61214789;     bcMZkXJpPMUFcOM61214789 = bcMZkXJpPMUFcOM54060785;     bcMZkXJpPMUFcOM54060785 = bcMZkXJpPMUFcOM80720242;     bcMZkXJpPMUFcOM80720242 = bcMZkXJpPMUFcOM98887710;     bcMZkXJpPMUFcOM98887710 = bcMZkXJpPMUFcOM65829696;     bcMZkXJpPMUFcOM65829696 = bcMZkXJpPMUFcOM96053516;     bcMZkXJpPMUFcOM96053516 = bcMZkXJpPMUFcOM9213826;     bcMZkXJpPMUFcOM9213826 = bcMZkXJpPMUFcOM64645821;     bcMZkXJpPMUFcOM64645821 = bcMZkXJpPMUFcOM62213419;     bcMZkXJpPMUFcOM62213419 = bcMZkXJpPMUFcOM69846012;     bcMZkXJpPMUFcOM69846012 = bcMZkXJpPMUFcOM72378249;     bcMZkXJpPMUFcOM72378249 = bcMZkXJpPMUFcOM18775523;     bcMZkXJpPMUFcOM18775523 = bcMZkXJpPMUFcOM22986063;     bcMZkXJpPMUFcOM22986063 = bcMZkXJpPMUFcOM68291725;     bcMZkXJpPMUFcOM68291725 = bcMZkXJpPMUFcOM25038831;     bcMZkXJpPMUFcOM25038831 = bcMZkXJpPMUFcOM57851168;     bcMZkXJpPMUFcOM57851168 = bcMZkXJpPMUFcOM50678726;     bcMZkXJpPMUFcOM50678726 = bcMZkXJpPMUFcOM53917239;     bcMZkXJpPMUFcOM53917239 = bcMZkXJpPMUFcOM11900595;     bcMZkXJpPMUFcOM11900595 = bcMZkXJpPMUFcOM15807639;     bcMZkXJpPMUFcOM15807639 = bcMZkXJpPMUFcOM42806366;     bcMZkXJpPMUFcOM42806366 = bcMZkXJpPMUFcOM36975900;     bcMZkXJpPMUFcOM36975900 = bcMZkXJpPMUFcOM50570567;     bcMZkXJpPMUFcOM50570567 = bcMZkXJpPMUFcOM81352175;     bcMZkXJpPMUFcOM81352175 = bcMZkXJpPMUFcOM1668803;     bcMZkXJpPMUFcOM1668803 = bcMZkXJpPMUFcOM23038678;     bcMZkXJpPMUFcOM23038678 = bcMZkXJpPMUFcOM41496392;     bcMZkXJpPMUFcOM41496392 = bcMZkXJpPMUFcOM46129103;     bcMZkXJpPMUFcOM46129103 = bcMZkXJpPMUFcOM22516470;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bSbTJIlGscHvHDQwmzkAxaOFBRxUT7048449() {     int FUlePBlECizYyAU89687528 = -578511912;    int FUlePBlECizYyAU74591790 = -525814751;    int FUlePBlECizYyAU35909876 = -692488125;    int FUlePBlECizYyAU87159851 = -243274333;    int FUlePBlECizYyAU75495415 = -501557346;    int FUlePBlECizYyAU60992708 = -405796523;    int FUlePBlECizYyAU93165245 = -797736681;    int FUlePBlECizYyAU3937686 = -409497389;    int FUlePBlECizYyAU73812842 = -430986084;    int FUlePBlECizYyAU41747162 = -720523893;    int FUlePBlECizYyAU58267194 = -373016329;    int FUlePBlECizYyAU48610997 = -784750886;    int FUlePBlECizYyAU16255515 = -487723510;    int FUlePBlECizYyAU64338607 = -636481056;    int FUlePBlECizYyAU28671293 = -71813124;    int FUlePBlECizYyAU74852976 = -64137780;    int FUlePBlECizYyAU23289153 = -721863949;    int FUlePBlECizYyAU81700805 = -284409579;    int FUlePBlECizYyAU11468158 = -662826024;    int FUlePBlECizYyAU87393750 = -801495384;    int FUlePBlECizYyAU86587316 = -862964086;    int FUlePBlECizYyAU19083597 = -135356088;    int FUlePBlECizYyAU63892308 = -201877809;    int FUlePBlECizYyAU44023983 = -318684975;    int FUlePBlECizYyAU39169040 = -408845926;    int FUlePBlECizYyAU34199007 = -261182000;    int FUlePBlECizYyAU90853076 = -69739016;    int FUlePBlECizYyAU36264969 = -285285095;    int FUlePBlECizYyAU79631578 = -475273493;    int FUlePBlECizYyAU63556461 = -154691807;    int FUlePBlECizYyAU72389050 = -693900615;    int FUlePBlECizYyAU97003453 = -797780222;    int FUlePBlECizYyAU62702690 = -34985783;    int FUlePBlECizYyAU82080042 = -913426006;    int FUlePBlECizYyAU86502404 = -157108116;    int FUlePBlECizYyAU92814747 = -848111626;    int FUlePBlECizYyAU99786798 = -371838142;    int FUlePBlECizYyAU93355391 = -308958673;    int FUlePBlECizYyAU79011566 = -846665902;    int FUlePBlECizYyAU16952691 = -718557262;    int FUlePBlECizYyAU83723914 = -637828716;    int FUlePBlECizYyAU91533103 = 46362618;    int FUlePBlECizYyAU5478181 = -388666447;    int FUlePBlECizYyAU72760927 = -601668100;    int FUlePBlECizYyAU98708680 = -352100469;    int FUlePBlECizYyAU24225407 = -311981196;    int FUlePBlECizYyAU77283222 = -325366536;    int FUlePBlECizYyAU5791828 = -864508553;    int FUlePBlECizYyAU4649788 = -413320148;    int FUlePBlECizYyAU20379147 = -320918582;    int FUlePBlECizYyAU83952259 = -983076043;    int FUlePBlECizYyAU21076814 = -976093461;    int FUlePBlECizYyAU48197523 = -289734179;    int FUlePBlECizYyAU55398374 = -61073642;    int FUlePBlECizYyAU20461132 = -725141578;    int FUlePBlECizYyAU70603931 = -343155824;    int FUlePBlECizYyAU10699483 = -223936943;    int FUlePBlECizYyAU91885893 = -273803150;    int FUlePBlECizYyAU47990811 = -834428407;    int FUlePBlECizYyAU41296409 = -140375346;    int FUlePBlECizYyAU70139632 = -236057508;    int FUlePBlECizYyAU56900277 = -412451586;    int FUlePBlECizYyAU24306107 = -934223897;    int FUlePBlECizYyAU10256382 = -176294277;    int FUlePBlECizYyAU69358111 = 73376722;    int FUlePBlECizYyAU61263740 = -575236108;    int FUlePBlECizYyAU85908307 = -649765103;    int FUlePBlECizYyAU34175473 = -574297505;    int FUlePBlECizYyAU77836202 = -379372941;    int FUlePBlECizYyAU35856545 = -223701498;    int FUlePBlECizYyAU75066178 = -692299639;    int FUlePBlECizYyAU29933762 = -312905277;    int FUlePBlECizYyAU2689240 = -437743677;    int FUlePBlECizYyAU94515467 = -944268763;    int FUlePBlECizYyAU3669837 = -63666668;    int FUlePBlECizYyAU95054213 = -809326704;    int FUlePBlECizYyAU13605416 = -746689642;    int FUlePBlECizYyAU91131381 = -600209710;    int FUlePBlECizYyAU45315303 = -966584507;    int FUlePBlECizYyAU14943634 = 3135269;    int FUlePBlECizYyAU56915784 = -935815465;    int FUlePBlECizYyAU85061248 = -205230464;    int FUlePBlECizYyAU31615182 = -871964947;    int FUlePBlECizYyAU59252432 = -54354912;    int FUlePBlECizYyAU79604201 = -171615765;    int FUlePBlECizYyAU51312237 = -717807154;    int FUlePBlECizYyAU48805930 = -408046043;    int FUlePBlECizYyAU7304316 = -973912141;    int FUlePBlECizYyAU61618910 = -88284428;    int FUlePBlECizYyAU15898473 = -813952292;    int FUlePBlECizYyAU82115265 = -524174684;    int FUlePBlECizYyAU7900905 = 1965008;    int FUlePBlECizYyAU45364581 = -474530266;    int FUlePBlECizYyAU37715157 = -606290556;    int FUlePBlECizYyAU46813058 = -382499755;    int FUlePBlECizYyAU26823637 = -125377130;    int FUlePBlECizYyAU67226996 = -19413486;    int FUlePBlECizYyAU95221799 = -112372170;    int FUlePBlECizYyAU3402816 = -575044822;    int FUlePBlECizYyAU37444940 = -578511912;     FUlePBlECizYyAU89687528 = FUlePBlECizYyAU74591790;     FUlePBlECizYyAU74591790 = FUlePBlECizYyAU35909876;     FUlePBlECizYyAU35909876 = FUlePBlECizYyAU87159851;     FUlePBlECizYyAU87159851 = FUlePBlECizYyAU75495415;     FUlePBlECizYyAU75495415 = FUlePBlECizYyAU60992708;     FUlePBlECizYyAU60992708 = FUlePBlECizYyAU93165245;     FUlePBlECizYyAU93165245 = FUlePBlECizYyAU3937686;     FUlePBlECizYyAU3937686 = FUlePBlECizYyAU73812842;     FUlePBlECizYyAU73812842 = FUlePBlECizYyAU41747162;     FUlePBlECizYyAU41747162 = FUlePBlECizYyAU58267194;     FUlePBlECizYyAU58267194 = FUlePBlECizYyAU48610997;     FUlePBlECizYyAU48610997 = FUlePBlECizYyAU16255515;     FUlePBlECizYyAU16255515 = FUlePBlECizYyAU64338607;     FUlePBlECizYyAU64338607 = FUlePBlECizYyAU28671293;     FUlePBlECizYyAU28671293 = FUlePBlECizYyAU74852976;     FUlePBlECizYyAU74852976 = FUlePBlECizYyAU23289153;     FUlePBlECizYyAU23289153 = FUlePBlECizYyAU81700805;     FUlePBlECizYyAU81700805 = FUlePBlECizYyAU11468158;     FUlePBlECizYyAU11468158 = FUlePBlECizYyAU87393750;     FUlePBlECizYyAU87393750 = FUlePBlECizYyAU86587316;     FUlePBlECizYyAU86587316 = FUlePBlECizYyAU19083597;     FUlePBlECizYyAU19083597 = FUlePBlECizYyAU63892308;     FUlePBlECizYyAU63892308 = FUlePBlECizYyAU44023983;     FUlePBlECizYyAU44023983 = FUlePBlECizYyAU39169040;     FUlePBlECizYyAU39169040 = FUlePBlECizYyAU34199007;     FUlePBlECizYyAU34199007 = FUlePBlECizYyAU90853076;     FUlePBlECizYyAU90853076 = FUlePBlECizYyAU36264969;     FUlePBlECizYyAU36264969 = FUlePBlECizYyAU79631578;     FUlePBlECizYyAU79631578 = FUlePBlECizYyAU63556461;     FUlePBlECizYyAU63556461 = FUlePBlECizYyAU72389050;     FUlePBlECizYyAU72389050 = FUlePBlECizYyAU97003453;     FUlePBlECizYyAU97003453 = FUlePBlECizYyAU62702690;     FUlePBlECizYyAU62702690 = FUlePBlECizYyAU82080042;     FUlePBlECizYyAU82080042 = FUlePBlECizYyAU86502404;     FUlePBlECizYyAU86502404 = FUlePBlECizYyAU92814747;     FUlePBlECizYyAU92814747 = FUlePBlECizYyAU99786798;     FUlePBlECizYyAU99786798 = FUlePBlECizYyAU93355391;     FUlePBlECizYyAU93355391 = FUlePBlECizYyAU79011566;     FUlePBlECizYyAU79011566 = FUlePBlECizYyAU16952691;     FUlePBlECizYyAU16952691 = FUlePBlECizYyAU83723914;     FUlePBlECizYyAU83723914 = FUlePBlECizYyAU91533103;     FUlePBlECizYyAU91533103 = FUlePBlECizYyAU5478181;     FUlePBlECizYyAU5478181 = FUlePBlECizYyAU72760927;     FUlePBlECizYyAU72760927 = FUlePBlECizYyAU98708680;     FUlePBlECizYyAU98708680 = FUlePBlECizYyAU24225407;     FUlePBlECizYyAU24225407 = FUlePBlECizYyAU77283222;     FUlePBlECizYyAU77283222 = FUlePBlECizYyAU5791828;     FUlePBlECizYyAU5791828 = FUlePBlECizYyAU4649788;     FUlePBlECizYyAU4649788 = FUlePBlECizYyAU20379147;     FUlePBlECizYyAU20379147 = FUlePBlECizYyAU83952259;     FUlePBlECizYyAU83952259 = FUlePBlECizYyAU21076814;     FUlePBlECizYyAU21076814 = FUlePBlECizYyAU48197523;     FUlePBlECizYyAU48197523 = FUlePBlECizYyAU55398374;     FUlePBlECizYyAU55398374 = FUlePBlECizYyAU20461132;     FUlePBlECizYyAU20461132 = FUlePBlECizYyAU70603931;     FUlePBlECizYyAU70603931 = FUlePBlECizYyAU10699483;     FUlePBlECizYyAU10699483 = FUlePBlECizYyAU91885893;     FUlePBlECizYyAU91885893 = FUlePBlECizYyAU47990811;     FUlePBlECizYyAU47990811 = FUlePBlECizYyAU41296409;     FUlePBlECizYyAU41296409 = FUlePBlECizYyAU70139632;     FUlePBlECizYyAU70139632 = FUlePBlECizYyAU56900277;     FUlePBlECizYyAU56900277 = FUlePBlECizYyAU24306107;     FUlePBlECizYyAU24306107 = FUlePBlECizYyAU10256382;     FUlePBlECizYyAU10256382 = FUlePBlECizYyAU69358111;     FUlePBlECizYyAU69358111 = FUlePBlECizYyAU61263740;     FUlePBlECizYyAU61263740 = FUlePBlECizYyAU85908307;     FUlePBlECizYyAU85908307 = FUlePBlECizYyAU34175473;     FUlePBlECizYyAU34175473 = FUlePBlECizYyAU77836202;     FUlePBlECizYyAU77836202 = FUlePBlECizYyAU35856545;     FUlePBlECizYyAU35856545 = FUlePBlECizYyAU75066178;     FUlePBlECizYyAU75066178 = FUlePBlECizYyAU29933762;     FUlePBlECizYyAU29933762 = FUlePBlECizYyAU2689240;     FUlePBlECizYyAU2689240 = FUlePBlECizYyAU94515467;     FUlePBlECizYyAU94515467 = FUlePBlECizYyAU3669837;     FUlePBlECizYyAU3669837 = FUlePBlECizYyAU95054213;     FUlePBlECizYyAU95054213 = FUlePBlECizYyAU13605416;     FUlePBlECizYyAU13605416 = FUlePBlECizYyAU91131381;     FUlePBlECizYyAU91131381 = FUlePBlECizYyAU45315303;     FUlePBlECizYyAU45315303 = FUlePBlECizYyAU14943634;     FUlePBlECizYyAU14943634 = FUlePBlECizYyAU56915784;     FUlePBlECizYyAU56915784 = FUlePBlECizYyAU85061248;     FUlePBlECizYyAU85061248 = FUlePBlECizYyAU31615182;     FUlePBlECizYyAU31615182 = FUlePBlECizYyAU59252432;     FUlePBlECizYyAU59252432 = FUlePBlECizYyAU79604201;     FUlePBlECizYyAU79604201 = FUlePBlECizYyAU51312237;     FUlePBlECizYyAU51312237 = FUlePBlECizYyAU48805930;     FUlePBlECizYyAU48805930 = FUlePBlECizYyAU7304316;     FUlePBlECizYyAU7304316 = FUlePBlECizYyAU61618910;     FUlePBlECizYyAU61618910 = FUlePBlECizYyAU15898473;     FUlePBlECizYyAU15898473 = FUlePBlECizYyAU82115265;     FUlePBlECizYyAU82115265 = FUlePBlECizYyAU7900905;     FUlePBlECizYyAU7900905 = FUlePBlECizYyAU45364581;     FUlePBlECizYyAU45364581 = FUlePBlECizYyAU37715157;     FUlePBlECizYyAU37715157 = FUlePBlECizYyAU46813058;     FUlePBlECizYyAU46813058 = FUlePBlECizYyAU26823637;     FUlePBlECizYyAU26823637 = FUlePBlECizYyAU67226996;     FUlePBlECizYyAU67226996 = FUlePBlECizYyAU95221799;     FUlePBlECizYyAU95221799 = FUlePBlECizYyAU3402816;     FUlePBlECizYyAU3402816 = FUlePBlECizYyAU37444940;     FUlePBlECizYyAU37444940 = FUlePBlECizYyAU89687528;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void jXOVeMQCVhwEHgJiyqsRwtYQYfhDD9237788() {     int oBjsyrfehiIRyhy86200449 = -577312600;    int oBjsyrfehiIRyhy89674615 = -478503054;    int oBjsyrfehiIRyhy91592186 = -440590891;    int oBjsyrfehiIRyhy99708699 = -712088124;    int oBjsyrfehiIRyhy44147806 = -57144341;    int oBjsyrfehiIRyhy98239580 = -436324100;    int oBjsyrfehiIRyhy66979899 = -962654656;    int oBjsyrfehiIRyhy39389856 = -522381573;    int oBjsyrfehiIRyhy78067098 = -683403838;    int oBjsyrfehiIRyhy95751697 = -379249106;    int oBjsyrfehiIRyhy18483310 = -348251566;    int oBjsyrfehiIRyhy63627434 = -253483792;    int oBjsyrfehiIRyhy21526213 = -773509173;    int oBjsyrfehiIRyhy31413721 = -46002591;    int oBjsyrfehiIRyhy33571050 = 76860166;    int oBjsyrfehiIRyhy87809799 = -922758996;    int oBjsyrfehiIRyhy50980002 = -634710994;    int oBjsyrfehiIRyhy17836294 = -901935644;    int oBjsyrfehiIRyhy93391134 = -51953324;    int oBjsyrfehiIRyhy28119010 = -711020214;    int oBjsyrfehiIRyhy94266171 = 26486316;    int oBjsyrfehiIRyhy59985354 = -513373781;    int oBjsyrfehiIRyhy53389270 = -482797520;    int oBjsyrfehiIRyhy67848753 = -277802379;    int oBjsyrfehiIRyhy17444048 = 65728314;    int oBjsyrfehiIRyhy10223372 = -440477263;    int oBjsyrfehiIRyhy38749761 = -996920863;    int oBjsyrfehiIRyhy28515332 = 58958806;    int oBjsyrfehiIRyhy61827550 = -340651476;    int oBjsyrfehiIRyhy63810968 = -619468650;    int oBjsyrfehiIRyhy81483130 = -995868769;    int oBjsyrfehiIRyhy32753637 = -29786242;    int oBjsyrfehiIRyhy80147128 = 74917449;    int oBjsyrfehiIRyhy94739710 = -733936005;    int oBjsyrfehiIRyhy95987243 = -371773641;    int oBjsyrfehiIRyhy36486002 = -25188334;    int oBjsyrfehiIRyhy80343764 = -982262100;    int oBjsyrfehiIRyhy66482366 = -130090642;    int oBjsyrfehiIRyhy27745873 = -87734652;    int oBjsyrfehiIRyhy96470089 = -154905144;    int oBjsyrfehiIRyhy15994909 = -69370887;    int oBjsyrfehiIRyhy35726456 = -461078345;    int oBjsyrfehiIRyhy93020117 = -884939019;    int oBjsyrfehiIRyhy84969035 = -743301854;    int oBjsyrfehiIRyhy64695395 = -884254806;    int oBjsyrfehiIRyhy21906433 = -644308992;    int oBjsyrfehiIRyhy74195315 = -603887184;    int oBjsyrfehiIRyhy86281850 = 72397448;    int oBjsyrfehiIRyhy23258856 = 55175656;    int oBjsyrfehiIRyhy93180185 = -670458037;    int oBjsyrfehiIRyhy35740900 = -512455249;    int oBjsyrfehiIRyhy93485452 = -458258712;    int oBjsyrfehiIRyhy40340902 = 18357177;    int oBjsyrfehiIRyhy40905570 = -511274646;    int oBjsyrfehiIRyhy14971616 = -57863370;    int oBjsyrfehiIRyhy26215095 = 36061180;    int oBjsyrfehiIRyhy36285346 = -995705535;    int oBjsyrfehiIRyhy23743434 = -62788512;    int oBjsyrfehiIRyhy82264651 = -677816438;    int oBjsyrfehiIRyhy33924435 = -616667079;    int oBjsyrfehiIRyhy59489819 = -439403237;    int oBjsyrfehiIRyhy38464568 = -921613462;    int oBjsyrfehiIRyhy77562306 = -81730098;    int oBjsyrfehiIRyhy14256131 = 36064811;    int oBjsyrfehiIRyhy14268568 = -383380338;    int oBjsyrfehiIRyhy85729672 = -218465325;    int oBjsyrfehiIRyhy83480305 = -228401241;    int oBjsyrfehiIRyhy26786502 = 60426832;    int oBjsyrfehiIRyhy35426477 = -674228950;    int oBjsyrfehiIRyhy97085048 = -897951500;    int oBjsyrfehiIRyhy7466035 = -940496896;    int oBjsyrfehiIRyhy84497636 = -404620353;    int oBjsyrfehiIRyhy90090421 = -714200992;    int oBjsyrfehiIRyhy96921044 = -897048181;    int oBjsyrfehiIRyhy12124101 = -541649328;    int oBjsyrfehiIRyhy58539716 = -512435340;    int oBjsyrfehiIRyhy66965237 = -628434762;    int oBjsyrfehiIRyhy68420235 = -739495666;    int oBjsyrfehiIRyhy3153359 = -393547573;    int oBjsyrfehiIRyhy95537614 = -289962695;    int oBjsyrfehiIRyhy36028056 = -836590079;    int oBjsyrfehiIRyhy52467910 = -969318311;    int oBjsyrfehiIRyhy5256476 = -996216851;    int oBjsyrfehiIRyhy68647364 = -670193440;    int oBjsyrfehiIRyhy28070069 = -7013401;    int oBjsyrfehiIRyhy87997678 = -437610058;    int oBjsyrfehiIRyhy92412734 = 51856581;    int oBjsyrfehiIRyhy39241559 = -413807905;    int oBjsyrfehiIRyhy79768094 = -576072635;    int oBjsyrfehiIRyhy69772148 = -307834822;    int oBjsyrfehiIRyhy200657 = -29482800;    int oBjsyrfehiIRyhy56600331 = -819473588;    int oBjsyrfehiIRyhy84217714 = -452274204;    int oBjsyrfehiIRyhy93821438 = -471067574;    int oBjsyrfehiIRyhy36980270 = -715501907;    int oBjsyrfehiIRyhy77530341 = -147757425;    int oBjsyrfehiIRyhy58164150 = -279348247;    int oBjsyrfehiIRyhy78763986 = -821003830;    int oBjsyrfehiIRyhy70700467 = -259921517;    int oBjsyrfehiIRyhy78965722 = -577312600;     oBjsyrfehiIRyhy86200449 = oBjsyrfehiIRyhy89674615;     oBjsyrfehiIRyhy89674615 = oBjsyrfehiIRyhy91592186;     oBjsyrfehiIRyhy91592186 = oBjsyrfehiIRyhy99708699;     oBjsyrfehiIRyhy99708699 = oBjsyrfehiIRyhy44147806;     oBjsyrfehiIRyhy44147806 = oBjsyrfehiIRyhy98239580;     oBjsyrfehiIRyhy98239580 = oBjsyrfehiIRyhy66979899;     oBjsyrfehiIRyhy66979899 = oBjsyrfehiIRyhy39389856;     oBjsyrfehiIRyhy39389856 = oBjsyrfehiIRyhy78067098;     oBjsyrfehiIRyhy78067098 = oBjsyrfehiIRyhy95751697;     oBjsyrfehiIRyhy95751697 = oBjsyrfehiIRyhy18483310;     oBjsyrfehiIRyhy18483310 = oBjsyrfehiIRyhy63627434;     oBjsyrfehiIRyhy63627434 = oBjsyrfehiIRyhy21526213;     oBjsyrfehiIRyhy21526213 = oBjsyrfehiIRyhy31413721;     oBjsyrfehiIRyhy31413721 = oBjsyrfehiIRyhy33571050;     oBjsyrfehiIRyhy33571050 = oBjsyrfehiIRyhy87809799;     oBjsyrfehiIRyhy87809799 = oBjsyrfehiIRyhy50980002;     oBjsyrfehiIRyhy50980002 = oBjsyrfehiIRyhy17836294;     oBjsyrfehiIRyhy17836294 = oBjsyrfehiIRyhy93391134;     oBjsyrfehiIRyhy93391134 = oBjsyrfehiIRyhy28119010;     oBjsyrfehiIRyhy28119010 = oBjsyrfehiIRyhy94266171;     oBjsyrfehiIRyhy94266171 = oBjsyrfehiIRyhy59985354;     oBjsyrfehiIRyhy59985354 = oBjsyrfehiIRyhy53389270;     oBjsyrfehiIRyhy53389270 = oBjsyrfehiIRyhy67848753;     oBjsyrfehiIRyhy67848753 = oBjsyrfehiIRyhy17444048;     oBjsyrfehiIRyhy17444048 = oBjsyrfehiIRyhy10223372;     oBjsyrfehiIRyhy10223372 = oBjsyrfehiIRyhy38749761;     oBjsyrfehiIRyhy38749761 = oBjsyrfehiIRyhy28515332;     oBjsyrfehiIRyhy28515332 = oBjsyrfehiIRyhy61827550;     oBjsyrfehiIRyhy61827550 = oBjsyrfehiIRyhy63810968;     oBjsyrfehiIRyhy63810968 = oBjsyrfehiIRyhy81483130;     oBjsyrfehiIRyhy81483130 = oBjsyrfehiIRyhy32753637;     oBjsyrfehiIRyhy32753637 = oBjsyrfehiIRyhy80147128;     oBjsyrfehiIRyhy80147128 = oBjsyrfehiIRyhy94739710;     oBjsyrfehiIRyhy94739710 = oBjsyrfehiIRyhy95987243;     oBjsyrfehiIRyhy95987243 = oBjsyrfehiIRyhy36486002;     oBjsyrfehiIRyhy36486002 = oBjsyrfehiIRyhy80343764;     oBjsyrfehiIRyhy80343764 = oBjsyrfehiIRyhy66482366;     oBjsyrfehiIRyhy66482366 = oBjsyrfehiIRyhy27745873;     oBjsyrfehiIRyhy27745873 = oBjsyrfehiIRyhy96470089;     oBjsyrfehiIRyhy96470089 = oBjsyrfehiIRyhy15994909;     oBjsyrfehiIRyhy15994909 = oBjsyrfehiIRyhy35726456;     oBjsyrfehiIRyhy35726456 = oBjsyrfehiIRyhy93020117;     oBjsyrfehiIRyhy93020117 = oBjsyrfehiIRyhy84969035;     oBjsyrfehiIRyhy84969035 = oBjsyrfehiIRyhy64695395;     oBjsyrfehiIRyhy64695395 = oBjsyrfehiIRyhy21906433;     oBjsyrfehiIRyhy21906433 = oBjsyrfehiIRyhy74195315;     oBjsyrfehiIRyhy74195315 = oBjsyrfehiIRyhy86281850;     oBjsyrfehiIRyhy86281850 = oBjsyrfehiIRyhy23258856;     oBjsyrfehiIRyhy23258856 = oBjsyrfehiIRyhy93180185;     oBjsyrfehiIRyhy93180185 = oBjsyrfehiIRyhy35740900;     oBjsyrfehiIRyhy35740900 = oBjsyrfehiIRyhy93485452;     oBjsyrfehiIRyhy93485452 = oBjsyrfehiIRyhy40340902;     oBjsyrfehiIRyhy40340902 = oBjsyrfehiIRyhy40905570;     oBjsyrfehiIRyhy40905570 = oBjsyrfehiIRyhy14971616;     oBjsyrfehiIRyhy14971616 = oBjsyrfehiIRyhy26215095;     oBjsyrfehiIRyhy26215095 = oBjsyrfehiIRyhy36285346;     oBjsyrfehiIRyhy36285346 = oBjsyrfehiIRyhy23743434;     oBjsyrfehiIRyhy23743434 = oBjsyrfehiIRyhy82264651;     oBjsyrfehiIRyhy82264651 = oBjsyrfehiIRyhy33924435;     oBjsyrfehiIRyhy33924435 = oBjsyrfehiIRyhy59489819;     oBjsyrfehiIRyhy59489819 = oBjsyrfehiIRyhy38464568;     oBjsyrfehiIRyhy38464568 = oBjsyrfehiIRyhy77562306;     oBjsyrfehiIRyhy77562306 = oBjsyrfehiIRyhy14256131;     oBjsyrfehiIRyhy14256131 = oBjsyrfehiIRyhy14268568;     oBjsyrfehiIRyhy14268568 = oBjsyrfehiIRyhy85729672;     oBjsyrfehiIRyhy85729672 = oBjsyrfehiIRyhy83480305;     oBjsyrfehiIRyhy83480305 = oBjsyrfehiIRyhy26786502;     oBjsyrfehiIRyhy26786502 = oBjsyrfehiIRyhy35426477;     oBjsyrfehiIRyhy35426477 = oBjsyrfehiIRyhy97085048;     oBjsyrfehiIRyhy97085048 = oBjsyrfehiIRyhy7466035;     oBjsyrfehiIRyhy7466035 = oBjsyrfehiIRyhy84497636;     oBjsyrfehiIRyhy84497636 = oBjsyrfehiIRyhy90090421;     oBjsyrfehiIRyhy90090421 = oBjsyrfehiIRyhy96921044;     oBjsyrfehiIRyhy96921044 = oBjsyrfehiIRyhy12124101;     oBjsyrfehiIRyhy12124101 = oBjsyrfehiIRyhy58539716;     oBjsyrfehiIRyhy58539716 = oBjsyrfehiIRyhy66965237;     oBjsyrfehiIRyhy66965237 = oBjsyrfehiIRyhy68420235;     oBjsyrfehiIRyhy68420235 = oBjsyrfehiIRyhy3153359;     oBjsyrfehiIRyhy3153359 = oBjsyrfehiIRyhy95537614;     oBjsyrfehiIRyhy95537614 = oBjsyrfehiIRyhy36028056;     oBjsyrfehiIRyhy36028056 = oBjsyrfehiIRyhy52467910;     oBjsyrfehiIRyhy52467910 = oBjsyrfehiIRyhy5256476;     oBjsyrfehiIRyhy5256476 = oBjsyrfehiIRyhy68647364;     oBjsyrfehiIRyhy68647364 = oBjsyrfehiIRyhy28070069;     oBjsyrfehiIRyhy28070069 = oBjsyrfehiIRyhy87997678;     oBjsyrfehiIRyhy87997678 = oBjsyrfehiIRyhy92412734;     oBjsyrfehiIRyhy92412734 = oBjsyrfehiIRyhy39241559;     oBjsyrfehiIRyhy39241559 = oBjsyrfehiIRyhy79768094;     oBjsyrfehiIRyhy79768094 = oBjsyrfehiIRyhy69772148;     oBjsyrfehiIRyhy69772148 = oBjsyrfehiIRyhy200657;     oBjsyrfehiIRyhy200657 = oBjsyrfehiIRyhy56600331;     oBjsyrfehiIRyhy56600331 = oBjsyrfehiIRyhy84217714;     oBjsyrfehiIRyhy84217714 = oBjsyrfehiIRyhy93821438;     oBjsyrfehiIRyhy93821438 = oBjsyrfehiIRyhy36980270;     oBjsyrfehiIRyhy36980270 = oBjsyrfehiIRyhy77530341;     oBjsyrfehiIRyhy77530341 = oBjsyrfehiIRyhy58164150;     oBjsyrfehiIRyhy58164150 = oBjsyrfehiIRyhy78763986;     oBjsyrfehiIRyhy78763986 = oBjsyrfehiIRyhy70700467;     oBjsyrfehiIRyhy70700467 = oBjsyrfehiIRyhy78965722;     oBjsyrfehiIRyhy78965722 = oBjsyrfehiIRyhy86200449;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ycgSYnEtSCjyvKKZaVCWnbwpoTVzZ11427126() {     int zpbqEXPzqLqxuyb82713370 = -576113288;    int zpbqEXPzqLqxuyb4757442 = -431191356;    int zpbqEXPzqLqxuyb47274498 = -188693656;    int zpbqEXPzqLqxuyb12257548 = -80901914;    int zpbqEXPzqLqxuyb12800196 = -712731337;    int zpbqEXPzqLqxuyb35486453 = -466851676;    int zpbqEXPzqLqxuyb40794552 = -27572631;    int zpbqEXPzqLqxuyb74842026 = -635265758;    int zpbqEXPzqLqxuyb82321355 = -935821592;    int zpbqEXPzqLqxuyb49756234 = -37974319;    int zpbqEXPzqLqxuyb78699425 = -323486803;    int zpbqEXPzqLqxuyb78643871 = -822216697;    int zpbqEXPzqLqxuyb26796910 = 40705163;    int zpbqEXPzqLqxuyb98488833 = -555524126;    int zpbqEXPzqLqxuyb38470807 = -874466544;    int zpbqEXPzqLqxuyb766624 = -681380211;    int zpbqEXPzqLqxuyb78670850 = -547558038;    int zpbqEXPzqLqxuyb53971782 = -419461709;    int zpbqEXPzqLqxuyb75314110 = -541080625;    int zpbqEXPzqLqxuyb68844269 = -620545045;    int zpbqEXPzqLqxuyb1945027 = -184063282;    int zpbqEXPzqLqxuyb887112 = -891391473;    int zpbqEXPzqLqxuyb42886233 = -763717230;    int zpbqEXPzqLqxuyb91673523 = -236919784;    int zpbqEXPzqLqxuyb95719054 = -559697445;    int zpbqEXPzqLqxuyb86247735 = -619772526;    int zpbqEXPzqLqxuyb86646445 = -824102710;    int zpbqEXPzqLqxuyb20765694 = -696797293;    int zpbqEXPzqLqxuyb44023521 = -206029459;    int zpbqEXPzqLqxuyb64065476 = 15754508;    int zpbqEXPzqLqxuyb90577209 = -197836922;    int zpbqEXPzqLqxuyb68503820 = -361792262;    int zpbqEXPzqLqxuyb97591567 = -915179319;    int zpbqEXPzqLqxuyb7399379 = -554446005;    int zpbqEXPzqLqxuyb5472082 = -586439167;    int zpbqEXPzqLqxuyb80157255 = -302265043;    int zpbqEXPzqLqxuyb60900731 = -492686058;    int zpbqEXPzqLqxuyb39609340 = 48777390;    int zpbqEXPzqLqxuyb76480180 = -428803403;    int zpbqEXPzqLqxuyb75987488 = -691253026;    int zpbqEXPzqLqxuyb48265904 = -600913057;    int zpbqEXPzqLqxuyb79919807 = -968519307;    int zpbqEXPzqLqxuyb80562053 = -281211591;    int zpbqEXPzqLqxuyb97177143 = -884935607;    int zpbqEXPzqLqxuyb30682110 = -316409144;    int zpbqEXPzqLqxuyb19587460 = -976636788;    int zpbqEXPzqLqxuyb71107408 = -882407832;    int zpbqEXPzqLqxuyb66771874 = -90696552;    int zpbqEXPzqLqxuyb41867924 = -576328539;    int zpbqEXPzqLqxuyb65981224 = 80002508;    int zpbqEXPzqLqxuyb87529539 = -41834456;    int zpbqEXPzqLqxuyb65894090 = 59576038;    int zpbqEXPzqLqxuyb32484280 = -773551467;    int zpbqEXPzqLqxuyb26412766 = -961475650;    int zpbqEXPzqLqxuyb9482100 = -490585163;    int zpbqEXPzqLqxuyb81826258 = -684721816;    int zpbqEXPzqLqxuyb61871208 = -667474126;    int zpbqEXPzqLqxuyb55600974 = -951773873;    int zpbqEXPzqLqxuyb16538493 = -521204470;    int zpbqEXPzqLqxuyb26552460 = 7041188;    int zpbqEXPzqLqxuyb48840007 = -642748967;    int zpbqEXPzqLqxuyb20028858 = -330775339;    int zpbqEXPzqLqxuyb30818505 = -329236299;    int zpbqEXPzqLqxuyb18255880 = -851576100;    int zpbqEXPzqLqxuyb59179024 = -840137397;    int zpbqEXPzqLqxuyb10195605 = -961694542;    int zpbqEXPzqLqxuyb81052304 = -907037379;    int zpbqEXPzqLqxuyb19397532 = -404848832;    int zpbqEXPzqLqxuyb93016751 = -969084959;    int zpbqEXPzqLqxuyb58313552 = -472201501;    int zpbqEXPzqLqxuyb39865892 = -88694154;    int zpbqEXPzqLqxuyb39061510 = -496335428;    int zpbqEXPzqLqxuyb77491602 = -990658307;    int zpbqEXPzqLqxuyb99326621 = -849827599;    int zpbqEXPzqLqxuyb20578365 = 80368012;    int zpbqEXPzqLqxuyb22025219 = -215543976;    int zpbqEXPzqLqxuyb20325059 = -510179883;    int zpbqEXPzqLqxuyb45709090 = -878781623;    int zpbqEXPzqLqxuyb60991414 = -920510640;    int zpbqEXPzqLqxuyb76131595 = -583060658;    int zpbqEXPzqLqxuyb15140328 = -737364694;    int zpbqEXPzqLqxuyb19874572 = -633406159;    int zpbqEXPzqLqxuyb78897770 = -20468755;    int zpbqEXPzqLqxuyb78042296 = -186031967;    int zpbqEXPzqLqxuyb76535936 = -942411037;    int zpbqEXPzqLqxuyb24683120 = -157412961;    int zpbqEXPzqLqxuyb36019540 = -588240795;    int zpbqEXPzqLqxuyb71178801 = -953703669;    int zpbqEXPzqLqxuyb97917279 = 36139157;    int zpbqEXPzqLqxuyb23645823 = -901717352;    int zpbqEXPzqLqxuyb18286047 = -634790917;    int zpbqEXPzqLqxuyb5299758 = -540912185;    int zpbqEXPzqLqxuyb23070848 = -430018141;    int zpbqEXPzqLqxuyb49927720 = -335844591;    int zpbqEXPzqLqxuyb27147482 = 51495940;    int zpbqEXPzqLqxuyb28237046 = -170137719;    int zpbqEXPzqLqxuyb49101303 = -539283009;    int zpbqEXPzqLqxuyb62306173 = -429635491;    int zpbqEXPzqLqxuyb37998119 = 55201789;    int zpbqEXPzqLqxuyb20486505 = -576113288;     zpbqEXPzqLqxuyb82713370 = zpbqEXPzqLqxuyb4757442;     zpbqEXPzqLqxuyb4757442 = zpbqEXPzqLqxuyb47274498;     zpbqEXPzqLqxuyb47274498 = zpbqEXPzqLqxuyb12257548;     zpbqEXPzqLqxuyb12257548 = zpbqEXPzqLqxuyb12800196;     zpbqEXPzqLqxuyb12800196 = zpbqEXPzqLqxuyb35486453;     zpbqEXPzqLqxuyb35486453 = zpbqEXPzqLqxuyb40794552;     zpbqEXPzqLqxuyb40794552 = zpbqEXPzqLqxuyb74842026;     zpbqEXPzqLqxuyb74842026 = zpbqEXPzqLqxuyb82321355;     zpbqEXPzqLqxuyb82321355 = zpbqEXPzqLqxuyb49756234;     zpbqEXPzqLqxuyb49756234 = zpbqEXPzqLqxuyb78699425;     zpbqEXPzqLqxuyb78699425 = zpbqEXPzqLqxuyb78643871;     zpbqEXPzqLqxuyb78643871 = zpbqEXPzqLqxuyb26796910;     zpbqEXPzqLqxuyb26796910 = zpbqEXPzqLqxuyb98488833;     zpbqEXPzqLqxuyb98488833 = zpbqEXPzqLqxuyb38470807;     zpbqEXPzqLqxuyb38470807 = zpbqEXPzqLqxuyb766624;     zpbqEXPzqLqxuyb766624 = zpbqEXPzqLqxuyb78670850;     zpbqEXPzqLqxuyb78670850 = zpbqEXPzqLqxuyb53971782;     zpbqEXPzqLqxuyb53971782 = zpbqEXPzqLqxuyb75314110;     zpbqEXPzqLqxuyb75314110 = zpbqEXPzqLqxuyb68844269;     zpbqEXPzqLqxuyb68844269 = zpbqEXPzqLqxuyb1945027;     zpbqEXPzqLqxuyb1945027 = zpbqEXPzqLqxuyb887112;     zpbqEXPzqLqxuyb887112 = zpbqEXPzqLqxuyb42886233;     zpbqEXPzqLqxuyb42886233 = zpbqEXPzqLqxuyb91673523;     zpbqEXPzqLqxuyb91673523 = zpbqEXPzqLqxuyb95719054;     zpbqEXPzqLqxuyb95719054 = zpbqEXPzqLqxuyb86247735;     zpbqEXPzqLqxuyb86247735 = zpbqEXPzqLqxuyb86646445;     zpbqEXPzqLqxuyb86646445 = zpbqEXPzqLqxuyb20765694;     zpbqEXPzqLqxuyb20765694 = zpbqEXPzqLqxuyb44023521;     zpbqEXPzqLqxuyb44023521 = zpbqEXPzqLqxuyb64065476;     zpbqEXPzqLqxuyb64065476 = zpbqEXPzqLqxuyb90577209;     zpbqEXPzqLqxuyb90577209 = zpbqEXPzqLqxuyb68503820;     zpbqEXPzqLqxuyb68503820 = zpbqEXPzqLqxuyb97591567;     zpbqEXPzqLqxuyb97591567 = zpbqEXPzqLqxuyb7399379;     zpbqEXPzqLqxuyb7399379 = zpbqEXPzqLqxuyb5472082;     zpbqEXPzqLqxuyb5472082 = zpbqEXPzqLqxuyb80157255;     zpbqEXPzqLqxuyb80157255 = zpbqEXPzqLqxuyb60900731;     zpbqEXPzqLqxuyb60900731 = zpbqEXPzqLqxuyb39609340;     zpbqEXPzqLqxuyb39609340 = zpbqEXPzqLqxuyb76480180;     zpbqEXPzqLqxuyb76480180 = zpbqEXPzqLqxuyb75987488;     zpbqEXPzqLqxuyb75987488 = zpbqEXPzqLqxuyb48265904;     zpbqEXPzqLqxuyb48265904 = zpbqEXPzqLqxuyb79919807;     zpbqEXPzqLqxuyb79919807 = zpbqEXPzqLqxuyb80562053;     zpbqEXPzqLqxuyb80562053 = zpbqEXPzqLqxuyb97177143;     zpbqEXPzqLqxuyb97177143 = zpbqEXPzqLqxuyb30682110;     zpbqEXPzqLqxuyb30682110 = zpbqEXPzqLqxuyb19587460;     zpbqEXPzqLqxuyb19587460 = zpbqEXPzqLqxuyb71107408;     zpbqEXPzqLqxuyb71107408 = zpbqEXPzqLqxuyb66771874;     zpbqEXPzqLqxuyb66771874 = zpbqEXPzqLqxuyb41867924;     zpbqEXPzqLqxuyb41867924 = zpbqEXPzqLqxuyb65981224;     zpbqEXPzqLqxuyb65981224 = zpbqEXPzqLqxuyb87529539;     zpbqEXPzqLqxuyb87529539 = zpbqEXPzqLqxuyb65894090;     zpbqEXPzqLqxuyb65894090 = zpbqEXPzqLqxuyb32484280;     zpbqEXPzqLqxuyb32484280 = zpbqEXPzqLqxuyb26412766;     zpbqEXPzqLqxuyb26412766 = zpbqEXPzqLqxuyb9482100;     zpbqEXPzqLqxuyb9482100 = zpbqEXPzqLqxuyb81826258;     zpbqEXPzqLqxuyb81826258 = zpbqEXPzqLqxuyb61871208;     zpbqEXPzqLqxuyb61871208 = zpbqEXPzqLqxuyb55600974;     zpbqEXPzqLqxuyb55600974 = zpbqEXPzqLqxuyb16538493;     zpbqEXPzqLqxuyb16538493 = zpbqEXPzqLqxuyb26552460;     zpbqEXPzqLqxuyb26552460 = zpbqEXPzqLqxuyb48840007;     zpbqEXPzqLqxuyb48840007 = zpbqEXPzqLqxuyb20028858;     zpbqEXPzqLqxuyb20028858 = zpbqEXPzqLqxuyb30818505;     zpbqEXPzqLqxuyb30818505 = zpbqEXPzqLqxuyb18255880;     zpbqEXPzqLqxuyb18255880 = zpbqEXPzqLqxuyb59179024;     zpbqEXPzqLqxuyb59179024 = zpbqEXPzqLqxuyb10195605;     zpbqEXPzqLqxuyb10195605 = zpbqEXPzqLqxuyb81052304;     zpbqEXPzqLqxuyb81052304 = zpbqEXPzqLqxuyb19397532;     zpbqEXPzqLqxuyb19397532 = zpbqEXPzqLqxuyb93016751;     zpbqEXPzqLqxuyb93016751 = zpbqEXPzqLqxuyb58313552;     zpbqEXPzqLqxuyb58313552 = zpbqEXPzqLqxuyb39865892;     zpbqEXPzqLqxuyb39865892 = zpbqEXPzqLqxuyb39061510;     zpbqEXPzqLqxuyb39061510 = zpbqEXPzqLqxuyb77491602;     zpbqEXPzqLqxuyb77491602 = zpbqEXPzqLqxuyb99326621;     zpbqEXPzqLqxuyb99326621 = zpbqEXPzqLqxuyb20578365;     zpbqEXPzqLqxuyb20578365 = zpbqEXPzqLqxuyb22025219;     zpbqEXPzqLqxuyb22025219 = zpbqEXPzqLqxuyb20325059;     zpbqEXPzqLqxuyb20325059 = zpbqEXPzqLqxuyb45709090;     zpbqEXPzqLqxuyb45709090 = zpbqEXPzqLqxuyb60991414;     zpbqEXPzqLqxuyb60991414 = zpbqEXPzqLqxuyb76131595;     zpbqEXPzqLqxuyb76131595 = zpbqEXPzqLqxuyb15140328;     zpbqEXPzqLqxuyb15140328 = zpbqEXPzqLqxuyb19874572;     zpbqEXPzqLqxuyb19874572 = zpbqEXPzqLqxuyb78897770;     zpbqEXPzqLqxuyb78897770 = zpbqEXPzqLqxuyb78042296;     zpbqEXPzqLqxuyb78042296 = zpbqEXPzqLqxuyb76535936;     zpbqEXPzqLqxuyb76535936 = zpbqEXPzqLqxuyb24683120;     zpbqEXPzqLqxuyb24683120 = zpbqEXPzqLqxuyb36019540;     zpbqEXPzqLqxuyb36019540 = zpbqEXPzqLqxuyb71178801;     zpbqEXPzqLqxuyb71178801 = zpbqEXPzqLqxuyb97917279;     zpbqEXPzqLqxuyb97917279 = zpbqEXPzqLqxuyb23645823;     zpbqEXPzqLqxuyb23645823 = zpbqEXPzqLqxuyb18286047;     zpbqEXPzqLqxuyb18286047 = zpbqEXPzqLqxuyb5299758;     zpbqEXPzqLqxuyb5299758 = zpbqEXPzqLqxuyb23070848;     zpbqEXPzqLqxuyb23070848 = zpbqEXPzqLqxuyb49927720;     zpbqEXPzqLqxuyb49927720 = zpbqEXPzqLqxuyb27147482;     zpbqEXPzqLqxuyb27147482 = zpbqEXPzqLqxuyb28237046;     zpbqEXPzqLqxuyb28237046 = zpbqEXPzqLqxuyb49101303;     zpbqEXPzqLqxuyb49101303 = zpbqEXPzqLqxuyb62306173;     zpbqEXPzqLqxuyb62306173 = zpbqEXPzqLqxuyb37998119;     zpbqEXPzqLqxuyb37998119 = zpbqEXPzqLqxuyb20486505;     zpbqEXPzqLqxuyb20486505 = zpbqEXPzqLqxuyb82713370;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void rRKYNgunckFNQikbJijWskNoVIFkF13616465() {     int JkZpggJdqfFzJhX79226291 = -574913976;    int JkZpggJdqfFzJhX19840267 = -383879658;    int JkZpggJdqfFzJhX2956809 = 63203578;    int JkZpggJdqfFzJhX24806396 = -549715705;    int JkZpggJdqfFzJhX81452585 = -268318333;    int JkZpggJdqfFzJhX72733324 = -497379253;    int JkZpggJdqfFzJhX14609206 = -192490606;    int JkZpggJdqfFzJhX10294196 = -748149942;    int JkZpggJdqfFzJhX86575612 = -88239345;    int JkZpggJdqfFzJhX3760771 = -796699531;    int JkZpggJdqfFzJhX38915541 = -298722041;    int JkZpggJdqfFzJhX93660308 = -290949603;    int JkZpggJdqfFzJhX32067608 = -245080500;    int JkZpggJdqfFzJhX65563946 = 34954339;    int JkZpggJdqfFzJhX43370564 = -725793254;    int JkZpggJdqfFzJhX13723447 = -440001426;    int JkZpggJdqfFzJhX6361700 = -460405083;    int JkZpggJdqfFzJhX90107270 = 63012226;    int JkZpggJdqfFzJhX57237086 = 69792075;    int JkZpggJdqfFzJhX9569529 = -530069875;    int JkZpggJdqfFzJhX9623882 = -394612880;    int JkZpggJdqfFzJhX41788869 = -169409166;    int JkZpggJdqfFzJhX32383196 = 55363060;    int JkZpggJdqfFzJhX15498294 = -196037188;    int JkZpggJdqfFzJhX73994062 = -85123204;    int JkZpggJdqfFzJhX62272100 = -799067788;    int JkZpggJdqfFzJhX34543130 = -651284557;    int JkZpggJdqfFzJhX13016057 = -352553392;    int JkZpggJdqfFzJhX26219493 = -71407442;    int JkZpggJdqfFzJhX64319984 = -449022334;    int JkZpggJdqfFzJhX99671289 = -499805075;    int JkZpggJdqfFzJhX4254004 = -693798282;    int JkZpggJdqfFzJhX15036007 = -805276086;    int JkZpggJdqfFzJhX20059047 = -374956005;    int JkZpggJdqfFzJhX14956920 = -801104693;    int JkZpggJdqfFzJhX23828509 = -579341751;    int JkZpggJdqfFzJhX41457698 = -3110015;    int JkZpggJdqfFzJhX12736315 = -872354579;    int JkZpggJdqfFzJhX25214487 = -769872153;    int JkZpggJdqfFzJhX55504888 = -127600908;    int JkZpggJdqfFzJhX80536899 = -32455228;    int JkZpggJdqfFzJhX24113160 = -375960270;    int JkZpggJdqfFzJhX68103989 = -777484163;    int JkZpggJdqfFzJhX9385252 = 73430639;    int JkZpggJdqfFzJhX96668825 = -848563482;    int JkZpggJdqfFzJhX17268487 = -208964584;    int JkZpggJdqfFzJhX68019501 = -60928480;    int JkZpggJdqfFzJhX47261897 = -253790551;    int JkZpggJdqfFzJhX60476991 = -107832734;    int JkZpggJdqfFzJhX38782264 = -269536947;    int JkZpggJdqfFzJhX39318180 = -671213662;    int JkZpggJdqfFzJhX38302729 = -522589212;    int JkZpggJdqfFzJhX24627659 = -465460112;    int JkZpggJdqfFzJhX11919962 = -311676654;    int JkZpggJdqfFzJhX3992583 = -923306955;    int JkZpggJdqfFzJhX37437422 = -305504811;    int JkZpggJdqfFzJhX87457071 = -339242718;    int JkZpggJdqfFzJhX87458514 = -740759235;    int JkZpggJdqfFzJhX50812334 = -364592501;    int JkZpggJdqfFzJhX19180486 = -469250545;    int JkZpggJdqfFzJhX38190194 = -846094696;    int JkZpggJdqfFzJhX1593149 = -839937215;    int JkZpggJdqfFzJhX84074703 = -576742500;    int JkZpggJdqfFzJhX22255629 = -639217012;    int JkZpggJdqfFzJhX4089482 = -196894457;    int JkZpggJdqfFzJhX34661537 = -604923759;    int JkZpggJdqfFzJhX78624302 = -485673517;    int JkZpggJdqfFzJhX12008561 = -870124496;    int JkZpggJdqfFzJhX50607026 = -163940969;    int JkZpggJdqfFzJhX19542056 = -46451503;    int JkZpggJdqfFzJhX72265748 = -336891411;    int JkZpggJdqfFzJhX93625384 = -588050504;    int JkZpggJdqfFzJhX64892784 = -167115622;    int JkZpggJdqfFzJhX1732199 = -802607017;    int JkZpggJdqfFzJhX29032629 = -397614648;    int JkZpggJdqfFzJhX85510721 = 81347389;    int JkZpggJdqfFzJhX73684880 = -391925003;    int JkZpggJdqfFzJhX22997944 = 81932420;    int JkZpggJdqfFzJhX18829469 = -347473706;    int JkZpggJdqfFzJhX56725576 = -876158621;    int JkZpggJdqfFzJhX94252599 = -638139308;    int JkZpggJdqfFzJhX87281233 = -297494007;    int JkZpggJdqfFzJhX52539065 = -144720659;    int JkZpggJdqfFzJhX87437228 = -801870495;    int JkZpggJdqfFzJhX25001804 = -777808673;    int JkZpggJdqfFzJhX61368561 = -977215864;    int JkZpggJdqfFzJhX79626344 = -128338171;    int JkZpggJdqfFzJhX3116045 = -393599433;    int JkZpggJdqfFzJhX16066465 = -451649050;    int JkZpggJdqfFzJhX77519498 = -395599882;    int JkZpggJdqfFzJhX36371438 = -140099034;    int JkZpggJdqfFzJhX53999184 = -262350781;    int JkZpggJdqfFzJhX61923981 = -407762079;    int JkZpggJdqfFzJhX6034002 = -200621608;    int JkZpggJdqfFzJhX17314694 = -281506212;    int JkZpggJdqfFzJhX78943750 = -192518014;    int JkZpggJdqfFzJhX40038456 = -799217770;    int JkZpggJdqfFzJhX45848360 = -38267152;    int JkZpggJdqfFzJhX5295771 = -729674905;    int JkZpggJdqfFzJhX62007288 = -574913976;     JkZpggJdqfFzJhX79226291 = JkZpggJdqfFzJhX19840267;     JkZpggJdqfFzJhX19840267 = JkZpggJdqfFzJhX2956809;     JkZpggJdqfFzJhX2956809 = JkZpggJdqfFzJhX24806396;     JkZpggJdqfFzJhX24806396 = JkZpggJdqfFzJhX81452585;     JkZpggJdqfFzJhX81452585 = JkZpggJdqfFzJhX72733324;     JkZpggJdqfFzJhX72733324 = JkZpggJdqfFzJhX14609206;     JkZpggJdqfFzJhX14609206 = JkZpggJdqfFzJhX10294196;     JkZpggJdqfFzJhX10294196 = JkZpggJdqfFzJhX86575612;     JkZpggJdqfFzJhX86575612 = JkZpggJdqfFzJhX3760771;     JkZpggJdqfFzJhX3760771 = JkZpggJdqfFzJhX38915541;     JkZpggJdqfFzJhX38915541 = JkZpggJdqfFzJhX93660308;     JkZpggJdqfFzJhX93660308 = JkZpggJdqfFzJhX32067608;     JkZpggJdqfFzJhX32067608 = JkZpggJdqfFzJhX65563946;     JkZpggJdqfFzJhX65563946 = JkZpggJdqfFzJhX43370564;     JkZpggJdqfFzJhX43370564 = JkZpggJdqfFzJhX13723447;     JkZpggJdqfFzJhX13723447 = JkZpggJdqfFzJhX6361700;     JkZpggJdqfFzJhX6361700 = JkZpggJdqfFzJhX90107270;     JkZpggJdqfFzJhX90107270 = JkZpggJdqfFzJhX57237086;     JkZpggJdqfFzJhX57237086 = JkZpggJdqfFzJhX9569529;     JkZpggJdqfFzJhX9569529 = JkZpggJdqfFzJhX9623882;     JkZpggJdqfFzJhX9623882 = JkZpggJdqfFzJhX41788869;     JkZpggJdqfFzJhX41788869 = JkZpggJdqfFzJhX32383196;     JkZpggJdqfFzJhX32383196 = JkZpggJdqfFzJhX15498294;     JkZpggJdqfFzJhX15498294 = JkZpggJdqfFzJhX73994062;     JkZpggJdqfFzJhX73994062 = JkZpggJdqfFzJhX62272100;     JkZpggJdqfFzJhX62272100 = JkZpggJdqfFzJhX34543130;     JkZpggJdqfFzJhX34543130 = JkZpggJdqfFzJhX13016057;     JkZpggJdqfFzJhX13016057 = JkZpggJdqfFzJhX26219493;     JkZpggJdqfFzJhX26219493 = JkZpggJdqfFzJhX64319984;     JkZpggJdqfFzJhX64319984 = JkZpggJdqfFzJhX99671289;     JkZpggJdqfFzJhX99671289 = JkZpggJdqfFzJhX4254004;     JkZpggJdqfFzJhX4254004 = JkZpggJdqfFzJhX15036007;     JkZpggJdqfFzJhX15036007 = JkZpggJdqfFzJhX20059047;     JkZpggJdqfFzJhX20059047 = JkZpggJdqfFzJhX14956920;     JkZpggJdqfFzJhX14956920 = JkZpggJdqfFzJhX23828509;     JkZpggJdqfFzJhX23828509 = JkZpggJdqfFzJhX41457698;     JkZpggJdqfFzJhX41457698 = JkZpggJdqfFzJhX12736315;     JkZpggJdqfFzJhX12736315 = JkZpggJdqfFzJhX25214487;     JkZpggJdqfFzJhX25214487 = JkZpggJdqfFzJhX55504888;     JkZpggJdqfFzJhX55504888 = JkZpggJdqfFzJhX80536899;     JkZpggJdqfFzJhX80536899 = JkZpggJdqfFzJhX24113160;     JkZpggJdqfFzJhX24113160 = JkZpggJdqfFzJhX68103989;     JkZpggJdqfFzJhX68103989 = JkZpggJdqfFzJhX9385252;     JkZpggJdqfFzJhX9385252 = JkZpggJdqfFzJhX96668825;     JkZpggJdqfFzJhX96668825 = JkZpggJdqfFzJhX17268487;     JkZpggJdqfFzJhX17268487 = JkZpggJdqfFzJhX68019501;     JkZpggJdqfFzJhX68019501 = JkZpggJdqfFzJhX47261897;     JkZpggJdqfFzJhX47261897 = JkZpggJdqfFzJhX60476991;     JkZpggJdqfFzJhX60476991 = JkZpggJdqfFzJhX38782264;     JkZpggJdqfFzJhX38782264 = JkZpggJdqfFzJhX39318180;     JkZpggJdqfFzJhX39318180 = JkZpggJdqfFzJhX38302729;     JkZpggJdqfFzJhX38302729 = JkZpggJdqfFzJhX24627659;     JkZpggJdqfFzJhX24627659 = JkZpggJdqfFzJhX11919962;     JkZpggJdqfFzJhX11919962 = JkZpggJdqfFzJhX3992583;     JkZpggJdqfFzJhX3992583 = JkZpggJdqfFzJhX37437422;     JkZpggJdqfFzJhX37437422 = JkZpggJdqfFzJhX87457071;     JkZpggJdqfFzJhX87457071 = JkZpggJdqfFzJhX87458514;     JkZpggJdqfFzJhX87458514 = JkZpggJdqfFzJhX50812334;     JkZpggJdqfFzJhX50812334 = JkZpggJdqfFzJhX19180486;     JkZpggJdqfFzJhX19180486 = JkZpggJdqfFzJhX38190194;     JkZpggJdqfFzJhX38190194 = JkZpggJdqfFzJhX1593149;     JkZpggJdqfFzJhX1593149 = JkZpggJdqfFzJhX84074703;     JkZpggJdqfFzJhX84074703 = JkZpggJdqfFzJhX22255629;     JkZpggJdqfFzJhX22255629 = JkZpggJdqfFzJhX4089482;     JkZpggJdqfFzJhX4089482 = JkZpggJdqfFzJhX34661537;     JkZpggJdqfFzJhX34661537 = JkZpggJdqfFzJhX78624302;     JkZpggJdqfFzJhX78624302 = JkZpggJdqfFzJhX12008561;     JkZpggJdqfFzJhX12008561 = JkZpggJdqfFzJhX50607026;     JkZpggJdqfFzJhX50607026 = JkZpggJdqfFzJhX19542056;     JkZpggJdqfFzJhX19542056 = JkZpggJdqfFzJhX72265748;     JkZpggJdqfFzJhX72265748 = JkZpggJdqfFzJhX93625384;     JkZpggJdqfFzJhX93625384 = JkZpggJdqfFzJhX64892784;     JkZpggJdqfFzJhX64892784 = JkZpggJdqfFzJhX1732199;     JkZpggJdqfFzJhX1732199 = JkZpggJdqfFzJhX29032629;     JkZpggJdqfFzJhX29032629 = JkZpggJdqfFzJhX85510721;     JkZpggJdqfFzJhX85510721 = JkZpggJdqfFzJhX73684880;     JkZpggJdqfFzJhX73684880 = JkZpggJdqfFzJhX22997944;     JkZpggJdqfFzJhX22997944 = JkZpggJdqfFzJhX18829469;     JkZpggJdqfFzJhX18829469 = JkZpggJdqfFzJhX56725576;     JkZpggJdqfFzJhX56725576 = JkZpggJdqfFzJhX94252599;     JkZpggJdqfFzJhX94252599 = JkZpggJdqfFzJhX87281233;     JkZpggJdqfFzJhX87281233 = JkZpggJdqfFzJhX52539065;     JkZpggJdqfFzJhX52539065 = JkZpggJdqfFzJhX87437228;     JkZpggJdqfFzJhX87437228 = JkZpggJdqfFzJhX25001804;     JkZpggJdqfFzJhX25001804 = JkZpggJdqfFzJhX61368561;     JkZpggJdqfFzJhX61368561 = JkZpggJdqfFzJhX79626344;     JkZpggJdqfFzJhX79626344 = JkZpggJdqfFzJhX3116045;     JkZpggJdqfFzJhX3116045 = JkZpggJdqfFzJhX16066465;     JkZpggJdqfFzJhX16066465 = JkZpggJdqfFzJhX77519498;     JkZpggJdqfFzJhX77519498 = JkZpggJdqfFzJhX36371438;     JkZpggJdqfFzJhX36371438 = JkZpggJdqfFzJhX53999184;     JkZpggJdqfFzJhX53999184 = JkZpggJdqfFzJhX61923981;     JkZpggJdqfFzJhX61923981 = JkZpggJdqfFzJhX6034002;     JkZpggJdqfFzJhX6034002 = JkZpggJdqfFzJhX17314694;     JkZpggJdqfFzJhX17314694 = JkZpggJdqfFzJhX78943750;     JkZpggJdqfFzJhX78943750 = JkZpggJdqfFzJhX40038456;     JkZpggJdqfFzJhX40038456 = JkZpggJdqfFzJhX45848360;     JkZpggJdqfFzJhX45848360 = JkZpggJdqfFzJhX5295771;     JkZpggJdqfFzJhX5295771 = JkZpggJdqfFzJhX62007288;     JkZpggJdqfFzJhX62007288 = JkZpggJdqfFzJhX79226291;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FnmNWpGxNqQfmHnboRYWlaJpZXkzr63563271() {     int cmhIjflYNUYBwgN46397351 = -219339654;    int cmhIjflYNUYBwgN65024293 = -165616134;    int cmhIjflYNUYBwgN37294827 = -170065034;    int cmhIjflYNUYBwgN92206670 = -288127054;    int cmhIjflYNUYBwgN62715195 = -238956355;    int cmhIjflYNUYBwgN65174527 = -530055169;    int cmhIjflYNUYBwgN38255418 = -651647824;    int cmhIjflYNUYBwgN33374696 = -403673899;    int cmhIjflYNUYBwgN53954759 = -720358358;    int cmhIjflYNUYBwgN84092254 = -19713224;    int cmhIjflYNUYBwgN21060212 = -349387135;    int cmhIjflYNUYBwgN39931640 = -339967939;    int cmhIjflYNUYBwgN85312778 = -418744747;    int cmhIjflYNUYBwgN62641873 = 69557645;    int cmhIjflYNUYBwgN85818232 = -764157115;    int cmhIjflYNUYBwgN84644363 = -535810578;    int cmhIjflYNUYBwgN72052615 = -462822688;    int cmhIjflYNUYBwgN23535720 = -146608414;    int cmhIjflYNUYBwgN439575 = -508540901;    int cmhIjflYNUYBwgN37007734 = -369936559;    int cmhIjflYNUYBwgN92769066 = -803375245;    int cmhIjflYNUYBwgN65790126 = -368720281;    int cmhIjflYNUYBwgN86945570 = -334441954;    int cmhIjflYNUYBwgN29522493 = -966634976;    int cmhIjflYNUYBwgN87950768 = 40240202;    int cmhIjflYNUYBwgN52319968 = -68441567;    int cmhIjflYNUYBwgN20479019 = -572844466;    int cmhIjflYNUYBwgN77645119 = 95533730;    int cmhIjflYNUYBwgN27879505 = -685073327;    int cmhIjflYNUYBwgN27656717 = -696762873;    int cmhIjflYNUYBwgN82424686 = -163113679;    int cmhIjflYNUYBwgN99791789 = -962279704;    int cmhIjflYNUYBwgN29973383 = -440390247;    int cmhIjflYNUYBwgN97800853 = -982313956;    int cmhIjflYNUYBwgN20107957 = -603403073;    int cmhIjflYNUYBwgN84664832 = -855879261;    int cmhIjflYNUYBwgN53864783 = -702473802;    int cmhIjflYNUYBwgN68738098 = -482336976;    int cmhIjflYNUYBwgN17168486 = -353988124;    int cmhIjflYNUYBwgN58252692 = -572125467;    int cmhIjflYNUYBwgN85025510 = -860072379;    int cmhIjflYNUYBwgN11091842 = -458998219;    int cmhIjflYNUYBwgN44329525 = -701380533;    int cmhIjflYNUYBwgN68869761 = -583213643;    int cmhIjflYNUYBwgN74591546 = -116104812;    int cmhIjflYNUYBwgN80927379 = -487584864;    int cmhIjflYNUYBwgN63365004 = 25761694;    int cmhIjflYNUYBwgN17982550 = -260253489;    int cmhIjflYNUYBwgN65869122 = -61337037;    int cmhIjflYNUYBwgN99965366 = -215060395;    int cmhIjflYNUYBwgN46036775 = -168609982;    int cmhIjflYNUYBwgN38035614 = -691828436;    int cmhIjflYNUYBwgN96398345 = -193112790;    int cmhIjflYNUYBwgN77404191 = -890839303;    int cmhIjflYNUYBwgN70794204 = -610070385;    int cmhIjflYNUYBwgN80607224 = -850619373;    int cmhIjflYNUYBwgN78078723 = -831174181;    int cmhIjflYNUYBwgN7772334 = -203430059;    int cmhIjflYNUYBwgN4255902 = -228367257;    int cmhIjflYNUYBwgN10395227 = -70514788;    int cmhIjflYNUYBwgN44695508 = -957210703;    int cmhIjflYNUYBwgN60610298 = -647181554;    int cmhIjflYNUYBwgN5495192 = -718600572;    int cmhIjflYNUYBwgN26298043 = 76404515;    int cmhIjflYNUYBwgN1667568 = -856599545;    int cmhIjflYNUYBwgN21268422 = -387107431;    int cmhIjflYNUYBwgN9958258 = -899577693;    int cmhIjflYNUYBwgN87511925 = -436430792;    int cmhIjflYNUYBwgN42533916 = -327039282;    int cmhIjflYNUYBwgN1153401 = -908277854;    int cmhIjflYNUYBwgN30779580 = -833336776;    int cmhIjflYNUYBwgN3314518 = -980485712;    int cmhIjflYNUYBwgN6367235 = -792620290;    int cmhIjflYNUYBwgN42186882 = -936415434;    int cmhIjflYNUYBwgN51982223 = -509864181;    int cmhIjflYNUYBwgN81677224 = -244377026;    int cmhIjflYNUYBwgN21460601 = -667339748;    int cmhIjflYNUYBwgN18075809 = -751228311;    int cmhIjflYNUYBwgN54930947 = -750530164;    int cmhIjflYNUYBwgN7023389 = -472174935;    int cmhIjflYNUYBwgN88954964 = 5796738;    int cmhIjflYNUYBwgN2496470 = -212590977;    int cmhIjflYNUYBwgN11775998 = -843129234;    int cmhIjflYNUYBwgN27914139 = -370012933;    int cmhIjflYNUYBwgN81619942 = -428152892;    int cmhIjflYNUYBwgN44389073 = -471285244;    int cmhIjflYNUYBwgN3393444 = -669166915;    int cmhIjflYNUYBwgN52569192 = -549550944;    int cmhIjflYNUYBwgN27006649 = -272243572;    int cmhIjflYNUYBwgN39500732 = -752783701;    int cmhIjflYNUYBwgN6586109 = 75294919;    int cmhIjflYNUYBwgN46092450 = -399043744;    int cmhIjflYNUYBwgN64482196 = -153969720;    int cmhIjflYNUYBwgN6773259 = -183473337;    int cmhIjflYNUYBwgN13557185 = -614914764;    int cmhIjflYNUYBwgN24415212 = -112890825;    int cmhIjflYNUYBwgN5596651 = -740397648;    int cmhIjflYNUYBwgN18031483 = -677785048;    int cmhIjflYNUYBwgN67202194 = -726614099;    int cmhIjflYNUYBwgN53323125 = -219339654;     cmhIjflYNUYBwgN46397351 = cmhIjflYNUYBwgN65024293;     cmhIjflYNUYBwgN65024293 = cmhIjflYNUYBwgN37294827;     cmhIjflYNUYBwgN37294827 = cmhIjflYNUYBwgN92206670;     cmhIjflYNUYBwgN92206670 = cmhIjflYNUYBwgN62715195;     cmhIjflYNUYBwgN62715195 = cmhIjflYNUYBwgN65174527;     cmhIjflYNUYBwgN65174527 = cmhIjflYNUYBwgN38255418;     cmhIjflYNUYBwgN38255418 = cmhIjflYNUYBwgN33374696;     cmhIjflYNUYBwgN33374696 = cmhIjflYNUYBwgN53954759;     cmhIjflYNUYBwgN53954759 = cmhIjflYNUYBwgN84092254;     cmhIjflYNUYBwgN84092254 = cmhIjflYNUYBwgN21060212;     cmhIjflYNUYBwgN21060212 = cmhIjflYNUYBwgN39931640;     cmhIjflYNUYBwgN39931640 = cmhIjflYNUYBwgN85312778;     cmhIjflYNUYBwgN85312778 = cmhIjflYNUYBwgN62641873;     cmhIjflYNUYBwgN62641873 = cmhIjflYNUYBwgN85818232;     cmhIjflYNUYBwgN85818232 = cmhIjflYNUYBwgN84644363;     cmhIjflYNUYBwgN84644363 = cmhIjflYNUYBwgN72052615;     cmhIjflYNUYBwgN72052615 = cmhIjflYNUYBwgN23535720;     cmhIjflYNUYBwgN23535720 = cmhIjflYNUYBwgN439575;     cmhIjflYNUYBwgN439575 = cmhIjflYNUYBwgN37007734;     cmhIjflYNUYBwgN37007734 = cmhIjflYNUYBwgN92769066;     cmhIjflYNUYBwgN92769066 = cmhIjflYNUYBwgN65790126;     cmhIjflYNUYBwgN65790126 = cmhIjflYNUYBwgN86945570;     cmhIjflYNUYBwgN86945570 = cmhIjflYNUYBwgN29522493;     cmhIjflYNUYBwgN29522493 = cmhIjflYNUYBwgN87950768;     cmhIjflYNUYBwgN87950768 = cmhIjflYNUYBwgN52319968;     cmhIjflYNUYBwgN52319968 = cmhIjflYNUYBwgN20479019;     cmhIjflYNUYBwgN20479019 = cmhIjflYNUYBwgN77645119;     cmhIjflYNUYBwgN77645119 = cmhIjflYNUYBwgN27879505;     cmhIjflYNUYBwgN27879505 = cmhIjflYNUYBwgN27656717;     cmhIjflYNUYBwgN27656717 = cmhIjflYNUYBwgN82424686;     cmhIjflYNUYBwgN82424686 = cmhIjflYNUYBwgN99791789;     cmhIjflYNUYBwgN99791789 = cmhIjflYNUYBwgN29973383;     cmhIjflYNUYBwgN29973383 = cmhIjflYNUYBwgN97800853;     cmhIjflYNUYBwgN97800853 = cmhIjflYNUYBwgN20107957;     cmhIjflYNUYBwgN20107957 = cmhIjflYNUYBwgN84664832;     cmhIjflYNUYBwgN84664832 = cmhIjflYNUYBwgN53864783;     cmhIjflYNUYBwgN53864783 = cmhIjflYNUYBwgN68738098;     cmhIjflYNUYBwgN68738098 = cmhIjflYNUYBwgN17168486;     cmhIjflYNUYBwgN17168486 = cmhIjflYNUYBwgN58252692;     cmhIjflYNUYBwgN58252692 = cmhIjflYNUYBwgN85025510;     cmhIjflYNUYBwgN85025510 = cmhIjflYNUYBwgN11091842;     cmhIjflYNUYBwgN11091842 = cmhIjflYNUYBwgN44329525;     cmhIjflYNUYBwgN44329525 = cmhIjflYNUYBwgN68869761;     cmhIjflYNUYBwgN68869761 = cmhIjflYNUYBwgN74591546;     cmhIjflYNUYBwgN74591546 = cmhIjflYNUYBwgN80927379;     cmhIjflYNUYBwgN80927379 = cmhIjflYNUYBwgN63365004;     cmhIjflYNUYBwgN63365004 = cmhIjflYNUYBwgN17982550;     cmhIjflYNUYBwgN17982550 = cmhIjflYNUYBwgN65869122;     cmhIjflYNUYBwgN65869122 = cmhIjflYNUYBwgN99965366;     cmhIjflYNUYBwgN99965366 = cmhIjflYNUYBwgN46036775;     cmhIjflYNUYBwgN46036775 = cmhIjflYNUYBwgN38035614;     cmhIjflYNUYBwgN38035614 = cmhIjflYNUYBwgN96398345;     cmhIjflYNUYBwgN96398345 = cmhIjflYNUYBwgN77404191;     cmhIjflYNUYBwgN77404191 = cmhIjflYNUYBwgN70794204;     cmhIjflYNUYBwgN70794204 = cmhIjflYNUYBwgN80607224;     cmhIjflYNUYBwgN80607224 = cmhIjflYNUYBwgN78078723;     cmhIjflYNUYBwgN78078723 = cmhIjflYNUYBwgN7772334;     cmhIjflYNUYBwgN7772334 = cmhIjflYNUYBwgN4255902;     cmhIjflYNUYBwgN4255902 = cmhIjflYNUYBwgN10395227;     cmhIjflYNUYBwgN10395227 = cmhIjflYNUYBwgN44695508;     cmhIjflYNUYBwgN44695508 = cmhIjflYNUYBwgN60610298;     cmhIjflYNUYBwgN60610298 = cmhIjflYNUYBwgN5495192;     cmhIjflYNUYBwgN5495192 = cmhIjflYNUYBwgN26298043;     cmhIjflYNUYBwgN26298043 = cmhIjflYNUYBwgN1667568;     cmhIjflYNUYBwgN1667568 = cmhIjflYNUYBwgN21268422;     cmhIjflYNUYBwgN21268422 = cmhIjflYNUYBwgN9958258;     cmhIjflYNUYBwgN9958258 = cmhIjflYNUYBwgN87511925;     cmhIjflYNUYBwgN87511925 = cmhIjflYNUYBwgN42533916;     cmhIjflYNUYBwgN42533916 = cmhIjflYNUYBwgN1153401;     cmhIjflYNUYBwgN1153401 = cmhIjflYNUYBwgN30779580;     cmhIjflYNUYBwgN30779580 = cmhIjflYNUYBwgN3314518;     cmhIjflYNUYBwgN3314518 = cmhIjflYNUYBwgN6367235;     cmhIjflYNUYBwgN6367235 = cmhIjflYNUYBwgN42186882;     cmhIjflYNUYBwgN42186882 = cmhIjflYNUYBwgN51982223;     cmhIjflYNUYBwgN51982223 = cmhIjflYNUYBwgN81677224;     cmhIjflYNUYBwgN81677224 = cmhIjflYNUYBwgN21460601;     cmhIjflYNUYBwgN21460601 = cmhIjflYNUYBwgN18075809;     cmhIjflYNUYBwgN18075809 = cmhIjflYNUYBwgN54930947;     cmhIjflYNUYBwgN54930947 = cmhIjflYNUYBwgN7023389;     cmhIjflYNUYBwgN7023389 = cmhIjflYNUYBwgN88954964;     cmhIjflYNUYBwgN88954964 = cmhIjflYNUYBwgN2496470;     cmhIjflYNUYBwgN2496470 = cmhIjflYNUYBwgN11775998;     cmhIjflYNUYBwgN11775998 = cmhIjflYNUYBwgN27914139;     cmhIjflYNUYBwgN27914139 = cmhIjflYNUYBwgN81619942;     cmhIjflYNUYBwgN81619942 = cmhIjflYNUYBwgN44389073;     cmhIjflYNUYBwgN44389073 = cmhIjflYNUYBwgN3393444;     cmhIjflYNUYBwgN3393444 = cmhIjflYNUYBwgN52569192;     cmhIjflYNUYBwgN52569192 = cmhIjflYNUYBwgN27006649;     cmhIjflYNUYBwgN27006649 = cmhIjflYNUYBwgN39500732;     cmhIjflYNUYBwgN39500732 = cmhIjflYNUYBwgN6586109;     cmhIjflYNUYBwgN6586109 = cmhIjflYNUYBwgN46092450;     cmhIjflYNUYBwgN46092450 = cmhIjflYNUYBwgN64482196;     cmhIjflYNUYBwgN64482196 = cmhIjflYNUYBwgN6773259;     cmhIjflYNUYBwgN6773259 = cmhIjflYNUYBwgN13557185;     cmhIjflYNUYBwgN13557185 = cmhIjflYNUYBwgN24415212;     cmhIjflYNUYBwgN24415212 = cmhIjflYNUYBwgN5596651;     cmhIjflYNUYBwgN5596651 = cmhIjflYNUYBwgN18031483;     cmhIjflYNUYBwgN18031483 = cmhIjflYNUYBwgN67202194;     cmhIjflYNUYBwgN67202194 = cmhIjflYNUYBwgN53323125;     cmhIjflYNUYBwgN53323125 = cmhIjflYNUYBwgN46397351;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ORONEoisUbEKzYSSOuElRdqlhILDD65752610() {     int DyawWKTtpdLlsMc42910271 = -218140342;    int DyawWKTtpdLlsMc80107118 = -118304436;    int DyawWKTtpdLlsMc92977137 = 81832201;    int DyawWKTtpdLlsMc4755519 = -756940845;    int DyawWKTtpdLlsMc31367585 = -894543350;    int DyawWKTtpdLlsMc2421400 = -560582745;    int DyawWKTtpdLlsMc12070072 = -816565799;    int DyawWKTtpdLlsMc68826866 = -516558083;    int DyawWKTtpdLlsMc58209016 = -972776112;    int DyawWKTtpdLlsMc38096790 = -778438436;    int DyawWKTtpdLlsMc81276327 = -324622372;    int DyawWKTtpdLlsMc54948077 = -908700845;    int DyawWKTtpdLlsMc90583476 = -704530411;    int DyawWKTtpdLlsMc29716986 = -439963890;    int DyawWKTtpdLlsMc90717989 = -615483825;    int DyawWKTtpdLlsMc97601187 = -294431793;    int DyawWKTtpdLlsMc99743464 = -375669733;    int DyawWKTtpdLlsMc59671208 = -764134479;    int DyawWKTtpdLlsMc82362550 = -997668201;    int DyawWKTtpdLlsMc77732993 = -279461390;    int DyawWKTtpdLlsMc447922 = 86075157;    int DyawWKTtpdLlsMc6691884 = -746737974;    int DyawWKTtpdLlsMc76442532 = -615361664;    int DyawWKTtpdLlsMc53347264 = -925752380;    int DyawWKTtpdLlsMc66225775 = -585185558;    int DyawWKTtpdLlsMc28344333 = -247736830;    int DyawWKTtpdLlsMc68375704 = -400026313;    int DyawWKTtpdLlsMc69895482 = -660222369;    int DyawWKTtpdLlsMc10075476 = -550451310;    int DyawWKTtpdLlsMc27911225 = -61539716;    int DyawWKTtpdLlsMc91518766 = -465081833;    int DyawWKTtpdLlsMc35541973 = -194285724;    int DyawWKTtpdLlsMc47417822 = -330487015;    int DyawWKTtpdLlsMc10460522 = -802823956;    int DyawWKTtpdLlsMc29592796 = -818068599;    int DyawWKTtpdLlsMc28336086 = -32955970;    int DyawWKTtpdLlsMc34421750 = -212897760;    int DyawWKTtpdLlsMc41865072 = -303468945;    int DyawWKTtpdLlsMc65902792 = -695056874;    int DyawWKTtpdLlsMc37770092 = -8473349;    int DyawWKTtpdLlsMc17296506 = -291614549;    int DyawWKTtpdLlsMc55285194 = -966439182;    int DyawWKTtpdLlsMc31871461 = -97653105;    int DyawWKTtpdLlsMc81077869 = -724847397;    int DyawWKTtpdLlsMc40578261 = -648259150;    int DyawWKTtpdLlsMc78608406 = -819912660;    int DyawWKTtpdLlsMc60277097 = -252758954;    int DyawWKTtpdLlsMc98472572 = -423347488;    int DyawWKTtpdLlsMc84478190 = -692841232;    int DyawWKTtpdLlsMc72766405 = -564599850;    int DyawWKTtpdLlsMc97825415 = -797989188;    int DyawWKTtpdLlsMc10444253 = -173993686;    int DyawWKTtpdLlsMc88541724 = -985021434;    int DyawWKTtpdLlsMc62911387 = -241040307;    int DyawWKTtpdLlsMc65304688 = 57207823;    int DyawWKTtpdLlsMc36218388 = -471402369;    int DyawWKTtpdLlsMc3664586 = -502942773;    int DyawWKTtpdLlsMc39629874 = 7584580;    int DyawWKTtpdLlsMc38529743 = -71755288;    int DyawWKTtpdLlsMc3023252 = -546806521;    int DyawWKTtpdLlsMc34045696 = -60556433;    int DyawWKTtpdLlsMc42174589 = -56343430;    int DyawWKTtpdLlsMc58751391 = -966106773;    int DyawWKTtpdLlsMc30297792 = -811236396;    int DyawWKTtpdLlsMc46578024 = -213356604;    int DyawWKTtpdLlsMc45734354 = -30336648;    int DyawWKTtpdLlsMc7530256 = -478213831;    int DyawWKTtpdLlsMc80122955 = -901706455;    int DyawWKTtpdLlsMc124191 = -621895291;    int DyawWKTtpdLlsMc62381903 = -482527856;    int DyawWKTtpdLlsMc63179437 = 18465967;    int DyawWKTtpdLlsMc57878392 = 27799212;    int DyawWKTtpdLlsMc93768416 = 30922395;    int DyawWKTtpdLlsMc44592459 = -889194853;    int DyawWKTtpdLlsMc60436488 = -987846841;    int DyawWKTtpdLlsMc45162728 = 52514338;    int DyawWKTtpdLlsMc74820422 = -549084869;    int DyawWKTtpdLlsMc95364662 = -890514268;    int DyawWKTtpdLlsMc12769003 = -177493230;    int DyawWKTtpdLlsMc87617369 = -765272899;    int DyawWKTtpdLlsMc68067236 = -994977877;    int DyawWKTtpdLlsMc69903131 = -976678825;    int DyawWKTtpdLlsMc85417292 = -967381138;    int DyawWKTtpdLlsMc37309071 = -985851461;    int DyawWKTtpdLlsMc30085810 = -263550528;    int DyawWKTtpdLlsMc81074514 = -191088147;    int DyawWKTtpdLlsMc47000249 = -209264291;    int DyawWKTtpdLlsMc84506435 = 10553292;    int DyawWKTtpdLlsMc45155833 = -760031779;    int DyawWKTtpdLlsMc93374407 = -246666231;    int DyawWKTtpdLlsMc24671500 = -530013197;    int DyawWKTtpdLlsMc94791876 = -120482341;    int DyawWKTtpdLlsMc3335330 = -131713658;    int DyawWKTtpdLlsMc62879540 = -48250354;    int DyawWKTtpdLlsMc3724397 = -947916917;    int DyawWKTtpdLlsMc75121916 = -135271119;    int DyawWKTtpdLlsMc96533803 = 99667591;    int DyawWKTtpdLlsMc1573670 = -286416709;    int DyawWKTtpdLlsMc34499846 = -411490793;    int DyawWKTtpdLlsMc94843907 = -218140342;     DyawWKTtpdLlsMc42910271 = DyawWKTtpdLlsMc80107118;     DyawWKTtpdLlsMc80107118 = DyawWKTtpdLlsMc92977137;     DyawWKTtpdLlsMc92977137 = DyawWKTtpdLlsMc4755519;     DyawWKTtpdLlsMc4755519 = DyawWKTtpdLlsMc31367585;     DyawWKTtpdLlsMc31367585 = DyawWKTtpdLlsMc2421400;     DyawWKTtpdLlsMc2421400 = DyawWKTtpdLlsMc12070072;     DyawWKTtpdLlsMc12070072 = DyawWKTtpdLlsMc68826866;     DyawWKTtpdLlsMc68826866 = DyawWKTtpdLlsMc58209016;     DyawWKTtpdLlsMc58209016 = DyawWKTtpdLlsMc38096790;     DyawWKTtpdLlsMc38096790 = DyawWKTtpdLlsMc81276327;     DyawWKTtpdLlsMc81276327 = DyawWKTtpdLlsMc54948077;     DyawWKTtpdLlsMc54948077 = DyawWKTtpdLlsMc90583476;     DyawWKTtpdLlsMc90583476 = DyawWKTtpdLlsMc29716986;     DyawWKTtpdLlsMc29716986 = DyawWKTtpdLlsMc90717989;     DyawWKTtpdLlsMc90717989 = DyawWKTtpdLlsMc97601187;     DyawWKTtpdLlsMc97601187 = DyawWKTtpdLlsMc99743464;     DyawWKTtpdLlsMc99743464 = DyawWKTtpdLlsMc59671208;     DyawWKTtpdLlsMc59671208 = DyawWKTtpdLlsMc82362550;     DyawWKTtpdLlsMc82362550 = DyawWKTtpdLlsMc77732993;     DyawWKTtpdLlsMc77732993 = DyawWKTtpdLlsMc447922;     DyawWKTtpdLlsMc447922 = DyawWKTtpdLlsMc6691884;     DyawWKTtpdLlsMc6691884 = DyawWKTtpdLlsMc76442532;     DyawWKTtpdLlsMc76442532 = DyawWKTtpdLlsMc53347264;     DyawWKTtpdLlsMc53347264 = DyawWKTtpdLlsMc66225775;     DyawWKTtpdLlsMc66225775 = DyawWKTtpdLlsMc28344333;     DyawWKTtpdLlsMc28344333 = DyawWKTtpdLlsMc68375704;     DyawWKTtpdLlsMc68375704 = DyawWKTtpdLlsMc69895482;     DyawWKTtpdLlsMc69895482 = DyawWKTtpdLlsMc10075476;     DyawWKTtpdLlsMc10075476 = DyawWKTtpdLlsMc27911225;     DyawWKTtpdLlsMc27911225 = DyawWKTtpdLlsMc91518766;     DyawWKTtpdLlsMc91518766 = DyawWKTtpdLlsMc35541973;     DyawWKTtpdLlsMc35541973 = DyawWKTtpdLlsMc47417822;     DyawWKTtpdLlsMc47417822 = DyawWKTtpdLlsMc10460522;     DyawWKTtpdLlsMc10460522 = DyawWKTtpdLlsMc29592796;     DyawWKTtpdLlsMc29592796 = DyawWKTtpdLlsMc28336086;     DyawWKTtpdLlsMc28336086 = DyawWKTtpdLlsMc34421750;     DyawWKTtpdLlsMc34421750 = DyawWKTtpdLlsMc41865072;     DyawWKTtpdLlsMc41865072 = DyawWKTtpdLlsMc65902792;     DyawWKTtpdLlsMc65902792 = DyawWKTtpdLlsMc37770092;     DyawWKTtpdLlsMc37770092 = DyawWKTtpdLlsMc17296506;     DyawWKTtpdLlsMc17296506 = DyawWKTtpdLlsMc55285194;     DyawWKTtpdLlsMc55285194 = DyawWKTtpdLlsMc31871461;     DyawWKTtpdLlsMc31871461 = DyawWKTtpdLlsMc81077869;     DyawWKTtpdLlsMc81077869 = DyawWKTtpdLlsMc40578261;     DyawWKTtpdLlsMc40578261 = DyawWKTtpdLlsMc78608406;     DyawWKTtpdLlsMc78608406 = DyawWKTtpdLlsMc60277097;     DyawWKTtpdLlsMc60277097 = DyawWKTtpdLlsMc98472572;     DyawWKTtpdLlsMc98472572 = DyawWKTtpdLlsMc84478190;     DyawWKTtpdLlsMc84478190 = DyawWKTtpdLlsMc72766405;     DyawWKTtpdLlsMc72766405 = DyawWKTtpdLlsMc97825415;     DyawWKTtpdLlsMc97825415 = DyawWKTtpdLlsMc10444253;     DyawWKTtpdLlsMc10444253 = DyawWKTtpdLlsMc88541724;     DyawWKTtpdLlsMc88541724 = DyawWKTtpdLlsMc62911387;     DyawWKTtpdLlsMc62911387 = DyawWKTtpdLlsMc65304688;     DyawWKTtpdLlsMc65304688 = DyawWKTtpdLlsMc36218388;     DyawWKTtpdLlsMc36218388 = DyawWKTtpdLlsMc3664586;     DyawWKTtpdLlsMc3664586 = DyawWKTtpdLlsMc39629874;     DyawWKTtpdLlsMc39629874 = DyawWKTtpdLlsMc38529743;     DyawWKTtpdLlsMc38529743 = DyawWKTtpdLlsMc3023252;     DyawWKTtpdLlsMc3023252 = DyawWKTtpdLlsMc34045696;     DyawWKTtpdLlsMc34045696 = DyawWKTtpdLlsMc42174589;     DyawWKTtpdLlsMc42174589 = DyawWKTtpdLlsMc58751391;     DyawWKTtpdLlsMc58751391 = DyawWKTtpdLlsMc30297792;     DyawWKTtpdLlsMc30297792 = DyawWKTtpdLlsMc46578024;     DyawWKTtpdLlsMc46578024 = DyawWKTtpdLlsMc45734354;     DyawWKTtpdLlsMc45734354 = DyawWKTtpdLlsMc7530256;     DyawWKTtpdLlsMc7530256 = DyawWKTtpdLlsMc80122955;     DyawWKTtpdLlsMc80122955 = DyawWKTtpdLlsMc124191;     DyawWKTtpdLlsMc124191 = DyawWKTtpdLlsMc62381903;     DyawWKTtpdLlsMc62381903 = DyawWKTtpdLlsMc63179437;     DyawWKTtpdLlsMc63179437 = DyawWKTtpdLlsMc57878392;     DyawWKTtpdLlsMc57878392 = DyawWKTtpdLlsMc93768416;     DyawWKTtpdLlsMc93768416 = DyawWKTtpdLlsMc44592459;     DyawWKTtpdLlsMc44592459 = DyawWKTtpdLlsMc60436488;     DyawWKTtpdLlsMc60436488 = DyawWKTtpdLlsMc45162728;     DyawWKTtpdLlsMc45162728 = DyawWKTtpdLlsMc74820422;     DyawWKTtpdLlsMc74820422 = DyawWKTtpdLlsMc95364662;     DyawWKTtpdLlsMc95364662 = DyawWKTtpdLlsMc12769003;     DyawWKTtpdLlsMc12769003 = DyawWKTtpdLlsMc87617369;     DyawWKTtpdLlsMc87617369 = DyawWKTtpdLlsMc68067236;     DyawWKTtpdLlsMc68067236 = DyawWKTtpdLlsMc69903131;     DyawWKTtpdLlsMc69903131 = DyawWKTtpdLlsMc85417292;     DyawWKTtpdLlsMc85417292 = DyawWKTtpdLlsMc37309071;     DyawWKTtpdLlsMc37309071 = DyawWKTtpdLlsMc30085810;     DyawWKTtpdLlsMc30085810 = DyawWKTtpdLlsMc81074514;     DyawWKTtpdLlsMc81074514 = DyawWKTtpdLlsMc47000249;     DyawWKTtpdLlsMc47000249 = DyawWKTtpdLlsMc84506435;     DyawWKTtpdLlsMc84506435 = DyawWKTtpdLlsMc45155833;     DyawWKTtpdLlsMc45155833 = DyawWKTtpdLlsMc93374407;     DyawWKTtpdLlsMc93374407 = DyawWKTtpdLlsMc24671500;     DyawWKTtpdLlsMc24671500 = DyawWKTtpdLlsMc94791876;     DyawWKTtpdLlsMc94791876 = DyawWKTtpdLlsMc3335330;     DyawWKTtpdLlsMc3335330 = DyawWKTtpdLlsMc62879540;     DyawWKTtpdLlsMc62879540 = DyawWKTtpdLlsMc3724397;     DyawWKTtpdLlsMc3724397 = DyawWKTtpdLlsMc75121916;     DyawWKTtpdLlsMc75121916 = DyawWKTtpdLlsMc96533803;     DyawWKTtpdLlsMc96533803 = DyawWKTtpdLlsMc1573670;     DyawWKTtpdLlsMc1573670 = DyawWKTtpdLlsMc34499846;     DyawWKTtpdLlsMc34499846 = DyawWKTtpdLlsMc94843907;     DyawWKTtpdLlsMc94843907 = DyawWKTtpdLlsMc42910271;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void SfUBcaqkdTDfhTatpioWfuRSJXvXo67941948() {     int StjjGJKzfjJbaTv39423192 = -216941030;    int StjjGJKzfjJbaTv95189943 = -70992739;    int StjjGJKzfjJbaTv48659449 = -766270565;    int StjjGJKzfjJbaTv17304367 = -125754636;    int StjjGJKzfjJbaTv19975 = -450130346;    int StjjGJKzfjJbaTv39668271 = -591110322;    int StjjGJKzfjJbaTv85884724 = -981483774;    int StjjGJKzfjJbaTv4279037 = -629442267;    int StjjGJKzfjJbaTv62463273 = -125193865;    int StjjGJKzfjJbaTv92101326 = -437163649;    int StjjGJKzfjJbaTv41492443 = -299857609;    int StjjGJKzfjJbaTv69964514 = -377433751;    int StjjGJKzfjJbaTv95854174 = -990316074;    int StjjGJKzfjJbaTv96792099 = -949485425;    int StjjGJKzfjJbaTv95617746 = -466810535;    int StjjGJKzfjJbaTv10558011 = -53053008;    int StjjGJKzfjJbaTv27434313 = -288516778;    int StjjGJKzfjJbaTv95806696 = -281660544;    int StjjGJKzfjJbaTv64285527 = -386795501;    int StjjGJKzfjJbaTv18458253 = -188986220;    int StjjGJKzfjJbaTv8126777 = -124474441;    int StjjGJKzfjJbaTv47593641 = -24755666;    int StjjGJKzfjJbaTv65939495 = -896281374;    int StjjGJKzfjJbaTv77172034 = -884869784;    int StjjGJKzfjJbaTv44500783 = -110611317;    int StjjGJKzfjJbaTv4368698 = -427032093;    int StjjGJKzfjJbaTv16272389 = -227208160;    int StjjGJKzfjJbaTv62145845 = -315978468;    int StjjGJKzfjJbaTv92271447 = -415829293;    int StjjGJKzfjJbaTv28165733 = -526316558;    int StjjGJKzfjJbaTv612846 = -767049986;    int StjjGJKzfjJbaTv71292156 = -526291745;    int StjjGJKzfjJbaTv64862260 = -220583783;    int StjjGJKzfjJbaTv23120190 = -623333956;    int StjjGJKzfjJbaTv39077634 = 67265875;    int StjjGJKzfjJbaTv72007339 = -310032678;    int StjjGJKzfjJbaTv14978717 = -823321718;    int StjjGJKzfjJbaTv14992047 = -124600914;    int StjjGJKzfjJbaTv14637099 = 63874375;    int StjjGJKzfjJbaTv17287491 = -544821231;    int StjjGJKzfjJbaTv49567500 = -823156720;    int StjjGJKzfjJbaTv99478546 = -373880144;    int StjjGJKzfjJbaTv19413397 = -593925677;    int StjjGJKzfjJbaTv93285977 = -866481150;    int StjjGJKzfjJbaTv6564977 = -80413488;    int StjjGJKzfjJbaTv76289433 = -52240455;    int StjjGJKzfjJbaTv57189190 = -531279602;    int StjjGJKzfjJbaTv78962596 = -586441487;    int StjjGJKzfjJbaTv3087259 = -224345427;    int StjjGJKzfjJbaTv45567444 = -914139305;    int StjjGJKzfjJbaTv49614055 = -327368395;    int StjjGJKzfjJbaTv82852890 = -756158936;    int StjjGJKzfjJbaTv80685102 = -676930078;    int StjjGJKzfjJbaTv48418582 = -691241311;    int StjjGJKzfjJbaTv59815171 = -375513970;    int StjjGJKzfjJbaTv91829551 = -92185364;    int StjjGJKzfjJbaTv29250449 = -174711365;    int StjjGJKzfjJbaTv71487414 = -881400781;    int StjjGJKzfjJbaTv72803584 = 84856681;    int StjjGJKzfjJbaTv95651277 = 76901746;    int StjjGJKzfjJbaTv23395883 = -263902162;    int StjjGJKzfjJbaTv23738880 = -565505306;    int StjjGJKzfjJbaTv12007590 = -113612975;    int StjjGJKzfjJbaTv34297541 = -598877308;    int StjjGJKzfjJbaTv91488480 = -670113664;    int StjjGJKzfjJbaTv70200286 = -773565865;    int StjjGJKzfjJbaTv5102254 = -56849968;    int StjjGJKzfjJbaTv72733984 = -266982119;    int StjjGJKzfjJbaTv57714465 = -916751300;    int StjjGJKzfjJbaTv23610407 = -56777858;    int StjjGJKzfjJbaTv95579293 = -229731291;    int StjjGJKzfjJbaTv12442267 = -63915864;    int StjjGJKzfjJbaTv81169597 = -245534920;    int StjjGJKzfjJbaTv46998036 = -841974271;    int StjjGJKzfjJbaTv68890752 = -365829501;    int StjjGJKzfjJbaTv8648231 = -750594298;    int StjjGJKzfjJbaTv28180244 = -430829990;    int StjjGJKzfjJbaTv72653517 = 70199775;    int StjjGJKzfjJbaTv70607058 = -704456297;    int StjjGJKzfjJbaTv68211350 = 41629138;    int StjjGJKzfjJbaTv47179508 = -895752491;    int StjjGJKzfjJbaTv37309793 = -640766673;    int StjjGJKzfjJbaTv59058586 = 8366958;    int StjjGJKzfjJbaTv46704003 = -501689989;    int StjjGJKzfjJbaTv78551677 = -98948164;    int StjjGJKzfjJbaTv17759956 = 89108949;    int StjjGJKzfjJbaTv90607053 = -849361667;    int StjjGJKzfjJbaTv16443678 = -529342472;    int StjjGJKzfjJbaTv63305018 = -147819987;    int StjjGJKzfjJbaTv47248082 = -840548761;    int StjjGJKzfjJbaTv42756891 = -35321314;    int StjjGJKzfjJbaTv43491302 = -941920937;    int StjjGJKzfjJbaTv42188463 = -109457595;    int StjjGJKzfjJbaTv18985822 = 86972629;    int StjjGJKzfjJbaTv93891608 = -180919069;    int StjjGJKzfjJbaTv25828621 = -157651414;    int StjjGJKzfjJbaTv87470956 = -160267170;    int StjjGJKzfjJbaTv85115856 = -995048370;    int StjjGJKzfjJbaTv1797498 = -96367487;    int StjjGJKzfjJbaTv36364690 = -216941030;     StjjGJKzfjJbaTv39423192 = StjjGJKzfjJbaTv95189943;     StjjGJKzfjJbaTv95189943 = StjjGJKzfjJbaTv48659449;     StjjGJKzfjJbaTv48659449 = StjjGJKzfjJbaTv17304367;     StjjGJKzfjJbaTv17304367 = StjjGJKzfjJbaTv19975;     StjjGJKzfjJbaTv19975 = StjjGJKzfjJbaTv39668271;     StjjGJKzfjJbaTv39668271 = StjjGJKzfjJbaTv85884724;     StjjGJKzfjJbaTv85884724 = StjjGJKzfjJbaTv4279037;     StjjGJKzfjJbaTv4279037 = StjjGJKzfjJbaTv62463273;     StjjGJKzfjJbaTv62463273 = StjjGJKzfjJbaTv92101326;     StjjGJKzfjJbaTv92101326 = StjjGJKzfjJbaTv41492443;     StjjGJKzfjJbaTv41492443 = StjjGJKzfjJbaTv69964514;     StjjGJKzfjJbaTv69964514 = StjjGJKzfjJbaTv95854174;     StjjGJKzfjJbaTv95854174 = StjjGJKzfjJbaTv96792099;     StjjGJKzfjJbaTv96792099 = StjjGJKzfjJbaTv95617746;     StjjGJKzfjJbaTv95617746 = StjjGJKzfjJbaTv10558011;     StjjGJKzfjJbaTv10558011 = StjjGJKzfjJbaTv27434313;     StjjGJKzfjJbaTv27434313 = StjjGJKzfjJbaTv95806696;     StjjGJKzfjJbaTv95806696 = StjjGJKzfjJbaTv64285527;     StjjGJKzfjJbaTv64285527 = StjjGJKzfjJbaTv18458253;     StjjGJKzfjJbaTv18458253 = StjjGJKzfjJbaTv8126777;     StjjGJKzfjJbaTv8126777 = StjjGJKzfjJbaTv47593641;     StjjGJKzfjJbaTv47593641 = StjjGJKzfjJbaTv65939495;     StjjGJKzfjJbaTv65939495 = StjjGJKzfjJbaTv77172034;     StjjGJKzfjJbaTv77172034 = StjjGJKzfjJbaTv44500783;     StjjGJKzfjJbaTv44500783 = StjjGJKzfjJbaTv4368698;     StjjGJKzfjJbaTv4368698 = StjjGJKzfjJbaTv16272389;     StjjGJKzfjJbaTv16272389 = StjjGJKzfjJbaTv62145845;     StjjGJKzfjJbaTv62145845 = StjjGJKzfjJbaTv92271447;     StjjGJKzfjJbaTv92271447 = StjjGJKzfjJbaTv28165733;     StjjGJKzfjJbaTv28165733 = StjjGJKzfjJbaTv612846;     StjjGJKzfjJbaTv612846 = StjjGJKzfjJbaTv71292156;     StjjGJKzfjJbaTv71292156 = StjjGJKzfjJbaTv64862260;     StjjGJKzfjJbaTv64862260 = StjjGJKzfjJbaTv23120190;     StjjGJKzfjJbaTv23120190 = StjjGJKzfjJbaTv39077634;     StjjGJKzfjJbaTv39077634 = StjjGJKzfjJbaTv72007339;     StjjGJKzfjJbaTv72007339 = StjjGJKzfjJbaTv14978717;     StjjGJKzfjJbaTv14978717 = StjjGJKzfjJbaTv14992047;     StjjGJKzfjJbaTv14992047 = StjjGJKzfjJbaTv14637099;     StjjGJKzfjJbaTv14637099 = StjjGJKzfjJbaTv17287491;     StjjGJKzfjJbaTv17287491 = StjjGJKzfjJbaTv49567500;     StjjGJKzfjJbaTv49567500 = StjjGJKzfjJbaTv99478546;     StjjGJKzfjJbaTv99478546 = StjjGJKzfjJbaTv19413397;     StjjGJKzfjJbaTv19413397 = StjjGJKzfjJbaTv93285977;     StjjGJKzfjJbaTv93285977 = StjjGJKzfjJbaTv6564977;     StjjGJKzfjJbaTv6564977 = StjjGJKzfjJbaTv76289433;     StjjGJKzfjJbaTv76289433 = StjjGJKzfjJbaTv57189190;     StjjGJKzfjJbaTv57189190 = StjjGJKzfjJbaTv78962596;     StjjGJKzfjJbaTv78962596 = StjjGJKzfjJbaTv3087259;     StjjGJKzfjJbaTv3087259 = StjjGJKzfjJbaTv45567444;     StjjGJKzfjJbaTv45567444 = StjjGJKzfjJbaTv49614055;     StjjGJKzfjJbaTv49614055 = StjjGJKzfjJbaTv82852890;     StjjGJKzfjJbaTv82852890 = StjjGJKzfjJbaTv80685102;     StjjGJKzfjJbaTv80685102 = StjjGJKzfjJbaTv48418582;     StjjGJKzfjJbaTv48418582 = StjjGJKzfjJbaTv59815171;     StjjGJKzfjJbaTv59815171 = StjjGJKzfjJbaTv91829551;     StjjGJKzfjJbaTv91829551 = StjjGJKzfjJbaTv29250449;     StjjGJKzfjJbaTv29250449 = StjjGJKzfjJbaTv71487414;     StjjGJKzfjJbaTv71487414 = StjjGJKzfjJbaTv72803584;     StjjGJKzfjJbaTv72803584 = StjjGJKzfjJbaTv95651277;     StjjGJKzfjJbaTv95651277 = StjjGJKzfjJbaTv23395883;     StjjGJKzfjJbaTv23395883 = StjjGJKzfjJbaTv23738880;     StjjGJKzfjJbaTv23738880 = StjjGJKzfjJbaTv12007590;     StjjGJKzfjJbaTv12007590 = StjjGJKzfjJbaTv34297541;     StjjGJKzfjJbaTv34297541 = StjjGJKzfjJbaTv91488480;     StjjGJKzfjJbaTv91488480 = StjjGJKzfjJbaTv70200286;     StjjGJKzfjJbaTv70200286 = StjjGJKzfjJbaTv5102254;     StjjGJKzfjJbaTv5102254 = StjjGJKzfjJbaTv72733984;     StjjGJKzfjJbaTv72733984 = StjjGJKzfjJbaTv57714465;     StjjGJKzfjJbaTv57714465 = StjjGJKzfjJbaTv23610407;     StjjGJKzfjJbaTv23610407 = StjjGJKzfjJbaTv95579293;     StjjGJKzfjJbaTv95579293 = StjjGJKzfjJbaTv12442267;     StjjGJKzfjJbaTv12442267 = StjjGJKzfjJbaTv81169597;     StjjGJKzfjJbaTv81169597 = StjjGJKzfjJbaTv46998036;     StjjGJKzfjJbaTv46998036 = StjjGJKzfjJbaTv68890752;     StjjGJKzfjJbaTv68890752 = StjjGJKzfjJbaTv8648231;     StjjGJKzfjJbaTv8648231 = StjjGJKzfjJbaTv28180244;     StjjGJKzfjJbaTv28180244 = StjjGJKzfjJbaTv72653517;     StjjGJKzfjJbaTv72653517 = StjjGJKzfjJbaTv70607058;     StjjGJKzfjJbaTv70607058 = StjjGJKzfjJbaTv68211350;     StjjGJKzfjJbaTv68211350 = StjjGJKzfjJbaTv47179508;     StjjGJKzfjJbaTv47179508 = StjjGJKzfjJbaTv37309793;     StjjGJKzfjJbaTv37309793 = StjjGJKzfjJbaTv59058586;     StjjGJKzfjJbaTv59058586 = StjjGJKzfjJbaTv46704003;     StjjGJKzfjJbaTv46704003 = StjjGJKzfjJbaTv78551677;     StjjGJKzfjJbaTv78551677 = StjjGJKzfjJbaTv17759956;     StjjGJKzfjJbaTv17759956 = StjjGJKzfjJbaTv90607053;     StjjGJKzfjJbaTv90607053 = StjjGJKzfjJbaTv16443678;     StjjGJKzfjJbaTv16443678 = StjjGJKzfjJbaTv63305018;     StjjGJKzfjJbaTv63305018 = StjjGJKzfjJbaTv47248082;     StjjGJKzfjJbaTv47248082 = StjjGJKzfjJbaTv42756891;     StjjGJKzfjJbaTv42756891 = StjjGJKzfjJbaTv43491302;     StjjGJKzfjJbaTv43491302 = StjjGJKzfjJbaTv42188463;     StjjGJKzfjJbaTv42188463 = StjjGJKzfjJbaTv18985822;     StjjGJKzfjJbaTv18985822 = StjjGJKzfjJbaTv93891608;     StjjGJKzfjJbaTv93891608 = StjjGJKzfjJbaTv25828621;     StjjGJKzfjJbaTv25828621 = StjjGJKzfjJbaTv87470956;     StjjGJKzfjJbaTv87470956 = StjjGJKzfjJbaTv85115856;     StjjGJKzfjJbaTv85115856 = StjjGJKzfjJbaTv1797498;     StjjGJKzfjJbaTv1797498 = StjjGJKzfjJbaTv36364690;     StjjGJKzfjJbaTv36364690 = StjjGJKzfjJbaTv39423192;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void QXHHfRwTtMoXxkIoLdFUwilnxRPgi70131286() {     int yDwxbjswIoYGTxz35936113 = -215741718;    int yDwxbjswIoYGTxz10272770 = -23681041;    int yDwxbjswIoYGTxz4341760 = -514373330;    int yDwxbjswIoYGTxz29853215 = -594568426;    int yDwxbjswIoYGTxz68672365 = -5717342;    int yDwxbjswIoYGTxz76915143 = -621637898;    int yDwxbjswIoYGTxz59699378 = -46401749;    int yDwxbjswIoYGTxz39731207 = -742326451;    int yDwxbjswIoYGTxz66717530 = -377611619;    int yDwxbjswIoYGTxz46105863 = -95888862;    int yDwxbjswIoYGTxz1708558 = -275092846;    int yDwxbjswIoYGTxz84980951 = -946166657;    int yDwxbjswIoYGTxz1124872 = -176101738;    int yDwxbjswIoYGTxz63867212 = -359006960;    int yDwxbjswIoYGTxz517504 = -318137245;    int yDwxbjswIoYGTxz23514835 = -911674223;    int yDwxbjswIoYGTxz55125162 = -201363822;    int yDwxbjswIoYGTxz31942185 = -899186609;    int yDwxbjswIoYGTxz46208503 = -875922802;    int yDwxbjswIoYGTxz59183511 = -98511051;    int yDwxbjswIoYGTxz15805632 = -335024040;    int yDwxbjswIoYGTxz88495398 = -402773359;    int yDwxbjswIoYGTxz55436457 = -77201085;    int yDwxbjswIoYGTxz996805 = -843987188;    int yDwxbjswIoYGTxz22775790 = -736037076;    int yDwxbjswIoYGTxz80393062 = -606327355;    int yDwxbjswIoYGTxz64169073 = -54390007;    int yDwxbjswIoYGTxz54396208 = 28265433;    int yDwxbjswIoYGTxz74467418 = -281207276;    int yDwxbjswIoYGTxz28420240 = -991093400;    int yDwxbjswIoYGTxz9706926 = 30981861;    int yDwxbjswIoYGTxz7042340 = -858297765;    int yDwxbjswIoYGTxz82306699 = -110680551;    int yDwxbjswIoYGTxz35779858 = -443843955;    int yDwxbjswIoYGTxz48562472 = -147399650;    int yDwxbjswIoYGTxz15678593 = -587109386;    int yDwxbjswIoYGTxz95535683 = -333745676;    int yDwxbjswIoYGTxz88119020 = 54267117;    int yDwxbjswIoYGTxz63371406 = -277194375;    int yDwxbjswIoYGTxz96804890 = 18830887;    int yDwxbjswIoYGTxz81838495 = -254698890;    int yDwxbjswIoYGTxz43671898 = -881321107;    int yDwxbjswIoYGTxz6955334 = 9801751;    int yDwxbjswIoYGTxz5494087 = 91885096;    int yDwxbjswIoYGTxz72551691 = -612567826;    int yDwxbjswIoYGTxz73970459 = -384568251;    int yDwxbjswIoYGTxz54101283 = -809800251;    int yDwxbjswIoYGTxz59452619 = -749535487;    int yDwxbjswIoYGTxz21696327 = -855849622;    int yDwxbjswIoYGTxz18368484 = -163678760;    int yDwxbjswIoYGTxz1402696 = -956747601;    int yDwxbjswIoYGTxz55261529 = -238324186;    int yDwxbjswIoYGTxz72828481 = -368838723;    int yDwxbjswIoYGTxz33925778 = -41442315;    int yDwxbjswIoYGTxz54325655 = -808235762;    int yDwxbjswIoYGTxz47440715 = -812968360;    int yDwxbjswIoYGTxz54836312 = -946479956;    int yDwxbjswIoYGTxz3344956 = -670386143;    int yDwxbjswIoYGTxz7077425 = -858531351;    int yDwxbjswIoYGTxz88279302 = -399389987;    int yDwxbjswIoYGTxz12746071 = -467247892;    int yDwxbjswIoYGTxz5303170 = 25332818;    int yDwxbjswIoYGTxz65263789 = -361119176;    int yDwxbjswIoYGTxz38297290 = -386518219;    int yDwxbjswIoYGTxz36398938 = -26870723;    int yDwxbjswIoYGTxz94666218 = -416795082;    int yDwxbjswIoYGTxz2674253 = -735486106;    int yDwxbjswIoYGTxz65345014 = -732257783;    int yDwxbjswIoYGTxz15304740 = -111607310;    int yDwxbjswIoYGTxz84838910 = -731027859;    int yDwxbjswIoYGTxz27979151 = -477928548;    int yDwxbjswIoYGTxz67006141 = -155630940;    int yDwxbjswIoYGTxz68570779 = -521992234;    int yDwxbjswIoYGTxz49403613 = -794753689;    int yDwxbjswIoYGTxz77345016 = -843812161;    int yDwxbjswIoYGTxz72133733 = -453702933;    int yDwxbjswIoYGTxz81540064 = -312575110;    int yDwxbjswIoYGTxz49942371 = -69086181;    int yDwxbjswIoYGTxz28445113 = -131419363;    int yDwxbjswIoYGTxz48805330 = -251468825;    int yDwxbjswIoYGTxz26291780 = -796527105;    int yDwxbjswIoYGTxz4716455 = -304854520;    int yDwxbjswIoYGTxz32699881 = -115884946;    int yDwxbjswIoYGTxz56098935 = -17528516;    int yDwxbjswIoYGTxz27017545 = 65654200;    int yDwxbjswIoYGTxz54445396 = -730693954;    int yDwxbjswIoYGTxz34213859 = -389459043;    int yDwxbjswIoYGTxz48380921 = 30761764;    int yDwxbjswIoYGTxz81454203 = -635608194;    int yDwxbjswIoYGTxz1121758 = -334431291;    int yDwxbjswIoYGTxz60842281 = -640629431;    int yDwxbjswIoYGTxz92190728 = -663359534;    int yDwxbjswIoYGTxz81041596 = -87201533;    int yDwxbjswIoYGTxz75092103 = -877804389;    int yDwxbjswIoYGTxz84058819 = -513921222;    int yDwxbjswIoYGTxz76535325 = -180031708;    int yDwxbjswIoYGTxz78408109 = -420201931;    int yDwxbjswIoYGTxz68658043 = -603680030;    int yDwxbjswIoYGTxz69095148 = -881244181;    int yDwxbjswIoYGTxz77885472 = -215741718;     yDwxbjswIoYGTxz35936113 = yDwxbjswIoYGTxz10272770;     yDwxbjswIoYGTxz10272770 = yDwxbjswIoYGTxz4341760;     yDwxbjswIoYGTxz4341760 = yDwxbjswIoYGTxz29853215;     yDwxbjswIoYGTxz29853215 = yDwxbjswIoYGTxz68672365;     yDwxbjswIoYGTxz68672365 = yDwxbjswIoYGTxz76915143;     yDwxbjswIoYGTxz76915143 = yDwxbjswIoYGTxz59699378;     yDwxbjswIoYGTxz59699378 = yDwxbjswIoYGTxz39731207;     yDwxbjswIoYGTxz39731207 = yDwxbjswIoYGTxz66717530;     yDwxbjswIoYGTxz66717530 = yDwxbjswIoYGTxz46105863;     yDwxbjswIoYGTxz46105863 = yDwxbjswIoYGTxz1708558;     yDwxbjswIoYGTxz1708558 = yDwxbjswIoYGTxz84980951;     yDwxbjswIoYGTxz84980951 = yDwxbjswIoYGTxz1124872;     yDwxbjswIoYGTxz1124872 = yDwxbjswIoYGTxz63867212;     yDwxbjswIoYGTxz63867212 = yDwxbjswIoYGTxz517504;     yDwxbjswIoYGTxz517504 = yDwxbjswIoYGTxz23514835;     yDwxbjswIoYGTxz23514835 = yDwxbjswIoYGTxz55125162;     yDwxbjswIoYGTxz55125162 = yDwxbjswIoYGTxz31942185;     yDwxbjswIoYGTxz31942185 = yDwxbjswIoYGTxz46208503;     yDwxbjswIoYGTxz46208503 = yDwxbjswIoYGTxz59183511;     yDwxbjswIoYGTxz59183511 = yDwxbjswIoYGTxz15805632;     yDwxbjswIoYGTxz15805632 = yDwxbjswIoYGTxz88495398;     yDwxbjswIoYGTxz88495398 = yDwxbjswIoYGTxz55436457;     yDwxbjswIoYGTxz55436457 = yDwxbjswIoYGTxz996805;     yDwxbjswIoYGTxz996805 = yDwxbjswIoYGTxz22775790;     yDwxbjswIoYGTxz22775790 = yDwxbjswIoYGTxz80393062;     yDwxbjswIoYGTxz80393062 = yDwxbjswIoYGTxz64169073;     yDwxbjswIoYGTxz64169073 = yDwxbjswIoYGTxz54396208;     yDwxbjswIoYGTxz54396208 = yDwxbjswIoYGTxz74467418;     yDwxbjswIoYGTxz74467418 = yDwxbjswIoYGTxz28420240;     yDwxbjswIoYGTxz28420240 = yDwxbjswIoYGTxz9706926;     yDwxbjswIoYGTxz9706926 = yDwxbjswIoYGTxz7042340;     yDwxbjswIoYGTxz7042340 = yDwxbjswIoYGTxz82306699;     yDwxbjswIoYGTxz82306699 = yDwxbjswIoYGTxz35779858;     yDwxbjswIoYGTxz35779858 = yDwxbjswIoYGTxz48562472;     yDwxbjswIoYGTxz48562472 = yDwxbjswIoYGTxz15678593;     yDwxbjswIoYGTxz15678593 = yDwxbjswIoYGTxz95535683;     yDwxbjswIoYGTxz95535683 = yDwxbjswIoYGTxz88119020;     yDwxbjswIoYGTxz88119020 = yDwxbjswIoYGTxz63371406;     yDwxbjswIoYGTxz63371406 = yDwxbjswIoYGTxz96804890;     yDwxbjswIoYGTxz96804890 = yDwxbjswIoYGTxz81838495;     yDwxbjswIoYGTxz81838495 = yDwxbjswIoYGTxz43671898;     yDwxbjswIoYGTxz43671898 = yDwxbjswIoYGTxz6955334;     yDwxbjswIoYGTxz6955334 = yDwxbjswIoYGTxz5494087;     yDwxbjswIoYGTxz5494087 = yDwxbjswIoYGTxz72551691;     yDwxbjswIoYGTxz72551691 = yDwxbjswIoYGTxz73970459;     yDwxbjswIoYGTxz73970459 = yDwxbjswIoYGTxz54101283;     yDwxbjswIoYGTxz54101283 = yDwxbjswIoYGTxz59452619;     yDwxbjswIoYGTxz59452619 = yDwxbjswIoYGTxz21696327;     yDwxbjswIoYGTxz21696327 = yDwxbjswIoYGTxz18368484;     yDwxbjswIoYGTxz18368484 = yDwxbjswIoYGTxz1402696;     yDwxbjswIoYGTxz1402696 = yDwxbjswIoYGTxz55261529;     yDwxbjswIoYGTxz55261529 = yDwxbjswIoYGTxz72828481;     yDwxbjswIoYGTxz72828481 = yDwxbjswIoYGTxz33925778;     yDwxbjswIoYGTxz33925778 = yDwxbjswIoYGTxz54325655;     yDwxbjswIoYGTxz54325655 = yDwxbjswIoYGTxz47440715;     yDwxbjswIoYGTxz47440715 = yDwxbjswIoYGTxz54836312;     yDwxbjswIoYGTxz54836312 = yDwxbjswIoYGTxz3344956;     yDwxbjswIoYGTxz3344956 = yDwxbjswIoYGTxz7077425;     yDwxbjswIoYGTxz7077425 = yDwxbjswIoYGTxz88279302;     yDwxbjswIoYGTxz88279302 = yDwxbjswIoYGTxz12746071;     yDwxbjswIoYGTxz12746071 = yDwxbjswIoYGTxz5303170;     yDwxbjswIoYGTxz5303170 = yDwxbjswIoYGTxz65263789;     yDwxbjswIoYGTxz65263789 = yDwxbjswIoYGTxz38297290;     yDwxbjswIoYGTxz38297290 = yDwxbjswIoYGTxz36398938;     yDwxbjswIoYGTxz36398938 = yDwxbjswIoYGTxz94666218;     yDwxbjswIoYGTxz94666218 = yDwxbjswIoYGTxz2674253;     yDwxbjswIoYGTxz2674253 = yDwxbjswIoYGTxz65345014;     yDwxbjswIoYGTxz65345014 = yDwxbjswIoYGTxz15304740;     yDwxbjswIoYGTxz15304740 = yDwxbjswIoYGTxz84838910;     yDwxbjswIoYGTxz84838910 = yDwxbjswIoYGTxz27979151;     yDwxbjswIoYGTxz27979151 = yDwxbjswIoYGTxz67006141;     yDwxbjswIoYGTxz67006141 = yDwxbjswIoYGTxz68570779;     yDwxbjswIoYGTxz68570779 = yDwxbjswIoYGTxz49403613;     yDwxbjswIoYGTxz49403613 = yDwxbjswIoYGTxz77345016;     yDwxbjswIoYGTxz77345016 = yDwxbjswIoYGTxz72133733;     yDwxbjswIoYGTxz72133733 = yDwxbjswIoYGTxz81540064;     yDwxbjswIoYGTxz81540064 = yDwxbjswIoYGTxz49942371;     yDwxbjswIoYGTxz49942371 = yDwxbjswIoYGTxz28445113;     yDwxbjswIoYGTxz28445113 = yDwxbjswIoYGTxz48805330;     yDwxbjswIoYGTxz48805330 = yDwxbjswIoYGTxz26291780;     yDwxbjswIoYGTxz26291780 = yDwxbjswIoYGTxz4716455;     yDwxbjswIoYGTxz4716455 = yDwxbjswIoYGTxz32699881;     yDwxbjswIoYGTxz32699881 = yDwxbjswIoYGTxz56098935;     yDwxbjswIoYGTxz56098935 = yDwxbjswIoYGTxz27017545;     yDwxbjswIoYGTxz27017545 = yDwxbjswIoYGTxz54445396;     yDwxbjswIoYGTxz54445396 = yDwxbjswIoYGTxz34213859;     yDwxbjswIoYGTxz34213859 = yDwxbjswIoYGTxz48380921;     yDwxbjswIoYGTxz48380921 = yDwxbjswIoYGTxz81454203;     yDwxbjswIoYGTxz81454203 = yDwxbjswIoYGTxz1121758;     yDwxbjswIoYGTxz1121758 = yDwxbjswIoYGTxz60842281;     yDwxbjswIoYGTxz60842281 = yDwxbjswIoYGTxz92190728;     yDwxbjswIoYGTxz92190728 = yDwxbjswIoYGTxz81041596;     yDwxbjswIoYGTxz81041596 = yDwxbjswIoYGTxz75092103;     yDwxbjswIoYGTxz75092103 = yDwxbjswIoYGTxz84058819;     yDwxbjswIoYGTxz84058819 = yDwxbjswIoYGTxz76535325;     yDwxbjswIoYGTxz76535325 = yDwxbjswIoYGTxz78408109;     yDwxbjswIoYGTxz78408109 = yDwxbjswIoYGTxz68658043;     yDwxbjswIoYGTxz68658043 = yDwxbjswIoYGTxz69095148;     yDwxbjswIoYGTxz69095148 = yDwxbjswIoYGTxz77885472;     yDwxbjswIoYGTxz77885472 = yDwxbjswIoYGTxz35936113;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void EOJdDaTqkuNmEaJswQHjQhMbvVUUM20078094() {     int esWlzCZbRTPhouB3107173 = -960167395;    int esWlzCZbRTPhouB55456795 = -905417517;    int esWlzCZbRTPhouB38679778 = -747641942;    int esWlzCZbRTPhouB97253489 = -332979776;    int esWlzCZbRTPhouB49934974 = 23644636;    int esWlzCZbRTPhouB69356346 = -654313814;    int esWlzCZbRTPhouB83345590 = -505558967;    int esWlzCZbRTPhouB62811707 = -397850408;    int esWlzCZbRTPhouB34096677 = 90269368;    int esWlzCZbRTPhouB26437347 = -418902554;    int esWlzCZbRTPhouB83853229 = -325757941;    int esWlzCZbRTPhouB31252283 = -995184993;    int esWlzCZbRTPhouB54370043 = -349765985;    int esWlzCZbRTPhouB60945139 = -324403653;    int esWlzCZbRTPhouB42965171 = -356501107;    int esWlzCZbRTPhouB94435751 = 92516625;    int esWlzCZbRTPhouB20816078 = -203781427;    int esWlzCZbRTPhouB65370634 = -8807248;    int esWlzCZbRTPhouB89410990 = -354255777;    int esWlzCZbRTPhouB86621716 = 61622265;    int esWlzCZbRTPhouB98950816 = -743786404;    int esWlzCZbRTPhouB12496655 = -602084474;    int esWlzCZbRTPhouB9998832 = -467006098;    int esWlzCZbRTPhouB15021004 = -514584976;    int esWlzCZbRTPhouB36732496 = -610673670;    int esWlzCZbRTPhouB70440930 = -975701135;    int esWlzCZbRTPhouB50104962 = 24050084;    int esWlzCZbRTPhouB19025271 = -623647445;    int esWlzCZbRTPhouB76127430 = -894873161;    int esWlzCZbRTPhouB91756973 = -138833940;    int esWlzCZbRTPhouB92460323 = -732326744;    int esWlzCZbRTPhouB2580127 = -26779187;    int esWlzCZbRTPhouB97244075 = -845794711;    int esWlzCZbRTPhouB13521665 = 48798093;    int esWlzCZbRTPhouB53713509 = 50301969;    int esWlzCZbRTPhouB76514916 = -863646897;    int esWlzCZbRTPhouB7942769 = 66890537;    int esWlzCZbRTPhouB44120804 = -655715280;    int esWlzCZbRTPhouB55325405 = -961310346;    int esWlzCZbRTPhouB99552694 = -425693672;    int esWlzCZbRTPhouB86327106 = 17683959;    int esWlzCZbRTPhouB30650581 = -964359057;    int esWlzCZbRTPhouB83180869 = 85905381;    int esWlzCZbRTPhouB64978596 = -564759186;    int esWlzCZbRTPhouB50474413 = -980109156;    int esWlzCZbRTPhouB37629353 = -663188531;    int esWlzCZbRTPhouB49446786 = -723110076;    int esWlzCZbRTPhouB30173272 = -755998425;    int esWlzCZbRTPhouB27088458 = -809353925;    int esWlzCZbRTPhouB79551585 = -109202207;    int esWlzCZbRTPhouB8121291 = -454143921;    int esWlzCZbRTPhouB54994414 = -407563410;    int esWlzCZbRTPhouB44599168 = -96491401;    int esWlzCZbRTPhouB99410007 = -620604965;    int esWlzCZbRTPhouB21127277 = -494999191;    int esWlzCZbRTPhouB90610517 = -258082922;    int esWlzCZbRTPhouB45457964 = -338411419;    int esWlzCZbRTPhouB23658774 = -133056967;    int esWlzCZbRTPhouB60520993 = -722306106;    int esWlzCZbRTPhouB79494043 = -654230;    int esWlzCZbRTPhouB19251384 = -578363899;    int esWlzCZbRTPhouB64320320 = -881911522;    int esWlzCZbRTPhouB86684276 = -502977248;    int esWlzCZbRTPhouB42339704 = -770896693;    int esWlzCZbRTPhouB33977024 = -686575811;    int esWlzCZbRTPhouB81273103 = -198978755;    int esWlzCZbRTPhouB34008207 = -49390282;    int esWlzCZbRTPhouB40848379 = -298564079;    int esWlzCZbRTPhouB7231630 = -274705623;    int esWlzCZbRTPhouB66450255 = -492854211;    int esWlzCZbRTPhouB86492982 = -974373912;    int esWlzCZbRTPhouB76695273 = -548066148;    int esWlzCZbRTPhouB10045230 = -47496903;    int esWlzCZbRTPhouB89858296 = -928562106;    int esWlzCZbRTPhouB294611 = -956061694;    int esWlzCZbRTPhouB68300236 = -779427348;    int esWlzCZbRTPhouB29315785 = -587989855;    int esWlzCZbRTPhouB45020236 = -902246913;    int esWlzCZbRTPhouB64546591 = -534475820;    int esWlzCZbRTPhouB99103143 = -947485139;    int esWlzCZbRTPhouB20994145 = -152591059;    int esWlzCZbRTPhouB19931691 = -219951491;    int esWlzCZbRTPhouB91936813 = -814293521;    int esWlzCZbRTPhouB96575844 = -685670954;    int esWlzCZbRTPhouB83635683 = -684690019;    int esWlzCZbRTPhouB37465909 = -224763334;    int esWlzCZbRTPhouB57980958 = -930287786;    int esWlzCZbRTPhouB97834068 = -125189747;    int esWlzCZbRTPhouB92394387 = -456202716;    int esWlzCZbRTPhouB63102991 = -691615109;    int esWlzCZbRTPhouB31056953 = -425235478;    int esWlzCZbRTPhouB84283994 = -800052497;    int esWlzCZbRTPhouB83599811 = -933409174;    int esWlzCZbRTPhouB75831361 = -860656117;    int esWlzCZbRTPhouB80301311 = -847329773;    int esWlzCZbRTPhouB22006787 = -100404519;    int esWlzCZbRTPhouB43966304 = -361381809;    int esWlzCZbRTPhouB40841166 = -143197927;    int esWlzCZbRTPhouB31001573 = -878183375;    int esWlzCZbRTPhouB69201309 = -960167395;     esWlzCZbRTPhouB3107173 = esWlzCZbRTPhouB55456795;     esWlzCZbRTPhouB55456795 = esWlzCZbRTPhouB38679778;     esWlzCZbRTPhouB38679778 = esWlzCZbRTPhouB97253489;     esWlzCZbRTPhouB97253489 = esWlzCZbRTPhouB49934974;     esWlzCZbRTPhouB49934974 = esWlzCZbRTPhouB69356346;     esWlzCZbRTPhouB69356346 = esWlzCZbRTPhouB83345590;     esWlzCZbRTPhouB83345590 = esWlzCZbRTPhouB62811707;     esWlzCZbRTPhouB62811707 = esWlzCZbRTPhouB34096677;     esWlzCZbRTPhouB34096677 = esWlzCZbRTPhouB26437347;     esWlzCZbRTPhouB26437347 = esWlzCZbRTPhouB83853229;     esWlzCZbRTPhouB83853229 = esWlzCZbRTPhouB31252283;     esWlzCZbRTPhouB31252283 = esWlzCZbRTPhouB54370043;     esWlzCZbRTPhouB54370043 = esWlzCZbRTPhouB60945139;     esWlzCZbRTPhouB60945139 = esWlzCZbRTPhouB42965171;     esWlzCZbRTPhouB42965171 = esWlzCZbRTPhouB94435751;     esWlzCZbRTPhouB94435751 = esWlzCZbRTPhouB20816078;     esWlzCZbRTPhouB20816078 = esWlzCZbRTPhouB65370634;     esWlzCZbRTPhouB65370634 = esWlzCZbRTPhouB89410990;     esWlzCZbRTPhouB89410990 = esWlzCZbRTPhouB86621716;     esWlzCZbRTPhouB86621716 = esWlzCZbRTPhouB98950816;     esWlzCZbRTPhouB98950816 = esWlzCZbRTPhouB12496655;     esWlzCZbRTPhouB12496655 = esWlzCZbRTPhouB9998832;     esWlzCZbRTPhouB9998832 = esWlzCZbRTPhouB15021004;     esWlzCZbRTPhouB15021004 = esWlzCZbRTPhouB36732496;     esWlzCZbRTPhouB36732496 = esWlzCZbRTPhouB70440930;     esWlzCZbRTPhouB70440930 = esWlzCZbRTPhouB50104962;     esWlzCZbRTPhouB50104962 = esWlzCZbRTPhouB19025271;     esWlzCZbRTPhouB19025271 = esWlzCZbRTPhouB76127430;     esWlzCZbRTPhouB76127430 = esWlzCZbRTPhouB91756973;     esWlzCZbRTPhouB91756973 = esWlzCZbRTPhouB92460323;     esWlzCZbRTPhouB92460323 = esWlzCZbRTPhouB2580127;     esWlzCZbRTPhouB2580127 = esWlzCZbRTPhouB97244075;     esWlzCZbRTPhouB97244075 = esWlzCZbRTPhouB13521665;     esWlzCZbRTPhouB13521665 = esWlzCZbRTPhouB53713509;     esWlzCZbRTPhouB53713509 = esWlzCZbRTPhouB76514916;     esWlzCZbRTPhouB76514916 = esWlzCZbRTPhouB7942769;     esWlzCZbRTPhouB7942769 = esWlzCZbRTPhouB44120804;     esWlzCZbRTPhouB44120804 = esWlzCZbRTPhouB55325405;     esWlzCZbRTPhouB55325405 = esWlzCZbRTPhouB99552694;     esWlzCZbRTPhouB99552694 = esWlzCZbRTPhouB86327106;     esWlzCZbRTPhouB86327106 = esWlzCZbRTPhouB30650581;     esWlzCZbRTPhouB30650581 = esWlzCZbRTPhouB83180869;     esWlzCZbRTPhouB83180869 = esWlzCZbRTPhouB64978596;     esWlzCZbRTPhouB64978596 = esWlzCZbRTPhouB50474413;     esWlzCZbRTPhouB50474413 = esWlzCZbRTPhouB37629353;     esWlzCZbRTPhouB37629353 = esWlzCZbRTPhouB49446786;     esWlzCZbRTPhouB49446786 = esWlzCZbRTPhouB30173272;     esWlzCZbRTPhouB30173272 = esWlzCZbRTPhouB27088458;     esWlzCZbRTPhouB27088458 = esWlzCZbRTPhouB79551585;     esWlzCZbRTPhouB79551585 = esWlzCZbRTPhouB8121291;     esWlzCZbRTPhouB8121291 = esWlzCZbRTPhouB54994414;     esWlzCZbRTPhouB54994414 = esWlzCZbRTPhouB44599168;     esWlzCZbRTPhouB44599168 = esWlzCZbRTPhouB99410007;     esWlzCZbRTPhouB99410007 = esWlzCZbRTPhouB21127277;     esWlzCZbRTPhouB21127277 = esWlzCZbRTPhouB90610517;     esWlzCZbRTPhouB90610517 = esWlzCZbRTPhouB45457964;     esWlzCZbRTPhouB45457964 = esWlzCZbRTPhouB23658774;     esWlzCZbRTPhouB23658774 = esWlzCZbRTPhouB60520993;     esWlzCZbRTPhouB60520993 = esWlzCZbRTPhouB79494043;     esWlzCZbRTPhouB79494043 = esWlzCZbRTPhouB19251384;     esWlzCZbRTPhouB19251384 = esWlzCZbRTPhouB64320320;     esWlzCZbRTPhouB64320320 = esWlzCZbRTPhouB86684276;     esWlzCZbRTPhouB86684276 = esWlzCZbRTPhouB42339704;     esWlzCZbRTPhouB42339704 = esWlzCZbRTPhouB33977024;     esWlzCZbRTPhouB33977024 = esWlzCZbRTPhouB81273103;     esWlzCZbRTPhouB81273103 = esWlzCZbRTPhouB34008207;     esWlzCZbRTPhouB34008207 = esWlzCZbRTPhouB40848379;     esWlzCZbRTPhouB40848379 = esWlzCZbRTPhouB7231630;     esWlzCZbRTPhouB7231630 = esWlzCZbRTPhouB66450255;     esWlzCZbRTPhouB66450255 = esWlzCZbRTPhouB86492982;     esWlzCZbRTPhouB86492982 = esWlzCZbRTPhouB76695273;     esWlzCZbRTPhouB76695273 = esWlzCZbRTPhouB10045230;     esWlzCZbRTPhouB10045230 = esWlzCZbRTPhouB89858296;     esWlzCZbRTPhouB89858296 = esWlzCZbRTPhouB294611;     esWlzCZbRTPhouB294611 = esWlzCZbRTPhouB68300236;     esWlzCZbRTPhouB68300236 = esWlzCZbRTPhouB29315785;     esWlzCZbRTPhouB29315785 = esWlzCZbRTPhouB45020236;     esWlzCZbRTPhouB45020236 = esWlzCZbRTPhouB64546591;     esWlzCZbRTPhouB64546591 = esWlzCZbRTPhouB99103143;     esWlzCZbRTPhouB99103143 = esWlzCZbRTPhouB20994145;     esWlzCZbRTPhouB20994145 = esWlzCZbRTPhouB19931691;     esWlzCZbRTPhouB19931691 = esWlzCZbRTPhouB91936813;     esWlzCZbRTPhouB91936813 = esWlzCZbRTPhouB96575844;     esWlzCZbRTPhouB96575844 = esWlzCZbRTPhouB83635683;     esWlzCZbRTPhouB83635683 = esWlzCZbRTPhouB37465909;     esWlzCZbRTPhouB37465909 = esWlzCZbRTPhouB57980958;     esWlzCZbRTPhouB57980958 = esWlzCZbRTPhouB97834068;     esWlzCZbRTPhouB97834068 = esWlzCZbRTPhouB92394387;     esWlzCZbRTPhouB92394387 = esWlzCZbRTPhouB63102991;     esWlzCZbRTPhouB63102991 = esWlzCZbRTPhouB31056953;     esWlzCZbRTPhouB31056953 = esWlzCZbRTPhouB84283994;     esWlzCZbRTPhouB84283994 = esWlzCZbRTPhouB83599811;     esWlzCZbRTPhouB83599811 = esWlzCZbRTPhouB75831361;     esWlzCZbRTPhouB75831361 = esWlzCZbRTPhouB80301311;     esWlzCZbRTPhouB80301311 = esWlzCZbRTPhouB22006787;     esWlzCZbRTPhouB22006787 = esWlzCZbRTPhouB43966304;     esWlzCZbRTPhouB43966304 = esWlzCZbRTPhouB40841166;     esWlzCZbRTPhouB40841166 = esWlzCZbRTPhouB31001573;     esWlzCZbRTPhouB31001573 = esWlzCZbRTPhouB69201309;     esWlzCZbRTPhouB69201309 = esWlzCZbRTPhouB3107173;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mbrNXnuDBeNyEFgqkcdDDrQHIaNtn22267433() {     int FMBqnXISfYllABx99620093 = -958968083;    int FMBqnXISfYllABx70539621 = -858105819;    int FMBqnXISfYllABx94362088 = -495744708;    int FMBqnXISfYllABx9802338 = -801793566;    int FMBqnXISfYllABx18587364 = -631942359;    int FMBqnXISfYllABx6603218 = -684841390;    int FMBqnXISfYllABx57160244 = -670476942;    int FMBqnXISfYllABx98263877 = -510734592;    int FMBqnXISfYllABx38350934 = -162148385;    int FMBqnXISfYllABx80441883 = -77627767;    int FMBqnXISfYllABx44069345 = -300993178;    int FMBqnXISfYllABx46268720 = -463917898;    int FMBqnXISfYllABx59640740 = -635551649;    int FMBqnXISfYllABx28020252 = -833925188;    int FMBqnXISfYllABx47864928 = -207827817;    int FMBqnXISfYllABx7392575 = -766104590;    int FMBqnXISfYllABx48506926 = -116628472;    int FMBqnXISfYllABx1506123 = -626333314;    int FMBqnXISfYllABx71333967 = -843383078;    int FMBqnXISfYllABx27346976 = -947902565;    int FMBqnXISfYllABx6629672 = -954336003;    int FMBqnXISfYllABx53398412 = -980102166;    int FMBqnXISfYllABx99495794 = -747925808;    int FMBqnXISfYllABx38845774 = -473702380;    int FMBqnXISfYllABx15007504 = -136099429;    int FMBqnXISfYllABx46465295 = -54996397;    int FMBqnXISfYllABx98001646 = -903131763;    int FMBqnXISfYllABx11275634 = -279403544;    int FMBqnXISfYllABx58323402 = -760251144;    int FMBqnXISfYllABx92011481 = -603610782;    int FMBqnXISfYllABx1554403 = 65705103;    int FMBqnXISfYllABx38330309 = -358785207;    int FMBqnXISfYllABx14688515 = -735891479;    int FMBqnXISfYllABx26181333 = -871711906;    int FMBqnXISfYllABx63198347 = -164363556;    int FMBqnXISfYllABx20186170 = -40723605;    int FMBqnXISfYllABx88499735 = -543533421;    int FMBqnXISfYllABx17247779 = -476847249;    int FMBqnXISfYllABx4059712 = -202379096;    int FMBqnXISfYllABx79070094 = -962041554;    int FMBqnXISfYllABx18598102 = -513858212;    int FMBqnXISfYllABx74843932 = -371800019;    int FMBqnXISfYllABx70722805 = -410367191;    int FMBqnXISfYllABx77186704 = -706392939;    int FMBqnXISfYllABx16461128 = -412263494;    int FMBqnXISfYllABx35310380 = -995516327;    int FMBqnXISfYllABx46358879 = 98369276;    int FMBqnXISfYllABx10663295 = -919092424;    int FMBqnXISfYllABx45697526 = -340858120;    int FMBqnXISfYllABx52352625 = -458741663;    int FMBqnXISfYllABx59909931 = 16476873;    int FMBqnXISfYllABx27403053 = -989728660;    int FMBqnXISfYllABx36742547 = -888400045;    int FMBqnXISfYllABx84917203 = 29194031;    int FMBqnXISfYllABx15637761 = -927720984;    int FMBqnXISfYllABx46221681 = -978865917;    int FMBqnXISfYllABx71043826 = -10180011;    int FMBqnXISfYllABx55516315 = 77957672;    int FMBqnXISfYllABx94794834 = -565694138;    int FMBqnXISfYllABx72122069 = -476945963;    int FMBqnXISfYllABx8601572 = -781709628;    int FMBqnXISfYllABx45884610 = -291073398;    int FMBqnXISfYllABx39940476 = -750483449;    int FMBqnXISfYllABx46339453 = -558537604;    int FMBqnXISfYllABx78887480 = -43332871;    int FMBqnXISfYllABx5739036 = -942207971;    int FMBqnXISfYllABx31580206 = -728026420;    int FMBqnXISfYllABx33459408 = -763839743;    int FMBqnXISfYllABx64821904 = -569561632;    int FMBqnXISfYllABx27678759 = -67104212;    int FMBqnXISfYllABx18892840 = -122571170;    int FMBqnXISfYllABx31259148 = -639781224;    int FMBqnXISfYllABx97446411 = -323954218;    int FMBqnXISfYllABx92263873 = -881341524;    int FMBqnXISfYllABx8748875 = -334044354;    int FMBqnXISfYllABx31785739 = -482535984;    int FMBqnXISfYllABx82675606 = -469734976;    int FMBqnXISfYllABx22309090 = 58467130;    int FMBqnXISfYllABx22384647 = 38561113;    int FMBqnXISfYllABx79697123 = -140583102;    int FMBqnXISfYllABx106417 = -53365674;    int FMBqnXISfYllABx87338352 = -984039339;    int FMBqnXISfYllABx65578108 = -938545425;    int FMBqnXISfYllABx5970777 = -201509482;    int FMBqnXISfYllABx32101551 = -520087655;    int FMBqnXISfYllABx74151350 = 55433763;    int FMBqnXISfYllABx1587763 = -470385162;    int FMBqnXISfYllABx29771312 = -665085511;    int FMBqnXISfYllABx10543573 = -943990923;    int FMBqnXISfYllABx16976667 = -185497639;    int FMBqnXISfYllABx49142343 = 69456406;    int FMBqnXISfYllABx32983421 = -521491093;    int FMBqnXISfYllABx22452945 = -911153112;    int FMBqnXISfYllABx31937642 = -725433134;    int FMBqnXISfYllABx70468522 = -80331926;    int FMBqnXISfYllABx72713491 = -122784814;    int FMBqnXISfYllABx34903457 = -621316571;    int FMBqnXISfYllABx24383353 = -851829588;    int FMBqnXISfYllABx98299224 = -563060069;    int FMBqnXISfYllABx10722093 = -958968083;     FMBqnXISfYllABx99620093 = FMBqnXISfYllABx70539621;     FMBqnXISfYllABx70539621 = FMBqnXISfYllABx94362088;     FMBqnXISfYllABx94362088 = FMBqnXISfYllABx9802338;     FMBqnXISfYllABx9802338 = FMBqnXISfYllABx18587364;     FMBqnXISfYllABx18587364 = FMBqnXISfYllABx6603218;     FMBqnXISfYllABx6603218 = FMBqnXISfYllABx57160244;     FMBqnXISfYllABx57160244 = FMBqnXISfYllABx98263877;     FMBqnXISfYllABx98263877 = FMBqnXISfYllABx38350934;     FMBqnXISfYllABx38350934 = FMBqnXISfYllABx80441883;     FMBqnXISfYllABx80441883 = FMBqnXISfYllABx44069345;     FMBqnXISfYllABx44069345 = FMBqnXISfYllABx46268720;     FMBqnXISfYllABx46268720 = FMBqnXISfYllABx59640740;     FMBqnXISfYllABx59640740 = FMBqnXISfYllABx28020252;     FMBqnXISfYllABx28020252 = FMBqnXISfYllABx47864928;     FMBqnXISfYllABx47864928 = FMBqnXISfYllABx7392575;     FMBqnXISfYllABx7392575 = FMBqnXISfYllABx48506926;     FMBqnXISfYllABx48506926 = FMBqnXISfYllABx1506123;     FMBqnXISfYllABx1506123 = FMBqnXISfYllABx71333967;     FMBqnXISfYllABx71333967 = FMBqnXISfYllABx27346976;     FMBqnXISfYllABx27346976 = FMBqnXISfYllABx6629672;     FMBqnXISfYllABx6629672 = FMBqnXISfYllABx53398412;     FMBqnXISfYllABx53398412 = FMBqnXISfYllABx99495794;     FMBqnXISfYllABx99495794 = FMBqnXISfYllABx38845774;     FMBqnXISfYllABx38845774 = FMBqnXISfYllABx15007504;     FMBqnXISfYllABx15007504 = FMBqnXISfYllABx46465295;     FMBqnXISfYllABx46465295 = FMBqnXISfYllABx98001646;     FMBqnXISfYllABx98001646 = FMBqnXISfYllABx11275634;     FMBqnXISfYllABx11275634 = FMBqnXISfYllABx58323402;     FMBqnXISfYllABx58323402 = FMBqnXISfYllABx92011481;     FMBqnXISfYllABx92011481 = FMBqnXISfYllABx1554403;     FMBqnXISfYllABx1554403 = FMBqnXISfYllABx38330309;     FMBqnXISfYllABx38330309 = FMBqnXISfYllABx14688515;     FMBqnXISfYllABx14688515 = FMBqnXISfYllABx26181333;     FMBqnXISfYllABx26181333 = FMBqnXISfYllABx63198347;     FMBqnXISfYllABx63198347 = FMBqnXISfYllABx20186170;     FMBqnXISfYllABx20186170 = FMBqnXISfYllABx88499735;     FMBqnXISfYllABx88499735 = FMBqnXISfYllABx17247779;     FMBqnXISfYllABx17247779 = FMBqnXISfYllABx4059712;     FMBqnXISfYllABx4059712 = FMBqnXISfYllABx79070094;     FMBqnXISfYllABx79070094 = FMBqnXISfYllABx18598102;     FMBqnXISfYllABx18598102 = FMBqnXISfYllABx74843932;     FMBqnXISfYllABx74843932 = FMBqnXISfYllABx70722805;     FMBqnXISfYllABx70722805 = FMBqnXISfYllABx77186704;     FMBqnXISfYllABx77186704 = FMBqnXISfYllABx16461128;     FMBqnXISfYllABx16461128 = FMBqnXISfYllABx35310380;     FMBqnXISfYllABx35310380 = FMBqnXISfYllABx46358879;     FMBqnXISfYllABx46358879 = FMBqnXISfYllABx10663295;     FMBqnXISfYllABx10663295 = FMBqnXISfYllABx45697526;     FMBqnXISfYllABx45697526 = FMBqnXISfYllABx52352625;     FMBqnXISfYllABx52352625 = FMBqnXISfYllABx59909931;     FMBqnXISfYllABx59909931 = FMBqnXISfYllABx27403053;     FMBqnXISfYllABx27403053 = FMBqnXISfYllABx36742547;     FMBqnXISfYllABx36742547 = FMBqnXISfYllABx84917203;     FMBqnXISfYllABx84917203 = FMBqnXISfYllABx15637761;     FMBqnXISfYllABx15637761 = FMBqnXISfYllABx46221681;     FMBqnXISfYllABx46221681 = FMBqnXISfYllABx71043826;     FMBqnXISfYllABx71043826 = FMBqnXISfYllABx55516315;     FMBqnXISfYllABx55516315 = FMBqnXISfYllABx94794834;     FMBqnXISfYllABx94794834 = FMBqnXISfYllABx72122069;     FMBqnXISfYllABx72122069 = FMBqnXISfYllABx8601572;     FMBqnXISfYllABx8601572 = FMBqnXISfYllABx45884610;     FMBqnXISfYllABx45884610 = FMBqnXISfYllABx39940476;     FMBqnXISfYllABx39940476 = FMBqnXISfYllABx46339453;     FMBqnXISfYllABx46339453 = FMBqnXISfYllABx78887480;     FMBqnXISfYllABx78887480 = FMBqnXISfYllABx5739036;     FMBqnXISfYllABx5739036 = FMBqnXISfYllABx31580206;     FMBqnXISfYllABx31580206 = FMBqnXISfYllABx33459408;     FMBqnXISfYllABx33459408 = FMBqnXISfYllABx64821904;     FMBqnXISfYllABx64821904 = FMBqnXISfYllABx27678759;     FMBqnXISfYllABx27678759 = FMBqnXISfYllABx18892840;     FMBqnXISfYllABx18892840 = FMBqnXISfYllABx31259148;     FMBqnXISfYllABx31259148 = FMBqnXISfYllABx97446411;     FMBqnXISfYllABx97446411 = FMBqnXISfYllABx92263873;     FMBqnXISfYllABx92263873 = FMBqnXISfYllABx8748875;     FMBqnXISfYllABx8748875 = FMBqnXISfYllABx31785739;     FMBqnXISfYllABx31785739 = FMBqnXISfYllABx82675606;     FMBqnXISfYllABx82675606 = FMBqnXISfYllABx22309090;     FMBqnXISfYllABx22309090 = FMBqnXISfYllABx22384647;     FMBqnXISfYllABx22384647 = FMBqnXISfYllABx79697123;     FMBqnXISfYllABx79697123 = FMBqnXISfYllABx106417;     FMBqnXISfYllABx106417 = FMBqnXISfYllABx87338352;     FMBqnXISfYllABx87338352 = FMBqnXISfYllABx65578108;     FMBqnXISfYllABx65578108 = FMBqnXISfYllABx5970777;     FMBqnXISfYllABx5970777 = FMBqnXISfYllABx32101551;     FMBqnXISfYllABx32101551 = FMBqnXISfYllABx74151350;     FMBqnXISfYllABx74151350 = FMBqnXISfYllABx1587763;     FMBqnXISfYllABx1587763 = FMBqnXISfYllABx29771312;     FMBqnXISfYllABx29771312 = FMBqnXISfYllABx10543573;     FMBqnXISfYllABx10543573 = FMBqnXISfYllABx16976667;     FMBqnXISfYllABx16976667 = FMBqnXISfYllABx49142343;     FMBqnXISfYllABx49142343 = FMBqnXISfYllABx32983421;     FMBqnXISfYllABx32983421 = FMBqnXISfYllABx22452945;     FMBqnXISfYllABx22452945 = FMBqnXISfYllABx31937642;     FMBqnXISfYllABx31937642 = FMBqnXISfYllABx70468522;     FMBqnXISfYllABx70468522 = FMBqnXISfYllABx72713491;     FMBqnXISfYllABx72713491 = FMBqnXISfYllABx34903457;     FMBqnXISfYllABx34903457 = FMBqnXISfYllABx24383353;     FMBqnXISfYllABx24383353 = FMBqnXISfYllABx98299224;     FMBqnXISfYllABx98299224 = FMBqnXISfYllABx10722093;     FMBqnXISfYllABx10722093 = FMBqnXISfYllABx99620093;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NtVHQnspLBvGuHcaAcafMBUPMpazL24456771() {     int LJIqhYEUuKonVUv96133013 = -957768771;    int LJIqhYEUuKonVUv85622446 = -810794121;    int LJIqhYEUuKonVUv50044399 = -243847473;    int LJIqhYEUuKonVUv22351186 = -170607357;    int LJIqhYEUuKonVUv87239754 = -187529355;    int LJIqhYEUuKonVUv43850090 = -715368967;    int LJIqhYEUuKonVUv30974897 = -835394917;    int LJIqhYEUuKonVUv33716048 = -623618777;    int LJIqhYEUuKonVUv42605191 = -414566139;    int LJIqhYEUuKonVUv34446419 = -836352980;    int LJIqhYEUuKonVUv4285461 = -276228415;    int LJIqhYEUuKonVUv61285157 = 67349196;    int LJIqhYEUuKonVUv64911438 = -921337312;    int LJIqhYEUuKonVUv95095365 = -243446723;    int LJIqhYEUuKonVUv52764686 = -59154527;    int LJIqhYEUuKonVUv20349399 = -524725805;    int LJIqhYEUuKonVUv76197775 = -29475517;    int LJIqhYEUuKonVUv37641611 = -143859379;    int LJIqhYEUuKonVUv53256943 = -232510378;    int LJIqhYEUuKonVUv68072235 = -857427395;    int LJIqhYEUuKonVUv14308527 = -64885601;    int LJIqhYEUuKonVUv94300169 = -258119859;    int LJIqhYEUuKonVUv88992757 = 71154481;    int LJIqhYEUuKonVUv62670544 = -432819785;    int LJIqhYEUuKonVUv93282510 = -761525189;    int LJIqhYEUuKonVUv22489660 = -234291660;    int LJIqhYEUuKonVUv45898332 = -730313609;    int LJIqhYEUuKonVUv3525996 = 64840357;    int LJIqhYEUuKonVUv40519373 = -625629127;    int LJIqhYEUuKonVUv92265988 = 31612376;    int LJIqhYEUuKonVUv10648483 = -236263050;    int LJIqhYEUuKonVUv74080492 = -690791227;    int LJIqhYEUuKonVUv32132954 = -625988247;    int LJIqhYEUuKonVUv38841001 = -692221906;    int LJIqhYEUuKonVUv72683186 = -379029082;    int LJIqhYEUuKonVUv63857423 = -317800313;    int LJIqhYEUuKonVUv69056702 = -53957378;    int LJIqhYEUuKonVUv90374752 = -297979218;    int LJIqhYEUuKonVUv52794018 = -543447847;    int LJIqhYEUuKonVUv58587493 = -398389436;    int LJIqhYEUuKonVUv50869096 = 54599618;    int LJIqhYEUuKonVUv19037285 = -879240982;    int LJIqhYEUuKonVUv58264741 = -906639763;    int LJIqhYEUuKonVUv89394812 = -848026693;    int LJIqhYEUuKonVUv82447842 = -944417832;    int LJIqhYEUuKonVUv32991406 = -227844123;    int LJIqhYEUuKonVUv43270972 = -180151372;    int LJIqhYEUuKonVUv91153318 = 17813577;    int LJIqhYEUuKonVUv64306593 = -972362315;    int LJIqhYEUuKonVUv25153664 = -808281118;    int LJIqhYEUuKonVUv11698571 = -612902334;    int LJIqhYEUuKonVUv99811691 = -471893910;    int LJIqhYEUuKonVUv28885925 = -580308690;    int LJIqhYEUuKonVUv70424399 = -421006973;    int LJIqhYEUuKonVUv10148244 = -260442776;    int LJIqhYEUuKonVUv1832845 = -599648913;    int LJIqhYEUuKonVUv96629689 = -781948603;    int LJIqhYEUuKonVUv87373855 = -811027689;    int LJIqhYEUuKonVUv29068675 = -409082169;    int LJIqhYEUuKonVUv64750094 = -953237696;    int LJIqhYEUuKonVUv97951758 = -985055358;    int LJIqhYEUuKonVUv27448901 = -800235274;    int LJIqhYEUuKonVUv93196674 = -997989650;    int LJIqhYEUuKonVUv50339202 = -346178515;    int LJIqhYEUuKonVUv23797937 = -500089930;    int LJIqhYEUuKonVUv30204968 = -585437188;    int LJIqhYEUuKonVUv29152204 = -306662558;    int LJIqhYEUuKonVUv26070437 = -129115406;    int LJIqhYEUuKonVUv22412179 = -864417642;    int LJIqhYEUuKonVUv88907262 = -741354214;    int LJIqhYEUuKonVUv51292696 = -370768427;    int LJIqhYEUuKonVUv85823022 = -731496299;    int LJIqhYEUuKonVUv84847592 = -600411532;    int LJIqhYEUuKonVUv94669450 = -834120942;    int LJIqhYEUuKonVUv17203139 = -812027014;    int LJIqhYEUuKonVUv95271242 = -185644620;    int LJIqhYEUuKonVUv36035428 = -351480096;    int LJIqhYEUuKonVUv99597944 = -80818826;    int LJIqhYEUuKonVUv80222701 = -488401953;    int LJIqhYEUuKonVUv60291104 = -433681066;    int LJIqhYEUuKonVUv79218688 = 45859712;    int LJIqhYEUuKonVUv54745014 = -648127187;    int LJIqhYEUuKonVUv39219403 = 37202672;    int LJIqhYEUuKonVUv15365709 = -817348010;    int LJIqhYEUuKonVUv80567418 = -355485291;    int LJIqhYEUuKonVUv10836792 = -764369140;    int LJIqhYEUuKonVUv45194568 = -10482538;    int LJIqhYEUuKonVUv61708554 = -104981275;    int LJIqhYEUuKonVUv28692757 = -331779131;    int LJIqhYEUuKonVUv70850341 = -779380170;    int LJIqhYEUuKonVUv67227734 = -535851711;    int LJIqhYEUuKonVUv81682847 = -242929689;    int LJIqhYEUuKonVUv61306078 = -888897049;    int LJIqhYEUuKonVUv88043923 = -590210152;    int LJIqhYEUuKonVUv60635734 = -413334078;    int LJIqhYEUuKonVUv23420196 = -145165108;    int LJIqhYEUuKonVUv25840610 = -881251332;    int LJIqhYEUuKonVUv7925540 = -460461248;    int LJIqhYEUuKonVUv65596876 = -247936763;    int LJIqhYEUuKonVUv52242875 = -957768771;     LJIqhYEUuKonVUv96133013 = LJIqhYEUuKonVUv85622446;     LJIqhYEUuKonVUv85622446 = LJIqhYEUuKonVUv50044399;     LJIqhYEUuKonVUv50044399 = LJIqhYEUuKonVUv22351186;     LJIqhYEUuKonVUv22351186 = LJIqhYEUuKonVUv87239754;     LJIqhYEUuKonVUv87239754 = LJIqhYEUuKonVUv43850090;     LJIqhYEUuKonVUv43850090 = LJIqhYEUuKonVUv30974897;     LJIqhYEUuKonVUv30974897 = LJIqhYEUuKonVUv33716048;     LJIqhYEUuKonVUv33716048 = LJIqhYEUuKonVUv42605191;     LJIqhYEUuKonVUv42605191 = LJIqhYEUuKonVUv34446419;     LJIqhYEUuKonVUv34446419 = LJIqhYEUuKonVUv4285461;     LJIqhYEUuKonVUv4285461 = LJIqhYEUuKonVUv61285157;     LJIqhYEUuKonVUv61285157 = LJIqhYEUuKonVUv64911438;     LJIqhYEUuKonVUv64911438 = LJIqhYEUuKonVUv95095365;     LJIqhYEUuKonVUv95095365 = LJIqhYEUuKonVUv52764686;     LJIqhYEUuKonVUv52764686 = LJIqhYEUuKonVUv20349399;     LJIqhYEUuKonVUv20349399 = LJIqhYEUuKonVUv76197775;     LJIqhYEUuKonVUv76197775 = LJIqhYEUuKonVUv37641611;     LJIqhYEUuKonVUv37641611 = LJIqhYEUuKonVUv53256943;     LJIqhYEUuKonVUv53256943 = LJIqhYEUuKonVUv68072235;     LJIqhYEUuKonVUv68072235 = LJIqhYEUuKonVUv14308527;     LJIqhYEUuKonVUv14308527 = LJIqhYEUuKonVUv94300169;     LJIqhYEUuKonVUv94300169 = LJIqhYEUuKonVUv88992757;     LJIqhYEUuKonVUv88992757 = LJIqhYEUuKonVUv62670544;     LJIqhYEUuKonVUv62670544 = LJIqhYEUuKonVUv93282510;     LJIqhYEUuKonVUv93282510 = LJIqhYEUuKonVUv22489660;     LJIqhYEUuKonVUv22489660 = LJIqhYEUuKonVUv45898332;     LJIqhYEUuKonVUv45898332 = LJIqhYEUuKonVUv3525996;     LJIqhYEUuKonVUv3525996 = LJIqhYEUuKonVUv40519373;     LJIqhYEUuKonVUv40519373 = LJIqhYEUuKonVUv92265988;     LJIqhYEUuKonVUv92265988 = LJIqhYEUuKonVUv10648483;     LJIqhYEUuKonVUv10648483 = LJIqhYEUuKonVUv74080492;     LJIqhYEUuKonVUv74080492 = LJIqhYEUuKonVUv32132954;     LJIqhYEUuKonVUv32132954 = LJIqhYEUuKonVUv38841001;     LJIqhYEUuKonVUv38841001 = LJIqhYEUuKonVUv72683186;     LJIqhYEUuKonVUv72683186 = LJIqhYEUuKonVUv63857423;     LJIqhYEUuKonVUv63857423 = LJIqhYEUuKonVUv69056702;     LJIqhYEUuKonVUv69056702 = LJIqhYEUuKonVUv90374752;     LJIqhYEUuKonVUv90374752 = LJIqhYEUuKonVUv52794018;     LJIqhYEUuKonVUv52794018 = LJIqhYEUuKonVUv58587493;     LJIqhYEUuKonVUv58587493 = LJIqhYEUuKonVUv50869096;     LJIqhYEUuKonVUv50869096 = LJIqhYEUuKonVUv19037285;     LJIqhYEUuKonVUv19037285 = LJIqhYEUuKonVUv58264741;     LJIqhYEUuKonVUv58264741 = LJIqhYEUuKonVUv89394812;     LJIqhYEUuKonVUv89394812 = LJIqhYEUuKonVUv82447842;     LJIqhYEUuKonVUv82447842 = LJIqhYEUuKonVUv32991406;     LJIqhYEUuKonVUv32991406 = LJIqhYEUuKonVUv43270972;     LJIqhYEUuKonVUv43270972 = LJIqhYEUuKonVUv91153318;     LJIqhYEUuKonVUv91153318 = LJIqhYEUuKonVUv64306593;     LJIqhYEUuKonVUv64306593 = LJIqhYEUuKonVUv25153664;     LJIqhYEUuKonVUv25153664 = LJIqhYEUuKonVUv11698571;     LJIqhYEUuKonVUv11698571 = LJIqhYEUuKonVUv99811691;     LJIqhYEUuKonVUv99811691 = LJIqhYEUuKonVUv28885925;     LJIqhYEUuKonVUv28885925 = LJIqhYEUuKonVUv70424399;     LJIqhYEUuKonVUv70424399 = LJIqhYEUuKonVUv10148244;     LJIqhYEUuKonVUv10148244 = LJIqhYEUuKonVUv1832845;     LJIqhYEUuKonVUv1832845 = LJIqhYEUuKonVUv96629689;     LJIqhYEUuKonVUv96629689 = LJIqhYEUuKonVUv87373855;     LJIqhYEUuKonVUv87373855 = LJIqhYEUuKonVUv29068675;     LJIqhYEUuKonVUv29068675 = LJIqhYEUuKonVUv64750094;     LJIqhYEUuKonVUv64750094 = LJIqhYEUuKonVUv97951758;     LJIqhYEUuKonVUv97951758 = LJIqhYEUuKonVUv27448901;     LJIqhYEUuKonVUv27448901 = LJIqhYEUuKonVUv93196674;     LJIqhYEUuKonVUv93196674 = LJIqhYEUuKonVUv50339202;     LJIqhYEUuKonVUv50339202 = LJIqhYEUuKonVUv23797937;     LJIqhYEUuKonVUv23797937 = LJIqhYEUuKonVUv30204968;     LJIqhYEUuKonVUv30204968 = LJIqhYEUuKonVUv29152204;     LJIqhYEUuKonVUv29152204 = LJIqhYEUuKonVUv26070437;     LJIqhYEUuKonVUv26070437 = LJIqhYEUuKonVUv22412179;     LJIqhYEUuKonVUv22412179 = LJIqhYEUuKonVUv88907262;     LJIqhYEUuKonVUv88907262 = LJIqhYEUuKonVUv51292696;     LJIqhYEUuKonVUv51292696 = LJIqhYEUuKonVUv85823022;     LJIqhYEUuKonVUv85823022 = LJIqhYEUuKonVUv84847592;     LJIqhYEUuKonVUv84847592 = LJIqhYEUuKonVUv94669450;     LJIqhYEUuKonVUv94669450 = LJIqhYEUuKonVUv17203139;     LJIqhYEUuKonVUv17203139 = LJIqhYEUuKonVUv95271242;     LJIqhYEUuKonVUv95271242 = LJIqhYEUuKonVUv36035428;     LJIqhYEUuKonVUv36035428 = LJIqhYEUuKonVUv99597944;     LJIqhYEUuKonVUv99597944 = LJIqhYEUuKonVUv80222701;     LJIqhYEUuKonVUv80222701 = LJIqhYEUuKonVUv60291104;     LJIqhYEUuKonVUv60291104 = LJIqhYEUuKonVUv79218688;     LJIqhYEUuKonVUv79218688 = LJIqhYEUuKonVUv54745014;     LJIqhYEUuKonVUv54745014 = LJIqhYEUuKonVUv39219403;     LJIqhYEUuKonVUv39219403 = LJIqhYEUuKonVUv15365709;     LJIqhYEUuKonVUv15365709 = LJIqhYEUuKonVUv80567418;     LJIqhYEUuKonVUv80567418 = LJIqhYEUuKonVUv10836792;     LJIqhYEUuKonVUv10836792 = LJIqhYEUuKonVUv45194568;     LJIqhYEUuKonVUv45194568 = LJIqhYEUuKonVUv61708554;     LJIqhYEUuKonVUv61708554 = LJIqhYEUuKonVUv28692757;     LJIqhYEUuKonVUv28692757 = LJIqhYEUuKonVUv70850341;     LJIqhYEUuKonVUv70850341 = LJIqhYEUuKonVUv67227734;     LJIqhYEUuKonVUv67227734 = LJIqhYEUuKonVUv81682847;     LJIqhYEUuKonVUv81682847 = LJIqhYEUuKonVUv61306078;     LJIqhYEUuKonVUv61306078 = LJIqhYEUuKonVUv88043923;     LJIqhYEUuKonVUv88043923 = LJIqhYEUuKonVUv60635734;     LJIqhYEUuKonVUv60635734 = LJIqhYEUuKonVUv23420196;     LJIqhYEUuKonVUv23420196 = LJIqhYEUuKonVUv25840610;     LJIqhYEUuKonVUv25840610 = LJIqhYEUuKonVUv7925540;     LJIqhYEUuKonVUv7925540 = LJIqhYEUuKonVUv65596876;     LJIqhYEUuKonVUv65596876 = LJIqhYEUuKonVUv52242875;     LJIqhYEUuKonVUv52242875 = LJIqhYEUuKonVUv96133013;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void QNURGEmCIaBvfwUnhBTCqIWRVVIgF26646109() {     int faAwxyYQOtksBxx92645934 = -956569460;    int faAwxyYQOtksBxx705272 = -763482423;    int faAwxyYQOtksBxx5726711 = 8049761;    int faAwxyYQOtksBxx34900034 = -639421148;    int faAwxyYQOtksBxx55892144 = -843116351;    int faAwxyYQOtksBxx81096962 = -745896543;    int faAwxyYQOtksBxx4789550 = 99687108;    int faAwxyYQOtksBxx69168218 = -736502961;    int faAwxyYQOtksBxx46859448 = -666983893;    int faAwxyYQOtksBxx88450955 = -495078193;    int faAwxyYQOtksBxx64501575 = -251463652;    int faAwxyYQOtksBxx76301594 = -501383710;    int faAwxyYQOtksBxx70182136 = -107122975;    int faAwxyYQOtksBxx62170478 = -752968258;    int faAwxyYQOtksBxx57664443 = 89518763;    int faAwxyYQOtksBxx33306222 = -283347020;    int faAwxyYQOtksBxx3888624 = 57677439;    int faAwxyYQOtksBxx73777099 = -761385444;    int faAwxyYQOtksBxx35179919 = -721637678;    int faAwxyYQOtksBxx8797495 = -766952226;    int faAwxyYQOtksBxx21987382 = -275435199;    int faAwxyYQOtksBxx35201927 = -636137552;    int faAwxyYQOtksBxx78489719 = -209765229;    int faAwxyYQOtksBxx86495314 = -391937189;    int faAwxyYQOtksBxx71557518 = -286950948;    int faAwxyYQOtksBxx98514024 = -413586923;    int faAwxyYQOtksBxx93795016 = -557495456;    int faAwxyYQOtksBxx95776358 = -690915742;    int faAwxyYQOtksBxx22715344 = -491007110;    int faAwxyYQOtksBxx92520496 = -433164467;    int faAwxyYQOtksBxx19742562 = -538231203;    int faAwxyYQOtksBxx9830676 = 77202752;    int faAwxyYQOtksBxx49577392 = -516085015;    int faAwxyYQOtksBxx51500669 = -512731906;    int faAwxyYQOtksBxx82168024 = -593694608;    int faAwxyYQOtksBxx7528677 = -594877022;    int faAwxyYQOtksBxx49613669 = -664381336;    int faAwxyYQOtksBxx63501727 = -119111186;    int faAwxyYQOtksBxx1528325 = -884516597;    int faAwxyYQOtksBxx38104892 = -934737318;    int faAwxyYQOtksBxx83140091 = -476942553;    int faAwxyYQOtksBxx63230637 = -286681944;    int faAwxyYQOtksBxx45806677 = -302912335;    int faAwxyYQOtksBxx1602921 = -989660447;    int faAwxyYQOtksBxx48434557 = -376572169;    int faAwxyYQOtksBxx30672433 = -560171919;    int faAwxyYQOtksBxx40183065 = -458672021;    int faAwxyYQOtksBxx71643341 = -145280423;    int faAwxyYQOtksBxx82915661 = -503866510;    int faAwxyYQOtksBxx97954702 = -57820573;    int faAwxyYQOtksBxx63487211 = -142281540;    int faAwxyYQOtksBxx72220329 = 45940840;    int faAwxyYQOtksBxx21029304 = -272217334;    int faAwxyYQOtksBxx55931595 = -871207977;    int faAwxyYQOtksBxx4658728 = -693164569;    int faAwxyYQOtksBxx57444008 = -220431908;    int faAwxyYQOtksBxx22215553 = -453717195;    int faAwxyYQOtksBxx19231396 = -600013051;    int faAwxyYQOtksBxx63342516 = -252470200;    int faAwxyYQOtksBxx57378120 = -329529429;    int faAwxyYQOtksBxx87301946 = -88401087;    int faAwxyYQOtksBxx9013192 = -209397150;    int faAwxyYQOtksBxx46452874 = -145495852;    int faAwxyYQOtksBxx54338951 = -133819427;    int faAwxyYQOtksBxx68708394 = -956846990;    int faAwxyYQOtksBxx54670900 = -228666405;    int faAwxyYQOtksBxx26724202 = -985298696;    int faAwxyYQOtksBxx18681467 = -594391070;    int faAwxyYQOtksBxx80002453 = -59273651;    int faAwxyYQOtksBxx50135766 = -315604216;    int faAwxyYQOtksBxx83692553 = -618965685;    int faAwxyYQOtksBxx40386897 = -823211375;    int faAwxyYQOtksBxx72248774 = -876868847;    int faAwxyYQOtksBxx97075027 = -786900361;    int faAwxyYQOtksBxx25657403 = -190009674;    int faAwxyYQOtksBxx58756745 = -988753255;    int faAwxyYQOtksBxx89395249 = -233225217;    int faAwxyYQOtksBxx76886798 = -220104783;    int faAwxyYQOtksBxx38060757 = 84634980;    int faAwxyYQOtksBxx40885085 = -726779029;    int faAwxyYQOtksBxx58330960 = -954914902;    int faAwxyYQOtksBxx22151675 = -312215034;    int faAwxyYQOtksBxx12860697 = -87049232;    int faAwxyYQOtksBxx24760641 = -333186537;    int faAwxyYQOtksBxx29033286 = -190882927;    int faAwxyYQOtksBxx47522232 = -484172043;    int faAwxyYQOtksBxx88801372 = -650579914;    int faAwxyYQOtksBxx93645797 = -644877039;    int faAwxyYQOtksBxx46841942 = -819567338;    int faAwxyYQOtksBxx24724017 = -273262700;    int faAwxyYQOtksBxx85313124 = -41159828;    int faAwxyYQOtksBxx30382273 = 35631714;    int faAwxyYQOtksBxx159211 = -866640987;    int faAwxyYQOtksBxx44150205 = -454987169;    int faAwxyYQOtksBxx50802946 = -746336231;    int faAwxyYQOtksBxx74126900 = -167545403;    int faAwxyYQOtksBxx16777764 = -41186093;    int faAwxyYQOtksBxx91467726 = -69092909;    int faAwxyYQOtksBxx32894527 = 67186543;    int faAwxyYQOtksBxx93763657 = -956569460;     faAwxyYQOtksBxx92645934 = faAwxyYQOtksBxx705272;     faAwxyYQOtksBxx705272 = faAwxyYQOtksBxx5726711;     faAwxyYQOtksBxx5726711 = faAwxyYQOtksBxx34900034;     faAwxyYQOtksBxx34900034 = faAwxyYQOtksBxx55892144;     faAwxyYQOtksBxx55892144 = faAwxyYQOtksBxx81096962;     faAwxyYQOtksBxx81096962 = faAwxyYQOtksBxx4789550;     faAwxyYQOtksBxx4789550 = faAwxyYQOtksBxx69168218;     faAwxyYQOtksBxx69168218 = faAwxyYQOtksBxx46859448;     faAwxyYQOtksBxx46859448 = faAwxyYQOtksBxx88450955;     faAwxyYQOtksBxx88450955 = faAwxyYQOtksBxx64501575;     faAwxyYQOtksBxx64501575 = faAwxyYQOtksBxx76301594;     faAwxyYQOtksBxx76301594 = faAwxyYQOtksBxx70182136;     faAwxyYQOtksBxx70182136 = faAwxyYQOtksBxx62170478;     faAwxyYQOtksBxx62170478 = faAwxyYQOtksBxx57664443;     faAwxyYQOtksBxx57664443 = faAwxyYQOtksBxx33306222;     faAwxyYQOtksBxx33306222 = faAwxyYQOtksBxx3888624;     faAwxyYQOtksBxx3888624 = faAwxyYQOtksBxx73777099;     faAwxyYQOtksBxx73777099 = faAwxyYQOtksBxx35179919;     faAwxyYQOtksBxx35179919 = faAwxyYQOtksBxx8797495;     faAwxyYQOtksBxx8797495 = faAwxyYQOtksBxx21987382;     faAwxyYQOtksBxx21987382 = faAwxyYQOtksBxx35201927;     faAwxyYQOtksBxx35201927 = faAwxyYQOtksBxx78489719;     faAwxyYQOtksBxx78489719 = faAwxyYQOtksBxx86495314;     faAwxyYQOtksBxx86495314 = faAwxyYQOtksBxx71557518;     faAwxyYQOtksBxx71557518 = faAwxyYQOtksBxx98514024;     faAwxyYQOtksBxx98514024 = faAwxyYQOtksBxx93795016;     faAwxyYQOtksBxx93795016 = faAwxyYQOtksBxx95776358;     faAwxyYQOtksBxx95776358 = faAwxyYQOtksBxx22715344;     faAwxyYQOtksBxx22715344 = faAwxyYQOtksBxx92520496;     faAwxyYQOtksBxx92520496 = faAwxyYQOtksBxx19742562;     faAwxyYQOtksBxx19742562 = faAwxyYQOtksBxx9830676;     faAwxyYQOtksBxx9830676 = faAwxyYQOtksBxx49577392;     faAwxyYQOtksBxx49577392 = faAwxyYQOtksBxx51500669;     faAwxyYQOtksBxx51500669 = faAwxyYQOtksBxx82168024;     faAwxyYQOtksBxx82168024 = faAwxyYQOtksBxx7528677;     faAwxyYQOtksBxx7528677 = faAwxyYQOtksBxx49613669;     faAwxyYQOtksBxx49613669 = faAwxyYQOtksBxx63501727;     faAwxyYQOtksBxx63501727 = faAwxyYQOtksBxx1528325;     faAwxyYQOtksBxx1528325 = faAwxyYQOtksBxx38104892;     faAwxyYQOtksBxx38104892 = faAwxyYQOtksBxx83140091;     faAwxyYQOtksBxx83140091 = faAwxyYQOtksBxx63230637;     faAwxyYQOtksBxx63230637 = faAwxyYQOtksBxx45806677;     faAwxyYQOtksBxx45806677 = faAwxyYQOtksBxx1602921;     faAwxyYQOtksBxx1602921 = faAwxyYQOtksBxx48434557;     faAwxyYQOtksBxx48434557 = faAwxyYQOtksBxx30672433;     faAwxyYQOtksBxx30672433 = faAwxyYQOtksBxx40183065;     faAwxyYQOtksBxx40183065 = faAwxyYQOtksBxx71643341;     faAwxyYQOtksBxx71643341 = faAwxyYQOtksBxx82915661;     faAwxyYQOtksBxx82915661 = faAwxyYQOtksBxx97954702;     faAwxyYQOtksBxx97954702 = faAwxyYQOtksBxx63487211;     faAwxyYQOtksBxx63487211 = faAwxyYQOtksBxx72220329;     faAwxyYQOtksBxx72220329 = faAwxyYQOtksBxx21029304;     faAwxyYQOtksBxx21029304 = faAwxyYQOtksBxx55931595;     faAwxyYQOtksBxx55931595 = faAwxyYQOtksBxx4658728;     faAwxyYQOtksBxx4658728 = faAwxyYQOtksBxx57444008;     faAwxyYQOtksBxx57444008 = faAwxyYQOtksBxx22215553;     faAwxyYQOtksBxx22215553 = faAwxyYQOtksBxx19231396;     faAwxyYQOtksBxx19231396 = faAwxyYQOtksBxx63342516;     faAwxyYQOtksBxx63342516 = faAwxyYQOtksBxx57378120;     faAwxyYQOtksBxx57378120 = faAwxyYQOtksBxx87301946;     faAwxyYQOtksBxx87301946 = faAwxyYQOtksBxx9013192;     faAwxyYQOtksBxx9013192 = faAwxyYQOtksBxx46452874;     faAwxyYQOtksBxx46452874 = faAwxyYQOtksBxx54338951;     faAwxyYQOtksBxx54338951 = faAwxyYQOtksBxx68708394;     faAwxyYQOtksBxx68708394 = faAwxyYQOtksBxx54670900;     faAwxyYQOtksBxx54670900 = faAwxyYQOtksBxx26724202;     faAwxyYQOtksBxx26724202 = faAwxyYQOtksBxx18681467;     faAwxyYQOtksBxx18681467 = faAwxyYQOtksBxx80002453;     faAwxyYQOtksBxx80002453 = faAwxyYQOtksBxx50135766;     faAwxyYQOtksBxx50135766 = faAwxyYQOtksBxx83692553;     faAwxyYQOtksBxx83692553 = faAwxyYQOtksBxx40386897;     faAwxyYQOtksBxx40386897 = faAwxyYQOtksBxx72248774;     faAwxyYQOtksBxx72248774 = faAwxyYQOtksBxx97075027;     faAwxyYQOtksBxx97075027 = faAwxyYQOtksBxx25657403;     faAwxyYQOtksBxx25657403 = faAwxyYQOtksBxx58756745;     faAwxyYQOtksBxx58756745 = faAwxyYQOtksBxx89395249;     faAwxyYQOtksBxx89395249 = faAwxyYQOtksBxx76886798;     faAwxyYQOtksBxx76886798 = faAwxyYQOtksBxx38060757;     faAwxyYQOtksBxx38060757 = faAwxyYQOtksBxx40885085;     faAwxyYQOtksBxx40885085 = faAwxyYQOtksBxx58330960;     faAwxyYQOtksBxx58330960 = faAwxyYQOtksBxx22151675;     faAwxyYQOtksBxx22151675 = faAwxyYQOtksBxx12860697;     faAwxyYQOtksBxx12860697 = faAwxyYQOtksBxx24760641;     faAwxyYQOtksBxx24760641 = faAwxyYQOtksBxx29033286;     faAwxyYQOtksBxx29033286 = faAwxyYQOtksBxx47522232;     faAwxyYQOtksBxx47522232 = faAwxyYQOtksBxx88801372;     faAwxyYQOtksBxx88801372 = faAwxyYQOtksBxx93645797;     faAwxyYQOtksBxx93645797 = faAwxyYQOtksBxx46841942;     faAwxyYQOtksBxx46841942 = faAwxyYQOtksBxx24724017;     faAwxyYQOtksBxx24724017 = faAwxyYQOtksBxx85313124;     faAwxyYQOtksBxx85313124 = faAwxyYQOtksBxx30382273;     faAwxyYQOtksBxx30382273 = faAwxyYQOtksBxx159211;     faAwxyYQOtksBxx159211 = faAwxyYQOtksBxx44150205;     faAwxyYQOtksBxx44150205 = faAwxyYQOtksBxx50802946;     faAwxyYQOtksBxx50802946 = faAwxyYQOtksBxx74126900;     faAwxyYQOtksBxx74126900 = faAwxyYQOtksBxx16777764;     faAwxyYQOtksBxx16777764 = faAwxyYQOtksBxx91467726;     faAwxyYQOtksBxx91467726 = faAwxyYQOtksBxx32894527;     faAwxyYQOtksBxx32894527 = faAwxyYQOtksBxx93763657;     faAwxyYQOtksBxx93763657 = faAwxyYQOtksBxx92645934;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void osBnpOkgWmkwUKHPgKgeHJoPXDbWD76592916() {     int hsKRYGtWzflirkb59816994 = -600995137;    int hsKRYGtWzflirkb45889298 = -545218900;    int hsKRYGtWzflirkb40064728 = -225218851;    int hsKRYGtWzflirkb2300309 = -377832497;    int hsKRYGtWzflirkb37154754 = -813754373;    int hsKRYGtWzflirkb73538164 = -778572459;    int hsKRYGtWzflirkb28435763 = -359470109;    int hsKRYGtWzflirkb92248717 = -392026918;    int hsKRYGtWzflirkb14238595 = -199102905;    int hsKRYGtWzflirkb68782439 = -818091885;    int hsKRYGtWzflirkb46646247 = -302128747;    int hsKRYGtWzflirkb22572926 = -550402046;    int hsKRYGtWzflirkb23427307 = -280787223;    int hsKRYGtWzflirkb59248405 = -718364952;    int hsKRYGtWzflirkb112111 = 51154902;    int hsKRYGtWzflirkb4227139 = -379156172;    int hsKRYGtWzflirkb69579540 = 55259834;    int hsKRYGtWzflirkb7205549 = -971006083;    int hsKRYGtWzflirkb78382407 = -199970654;    int hsKRYGtWzflirkb36235700 = -606818910;    int hsKRYGtWzflirkb5132567 = -684197564;    int hsKRYGtWzflirkb59203183 = -835448667;    int hsKRYGtWzflirkb33052094 = -599570242;    int hsKRYGtWzflirkb519514 = -62534977;    int hsKRYGtWzflirkb85514224 = -161587542;    int hsKRYGtWzflirkb88561892 = -782960702;    int hsKRYGtWzflirkb79730905 = -479055365;    int hsKRYGtWzflirkb60405422 = -242828620;    int hsKRYGtWzflirkb24375357 = -4672995;    int hsKRYGtWzflirkb55857230 = -680905006;    int hsKRYGtWzflirkb2495960 = -201539808;    int hsKRYGtWzflirkb5368463 = -191278669;    int hsKRYGtWzflirkb64514769 = -151199175;    int hsKRYGtWzflirkb29242476 = -20089857;    int hsKRYGtWzflirkb87319061 = -395992988;    int hsKRYGtWzflirkb68365000 = -871414532;    int hsKRYGtWzflirkb62020754 = -263745123;    int hsKRYGtWzflirkb19503511 = -829093584;    int hsKRYGtWzflirkb93482323 = -468632568;    int hsKRYGtWzflirkb40852697 = -279261877;    int hsKRYGtWzflirkb87628702 = -204559703;    int hsKRYGtWzflirkb50209319 = -369719894;    int hsKRYGtWzflirkb22032214 = -226808705;    int hsKRYGtWzflirkb61087431 = -546304729;    int hsKRYGtWzflirkb26357279 = -744113500;    int hsKRYGtWzflirkb94331326 = -838792199;    int hsKRYGtWzflirkb35528568 = -371981846;    int hsKRYGtWzflirkb42363994 = -151743361;    int hsKRYGtWzflirkb88307792 = -457370813;    int hsKRYGtWzflirkb59137805 = -3344020;    int hsKRYGtWzflirkb70205806 = -739677860;    int hsKRYGtWzflirkb71953214 = -123298384;    int hsKRYGtWzflirkb92799990 = 129988;    int hsKRYGtWzflirkb21415824 = -350370626;    int hsKRYGtWzflirkb71460349 = -379927998;    int hsKRYGtWzflirkb613811 = -765546470;    int hsKRYGtWzflirkb12837204 = -945648658;    int hsKRYGtWzflirkb39545215 = -62683875;    int hsKRYGtWzflirkb16786084 = -116244956;    int hsKRYGtWzflirkb48592861 = 69206329;    int hsKRYGtWzflirkb93807259 = -199517094;    int hsKRYGtWzflirkb68030341 = -16641490;    int hsKRYGtWzflirkb67873361 = -287353923;    int hsKRYGtWzflirkb58381365 = -518197900;    int hsKRYGtWzflirkb66286480 = -516552078;    int hsKRYGtWzflirkb41277784 = -10850078;    int hsKRYGtWzflirkb58058157 = -299202871;    int hsKRYGtWzflirkb94184831 = -160697366;    int hsKRYGtWzflirkb71929343 = -222371964;    int hsKRYGtWzflirkb31747111 = -77430567;    int hsKRYGtWzflirkb42206385 = -15411049;    int hsKRYGtWzflirkb50076029 = -115646583;    int hsKRYGtWzflirkb13723225 = -402373516;    int hsKRYGtWzflirkb37529711 = -920708778;    int hsKRYGtWzflirkb48606997 = -302259207;    int hsKRYGtWzflirkb54923248 = -214477670;    int hsKRYGtWzflirkb37170970 = -508639962;    int hsKRYGtWzflirkb71964663 = 46734486;    int hsKRYGtWzflirkb74162235 = -318421477;    int hsKRYGtWzflirkb91182897 = -322795343;    int hsKRYGtWzflirkb53033324 = -310978856;    int hsKRYGtWzflirkb37366912 = -227312005;    int hsKRYGtWzflirkb72097629 = -785457807;    int hsKRYGtWzflirkb65237551 = 98671025;    int hsKRYGtWzflirkb85651424 = -941227146;    int hsKRYGtWzflirkb30542745 = 21758576;    int hsKRYGtWzflirkb12568472 = -91408658;    int hsKRYGtWzflirkb43098945 = -800828550;    int hsKRYGtWzflirkb57782126 = -640161860;    int hsKRYGtWzflirkb86705250 = -630446518;    int hsKRYGtWzflirkb55527796 = -925765875;    int hsKRYGtWzflirkb22475540 = -101061249;    int hsKRYGtWzflirkb2717427 = -612848628;    int hsKRYGtWzflirkb44889463 = -437838897;    int hsKRYGtWzflirkb47045437 = 20255217;    int hsKRYGtWzflirkb19598362 = -87918214;    int hsKRYGtWzflirkb82335957 = 17634029;    int hsKRYGtWzflirkb63650849 = -708610806;    int hsKRYGtWzflirkb94800951 = 70247349;    int hsKRYGtWzflirkb85079494 = -600995137;     hsKRYGtWzflirkb59816994 = hsKRYGtWzflirkb45889298;     hsKRYGtWzflirkb45889298 = hsKRYGtWzflirkb40064728;     hsKRYGtWzflirkb40064728 = hsKRYGtWzflirkb2300309;     hsKRYGtWzflirkb2300309 = hsKRYGtWzflirkb37154754;     hsKRYGtWzflirkb37154754 = hsKRYGtWzflirkb73538164;     hsKRYGtWzflirkb73538164 = hsKRYGtWzflirkb28435763;     hsKRYGtWzflirkb28435763 = hsKRYGtWzflirkb92248717;     hsKRYGtWzflirkb92248717 = hsKRYGtWzflirkb14238595;     hsKRYGtWzflirkb14238595 = hsKRYGtWzflirkb68782439;     hsKRYGtWzflirkb68782439 = hsKRYGtWzflirkb46646247;     hsKRYGtWzflirkb46646247 = hsKRYGtWzflirkb22572926;     hsKRYGtWzflirkb22572926 = hsKRYGtWzflirkb23427307;     hsKRYGtWzflirkb23427307 = hsKRYGtWzflirkb59248405;     hsKRYGtWzflirkb59248405 = hsKRYGtWzflirkb112111;     hsKRYGtWzflirkb112111 = hsKRYGtWzflirkb4227139;     hsKRYGtWzflirkb4227139 = hsKRYGtWzflirkb69579540;     hsKRYGtWzflirkb69579540 = hsKRYGtWzflirkb7205549;     hsKRYGtWzflirkb7205549 = hsKRYGtWzflirkb78382407;     hsKRYGtWzflirkb78382407 = hsKRYGtWzflirkb36235700;     hsKRYGtWzflirkb36235700 = hsKRYGtWzflirkb5132567;     hsKRYGtWzflirkb5132567 = hsKRYGtWzflirkb59203183;     hsKRYGtWzflirkb59203183 = hsKRYGtWzflirkb33052094;     hsKRYGtWzflirkb33052094 = hsKRYGtWzflirkb519514;     hsKRYGtWzflirkb519514 = hsKRYGtWzflirkb85514224;     hsKRYGtWzflirkb85514224 = hsKRYGtWzflirkb88561892;     hsKRYGtWzflirkb88561892 = hsKRYGtWzflirkb79730905;     hsKRYGtWzflirkb79730905 = hsKRYGtWzflirkb60405422;     hsKRYGtWzflirkb60405422 = hsKRYGtWzflirkb24375357;     hsKRYGtWzflirkb24375357 = hsKRYGtWzflirkb55857230;     hsKRYGtWzflirkb55857230 = hsKRYGtWzflirkb2495960;     hsKRYGtWzflirkb2495960 = hsKRYGtWzflirkb5368463;     hsKRYGtWzflirkb5368463 = hsKRYGtWzflirkb64514769;     hsKRYGtWzflirkb64514769 = hsKRYGtWzflirkb29242476;     hsKRYGtWzflirkb29242476 = hsKRYGtWzflirkb87319061;     hsKRYGtWzflirkb87319061 = hsKRYGtWzflirkb68365000;     hsKRYGtWzflirkb68365000 = hsKRYGtWzflirkb62020754;     hsKRYGtWzflirkb62020754 = hsKRYGtWzflirkb19503511;     hsKRYGtWzflirkb19503511 = hsKRYGtWzflirkb93482323;     hsKRYGtWzflirkb93482323 = hsKRYGtWzflirkb40852697;     hsKRYGtWzflirkb40852697 = hsKRYGtWzflirkb87628702;     hsKRYGtWzflirkb87628702 = hsKRYGtWzflirkb50209319;     hsKRYGtWzflirkb50209319 = hsKRYGtWzflirkb22032214;     hsKRYGtWzflirkb22032214 = hsKRYGtWzflirkb61087431;     hsKRYGtWzflirkb61087431 = hsKRYGtWzflirkb26357279;     hsKRYGtWzflirkb26357279 = hsKRYGtWzflirkb94331326;     hsKRYGtWzflirkb94331326 = hsKRYGtWzflirkb35528568;     hsKRYGtWzflirkb35528568 = hsKRYGtWzflirkb42363994;     hsKRYGtWzflirkb42363994 = hsKRYGtWzflirkb88307792;     hsKRYGtWzflirkb88307792 = hsKRYGtWzflirkb59137805;     hsKRYGtWzflirkb59137805 = hsKRYGtWzflirkb70205806;     hsKRYGtWzflirkb70205806 = hsKRYGtWzflirkb71953214;     hsKRYGtWzflirkb71953214 = hsKRYGtWzflirkb92799990;     hsKRYGtWzflirkb92799990 = hsKRYGtWzflirkb21415824;     hsKRYGtWzflirkb21415824 = hsKRYGtWzflirkb71460349;     hsKRYGtWzflirkb71460349 = hsKRYGtWzflirkb613811;     hsKRYGtWzflirkb613811 = hsKRYGtWzflirkb12837204;     hsKRYGtWzflirkb12837204 = hsKRYGtWzflirkb39545215;     hsKRYGtWzflirkb39545215 = hsKRYGtWzflirkb16786084;     hsKRYGtWzflirkb16786084 = hsKRYGtWzflirkb48592861;     hsKRYGtWzflirkb48592861 = hsKRYGtWzflirkb93807259;     hsKRYGtWzflirkb93807259 = hsKRYGtWzflirkb68030341;     hsKRYGtWzflirkb68030341 = hsKRYGtWzflirkb67873361;     hsKRYGtWzflirkb67873361 = hsKRYGtWzflirkb58381365;     hsKRYGtWzflirkb58381365 = hsKRYGtWzflirkb66286480;     hsKRYGtWzflirkb66286480 = hsKRYGtWzflirkb41277784;     hsKRYGtWzflirkb41277784 = hsKRYGtWzflirkb58058157;     hsKRYGtWzflirkb58058157 = hsKRYGtWzflirkb94184831;     hsKRYGtWzflirkb94184831 = hsKRYGtWzflirkb71929343;     hsKRYGtWzflirkb71929343 = hsKRYGtWzflirkb31747111;     hsKRYGtWzflirkb31747111 = hsKRYGtWzflirkb42206385;     hsKRYGtWzflirkb42206385 = hsKRYGtWzflirkb50076029;     hsKRYGtWzflirkb50076029 = hsKRYGtWzflirkb13723225;     hsKRYGtWzflirkb13723225 = hsKRYGtWzflirkb37529711;     hsKRYGtWzflirkb37529711 = hsKRYGtWzflirkb48606997;     hsKRYGtWzflirkb48606997 = hsKRYGtWzflirkb54923248;     hsKRYGtWzflirkb54923248 = hsKRYGtWzflirkb37170970;     hsKRYGtWzflirkb37170970 = hsKRYGtWzflirkb71964663;     hsKRYGtWzflirkb71964663 = hsKRYGtWzflirkb74162235;     hsKRYGtWzflirkb74162235 = hsKRYGtWzflirkb91182897;     hsKRYGtWzflirkb91182897 = hsKRYGtWzflirkb53033324;     hsKRYGtWzflirkb53033324 = hsKRYGtWzflirkb37366912;     hsKRYGtWzflirkb37366912 = hsKRYGtWzflirkb72097629;     hsKRYGtWzflirkb72097629 = hsKRYGtWzflirkb65237551;     hsKRYGtWzflirkb65237551 = hsKRYGtWzflirkb85651424;     hsKRYGtWzflirkb85651424 = hsKRYGtWzflirkb30542745;     hsKRYGtWzflirkb30542745 = hsKRYGtWzflirkb12568472;     hsKRYGtWzflirkb12568472 = hsKRYGtWzflirkb43098945;     hsKRYGtWzflirkb43098945 = hsKRYGtWzflirkb57782126;     hsKRYGtWzflirkb57782126 = hsKRYGtWzflirkb86705250;     hsKRYGtWzflirkb86705250 = hsKRYGtWzflirkb55527796;     hsKRYGtWzflirkb55527796 = hsKRYGtWzflirkb22475540;     hsKRYGtWzflirkb22475540 = hsKRYGtWzflirkb2717427;     hsKRYGtWzflirkb2717427 = hsKRYGtWzflirkb44889463;     hsKRYGtWzflirkb44889463 = hsKRYGtWzflirkb47045437;     hsKRYGtWzflirkb47045437 = hsKRYGtWzflirkb19598362;     hsKRYGtWzflirkb19598362 = hsKRYGtWzflirkb82335957;     hsKRYGtWzflirkb82335957 = hsKRYGtWzflirkb63650849;     hsKRYGtWzflirkb63650849 = hsKRYGtWzflirkb94800951;     hsKRYGtWzflirkb94800951 = hsKRYGtWzflirkb85079494;     hsKRYGtWzflirkb85079494 = hsKRYGtWzflirkb59816994;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void qSjJeqHIAWTNEDRAUUUZByCTTiJUu78782254() {     int xbqpfglHMPUpYHj56329915 = -599795825;    int xbqpfglHMPUpYHj60972123 = -497907202;    int xbqpfglHMPUpYHj95747039 = 26678384;    int xbqpfglHMPUpYHj14849157 = -846646288;    int xbqpfglHMPUpYHj5807144 = -369341368;    int xbqpfglHMPUpYHj10785037 = -809100036;    int xbqpfglHMPUpYHj2250417 = -524388084;    int xbqpfglHMPUpYHj27700888 = -504911102;    int xbqpfglHMPUpYHj18492852 = -451520659;    int xbqpfglHMPUpYHj22786976 = -476817098;    int xbqpfglHMPUpYHj6862363 = -277363984;    int xbqpfglHMPUpYHj37589363 = -19134952;    int xbqpfglHMPUpYHj28698005 = -566572886;    int xbqpfglHMPUpYHj26323518 = -127886487;    int xbqpfglHMPUpYHj5011868 = -900171808;    int xbqpfglHMPUpYHj17183963 = -137777387;    int xbqpfglHMPUpYHj97270388 = -957587211;    int xbqpfglHMPUpYHj43341037 = -488532148;    int xbqpfglHMPUpYHj60305383 = -689097954;    int xbqpfglHMPUpYHj76960959 = -516343740;    int xbqpfglHMPUpYHj12811422 = -894747162;    int xbqpfglHMPUpYHj104941 = -113466359;    int xbqpfglHMPUpYHj22549057 = -880489953;    int xbqpfglHMPUpYHj24344284 = -21652381;    int xbqpfglHMPUpYHj63789231 = -787013301;    int xbqpfglHMPUpYHj64586257 = -962255964;    int xbqpfglHMPUpYHj27627590 = -306237212;    int xbqpfglHMPUpYHj52655784 = -998584719;    int xbqpfglHMPUpYHj6571328 = -970050978;    int xbqpfglHMPUpYHj56111738 = -45681848;    int xbqpfglHMPUpYHj11590039 = -503507961;    int xbqpfglHMPUpYHj41118646 = -523284690;    int xbqpfglHMPUpYHj81959207 = -41295943;    int xbqpfglHMPUpYHj41902144 = -940599857;    int xbqpfglHMPUpYHj96803899 = -610658514;    int xbqpfglHMPUpYHj12036254 = -48491240;    int xbqpfglHMPUpYHj42577721 = -874169081;    int xbqpfglHMPUpYHj92630484 = -650225552;    int xbqpfglHMPUpYHj42216631 = -809701318;    int xbqpfglHMPUpYHj20370096 = -815609759;    int xbqpfglHMPUpYHj19899698 = -736101874;    int xbqpfglHMPUpYHj94402671 = -877160856;    int xbqpfglHMPUpYHj9574150 = -723081277;    int xbqpfglHMPUpYHj73295539 = -687938482;    int xbqpfglHMPUpYHj92343993 = -176267838;    int xbqpfglHMPUpYHj92012352 = -71119995;    int xbqpfglHMPUpYHj32440661 = -650502494;    int xbqpfglHMPUpYHj22854017 = -314837360;    int xbqpfglHMPUpYHj6916861 = 11124992;    int xbqpfglHMPUpYHj31938845 = -352883475;    int xbqpfglHMPUpYHj21994446 = -269057066;    int xbqpfglHMPUpYHj44361853 = -705463634;    int xbqpfglHMPUpYHj84943369 = -791778656;    int xbqpfglHMPUpYHj6923020 = -800571630;    int xbqpfglHMPUpYHj65970833 = -812649790;    int xbqpfglHMPUpYHj56224974 = -386329466;    int xbqpfglHMPUpYHj38423067 = -617417250;    int xbqpfglHMPUpYHj71402755 = -951669236;    int xbqpfglHMPUpYHj51059925 = 40367013;    int xbqpfglHMPUpYHj41220887 = -407085404;    int xbqpfglHMPUpYHj83157447 = -402862824;    int xbqpfglHMPUpYHj49594632 = -525803366;    int xbqpfglHMPUpYHj21129561 = -534860125;    int xbqpfglHMPUpYHj62381114 = -305838811;    int xbqpfglHMPUpYHj11196937 = -973309138;    int xbqpfglHMPUpYHj65743716 = -754079295;    int xbqpfglHMPUpYHj55630155 = -977839009;    int xbqpfglHMPUpYHj86795860 = -625973030;    int xbqpfglHMPUpYHj29519618 = -517227973;    int xbqpfglHMPUpYHj92975614 = -751680569;    int xbqpfglHMPUpYHj74606241 = -263608307;    int xbqpfglHMPUpYHj4639904 = -207361659;    int xbqpfglHMPUpYHj1124407 = -678830830;    int xbqpfglHMPUpYHj39935287 = -873488196;    int xbqpfglHMPUpYHj57061261 = -780241867;    int xbqpfglHMPUpYHj18408751 = 82413694;    int xbqpfglHMPUpYHj90530791 = -390385082;    int xbqpfglHMPUpYHj49253517 = -92551471;    int xbqpfglHMPUpYHj32000290 = -845384544;    int xbqpfglHMPUpYHj71776878 = -615893306;    int xbqpfglHMPUpYHj32145596 = -211753471;    int xbqpfglHMPUpYHj4773574 = -991399853;    int xbqpfglHMPUpYHj45738924 = -909709711;    int xbqpfglHMPUpYHj74632483 = -517167503;    int xbqpfglHMPUpYHj34117292 = -776624782;    int xbqpfglHMPUpYHj67228186 = -798044327;    int xbqpfglHMPUpYHj56175277 = -731506034;    int xbqpfglHMPUpYHj75036187 = -240724314;    int xbqpfglHMPUpYHj75931311 = -27950067;    int xbqpfglHMPUpYHj40578925 = -124329048;    int xbqpfglHMPUpYHj73613186 = -431073991;    int xbqpfglHMPUpYHj71174965 = -922499845;    int xbqpfglHMPUpYHj41570560 = -590592566;    int xbqpfglHMPUpYHj995745 = -302615914;    int xbqpfglHMPUpYHj37212649 = -312746935;    int xbqpfglHMPUpYHj70305066 = -110298508;    int xbqpfglHMPUpYHj73273111 = -242300732;    int xbqpfglHMPUpYHj47193036 = -317242466;    int xbqpfglHMPUpYHj62098603 = -714629345;    int xbqpfglHMPUpYHj26600277 = -599795825;     xbqpfglHMPUpYHj56329915 = xbqpfglHMPUpYHj60972123;     xbqpfglHMPUpYHj60972123 = xbqpfglHMPUpYHj95747039;     xbqpfglHMPUpYHj95747039 = xbqpfglHMPUpYHj14849157;     xbqpfglHMPUpYHj14849157 = xbqpfglHMPUpYHj5807144;     xbqpfglHMPUpYHj5807144 = xbqpfglHMPUpYHj10785037;     xbqpfglHMPUpYHj10785037 = xbqpfglHMPUpYHj2250417;     xbqpfglHMPUpYHj2250417 = xbqpfglHMPUpYHj27700888;     xbqpfglHMPUpYHj27700888 = xbqpfglHMPUpYHj18492852;     xbqpfglHMPUpYHj18492852 = xbqpfglHMPUpYHj22786976;     xbqpfglHMPUpYHj22786976 = xbqpfglHMPUpYHj6862363;     xbqpfglHMPUpYHj6862363 = xbqpfglHMPUpYHj37589363;     xbqpfglHMPUpYHj37589363 = xbqpfglHMPUpYHj28698005;     xbqpfglHMPUpYHj28698005 = xbqpfglHMPUpYHj26323518;     xbqpfglHMPUpYHj26323518 = xbqpfglHMPUpYHj5011868;     xbqpfglHMPUpYHj5011868 = xbqpfglHMPUpYHj17183963;     xbqpfglHMPUpYHj17183963 = xbqpfglHMPUpYHj97270388;     xbqpfglHMPUpYHj97270388 = xbqpfglHMPUpYHj43341037;     xbqpfglHMPUpYHj43341037 = xbqpfglHMPUpYHj60305383;     xbqpfglHMPUpYHj60305383 = xbqpfglHMPUpYHj76960959;     xbqpfglHMPUpYHj76960959 = xbqpfglHMPUpYHj12811422;     xbqpfglHMPUpYHj12811422 = xbqpfglHMPUpYHj104941;     xbqpfglHMPUpYHj104941 = xbqpfglHMPUpYHj22549057;     xbqpfglHMPUpYHj22549057 = xbqpfglHMPUpYHj24344284;     xbqpfglHMPUpYHj24344284 = xbqpfglHMPUpYHj63789231;     xbqpfglHMPUpYHj63789231 = xbqpfglHMPUpYHj64586257;     xbqpfglHMPUpYHj64586257 = xbqpfglHMPUpYHj27627590;     xbqpfglHMPUpYHj27627590 = xbqpfglHMPUpYHj52655784;     xbqpfglHMPUpYHj52655784 = xbqpfglHMPUpYHj6571328;     xbqpfglHMPUpYHj6571328 = xbqpfglHMPUpYHj56111738;     xbqpfglHMPUpYHj56111738 = xbqpfglHMPUpYHj11590039;     xbqpfglHMPUpYHj11590039 = xbqpfglHMPUpYHj41118646;     xbqpfglHMPUpYHj41118646 = xbqpfglHMPUpYHj81959207;     xbqpfglHMPUpYHj81959207 = xbqpfglHMPUpYHj41902144;     xbqpfglHMPUpYHj41902144 = xbqpfglHMPUpYHj96803899;     xbqpfglHMPUpYHj96803899 = xbqpfglHMPUpYHj12036254;     xbqpfglHMPUpYHj12036254 = xbqpfglHMPUpYHj42577721;     xbqpfglHMPUpYHj42577721 = xbqpfglHMPUpYHj92630484;     xbqpfglHMPUpYHj92630484 = xbqpfglHMPUpYHj42216631;     xbqpfglHMPUpYHj42216631 = xbqpfglHMPUpYHj20370096;     xbqpfglHMPUpYHj20370096 = xbqpfglHMPUpYHj19899698;     xbqpfglHMPUpYHj19899698 = xbqpfglHMPUpYHj94402671;     xbqpfglHMPUpYHj94402671 = xbqpfglHMPUpYHj9574150;     xbqpfglHMPUpYHj9574150 = xbqpfglHMPUpYHj73295539;     xbqpfglHMPUpYHj73295539 = xbqpfglHMPUpYHj92343993;     xbqpfglHMPUpYHj92343993 = xbqpfglHMPUpYHj92012352;     xbqpfglHMPUpYHj92012352 = xbqpfglHMPUpYHj32440661;     xbqpfglHMPUpYHj32440661 = xbqpfglHMPUpYHj22854017;     xbqpfglHMPUpYHj22854017 = xbqpfglHMPUpYHj6916861;     xbqpfglHMPUpYHj6916861 = xbqpfglHMPUpYHj31938845;     xbqpfglHMPUpYHj31938845 = xbqpfglHMPUpYHj21994446;     xbqpfglHMPUpYHj21994446 = xbqpfglHMPUpYHj44361853;     xbqpfglHMPUpYHj44361853 = xbqpfglHMPUpYHj84943369;     xbqpfglHMPUpYHj84943369 = xbqpfglHMPUpYHj6923020;     xbqpfglHMPUpYHj6923020 = xbqpfglHMPUpYHj65970833;     xbqpfglHMPUpYHj65970833 = xbqpfglHMPUpYHj56224974;     xbqpfglHMPUpYHj56224974 = xbqpfglHMPUpYHj38423067;     xbqpfglHMPUpYHj38423067 = xbqpfglHMPUpYHj71402755;     xbqpfglHMPUpYHj71402755 = xbqpfglHMPUpYHj51059925;     xbqpfglHMPUpYHj51059925 = xbqpfglHMPUpYHj41220887;     xbqpfglHMPUpYHj41220887 = xbqpfglHMPUpYHj83157447;     xbqpfglHMPUpYHj83157447 = xbqpfglHMPUpYHj49594632;     xbqpfglHMPUpYHj49594632 = xbqpfglHMPUpYHj21129561;     xbqpfglHMPUpYHj21129561 = xbqpfglHMPUpYHj62381114;     xbqpfglHMPUpYHj62381114 = xbqpfglHMPUpYHj11196937;     xbqpfglHMPUpYHj11196937 = xbqpfglHMPUpYHj65743716;     xbqpfglHMPUpYHj65743716 = xbqpfglHMPUpYHj55630155;     xbqpfglHMPUpYHj55630155 = xbqpfglHMPUpYHj86795860;     xbqpfglHMPUpYHj86795860 = xbqpfglHMPUpYHj29519618;     xbqpfglHMPUpYHj29519618 = xbqpfglHMPUpYHj92975614;     xbqpfglHMPUpYHj92975614 = xbqpfglHMPUpYHj74606241;     xbqpfglHMPUpYHj74606241 = xbqpfglHMPUpYHj4639904;     xbqpfglHMPUpYHj4639904 = xbqpfglHMPUpYHj1124407;     xbqpfglHMPUpYHj1124407 = xbqpfglHMPUpYHj39935287;     xbqpfglHMPUpYHj39935287 = xbqpfglHMPUpYHj57061261;     xbqpfglHMPUpYHj57061261 = xbqpfglHMPUpYHj18408751;     xbqpfglHMPUpYHj18408751 = xbqpfglHMPUpYHj90530791;     xbqpfglHMPUpYHj90530791 = xbqpfglHMPUpYHj49253517;     xbqpfglHMPUpYHj49253517 = xbqpfglHMPUpYHj32000290;     xbqpfglHMPUpYHj32000290 = xbqpfglHMPUpYHj71776878;     xbqpfglHMPUpYHj71776878 = xbqpfglHMPUpYHj32145596;     xbqpfglHMPUpYHj32145596 = xbqpfglHMPUpYHj4773574;     xbqpfglHMPUpYHj4773574 = xbqpfglHMPUpYHj45738924;     xbqpfglHMPUpYHj45738924 = xbqpfglHMPUpYHj74632483;     xbqpfglHMPUpYHj74632483 = xbqpfglHMPUpYHj34117292;     xbqpfglHMPUpYHj34117292 = xbqpfglHMPUpYHj67228186;     xbqpfglHMPUpYHj67228186 = xbqpfglHMPUpYHj56175277;     xbqpfglHMPUpYHj56175277 = xbqpfglHMPUpYHj75036187;     xbqpfglHMPUpYHj75036187 = xbqpfglHMPUpYHj75931311;     xbqpfglHMPUpYHj75931311 = xbqpfglHMPUpYHj40578925;     xbqpfglHMPUpYHj40578925 = xbqpfglHMPUpYHj73613186;     xbqpfglHMPUpYHj73613186 = xbqpfglHMPUpYHj71174965;     xbqpfglHMPUpYHj71174965 = xbqpfglHMPUpYHj41570560;     xbqpfglHMPUpYHj41570560 = xbqpfglHMPUpYHj995745;     xbqpfglHMPUpYHj995745 = xbqpfglHMPUpYHj37212649;     xbqpfglHMPUpYHj37212649 = xbqpfglHMPUpYHj70305066;     xbqpfglHMPUpYHj70305066 = xbqpfglHMPUpYHj73273111;     xbqpfglHMPUpYHj73273111 = xbqpfglHMPUpYHj47193036;     xbqpfglHMPUpYHj47193036 = xbqpfglHMPUpYHj62098603;     xbqpfglHMPUpYHj62098603 = xbqpfglHMPUpYHj26600277;     xbqpfglHMPUpYHj26600277 = xbqpfglHMPUpYHj56329915;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void rOcPkeIubaCPaZbSjyQEMnkPpoqCz80971593() {     int JhoJqwKgJBygJSX52842836 = -598596513;    int JhoJqwKgJBygJSX76054949 = -450595504;    int JhoJqwKgJBygJSX51429350 = -821424382;    int JhoJqwKgJBygJSX27398005 = -215460078;    int JhoJqwKgJBygJSX74459533 = 75071636;    int JhoJqwKgJBygJSX48031909 = -839627612;    int JhoJqwKgJBygJSX76065069 = -689306059;    int JhoJqwKgJBygJSX63153058 = -617795286;    int JhoJqwKgJBygJSX22747109 = -703938413;    int JhoJqwKgJBygJSX76791511 = -135542311;    int JhoJqwKgJBygJSX67078478 = -252599221;    int JhoJqwKgJBygJSX52605800 = -587867857;    int JhoJqwKgJBygJSX33968702 = -852358550;    int JhoJqwKgJBygJSX93398631 = -637408022;    int JhoJqwKgJBygJSX9911625 = -751498518;    int JhoJqwKgJBygJSX30140786 = -996398602;    int JhoJqwKgJBygJSX24961237 = -870434256;    int JhoJqwKgJBygJSX79476524 = -6058213;    int JhoJqwKgJBygJSX42228360 = -78225254;    int JhoJqwKgJBygJSX17686219 = -425868571;    int JhoJqwKgJBygJSX20490277 = -5296760;    int JhoJqwKgJBygJSX41006698 = -491484052;    int JhoJqwKgJBygJSX12046019 = -61409663;    int JhoJqwKgJBygJSX48169054 = 19230215;    int JhoJqwKgJBygJSX42064239 = -312439060;    int JhoJqwKgJBygJSX40610622 = -41551227;    int JhoJqwKgJBygJSX75524274 = -133419059;    int JhoJqwKgJBygJSX44906147 = -654340818;    int JhoJqwKgJBygJSX88767298 = -835428961;    int JhoJqwKgJBygJSX56366245 = -510458690;    int JhoJqwKgJBygJSX20684119 = -805476114;    int JhoJqwKgJBygJSX76868829 = -855290710;    int JhoJqwKgJBygJSX99403646 = 68607289;    int JhoJqwKgJBygJSX54561812 = -761109857;    int JhoJqwKgJBygJSX6288739 = -825324039;    int JhoJqwKgJBygJSX55707507 = -325567949;    int JhoJqwKgJBygJSX23134688 = -384593039;    int JhoJqwKgJBygJSX65757459 = -471357521;    int JhoJqwKgJBygJSX90950937 = -50770069;    int JhoJqwKgJBygJSX99887495 = -251957641;    int JhoJqwKgJBygJSX52170693 = -167644045;    int JhoJqwKgJBygJSX38596024 = -284601819;    int JhoJqwKgJBygJSX97116085 = -119353849;    int JhoJqwKgJBygJSX85503647 = -829572236;    int JhoJqwKgJBygJSX58330709 = -708422175;    int JhoJqwKgJBygJSX89693379 = -403447791;    int JhoJqwKgJBygJSX29352754 = -929023142;    int JhoJqwKgJBygJSX3344040 = -477931359;    int JhoJqwKgJBygJSX25525929 = -620379203;    int JhoJqwKgJBygJSX4739884 = -702422931;    int JhoJqwKgJBygJSX73783086 = -898436273;    int JhoJqwKgJBygJSX16770492 = -187628885;    int JhoJqwKgJBygJSX77086747 = -483687301;    int JhoJqwKgJBygJSX92430215 = -150772634;    int JhoJqwKgJBygJSX60481316 = -145371583;    int JhoJqwKgJBygJSX11836138 = -7112462;    int JhoJqwKgJBygJSX64008930 = -289185841;    int JhoJqwKgJBygJSX3260296 = -740654597;    int JhoJqwKgJBygJSX85333766 = -903021018;    int JhoJqwKgJBygJSX33848912 = -883377138;    int JhoJqwKgJBygJSX72507634 = -606208553;    int JhoJqwKgJBygJSX31158923 = 65034758;    int JhoJqwKgJBygJSX74385759 = -782366326;    int JhoJqwKgJBygJSX66380863 = -93479723;    int JhoJqwKgJBygJSX56107393 = -330066197;    int JhoJqwKgJBygJSX90209648 = -397308512;    int JhoJqwKgJBygJSX53202153 = -556475147;    int JhoJqwKgJBygJSX79406890 = 8751307;    int JhoJqwKgJBygJSX87109892 = -812083983;    int JhoJqwKgJBygJSX54204117 = -325930570;    int JhoJqwKgJBygJSX7006099 = -511805564;    int JhoJqwKgJBygJSX59203778 = -299076735;    int JhoJqwKgJBygJSX88525587 = -955288145;    int JhoJqwKgJBygJSX42340864 = -826267614;    int JhoJqwKgJBygJSX65515526 = -158224527;    int JhoJqwKgJBygJSX81894253 = -720694942;    int JhoJqwKgJBygJSX43890612 = -272130203;    int JhoJqwKgJBygJSX26542372 = -231837428;    int JhoJqwKgJBygJSX89838345 = -272347610;    int JhoJqwKgJBygJSX52370859 = -908991270;    int JhoJqwKgJBygJSX11257868 = -112528085;    int JhoJqwKgJBygJSX72180234 = -655487700;    int JhoJqwKgJBygJSX19380219 = 66038385;    int JhoJqwKgJBygJSX84027415 = -33006031;    int JhoJqwKgJBygJSX82583159 = -612022418;    int JhoJqwKgJBygJSX3913627 = -517847230;    int JhoJqwKgJBygJSX99782081 = -271603410;    int JhoJqwKgJBygJSX6973431 = -780620078;    int JhoJqwKgJBygJSX94080495 = -515738275;    int JhoJqwKgJBygJSX94452600 = -718211578;    int JhoJqwKgJBygJSX91698577 = 63617892;    int JhoJqwKgJBygJSX19874392 = -643938442;    int JhoJqwKgJBygJSX80423692 = -568336503;    int JhoJqwKgJBygJSX57102025 = -167392932;    int JhoJqwKgJBygJSX27379861 = -645749088;    int JhoJqwKgJBygJSX21011771 = -132678803;    int JhoJqwKgJBygJSX64210264 = -502235493;    int JhoJqwKgJBygJSX30735223 = 74125873;    int JhoJqwKgJBygJSX29396254 = -399506039;    int JhoJqwKgJBygJSX68121060 = -598596513;     JhoJqwKgJBygJSX52842836 = JhoJqwKgJBygJSX76054949;     JhoJqwKgJBygJSX76054949 = JhoJqwKgJBygJSX51429350;     JhoJqwKgJBygJSX51429350 = JhoJqwKgJBygJSX27398005;     JhoJqwKgJBygJSX27398005 = JhoJqwKgJBygJSX74459533;     JhoJqwKgJBygJSX74459533 = JhoJqwKgJBygJSX48031909;     JhoJqwKgJBygJSX48031909 = JhoJqwKgJBygJSX76065069;     JhoJqwKgJBygJSX76065069 = JhoJqwKgJBygJSX63153058;     JhoJqwKgJBygJSX63153058 = JhoJqwKgJBygJSX22747109;     JhoJqwKgJBygJSX22747109 = JhoJqwKgJBygJSX76791511;     JhoJqwKgJBygJSX76791511 = JhoJqwKgJBygJSX67078478;     JhoJqwKgJBygJSX67078478 = JhoJqwKgJBygJSX52605800;     JhoJqwKgJBygJSX52605800 = JhoJqwKgJBygJSX33968702;     JhoJqwKgJBygJSX33968702 = JhoJqwKgJBygJSX93398631;     JhoJqwKgJBygJSX93398631 = JhoJqwKgJBygJSX9911625;     JhoJqwKgJBygJSX9911625 = JhoJqwKgJBygJSX30140786;     JhoJqwKgJBygJSX30140786 = JhoJqwKgJBygJSX24961237;     JhoJqwKgJBygJSX24961237 = JhoJqwKgJBygJSX79476524;     JhoJqwKgJBygJSX79476524 = JhoJqwKgJBygJSX42228360;     JhoJqwKgJBygJSX42228360 = JhoJqwKgJBygJSX17686219;     JhoJqwKgJBygJSX17686219 = JhoJqwKgJBygJSX20490277;     JhoJqwKgJBygJSX20490277 = JhoJqwKgJBygJSX41006698;     JhoJqwKgJBygJSX41006698 = JhoJqwKgJBygJSX12046019;     JhoJqwKgJBygJSX12046019 = JhoJqwKgJBygJSX48169054;     JhoJqwKgJBygJSX48169054 = JhoJqwKgJBygJSX42064239;     JhoJqwKgJBygJSX42064239 = JhoJqwKgJBygJSX40610622;     JhoJqwKgJBygJSX40610622 = JhoJqwKgJBygJSX75524274;     JhoJqwKgJBygJSX75524274 = JhoJqwKgJBygJSX44906147;     JhoJqwKgJBygJSX44906147 = JhoJqwKgJBygJSX88767298;     JhoJqwKgJBygJSX88767298 = JhoJqwKgJBygJSX56366245;     JhoJqwKgJBygJSX56366245 = JhoJqwKgJBygJSX20684119;     JhoJqwKgJBygJSX20684119 = JhoJqwKgJBygJSX76868829;     JhoJqwKgJBygJSX76868829 = JhoJqwKgJBygJSX99403646;     JhoJqwKgJBygJSX99403646 = JhoJqwKgJBygJSX54561812;     JhoJqwKgJBygJSX54561812 = JhoJqwKgJBygJSX6288739;     JhoJqwKgJBygJSX6288739 = JhoJqwKgJBygJSX55707507;     JhoJqwKgJBygJSX55707507 = JhoJqwKgJBygJSX23134688;     JhoJqwKgJBygJSX23134688 = JhoJqwKgJBygJSX65757459;     JhoJqwKgJBygJSX65757459 = JhoJqwKgJBygJSX90950937;     JhoJqwKgJBygJSX90950937 = JhoJqwKgJBygJSX99887495;     JhoJqwKgJBygJSX99887495 = JhoJqwKgJBygJSX52170693;     JhoJqwKgJBygJSX52170693 = JhoJqwKgJBygJSX38596024;     JhoJqwKgJBygJSX38596024 = JhoJqwKgJBygJSX97116085;     JhoJqwKgJBygJSX97116085 = JhoJqwKgJBygJSX85503647;     JhoJqwKgJBygJSX85503647 = JhoJqwKgJBygJSX58330709;     JhoJqwKgJBygJSX58330709 = JhoJqwKgJBygJSX89693379;     JhoJqwKgJBygJSX89693379 = JhoJqwKgJBygJSX29352754;     JhoJqwKgJBygJSX29352754 = JhoJqwKgJBygJSX3344040;     JhoJqwKgJBygJSX3344040 = JhoJqwKgJBygJSX25525929;     JhoJqwKgJBygJSX25525929 = JhoJqwKgJBygJSX4739884;     JhoJqwKgJBygJSX4739884 = JhoJqwKgJBygJSX73783086;     JhoJqwKgJBygJSX73783086 = JhoJqwKgJBygJSX16770492;     JhoJqwKgJBygJSX16770492 = JhoJqwKgJBygJSX77086747;     JhoJqwKgJBygJSX77086747 = JhoJqwKgJBygJSX92430215;     JhoJqwKgJBygJSX92430215 = JhoJqwKgJBygJSX60481316;     JhoJqwKgJBygJSX60481316 = JhoJqwKgJBygJSX11836138;     JhoJqwKgJBygJSX11836138 = JhoJqwKgJBygJSX64008930;     JhoJqwKgJBygJSX64008930 = JhoJqwKgJBygJSX3260296;     JhoJqwKgJBygJSX3260296 = JhoJqwKgJBygJSX85333766;     JhoJqwKgJBygJSX85333766 = JhoJqwKgJBygJSX33848912;     JhoJqwKgJBygJSX33848912 = JhoJqwKgJBygJSX72507634;     JhoJqwKgJBygJSX72507634 = JhoJqwKgJBygJSX31158923;     JhoJqwKgJBygJSX31158923 = JhoJqwKgJBygJSX74385759;     JhoJqwKgJBygJSX74385759 = JhoJqwKgJBygJSX66380863;     JhoJqwKgJBygJSX66380863 = JhoJqwKgJBygJSX56107393;     JhoJqwKgJBygJSX56107393 = JhoJqwKgJBygJSX90209648;     JhoJqwKgJBygJSX90209648 = JhoJqwKgJBygJSX53202153;     JhoJqwKgJBygJSX53202153 = JhoJqwKgJBygJSX79406890;     JhoJqwKgJBygJSX79406890 = JhoJqwKgJBygJSX87109892;     JhoJqwKgJBygJSX87109892 = JhoJqwKgJBygJSX54204117;     JhoJqwKgJBygJSX54204117 = JhoJqwKgJBygJSX7006099;     JhoJqwKgJBygJSX7006099 = JhoJqwKgJBygJSX59203778;     JhoJqwKgJBygJSX59203778 = JhoJqwKgJBygJSX88525587;     JhoJqwKgJBygJSX88525587 = JhoJqwKgJBygJSX42340864;     JhoJqwKgJBygJSX42340864 = JhoJqwKgJBygJSX65515526;     JhoJqwKgJBygJSX65515526 = JhoJqwKgJBygJSX81894253;     JhoJqwKgJBygJSX81894253 = JhoJqwKgJBygJSX43890612;     JhoJqwKgJBygJSX43890612 = JhoJqwKgJBygJSX26542372;     JhoJqwKgJBygJSX26542372 = JhoJqwKgJBygJSX89838345;     JhoJqwKgJBygJSX89838345 = JhoJqwKgJBygJSX52370859;     JhoJqwKgJBygJSX52370859 = JhoJqwKgJBygJSX11257868;     JhoJqwKgJBygJSX11257868 = JhoJqwKgJBygJSX72180234;     JhoJqwKgJBygJSX72180234 = JhoJqwKgJBygJSX19380219;     JhoJqwKgJBygJSX19380219 = JhoJqwKgJBygJSX84027415;     JhoJqwKgJBygJSX84027415 = JhoJqwKgJBygJSX82583159;     JhoJqwKgJBygJSX82583159 = JhoJqwKgJBygJSX3913627;     JhoJqwKgJBygJSX3913627 = JhoJqwKgJBygJSX99782081;     JhoJqwKgJBygJSX99782081 = JhoJqwKgJBygJSX6973431;     JhoJqwKgJBygJSX6973431 = JhoJqwKgJBygJSX94080495;     JhoJqwKgJBygJSX94080495 = JhoJqwKgJBygJSX94452600;     JhoJqwKgJBygJSX94452600 = JhoJqwKgJBygJSX91698577;     JhoJqwKgJBygJSX91698577 = JhoJqwKgJBygJSX19874392;     JhoJqwKgJBygJSX19874392 = JhoJqwKgJBygJSX80423692;     JhoJqwKgJBygJSX80423692 = JhoJqwKgJBygJSX57102025;     JhoJqwKgJBygJSX57102025 = JhoJqwKgJBygJSX27379861;     JhoJqwKgJBygJSX27379861 = JhoJqwKgJBygJSX21011771;     JhoJqwKgJBygJSX21011771 = JhoJqwKgJBygJSX64210264;     JhoJqwKgJBygJSX64210264 = JhoJqwKgJBygJSX30735223;     JhoJqwKgJBygJSX30735223 = JhoJqwKgJBygJSX29396254;     JhoJqwKgJBygJSX29396254 = JhoJqwKgJBygJSX68121060;     JhoJqwKgJBygJSX68121060 = JhoJqwKgJBygJSX52842836;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FSzTtSWfrIijigkNuSEanvlzDdXuu83160931() {     int pWaxhvuqZUoclaD49355757 = -597397201;    int pWaxhvuqZUoclaD91137774 = -403283806;    int pWaxhvuqZUoclaD7111661 = -569527148;    int pWaxhvuqZUoclaD39946853 = -684273869;    int pWaxhvuqZUoclaD43111923 = -580515360;    int pWaxhvuqZUoclaD85278781 = -870155188;    int pWaxhvuqZUoclaD49879722 = -854224034;    int pWaxhvuqZUoclaD98605228 = -730679470;    int pWaxhvuqZUoclaD27001365 = -956356167;    int pWaxhvuqZUoclaD30796048 = -894267524;    int pWaxhvuqZUoclaD27294593 = -227834458;    int pWaxhvuqZUoclaD67622237 = -56600763;    int pWaxhvuqZUoclaD39239400 = -38144213;    int pWaxhvuqZUoclaD60473744 = -46929557;    int pWaxhvuqZUoclaD14811382 = -602825228;    int pWaxhvuqZUoclaD43097610 = -755019818;    int pWaxhvuqZUoclaD52652086 = -783281300;    int pWaxhvuqZUoclaD15612013 = -623584279;    int pWaxhvuqZUoclaD24151336 = -567352555;    int pWaxhvuqZUoclaD58411477 = -335393401;    int pWaxhvuqZUoclaD28169132 = -215846358;    int pWaxhvuqZUoclaD81908455 = -869501745;    int pWaxhvuqZUoclaD1542982 = -342329374;    int pWaxhvuqZUoclaD71993825 = 60112811;    int pWaxhvuqZUoclaD20339246 = -937864820;    int pWaxhvuqZUoclaD16634986 = -220846490;    int pWaxhvuqZUoclaD23420959 = 39399094;    int pWaxhvuqZUoclaD37156510 = -310096917;    int pWaxhvuqZUoclaD70963270 = -700806944;    int pWaxhvuqZUoclaD56620753 = -975235533;    int pWaxhvuqZUoclaD29778198 = -7444268;    int pWaxhvuqZUoclaD12619013 = -87296730;    int pWaxhvuqZUoclaD16848085 = -921489479;    int pWaxhvuqZUoclaD67221480 = -581619856;    int pWaxhvuqZUoclaD15773577 = 60010435;    int pWaxhvuqZUoclaD99378761 = -602644657;    int pWaxhvuqZUoclaD3691655 = -995016996;    int pWaxhvuqZUoclaD38884433 = -292489490;    int pWaxhvuqZUoclaD39685244 = -391838819;    int pWaxhvuqZUoclaD79404894 = -788305523;    int pWaxhvuqZUoclaD84441687 = -699186215;    int pWaxhvuqZUoclaD82789375 = -792042781;    int pWaxhvuqZUoclaD84658021 = -615626422;    int pWaxhvuqZUoclaD97711755 = -971205990;    int pWaxhvuqZUoclaD24317424 = -140576513;    int pWaxhvuqZUoclaD87374406 = -735775587;    int pWaxhvuqZUoclaD26264846 = -107543791;    int pWaxhvuqZUoclaD83834063 = -641025358;    int pWaxhvuqZUoclaD44134997 = -151883398;    int pWaxhvuqZUoclaD77540922 = 48037614;    int pWaxhvuqZUoclaD25571727 = -427815479;    int pWaxhvuqZUoclaD89179129 = -769794135;    int pWaxhvuqZUoclaD69230126 = -175595945;    int pWaxhvuqZUoclaD77937411 = -600973638;    int pWaxhvuqZUoclaD54991800 = -578093375;    int pWaxhvuqZUoclaD67447301 = -727895457;    int pWaxhvuqZUoclaD89594792 = 39045567;    int pWaxhvuqZUoclaD35117836 = -529639959;    int pWaxhvuqZUoclaD19607607 = -746409050;    int pWaxhvuqZUoclaD26476938 = -259668871;    int pWaxhvuqZUoclaD61857822 = -809554283;    int pWaxhvuqZUoclaD12723213 = -444127118;    int pWaxhvuqZUoclaD27641959 = 70127473;    int pWaxhvuqZUoclaD70380612 = -981120634;    int pWaxhvuqZUoclaD1017851 = -786823257;    int pWaxhvuqZUoclaD14675581 = -40537728;    int pWaxhvuqZUoclaD50774152 = -135111285;    int pWaxhvuqZUoclaD72017919 = -456524357;    int pWaxhvuqZUoclaD44700167 = -6939992;    int pWaxhvuqZUoclaD15432621 = 99819428;    int pWaxhvuqZUoclaD39405955 = -760002822;    int pWaxhvuqZUoclaD13767653 = -390791811;    int pWaxhvuqZUoclaD75926769 = -131745460;    int pWaxhvuqZUoclaD44746441 = -779047032;    int pWaxhvuqZUoclaD73969790 = -636207186;    int pWaxhvuqZUoclaD45379756 = -423803577;    int pWaxhvuqZUoclaD97250433 = -153875323;    int pWaxhvuqZUoclaD3831226 = -371123385;    int pWaxhvuqZUoclaD47676401 = -799310677;    int pWaxhvuqZUoclaD32964840 = -102089233;    int pWaxhvuqZUoclaD90370139 = -13302700;    int pWaxhvuqZUoclaD39586896 = -319575548;    int pWaxhvuqZUoclaD93021512 = -58213519;    int pWaxhvuqZUoclaD93422347 = -648844559;    int pWaxhvuqZUoclaD31049027 = -447420054;    int pWaxhvuqZUoclaD40599068 = -237650133;    int pWaxhvuqZUoclaD43388886 = -911700786;    int pWaxhvuqZUoclaD38910674 = -220515841;    int pWaxhvuqZUoclaD12229681 = 96473518;    int pWaxhvuqZUoclaD48326275 = -212094109;    int pWaxhvuqZUoclaD9783969 = -541690224;    int pWaxhvuqZUoclaD68573818 = -365377038;    int pWaxhvuqZUoclaD19276826 = -546080441;    int pWaxhvuqZUoclaD13208307 = -32169949;    int pWaxhvuqZUoclaD17547073 = -978751240;    int pWaxhvuqZUoclaD71718474 = -155059097;    int pWaxhvuqZUoclaD55147417 = -762170255;    int pWaxhvuqZUoclaD14277410 = -634505788;    int pWaxhvuqZUoclaD96693905 = -84382734;    int pWaxhvuqZUoclaD9641843 = -597397201;     pWaxhvuqZUoclaD49355757 = pWaxhvuqZUoclaD91137774;     pWaxhvuqZUoclaD91137774 = pWaxhvuqZUoclaD7111661;     pWaxhvuqZUoclaD7111661 = pWaxhvuqZUoclaD39946853;     pWaxhvuqZUoclaD39946853 = pWaxhvuqZUoclaD43111923;     pWaxhvuqZUoclaD43111923 = pWaxhvuqZUoclaD85278781;     pWaxhvuqZUoclaD85278781 = pWaxhvuqZUoclaD49879722;     pWaxhvuqZUoclaD49879722 = pWaxhvuqZUoclaD98605228;     pWaxhvuqZUoclaD98605228 = pWaxhvuqZUoclaD27001365;     pWaxhvuqZUoclaD27001365 = pWaxhvuqZUoclaD30796048;     pWaxhvuqZUoclaD30796048 = pWaxhvuqZUoclaD27294593;     pWaxhvuqZUoclaD27294593 = pWaxhvuqZUoclaD67622237;     pWaxhvuqZUoclaD67622237 = pWaxhvuqZUoclaD39239400;     pWaxhvuqZUoclaD39239400 = pWaxhvuqZUoclaD60473744;     pWaxhvuqZUoclaD60473744 = pWaxhvuqZUoclaD14811382;     pWaxhvuqZUoclaD14811382 = pWaxhvuqZUoclaD43097610;     pWaxhvuqZUoclaD43097610 = pWaxhvuqZUoclaD52652086;     pWaxhvuqZUoclaD52652086 = pWaxhvuqZUoclaD15612013;     pWaxhvuqZUoclaD15612013 = pWaxhvuqZUoclaD24151336;     pWaxhvuqZUoclaD24151336 = pWaxhvuqZUoclaD58411477;     pWaxhvuqZUoclaD58411477 = pWaxhvuqZUoclaD28169132;     pWaxhvuqZUoclaD28169132 = pWaxhvuqZUoclaD81908455;     pWaxhvuqZUoclaD81908455 = pWaxhvuqZUoclaD1542982;     pWaxhvuqZUoclaD1542982 = pWaxhvuqZUoclaD71993825;     pWaxhvuqZUoclaD71993825 = pWaxhvuqZUoclaD20339246;     pWaxhvuqZUoclaD20339246 = pWaxhvuqZUoclaD16634986;     pWaxhvuqZUoclaD16634986 = pWaxhvuqZUoclaD23420959;     pWaxhvuqZUoclaD23420959 = pWaxhvuqZUoclaD37156510;     pWaxhvuqZUoclaD37156510 = pWaxhvuqZUoclaD70963270;     pWaxhvuqZUoclaD70963270 = pWaxhvuqZUoclaD56620753;     pWaxhvuqZUoclaD56620753 = pWaxhvuqZUoclaD29778198;     pWaxhvuqZUoclaD29778198 = pWaxhvuqZUoclaD12619013;     pWaxhvuqZUoclaD12619013 = pWaxhvuqZUoclaD16848085;     pWaxhvuqZUoclaD16848085 = pWaxhvuqZUoclaD67221480;     pWaxhvuqZUoclaD67221480 = pWaxhvuqZUoclaD15773577;     pWaxhvuqZUoclaD15773577 = pWaxhvuqZUoclaD99378761;     pWaxhvuqZUoclaD99378761 = pWaxhvuqZUoclaD3691655;     pWaxhvuqZUoclaD3691655 = pWaxhvuqZUoclaD38884433;     pWaxhvuqZUoclaD38884433 = pWaxhvuqZUoclaD39685244;     pWaxhvuqZUoclaD39685244 = pWaxhvuqZUoclaD79404894;     pWaxhvuqZUoclaD79404894 = pWaxhvuqZUoclaD84441687;     pWaxhvuqZUoclaD84441687 = pWaxhvuqZUoclaD82789375;     pWaxhvuqZUoclaD82789375 = pWaxhvuqZUoclaD84658021;     pWaxhvuqZUoclaD84658021 = pWaxhvuqZUoclaD97711755;     pWaxhvuqZUoclaD97711755 = pWaxhvuqZUoclaD24317424;     pWaxhvuqZUoclaD24317424 = pWaxhvuqZUoclaD87374406;     pWaxhvuqZUoclaD87374406 = pWaxhvuqZUoclaD26264846;     pWaxhvuqZUoclaD26264846 = pWaxhvuqZUoclaD83834063;     pWaxhvuqZUoclaD83834063 = pWaxhvuqZUoclaD44134997;     pWaxhvuqZUoclaD44134997 = pWaxhvuqZUoclaD77540922;     pWaxhvuqZUoclaD77540922 = pWaxhvuqZUoclaD25571727;     pWaxhvuqZUoclaD25571727 = pWaxhvuqZUoclaD89179129;     pWaxhvuqZUoclaD89179129 = pWaxhvuqZUoclaD69230126;     pWaxhvuqZUoclaD69230126 = pWaxhvuqZUoclaD77937411;     pWaxhvuqZUoclaD77937411 = pWaxhvuqZUoclaD54991800;     pWaxhvuqZUoclaD54991800 = pWaxhvuqZUoclaD67447301;     pWaxhvuqZUoclaD67447301 = pWaxhvuqZUoclaD89594792;     pWaxhvuqZUoclaD89594792 = pWaxhvuqZUoclaD35117836;     pWaxhvuqZUoclaD35117836 = pWaxhvuqZUoclaD19607607;     pWaxhvuqZUoclaD19607607 = pWaxhvuqZUoclaD26476938;     pWaxhvuqZUoclaD26476938 = pWaxhvuqZUoclaD61857822;     pWaxhvuqZUoclaD61857822 = pWaxhvuqZUoclaD12723213;     pWaxhvuqZUoclaD12723213 = pWaxhvuqZUoclaD27641959;     pWaxhvuqZUoclaD27641959 = pWaxhvuqZUoclaD70380612;     pWaxhvuqZUoclaD70380612 = pWaxhvuqZUoclaD1017851;     pWaxhvuqZUoclaD1017851 = pWaxhvuqZUoclaD14675581;     pWaxhvuqZUoclaD14675581 = pWaxhvuqZUoclaD50774152;     pWaxhvuqZUoclaD50774152 = pWaxhvuqZUoclaD72017919;     pWaxhvuqZUoclaD72017919 = pWaxhvuqZUoclaD44700167;     pWaxhvuqZUoclaD44700167 = pWaxhvuqZUoclaD15432621;     pWaxhvuqZUoclaD15432621 = pWaxhvuqZUoclaD39405955;     pWaxhvuqZUoclaD39405955 = pWaxhvuqZUoclaD13767653;     pWaxhvuqZUoclaD13767653 = pWaxhvuqZUoclaD75926769;     pWaxhvuqZUoclaD75926769 = pWaxhvuqZUoclaD44746441;     pWaxhvuqZUoclaD44746441 = pWaxhvuqZUoclaD73969790;     pWaxhvuqZUoclaD73969790 = pWaxhvuqZUoclaD45379756;     pWaxhvuqZUoclaD45379756 = pWaxhvuqZUoclaD97250433;     pWaxhvuqZUoclaD97250433 = pWaxhvuqZUoclaD3831226;     pWaxhvuqZUoclaD3831226 = pWaxhvuqZUoclaD47676401;     pWaxhvuqZUoclaD47676401 = pWaxhvuqZUoclaD32964840;     pWaxhvuqZUoclaD32964840 = pWaxhvuqZUoclaD90370139;     pWaxhvuqZUoclaD90370139 = pWaxhvuqZUoclaD39586896;     pWaxhvuqZUoclaD39586896 = pWaxhvuqZUoclaD93021512;     pWaxhvuqZUoclaD93021512 = pWaxhvuqZUoclaD93422347;     pWaxhvuqZUoclaD93422347 = pWaxhvuqZUoclaD31049027;     pWaxhvuqZUoclaD31049027 = pWaxhvuqZUoclaD40599068;     pWaxhvuqZUoclaD40599068 = pWaxhvuqZUoclaD43388886;     pWaxhvuqZUoclaD43388886 = pWaxhvuqZUoclaD38910674;     pWaxhvuqZUoclaD38910674 = pWaxhvuqZUoclaD12229681;     pWaxhvuqZUoclaD12229681 = pWaxhvuqZUoclaD48326275;     pWaxhvuqZUoclaD48326275 = pWaxhvuqZUoclaD9783969;     pWaxhvuqZUoclaD9783969 = pWaxhvuqZUoclaD68573818;     pWaxhvuqZUoclaD68573818 = pWaxhvuqZUoclaD19276826;     pWaxhvuqZUoclaD19276826 = pWaxhvuqZUoclaD13208307;     pWaxhvuqZUoclaD13208307 = pWaxhvuqZUoclaD17547073;     pWaxhvuqZUoclaD17547073 = pWaxhvuqZUoclaD71718474;     pWaxhvuqZUoclaD71718474 = pWaxhvuqZUoclaD55147417;     pWaxhvuqZUoclaD55147417 = pWaxhvuqZUoclaD14277410;     pWaxhvuqZUoclaD14277410 = pWaxhvuqZUoclaD96693905;     pWaxhvuqZUoclaD96693905 = pWaxhvuqZUoclaD9641843;     pWaxhvuqZUoclaD9641843 = pWaxhvuqZUoclaD49355757;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MvpYvscjUimvpxftmIEUlIuJiyxwd33107739() {     int vmqxQZHfWEtzmuj16526816 = -241822878;    int vmqxQZHfWEtzmuj36321801 = -185020282;    int vmqxQZHfWEtzmuj41449679 = -802795759;    int vmqxQZHfWEtzmuj7347128 = -422685219;    int vmqxQZHfWEtzmuj24374533 = -551153382;    int vmqxQZHfWEtzmuj77719983 = -902831105;    int vmqxQZHfWEtzmuj73525935 = -213381252;    int vmqxQZHfWEtzmuj21685729 = -386203427;    int vmqxQZHfWEtzmuj94380512 = -488475179;    int vmqxQZHfWEtzmuj11127532 = -117281216;    int vmqxQZHfWEtzmuj9439265 = -278499553;    int vmqxQZHfWEtzmuj13893568 = -105619099;    int vmqxQZHfWEtzmuj92484570 = -211808460;    int vmqxQZHfWEtzmuj57551670 = -12326250;    int vmqxQZHfWEtzmuj57259050 = -641189090;    int vmqxQZHfWEtzmuj14018526 = -850828969;    int vmqxQZHfWEtzmuj18343002 = -785698905;    int vmqxQZHfWEtzmuj49040463 = -833204918;    int vmqxQZHfWEtzmuj67353823 = -45685530;    int vmqxQZHfWEtzmuj85849683 = -175260085;    int vmqxQZHfWEtzmuj11314318 = -624608723;    int vmqxQZHfWEtzmuj5909712 = 31187140;    int vmqxQZHfWEtzmuj56105356 = -732134387;    int vmqxQZHfWEtzmuj86018023 = -710484977;    int vmqxQZHfWEtzmuj34295952 = -812501414;    int vmqxQZHfWEtzmuj6682855 = -590220269;    int vmqxQZHfWEtzmuj9356848 = -982160815;    int vmqxQZHfWEtzmuj1785573 = -962009795;    int vmqxQZHfWEtzmuj72623282 = -214472829;    int vmqxQZHfWEtzmuj19957487 = -122976072;    int vmqxQZHfWEtzmuj12531596 = -770752872;    int vmqxQZHfWEtzmuj8156799 = -355778152;    int vmqxQZHfWEtzmuj31785462 = -556603640;    int vmqxQZHfWEtzmuj44963287 = -88977808;    int vmqxQZHfWEtzmuj20924614 = -842287946;    int vmqxQZHfWEtzmuj60215084 = -879182167;    int vmqxQZHfWEtzmuj16098740 = -594380783;    int vmqxQZHfWEtzmuj94886216 = 97528113;    int vmqxQZHfWEtzmuj31639243 = 24045210;    int vmqxQZHfWEtzmuj82152699 = -132830082;    int vmqxQZHfWEtzmuj88930298 = -426803366;    int vmqxQZHfWEtzmuj69768058 = -875080731;    int vmqxQZHfWEtzmuj60883557 = -539522792;    int vmqxQZHfWEtzmuj57196266 = -527850271;    int vmqxQZHfWEtzmuj2240145 = -508117844;    int vmqxQZHfWEtzmuj51033299 = 85604133;    int vmqxQZHfWEtzmuj21610350 = -20853616;    int vmqxQZHfWEtzmuj54554715 = -647488297;    int vmqxQZHfWEtzmuj49527128 = -105387701;    int vmqxQZHfWEtzmuj38724025 = -997485833;    int vmqxQZHfWEtzmuj32290322 = 74788201;    int vmqxQZHfWEtzmuj88912015 = -939033359;    int vmqxQZHfWEtzmuj41000813 = 96751377;    int vmqxQZHfWEtzmuj43421641 = -80136287;    int vmqxQZHfWEtzmuj21793422 = -264856804;    int vmqxQZHfWEtzmuj10617104 = -173010019;    int vmqxQZHfWEtzmuj80216444 = -452885896;    int vmqxQZHfWEtzmuj55431655 = 7689217;    int vmqxQZHfWEtzmuj73051175 = -610183805;    int vmqxQZHfWEtzmuj17691679 = -960933113;    int vmqxQZHfWEtzmuj68363135 = -920670290;    int vmqxQZHfWEtzmuj71740363 = -251371458;    int vmqxQZHfWEtzmuj49062447 = -71730599;    int vmqxQZHfWEtzmuj74423026 = -265499108;    int vmqxQZHfWEtzmuj98595936 = -346528345;    int vmqxQZHfWEtzmuj1282466 = -922721401;    int vmqxQZHfWEtzmuj82108106 = -549015460;    int vmqxQZHfWEtzmuj47521284 = -22830653;    int vmqxQZHfWEtzmuj36627057 = -170038305;    int vmqxQZHfWEtzmuj97043965 = -762006923;    int vmqxQZHfWEtzmuj97919786 = -156448186;    int vmqxQZHfWEtzmuj23456786 = -783227019;    int vmqxQZHfWEtzmuj17401220 = -757250128;    int vmqxQZHfWEtzmuj85201124 = -912855449;    int vmqxQZHfWEtzmuj96919384 = -748456720;    int vmqxQZHfWEtzmuj41546259 = -749527992;    int vmqxQZHfWEtzmuj45026154 = -429290069;    int vmqxQZHfWEtzmuj98909089 = -104284116;    int vmqxQZHfWEtzmuj83777878 = -102367134;    int vmqxQZHfWEtzmuj83262652 = -798105547;    int vmqxQZHfWEtzmuj85072504 = -469366653;    int vmqxQZHfWEtzmuj54802133 = -234672519;    int vmqxQZHfWEtzmuj52258445 = -756622094;    int vmqxQZHfWEtzmuj33899257 = -216986996;    int vmqxQZHfWEtzmuj87667165 = -97764273;    int vmqxQZHfWEtzmuj23619581 = -831719514;    int vmqxQZHfWEtzmuj67155986 = -352529529;    int vmqxQZHfWEtzmuj88363821 = -376467353;    int vmqxQZHfWEtzmuj23169865 = -824121004;    int vmqxQZHfWEtzmuj10307510 = -569277927;    int vmqxQZHfWEtzmuj79998639 = -326296271;    int vmqxQZHfWEtzmuj60667084 = -502070001;    int vmqxQZHfWEtzmuj21835042 = -292288082;    int vmqxQZHfWEtzmuj13947565 = -15021677;    int vmqxQZHfWEtzmuj13789564 = -212159792;    int vmqxQZHfWEtzmuj17189936 = -75431908;    int vmqxQZHfWEtzmuj20705612 = -703350132;    int vmqxQZHfWEtzmuj86460531 = -174023684;    int vmqxQZHfWEtzmuj58600330 = -81321927;    int vmqxQZHfWEtzmuj957680 = -241822878;     vmqxQZHfWEtzmuj16526816 = vmqxQZHfWEtzmuj36321801;     vmqxQZHfWEtzmuj36321801 = vmqxQZHfWEtzmuj41449679;     vmqxQZHfWEtzmuj41449679 = vmqxQZHfWEtzmuj7347128;     vmqxQZHfWEtzmuj7347128 = vmqxQZHfWEtzmuj24374533;     vmqxQZHfWEtzmuj24374533 = vmqxQZHfWEtzmuj77719983;     vmqxQZHfWEtzmuj77719983 = vmqxQZHfWEtzmuj73525935;     vmqxQZHfWEtzmuj73525935 = vmqxQZHfWEtzmuj21685729;     vmqxQZHfWEtzmuj21685729 = vmqxQZHfWEtzmuj94380512;     vmqxQZHfWEtzmuj94380512 = vmqxQZHfWEtzmuj11127532;     vmqxQZHfWEtzmuj11127532 = vmqxQZHfWEtzmuj9439265;     vmqxQZHfWEtzmuj9439265 = vmqxQZHfWEtzmuj13893568;     vmqxQZHfWEtzmuj13893568 = vmqxQZHfWEtzmuj92484570;     vmqxQZHfWEtzmuj92484570 = vmqxQZHfWEtzmuj57551670;     vmqxQZHfWEtzmuj57551670 = vmqxQZHfWEtzmuj57259050;     vmqxQZHfWEtzmuj57259050 = vmqxQZHfWEtzmuj14018526;     vmqxQZHfWEtzmuj14018526 = vmqxQZHfWEtzmuj18343002;     vmqxQZHfWEtzmuj18343002 = vmqxQZHfWEtzmuj49040463;     vmqxQZHfWEtzmuj49040463 = vmqxQZHfWEtzmuj67353823;     vmqxQZHfWEtzmuj67353823 = vmqxQZHfWEtzmuj85849683;     vmqxQZHfWEtzmuj85849683 = vmqxQZHfWEtzmuj11314318;     vmqxQZHfWEtzmuj11314318 = vmqxQZHfWEtzmuj5909712;     vmqxQZHfWEtzmuj5909712 = vmqxQZHfWEtzmuj56105356;     vmqxQZHfWEtzmuj56105356 = vmqxQZHfWEtzmuj86018023;     vmqxQZHfWEtzmuj86018023 = vmqxQZHfWEtzmuj34295952;     vmqxQZHfWEtzmuj34295952 = vmqxQZHfWEtzmuj6682855;     vmqxQZHfWEtzmuj6682855 = vmqxQZHfWEtzmuj9356848;     vmqxQZHfWEtzmuj9356848 = vmqxQZHfWEtzmuj1785573;     vmqxQZHfWEtzmuj1785573 = vmqxQZHfWEtzmuj72623282;     vmqxQZHfWEtzmuj72623282 = vmqxQZHfWEtzmuj19957487;     vmqxQZHfWEtzmuj19957487 = vmqxQZHfWEtzmuj12531596;     vmqxQZHfWEtzmuj12531596 = vmqxQZHfWEtzmuj8156799;     vmqxQZHfWEtzmuj8156799 = vmqxQZHfWEtzmuj31785462;     vmqxQZHfWEtzmuj31785462 = vmqxQZHfWEtzmuj44963287;     vmqxQZHfWEtzmuj44963287 = vmqxQZHfWEtzmuj20924614;     vmqxQZHfWEtzmuj20924614 = vmqxQZHfWEtzmuj60215084;     vmqxQZHfWEtzmuj60215084 = vmqxQZHfWEtzmuj16098740;     vmqxQZHfWEtzmuj16098740 = vmqxQZHfWEtzmuj94886216;     vmqxQZHfWEtzmuj94886216 = vmqxQZHfWEtzmuj31639243;     vmqxQZHfWEtzmuj31639243 = vmqxQZHfWEtzmuj82152699;     vmqxQZHfWEtzmuj82152699 = vmqxQZHfWEtzmuj88930298;     vmqxQZHfWEtzmuj88930298 = vmqxQZHfWEtzmuj69768058;     vmqxQZHfWEtzmuj69768058 = vmqxQZHfWEtzmuj60883557;     vmqxQZHfWEtzmuj60883557 = vmqxQZHfWEtzmuj57196266;     vmqxQZHfWEtzmuj57196266 = vmqxQZHfWEtzmuj2240145;     vmqxQZHfWEtzmuj2240145 = vmqxQZHfWEtzmuj51033299;     vmqxQZHfWEtzmuj51033299 = vmqxQZHfWEtzmuj21610350;     vmqxQZHfWEtzmuj21610350 = vmqxQZHfWEtzmuj54554715;     vmqxQZHfWEtzmuj54554715 = vmqxQZHfWEtzmuj49527128;     vmqxQZHfWEtzmuj49527128 = vmqxQZHfWEtzmuj38724025;     vmqxQZHfWEtzmuj38724025 = vmqxQZHfWEtzmuj32290322;     vmqxQZHfWEtzmuj32290322 = vmqxQZHfWEtzmuj88912015;     vmqxQZHfWEtzmuj88912015 = vmqxQZHfWEtzmuj41000813;     vmqxQZHfWEtzmuj41000813 = vmqxQZHfWEtzmuj43421641;     vmqxQZHfWEtzmuj43421641 = vmqxQZHfWEtzmuj21793422;     vmqxQZHfWEtzmuj21793422 = vmqxQZHfWEtzmuj10617104;     vmqxQZHfWEtzmuj10617104 = vmqxQZHfWEtzmuj80216444;     vmqxQZHfWEtzmuj80216444 = vmqxQZHfWEtzmuj55431655;     vmqxQZHfWEtzmuj55431655 = vmqxQZHfWEtzmuj73051175;     vmqxQZHfWEtzmuj73051175 = vmqxQZHfWEtzmuj17691679;     vmqxQZHfWEtzmuj17691679 = vmqxQZHfWEtzmuj68363135;     vmqxQZHfWEtzmuj68363135 = vmqxQZHfWEtzmuj71740363;     vmqxQZHfWEtzmuj71740363 = vmqxQZHfWEtzmuj49062447;     vmqxQZHfWEtzmuj49062447 = vmqxQZHfWEtzmuj74423026;     vmqxQZHfWEtzmuj74423026 = vmqxQZHfWEtzmuj98595936;     vmqxQZHfWEtzmuj98595936 = vmqxQZHfWEtzmuj1282466;     vmqxQZHfWEtzmuj1282466 = vmqxQZHfWEtzmuj82108106;     vmqxQZHfWEtzmuj82108106 = vmqxQZHfWEtzmuj47521284;     vmqxQZHfWEtzmuj47521284 = vmqxQZHfWEtzmuj36627057;     vmqxQZHfWEtzmuj36627057 = vmqxQZHfWEtzmuj97043965;     vmqxQZHfWEtzmuj97043965 = vmqxQZHfWEtzmuj97919786;     vmqxQZHfWEtzmuj97919786 = vmqxQZHfWEtzmuj23456786;     vmqxQZHfWEtzmuj23456786 = vmqxQZHfWEtzmuj17401220;     vmqxQZHfWEtzmuj17401220 = vmqxQZHfWEtzmuj85201124;     vmqxQZHfWEtzmuj85201124 = vmqxQZHfWEtzmuj96919384;     vmqxQZHfWEtzmuj96919384 = vmqxQZHfWEtzmuj41546259;     vmqxQZHfWEtzmuj41546259 = vmqxQZHfWEtzmuj45026154;     vmqxQZHfWEtzmuj45026154 = vmqxQZHfWEtzmuj98909089;     vmqxQZHfWEtzmuj98909089 = vmqxQZHfWEtzmuj83777878;     vmqxQZHfWEtzmuj83777878 = vmqxQZHfWEtzmuj83262652;     vmqxQZHfWEtzmuj83262652 = vmqxQZHfWEtzmuj85072504;     vmqxQZHfWEtzmuj85072504 = vmqxQZHfWEtzmuj54802133;     vmqxQZHfWEtzmuj54802133 = vmqxQZHfWEtzmuj52258445;     vmqxQZHfWEtzmuj52258445 = vmqxQZHfWEtzmuj33899257;     vmqxQZHfWEtzmuj33899257 = vmqxQZHfWEtzmuj87667165;     vmqxQZHfWEtzmuj87667165 = vmqxQZHfWEtzmuj23619581;     vmqxQZHfWEtzmuj23619581 = vmqxQZHfWEtzmuj67155986;     vmqxQZHfWEtzmuj67155986 = vmqxQZHfWEtzmuj88363821;     vmqxQZHfWEtzmuj88363821 = vmqxQZHfWEtzmuj23169865;     vmqxQZHfWEtzmuj23169865 = vmqxQZHfWEtzmuj10307510;     vmqxQZHfWEtzmuj10307510 = vmqxQZHfWEtzmuj79998639;     vmqxQZHfWEtzmuj79998639 = vmqxQZHfWEtzmuj60667084;     vmqxQZHfWEtzmuj60667084 = vmqxQZHfWEtzmuj21835042;     vmqxQZHfWEtzmuj21835042 = vmqxQZHfWEtzmuj13947565;     vmqxQZHfWEtzmuj13947565 = vmqxQZHfWEtzmuj13789564;     vmqxQZHfWEtzmuj13789564 = vmqxQZHfWEtzmuj17189936;     vmqxQZHfWEtzmuj17189936 = vmqxQZHfWEtzmuj20705612;     vmqxQZHfWEtzmuj20705612 = vmqxQZHfWEtzmuj86460531;     vmqxQZHfWEtzmuj86460531 = vmqxQZHfWEtzmuj58600330;     vmqxQZHfWEtzmuj58600330 = vmqxQZHfWEtzmuj957680;     vmqxQZHfWEtzmuj957680 = vmqxQZHfWEtzmuj16526816;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xQSohIQHrbYmdzIopaYUTWYyxxZBa35297077() {     int nJPUWbaDldayAoF13039737 = -240623566;    int nJPUWbaDldayAoF51404626 = -137708585;    int nJPUWbaDldayAoF97131989 = -550898525;    int nJPUWbaDldayAoF19895976 = -891499009;    int nJPUWbaDldayAoF93026922 = -106740377;    int nJPUWbaDldayAoF14966856 = -933358681;    int nJPUWbaDldayAoF47340589 = -378299227;    int nJPUWbaDldayAoF57137899 = -499087612;    int nJPUWbaDldayAoF98634769 = -740892933;    int nJPUWbaDldayAoF65132068 = -876006429;    int nJPUWbaDldayAoF69655380 = -253734790;    int nJPUWbaDldayAoF28910005 = -674352005;    int nJPUWbaDldayAoF97755268 = -497594124;    int nJPUWbaDldayAoF24626784 = -521847785;    int nJPUWbaDldayAoF62158807 = -492515800;    int nJPUWbaDldayAoF26975350 = -609450184;    int nJPUWbaDldayAoF46033851 = -698545950;    int nJPUWbaDldayAoF85175950 = -350730983;    int nJPUWbaDldayAoF49276800 = -534812831;    int nJPUWbaDldayAoF26574942 = -84784916;    int nJPUWbaDldayAoF18993173 = -835158321;    int nJPUWbaDldayAoF46811469 = -346830552;    int nJPUWbaDldayAoF45602319 = 86945903;    int nJPUWbaDldayAoF9842795 = -669602381;    int nJPUWbaDldayAoF12570960 = -337927173;    int nJPUWbaDldayAoF82707218 = -769515532;    int nJPUWbaDldayAoF57253533 = -809342662;    int nJPUWbaDldayAoF94035935 = -617765894;    int nJPUWbaDldayAoF54819253 = -79850812;    int nJPUWbaDldayAoF20211994 = -587752914;    int nJPUWbaDldayAoF21625675 = 27278975;    int nJPUWbaDldayAoF43906982 = -687784172;    int nJPUWbaDldayAoF49229900 = -446700407;    int nJPUWbaDldayAoF57622955 = 90512192;    int nJPUWbaDldayAoF30409452 = 43046529;    int nJPUWbaDldayAoF3886338 = -56258875;    int nJPUWbaDldayAoF96655706 = -104804741;    int nJPUWbaDldayAoF68013191 = -823603856;    int nJPUWbaDldayAoF80373549 = -317023540;    int nJPUWbaDldayAoF61670098 = -669177964;    int nJPUWbaDldayAoF21201294 = -958345536;    int nJPUWbaDldayAoF13961410 = -282521693;    int nJPUWbaDldayAoF48425494 = 64204636;    int nJPUWbaDldayAoF69404374 = -669484025;    int nJPUWbaDldayAoF68226860 = 59727818;    int nJPUWbaDldayAoF48714326 = -246723663;    int nJPUWbaDldayAoF18522443 = -299374264;    int nJPUWbaDldayAoF35044739 = -810582296;    int nJPUWbaDldayAoF68136195 = -736891896;    int nJPUWbaDldayAoF11525064 = -247025288;    int nJPUWbaDldayAoF84078961 = -554591005;    int nJPUWbaDldayAoF61320653 = -421198609;    int nJPUWbaDldayAoF33144192 = -695157267;    int nJPUWbaDldayAoF28928837 = -530337291;    int nJPUWbaDldayAoF16303905 = -697578597;    int nJPUWbaDldayAoF66228267 = -893793015;    int nJPUWbaDldayAoF5802308 = -124654488;    int nJPUWbaDldayAoF87289195 = -881296144;    int nJPUWbaDldayAoF7325017 = -453571837;    int nJPUWbaDldayAoF10319704 = -337224846;    int nJPUWbaDldayAoF57713323 = -24016020;    int nJPUWbaDldayAoF53304653 = -760533334;    int nJPUWbaDldayAoF2318646 = -319236800;    int nJPUWbaDldayAoF78422775 = -53140019;    int nJPUWbaDldayAoF43506393 = -803285404;    int nJPUWbaDldayAoF25748398 = -565950618;    int nJPUWbaDldayAoF79680105 = -127651598;    int nJPUWbaDldayAoF40132314 = -488106317;    int nJPUWbaDldayAoF94217331 = -464894314;    int nJPUWbaDldayAoF58272469 = -336256925;    int nJPUWbaDldayAoF30319644 = -404645443;    int nJPUWbaDldayAoF78020660 = -874942095;    int nJPUWbaDldayAoF4802402 = 66292557;    int nJPUWbaDldayAoF87606701 = -865634868;    int nJPUWbaDldayAoF5373649 = -126439380;    int nJPUWbaDldayAoF5031763 = -452636628;    int nJPUWbaDldayAoF98385975 = -311035189;    int nJPUWbaDldayAoF76197944 = -243570073;    int nJPUWbaDldayAoF41615934 = -629330200;    int nJPUWbaDldayAoF63856633 = 8796490;    int nJPUWbaDldayAoF64184776 = -370141268;    int nJPUWbaDldayAoF22208794 = -998760367;    int nJPUWbaDldayAoF25899740 = -880873998;    int nJPUWbaDldayAoF43294189 = -832825524;    int nJPUWbaDldayAoF36133033 = 66838091;    int nJPUWbaDldayAoF60305021 = -551522417;    int nJPUWbaDldayAoF10762791 = -992626905;    int nJPUWbaDldayAoF20301064 = -916363117;    int nJPUWbaDldayAoF41319050 = -211909211;    int nJPUWbaDldayAoF64181184 = -63160457;    int nJPUWbaDldayAoF98084030 = -931604388;    int nJPUWbaDldayAoF9366511 = -223508598;    int nJPUWbaDldayAoF60688174 = -270032020;    int nJPUWbaDldayAoF70053846 = -979798695;    int nJPUWbaDldayAoF3956776 = -545161945;    int nJPUWbaDldayAoF67896640 = -97812203;    int nJPUWbaDldayAoF11642765 = -963284894;    int nJPUWbaDldayAoF70002718 = -882655345;    int nJPUWbaDldayAoF25897982 = -866198621;    int nJPUWbaDldayAoF42478462 = -240623566;     nJPUWbaDldayAoF13039737 = nJPUWbaDldayAoF51404626;     nJPUWbaDldayAoF51404626 = nJPUWbaDldayAoF97131989;     nJPUWbaDldayAoF97131989 = nJPUWbaDldayAoF19895976;     nJPUWbaDldayAoF19895976 = nJPUWbaDldayAoF93026922;     nJPUWbaDldayAoF93026922 = nJPUWbaDldayAoF14966856;     nJPUWbaDldayAoF14966856 = nJPUWbaDldayAoF47340589;     nJPUWbaDldayAoF47340589 = nJPUWbaDldayAoF57137899;     nJPUWbaDldayAoF57137899 = nJPUWbaDldayAoF98634769;     nJPUWbaDldayAoF98634769 = nJPUWbaDldayAoF65132068;     nJPUWbaDldayAoF65132068 = nJPUWbaDldayAoF69655380;     nJPUWbaDldayAoF69655380 = nJPUWbaDldayAoF28910005;     nJPUWbaDldayAoF28910005 = nJPUWbaDldayAoF97755268;     nJPUWbaDldayAoF97755268 = nJPUWbaDldayAoF24626784;     nJPUWbaDldayAoF24626784 = nJPUWbaDldayAoF62158807;     nJPUWbaDldayAoF62158807 = nJPUWbaDldayAoF26975350;     nJPUWbaDldayAoF26975350 = nJPUWbaDldayAoF46033851;     nJPUWbaDldayAoF46033851 = nJPUWbaDldayAoF85175950;     nJPUWbaDldayAoF85175950 = nJPUWbaDldayAoF49276800;     nJPUWbaDldayAoF49276800 = nJPUWbaDldayAoF26574942;     nJPUWbaDldayAoF26574942 = nJPUWbaDldayAoF18993173;     nJPUWbaDldayAoF18993173 = nJPUWbaDldayAoF46811469;     nJPUWbaDldayAoF46811469 = nJPUWbaDldayAoF45602319;     nJPUWbaDldayAoF45602319 = nJPUWbaDldayAoF9842795;     nJPUWbaDldayAoF9842795 = nJPUWbaDldayAoF12570960;     nJPUWbaDldayAoF12570960 = nJPUWbaDldayAoF82707218;     nJPUWbaDldayAoF82707218 = nJPUWbaDldayAoF57253533;     nJPUWbaDldayAoF57253533 = nJPUWbaDldayAoF94035935;     nJPUWbaDldayAoF94035935 = nJPUWbaDldayAoF54819253;     nJPUWbaDldayAoF54819253 = nJPUWbaDldayAoF20211994;     nJPUWbaDldayAoF20211994 = nJPUWbaDldayAoF21625675;     nJPUWbaDldayAoF21625675 = nJPUWbaDldayAoF43906982;     nJPUWbaDldayAoF43906982 = nJPUWbaDldayAoF49229900;     nJPUWbaDldayAoF49229900 = nJPUWbaDldayAoF57622955;     nJPUWbaDldayAoF57622955 = nJPUWbaDldayAoF30409452;     nJPUWbaDldayAoF30409452 = nJPUWbaDldayAoF3886338;     nJPUWbaDldayAoF3886338 = nJPUWbaDldayAoF96655706;     nJPUWbaDldayAoF96655706 = nJPUWbaDldayAoF68013191;     nJPUWbaDldayAoF68013191 = nJPUWbaDldayAoF80373549;     nJPUWbaDldayAoF80373549 = nJPUWbaDldayAoF61670098;     nJPUWbaDldayAoF61670098 = nJPUWbaDldayAoF21201294;     nJPUWbaDldayAoF21201294 = nJPUWbaDldayAoF13961410;     nJPUWbaDldayAoF13961410 = nJPUWbaDldayAoF48425494;     nJPUWbaDldayAoF48425494 = nJPUWbaDldayAoF69404374;     nJPUWbaDldayAoF69404374 = nJPUWbaDldayAoF68226860;     nJPUWbaDldayAoF68226860 = nJPUWbaDldayAoF48714326;     nJPUWbaDldayAoF48714326 = nJPUWbaDldayAoF18522443;     nJPUWbaDldayAoF18522443 = nJPUWbaDldayAoF35044739;     nJPUWbaDldayAoF35044739 = nJPUWbaDldayAoF68136195;     nJPUWbaDldayAoF68136195 = nJPUWbaDldayAoF11525064;     nJPUWbaDldayAoF11525064 = nJPUWbaDldayAoF84078961;     nJPUWbaDldayAoF84078961 = nJPUWbaDldayAoF61320653;     nJPUWbaDldayAoF61320653 = nJPUWbaDldayAoF33144192;     nJPUWbaDldayAoF33144192 = nJPUWbaDldayAoF28928837;     nJPUWbaDldayAoF28928837 = nJPUWbaDldayAoF16303905;     nJPUWbaDldayAoF16303905 = nJPUWbaDldayAoF66228267;     nJPUWbaDldayAoF66228267 = nJPUWbaDldayAoF5802308;     nJPUWbaDldayAoF5802308 = nJPUWbaDldayAoF87289195;     nJPUWbaDldayAoF87289195 = nJPUWbaDldayAoF7325017;     nJPUWbaDldayAoF7325017 = nJPUWbaDldayAoF10319704;     nJPUWbaDldayAoF10319704 = nJPUWbaDldayAoF57713323;     nJPUWbaDldayAoF57713323 = nJPUWbaDldayAoF53304653;     nJPUWbaDldayAoF53304653 = nJPUWbaDldayAoF2318646;     nJPUWbaDldayAoF2318646 = nJPUWbaDldayAoF78422775;     nJPUWbaDldayAoF78422775 = nJPUWbaDldayAoF43506393;     nJPUWbaDldayAoF43506393 = nJPUWbaDldayAoF25748398;     nJPUWbaDldayAoF25748398 = nJPUWbaDldayAoF79680105;     nJPUWbaDldayAoF79680105 = nJPUWbaDldayAoF40132314;     nJPUWbaDldayAoF40132314 = nJPUWbaDldayAoF94217331;     nJPUWbaDldayAoF94217331 = nJPUWbaDldayAoF58272469;     nJPUWbaDldayAoF58272469 = nJPUWbaDldayAoF30319644;     nJPUWbaDldayAoF30319644 = nJPUWbaDldayAoF78020660;     nJPUWbaDldayAoF78020660 = nJPUWbaDldayAoF4802402;     nJPUWbaDldayAoF4802402 = nJPUWbaDldayAoF87606701;     nJPUWbaDldayAoF87606701 = nJPUWbaDldayAoF5373649;     nJPUWbaDldayAoF5373649 = nJPUWbaDldayAoF5031763;     nJPUWbaDldayAoF5031763 = nJPUWbaDldayAoF98385975;     nJPUWbaDldayAoF98385975 = nJPUWbaDldayAoF76197944;     nJPUWbaDldayAoF76197944 = nJPUWbaDldayAoF41615934;     nJPUWbaDldayAoF41615934 = nJPUWbaDldayAoF63856633;     nJPUWbaDldayAoF63856633 = nJPUWbaDldayAoF64184776;     nJPUWbaDldayAoF64184776 = nJPUWbaDldayAoF22208794;     nJPUWbaDldayAoF22208794 = nJPUWbaDldayAoF25899740;     nJPUWbaDldayAoF25899740 = nJPUWbaDldayAoF43294189;     nJPUWbaDldayAoF43294189 = nJPUWbaDldayAoF36133033;     nJPUWbaDldayAoF36133033 = nJPUWbaDldayAoF60305021;     nJPUWbaDldayAoF60305021 = nJPUWbaDldayAoF10762791;     nJPUWbaDldayAoF10762791 = nJPUWbaDldayAoF20301064;     nJPUWbaDldayAoF20301064 = nJPUWbaDldayAoF41319050;     nJPUWbaDldayAoF41319050 = nJPUWbaDldayAoF64181184;     nJPUWbaDldayAoF64181184 = nJPUWbaDldayAoF98084030;     nJPUWbaDldayAoF98084030 = nJPUWbaDldayAoF9366511;     nJPUWbaDldayAoF9366511 = nJPUWbaDldayAoF60688174;     nJPUWbaDldayAoF60688174 = nJPUWbaDldayAoF70053846;     nJPUWbaDldayAoF70053846 = nJPUWbaDldayAoF3956776;     nJPUWbaDldayAoF3956776 = nJPUWbaDldayAoF67896640;     nJPUWbaDldayAoF67896640 = nJPUWbaDldayAoF11642765;     nJPUWbaDldayAoF11642765 = nJPUWbaDldayAoF70002718;     nJPUWbaDldayAoF70002718 = nJPUWbaDldayAoF25897982;     nJPUWbaDldayAoF25897982 = nJPUWbaDldayAoF42478462;     nJPUWbaDldayAoF42478462 = nJPUWbaDldayAoF13039737;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MNcgCKoyAkpdgiRwiDMjbssHSenXq37486416() {     int aruEOlzBmagHruc9552658 = -239424254;    int aruEOlzBmagHruc66487451 = -90396887;    int aruEOlzBmagHruc52814301 = -299001291;    int aruEOlzBmagHruc32444824 = -260312800;    int aruEOlzBmagHruc61679312 = -762327373;    int aruEOlzBmagHruc52213728 = -963886257;    int aruEOlzBmagHruc21155242 = -543217202;    int aruEOlzBmagHruc92590069 = -611971796;    int aruEOlzBmagHruc2889026 = -993310686;    int aruEOlzBmagHruc19136604 = -534731642;    int aruEOlzBmagHruc29871496 = -228970027;    int aruEOlzBmagHruc43926442 = -143084911;    int aruEOlzBmagHruc3025967 = -783379787;    int aruEOlzBmagHruc91701896 = 68630680;    int aruEOlzBmagHruc67058564 = -343842510;    int aruEOlzBmagHruc39932174 = -368071399;    int aruEOlzBmagHruc73724699 = -611392995;    int aruEOlzBmagHruc21311439 = -968257048;    int aruEOlzBmagHruc31199776 = 76059869;    int aruEOlzBmagHruc67300201 = 5690254;    int aruEOlzBmagHruc26672027 = 54292081;    int aruEOlzBmagHruc87713226 = -724848245;    int aruEOlzBmagHruc35099281 = -193973808;    int aruEOlzBmagHruc33667565 = -628719786;    int aruEOlzBmagHruc90845966 = -963352932;    int aruEOlzBmagHruc58731583 = -948810794;    int aruEOlzBmagHruc5150218 = -636524509;    int aruEOlzBmagHruc86286297 = -273521992;    int aruEOlzBmagHruc37015225 = 54771205;    int aruEOlzBmagHruc20466502 = 47470244;    int aruEOlzBmagHruc30719755 = -274689178;    int aruEOlzBmagHruc79657165 = 80209807;    int aruEOlzBmagHruc66674339 = -336797175;    int aruEOlzBmagHruc70282623 = -829997807;    int aruEOlzBmagHruc39894291 = -171618997;    int aruEOlzBmagHruc47557591 = -333335584;    int aruEOlzBmagHruc77212672 = -715228699;    int aruEOlzBmagHruc41140165 = -644735825;    int aruEOlzBmagHruc29107857 = -658092291;    int aruEOlzBmagHruc41187497 = -105525846;    int aruEOlzBmagHruc53472289 = -389887707;    int aruEOlzBmagHruc58154762 = -789962656;    int aruEOlzBmagHruc35967430 = -432067936;    int aruEOlzBmagHruc81612482 = -811117779;    int aruEOlzBmagHruc34213575 = -472426519;    int aruEOlzBmagHruc46395353 = -579051459;    int aruEOlzBmagHruc15434536 = -577894913;    int aruEOlzBmagHruc15534762 = -973676295;    int aruEOlzBmagHruc86745263 = -268396091;    int aruEOlzBmagHruc84326103 = -596564743;    int aruEOlzBmagHruc35867602 = -83970212;    int aruEOlzBmagHruc33729292 = 96636141;    int aruEOlzBmagHruc25287570 = -387065912;    int aruEOlzBmagHruc14436032 = -980538295;    int aruEOlzBmagHruc10814389 = -30300389;    int aruEOlzBmagHruc21839431 = -514576010;    int aruEOlzBmagHruc31388171 = -896423080;    int aruEOlzBmagHruc19146737 = -670281505;    int aruEOlzBmagHruc41598857 = -296959868;    int aruEOlzBmagHruc2947730 = -813516579;    int aruEOlzBmagHruc47063510 = -227361749;    int aruEOlzBmagHruc34868944 = -169695210;    int aruEOlzBmagHruc55574844 = -566743002;    int aruEOlzBmagHruc82422524 = -940780930;    int aruEOlzBmagHruc88416849 = -160042464;    int aruEOlzBmagHruc50214330 = -209179835;    int aruEOlzBmagHruc77252103 = -806287736;    int aruEOlzBmagHruc32743343 = -953381981;    int aruEOlzBmagHruc51807606 = -759750324;    int aruEOlzBmagHruc19500973 = 89493073;    int aruEOlzBmagHruc62719501 = -652842701;    int aruEOlzBmagHruc32584534 = -966657170;    int aruEOlzBmagHruc92203582 = -210164758;    int aruEOlzBmagHruc90012278 = -818414286;    int aruEOlzBmagHruc13827913 = -604422039;    int aruEOlzBmagHruc68517265 = -155745263;    int aruEOlzBmagHruc51745797 = -192780310;    int aruEOlzBmagHruc53486799 = -382856029;    int aruEOlzBmagHruc99453989 = -56293267;    int aruEOlzBmagHruc44450614 = -284301474;    int aruEOlzBmagHruc43297048 = -270915882;    int aruEOlzBmagHruc89615455 = -662848214;    int aruEOlzBmagHruc99541034 = 94874098;    int aruEOlzBmagHruc52689122 = -348664052;    int aruEOlzBmagHruc84598899 = -868559545;    int aruEOlzBmagHruc96990462 = -271325320;    int aruEOlzBmagHruc54369595 = -532724281;    int aruEOlzBmagHruc52238307 = -356258880;    int aruEOlzBmagHruc59468234 = -699697419;    int aruEOlzBmagHruc18054860 = -657042987;    int aruEOlzBmagHruc16169421 = -436912505;    int aruEOlzBmagHruc58065936 = 55052806;    int aruEOlzBmagHruc99541307 = -247775957;    int aruEOlzBmagHruc26160128 = -844575712;    int aruEOlzBmagHruc94123987 = -878164097;    int aruEOlzBmagHruc18603345 = -120192497;    int aruEOlzBmagHruc2579918 = -123219655;    int aruEOlzBmagHruc53544905 = -491287006;    int aruEOlzBmagHruc93195632 = -551075316;    int aruEOlzBmagHruc83999244 = -239424254;     aruEOlzBmagHruc9552658 = aruEOlzBmagHruc66487451;     aruEOlzBmagHruc66487451 = aruEOlzBmagHruc52814301;     aruEOlzBmagHruc52814301 = aruEOlzBmagHruc32444824;     aruEOlzBmagHruc32444824 = aruEOlzBmagHruc61679312;     aruEOlzBmagHruc61679312 = aruEOlzBmagHruc52213728;     aruEOlzBmagHruc52213728 = aruEOlzBmagHruc21155242;     aruEOlzBmagHruc21155242 = aruEOlzBmagHruc92590069;     aruEOlzBmagHruc92590069 = aruEOlzBmagHruc2889026;     aruEOlzBmagHruc2889026 = aruEOlzBmagHruc19136604;     aruEOlzBmagHruc19136604 = aruEOlzBmagHruc29871496;     aruEOlzBmagHruc29871496 = aruEOlzBmagHruc43926442;     aruEOlzBmagHruc43926442 = aruEOlzBmagHruc3025967;     aruEOlzBmagHruc3025967 = aruEOlzBmagHruc91701896;     aruEOlzBmagHruc91701896 = aruEOlzBmagHruc67058564;     aruEOlzBmagHruc67058564 = aruEOlzBmagHruc39932174;     aruEOlzBmagHruc39932174 = aruEOlzBmagHruc73724699;     aruEOlzBmagHruc73724699 = aruEOlzBmagHruc21311439;     aruEOlzBmagHruc21311439 = aruEOlzBmagHruc31199776;     aruEOlzBmagHruc31199776 = aruEOlzBmagHruc67300201;     aruEOlzBmagHruc67300201 = aruEOlzBmagHruc26672027;     aruEOlzBmagHruc26672027 = aruEOlzBmagHruc87713226;     aruEOlzBmagHruc87713226 = aruEOlzBmagHruc35099281;     aruEOlzBmagHruc35099281 = aruEOlzBmagHruc33667565;     aruEOlzBmagHruc33667565 = aruEOlzBmagHruc90845966;     aruEOlzBmagHruc90845966 = aruEOlzBmagHruc58731583;     aruEOlzBmagHruc58731583 = aruEOlzBmagHruc5150218;     aruEOlzBmagHruc5150218 = aruEOlzBmagHruc86286297;     aruEOlzBmagHruc86286297 = aruEOlzBmagHruc37015225;     aruEOlzBmagHruc37015225 = aruEOlzBmagHruc20466502;     aruEOlzBmagHruc20466502 = aruEOlzBmagHruc30719755;     aruEOlzBmagHruc30719755 = aruEOlzBmagHruc79657165;     aruEOlzBmagHruc79657165 = aruEOlzBmagHruc66674339;     aruEOlzBmagHruc66674339 = aruEOlzBmagHruc70282623;     aruEOlzBmagHruc70282623 = aruEOlzBmagHruc39894291;     aruEOlzBmagHruc39894291 = aruEOlzBmagHruc47557591;     aruEOlzBmagHruc47557591 = aruEOlzBmagHruc77212672;     aruEOlzBmagHruc77212672 = aruEOlzBmagHruc41140165;     aruEOlzBmagHruc41140165 = aruEOlzBmagHruc29107857;     aruEOlzBmagHruc29107857 = aruEOlzBmagHruc41187497;     aruEOlzBmagHruc41187497 = aruEOlzBmagHruc53472289;     aruEOlzBmagHruc53472289 = aruEOlzBmagHruc58154762;     aruEOlzBmagHruc58154762 = aruEOlzBmagHruc35967430;     aruEOlzBmagHruc35967430 = aruEOlzBmagHruc81612482;     aruEOlzBmagHruc81612482 = aruEOlzBmagHruc34213575;     aruEOlzBmagHruc34213575 = aruEOlzBmagHruc46395353;     aruEOlzBmagHruc46395353 = aruEOlzBmagHruc15434536;     aruEOlzBmagHruc15434536 = aruEOlzBmagHruc15534762;     aruEOlzBmagHruc15534762 = aruEOlzBmagHruc86745263;     aruEOlzBmagHruc86745263 = aruEOlzBmagHruc84326103;     aruEOlzBmagHruc84326103 = aruEOlzBmagHruc35867602;     aruEOlzBmagHruc35867602 = aruEOlzBmagHruc33729292;     aruEOlzBmagHruc33729292 = aruEOlzBmagHruc25287570;     aruEOlzBmagHruc25287570 = aruEOlzBmagHruc14436032;     aruEOlzBmagHruc14436032 = aruEOlzBmagHruc10814389;     aruEOlzBmagHruc10814389 = aruEOlzBmagHruc21839431;     aruEOlzBmagHruc21839431 = aruEOlzBmagHruc31388171;     aruEOlzBmagHruc31388171 = aruEOlzBmagHruc19146737;     aruEOlzBmagHruc19146737 = aruEOlzBmagHruc41598857;     aruEOlzBmagHruc41598857 = aruEOlzBmagHruc2947730;     aruEOlzBmagHruc2947730 = aruEOlzBmagHruc47063510;     aruEOlzBmagHruc47063510 = aruEOlzBmagHruc34868944;     aruEOlzBmagHruc34868944 = aruEOlzBmagHruc55574844;     aruEOlzBmagHruc55574844 = aruEOlzBmagHruc82422524;     aruEOlzBmagHruc82422524 = aruEOlzBmagHruc88416849;     aruEOlzBmagHruc88416849 = aruEOlzBmagHruc50214330;     aruEOlzBmagHruc50214330 = aruEOlzBmagHruc77252103;     aruEOlzBmagHruc77252103 = aruEOlzBmagHruc32743343;     aruEOlzBmagHruc32743343 = aruEOlzBmagHruc51807606;     aruEOlzBmagHruc51807606 = aruEOlzBmagHruc19500973;     aruEOlzBmagHruc19500973 = aruEOlzBmagHruc62719501;     aruEOlzBmagHruc62719501 = aruEOlzBmagHruc32584534;     aruEOlzBmagHruc32584534 = aruEOlzBmagHruc92203582;     aruEOlzBmagHruc92203582 = aruEOlzBmagHruc90012278;     aruEOlzBmagHruc90012278 = aruEOlzBmagHruc13827913;     aruEOlzBmagHruc13827913 = aruEOlzBmagHruc68517265;     aruEOlzBmagHruc68517265 = aruEOlzBmagHruc51745797;     aruEOlzBmagHruc51745797 = aruEOlzBmagHruc53486799;     aruEOlzBmagHruc53486799 = aruEOlzBmagHruc99453989;     aruEOlzBmagHruc99453989 = aruEOlzBmagHruc44450614;     aruEOlzBmagHruc44450614 = aruEOlzBmagHruc43297048;     aruEOlzBmagHruc43297048 = aruEOlzBmagHruc89615455;     aruEOlzBmagHruc89615455 = aruEOlzBmagHruc99541034;     aruEOlzBmagHruc99541034 = aruEOlzBmagHruc52689122;     aruEOlzBmagHruc52689122 = aruEOlzBmagHruc84598899;     aruEOlzBmagHruc84598899 = aruEOlzBmagHruc96990462;     aruEOlzBmagHruc96990462 = aruEOlzBmagHruc54369595;     aruEOlzBmagHruc54369595 = aruEOlzBmagHruc52238307;     aruEOlzBmagHruc52238307 = aruEOlzBmagHruc59468234;     aruEOlzBmagHruc59468234 = aruEOlzBmagHruc18054860;     aruEOlzBmagHruc18054860 = aruEOlzBmagHruc16169421;     aruEOlzBmagHruc16169421 = aruEOlzBmagHruc58065936;     aruEOlzBmagHruc58065936 = aruEOlzBmagHruc99541307;     aruEOlzBmagHruc99541307 = aruEOlzBmagHruc26160128;     aruEOlzBmagHruc26160128 = aruEOlzBmagHruc94123987;     aruEOlzBmagHruc94123987 = aruEOlzBmagHruc18603345;     aruEOlzBmagHruc18603345 = aruEOlzBmagHruc2579918;     aruEOlzBmagHruc2579918 = aruEOlzBmagHruc53544905;     aruEOlzBmagHruc53544905 = aruEOlzBmagHruc93195632;     aruEOlzBmagHruc93195632 = aruEOlzBmagHruc83999244;     aruEOlzBmagHruc83999244 = aruEOlzBmagHruc9552658;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ByAYcLfwLYnIpeIfQukkapsTwiKCp39675754() {     int nqAJZBZAitkQtyB6065579 = -238224943;    int nqAJZBZAitkQtyB81570277 = -43085189;    int nqAJZBZAitkQtyB8496612 = -47104056;    int nqAJZBZAitkQtyB44993672 = -729126590;    int nqAJZBZAitkQtyB30331703 = -317914369;    int nqAJZBZAitkQtyB89460599 = -994413834;    int nqAJZBZAitkQtyB94969894 = -708135177;    int nqAJZBZAitkQtyB28042240 = -724855980;    int nqAJZBZAitkQtyB7143283 = -145728440;    int nqAJZBZAitkQtyB73141140 = -193456854;    int nqAJZBZAitkQtyB90087610 = -204205264;    int nqAJZBZAitkQtyB58942879 = -711817817;    int nqAJZBZAitkQtyB8296664 = 30834549;    int nqAJZBZAitkQtyB58777010 = -440890855;    int nqAJZBZAitkQtyB71958321 = -195169220;    int nqAJZBZAitkQtyB52888997 = -126692615;    int nqAJZBZAitkQtyB1415549 = -524240039;    int nqAJZBZAitkQtyB57446927 = -485783113;    int nqAJZBZAitkQtyB13122752 = -413067431;    int nqAJZBZAitkQtyB8025461 = 96165424;    int nqAJZBZAitkQtyB34350882 = -156257517;    int nqAJZBZAitkQtyB28614984 = -2865937;    int nqAJZBZAitkQtyB24596244 = -474893518;    int nqAJZBZAitkQtyB57492335 = -587837190;    int nqAJZBZAitkQtyB69120973 = -488778692;    int nqAJZBZAitkQtyB34755948 = -28106057;    int nqAJZBZAitkQtyB53046902 = -463706356;    int nqAJZBZAitkQtyB78536660 = 70721909;    int nqAJZBZAitkQtyB19211196 = -910606778;    int nqAJZBZAitkQtyB20721010 = -417306599;    int nqAJZBZAitkQtyB39813834 = -576657332;    int nqAJZBZAitkQtyB15407349 = -251796213;    int nqAJZBZAitkQtyB84118778 = -226893943;    int nqAJZBZAitkQtyB82942291 = -650507807;    int nqAJZBZAitkQtyB49379129 = -386284523;    int nqAJZBZAitkQtyB91228845 = -610412292;    int nqAJZBZAitkQtyB57769639 = -225652657;    int nqAJZBZAitkQtyB14267140 = -465867793;    int nqAJZBZAitkQtyB77842163 = -999161041;    int nqAJZBZAitkQtyB20704897 = -641873728;    int nqAJZBZAitkQtyB85743283 = -921429878;    int nqAJZBZAitkQtyB2348115 = -197403618;    int nqAJZBZAitkQtyB23509366 = -928340508;    int nqAJZBZAitkQtyB93820590 = -952751533;    int nqAJZBZAitkQtyB200290 = 95419143;    int nqAJZBZAitkQtyB44076380 = -911379255;    int nqAJZBZAitkQtyB12346628 = -856415561;    int nqAJZBZAitkQtyB96024785 = -36770294;    int nqAJZBZAitkQtyB5354332 = -899900286;    int nqAJZBZAitkQtyB57127142 = -946104199;    int nqAJZBZAitkQtyB87656242 = -713349418;    int nqAJZBZAitkQtyB6137930 = -485529109;    int nqAJZBZAitkQtyB17430949 = -78974556;    int nqAJZBZAitkQtyB99943227 = -330739299;    int nqAJZBZAitkQtyB5324873 = -463022182;    int nqAJZBZAitkQtyB77450594 = -135359006;    int nqAJZBZAitkQtyB56974033 = -568191671;    int nqAJZBZAitkQtyB51004277 = -459266867;    int nqAJZBZAitkQtyB75872698 = -140347899;    int nqAJZBZAitkQtyB95575754 = -189808312;    int nqAJZBZAitkQtyB36413698 = -430707479;    int nqAJZBZAitkQtyB16433235 = -678857086;    int nqAJZBZAitkQtyB8831044 = -814249203;    int nqAJZBZAitkQtyB86422273 = -728421842;    int nqAJZBZAitkQtyB33327307 = -616799523;    int nqAJZBZAitkQtyB74680262 = -952409052;    int nqAJZBZAitkQtyB74824101 = -384923874;    int nqAJZBZAitkQtyB25354373 = -318657644;    int nqAJZBZAitkQtyB9397881 = 45393667;    int nqAJZBZAitkQtyB80729476 = -584756928;    int nqAJZBZAitkQtyB95119357 = -901039958;    int nqAJZBZAitkQtyB87148408 = 41627754;    int nqAJZBZAitkQtyB79604764 = -486622073;    int nqAJZBZAitkQtyB92417855 = -771193704;    int nqAJZBZAitkQtyB22282177 = 17595301;    int nqAJZBZAitkQtyB32002768 = -958853899;    int nqAJZBZAitkQtyB5105619 = -74525430;    int nqAJZBZAitkQtyB30775653 = -522141986;    int nqAJZBZAitkQtyB57292045 = -583256333;    int nqAJZBZAitkQtyB25044594 = -577399437;    int nqAJZBZAitkQtyB22409320 = -171690497;    int nqAJZBZAitkQtyB57022117 = -326936062;    int nqAJZBZAitkQtyB73182329 = -29377806;    int nqAJZBZAitkQtyB62084054 = -964502580;    int nqAJZBZAitkQtyB33064767 = -703957181;    int nqAJZBZAitkQtyB33675904 = 8871777;    int nqAJZBZAitkQtyB97976400 = -72821657;    int nqAJZBZAitkQtyB84175550 = -896154644;    int nqAJZBZAitkQtyB77617419 = -87485626;    int nqAJZBZAitkQtyB71928534 = -150925517;    int nqAJZBZAitkQtyB34254812 = 57779379;    int nqAJZBZAitkQtyB6765363 = -766385790;    int nqAJZBZAitkQtyB38394441 = -225519895;    int nqAJZBZAitkQtyB82266408 = -709352729;    int nqAJZBZAitkQtyB84291199 = -111166250;    int nqAJZBZAitkQtyB69310049 = -142572792;    int nqAJZBZAitkQtyB93517070 = -383154416;    int nqAJZBZAitkQtyB37087092 = -99918666;    int nqAJZBZAitkQtyB60493284 = -235952010;    int nqAJZBZAitkQtyB25520028 = -238224943;     nqAJZBZAitkQtyB6065579 = nqAJZBZAitkQtyB81570277;     nqAJZBZAitkQtyB81570277 = nqAJZBZAitkQtyB8496612;     nqAJZBZAitkQtyB8496612 = nqAJZBZAitkQtyB44993672;     nqAJZBZAitkQtyB44993672 = nqAJZBZAitkQtyB30331703;     nqAJZBZAitkQtyB30331703 = nqAJZBZAitkQtyB89460599;     nqAJZBZAitkQtyB89460599 = nqAJZBZAitkQtyB94969894;     nqAJZBZAitkQtyB94969894 = nqAJZBZAitkQtyB28042240;     nqAJZBZAitkQtyB28042240 = nqAJZBZAitkQtyB7143283;     nqAJZBZAitkQtyB7143283 = nqAJZBZAitkQtyB73141140;     nqAJZBZAitkQtyB73141140 = nqAJZBZAitkQtyB90087610;     nqAJZBZAitkQtyB90087610 = nqAJZBZAitkQtyB58942879;     nqAJZBZAitkQtyB58942879 = nqAJZBZAitkQtyB8296664;     nqAJZBZAitkQtyB8296664 = nqAJZBZAitkQtyB58777010;     nqAJZBZAitkQtyB58777010 = nqAJZBZAitkQtyB71958321;     nqAJZBZAitkQtyB71958321 = nqAJZBZAitkQtyB52888997;     nqAJZBZAitkQtyB52888997 = nqAJZBZAitkQtyB1415549;     nqAJZBZAitkQtyB1415549 = nqAJZBZAitkQtyB57446927;     nqAJZBZAitkQtyB57446927 = nqAJZBZAitkQtyB13122752;     nqAJZBZAitkQtyB13122752 = nqAJZBZAitkQtyB8025461;     nqAJZBZAitkQtyB8025461 = nqAJZBZAitkQtyB34350882;     nqAJZBZAitkQtyB34350882 = nqAJZBZAitkQtyB28614984;     nqAJZBZAitkQtyB28614984 = nqAJZBZAitkQtyB24596244;     nqAJZBZAitkQtyB24596244 = nqAJZBZAitkQtyB57492335;     nqAJZBZAitkQtyB57492335 = nqAJZBZAitkQtyB69120973;     nqAJZBZAitkQtyB69120973 = nqAJZBZAitkQtyB34755948;     nqAJZBZAitkQtyB34755948 = nqAJZBZAitkQtyB53046902;     nqAJZBZAitkQtyB53046902 = nqAJZBZAitkQtyB78536660;     nqAJZBZAitkQtyB78536660 = nqAJZBZAitkQtyB19211196;     nqAJZBZAitkQtyB19211196 = nqAJZBZAitkQtyB20721010;     nqAJZBZAitkQtyB20721010 = nqAJZBZAitkQtyB39813834;     nqAJZBZAitkQtyB39813834 = nqAJZBZAitkQtyB15407349;     nqAJZBZAitkQtyB15407349 = nqAJZBZAitkQtyB84118778;     nqAJZBZAitkQtyB84118778 = nqAJZBZAitkQtyB82942291;     nqAJZBZAitkQtyB82942291 = nqAJZBZAitkQtyB49379129;     nqAJZBZAitkQtyB49379129 = nqAJZBZAitkQtyB91228845;     nqAJZBZAitkQtyB91228845 = nqAJZBZAitkQtyB57769639;     nqAJZBZAitkQtyB57769639 = nqAJZBZAitkQtyB14267140;     nqAJZBZAitkQtyB14267140 = nqAJZBZAitkQtyB77842163;     nqAJZBZAitkQtyB77842163 = nqAJZBZAitkQtyB20704897;     nqAJZBZAitkQtyB20704897 = nqAJZBZAitkQtyB85743283;     nqAJZBZAitkQtyB85743283 = nqAJZBZAitkQtyB2348115;     nqAJZBZAitkQtyB2348115 = nqAJZBZAitkQtyB23509366;     nqAJZBZAitkQtyB23509366 = nqAJZBZAitkQtyB93820590;     nqAJZBZAitkQtyB93820590 = nqAJZBZAitkQtyB200290;     nqAJZBZAitkQtyB200290 = nqAJZBZAitkQtyB44076380;     nqAJZBZAitkQtyB44076380 = nqAJZBZAitkQtyB12346628;     nqAJZBZAitkQtyB12346628 = nqAJZBZAitkQtyB96024785;     nqAJZBZAitkQtyB96024785 = nqAJZBZAitkQtyB5354332;     nqAJZBZAitkQtyB5354332 = nqAJZBZAitkQtyB57127142;     nqAJZBZAitkQtyB57127142 = nqAJZBZAitkQtyB87656242;     nqAJZBZAitkQtyB87656242 = nqAJZBZAitkQtyB6137930;     nqAJZBZAitkQtyB6137930 = nqAJZBZAitkQtyB17430949;     nqAJZBZAitkQtyB17430949 = nqAJZBZAitkQtyB99943227;     nqAJZBZAitkQtyB99943227 = nqAJZBZAitkQtyB5324873;     nqAJZBZAitkQtyB5324873 = nqAJZBZAitkQtyB77450594;     nqAJZBZAitkQtyB77450594 = nqAJZBZAitkQtyB56974033;     nqAJZBZAitkQtyB56974033 = nqAJZBZAitkQtyB51004277;     nqAJZBZAitkQtyB51004277 = nqAJZBZAitkQtyB75872698;     nqAJZBZAitkQtyB75872698 = nqAJZBZAitkQtyB95575754;     nqAJZBZAitkQtyB95575754 = nqAJZBZAitkQtyB36413698;     nqAJZBZAitkQtyB36413698 = nqAJZBZAitkQtyB16433235;     nqAJZBZAitkQtyB16433235 = nqAJZBZAitkQtyB8831044;     nqAJZBZAitkQtyB8831044 = nqAJZBZAitkQtyB86422273;     nqAJZBZAitkQtyB86422273 = nqAJZBZAitkQtyB33327307;     nqAJZBZAitkQtyB33327307 = nqAJZBZAitkQtyB74680262;     nqAJZBZAitkQtyB74680262 = nqAJZBZAitkQtyB74824101;     nqAJZBZAitkQtyB74824101 = nqAJZBZAitkQtyB25354373;     nqAJZBZAitkQtyB25354373 = nqAJZBZAitkQtyB9397881;     nqAJZBZAitkQtyB9397881 = nqAJZBZAitkQtyB80729476;     nqAJZBZAitkQtyB80729476 = nqAJZBZAitkQtyB95119357;     nqAJZBZAitkQtyB95119357 = nqAJZBZAitkQtyB87148408;     nqAJZBZAitkQtyB87148408 = nqAJZBZAitkQtyB79604764;     nqAJZBZAitkQtyB79604764 = nqAJZBZAitkQtyB92417855;     nqAJZBZAitkQtyB92417855 = nqAJZBZAitkQtyB22282177;     nqAJZBZAitkQtyB22282177 = nqAJZBZAitkQtyB32002768;     nqAJZBZAitkQtyB32002768 = nqAJZBZAitkQtyB5105619;     nqAJZBZAitkQtyB5105619 = nqAJZBZAitkQtyB30775653;     nqAJZBZAitkQtyB30775653 = nqAJZBZAitkQtyB57292045;     nqAJZBZAitkQtyB57292045 = nqAJZBZAitkQtyB25044594;     nqAJZBZAitkQtyB25044594 = nqAJZBZAitkQtyB22409320;     nqAJZBZAitkQtyB22409320 = nqAJZBZAitkQtyB57022117;     nqAJZBZAitkQtyB57022117 = nqAJZBZAitkQtyB73182329;     nqAJZBZAitkQtyB73182329 = nqAJZBZAitkQtyB62084054;     nqAJZBZAitkQtyB62084054 = nqAJZBZAitkQtyB33064767;     nqAJZBZAitkQtyB33064767 = nqAJZBZAitkQtyB33675904;     nqAJZBZAitkQtyB33675904 = nqAJZBZAitkQtyB97976400;     nqAJZBZAitkQtyB97976400 = nqAJZBZAitkQtyB84175550;     nqAJZBZAitkQtyB84175550 = nqAJZBZAitkQtyB77617419;     nqAJZBZAitkQtyB77617419 = nqAJZBZAitkQtyB71928534;     nqAJZBZAitkQtyB71928534 = nqAJZBZAitkQtyB34254812;     nqAJZBZAitkQtyB34254812 = nqAJZBZAitkQtyB6765363;     nqAJZBZAitkQtyB6765363 = nqAJZBZAitkQtyB38394441;     nqAJZBZAitkQtyB38394441 = nqAJZBZAitkQtyB82266408;     nqAJZBZAitkQtyB82266408 = nqAJZBZAitkQtyB84291199;     nqAJZBZAitkQtyB84291199 = nqAJZBZAitkQtyB69310049;     nqAJZBZAitkQtyB69310049 = nqAJZBZAitkQtyB93517070;     nqAJZBZAitkQtyB93517070 = nqAJZBZAitkQtyB37087092;     nqAJZBZAitkQtyB37087092 = nqAJZBZAitkQtyB60493284;     nqAJZBZAitkQtyB60493284 = nqAJZBZAitkQtyB25520028;     nqAJZBZAitkQtyB25520028 = nqAJZBZAitkQtyB6065579;}
// Junk Finished
