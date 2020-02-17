#pragma once
#include "vfunc_hook.hpp"
#include "Protobuffs.h"
class Protobuffs;
extern Protobuffs* zwrite;
namespace Installer
{
	inline vfunc_hook nethook;
	inline vfunc_hook ananýnamý;
	inline vfunc_hook direct3d_hook;
	unsigned long __stdcall LoadLumi(void *unused);
	void UnloadLumi();

}