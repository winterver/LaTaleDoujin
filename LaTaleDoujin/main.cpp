#include "D3D11Application.h"
#include <crtdbg.h>

#if defined(_DEBUG)
int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#else
int WINAPI WinMain() {
#endif

	D3D11Application d3d11App(L"La Tale Doujin", 1360, 768);

	if (!d3d11App.Init())
		return -1;

	return d3d11App.Run();
}