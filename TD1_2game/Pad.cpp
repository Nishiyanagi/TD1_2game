
#include <Novice.h>


int n = 0;

int Rx;
int Ry;

int Lx;
int Ly;

if (Novice::IsTriggerButton(0, kPadButton8) || Novice::IsTriggerButton(0, kPadButton10)) {

	n = 1;

}
if (Novice::IsTriggerButton(0, kPadButton9) || Novice::IsTriggerButton(0, kPadButton11)) {

	n = 1;

}

Novice::GetAnalogInputRight(0, &Rx, &Ry);
Novice::GetAnalogInputLeft(0, &Lx, &Ly);

Novice::ScreenPrintf(0, 100, "n = %d", n);


Novice::ScreenPrintf(0, 200, "Rx = %d,Ry = %d", Rx, Ry);
Novice::ScreenPrintf(0, 220, "Lx = %d,Ly = %d", Lx, Ly);
