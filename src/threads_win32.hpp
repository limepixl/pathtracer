#pragma once

void *CreateThreadWin32(void *param);
void WaitForThreadWin32(void *threadHandle);
void CloseThreadWin32(void *threadHandle);
bool CanThreadStart(void *handle);