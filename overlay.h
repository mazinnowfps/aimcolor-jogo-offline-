#pragma once
#include <windows.h>

bool OverlayCreate(HINSTANCE hInst);
void OverlaySetRadius(int r);
void OverlaySetVisible(bool v);
void OverlaySetFollowCursor(bool follow);   // NOVO
void OverlayDestroy();