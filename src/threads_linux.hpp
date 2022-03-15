#pragma once

void *CreateThreadLinux(void *param);
void WaitForThreadLinux(void *threadHandle);
bool CanThreadStartLinux(void *threadHandle);