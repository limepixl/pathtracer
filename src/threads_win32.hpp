#pragma once

void *CreateThreadWin32(void *param);
void WaitForThreadWin32(void *threadHandle);
void CloseThreadWin32(void *threadHandle);
bool CanThreadStartWin32(void *handle);