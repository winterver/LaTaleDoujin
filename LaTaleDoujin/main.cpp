#include "LaTaleDoujin.h"
#include <crtdbg.h>

#if defined(_DEBUG)
int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#else
int WINAPI WinMain() {
#endif

	LaTaleDoujin latale;

	if (!latale.Init())
		return -1;

	return latale.Run();
}