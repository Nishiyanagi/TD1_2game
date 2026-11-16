#pragma once

class Pads {
public:

	void Show();//デバック面
	void Input();//入力操作


private:

	int n_{};//入力回数

	int Rx_{};//右スティックX
	int Ry_{};//右スティックY

	int Lx_{};//左スティックX
	int Ly_{};//左スティックY
};