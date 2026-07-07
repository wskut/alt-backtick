#pragma once

#include <windows.h>
#include <vector>
#include "ProcessList.h"

bool CreateOverlayWindow(HINSTANCE hInstance);
void DestroyOverlayWindow();
void ShowOverlay(const std::vector<ProcessEntry>& list, size_t selection);
void UpdateSelection(size_t selection);
void HideOverlay();
bool IsOverlayVisible();
