#pragma once

// 操作デバイスの切り替え(キーボード+マウス / パッド)。
// 使っていないデバイスの入力は「何も押されていない」ことになる。
//
// 使い方:
//   ・切り替え: InputDevice::SetUsePad(true / false) か InputDevice::Toggle()
//     (今は main.cpp で K キーに割り当て。タイトル/ポーズ画面ができたら SetUsePad を呼ぶ)
//   ・キーボード: main.cpp で FilterKeys() を通した配列を、ゲームの処理に渡す
//   ・マウス/パッド: Novice::IsPressMouse などの代わりに、同名の InputDevice:: の関数を使う
namespace InputDevice {

	bool UsePad();                    // true: パッド操作 / false: キーボード+マウス操作
	void SetUsePad(bool usePad);
	void Toggle();

	// キー入力の配列(256個)をコピーする。パッド操作中は全部 0 になる
	void FilterKeys(const char* src, char* dst);

	// マウス(キーボード+マウス操作のときだけ有効)
	bool IsPressMouse(int button);
	void GetMousePosition(int* x, int* y);   // パッド操作中は 0, 0 を返す

	// パッド(パッド操作のときだけ有効)。button には kPadButton10 などを渡す
	bool IsPressButton(int button);
	bool IsTriggerButton(int button);
	void GetAnalogInputLeft(int* x, int* y);    // キーボード+マウス操作中は 0, 0 を返す
	void GetAnalogInputRight(int* x, int* y);   // キーボード+マウス操作中は 0, 0 を返す

} // namespace InputDevice