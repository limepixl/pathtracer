#pragma once

void *CreateThreadLinux(void *param);
void WaitForThreadLinux(void *thread_handle);
bool CanThreadStartLinux(void *thread_handle);