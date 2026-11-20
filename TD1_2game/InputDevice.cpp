#include "InputDevice.h"
#include <Novice.h>
#include <string.h>

namespace {
	bool g_usePad = false;   // 最初はキーボード+マウス
}

namespace InputDevice {

	bool UsePad() { return g_usePad; }

	void SetUsePad(bool usePad) { g_usePad = usePad; }

	void Toggle() { g_usePad = !g_usePad; }

	void FilterKeys(const char* src, char* dst)
	{
		if (g_usePad) {
			memset(dst, 0, 256);
		}
		else {
			memcpy(dst, src, 256);
		}
	}

	bool IsPressMouse(int button)
	{
		return !g_usePad && Novice::IsPressMouse(button);
	}

	void GetMousePosition(int* x, int* y)
	{
		if (g_usePad) {
			*x = 0;
			*y = 0;
			return;
		}
		Novice::GetMousePosition(x, y);
	}

	bool IsPressButton(int button)
	{
		return g_usePad && Novice::IsPressButton(0, static_cast<PadButton>(button));
	}

	bool IsTriggerButton(int button)
	{
		return g_usePad && Novice::IsTriggerButton(0, static_cast<PadButton>(button));
	}

	void GetAnalogInputLeft(int* x, int* y)
	{
		if (!g_usePad) {
			*x = 0;
			*y = 0;
			return;
		}
		Novice::GetAnalogInputLeft(0, x, y);
	}

	void GetAnalogInputRight(int* x, int* y)
	{
		if (!g_usePad) {
			*x = 0;
			*y = 0;
			return;
		}
		Novice::GetAnalogInputRight(0, x, y);
	}

} // namespace InputDevice