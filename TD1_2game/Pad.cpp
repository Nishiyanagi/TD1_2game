#include <Novice.h>
#include "Pad.h"

/*
kPadButton0  ・・・ 上十字キー
kPadButton1  ・・・ 下十字キー
kPadButton2  ・・・ 左十字キー
kPadButton3  ・・・ 右十字キー
kPadButton4  ・・・ START		「≡ボタン」
kPadButton5  ・・・ BACK		「□が斜めに二つ並んでるボタン」
kPadButton6  ・・・ L3			「Lスティック押し込み」
kPadButton7  ・・・ R3			「Rスティック押し込み」
kPadButton8  ・・・ L1
kPadButton9  ・・・ R1
kPadButton10 ・・・ A
kPadButton11 ・・・ B
kPadButton12 ・・・ X
kPadButton13 ・・・ Y
kPadButton14 ・・・ L2
kPadButton15 ・・・ R2
*/
void Pads::Input() {
	Novice::GetAnalogInputRight(0, &Rx_, &Ry_);
	Novice::GetAnalogInputLeft(0, &Lx_, &Ly_);


	n_ = 0;
	
	/*if ( Novice::IsTriggerButton(0, kPadButton14)) {

		n_++;

	}*/

	
	if (Novice::IsTriggerButton(0, kPadButton8) || Novice::IsTriggerButton(0, kPadButton14)) {//L1かL2を押したら

		n_++;

	}

	if (Novice::IsTriggerButton(0, kPadButton9) || Novice::IsTriggerButton(0, kPadButton15)) {//R1かR2を押したら

		n_++;

	}
	/*
	*/
	


}


void Pads::Show() {
	Novice::ScreenPrintf(0, 100, "n = %d", n_);

	Novice::ScreenPrintf(0, 200, "Rx = %d,Ry = %d", Rx_, Ry_);
	Novice::ScreenPrintf(0, 220, "Lx = %d,Ly = %d", Lx_, Ly_);

}
