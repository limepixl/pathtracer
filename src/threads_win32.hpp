#pragma once

void *CreateThreadWin32(void *param);
void WaitForThreadWin32(void *thread_handle);
void CloseThreadWin32(void *thread_handle);
bool CanThreadStartWin32(void *handle);