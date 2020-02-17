#include "Install.hpp"
#include "Gamehooking.hpp"
#include "helpers/Utils.hpp"
#include "features/Glow.hpp"
#include "features/PlayerHurt.hpp"
#include "features/BulletImpact.hpp"
#include "features/KitParser.hpp"
#include "intrin.h"
#include "Dllmain.h"
#include <thread>
#include <chrono>
#include "Protobuffs.h"
#include "vfunc_hook.hpp"
#include "Gameui.h"


IVEngineClient			*g_EngineClient = nullptr;
IBaseClientDLL			*g_CHLClient = nullptr;
ISteamGameCoordinator* g_SteamGameCoordinator = nullptr;
IClientEntityList		*g_EntityList = nullptr;
CGlobalVarsBase			*g_GlobalVars = nullptr;
IEngineTrace			*g_EngineTrace = nullptr;
ICvar					*g_CVar = nullptr;
IPanel					*g_VGuiPanel = nullptr;
IClientMode				*g_ClientMode = nullptr;
IVDebugOverlay			*g_DebugOverlay = nullptr;
ISurface				*g_VGuiSurface = nullptr;
CInput					*g_Input = nullptr;
IVModelInfoClient		*g_MdlInfo = nullptr;
IVModelRender			*g_MdlRender = nullptr;
IVRenderView			*g_RenderView = nullptr;
IMaterialSystem			*g_MatSystem = nullptr;
IGameEventManager2		*g_GameEvents = nullptr;
IMoveHelper				*g_MoveHelper = nullptr;
IDirect3DDevice9		*g_D3DDevice9 = nullptr;
IMDLCache				*g_MdlCache = nullptr;
IPrediction				*g_Prediction = nullptr;
CGameMovement			*g_GameMovement = nullptr;
IEngineSound			*g_EngineSound = nullptr;
CGlowObjectManager		*g_GlowObjManager = nullptr;
CClientState			*g_ClientState = nullptr;
IPhysicsSurfaceProps	*g_PhysSurface = nullptr;
IInputSystem			*g_InputSystem = nullptr;
DWORD					*g_InputInternal = nullptr;
IMemAlloc				*g_pMemAlloc = nullptr;
IViewRenderBeams	    *g_RenderBeams = nullptr;
ILocalize				*g_Localize = nullptr;
C_BasePlayer		    *g_LocalPlayer = nullptr;

namespace Offsets
{
	DWORD invalidateBoneCache = 0x00;
	DWORD smokeCount = 0x00;
	DWORD playerResource = 0x00;
	DWORD bOverridePostProcessingDisable = 0x00;
	DWORD getSequenceActivity = 0x00;
	DWORD lgtSmoke = 0x00;
	DWORD dwCCSPlayerRenderablevftable = 0x00;
	DWORD reevauluate_anim_lod = 0x00;
}

std::unique_ptr<ShadowVTManager> g_pVguiPanelHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pClientModeHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pVguiSurfHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pD3DDevHook = nullptr;
std::unique_ptr<ShadowVTManager> g_D3DDevice = nullptr;
std::unique_ptr<ShadowVTManager> g_pClientHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pMaterialSystemHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pDMEHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pInputInternalHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pSceneEndHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pFireBulletHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pPredictionHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pConvarHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pClientStateHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pNetChannelHook = nullptr;
std::unique_ptr<ShadowVTManager> g_pEngineClientHook = nullptr;


PaintTraverse_t o_PaintTraverse = nullptr;
CreateMove_t o_CreateMove = nullptr;
EndScene_t o_EndScene = nullptr;
Reset_t o_Reset = nullptr;
FrameStageNotify_t o_FrameStageNotify = nullptr;
FireEventClientSide_t o_FireEventClientSide = nullptr;
BeginFrame_t o_BeginFrame = nullptr;
OverrideView_t o_OverrideView = nullptr;
SetMouseCodeState_t o_SetMouseCodeState = nullptr;
SetKeyCodeState_t o_SetKeyCodeState = nullptr;
FireBullets_t o_FireBullets = nullptr;
InPrediction_t o_OriginalInPrediction = nullptr;
TempEntities_t o_TempEntities = nullptr;
SceneEnd_t o_SceneEnd = nullptr;
SetupBones_t o_SetupBones = nullptr;
GetBool_t o_GetBool = nullptr;
GetViewmodelFov_t o_GetViewmodelFov = nullptr;

RunCommand_t o_RunCommand = nullptr;
SendDatagram_t o_SendDatagram = nullptr;
WriteUsercmdDeltaToBuffer_t o_WriteUsercmdDeltaToBuffer = nullptr;
IsBoxVisible_t o_IsBoxVisible = nullptr;
IsHLTV_t o_IsHLTV = nullptr;
CusorFunc_t o_LockCursor = nullptr;
CusorFunc_t o_UnlockCursor = nullptr;

RecvVarProxyFn o_didSmokeEffect = nullptr;
RecvVarProxyFn o_nSequence = nullptr;

HWND window = nullptr;
WNDPROC oldWindowProc = nullptr;



Protobuffs* zwrite;
enum EGCResultxd
{
	k_EGCResultOKxd = 0,
	k_EGCResultNoMessagexd = 1,           // There is no message in the queue
	k_EGCResultBufferTooSmallxd = 2,      // The buffer is too small for the requested message
	k_EGCResultNotLoggedOnxd = 3,         // The client is not logged onto Steam
	k_EGCResultInvalidMessagexd = 4,      // Something was wrong with the message being sent with SendMessage
};
using GCRetrieveMessage = EGCResultxd(__thiscall*)(void*, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize);
using GCSendMessage = EGCResultxd(__thiscall*)(void*, uint32_t unMsgType, const void* pubData, uint32_t cubData);
EGCResultxd __fastcall hkGCRetrieveMessage(void* ecx, void*, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)
{
	static auto oGCRetrieveMessage = Installer::nethook.get_original<GCRetrieveMessage>(2);
	auto status = oGCRetrieveMessage(ecx, punMsgType, pubDest, cubDest, pcubMsgSize);
	if (status == k_EGCResultOK)
	{

		void* thisPtr = nullptr;
		__asm mov thisPtr, ebx;
		auto oldEBP = *reinterpret_cast<void**>((uint32_t)_AddressOfReturnAddress() - 4);

		uint32_t messageType = *punMsgType & 0x7FFFFFFF;
		zwrite->ReceiveMessage(thisPtr, oldEBP, messageType, pubDest, cubDest, pcubMsgSize);
		//if (g_Menu.Config.autoaccept && !g_pEngine->IsConnected() && !g_pEngine->IsInGame())
		{
			//	static auto SetLocalPlayerReadyFn = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));
			//	if (SetLocalPlayerReadyFn) SetLocalPlayerReadyFn("");
		}
	}
	return status;
}
EGCResultxd __fastcall hkGCSendMessage(void* ecx, void*, uint32_t unMsgType, const void* pubData, uint32_t cubData)
{
	static auto oGCSendMessage = Installer::nethook.get_original<GCSendMessage>(0);

	EGCResult status;

	bool sendMessage = zwrite->PreSendMessage(unMsgType, const_cast<void*>(pubData), cubData);

	if (!sendMessage)
		return k_EGCResultOKxd;

	return oGCSendMessage(ecx, unMsgType, const_cast<void*>(pubData), cubData);
}


Interfaces::Interfaces() noexcept
{
	client = find<Client>(L"client_panorama", "VClient018");
	cvar = find<Cvar>(L"vstdlib", "VEngineCvar007");
	engine = find<Engine>(L"engine", "VEngineClient014");
	engineTrace = find<EngineTrace>(L"engine", "EngineTraceClient004");
	entityList = find<EntityList>(L"client_panorama", "VClientEntityList003");
	gameEventManager = find<GameEventManager>(L"engine", "GAMEEVENTSMANAGER002");
	gameUI = find<GameUI>(L"client_panorama", "GameUI011");
	inputSystem = find<InputSystem>(L"inputsystem", "InputSystemVersion001");
	localize = find<Localize>(L"localize", "Localize_001");
	materialSystem = find<MaterialSystem>(L"materialsystem", "VMaterialSystem080");
	modelInfo = find<ModelInfo>(L"engine", "VModelInfoClient004");
	modelRender = find<ModelRender>(L"engine", "VEngineModel016");
	panel = find<Panel>(L"vgui2", "VGUI_Panel009");
	physicsSurfaceProps = find<PhysicsSurfaceProps>(L"vphysics", "VPhysicsSurfaceProps001");
	renderView = find<RenderView>(L"engine", "VEngineRenderView014");
	resourceAccessControl = find<ResourceAccessControl>(L"datacache", "VResourceAccessControl001");
	surface = find<Surface>(L"vguimatsurface", "VGUI_Surface031");
	sound = find<Sound>(L"engine", "IEngineSoundClient003");
	soundEmitter = find<SoundEmitter>(L"soundemittersystem", "VSoundEmitter003");

}


#include "Options.hpp"
using ScreenAspectRatio = void(__thiscall*)(void* pEcx, void* pEdx, int32_t iWidth, int32_t iHeight);
float __fastcall Hook_GetScreenAspectRatio(void* pEcx, void* pEdx, int32_t iWidth, int32_t iHeight)
{
	static auto ofunct = Installer::ananýnamý.get_original<ScreenAspectRatio>(101);
	int x, y; g_EngineClient->GetScreenSize(x, y);
	if (!g_Options.aspectratioenable) return ((float)x / y);
	else return ((float)g_Options.aspectratioen_x / g_Options.aspectratioen_y);
	ofunct(pEcx, pEdx, iWidth, iHeight);
}



unsigned long __stdcall Installer::LoadLumi(void *unused)
{
	FUNCTION_GUARD;

	while (!GetModuleHandleA("serverbrowser.dll"))
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	auto SteamClient = ((ISteamClient * (__cdecl*)(void))GetProcAddress(GetModuleHandleA("steam_api.dll"), "SteamClient"))();
	g_CHLClient = Iface::IfaceMngr::getIface<IBaseClientDLL>("client_panorama.dll", "VClient0");
	g_EntityList = Iface::IfaceMngr::getIface<IClientEntityList>("client_panorama.dll", "VClientEntityList");
	g_Prediction = Iface::IfaceMngr::getIface<IPrediction>("client_panorama.dll", "VClientPrediction");
	g_GameMovement = Iface::IfaceMngr::getIface<CGameMovement>("client_panorama.dll", "GameMovement");
	g_MdlCache = Iface::IfaceMngr::getIface<IMDLCache>("datacache.dll", "MDLCache");
	g_EngineClient = Iface::IfaceMngr::getIface<IVEngineClient>("engine.dll", "VEngineClient");
	g_MdlInfo = Iface::IfaceMngr::getIface<IVModelInfoClient>("engine.dll", "VModelInfoClient");
	g_MdlRender = Iface::IfaceMngr::getIface<IVModelRender>("engine.dll", "VEngineModel");
	g_RenderView = Iface::IfaceMngr::getIface<IVRenderView>("engine.dll", "VEngineRenderView");
	g_EngineTrace = Iface::IfaceMngr::getIface<IEngineTrace>("engine.dll", "EngineTraceClient");
	g_DebugOverlay = Iface::IfaceMngr::getIface<IVDebugOverlay>("engine.dll", "VDebugOverlay");
	g_GameEvents = Iface::IfaceMngr::getIface<IGameEventManager2>("engine.dll", "GAMEEVENTSMANAGER002", true);
	g_EngineSound = Iface::IfaceMngr::getIface<IEngineSound>("engine.dll", "IEngineSoundClient");
	g_MatSystem = Iface::IfaceMngr::getIface<IMaterialSystem>("materialsystem.dll", "VMaterialSystem");
	g_CVar = Iface::IfaceMngr::getIface<ICvar>("vstdlib.dll", "VEngineCvar");
	g_VGuiPanel = Iface::IfaceMngr::getIface<IPanel>("vgui2.dll", "VGUI_Panel");
	g_VGuiSurface = Iface::IfaceMngr::getIface<ISurface>("vguimatsurface.dll", "VGUI_Surface");
	g_PhysSurface = Iface::IfaceMngr::getIface<IPhysicsSurfaceProps>("vphysics.dll", "VPhysicsSurfaceProps");
	g_InputSystem = Iface::IfaceMngr::getIface<IInputSystem>("inputsystem.dll", "InputSystemVersion");
	g_InputInternal = Iface::IfaceMngr::getIface<DWORD>("vgui2.dll", "VGUI_InputInternal");
	g_Localize = Iface::IfaceMngr::getIface<ILocalize>("localize.dll", "Localize_");

	g_GlobalVars = **(CGlobalVarsBase***)((*(DWORD**)(g_CHLClient))[0] + 0x1B);
	
	g_ClientMode = **(IClientMode***)((*(DWORD**)g_CHLClient)[10] + 0x5);
	g_pMemAlloc = *(IMemAlloc**)(GetProcAddress(GetModuleHandle("tier0.dll"), "g_pMemAlloc"));

	auto client = GetModuleHandle("client_panorama.dll");
	auto engine = GetModuleHandle("engine.dll");
	auto dx9api = GetModuleHandle("shaderapidx9.dll");
	g_SteamGameCoordinator = (ISteamGameCoordinator*)SteamClient->GetISteamGenericInterface((void*)1, (void*)1, "SteamGameCoordinator001");
	g_ClientState = **(CClientState***)(Utils::PatternScan(engine, "A1 ? ? ? ? 8B 80 ? ? ? ? C3") + 1);
	g_GlowObjManager = *(CGlowObjectManager**)(Utils::PatternScan(client, "0F 11 05 ? ? ? ? 83 C8 01") + 3);
	g_MoveHelper = **(IMoveHelper***)(Utils::PatternScan(client, "8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01") + 2);
	g_RenderBeams = *(IViewRenderBeams**)(Utils::PatternScan(client, "A1 ? ? ? ? FF 10 A1 ? ? ? ? B9") + 0x1);
	g_Input = *(CInput**)(Utils::PatternScan(client, "B9 ? ? ? ? F3 0F 11 04 24 FF 50 10") + 0x1);

	auto D3DDevice9 = **(IDirect3DDevice9***)(Utils::PatternScan(dx9api, "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);
	auto dwFireBullets = *(DWORD**)(Utils::PatternScan(client, "55 8B EC 51 53 56 8B F1 BB ? ? ? ? B8") + 0x131);
	g_D3DDevice9 = **(IDirect3DDevice9 * **)(Utils::PatternScan(dx9api, "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);
	Offsets::lgtSmoke = (DWORD)Utils::PatternScan(client, "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
	Offsets::getSequenceActivity = (DWORD)Utils::PatternScan(client, "55 8B EC 53 8B 5D 08 56 8B F1 83");
	Offsets::smokeCount = *(DWORD*)(Utils::PatternScan(client, "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0") + 0x8);
	Offsets::invalidateBoneCache = (DWORD)Utils::PatternScan(client, "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81");
	Offsets::playerResource = *(DWORD*)(Utils::PatternScan(client, "8B 3D ? ? ? ? 85 FF 0F 84 ? ? ? ? 81 C7") + 2);
	Offsets::bOverridePostProcessingDisable = *(DWORD*)(Utils::PatternScan(client, "80 3D ? ? ? ? ? 53 56 57 0F 85") + 2);
	Offsets::dwCCSPlayerRenderablevftable = *(DWORD*)(Utils::PatternScan(client, "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C") + 0x4E);
	Offsets::reevauluate_anim_lod = (DWORD)Utils::PatternScan(client, "84 C0 0F 85 ? ? ? ? A1 ? ? ? ? 8B B7");

	NetMngr::Get().init();

	g_pDMEHook = std::make_unique<ShadowVTManager>();
	g_pD3DDevHook = std::make_unique<ShadowVTManager>();
	g_pClientHook = std::make_unique<ShadowVTManager>();
	g_pSceneEndHook = std::make_unique<ShadowVTManager>();
	g_pVguiPanelHook = std::make_unique<ShadowVTManager>();
	g_pVguiSurfHook = std::make_unique<ShadowVTManager>();
	g_pClientModeHook = std::make_unique<ShadowVTManager>();
	g_pPredictionHook = std::make_unique<ShadowVTManager>();
	g_pFireBulletHook = std::make_unique<ShadowVTManager>();
	g_pMaterialSystemHook = std::make_unique<ShadowVTManager>();
	g_pInputInternalHook = std::make_unique<ShadowVTManager>();
	g_pConvarHook = std::make_unique<ShadowVTManager>();
	g_pClientStateHook = std::make_unique<ShadowVTManager>();
	g_pEngineClientHook = std::make_unique<ShadowVTManager>();

	g_pFireBulletHook->Setup((PDWORD)dwFireBullets);
	g_pDMEHook->Setup(g_MdlRender);
	g_pD3DDevHook->Setup(D3DDevice9);
	g_pClientHook->Setup(g_CHLClient);
	g_pSceneEndHook->Setup(g_RenderView);
	g_pVguiPanelHook->Setup(g_VGuiPanel);
	g_pVguiSurfHook->Setup(g_VGuiSurface);
	direct3d_hook.setup(g_D3DDevice9);
	constexpr auto skechbaba = 42;
	constexpr auto trixbaba = 16;
	direct3d_hook.hook_index(skechbaba, Handlers::EndScene_h);
	direct3d_hook.hook_index(trixbaba, Handlers::Reset_h);
	g_pClientModeHook->Setup(g_ClientMode);
	nethook.setup(g_SteamGameCoordinator);
	g_pPredictionHook->Setup(g_Prediction);
	g_pMaterialSystemHook->Setup(g_MatSystem);
	g_pInputInternalHook->Setup(g_InputInternal);
	nethook.hook_index(0, hkGCSendMessage);
	nethook.hook_index(2, hkGCRetrieveMessage);
	g_pConvarHook->Setup(g_CVar->FindVar("sv_cheats"));
	g_pEngineClientHook->Setup(g_EngineClient);

	window = FindWindow("Valve001", NULL);
	oldWindowProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)Handlers::WndProc_h);
	interfaces.gameUI->messageBox(u8"TrixWare traduzido ENG-US by ch4z!", u8"https://fantailcommunity.xyz | https://discord.gg/hGRZrXN");


	g_pFireBulletHook->Hook(7, (FireBullets_t)Handlers::TEFireBulletsPostDataUpdate_h);
	g_pInputInternalHook->Hook(92, Handlers::SetMouseCodeState_h);
	g_pInputInternalHook->Hook(91, Handlers::SetKeyCodeState_h);
	g_pClientModeHook->Hook(35, Handlers::GetViewModelFov_h);
	g_pClientModeHook->Hook(18, Handlers::OverrideView_h);
	g_pClientModeHook->Hook(24, Handlers::CreateMove_h);
	g_pClientHook->Hook(37, Handlers::FrameStageNotify_h);
	g_pPredictionHook->Hook(14, Handlers::InPrediction_h);
	g_pPredictionHook->Hook(19, Handlers::RunCommand_h);
	g_pVguiPanelHook->Hook(41, Handlers::PaintTraverse_h);
	g_pMaterialSystemHook->Hook(42, Handlers::BeginFrame_h);
	g_pConvarHook->Hook(13, Handlers::GetBool_SVCheats_h);
	g_pVguiSurfHook->Hook(67, Handlers::LockCursor_h);
	g_pSceneEndHook->Hook(9, Handlers::SceneEnd_h);
	g_pD3DDevHook->Hook(42, Handlers::EndScene_h);
	g_pD3DDevHook->Hook(16, Handlers::Reset_h);
	g_pEngineClientHook->Hook(32, Handlers::IsBoxVisible_h);
	g_pEngineClientHook->Hook(93, Handlers::IsHLTV_h);
	ananýnamý.setup(g_EngineClient);
	ananýnamý.hook_index(101, Hook_GetScreenAspectRatio);

	o_FireBullets = g_pFireBulletHook->GetOriginal<FireBullets_t>(7);
	o_SetMouseCodeState = g_pInputInternalHook->GetOriginal<SetMouseCodeState_t>(92);
	o_SetKeyCodeState = g_pInputInternalHook->GetOriginal<SetKeyCodeState_t>(91);
	o_GetViewmodelFov = g_pClientModeHook->GetOriginal<GetViewmodelFov_t>(35);
	o_OverrideView = g_pClientModeHook->GetOriginal<OverrideView_t>(18);
	o_CreateMove = g_pClientModeHook->GetOriginal<CreateMove_t>(24);
	o_FrameStageNotify = g_pClientHook->GetOriginal<FrameStageNotify_t>(37);
	o_OriginalInPrediction = g_pPredictionHook->GetOriginal<InPrediction_t>(14);
	o_RunCommand = g_pPredictionHook->GetOriginal<RunCommand_t>(19);
	o_PaintTraverse = g_pVguiPanelHook->GetOriginal<PaintTraverse_t>(41);
	o_BeginFrame = g_pMaterialSystemHook->GetOriginal<BeginFrame_t>(42);
	o_GetBool = g_pConvarHook->GetOriginal<GetBool_t>(13);
	o_SceneEnd = g_pSceneEndHook->GetOriginal<SceneEnd_t>(9);
	o_EndScene = g_pD3DDevHook->GetOriginal<EndScene_t>(42);
	o_Reset = g_pD3DDevHook->GetOriginal<Reset_t>(16);
	o_IsBoxVisible = g_pEngineClientHook->GetOriginal<IsBoxVisible_t>(32);
	o_IsHLTV = g_pEngineClientHook->GetOriginal<IsHLTV_t>(93);
	o_UnlockCursor = g_pVguiSurfHook->GetOriginal<CusorFunc_t>(66);
	o_LockCursor = g_pVguiSurfHook->GetOriginal<CusorFunc_t>(67);

#ifdef INSTANT_DEFUSE_PLANT_EXPLOIT
	o_WriteUsercmdDeltaToBuffer = g_pClientHook->Hook(23, (WriteUsercmdDeltaToBuffer_t)Handlers::WriteUsercmdDeltaToBuffer_h);
#endif

	InitializeKits();

	NetMngr::Get().hookProp("CSmokeGrenadeProjectile", "m_bDidSmokeEffect", Proxies::didSmokeEffect, o_didSmokeEffect);
	NetMngr::Get().hookProp("CBaseViewModel", "m_nSequence", Proxies::nSequence, o_nSequence);
	PlayerHurtEvent::Get().RegisterSelf();
	BulletImpactEvent::Get().RegisterSelf();



	Utils::AttachConsole();

	Utils::ConsolePrint(true, "-= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -\n");
	Utils::ConsolePrint(true, "         Loading Cheat. Do not close this Window \n", __DATE__, __TIME__);
	Utils::ConsolePrint(true, "-= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -= -\n");
	Utils::ConsolePrint(true, "Downloading cheat...\n");
	Utils::ConsolePrint(true, "TriX Ware Reborn Injected!\n");
	Utils::ConsolePrint(true, "Translate by ch4z!\n");
	Utils::ConsolePrint(true, "Forum -> https://fantailcommunity.xyz \n");
	Utils::ConsolePrint(true, "Discord -> https://discord.gg/hGRZrXN \n");

	return EXIT_SUCCESS;
}

void Installer::UnloadLumi()
{
	Glow::ClearGlow();
	PlayerHurtEvent::Get().UnregisterSelf();
	BulletImpactEvent::Get().UnregisterSelf();

	SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)oldWindowProc);

	g_pDMEHook->RestoreTable();
	g_pD3DDevHook->RestoreTable();
	g_pClientHook->RestoreTable();
	g_pSceneEndHook->RestoreTable();
	g_pVguiPanelHook->RestoreTable();
	g_pVguiSurfHook->RestoreTable();
	g_pClientModeHook->RestoreTable();
	g_pPredictionHook->RestoreTable();
	g_pMaterialSystemHook->RestoreTable();
	g_pInputInternalHook->RestoreTable();
	g_pConvarHook->RestoreTable();
	g_pEngineClientHook->RestoreTable();
	g_pClientStateHook->RestoreTable();
}
